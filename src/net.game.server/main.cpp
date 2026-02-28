#include "pch.h"
#include "tcp_server.h"

#define IP "192.168.0.3"
#define TCP_PORT 60000
#define UDP_PORT 50000

int main()
{
	boost::asio::io_context context;
	boost::asio::ip::tcp::socket socket(context);
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(IP), TCP_PORT);
	boost::asio::ip::tcp::acceptor acceptor(context, endpoint);

	net::core::tcp_server server
	(
		context,
		std::move(socket),
		endpoint,
		acceptor
	);

	server.start();

	return 0;
}