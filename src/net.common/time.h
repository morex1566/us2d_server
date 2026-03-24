#pragma once
#include "net.common/singleton.h"
#include <chrono>

namespace net::common
{
    /// <summary>
    /// 전역 시간 관리 및 델타 타임 계산 클래스
    /// </summary>
    class time : public singleton<time>
    {
    public:

        /// <summary>
        /// 마지막 tick() 호출 이후 경과 시간 (초 단위)
        /// </summary>
        float delta_time() const { return dt_; }

        /// <summary>
        /// 현재 프레임의 시작 시각을 기록하고 델타 타임을 업데이트
        /// </summary>
        void tick();

    public:

        time();
        ~time() = default;

    private:

        std::chrono::steady_clock::time_point last_tick_;
        float dt_ = 0.0f;
    };
}
