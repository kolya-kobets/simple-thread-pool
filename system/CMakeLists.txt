find_package(Threads REQUIRED)

add_library(system STATIC log.h log.cpp taskrunner.h taskrunner.cpp worker.h worker.cpp)
target_link_libraries(system PRIVATE Threads::Threads)

target_include_directories(system PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

