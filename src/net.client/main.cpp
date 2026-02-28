#include "pch.h"
#include "tcp_client.h"

#define IP "192.168.0.3"
#define TCP_PORT 60000
#define UDP_PORT 50000
#define SESSION_SIZE 100000
#define SEND_BUFFER_SIZE 320LL * 1024 * 1024
#define RECV_BUFFER_SIZE 320LL * 1024 * 1024

int main()
{
	net::core::tcp_client client
	(
		IP,
		TCP_PORT
	);

	client.start();

	return 0;
}