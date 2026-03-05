#include "pch.h"
#include "connection.h"
#include "system_config.h"

net::core::connection::connection
(
    boost::asio::io_context& context,
    common::ts_memory_pool& server_memory_pool,
    moodycamel::ConcurrentQueue<void*>& recv_queue,
    boost::asio::ip::tcp::socket&& client_socket,
    uint64_t session_id
) : context(context),
    server_memory_pool(server_memory_pool),
    recv_queue(recv_queue),
    socket(std::move(client_socket)),
    session_id(session_id)
{
}

net::core::connection::~connection()
{
    clear();
}

void net::core::connection::clear()
{
    if (socket.is_open())
    {
        socket.close();
    }
}

void net::core::connection::start()
{
    is_running = true;

    async_read_header();

    std::cout << "client access approved. session_id: " << session_id << std::endl;
}

void net::core::connection::stop()
{
    is_running = false;
}

void net::core::connection::async_read_header()
{
    auto self = shared_from_this();

    boost::asio::async_read(socket, boost::asio::buffer(&curr_packet.header, sizeof(packet::packet_header)),
    [this, self](boost::system::error_code error, size_t)
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

        // 현재 패킷의 페이로드 읽기
        async_read_payload();
    });
}

void net::core::connection::async_read_payload()
{
    auto self = shared_from_this();

    boost::asio::async_read(socket, boost::asio::buffer(&curr_packet.payload, curr_packet.header.payload_size),
    [this, self](boost::system::error_code error, size_t)
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

        // 메모리 풀에 패킷 저장
        void* recved_packet = server_memory_pool.acquire(&curr_packet, sizeof(packet::packet_header) + curr_packet.header.payload_size);

        // 서버에게 패킷 도착 콜
        recv_queue.enqueue(recved_packet);

        // 다음 패킷 읽기
        async_read_header();
    });
}