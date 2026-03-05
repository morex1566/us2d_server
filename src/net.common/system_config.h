#pragma once

#include <cstdint>

namespace net::common
{
    /// <summary>
/// 시스템 하드웨어 상태 조회 유틸리티 (Thread-safe, stateless)
/// </summary>
    class system_config
    {

    public:

        /// <summary>
        /// 현재 사용 가능한 물리 RAM (bytes)
        /// </summary>
        static uint64_t available_ram_bytes();

        /// <summary>
        /// 현재 사용 가능한 물리 RAM 비율 (0.0 ~ 100.0)
        /// </summary>
        static double available_ram_percent();

        /// <summary>
        /// 현재 CPU 사용률 (0.0 ~ 100.0), 100ms 샘플링
        /// </summary>
        static double cpu_usage_percent();

    public:

        class tcp
        {
        public:
            /// <summary>
            /// 서버 -> 클라 패킷 전송 주기
            /// </summary>
            static constexpr uint8_t tick_rate = 30;
        };

        class session
        {
        public:
            /// <summary>
            /// 유저가 할당받는 패킷 풀 사이즈
            /// </summary>
            static constexpr uint16_t packets_per_user = 128;
        };

        class ts_memory_pool
        {
        public:
            /// <summary>
            /// 이벤트 기간 등 갑작스러운 유저 유입 대비
            /// </summary>
            static constexpr double memory_alloc_limit_percent = 80.f;

            /// <summary>
            /// 바이트 단위 (메모리 낭비 감소를 위해 16바이트 정렬)
            /// </summary>
            static constexpr size_t default_segment_alignment = 16;

            /// <summary>
            /// 동접자 1024명이 사용할 수 있는 크기
            /// </summary>
            static constexpr size_t default_segment_size = static_cast<size_t>(session::packets_per_user) * 1024;
        };
    };

}