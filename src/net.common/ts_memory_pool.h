#pragma once
#include "pch.h"
#include "system_config.h"
#include "concurrentqueue/concurrentqueue.h"

namespace net::common
{
    class ts_memory_pool;

    /// <summary>
    /// 개별 할당 정보 헤더 (64바이트 정렬 고려)
    /// </summary>
    struct block_header
    {
        uint32_t size;              // 헤더 포함 전체 크기
        struct segment* seg;        // 소속 세그먼트
        std::atomic<bool> released; // 해제 여부 (out-of-order 대응)
    };

    /// <summary>
    /// 메모리 세그먼트: 내부적으로 링버퍼와 64비트 인덱스를 사용하여 ABA 문제 방지
    /// </summary>
    struct segment
    {
    public:
        uint8_t* buffer;

        size_t size;

        ts_memory_pool* pool;

        // 거짓 공유(False Sharing) 방지를 위해 64바이트 캐시라인 정렬
        alignas(64) std::atomic<uint64_t> front;

        alignas(64) std::atomic<uint64_t> back;

        segment(size_t s, ts_memory_pool* p)
            : size(s), pool(p), front(0), back(0)
        {
            buffer = static_cast<uint8_t*>(_aligned_malloc(size, system_config::ts_memory_pool::default_segment_alignment));
        }

        ~segment()
        {
            if (buffer) _aligned_free(buffer);
        }

        bool is_empty() const { return front.load() == back.load(); }
    };

    /// <summary>
    /// 세그먼트 큐 기반 스레드 안전 메모리 풀
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
        /// 사용자 데이터 해제 및 세그먼트 회수 시도
        /// </summary>
        void release(void* ptr);

        /// <summary>
        /// 세그먼트를 큐에 다시 넣음
        /// </summary>
        void re_pool(segment* seg);

    private:
        /// <summary>
        /// 새 세그먼트 생성 및 추가 (RAM 한계 체크 포함)
        /// </summary>
        segment* expand();

        bool try_initialize_segment();

        bool try_switch_segment(segment* old_seg);

        void* prepare_block(segment* seg, size_t pos, size_t total_size, void* src, int src_size, size_t header_size);

    private:
        // 현재 할당이 진행 중인 세그먼트
        std::atomic<segment*> active_segment;

        // 가용한 세그먼트 풀
        moodycamel::ConcurrentQueue<segment*> segments;
        
        // expand() 중복 호출 방지용
        std::mutex expand_mutex;
    };

    inline static size_t align_up(size_t size, size_t alignment);
}