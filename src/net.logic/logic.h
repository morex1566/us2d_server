#pragma once
#include "packet.h"

namespace net::core
{
    class session;
    class tcp;
}

namespace net::logic
{
    void on_move_character
    (
        std::shared_ptr<net::core::session> session,
        net::core::tcp& server,
        std::shared_ptr<google::protobuf::Message> payload
    );

    void on_chat
    (
        std::shared_ptr<net::core::session> session,
        net::core::tcp& server,
        std::shared_ptr<google::protobuf::Message> payload
    );

    void on_connected
    (
        std::shared_ptr<net::core::session> session,
        net::core::tcp& server,
        std::shared_ptr<google::protobuf::Message> payload
    );

    inline std::unordered_map<net::packet::packet_type, void(*)(std::shared_ptr<net::core::session>, net::core::tcp&, std::shared_ptr<google::protobuf::Message>)> packet_handler_map
    {
        {net::packet::packet_type::transformation, &on_move_character},
        {net::packet::packet_type::chat, &on_chat}
    };
}
