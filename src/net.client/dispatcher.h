#pragma once
#include <vector>
#include <thread>
#include <atomic>

class dispatcher
{
public:
    dispatcher(uint8_t thread_count);

    ~dispatcher();

private:
    uint8_t thread_count;

    std::vector<std::thread> workers;

    std::atomic<bool> is_running;
};