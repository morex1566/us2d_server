#include "pch.h"
#include "tcp_server.h"

net::core::tcp_server::tcp_server
(
	asio::io_context& context,
	asio_ip::tcp::socket&& socket,
	asio_ip::tcp::endpoint& endpoint,
	asio_ip::tcp::acceptor& acceptor
)
	: m_context(context),
	m_socket(std::move(socket)),
	m_endpoint(endpoint),
	m_acceptor(acceptor)
{

}

net::core::tcp_server::~tcp_server()
{
	stop();
	clear();
}

void net::core::tcp_server::on_connected(const SOCKET client_socket)
{
}

void net::core::tcp_server::on_disconnected(const SOCKET client_socket)
{
}

void net::core::tcp_server::start()
{
	std::cout << "tcp_server::start() called\n";  // 임시 로그

	m_is_running = true;

	// async_accept() 명령 예약
	async_accept();

	// IOCP 이벤트 루프 시작
	// G : m_context.run()에 의해 여기서 block되는걸 방지
	m_context_worker = std::thread([this]()
	{
		m_context.run();
	});
}

void net::core::tcp_server::update()
{

}

void net::core::tcp_server::stop()
{
	m_is_running = false;

	m_context.stop();

	if (m_context_worker.joinable())
	{
		m_context_worker.join();
	}
}

void net::core::tcp_server::clear()
{
	if (m_socket.is_open())
	{
		m_socket.close();
	}
}

void net::core::tcp_server::async_accept()
{
	m_acceptor.async_accept([this](boost::system::error_code error, asio_ip::tcp::socket client_socket)
	{
		// 연결 O
		if (!error)
		{
			auto client_session = std::make_shared<session>(m_context, m_recv_buffer, std::move(client_socket), m_session_id++);
			client_session->start();

			// session 생성
			m_sessions.push_back(client_session);
			

			std::cout << "client access approved." << std::endl;
		}
		// 연결 X
		else
		{
			std::cout << "client access denied." << std::endl;
		}

		// 서버가 중지 X? -> accept() 재귀
		if (m_is_running)
		{
			async_accept();
		}
	});
}

void net::core::tcp_server::async_write_header()
{
	
}

void net::core::tcp_server::async_write_body()
{
}
