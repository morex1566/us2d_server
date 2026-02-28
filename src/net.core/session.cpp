#include "pch.h"
#include "session.h"

net::core::session::session(asio::io_context& context, common::ts_deque<packet::packet_request>& recv_buffer,
	asio_ip::tcp::socket&& client_socket, uint64_t id)
	: context(context), recv_buffer(recv_buffer),
	socket(std::move(client_socket)), id(id)
{

}

net::core::session::session(session&& other) noexcept
	: context(other.context), recv_buffer(other.recv_buffer),
	socket(std::move(other.socket)), id(other.id)
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

	// async_recv() 명령 예약
	async_read_header();
}

void net::core::session::stop()
{
	// 더 이상 async_read() recursive 수행 X
	is_running = false;
}

void net::core::session::async_read_header()
{
    // 새 패킷을 위한 공간 생성
    auto pkt = std::make_shared<packet::packet>();

    // 헤더 읽기 (pkt->header에 직접 기록)
    asio::async_read(socket, asio::buffer(&pkt->header, sizeof(packet::packet_header)),
    [this, pkt](boost::system::error_code error, size_t length)
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

        async_read_payload(pkt);
    });
}

void net::core::session::async_read_payload(std::shared_ptr<packet::packet> pkt)
{
    // 페이로드 크기만큼 버퍼 공간 확보
    uint32_t size = pkt->header.payload_size;
    pkt->payload.resize(size);

    // 페이로드 읽기
    asio::async_read(socket, asio::buffer(pkt->payload.data(), size),
    [this, pkt](boost::system::error_code error, size_t length)
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

        // 패킷 식별자 추출
        auto id = pkt->header.id;
        auto decoded_payload = packet::packet_serializer_map[id](pkt->payload, length);
        recv_buffer.push_back(packet::packet_request(id, decoded_payload));

        // 다음 패킷을 읽기 위해 순환
        async_read_header();
    });
}