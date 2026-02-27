#pragma once
#include <windows.h>
#include <chrono>
#include <memory>
#include "renderer.h"

namespace net::admin
{
    class app
    {
    public:
        app(HINSTANCE h_instance);
        ~app();

        bool initialize();
        void run();

    private:
        static LRESULT CALLBACK win32_proc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param);

        void update_server_data();

        void render();

        // 경로의 외부 프로세스 실행
        void execute_process(const std::string& exe_path);

        // 외부 프로세스가 실행하는 공유 메모리 생성
        void create_shared_memory();

    private:
        HINSTANCE h_instance;
        HWND window_handle = nullptr;
        std::unique_ptr<renderer> dx_renderer;
        bool is_running = true;
        std::chrono::steady_clock::time_point last_update_time;
    };
}
