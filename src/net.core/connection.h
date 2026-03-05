#pragma once
#include <optional>
#include "packet.h"
#include "ts_memory_pool.h"
#include "concurrentqueue/concurrentqueue.h"

namespace net::core
{
	class connection : public std::enable_shared_from_this<connection>
	{
	public:

		connection
		(
			boost::asio::io_context& context,
			common::ts_memory_pool& server_memory_pool,
			moodycamel::ConcurrentQueue<void*>& recv_queue,
			boost::asio::ip::tcp::socket&& client_socket,
			uint64_t session_id	
		);
		
		connection(const connection&) = delete;

		connection& operator=(const connection&) = delete;

		~connection();

		// 세션 종료
		void clear();

		void start();

		void stop();

		moodycamel::ConcurrentQueue<void*>& get_send_queue() { return send_queue; }
	
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

		/// <summary>
		/// 페이로드 수신용 메모리 풀 (슬랩 할당)
		/// </summary>
		common::ts_memory_pool& server_memory_pool;

		/// <summary>
		/// 클라 -> 서버 수신 큐 (ts_object_pool queue 모드)
		/// </summary>
		moodycamel::ConcurrentQueue<void*>& recv_queue;

		/// <summary>
		/// 수신 패킷 요청 큐
		/// </summary>
		moodycamel::ConcurrentQueue<void*> send_queue;

		/// <summary>
		/// 현재 수신 중인 패킷 헤더 저장용
		/// </summary>
		packet::packet curr_packet;
	};
}