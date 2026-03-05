#include "pch.h"
#include "ts_memory_pool.h"
#ifdef _WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#define _aligned_malloc(size, alignment) aligned_alloc(alignment, size)
#define _aligned_free(ptr) free(ptr)
#endif

namespace net::common
{
    // 정렬 도우미 (Internal)
    static inline size_t align_up(size_t size, size_t alignment) {
        return (size + alignment - 1) & ~(alignment - 1);
    }

    ts_memory_pool::ts_memory_pool()
        : active_segment(nullptr)
    {
        // 초기 세그먼트 생성
        active_segment.store(expand());
    }

    ts_memory_pool::~ts_memory_pool()
    {
        // 모든 세그먼트 자원 해제
        segment* seg = active_segment.load();
        if (seg) delete seg;

        while (segments.try_dequeue(seg))
        {
            delete seg;
        }
    }

    void* ts_memory_pool::acquire(void* ptr, int size)
    {
        size_t alignment = system_config::ts_memory_pool::default_segment_alignment;
        size_t header_size = align_up(sizeof(block_header), alignment);
        size_t total_size = align_up(header_size + size, alignment);

        while (true)
        {
            // 1. 세그먼트 확보
            segment* seg = active_segment.load(std::memory_order_acquire);
            if (seg == nullptr)
            {
                if (try_initialize_segment() == false)
                {
                    printf("[TsMemoryPool] Error: Failed to initialize first segment.\n");
                    return nullptr;
                }

                continue;
            }

            uint64_t current_back = seg->back.load(std::memory_order_relaxed);
            uint64_t current_front = seg->front.load(std::memory_order_acquire);
            size_t pos = static_cast<size_t>(current_back % seg->size);

            // 2. 남은 세그먼트 용량보다 데이터가 너무 커서... 교체 시도
            if (pos + total_size > seg->size || current_back + total_size > current_front + seg->size)
            {
                if (try_switch_segment(seg) == false)
                {
                    printf("[TsMemoryPool] Error: RAM limit reached. Allocation failed (size: %d).\n", size);
                    return nullptr;
                }

                continue;
            }

            // 3. 세그먼트에 데이터 저장 시도
            if (seg->back.compare_exchange_strong(current_back, current_back + total_size, std::memory_order_acq_rel))
            {
                return prepare_block(seg, pos, total_size, ptr, size, header_size);
            }
        }

        // 4. 데이터 저장 실패, critical error
        return nullptr;
    }

    void ts_memory_pool::release(void* ptr)
    {
        if (!ptr) return;

        size_t alignment = system_config::ts_memory_pool::default_segment_alignment;
        size_t header_size = align_up(sizeof(block_header), alignment);
        block_header* header = reinterpret_cast<block_header*>(reinterpret_cast<uint8_t*>(ptr) - header_size);

        segment* seg = header->seg;
        // 이제 모든 할당은 세그먼트 내에서 이루어지므로 seg가 nullptr일 수 없음
        assert(seg != nullptr && "Invalid segment pointer in block header");

        // 해제 마킹 (out-of-order release 가능)
        header->released.store(true, std::memory_order_release);

        // Front 전진 시도 (가능한 만큼 전진시켜 공간 확보)
        uint64_t current_front = seg->front.load(std::memory_order_acquire);
        while (current_front < seg->back.load(std::memory_order_acquire))
        {
            size_t pos = static_cast<size_t>(current_front % seg->size);
            block_header* front_h = reinterpret_cast<block_header*>(seg->buffer + pos);

            if (!front_h->released.load(std::memory_order_acquire))
                break;

            uint32_t block_size = front_h->size;
            if (seg->front.compare_exchange_strong(current_front, current_front + block_size, std::memory_order_acq_rel))
            {
                current_front += block_size;
                
                // 세그먼트가 완전히 비었는지 확인
                if (current_front == seg->back.load(std::memory_order_acquire))
                {
                    // 비었으며 활성 상태가 아니라면 큐로 회수
                    if (seg != active_segment.load(std::memory_order_relaxed))
                    {
                        re_pool(seg);
                    }
                    break;
                }
            }
            else
            {
                // 경쟁 시 갱신된 current_front로 다시 시도
                continue;
            }
        }
    }

    void ts_memory_pool::re_pool(segment* seg)
    {
        // 인덱스를 초기화하여 재사용 가능 상태로 만듦
        seg->front.store(0, std::memory_order_relaxed);
        seg->back.store(0, std::memory_order_relaxed);
        segments.enqueue(seg);
    }

    bool ts_memory_pool::try_initialize_segment()
    {
        segment* new_seg = expand();
        if (new_seg == nullptr) return false;

        segment* expected = nullptr;
        if (active_segment.compare_exchange_strong(expected, new_seg) == false)
        {
            // 다른 스레드가 먼저 초기화했다면, 버리지 않고 큐에 보관하여 활용
            segments.enqueue(new_seg);
        }
        return true;
    }

    bool ts_memory_pool::try_switch_segment(segment* old_seg)
    {
        segment* next_seg = nullptr;
        if (segments.try_dequeue(next_seg) == false)
        {
            next_seg = expand();
        }

        if (next_seg == nullptr) return false;

        if (active_segment.compare_exchange_strong(old_seg, next_seg))
        {
            return true;
        }

        // 다른 스레드가 이미 교체했다면 큐에 보관
        segments.enqueue(next_seg);
        return true;
    }

    void* ts_memory_pool::prepare_block(segment* seg, size_t pos, size_t total_size, void* src, int src_size, size_t header_size)
    {
        block_header* header = reinterpret_cast<block_header*>(seg->buffer + pos);
        header->size = static_cast<uint32_t>(total_size);
        header->seg = seg;
        header->released.store(false, std::memory_order_relaxed);

        void* user_ptr = reinterpret_cast<uint8_t*>(header) + header_size;
        if (src) memcpy(user_ptr, src, src_size);

        return user_ptr;
    }

    segment* ts_memory_pool::expand()
    {
        std::lock_guard<std::mutex> lock(expand_mutex);

        // 시스템 설정에 따른 RAM 가용성 체크
        double available = system_config::available_ram_percent();
        double limit = system_config::ts_memory_pool::memory_alloc_limit_percent;

        if (100.0 - available > limit)
        {
            // 사용률이 상한을 넘은 경우 확장 거부
            return nullptr;
        }

        segment* new_seg = new segment(system_config::ts_memory_pool::default_segment_size, this);
        return new_seg;
    }
}