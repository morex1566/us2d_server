#pragma once
#include "def.h"
#include <mutex>
#include <deque>

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

        void push_back(t&& in_item)
        {
            std::scoped_lock lock(mtx);
            deque.emplace_back(std::move(in_item));
        }

        void push_back(t& in_item)
        {
            std::scoped_lock lock(mtx);
            deque.emplace_back(std::move(in_item));
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

    protected:
        std::mutex mtx;
        std::deque<t> deque;
    };
}