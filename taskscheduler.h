#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include <memory>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

//
// class Task
//
class Task
{
public:
    virtual ~Task() {}
public:
    virtual void execute() = 0;
};



//
// class WorkingThread
//

class WorkingThread
{
public:
    WorkingThread(std::function<void()> notifyReady);
    bool run(std::function<void()> task);
    ~WorkingThread();
private:
    void runner();
    std::function<void()> m_notifyReady;

    std::function<void()> m_task;

    std::mutex m_mutex;
    std::condition_variable m_cond_variable;
    bool m_stopped = false;

    std::thread m_thread;
};

//
// TaskInfo
//
struct TaskInfo {
    using sys_time = std::chrono::time_point<std::chrono::system_clock>;
    std::shared_ptr<Task> task;
    sys_time next_call;
    std::chrono::milliseconds period;
    bool repeat;
};

inline bool compareByNextCall(const TaskInfo& left, const TaskInfo& right) {
    return left.next_call < right.next_call;
}


//
// Timer
//
class Timer {
public:
    Timer();
    ~Timer();
    void runTimer(TaskInfo::sys_time timePoint, std::function<void()> func);
    TaskInfo::sys_time timePoint();
    void stop();
private:
    void run();

    TaskInfo::sys_time m_timePoint = TaskInfo::sys_time::max();
    std::function<void()> m_func = nullptr;
    bool m_stopped = false;

    std::mutex m_mutex;
    std::condition_variable m_cond_variable;

    std::thread m_thread;
};


//
// class TaskScheduler
//
class TaskScheduler
{
public:
    TaskScheduler(int threadsNumber = 1);
    ~TaskScheduler();
public:
    // Add task to scheduler
    void addTask(std::shared_ptr<Task> task, int delayMs = 0, bool repeatable = false);
    // Stop scheduler
    void stop();
private:
    void schedule(std::shared_ptr<Task> task, std::chrono::milliseconds delay, bool repeatable);
    void runTasks();
    bool runTask(const TaskInfo& task);
    void setupTimer(TaskInfo::sys_time timePoint);

    std::priority_queue<TaskInfo, std::vector<TaskInfo>, decltype(&compareByNextCall)> m_waitingTasks;
    std::mutex m_tasks_lock;

    std::list<WorkingThread> m_threadPool;
    bool m_stopped = false;

    Timer m_timer;
};

#endif // TASKSCHEDULER_H
