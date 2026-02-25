#include "pch.h"
#include "tcp_client.h"

net::core::tcp_client::tcp_client(const std::string& host, asio_ip::port_type port)
	: m_socket(m_context), m_endpoint(asio::ip::make_address(host), port)
{

}

net::core::tcp_client::~tcp_client()
{
	stop();
	clear();
}

void net::core::tcp_client::on_connected()
{
	std::cout << "client is connected." << std::endl;
}

void net::core::tcp_client::on_disconnected()
{

}

void net::core::tcp_client::start()
{
	std::cout << "tcp_client::start() called\n";  // 임시 로그

	m_is_running = true;

	// socket.async_connect() 명령 예약
	async_connect();

	// IOCP 이벤트 루프 시작
	// G : m_context.run()에 의해 여기서 block되는걸 방지
	m_context_worker = std::thread([this]()
	{
		m_context.run();
	});
}

void net::core::tcp_client::update()
{

}

void net::core::tcp_client::stop()
{
	m_is_running = false;

	m_context.stop();

	if (m_context_worker.joinable())
	{
		m_context_worker.join();
	}
}

void net::core::tcp_client::clear()
{
	if (m_socket.is_open())
	{
		m_socket.close();
	}
}

void net::core::tcp_client::async_connect()
{
	m_socket.async_connect(m_endpoint, [this](const std::error_code& error)
	{
		// 연결 성공
		if (!error)
		{
			on_connected();
		}
		// 연결 실패
		else
		{
			
		}
	});
}
