#pragma once
#include "net.packet/packet.h"
#include "net.common/system_config.h"
#include "net.common/ts_memory_pool.h"
#include <moodycamel/blockingconcurrentqueue.h>
#include <functional>

namespace net::core
{
	class connection : public std::enable_shared_from_this<connection>
	{
	public:

		connection
		(
			boost::asio::io_context& context,
			boost::asio::ip::tcp::socket&& client_socket,
			uint64_t session_id,
			std::function<void(uint64_t)> on_disconnected
		);
		
		connection(const connection&) = delete;

		connection& operator=(const connection&) = delete;

		~connection();

		// 세션 종료
		void clear();

		void start();

		void stop();

		moodycamel::BlockingConcurrentQueue<net::packet::packet_request>& get_requests() { return requests; }
	
	private:

		// packet header 읽기
		void async_read_header();

		// packet payload 읽기
		void async_read_payload(std::shared_ptr<uint8_t> full_buffer, uint16_t payload_size);

	private:

		std::atomic<bool> is_running = false;

		std::atomic<bool> is_writing = false;

		std::atomic<bool> is_reading = false;

		boost::asio::io_context& context;

		boost::asio::strand<boost::asio::io_context::executor_type> strand;

		// 클라이언트 소켓
		boost::asio::ip::tcp::socket socket;

		// 세션의 고유 id
		uint64_t session_id;

		std::function<void(uint64_t)> on_disconnected;

		// 고정 크기 헤더 임시 버퍼 (20 bytes)
		uint8_t header_read_buffer[sizeof(net::protocol::packet_header)];

		// 디스패처가 읽는 부분
		// TODO : 시퀀스 -> priority queue로 구현?
		moodycamel::BlockingConcurrentQueue<net::packet::packet_request> requests;
	};
}