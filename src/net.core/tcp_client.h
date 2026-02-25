#pragma once
#include "def.h"

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

		void update();

		// 클라이언트 종료
		void stop();

		// 소켓 close()
		void clear();

		// 클라이언트 생명주기
		bool is_running() const { return m_is_running; }

	private:
		void async_connect();

	private:
		// 클라이언트 상태
		std::atomic<bool> m_is_running = false;

		// IOCP
		asio::io_context m_context;

		// IOCP 스레드
		std::thread m_context_worker;

		// 클라이언트 side 소켓
		asio_ip::tcp::socket m_socket;
		
		// 서버 주소
		asio_ip::tcp::endpoint m_endpoint;
	};
}