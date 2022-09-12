#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <thread>
#include <iostream>

extern std::mutex g_print_mtx;

template<class... Args>
void LOGI(Args... args) {
  const auto thread = std::this_thread::get_id();
  std::lock_guard<std::mutex> guard(g_print_mtx);
  std::cout << thread << " : ";
  ((std::cout << ' ' << args), ...);
  std::cout << std::endl;
}

#endif // LOG_H
