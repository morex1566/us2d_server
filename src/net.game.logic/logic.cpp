#include "pch.h"
#include "logic.h"
#include "packet.h"

namespace net::logic
{
    void on_move_character(packet::payload_sptr payload)
    {
        auto transform = std::static_pointer_cast<transformation>(payload);
        std::cout << "[Logic] Character Move: " << transform->x() << ", " << transform->y() << std::endl;
    }

    void on_chat(packet::payload_sptr payload)
    {

    }
}