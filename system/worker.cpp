#include "worker.h"

//
// Worker implementation
//

Worker::Worker()
    : m_thread( std::bind(&Worker::runner, this) )
{}

Worker::~Worker() {
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_stopped = true;
        m_cond_variable.notify_one();
    }
    // keep joining instead of detach, so we didn't send on_ready notification for
    // dead clients.
    m_thread.join();
}

bool Worker::run(const Work& work) {
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_work.task) {
        // finish current work first.
        return false;
    }
    m_work = work;
    m_cond_variable.notify_one();
    return true;
}

void Worker::runner() {
    while (!m_stopped) {
        Work work;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond_variable.wait(lock, [this]{return m_work.task || m_stopped;});
            work = m_work;
        }
        if (work.task) {
            work.task();
        }
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_work = Work();
        }
        if (work.on_ready) {
            work.on_ready();
        }
    }
}

