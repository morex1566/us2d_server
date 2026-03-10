#include "pch.h"
#include "logic.h"
#include "connection.h"
#include "tcp.h"

namespace net::logic
{
    void on_move_character(std::shared_ptr<net::core::session> session, net::core::tcp& server, std::shared_ptr<google::protobuf::Message> payload)
    {
        auto transform = std::static_pointer_cast<transformation>(payload);
        std::cout << "on_move_character: " << transform->x() << ", " << transform->y() << std::endl;
    }

    void on_chat(std::shared_ptr<net::core::session> session, net::core::tcp& server, std::shared_ptr<google::protobuf::Message> payload)
    {

    }

    void on_connected(std::shared_ptr<net::core::session> session, net::core::tcp& server, std::shared_ptr<google::protobuf::Message> payload)
    {

    }
}