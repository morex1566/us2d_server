#pragma once
#include "def.h"
#include "packet.h"
#include "character.h"

namespace net::game_logic
{
    // 현재 사용되는 패킷ID 기반 패킷작업 목록
    inline static std::unordered_map<
        net::packet::packet_id,
        std::function<void(std::shared_ptr<google::protobuf::Message>)>> packet_handler_map
    {
        { net::packet::packet_id::transformation, &net::game_logic::on_character_move }
    };
}