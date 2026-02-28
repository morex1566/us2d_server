#pragma once
#include "def.h"
#include "packet.h"
#include "character.h"

namespace net::packet
{
    // 현재 사용되는 패킷ID 기반 패킷작업 목록
    inline static std::unordered_map<net::packet::packet_id, payload_handle> packet_handler_map
    {
        { net::packet::packet_id::transformation, &net::packet::on_character_move }
    };
}
