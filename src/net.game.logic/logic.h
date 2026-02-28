#pragma once
#include "packet.h"

namespace net::logic
{
    // 게임 로직 핸들러 예시
    void on_move_character(packet::payload_sptr payload);

    void on_chat(packet::payload_sptr payload);

    inline std::unordered_map<packet::packet_id, void(*)(packet::payload_sptr)> packet_handler_map
    {
        // 여기에 게임 로직 추가 <---------------------------------------------------------
        {packet::packet_id::transformation, &on_move_character},
        {packet::packet_id::chat, &on_chat}
    };
}
