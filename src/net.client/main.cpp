#include "pch.h"
#include "tcp.h"
#include "logic.h"
#define IP "192.168.0.3"
#define TCP_PORT 60000
#define UDP_PORT 50000
#define SESSION_SIZE 100000
#define SEND_BUFFER_SIZE 320LL * 1024 * 1024
#define RECV_BUFFER_SIZE 320LL * 1024 * 1024

int main()
{
	net::core::tcp client
	(
		IP,
		TCP_PORT
	);

	client.start();

	while (client.is_active())
	{
		// 폴링할 로직
	}

	return 0;
}