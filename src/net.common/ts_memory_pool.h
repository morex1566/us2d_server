#pragma once
#include "pch.h"
#include "system_config.h"
#include "concurrentqueue/concurrentqueue.h"

namespace net::common
{
    /// <summary>
    /// 메모리 할당 받을 때 같이 붙는 메타 데이터
    /// </summary>
    struct block_header
    {
    public:

        /// <summary>
        /// 헤더 포함 전체 크기
        /// </summary>
        uint32_t size;

        /// <summary>
        /// 힙 할당 대응
        /// </summary>
        bool is_fallback;

        /// <summary>
        /// 비순차적 해제 대응
        /// </summary>
        std::atomic<bool> is_released;

    };

    /// <summary>
    /// 단일 선형 링버퍼 기반 스레드 안전 메모리 풀
    /// </summary>
    class ts_memory_pool
    {
    public:

        ts_memory_pool();

        ~ts_memory_pool();

        ts_memory_pool(const ts_memory_pool&) = delete;

        ts_memory_pool& operator=(const ts_memory_pool&) = delete;

    public:

        /// <summary>
        /// 사용자 데이터를 풀에서 할당받아 복사하고 주소 반환
        /// </summary>
        void* acquire(void* ptr, int size);

        /// <summary>
        /// 사용자 데이터 해제
        /// </summary>
        void release(void* ptr);

    private:

        /// <summary>
        /// 링버퍼 블록 할당 준비 및 데이터 복사
        /// </summary>
        void* acquire_block(size_t pos, size_t total_size, void* src, int src_size, size_t header_size);

        /// <summary>
        /// 링버퍼가 가득 찼을 경우 시스템 RAM 체크 후 힙 할당
        /// </summary>
        void* acquire_fallback(void* src, int src_size, size_t total_size, size_t header_size);

    private:

        /// <summary>
        /// 실제 메모리 버퍼 시작 주소
        /// </summary>
        uint8_t* buffer;

        /// <summary>
        /// 버퍼 전체 용량
        /// </summary>
        size_t capacity;

        /// <summary>
        /// False Sharing 방지 적용된 할당 해제 인덱스 (Front)
        /// </summary>
        alignas(64) std::atomic<uint64_t> front;

        /// <summary>
        /// False Sharing 방지 적용된 할당 인덱스 (Back)
        /// </summary>
        alignas(64) std::atomic<uint64_t> back;

    };

    /// <summary>
    /// 정렬 단위에 맞춰 크기 올림 처리
    /// </summary>
    inline static size_t align_up(size_t size, size_t alignment)
    {
        return (size + alignment - 1) & ~(alignment - 1);
    }
}
