#ifndef WORKER_H
#define WORKER_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>

using Task = std::function<void()>;

/**
 * @brief Work incapsulates task and on_ready notification.
 */
struct Work
{
    Task task;
    std::function<void()> on_ready;
};

/**
 * @brief Worker runs tasks asynchoniously in a dedicated thread.
 * If any work is in progress, another one is not allowed to be run.
 */
class Worker
{
public:
    Worker();
    ~Worker();
    bool run(const Work& work);
private:
    void runner();

    Work m_work; //TODO: think pf adding volatile keyword.

    std::mutex m_mutex;
    std::condition_variable m_cond_variable;
    volatile bool m_stopped = false;

    std::thread m_thread;
};

#endif // WORKER_H
