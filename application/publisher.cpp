#include "publisher.h"
#include "log.h"

Publisher::Publisher(const std::string& name, std::chrono::milliseconds run_time, std::chrono::milliseconds publish_delay, int priority, TaskRunner& runner)
    : m_name(name)
    , m_run_time(run_time)
    , m_publish_delay(publish_delay)
    , m_priority(priority)
    , m_runner(runner)
    , m_thread( std::bind(&Publisher::runner, this) )
{
}

Publisher::~Publisher() {
    m_stopped = true;
    m_thread.join();
}

void Publisher::runner() {
    while (!m_stopped) {
        const auto publish = [this]() {
            LOGI("Start ", m_name);
            std::this_thread::sleep_for(m_run_time);
            LOGI("End ", m_name);
        };
        m_runner.run(publish, m_priority);
        std::this_thread::sleep_for(m_publish_delay);
    }
}
