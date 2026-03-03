#pragma once
#include <memory>
#include <mutex>
#include <vector>

namespace net::common
{
    // Thread-safe object pool.
    // - acquire() : 풀에서 객체를 꺼내 shared_ptr 로 반환 (자동 반환 deleter 포함)
    // - 풀 고갈 시 fallback heap 할당; release() 에서 범위 검사 후 delete
    // - T 에 clear() 멤버가 있으면 반환 시 자동 호출 (컴파일 타임 검사)
    template<typename t> 
    class ts_pool : public std::enable_shared_from_this<ts_pool<t>>
    {
    public:
        static constexpr uint32_t MAX_CAPACITY = 40000;

        // C9: 멤버 변수 capacity와 이름 충돌 방지 → init_capacity
        explicit ts_pool(size_t init_capacity = MAX_CAPACITY) : capacity(init_capacity), t_buffer(new t[init_capacity])
        {
            t_ptr_buffer.reserve(init_capacity);

            for (size_t i = 0; i < init_capacity; ++i)
            {
                // C8: payload.reserve는 t가 해당 멤버를 가질 때만 호출 (컴파일 타임 분기)
                if constexpr (requires { t_buffer[i].payload.reserve(0); })
                {
                    t_buffer[i].payload.reserve(512);
                }
                t_ptr_buffer.push_back(&t_buffer[i]);
            }
        }

        ts_pool(const ts_pool&) = delete;

        ts_pool& operator=(const ts_pool&) = delete;

        ~ts_pool()
        {
            delete[] t_buffer;
        }

        std::shared_ptr<t> acquire()
        {
            // pool check & draw
            t* raw = nullptr;
            {
                std::scoped_lock lk(mtx);
                if (!t_ptr_buffer.empty())
                {
                    raw = t_ptr_buffer.back();
                    t_ptr_buffer.pop_back();
                }
            }

            // t is empty → shared_ptr 할당 (deleter: instance delete)
            if (raw == nullptr)
            {
                return std::shared_ptr<t>(new t());
            }

            // t is exist → shared_ptr 대여 (deleter: pool에 shared_ptr 반환)
            // NOTICE : std::shared_ptr가 관리하는 객체를 일시적인 래퍼 카운트 없이 접근하는 포인터
            // deleter: weak_ptr로 ts_pool 인스턴스 확인 후 반환
            std::weak_ptr<ts_pool<t>> weak = this->shared_from_this();
            return std::shared_ptr<t>(raw, [weak](t* p)
            {
                if (auto pool = weak.lock())
                {
                    pool->release(p);
                }
            });
        }

        size_t available() const
        {
            std::scoped_lock lk(mtx);
            return t_ptr_buffer.size();
        }

        void release(t* ptr)
        {
            // 풀 소속이 아닌 fallback 할당분
            if (ptr < &t_buffer[0] || ptr >= &t_buffer[capacity])
            {
                delete ptr;
                return;
            }

            // T에 clear()가 있으면 초기화
            if constexpr (requires { ptr->clear(); })
            {
                ptr->clear();
            }

            std::scoped_lock lk(mtx);
            t_ptr_buffer.push_back(ptr);
        }

    private:
        const size_t capacity;

        // NOTICE : shared_ptr로 하면 파편화 발생하기에 raw pointer로 메모리 한번에 할당된 블록
        t* t_buffer;

        // NOTICE : const 데이터를 수정하기 위해서 사용
        mutable std::mutex mtx;

        // t_buffer의 객체를 가리키는 블록
        std::vector<t*> t_ptr_buffer;
    };
}