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
		bool is_active() const { return is_running; }

	private:

		void async_accept();

	private:

		// 서버 상태
		std::atomic<bool> is_running = false;

		// IOCP
		asio::io_context& context;

		// IOCP 스레드
		std::thread context_worker;

		// IOCP 작업 없을 때 꺼지지 않도록
		asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard;

		// 연결할 서버 엔드포인트
		asio_ip::tcp::endpoint endpoint;

		// server side 소켓
		asio_ip::tcp::socket socket;

		// bind() + listen() + accept()
		asio_ip::tcp::acceptor& acceptor;

		// 클라이언트 uid
		std::atomic<uint64_t> session_id = 10000;

		// 클라 <-> 서버 연결 인터페이스
		common::ts_deque<std::shared_ptr<session>> sessions;

		// 서버 -> 클라 데이터 버퍼
		common::ts_deque<packet::packet_request> send_buffer;

		// 클라 -> 서버 데이터 버퍼
		common::ts_deque<packet::packet_request> recv_buffer;
	};
}