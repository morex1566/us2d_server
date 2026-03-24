#include "pch.h"
#include "tcp.h"
#include "net.common/log.h"

net::core::tcp::tcp()
	:	singleton(),
		socket(context),
		work_guard(boost::asio::make_work_guard(context))
{
	SPDLOG_INFO("create {} instance.", typeid(net::core::tcp).name());
}

net::core::tcp::~tcp()
{
	stop();
	clear();
}

void net::core::tcp::init(boost::asio::ip::port_type port)
{
	mode = mode::SERVER;
	endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port);
	acceptor.emplace(context, endpoint);
}

void net::core::tcp::init(const std::string& host, boost::asio::ip::port_type port)
{
	mode = mode::CLIENT;
	endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(host), port);
}

void net::core::tcp::start()
{
	is_running = true;

	if (mode == mode::SERVER)
	{
		async_accept();
		SPDLOG_INFO("start accept client.");
	}
	else
	{
		async_connect();
		SPDLOG_INFO("start connect server.");
	}

	// iocp 시작
	const int thread_count = std::thread::hardware_concurrency() - 1;
	for (int i = 0; i < thread_count; i++)
	{
		context_workers.emplace_back(
			[this]()
			{
				context.run();
			});
	}
}

void net::core::tcp::stop()
{
	is_running = false;

	context.stop();

	for (int i = 0; i < context_workers.size(); i++)
	{
		if (context_workers[i].joinable())
		{
			context_workers[i].join();
		}
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
	acceptor->async_accept(
		[this](boost::system::error_code error, boost::asio::ip::tcp::socket client_socket)
		{
			CHECK_ACCEPT_RETURN_VOID(error, on_operation_aborted(), on_connection_aborted(), on_accept_error());
			CHECK_RETURN_VOID(!is_running);

			// 세션 추가
			const uint32_t new_session_id = session_id_counter.fetch_add(1, std::memory_order::memory_order_release);
			auto new_session = std::make_shared<connection>
			(
				context,
				std::move(client_socket),
				new_session_id,
				[this](uint32_t id) { connections.erase(id); },
				requests
			);
			new_session->start();
			connections.insert(new_session_id, new_session);

			SPDLOG_INFO("client : {} connected.", new_session_id);

			// 다음 연결 수락
			async_accept();
		});
}

void net::core::tcp::async_connect()
{
	socket.async_connect(endpoint, 
		[this](const boost::system::error_code& error)
		{
			CHECK_RETURN_VOID_ERROR(error, "failed to connect. {}", error.message());

			// 세션 추가
			const uint64_t new_session_id = session_id_counter.fetch_add(1, std::memory_order::memory_order_release);
			auto new_session = std::make_shared<connection>
			(
				context,
				std::move(socket),
				new_session_id,
				[this](uint64_t id) { connections.erase((uint32_t)id); },
				requests
			);
			new_session->start();

			connections.insert(new_session_id, new_session);
		});
}

void net::core::tcp::on_operation_aborted()
{
	SPDLOG_WARN("connect aborted by server side.");
}

void net::core::tcp::on_connection_aborted()
{
	SPDLOG_WARN("connect aborted by client side.");
}

void net::core::tcp::on_accept_error()
{
	SPDLOG_ERROR("accept failed. severe error occured.");
	stop();
}