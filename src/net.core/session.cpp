#include "pch.h"
#include "session.h"
#include "logic.h"

net::core::session::session(asio::io_context& context, common::ts_deque<packet::packet_request>& recv_buffer,
	asio_ip::tcp::socket&& client_socket, uint64_t id)
	: m_context(context), m_recv_buffer(recv_buffer),
	m_socket(std::move(client_socket)), m_id(id)
{

}

net::core::session::session(session&& other) noexcept
	: m_context(other.m_context), m_recv_buffer(other.m_recv_buffer),
	m_socket(std::move(other.m_socket)), m_id(other.m_id)
{

}

net::core::session::~session()
{
	clear();
}

void net::core::session::clear()
{
	if (m_socket.is_open())
	{
		m_socket.close();
	}
}

void net::core::session::start()
{
	m_is_running = true;

	// async_recv() 명령 예약
	async_read();
}

void net::core::session::update()
{

}

void net::core::session::stop()
{
	// 더 이상 async_read() recursive 수행 X
	m_is_running = false;
}

void net::core::session::async_read()
{
	// 헤더 크기만큼 확보
	m_packet_stream.resize(sizeof(packet::packet_header));

	// 클라에서 보낸 데이터 읽기
	asio::async_read(m_socket, asio::buffer(m_packet_stream, sizeof(packet::packet_header)),
	[this](boost::system::error_code error, size_t length)
	{
		if (!error)
		{
			// stream에서 packet_header 파싱
			packet::packet_header* header = reinterpret_cast<packet::packet_header*>(m_packet_stream.data());
			packet::packet_id id = header->m_id;
			uint32_t size = header->m_size;

			// packet_header로 payload 읽기
			if (m_is_running)
			{
				async_read_impl(id, size);
			}
		}
		else
		{
			std::cout << error.what() << std::endl;
		}
	});
}

void net::core::session::async_read_impl(packet::packet_id id, uint32_t size)
{
	// payload 크기만큼 확보
	m_packet_stream.resize(sizeof(packet::packet_header));

	// 클라에서 보낸 데이터 읽기
	asio::async_read(m_socket, asio::buffer(m_packet_stream, size),
	[this, id, size](boost::system::error_code error, size_t)
	{
		if (!error)
		{
			auto packet = packet::packet_creator_map[id](m_packet_stream, size);
			auto handler = game_logic::packet_handler_map[id];
				
			// request 버퍼에 추가
			m_recv_buffer.push_back(packet::packet_request(id, packet, handler));

			// 다음 패킷 헤더 읽기
			if (m_is_running)
			{
				async_read();
			}
		}
		else
		{
			std::cout << error.what() << std::endl;
		}
	});
}