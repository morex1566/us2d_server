#include "pch.h"
#include "game_engine.h"

std::shared_ptr<game_object> game_engine::instantiate()
{
    auto obj = std::make_shared<game_object>();
    pending_objects.push_back(obj);
    return obj;
}

void game_engine::process_network_events()
{
    // TCP 서버 싱글톤 등을 통해 큐 접근 필요.
    // 임시 주석 처리. 실제 구현시 tcp::get_instance().sessions 등에서 큐를 가져와 처리해야함.
}

void game_engine::init()
{
    is_running = true;
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