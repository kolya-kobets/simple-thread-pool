#ifndef TASKRUNNER_H
#define TASKRUNNER_H

#include "worker.h"
#include <queue>
#include <list>
#include <atomic>

struct TaskInfo
{
    Task task;
    int priority = 0;
};

inline bool cmpPriorities(const TaskInfo& left, const TaskInfo& right) {
    return left.priority < right.priority;
}


/**
 * @brief TaskRunner class - runs tasks asynchroniously according to specified priorities.
 */
class TaskRunner
{
public:
    TaskRunner(int maxWorkers = 8);
    ~TaskRunner();
public:
    void run(const Task& task, int priority);
private:
    void schedule(const Task& task, int priority);
    void run_tasks();
    bool run_task(const Task& taskInfo);

    std::priority_queue<TaskInfo, std::vector<TaskInfo>, decltype(&cmpPriorities)> m_waiting_tasks;
    std::mutex m_tasks_lock;

    std::list<Worker> m_workers;
    std::mutex m_workers_lock;

    const int m_maxWorkers;
    std::atomic_bool m_stopped = false;
};
#endif // TASKRUNNER_H
