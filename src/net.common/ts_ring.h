#pragma once
#include "def.h"

namespace net::common
{
    template <typename t>
    class ts_ring
    {
    public:
        struct node
        {
            t data;
            std::atomic<bool> is_ready = false;

            node() : is_ready(false) {}
        };

    public:
        ts_ring(size_t size) : size(size), head(0), tail(0)
        {
            buffer.resize(size);
        }

        ts_ring(const ts_ring&) = delete;

        ts_ring operator=(const ts_ring&) = delete;

        ~ts_ring()
        {
            // std::vector가 자동 해제
        }

        bool push(const t& in)
        {
            size_t curr_head_idx = head.load(std::memory_order_relaxed);
            size_t next_head_idx;
            const auto capacity = size.load(std::memory_order_acquire);

            // 작업 공간 인덱스 획득 과정
            while (true)
            {
                next_head_idx = (curr_head_idx + 1) % capacity;

                // 실패 : 버퍼 꽉 참
                if (next_head_idx == tail.load(std::memory_order_acquire))
                {
                    return false;
                }

                // 성공 : current_head이 인덱스 get
                if (head.compare_exchange_weak(curr_head_idx, next_head_idx,
                    std::memory_order_release,
                    std::memory_order_relaxed))
                {
                    break;
                }
            }

            buffer[curr_head_idx].data = std::move(in);
            buffer[curr_head_idx].is_ready.store(true, std::memory_order_release);

            return true;
        }

        bool pop(t& out)
        {
            size_t curr_tail_idx = tail.load(std::memory_order_relaxed);
            size_t next_tail_idx;
            const auto capacity = size.load(std::memory_order_acquire);

            // 작업 공간 인덱스 획득 과정
            while (true)
            {
                // 실패 : 버퍼 비었음
                if (curr_tail_idx == head.load(std::memory_order_acquire))
                {
                    return false;
                }

                // 실패 : 데이터가 아직 안들어왔음
                if (buffer[curr_tail_idx].is_ready.load(std::memory_order_acquire) == false)
                {
                    return false;
                }

                next_tail_idx = (curr_tail_idx + 1) % capacity;

                // 성공 : current_tail이 인덱스 get
                if (tail.compare_exchange_weak(curr_tail_idx, next_tail_idx,
                    std::memory_order_release,
                    std::memory_order_relaxed))
                {
                    break;
                }
            }

            out = std::move(buffer[curr_tail_idx].data);
            buffer[curr_tail_idx].is_ready.store(false, std::memory_order_release);

            return true;
        }


    private:
        std::vector<node> buffer;
        std::atomic<size_t> size;
        std::atomic<size_t> head;
        std::atomic<size_t> tail;
    };
}