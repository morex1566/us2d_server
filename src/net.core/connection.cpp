#include "pch.h"
#include "connection.h"
#include "system_config.h"
#include "packet.h"
#include "logger.h"
#include "packet_generated.h"

net::core::connection::connection
(
    boost::asio::io_context& context,
    boost::asio::ip::tcp::socket&& client_socket,
    uint64_t session_id,
    net::common::ts_memory_pool memory_pool
) : context(context),
    strand(context.get_executor()),
    socket(std::move(client_socket)),
    session_id(session_id),
    memory_pool(memory_pool)
{

}

net::core::connection::~connection()
{
    clear();
}

void net::core::connection::clear()
{
    auto self = shared_from_this();

    boost::asio::post(strand, [this, self]
    {
        if (socket.is_open())
        {
            socket.close();
        }
    });
}

void net::core::connection::start()
{
    is_running = true;

    async_read_header();

    SPDLOG_INFO("client access approved. session_id : {}", session_id);
}

void net::core::connection::stop()
{
    auto self = shared_from_this();

    boost::asio::post(strand, [this, self] 
    {
        is_running = false;
    });
}

void net::core::connection::async_read_header()
{
    auto self = shared_from_this();

    boost::asio::async_read(socket, boost::asio::buffer(&header_read_buffer, sizeof(net::protocol::packet_header)), 
        boost::asio::bind_executor(strand, [this, self](boost::system::error_code error, size_t len)
        {
            CHECK_RETURN_VOID(error, error.what());

            const auto* header = reinterpret_cast<const net::protocol::packet_header*>(header_read_buffer);
            uint16_t payload_size = header->payload_size();
 
            // 패킷 로드
            auto packet_owner = memory_pool.rent(sizeof(net::protocol::packet_header) + payload_size);
            if (!packet_owner)
            {
                // TODO : 패킷 읽기 실패에 대한 로직
                return;
            }

            // 헤더 복사
            memcpy(packet_owner.get(), header_read_buffer, sizeof(net::protocol::packet_header));

            // 페이로드 읽기로 전환
            async_read_payload(packet_owner, payload_size);
        }));
}

void net::core::connection::async_read_payload(std::shared_ptr<uint8_t> packet_owner, uint16_t payload_size)
{
    auto self = shared_from_this();

    // 헤더 이후 위치부터 페이로드 수신
    boost::asio::async_read(socket, boost::asio::buffer(packet_owner.get() + sizeof(net::protocol::packet_header), payload_size),
        boost::asio::bind_executor(strand, [this, self, packet_owner, payload_size](boost::system::error_code error, size_t len)
        {
            CHECK_RETURN_VOID(error, error.what());

            const auto* header = reinterpret_cast<const net::protocol::packet_header*>(packet_owner.get());

            // 리퀘스트 등록
            net::packet::packet_request request{};
            request.packet_id = header->packet_id();
            request.payload_owner = packet_owner;
            request.payload_size = payload_size;

            this->requests.push(request);

            async_read_header();
        }));
}