#pragma once
#include <atomic>
#include <vector>

namespace net::common
{
    /// <summary>
    /// lock-free MPMC 링버퍼. push/pop 모두 CAS 기반으로 thread-safe
    /// </summary>
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
        ts_ring(size_t size) : size(size), front(0), back(0)
        {
            t_buffer.resize(size);
        }

        ts_ring(const ts_ring&) = delete;

        ts_ring operator=(const ts_ring&) = delete;

        ~ts_ring()
        {
            // std::vector가 자동 해제
        }

        /// <summary>
        /// 링버퍼에 데이터를 추가. 버퍼가 꽉 찬 경우 false 반환
        /// </summary>
        bool push(const t& in)
        {
            // 작업 공간 인덱스 획득 과정
            while (true)
            {
                size_t curr_front_idx = front.load(std::memory_order_relaxed);
                size_t next_front_idx = (curr_front_idx + 1) % capacity;
                const auto capacity = size.load(std::memory_order_acquire);

                // 실패 : 버퍼 꽉 참
                if (next_front_idx == back.load(std::memory_order_acquire))
                {
                    return false;
                }

                // 성공 : CAS 전 write 인덱스를 별도 보관 (CAS 성공 시 expected가 변경됨)
                size_t write_idx = curr_front_idx;
                if (front.compare_exchange_weak(curr_front_idx, next_front_idx,
                    std::memory_order_release,
                    std::memory_order_relaxed))
                {
                    t_buffer[write_idx].data = std::move(in);
                    t_buffer[write_idx].is_ready.store(true, std::memory_order_release);
                    return true;
                }
            }
        }

        /// <summary>
        /// 링버퍼에서 데이터를 꺼냄. 버퍼가 비어있거나 데이터가 준비 안된 경우 false 반환
        /// </summary>
        bool pop(t& out)
        {
            // 작업 공간 인덱스 획득 과정
            while (true)
            {
                size_t curr_back_idx = back.load(std::memory_order_relaxed);
                size_t next_back_idx;
                const auto capacity = size.load(std::memory_order_acquire);

                // 실패 : 버퍼 비었음
                if (curr_back_idx == front.load(std::memory_order_acquire))
                {
                    return false;
                }

                // 실패 : 데이터가 아직 안들어왔음
                if (t_buffer[curr_back_idx].is_ready.load(std::memory_order_acquire) == false)
                {
                    return false;
                }

                next_back_idx = (curr_back_idx + 1) % capacity;

                // 성공 : CAS 전 read 인덱스를 별도 보관 (CAS 성공 시 expected가 변경됨)
                size_t read_idx = curr_back_idx;
                if (back.compare_exchange_weak(curr_back_idx, next_back_idx,
                    std::memory_order_release,
                    std::memory_order_relaxed))
                {
                    out = std::move(t_buffer[read_idx].data);
                    t_buffer[read_idx].is_ready.store(false, std::memory_order_release);
                    return true;
                }
            }
        }

        /// <summary>
        /// 링버퍼 최대 크기 반환
        /// </summary>
        inline size_t capacity() const { return size.load(std::memory_order_relaxed); }

    private:
        std::vector<node> t_buffer;
        std::atomic<size_t> size;
        std::atomic<size_t> front;
        std::atomic<size_t> back;
    };
}