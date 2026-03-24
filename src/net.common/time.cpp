#include "pch.h"
#include "time.h"

namespace net::common
{
    time::time()
    {
        last_tick_ = std::chrono::steady_clock::now();
    }

    void time::tick()
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = now - last_tick_;
        dt_ = elapsed.count();
        last_tick_ = now;
    }
}
