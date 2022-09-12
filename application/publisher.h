#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <taskrunner.h>
#include <chrono>

class Publisher
{
public:
    Publisher(const std::string& name, std::chrono::milliseconds run_time, std::chrono::milliseconds publish_delay, int priority, TaskRunner& runner);
    ~Publisher();
private:
    void runner();

    const std::string m_name;
    const std::chrono::milliseconds m_run_time;
    const std::chrono::milliseconds m_publish_delay;
    const int m_priority;
    TaskRunner& m_runner;

    std::atomic_bool m_stopped = false;
    std::thread m_thread;
};

#endif // PUBLISHER_H
