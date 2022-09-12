#include "taskrunner.h"

TaskRunner::TaskRunner(int maxWorkers)
    : m_waiting_tasks(&cmpPriorities), m_maxWorkers(maxWorkers)
{}

TaskRunner::~TaskRunner() {
    m_stopped = true;
    m_workers.clear(); // join workers
}

void TaskRunner::run(const Task& task, int priority) {
    if (m_stopped) {
        return;
    }

    schedule(task, priority);
    run_tasks();
}

void TaskRunner::schedule(const Task& task, int priority) {
    std::lock_guard<std::mutex> lock(m_tasks_lock);

    m_waiting_tasks.push(TaskInfo{task, priority});
}

void TaskRunner::run_tasks() {
    if (m_stopped) {
        return;
    }
    std::lock_guard<std::mutex> lock(m_tasks_lock);

    while (!m_waiting_tasks.empty()) {
        const auto& top = m_waiting_tasks.top();
        bool running = run_task(top.task);
        if (running) {
            m_waiting_tasks.pop();
        } else {
            break; // all threads are busy
        }
    }
}

bool TaskRunner::run_task(const Task& task) {
    const Work work = {
        .task = task,
        .on_ready = std::bind(&TaskRunner::run_tasks, this)
    };

    for (auto& worker : m_workers) {
        if (worker.run(work)) {
            return true;
        }
    }

    std::lock_guard lock(m_workers_lock);
    if (m_workers.size() < m_maxWorkers) {
        m_workers.emplace_back();
        return m_workers.back().run(work);
    }

    return false;
}
