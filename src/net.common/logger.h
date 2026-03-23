#pragma once
#include "singleton.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#define CHECK_RETURN_VOID(condition, ...) \
    do { \
        if (condition) \
		{ \
            SPDLOG_ERROR(__VA_ARGS__); \
			return; \
        } \
    } while (0)

#define CHECK_RETURN(condition, ret_val, ...) \
    do { \
        if (condition) \
		{ \
            SPDLOG_ERROR(__VA_ARGS__); \
			return ret_val; \
        } \
    } while (0)

#define CHECK_CONTINUE(condition, ...) \
    do { \
        if (condition) \
		{ \
            SPDLOG_ERROR(__VA_ARGS__); \
        } \
    } while (0)
namespace net::common
{
	class logger : public singleton<logger>
	{
	public:

		void init()
		{
			auto console = spdlog::stdout_color_mt("console");
			auto err_logger = spdlog::stderr_color_mt("stderr");

			spdlog::set_default_logger(console);
			spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %s:%# (%!) - %v");
		}
	};
}