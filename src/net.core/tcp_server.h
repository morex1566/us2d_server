#pragma once
#include "def.h"
#include "session.h"
#include "ts_deque.h"
#include "packet.h"

namespace net::core
{
	class tcp_server
	{
	public:
		tcp_server
		(
			asio::io_context& context,
			asio_ip::tcp::socket&& socket,
			asio_ip::tcp::endpoint& endpoint,
			asio_ip::tcp::acceptor& acceptor
		);

		tcp_server(const tcp_server&) = delete;

		tcp_server& operator=(const tcp_server&) = delete;

		~tcp_server();

		// 클라이언트가 연결되면 호출됨
		void on_connected(const SOCKET client_socket);

		// 클라이언트가 끊기면 호출됨
		void on_disconnected(const SOCKET client_socket);

		// 클라이언트 accept() 시작
		void start();

		void update();

		// 클라이언트 accept() 종료
		void stop();

		// 모든 클라이언트 close() + IOCP stop()
		void clear();

		// 서버가 클라이언트를 받고 있는지?
		bool is_running() const { return m_is_running; }


	private:
		void async_accept();

		void async_write_header();

		void async_write_body();


	private:
		// 서버 상태
		std::atomic<bool> m_is_running = false;

		// IOCP
		asio::io_context& m_context;

		// IOCP 스레드
		std::thread m_context_worker;

		// 연결할 서버 엔드포인트
		asio_ip::tcp::endpoint m_endpoint;

		// server side 소켓
		asio_ip::tcp::socket m_socket;

		// bind() + listen() + accept()
		asio_ip::tcp::acceptor& m_acceptor;

		// 클라이언트 uid
		std::atomic<uint64_t> m_session_id = 10000;

		// 클라 <-> 서버 연결 인터페이스
		common::ts_deque<std::shared_ptr<session>> m_sessions;

		// 서버 -> 클라 데이터 버퍼
		common::ts_deque<packet::packet_request> m_send_buffer;

		// 클라 -> 서버 데이터 버퍼
		common::ts_deque<packet::packet_request> m_recv_buffer;
	};
}