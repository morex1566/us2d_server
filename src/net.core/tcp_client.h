#pragma once
#include "def.h"
#include "packet.h"

namespace net::core
{
	class tcp_client
	{
	public:

		tcp_client(const std::string& host, asio_ip::port_type port);

		tcp_client(const tcp_client&) = delete;

		tcp_client& operator=(const tcp_client&) = delete;

		tcp_client(tcp_client&&) = delete;

		tcp_client& operator=(tcp_client&&) = delete;

		~tcp_client();

		// async_connect() 성공 시 호출
		void on_connected();

		// async_connect() 실패 시 호출
		void on_disconnected();

		// 클라이언트 시작
		void start();

		// 클라이언트 종료
		void stop();

		// 소켓 close()
		void clear();

		// 클라이언트 생명주기
		bool is_active() const { return is_running; }

	private:

		void async_connect();

		void async_write_header(std::shared_ptr<packet::packet> pkt);

		void async_write_payload(std::shared_ptr<packet::packet> pkt);

	private:

		// 클라이언트 상태
		std::atomic<bool> is_running = false;

		// IOCP
		asio::io_context context;

		// IOCP 스레드
		std::thread context_worker;

		// IOCP 작업 없을 때 꺼지지 않도록
		asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard;

		// 클라이언트 side 소켓
		asio_ip::tcp::socket socket;
		
		// 서버 주소
		asio_ip::tcp::endpoint endpoint;
	};
}