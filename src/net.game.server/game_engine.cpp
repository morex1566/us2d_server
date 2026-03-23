#include "pch.h"
#include "game_engine.h"
#include <chrono>

std::shared_ptr<game_object> game_engine::instantiate()
{
    auto obj = std::make_shared<game_object>();
    pending_objects.push_back(obj);
    return obj;
}

void game_engine::destroy(std::shared_ptr<game_object> obj)
{
    // 나중에 삭제 로직 추가 (플래그 설정 등)
}

void game_engine::process_network_events()
{
    // TCP 서버 싱글톤 등을 통해 큐 접근 필요.
    // 임시 주석 처리. 실제 구현시 tcp::get_instance().sessions 등에서 큐를 가져와 처리해야함.
}

void game_engine::run_loop()
{
    is_running = true;
    auto last_time = std::chrono::high_resolution_clock::now();

    while (is_running)
    {
        auto current_time = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;

        process_network_events();
        fixed_update(0.016f);
        update(dt);
    }
}

void game_engine::update(float dt)
{
    for (auto& obj : game_objects)
    {
        if (!obj->active_self) continue;
        for (auto& comp : obj->get_components())
        {
            if (comp->enabled)
                comp->update(dt);
        }
    }
}

void game_engine::fixed_update(float fixed_dt)
{
    // 물리 시뮬레이션
}
