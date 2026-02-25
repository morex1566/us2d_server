#pragma once
#include "def.h"
#include "packet.h"
#include "ts_deque.h"

namespace net::core
{
	class session : public std::enable_shared_from_this<session>
	{
	public:
		session(asio::io_context& context, common::ts_deque<packet::packet_request>& recv_buffer,
			asio_ip::tcp::socket&& client_socket, uint64_t id);

		session(session&& other) noexcept;
		
		session(const session&) = delete;

		session& operator=(const session&) = delete;

		~session();

		// 세션 종료
		void clear();

		void start();

		void update();

		void stop();

		// packet header 읽기
		void async_read();
	
	private:
		// packet payload 읽기
		void async_read_impl(packet::packet_id id, uint32_t size);

	private:
		// 세션 상태
		std::atomic<bool> m_is_running = false;

		// 서버 IOCP
		asio::io_context& m_context;

		// 클라이언트 socket lazy init을 위한 std::unique_ptr
		asio_ip::tcp::socket m_socket;

		// 클라이언트 고유 id
		uint64_t m_id;

		// 서버의 클라이언트 request 버퍼
		common::ts_deque<packet::packet_request>& m_recv_buffer;

		// 클라이언트 데이터 수신용 stream
		std::vector<uint8_t> m_packet_stream;
	};
}