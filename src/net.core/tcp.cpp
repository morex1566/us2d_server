#include "pch.h"
#include "tcp.h"

// ─────────────────────────────────────────────
// 서버 모드 생성자
// ─────────────────────────────────────────────
net::core::tcp::tcp(boost::asio::ip::port_type port)
	: role_(role::server),
	  socket(context),
	  endpoint(boost::asio::ip::tcp::v4(), port),
	  work_guard(boost::asio::make_work_guard(context)),
      packet_pool(std::make_shared<common::ts_pool<packet::packet>>())
{
	acceptor.emplace(context, endpoint);
}

// ─────────────────────────────────────────────
// 클라이언트 모드 생성자
// ─────────────────────────────────────────────
net::core::tcp::tcp(const std::string& host, boost::asio::ip::port_type port)
	: role_(role::client),
	  socket(context),
	  endpoint(boost::asio::ip::make_address(host), port),
	  work_guard(boost::asio::make_work_guard(context)),
	  packet_pool(std::make_shared<common::ts_pool<packet::packet>>())
{
}

net::core::tcp::~tcp()
{
	stop();
	clear();
}

// ─────────────────────────────────────────────
// start
// ─────────────────────────────────────────────
void net::core::tcp::start()
{
	is_running = true;

	if (role_ == role::server)
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

// ─────────────────────────────────────────────
// stop
// ─────────────────────────────────────────────
void net::core::tcp::stop()
{
	is_running = false;

	context.stop();

	if (context_worker.joinable())
	{
		context_worker.join();
	}
}

// ─────────────────────────────────────────────
// clear
// ─────────────────────────────────────────────
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

// ─────────────────────────────────────────────
// send [서버 전용]
// ─────────────────────────────────────────────
void net::core::tcp::send(uint64_t session_id, packet::packet_type type, std::shared_ptr<google::protobuf::Message> payload)
{
	std::shared_ptr<session> target_session;

	if (sessions.get(session_id, target_session))
	{
		target_session->send(type, payload);
	}
	else
	{
		std::cout << "tcp::send() - session not found: " << session_id << std::endl;
	}
}

// ─────────────────────────────────────────────
// async_accept [서버 모드]
// ─────────────────────────────────────────────
void net::core::tcp::async_accept()
{
	acceptor->async_accept([this](boost::system::error_code error, boost::asio::ip::tcp::socket client_socket)
	{
		if (!error)
		{
			// 세션 추가
			const uint64_t new_session_id = session_id_counter++;
			auto new_session = std::make_shared<session>
			(
				context,
				packet_pool,
				recv_buffer,
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

// ─────────────────────────────────────────────
// async_connect [클라이언트 모드]
// ─────────────────────────────────────────────
void net::core::tcp::async_connect()
{
	socket.async_connect(endpoint, [this](const boost::system::error_code& error)
	{
		if (!error)
		{
			// 세션 추가
			const uint64_t new_session_id = session_id_counter++;
			auto new_session = std::make_shared<session>
			(
				context,
				packet_pool,
				recv_buffer,
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