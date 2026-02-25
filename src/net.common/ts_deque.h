#pragma once
#include "def.h"

namespace net::common
{
	template <typename t>
	class ts_deque
	{
	public:
		ts_deque() {}

		ts_deque(const ts_deque&) = delete;

		ts_deque& operator=(const ts_deque&) = delete;

		~ts_deque() {}

		// deque의 first 요소 참조
		const t& front()
		{
			std::scoped_lock lock(m_mtx);
			return m_deque.front();
		}

		// deque의 last 요소 참조
		const t& back()
		{
			std::scoped_lock lock(m_mtx);
			return m_deque.back();
		}

		// deque의 first 요소 추출
		t pop_front()
		{
			std::scoped_lock lock(m_mtx);
			auto t = std::move(m_deque.front());
			m_deque.pop_front();
			return t;
		}

		// deque의 마지막에 요소 삽입
		void push_back(const t& item)
		{
			std::scoped_lock lock(m_mtx);
			m_deque.emplace_back(std::move(item));
		}

		bool empty()
		{
			std::scoped_lock lock(m_mtx);
			return m_deque.empty();
		}

		size_t count()
		{
			std::scoped_lock lock(m_mtx);
			return m_deque.size();
		}

		void clear()
		{
			std::scoped_lock lock(m_mtx);
			m_deque.clear();
		}

	protected:
		std::mutex m_mtx;
		std::deque<t> m_deque;
	};
}