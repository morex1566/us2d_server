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

		//

	protected:

		//

	private:

		//

	public:

		connection
		(
			boost::asio::io_context& context,
			boost::asio::ip::tcp::socket&& client_socket,
			uint64_t connection_id,
			std::function<void(uint64_t)> on_disconnected,
			moodycamel::BlockingConcurrentQueue<net::packet::packet_request>& requests
		);

		connection(const connection&) = delete;

		connection& operator=(const connection&) = delete;

		~connection();

	protected:

		//

	private:

		//

	public:

		// 세션 종료
		void clear();

		void start();

		void stop();

	protected:

		//

	private:

		// packet header 읽기
		void async_read_header();

		// packet payload 읽기
		void async_read_payload(std::shared_ptr<uint8_t> packet_owner, uint16_t payload_size);

	protected:

		//

	private:

		std::atomic<bool> is_running = false;

		std::atomic<bool> is_writing = false;

		std::atomic<bool> is_reading = false;

		boost::asio::io_context& context;

		boost::asio::strand<boost::asio::io_context::executor_type> strand;

		// 클라이언트 소켓
		boost::asio::ip::tcp::socket socket;

		// 세션의 고유 id
		uint32_t connection_id;

		std::function<void(uint32_t)> on_disconnected;

		// 고정 크기 헤더 임시 버퍼 (20 bytes)
		uint8_t header_read_buffer[sizeof(net::protocol::packet_header)];

		// 디스패처가 읽는 부분
		// TODO : 시퀀스 -> priority queue로 구현?
		moodycamel::BlockingConcurrentQueue<net::packet::packet_request>& requests;
	};
}