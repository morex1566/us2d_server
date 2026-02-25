#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <type_traits>

namespace net::core
{
	namespace asio_ip = boost::asio::ip;
	namespace asio = boost::asio;
}