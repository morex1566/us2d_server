#include "pch.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include "input.h"

net::core::tcp_server::tcp_server
(
	asio::io_context& context,
	asio_ip::tcp::socket&& socket,
	asio_ip::tcp::endpoint& endpoint,
	asio_ip::tcp::acceptor& acceptor
)
	: context(context),
	socket(std::move(socket)),
	endpoint(endpoint),
	acceptor(acceptor), 
	work_guard(asio::make_work_guard(context))
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

	is_running = true;

	// async_accept() 명령 예약
	async_accept();

	// IOCP 이벤트 루프 시작
	// G : m_context.run()에 의해 여기서 block되는걸 방지
	context_worker = std::thread([this]()
	{
		context.run();
	});
}

void net::core::tcp_server::update()
{
	// TODO : 테스트 코드
	{
		packet::packet_request request;

		if (recv_buffer.pop_front(request))
		{
			request.execute();
		}
	}
}

void net::core::tcp_server::stop()
{
	is_running = false;

	context.stop();

	if (context_worker.joinable())
	{
		context_worker.join();
	}
}

void net::core::tcp_server::clear()
{
	if (socket.is_open())
	{
		socket.close();
	}
}

void net::core::tcp_server::async_accept()
{
	acceptor.async_accept([this](boost::system::error_code error, asio_ip::tcp::socket client_socket)
	{
		// 연결 O
		if (!error)
		{
			auto client_session = std::make_shared<session>(context, recv_buffer, std::move(client_socket), session_id++);
			client_session->start();

			// session 생성
			sessions.push_back(client_session);
			

			std::cout << "client access approved." << std::endl;
		}
		// 연결 X
		else
		{
			std::cout << "client access denied." << std::endl;
		}

		// 서버가 중지 X? -> accept() 재귀
		if (is_running)
		{
			async_accept();
		}
	});
}