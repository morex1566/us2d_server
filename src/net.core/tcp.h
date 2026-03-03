#pragma once
#include "session.h"
#include "ts_deque.h"
#include "ts_map.h"
#include "ts_pool.h"

namespace net::core
{
	/// <summary>
	/// TCP 서버 / 클라이언트 통합 클래스
	/// - 서버 모드: tcp(port)
	/// - 클라이언트 모드: tcp(host, port)
	/// </summary>
	class tcp
	{
	public:
		enum class role 
		{ 
			server, 
			client 
		};

		// 서버 모드 생성자
		explicit tcp(boost::asio::ip::port_type port);

		// 클라이언트 모드 생성자
		tcp(const std::string& host, boost::asio::ip::port_type port);

		tcp(const tcp&) = delete;

		tcp& operator=(const tcp&) = delete;

		tcp(tcp&&) = delete;

		tcp& operator=(tcp&&) = delete;

		~tcp();

		// 시작 (서버: accept 루프 / 클라이언트: connect 시도)
		void start();

		// 중지 (io_context stop + worker thread join)
		void stop();

		// 소켓/acceptor 정리
		void clear();

		// 현재 실행 중인지 여부
		bool is_active() const { return is_running; }

		// [서버 전용] 특정 세션에 패킷 전송
		void send(uint64_t session_id, packet::packet_type type, std::shared_ptr<google::protobuf::Message> payload);

	private:

		// 서버 모드
		void async_accept();

		// 클라이언트 모드
		void async_connect();

	private:

		// 역할 (서버 / 클라이언트)
		role role_;

		// 실행 여부
		std::atomic<bool> is_running{ false };

		// IOCP
		boost::asio::io_context context;

		// IOCP 스레드
		std::thread context_worker;

		// IOCP 작업 없을 때 꺼지지 않도록
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard;

		// TCP 소켓 (클라이언트 모드: connect 용 / 서버 모드: accept 임시 socket)
		boost::asio::ip::tcp::socket socket;

		// 접속 대상 엔드포인트
		boost::asio::ip::tcp::endpoint endpoint;

		// [서버 전용] acceptor
		std::optional<boost::asio::ip::tcp::acceptor> acceptor;

		// [서버 전용] 세션 uid 카운터
		std::atomic<uint64_t> session_id_counter{ 10000 };

		// [서버 전용] 세션 관리 맵
		common::ts_map<uint64_t, std::shared_ptr<session>> sessions;

		std::shared_ptr<common::ts_pool<packet::packet>> packet_pool;

		// 수신 버퍼
		common::ts_deque<packet::packet_request> recv_buffer;
	};
}
