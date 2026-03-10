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
    ts_memory_pool::ts_memory_pool()
        : capacity(system_config::ts_memory_pool::default_pool_capacity), front(0), back(0)
    {
        buffer = static_cast<uint8_t*>(_aligned_malloc(capacity, system_config::ts_memory_pool::pool_alignment));
    }

    ts_memory_pool::~ts_memory_pool()
    {
        if (buffer)
        {
            _aligned_free(buffer);
        }
    }

    void* ts_memory_pool::acquire(void* ptr, int size)
    {
        size_t alignment = system_config::ts_memory_pool::pool_alignment;
        size_t header_size = align_up(sizeof(block_header), alignment);
        size_t total_size = align_up(header_size + size, alignment);

        while (true)
        {
            uint64_t current_back = back.load(std::memory_order_relaxed);
            uint64_t current_front = front.load(std::memory_order_acquire);
            size_t pos = static_cast<size_t>(current_back % capacity);

            // 여유 공간 검사 (부족하면 힙 할당으로 fallback)
            if (current_back - current_front + total_size > capacity || pos + total_size > capacity)
            {
                return acquire_fallback(ptr, size, total_size, header_size);
            }

            // 실제 할당 시도
            if (back.compare_exchange_strong(current_back, current_back + total_size, std::memory_order_acq_rel))
            {
                return acquire_block(pos, total_size, ptr, size, header_size);
            }
        }
    }

    void ts_memory_pool::release(void* ptr)
    {
        // 1. 주소 유효성 검사
        if (!ptr)
        {
            return;
        }

        auto header_size = align_up(sizeof(block_header), system_config::ts_memory_pool::pool_alignment);
        auto header = reinterpret_cast<block_header*>(static_cast<uint8_t*>(ptr) - header_size);

        // 2. Fallback 할당 직접 처리
        if (header->is_fallback)
        {
            _aligned_free(header);
            return;
        }

        // 3. 링버퍼 블록 해제 마킹 및 Front 전진 시도
        header->is_released.store(true, std::memory_order_release);

        uint64_t current_front = front.load(std::memory_order_acquire);
        while (current_front < back.load(std::memory_order_acquire))
        {
            auto curr = reinterpret_cast<block_header*>(buffer + (current_front % capacity));

            if (!curr->is_released.load(std::memory_order_acquire))
            {
                break;
            }

            uint32_t block_size = curr->size;
            if (front.compare_exchange_strong(current_front, current_front + block_size))
            {
                current_front += block_size;
                continue;
            }

            break;
        }
    }

    void* ts_memory_pool::acquire_block(size_t pos, size_t total_size, void* src, int src_size, size_t header_size)
    {
        block_header* header = reinterpret_cast<block_header*>(buffer + pos);
        {
            header->size = static_cast<uint32_t>(total_size);
            header->is_fallback = false;
            header->is_released.store(false, std::memory_order_relaxed);
        }

        void* user_ptr = reinterpret_cast<uint8_t*>(header) + header_size;

        if (src)
        {
            memcpy(user_ptr, src, src_size);
        }

        return user_ptr;
    }

    void* ts_memory_pool::acquire_fallback(void* src, int src_size, size_t total_size, size_t header_size)
    {
        // 시스템 RAM 사용률 상한 체크
        if (100.0 - system_config::available_ram_percent() > system_config::ts_memory_pool::memory_alloc_limit_percent)
        {
            printf("[TsMemoryPool] Error: RAM limit reached. Fallback allocation failed.\n");
            return nullptr;
        }

        void* raw = _aligned_malloc(total_size, system_config::ts_memory_pool::pool_alignment);
        if (!raw)
        {
            return nullptr;
        }

        block_header* header = reinterpret_cast<block_header*>(raw);
        {
            header->size = static_cast<uint32_t>(total_size);
            header->is_fallback = true;
            header->is_released.store(false, std::memory_order_relaxed);
        }

        void* user_ptr = reinterpret_cast<uint8_t*>(header) + header_size;

        if (src)
        {
            memcpy(user_ptr, src, src_size);
        }

        return user_ptr;
    }
}