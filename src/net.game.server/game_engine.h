#pragma once
#include <vector>
#include <memory>
#include "game_object.h"

class game_engine
{
public:
    static game_engine& get_instance() 
    {
        static game_engine instance;
        return instance;
    }

    std::shared_ptr<game_object> instantiate();
    void destroy(std::shared_ptr<game_object> obj);

    void run_loop();

private:
    void process_network_events();
    void update(float dt);
    void fixed_update(float fixed_dt);

private:
    std::vector<std::shared_ptr<game_object>> game_objects;
    std::vector<std::shared_ptr<game_object>> pending_objects;
    bool is_running = false;
};