#include "pch.h"
#include "tcp.h"

net::core::tcp::tcp(boost::asio::ip::port_type port)
	: mode(mode::server),
	  socket(context),
	  endpoint(boost::asio::ip::tcp::v4(), port),
	  work_guard(boost::asio::make_work_guard(context))
{
	acceptor.emplace(context, endpoint);
}

net::core::tcp::tcp(const std::string& host, boost::asio::ip::port_type port)
	: mode(mode::client),
	  socket(context),
	  endpoint(boost::asio::ip::make_address(host), port),
	  work_guard(boost::asio::make_work_guard(context))
{
}

net::core::tcp::~tcp()
{
	stop();
	clear();
}

void net::core::tcp::start()
{
	is_running = true;

	if (mode == mode::server)
	{
		std::cout << "tcp server::start() - port: " << endpoint.port() << "\n";
		async_accept();
	}
	else
	{
		std::cout << "tcp client::start() - connecting to: " << endpoint.address().to_string() << ":" << endpoint.port() << "\n";
		async_connect();
	}

	// IOCP 이벤트 루프 별도 스레드에서 실행
	context_worker = std::thread([this]()
	{
		context.run();
	});
}

void net::core::tcp::stop()
{
	is_running = false;

	context.stop();

	if (context_worker.joinable())
	{
		context_worker.join();
	}
}

void net::core::tcp::clear()
{
	if (socket.is_open())
	{
		socket.close();
	}

	if (acceptor.has_value() && acceptor->is_open())
	{
		acceptor->close();
	}
}

void net::core::tcp::async_accept()
{
	if (mode == tcp::mode::client)
	{
		std::cout << "tcp mode is client." << std::endl;
		return;
	}

	acceptor->async_accept([this](boost::system::error_code error, boost::asio::ip::tcp::socket client_socket)
	{
		if (!error)
		{
			// 세션 추가
			const uint64_t new_session_id = session_id_counter++;
			auto new_session = std::make_shared<connection>
			(
				context,
				memory_pool,
				recv_queue,
				std::move(client_socket),
				new_session_id
			);
			new_session->start();
			sessions.insert(new_session_id, new_session);

			std::cout << "tcp::async_accept() - new session id: " << new_session_id << std::endl;
		}
		else
		{
			std::cout << "tcp::async_accept() error: " << error.message() << std::endl;
		}

		if (is_running)
		{
			async_accept();
		}
	});
}

void net::core::tcp::async_connect()
{
	if (mode == tcp::mode::server)
	{
		std::cout << "tcp mode is server." << std::endl;
		return;
	}

	socket.async_connect(endpoint, [this](const boost::system::error_code& error)
	{
		if (!error)
		{
			// 세션 추가
			const uint64_t new_session_id = session_id_counter.fetch_add(1, std::memory_order::memory_order_release);
			auto new_session = std::make_shared<connection>
			(
				context,
				memory_pool,
				recv_queue,
				std::move(socket),
				new_session_id
			);
			new_session->start();
			sessions.insert(new_session_id, new_session);

			std::cout << "tcp client is connected." << std::endl;
		}
		else
		{
			std::cout << "tcp::async_connect() error: " << error.message() << std::endl;
		}
	});
}