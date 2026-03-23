#pragma once
#include "system_config.h"
#include <boost/lockfree/stack.hpp>

namespace net::common
{
    class ts_memory_pool 
    {
    public:
        ts_memory_pool();

        ~ts_memory_pool();

        std::shared_ptr<uint8_t> rent(size_t size);

    private:
        // Internal methods to handle specific block types
        template<typename block_type>
        block_type* pop(boost::lockfree::stack<block_type*, boost::lockfree::fixed_sized<false>>& stack);

        template<typename block_type>
        bool push(block_type* ptr, boost::lockfree::stack<block_type*, boost::lockfree::fixed_sized<false>>& stack);

        // Pre-allocation helper
        template<typename block_type>
        void initialize_pool(boost::lockfree::stack<block_type*, boost::lockfree::fixed_sized<false>>& stack, size_t count);

        // Dynamic allocation check based on system_config RAM limits
        bool can_allocate_dynamic();

    private:
        boost::lockfree::stack<block64*, boost::lockfree::fixed_sized<false>> pool_64b
        { system_config::ts_memory_pool::pool_64b_count };

        boost::lockfree::stack<block128*, boost::lockfree::fixed_sized<false>> pool_128b
        { system_config::ts_memory_pool::pool_128b_count };

        boost::lockfree::stack<block256*, boost::lockfree::fixed_sized<false>> pool_256b
        { system_config::ts_memory_pool::pool_256b_count };

        boost::lockfree::stack<block512*, boost::lockfree::fixed_sized<false>> pool_512b
        { system_config::ts_memory_pool::pool_512b_count };

        boost::lockfree::stack<block1024*, boost::lockfree::fixed_sized<false>> pool_1024b
        { system_config::ts_memory_pool::pool_1024b_count };

        boost::lockfree::stack<block2048*, boost::lockfree::fixed_sized<false>> pool_2048b
        { system_config::ts_memory_pool::pool_2048b_count };

        boost::lockfree::stack<block4096*, boost::lockfree::fixed_sized<false>> pool_4096b
        { system_config::ts_memory_pool::pool_4096b_count };
    };

    // 1. 필요한 크기에 따른 풀 선택
    // 2. 스택에서 Pop 시도, 실패 시 동적으로 RAM 한도 내에서 할당
    // 3. 반환 시 해당 풀로 다시 반입되도록 하는 커스텀 Deleter 설정


    template<typename block_type>
    inline block_type* ts_memory_pool::pop(boost::lockfree::stack<block_type*, boost::lockfree::fixed_sized<false>>& stack)
    {
        block_type* ptr = nullptr;
        if (stack.pop(ptr)) return ptr;


        return nullptr;
    }

    template<typename block_type>
    inline bool ts_memory_pool::push(block_type* ptr, boost::lockfree::stack<block_type*, boost::lockfree::fixed_sized<false>>& stack)
    {
        return stack.push(ptr);
    }
}

