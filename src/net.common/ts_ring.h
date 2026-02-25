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
            t m_data;
            std::atomic<bool> m_is_ready = false;

            node() : m_is_ready(false) {}
        };

    public:
        ts_ring(size_t size) : m_size(size), m_head(0), m_tail(0)
        {
            m_buffer = new node[m_size];
        }

        ts_ring(const ts_ring&) = delete;

        ts_ring operator=(const ts_ring&) = delete;

        ~ts_ring()
        {
            delete[] m_buffer;
        }

        bool push(const t& in)
        {
            size_t curr_head_idx = m_head.load(std::memory_order_relaxed);
            size_t next_head_idx;

            // 작업 공간 인덱스 획득 과정
            while (true)
            {
                next_head_idx = (curr_head_idx + 1) % m_size;

                // 실패 : 버퍼 꽉 참
                if (next_head_idx == m_tail.load(std::memory_order_acquire))
                {
                    return false;
                }

                // 성공 : current_head이 인덱스 get
                if (m_head.compare_exchange_weak(curr_head_idx, next_head_idx,
                    std::memory_order_release,
                    std::memory_order_relaxed))
                {
                    break;
                }
            }

            m_buffer[curr_head_idx].m_data = std::move(in);
            m_buffer[curr_head_idx].m_is_ready.store(true, std::memory_order_release);

            return true;
        }

        bool pop(t& out)
        {
            size_t curr_tail_idx = m_tail.load(std::memory_order_relaxed);
            size_t next_tail_idx;

            // 작업 공간 인덱스 획득 과정
            while (true)
            {
                // 실패 : 버퍼 비었음
                if (curr_tail_idx == m_head.load(std::memory_order_acquire))
                {
                    return false;
                }

                // 실패 : 데이터가 아직 안들어왔음
                if (m_buffer[curr_tail_idx].m_is_ready.load(std::memory_order_acquire) == false)
                {
                    return false;
                }

                next_tail_idx = (curr_tail_idx + 1) % m_size;

                // 성공 : current_tail이 인덱스 get
                if (m_tail.compare_exchange_weak(curr_tail_idx, next_tail_idx,
                    std::memory_order_release,
                    std::memory_order_relaxed))
                {
                    break;
                }
            }

            out = std::move(m_buffer[curr_tail_idx].m_data);
            m_buffer[curr_tail_idx].m_is_ready.store(false, std::memory_order_release);

            return true;
        }


    private:
        std::vector<node> m_buffer;
        std::atomic<size_t> m_size;
        std::atomic<size_t> m_head;
        std::atomic<size_t> m_tail;
    };
}