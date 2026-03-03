#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>

namespace net::common
{
    template <typename t>
    class ts_deque
    {
    public:
        ts_deque() {}
        ts_deque(const ts_deque&) = delete;
        ts_deque& operator=(const ts_deque&) = delete;
        virtual ~ts_deque()
        {
            clear();
        }

        bool pop_front(t& out_item)
        {
            std::scoped_lock lock(mtx);
            if (deque.empty())
            {
                return false;
            }

            out_item = std::move(deque.front());
            deque.pop_front();
            return true;
        }

        void wait_and_pop(t& out_item)
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return !deque.empty(); });
            out_item = std::move(deque.front());
            deque.pop_front();
        }

        void push_back(t&& in_item)
        {
            {
                std::scoped_lock lock(mtx);
                deque.emplace_back(std::move(in_item));
            }
            cv.notify_one();
        }

        void push_back(const t& in_item)
        {
            {
                std::scoped_lock lock(mtx);
                deque.emplace_back(in_item);
            }
            cv.notify_one();
        }

        bool empty()
        {
            std::scoped_lock lock(mtx);
            return deque.empty();
        }

        size_t count()
        {
            std::scoped_lock lock(mtx);
            return deque.size();
        }

        void clear()
        {
            std::scoped_lock lock(mtx);
            deque.clear();
        }

    private:
        std::mutex mtx;

        std::deque<t> deque;

        std::condition_variable cv;
    };
}