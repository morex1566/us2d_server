#include "pch.h"
#include "dispatcher.h"

dispatcher::dispatcher(uint8_t thread_count) : thread_count(thread_count), is_running(false)
{
}

dispatcher::~dispatcher()
{
   
}