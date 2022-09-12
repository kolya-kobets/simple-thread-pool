#include "application.h"
#include <log.h>
#include <csignal>
#include <chrono>
#include <taskrunner.h>
#include "publisher.h"

std::atomic_bool Application::m_finish = false;

void Application::run()
{
    m_finish = false;

    std::signal(SIGINT, &Application::sighandler);

    LOGI("Application starts.");
    const auto start = std::chrono::steady_clock::now();

    run_publishers();

    const auto end = std::chrono::steady_clock::now();
    LOGI("Application finishes.");

    const auto runTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    LOGI(runTimeMs.count(), " ms.");

    std::signal(SIGINT, SIG_DFL);
}

void Application::run_publishers()
{
    TaskRunner task_runner;

    std::list<Publisher> publishers;
    for(int i=0; i<6; i++) {
        std::string name = "publisher"+std::to_string(i+1);
        const auto run_time = std::chrono::seconds(i+1);
        const auto publish_delay = std::chrono::seconds(i+1);
        int priority = i;
        publishers.emplace_back(name, run_time, publish_delay, priority, task_runner);
    }


    while(!m_finish) {
        //std::cin.ignore();
    }
}

void Application::sighandler(int /*signal*/)
{
    m_finish = true;
}

