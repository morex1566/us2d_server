#pragma once
#include <optional>
#include "packet.h"
#include "ts_deque.h"
#include "ts_pool.h"

namespace net::core
{
	class session : public std::enable_shared_from_this<session>
	{
	public:
		static constexpr uint32_t MAX_PAYLOAD_SIZE = 65536;

		session
		(
			boost::asio::io_context& context,
			std::shared_ptr<common::ts_pool<packet::packet>> server_packet_pool,
			common::ts_deque<packet::packet_request>& server_recv_buffer,
			boost::asio::ip::tcp::socket&& client_socket,
			uint64_t session_id	
		);

		session(session&& other) noexcept;
		
		session(const session&) = delete;

		session& operator=(const session&) = delete;

		~session();

		// 세션 종료
		void clear();

		void start();

		void stop();

		// 패킷 직렬화 후 비동기 전송
		void send(packet::packet_type type, std::shared_ptr<google::protobuf::Message> payload);
	
	private:

		// packet header 읽기
		void async_read_header();

		// packet payload 읽기
		void async_read_payload();

	private:

		std::atomic<bool> is_running{ false };

		boost::asio::io_context& context;

		// 클라이언트 소켓
		boost::asio::ip::tcp::socket socket;

		// 세션의 고유 id
		uint64_t session_id;

		// 서버 참조 : 세션이 사용할 패킷 풀
		std::shared_ptr<common::ts_pool<packet::packet>> server_packet_pool;

		// 서버 참조 : 클라 -> 서버 수신 버퍼
		common::ts_deque<packet::packet_request>& server_recv_buffer;

		// 패킷 풀에서 받은 데이터 수신용 패킷
		std::shared_ptr<packet::packet> current_pkt;
	};
}