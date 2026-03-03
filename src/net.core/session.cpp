#include "pch.h"
#include "session.h"

net::core::session::session
(
    boost::asio::io_context& context,
    std::shared_ptr<common::ts_pool<packet::packet>> server_packet_pool,
    common::ts_deque<packet::packet_request>& server_recv_buffer,
    boost::asio::ip::tcp::socket&& client_socket,
    uint64_t session_id
) : context(context),
    server_packet_pool(server_packet_pool),
    server_recv_buffer(server_recv_buffer),
    socket(std::move(client_socket)),
    session_id(session_id)
{
}

net::core::session::session(session&& other) noexcept
    : context(other.context),
      server_packet_pool(other.server_packet_pool),
      server_recv_buffer(other.server_recv_buffer),
      socket(std::move(other.socket)),
      session_id(other.session_id)
{
}

net::core::session::~session()
{
    clear();
}

void net::core::session::clear()
{
    if (socket.is_open())
    {
        socket.close();
    }
}

void net::core::session::start()
{
    is_running = true;

    async_read_header();

    std::cout << "client access approved. session_id: " << session_id << std::endl;
}

void net::core::session::stop()
{
    is_running = false;
}

void net::core::session::async_read_header()
{
    current_pkt = server_packet_pool->acquire();

    auto self = shared_from_this();
    boost::asio::async_read(socket, boost::asio::buffer(&current_pkt->header, sizeof(packet::packet_header)),
    [this, self](boost::system::error_code error, size_t /*length*/)
    {
        if (error)
        {
            std::cout << "header read error: " << error.message() << std::endl;
            return;
        }

        if (!is_running)
        {
            return;
        }

        async_read_payload();
    });
}

void net::core::session::async_read_payload()
{
    current_pkt->payload.resize(current_pkt->header.payload_size);

    auto self = shared_from_this();
    boost::asio::async_read(socket, boost::asio::buffer(current_pkt->payload.data(), current_pkt->header.payload_size),
    [this, self](boost::system::error_code error, size_t /*length*/)
    {
        if (error)
        {
            std::cout << "payload read error: " << error.message() << std::endl;
            return;
        }

        if (!is_running)
        {
            return;
        }

        auto request = packet::deserialize
        (
            current_pkt->header.type,
            session_id,
            current_pkt->payload,
            current_pkt->header.payload_size
        );
        server_recv_buffer.push_back(std::move(request));

        async_read_header();
    });
}

void net::core::session::send(packet::packet_type type, std::shared_ptr<google::protobuf::Message> payload)
{
    auto serialized = packet::serialize(type, *payload);

    auto buffer = std::make_shared<std::vector<uint8_t>>(std::move(serialized));

    auto self = shared_from_this();
    boost::asio::async_write(socket, boost::asio::buffer(*buffer),
    [self, buffer](boost::system::error_code error, size_t /*length*/)
    {
        if (error)
        {
            std::cout << "session::send() error: " << error.message() << std::endl;
        }
    });
}