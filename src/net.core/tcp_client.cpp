#include "pch.h"
#include "tcp_client.h"
#include "input.h"

net::core::tcp_client::tcp_client(const std::string& host, asio_ip::port_type port)
	: socket(context), endpoint(asio::ip::make_address(host), port), work_guard(asio::make_work_guard(context))
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

	is_running = true;

	// socket.async_connect() 명령 예약
	async_connect();

	// IOCP 이벤트 루프 시작
	// G : m_context.run()에 의해 여기서 block되는걸 방지
	context_worker = std::thread([this]()
	{
		context.run();
	});
}

void net::core::tcp_client::stop()
{
	is_running = false;

	context.stop();

	if (context_worker.joinable())
	{
		context_worker.join();
	}
}

void net::core::tcp_client::clear()
{
	if (socket.is_open())
	{
		socket.close();
	}
}

void net::core::tcp_client::async_connect()
{
	socket.async_connect(endpoint, [this](const std::error_code& error)
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

void net::core::tcp_client::async_write_header(std::shared_ptr<packet::packet> pkt)
{
	asio::async_write(socket, asio::buffer(&pkt->header, sizeof(packet::packet::header)),
	[this, pkt](boost::system::error_code error, size_t length)
	{
		if(!error)
		{
			async_write_payload(pkt);

			std::cout << "write packet_header done." << std::endl;
		}
		else
		{
			std::cout << error.what() << std::endl;
		}
	});
}

void net::core::tcp_client::async_write_payload(std::shared_ptr<packet::packet> pkt)
{
	asio::async_write(socket, asio::buffer(pkt->payload.data(), pkt->header.payload_size),
	[this, pkt](boost::system::error_code error, size_t length)
	{
		if (!error)
		{
			std::cout << "write packet_payload done." << std::endl;
		}
		else
		{
			std::cout << error.what() << std::endl;
		}
	});
}