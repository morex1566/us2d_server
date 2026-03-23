#include "pch.h"
#include "tcp.h"
#include "rudp.h"
#include "logic.h"
#include "logger.h"
#include "game_engine.h"
#define IP "192.168.0.3"
#define TCP_PORT 60000
#define RUDP_PORT 50000
#define SESSION_SIZE 100000
#define SEND_BUFFER_SIZE 320LL * 1024 * 1024
#define RECV_BUFFER_SIZE 320LL * 1024 * 1024

int main()
{
	net::common::logger& logger = net::common::logger::get_instance();
	{
		logger.init();
	}

	net::core::tcp& tcp = net::core::tcp::get_instance();
	{
		tcp.init(TCP_PORT);
	}

	net::core::rudp& rudp = net::core::rudp::get_instance();
	{
		rudp.init(RUDP_PORT);
	}

	game_engine engine;

	while (tcp.is_active() && rudp.is_active())
	{
		// 폴링할 로직
	}

	return 0;
}