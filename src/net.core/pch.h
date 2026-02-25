#pragma once
#ifndef PCH_H
#define PCH_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <type_traits>

#include "boost/asio.hpp"

#endif //PCH_H