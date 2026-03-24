#include "pch.h"
#include "tcp.h"
#include "net.common/log.h"

net::core::tcp::tcp()
	:	singleton(),
		socket(context),
		work_guard(boost::asio::make_work_guard(context))
{
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
	}
	else
	{
		async_connect();
	}

	// cpu 코어 수에 맞게 스레드 실행
	// 메인스레드 자리는 남김
	const int thread_count = std::thread::hardware_concurrency() - 1;
	for (int i = 0; i < thread_count; i++)
	{
		context_workers.emplace_back([this]()
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
	CHECK_RETURN_VOID(mode == tcp::mode::CLIENT, "tcp mode is client.");

	acceptor->async_accept([this](boost::system::error_code error, boost::asio::ip::tcp::socket client_socket)
	{
		if (!error)
		{
			// 세션 추가
			const uint64_t new_session_id = session_id_counter.fetch_add(1, std::memory_order::memory_order_release);
			auto new_session = std::make_shared<connection>
			(
				context,
				std::move(client_socket),
				new_session_id,
				[this](uint64_t id) { sessions.erase((uint32_t)id); }
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
	CHECK_RETURN_VOID(mode == tcp::mode::SERVER, "tcp mode is server.");

	socket.async_connect(endpoint, [this](const boost::system::error_code& error)
	{
		CHECK_RETURN_VOID(error, "failed to connect. {}", error.message());

		// 세션 추가
		const uint64_t new_session_id = session_id_counter.fetch_add(1, std::memory_order::memory_order_release);
		auto new_session = std::make_shared<connection>
		(
			context,
			std::move(socket),
			new_session_id,
			[this](uint64_t id) { sessions.erase((uint32_t)id); }
		);
		new_session->start();

		sessions.insert(new_session_id, new_session);
	});
}