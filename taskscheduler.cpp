#include "taskscheduler.h"

//
// WorkingThread implementation
//

WorkingThread::WorkingThread(std::function<void()> notifyReady)
    : m_notifyReady(notifyReady),
      m_thread( std::bind(&WorkingThread::runner, this) )
{}

bool WorkingThread::run(std::function<void()> newTask) {
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_task != nullptr) {
        // finish current task first.
        return false;
    }
    m_task = newTask;
    m_condVariable.notify_one();
    return true;
}

void WorkingThread::runner() {
    while (!m_stopped) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condVariable.wait(lock, [this]{return m_task != nullptr || m_stopped;});
            task = m_task;
        }
        if (task) {
            task();
        }
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_task = nullptr;
        }
        if (m_notifyReady) {
            m_notifyReady();
        }

    }
}

WorkingThread::~WorkingThread() {
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_stopped = true;
        m_condVariable.notify_one();
    }
    // keep joining instead of detach (TaskScheduler implementation requirement)
    m_thread.join();
}

//
// Timer implementation
//

Timer::Timer()
    : m_thread( std::bind(&Timer::run, this) )
{}

Timer::~Timer() {
    {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_stopped = true;
    m_condVariable.notify_one();
    }
    m_thread.join();
}

void Timer::runTimer(TaskInfo::sys_time timePoint, std::function<void()> func) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_timePoint = timePoint;
    m_func = func;
    m_condVariable.notify_one();
}

TaskInfo::sys_time Timer::timePoint() {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_timePoint;
}

void Timer::run() {
    while (!m_stopped) {

        bool timerChanged = false;
        std::function<void()> func = nullptr;
        TaskInfo::sys_time timePoint = TaskInfo::sys_time::max();

        {
            std::unique_lock<std::mutex> lock(m_mutex);

            func = m_func;
            timePoint = m_timePoint;

            m_timePoint = TaskInfo::sys_time::max();
            m_func = nullptr;

            m_condVariable.wait_until(lock, timePoint, [this, &func](){ return m_stopped || func || m_func; });
            timerChanged = m_func != nullptr;
        }

        // dont lock mutex when calling func(): it can use runTimer/timePoint routines.
        if (func != nullptr && !timerChanged) {
            func();
        }
    }
}

//
// TaskScheduler implementation
//

TaskScheduler::TaskScheduler(int threadsNumber)
    : m_waitingTasks(&compareByNextCall) {
    for(int i=0; i<threadsNumber; i++) {
        m_threadPool.emplace_back( std::bind(&TaskScheduler::runTasks, this) );
    }
}

TaskScheduler::~TaskScheduler() {
    stop();
}

void TaskScheduler::addTask(std::shared_ptr<Task> task, int delayMs, bool repeatable) {
    if (m_stopped) {
        return;
    }

    schedule(task, std::chrono::milliseconds(delayMs), repeatable);
    runTasks();
}

void TaskScheduler::stop() {
    m_stopped = true;
    m_threadPool.clear();
    m_timer.runTimer(TaskInfo::sys_time::max(), nullptr);
}

void TaskScheduler::schedule(std::shared_ptr<Task> task, std::chrono::milliseconds delay, bool repeatable) {
    std::lock_guard<std::mutex> lock(m_tasksLock);

    TaskInfo::sys_time next_call = std::chrono::system_clock::now() + delay;
    m_waitingTasks.push(TaskInfo{task, next_call, delay, repeatable});
}

void TaskScheduler::runTasks() {
    if (m_stopped) {
        return;
    }
    // this function is called from scheduler & worker thread
    std::lock_guard<std::mutex> lock(m_tasksLock);

    auto now = std::chrono::system_clock::now();
    while (!m_waitingTasks.empty()) {
        const auto& top = m_waitingTasks.top();
        if (top.next_call >= now) {
            setupTimer(top.next_call);
            return;
        }
        bool running = runTask(top);
        if (running) {
            m_waitingTasks.pop();
        } else {
            break; // all threads are busy
        }
    }
}

bool TaskScheduler::runTask(const TaskInfo& taskInfo) {
    auto func = [taskInfo, this]()
    {
        taskInfo.task->execute();
        if (taskInfo.repeat) {
            schedule(taskInfo.task, taskInfo.period, taskInfo.repeat);
        }
    };

    for (auto& thread : m_threadPool) {
        if (thread.run(func)) {
            return true;
        }
    }
    return false;
}

void TaskScheduler::setupTimer(TaskInfo::sys_time timePoint) {
    if (m_timer.timePoint() > timePoint) {
        m_timer.runTimer(timePoint, [this](){ runTasks(); });
    }
}
