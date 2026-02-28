#include "pch.h"
#include "app.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <tchar.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace net::admin
{
    app::app(HINSTANCE h_instance) : h_instance(h_instance)
    {
        dx_renderer = std::make_unique<renderer>();
    }

    app::~app()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (window_handle)
        {
            DestroyWindow(window_handle);
            UnregisterClassW(L"admin_tool", h_instance);
        }
    }

    bool app::initialize()
    {
        WNDCLASSEXW wc =
        {
            sizeof(wc),
            CS_CLASSDC,
            win32_proc,
            0L,
            0L,
            h_instance,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            L"admin_tool",
            nullptr
        };

        RegisterClassExW(&wc);

        window_handle = CreateWindowW
        (
            wc.lpszClassName,
            L"server admin tool",
            WS_POPUP,
            100, 100, 500, 400,
            nullptr, nullptr, wc.hInstance, nullptr
        );

        if (!window_handle) return false;

        SetWindowLongPtr(window_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        if (!dx_renderer->initialize(window_handle))
        {
            return false;
        }

        ShowWindow(window_handle, SW_SHOWDEFAULT);
        UpdateWindow(window_handle);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(window_handle);
        ImGui_ImplDX11_Init(dx_renderer->get_device(), dx_renderer->get_device_context());

        last_update_time = std::chrono::steady_clock::now();

        return true;
    }

    void app::run()
    {
        while (is_running)
        {
            MSG msg;
            while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                {
                    is_running = false;
                }
            }
            if (!is_running) break;

            auto current_time = std::chrono::steady_clock::now();
            std::chrono::duration<float> elapsed = current_time - last_update_time;

            if (elapsed.count() >= 0.1f)
            {
                update_server_data();
                last_update_time = current_time;
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            render();

            ImGui::Render();
            dx_renderer->begin_frame();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            dx_renderer->end_frame();
        }
    }

    LRESULT CALLBACK app::win32_proc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
    {
        if (ImGui_ImplWin32_WndProcHandler(window_handle, message, w_param, l_param))
            return true;

        app* self = reinterpret_cast<app*>(GetWindowLongPtr(window_handle, GWLP_USERDATA));

        switch (message)
        {
        case WM_NCHITTEST:
        {
            LRESULT hit = DefWindowProc(window_handle, message, w_param, l_param);
            if (hit == HTCLIENT)
            {
                return HTCAPTION;
            }
            return hit;
        }

        case WM_SIZE:
            if (self && self->dx_renderer && w_param != SIZE_MINIMIZED)
            {
                self->dx_renderer->handle_resize((UINT)LOWORD(l_param), (UINT)HIWORD(l_param));
            }
            return 0;

        case WM_SYSCOMMAND:
            if ((w_param & 0xfff0) == SC_KEYMENU) return 0;
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(window_handle, message, w_param, l_param);
    }

    void app::update_server_data()
    {
        // TODO: 서버 소켓에서 최신 데이터를 가져오는 코드 작성
    }

    void app::render()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("server dashboard", nullptr, window_flags);

        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "server status: online");
        ImGui::Separator();

        ImGui::Text("recv queue size: %d", 125);
        ImGui::Text("active sessions: %d", 42);
        ImGui::Text("update interval: 0.1s");

        ImGui::Spacing();
        if (ImGui::Button("spawn remote instance", ImVec2(-1, 40)))
        {
        }

        if (ImGui::Button("clear all queues", ImVec2(-1, 40)))
        {
            
        }

        ImGui::End();
    }
}