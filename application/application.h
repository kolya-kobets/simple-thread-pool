#ifndef APPLICATION_H
#define APPLICATION_H

#include <atomic>

class Application
{
public:
    static void run();
private:
    static void run_publishers();

    static void sighandler(int signal);
    static std::atomic_bool m_finish;
};

#endif // APPLICATION_H
