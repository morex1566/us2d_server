#include "pch.h"
#include <d3d11.h>
#include <dxgi.h>
#include <tchar.h>
#include <chrono> 
#include <Windows.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// 전역 변수 (자료형은 유지, 변수명은 소문자 스네이크)
ID3D11Device* g_pd3d_device = nullptr;
ID3D11DeviceContext* g_pd3d_device_context = nullptr;
IDXGISwapChain* g_p_swap_chain = nullptr;
ID3D11RenderTargetView* g_main_render_target_view = nullptr;

// 함수 전방 선언
bool create_device_d3d(HWND window_handle);
void cleanup_device_d3d();
void create_render_target();
void cleanup_render_target();
LRESULT WINAPI win32_proc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param);

// 메인 앱 클래스
class process
{
public:
    process(HINSTANCE h_instance) : h_instance(h_instance) { }

    ~process() = default;

    void run()
    {
        // 윈도우 클래스 등록
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

        // 윈도우 생성
        HWND window_handle = CreateWindowW
        (
            wc.lpszClassName,
            L"server admin tool",
            WS_POPUP,                   // <-- 테두리와 제목 표시줄이 없는 팝업 스타일
            100, 100, 500, 400,         // 초기 크기를 ImGui 창 크기와 맞추면 좋습니다.
            nullptr, nullptr, wc.hInstance, nullptr
        );

        // d3d 초기화
        if (!create_device_d3d(window_handle))
        {
            cleanup_device_d3d();
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return;
        }

        ShowWindow(window_handle, SW_SHOWDEFAULT);
        UpdateWindow(window_handle);

        // imgui 초기화
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(window_handle);
        ImGui_ImplDX11_Init(g_pd3d_device, g_pd3d_device_context);

        bool running = true;
        auto last_update_time = std::chrono::steady_clock::now();

        // 메인 루프
        while (running)
        {
            MSG msg;
            while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                {
                    running = false;
                }
            }
            if (!running) break;

            // 0.1초 간격 업데이트 로직
            auto current_time = std::chrono::steady_clock::now();
            std::chrono::duration<float> elapsed = current_time - last_update_time;

            if (elapsed.count() >= 0.1f)
            {
                // 여기서 서버 데이터를 갱신 (recv 큐, 세션 정보 등)
                update_server_data();
                last_update_time = current_time;
            }

            // imgui 프레임 시작
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // 관리자 모니터링 창
            {
                // ImGui 창이 전체 윈도우 크기를 꽉 채우도록 설정
                ImGui::SetNextWindowPos(ImVec2(0, 0));
                ImGui::SetNextWindowSize(io.DisplaySize);

                // 창의 제목줄이나 리사이즈 핸들을 숨기고 싶을 때 사용하는 플래그
                ImGuiWindowFlags window_flags = 0;
                window_flags |= ImGuiWindowFlags_NoTitleBar;
                window_flags |= ImGuiWindowFlags_NoResize;
                window_flags |= ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoCollapse;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
                ImGui::Begin("server dashboard");

                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "server status: online");
                ImGui::Separator();

                // 예시 데이터 출력 (나중에 변수로 교체)
                ImGui::Text("recv queue size: %d", 125);
                ImGui::Text("active sessions: %d", 42);
                ImGui::Text("update interval: 0.1s");

                ImGui::Spacing();
                if (ImGui::Button("spawn remote instance", ImVec2(-1, 40)))
                {
                    // 버튼 클릭 시 서버에 명령 전달
                }

                if (ImGui::Button("clear all queues", ImVec2(-1, 40)))
                {
                    // 서버 청소 명령
                }

                ImGui::End();
            }

            // 렌더링
            ImGui::Render();
            const float clear_color_with_alpha[4] = { 0.15f, 0.15f, 0.15f, 1.00f };
            g_pd3d_device_context->OMSetRenderTargets(1, &g_main_render_target_view, nullptr);
            g_pd3d_device_context->ClearRenderTargetView(g_main_render_target_view, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // 수직 동기화(vsync)를 끄고 싶으면 Present(0, 0)
            g_p_swap_chain->Present(1, 0);
        }

        // 리소스 정리
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        cleanup_device_d3d();
        DestroyWindow(window_handle);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
    }

private:
    void update_server_data()
    {
        // TODO: 서버 소켓에서 최신 데이터를 가져오는 코드 작성
    }

private:
    HINSTANCE h_instance;
};

// --- DirectX 11 도우미 함수 구현 ---

bool create_device_d3d(HWND window_handle)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = window_handle;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT create_device_flags = 0;
    D3D_FEATURE_LEVEL feature_level;
    const D3D_FEATURE_LEVEL feature_level_array[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_device_flags, feature_level_array, 2, D3D11_SDK_VERSION, &sd, &g_p_swap_chain, &g_pd3d_device, &feature_level, &g_pd3d_device_context);

    if (res != S_OK) return false;

    create_render_target();
    return true;
}

void cleanup_device_d3d()
{
    cleanup_render_target();
    if (g_p_swap_chain) { g_p_swap_chain->Release(); g_p_swap_chain = nullptr; }
    if (g_pd3d_device_context) { g_pd3d_device_context->Release(); g_pd3d_device_context = nullptr; }
    if (g_pd3d_device) { g_pd3d_device->Release(); g_pd3d_device = nullptr; }
}

void create_render_target()
{
    ID3D11Texture2D* p_back_buffer;
    g_p_swap_chain->GetBuffer(0, IID_PPV_ARGS(&p_back_buffer));
    g_pd3d_device->CreateRenderTargetView(p_back_buffer, nullptr, &g_main_render_target_view);
    p_back_buffer->Release();
}

void cleanup_render_target()
{
    if (g_main_render_target_view) { g_main_render_target_view->Release(); g_main_render_target_view = nullptr; }
}

// 윈도우 메시지 핸들러
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI win32_proc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
    if (ImGui_ImplWin32_WndProcHandler(window_handle, message, w_param, l_param))
        return true;

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
        if (g_pd3d_device != nullptr && w_param != SIZE_MINIMIZED)
        {
            cleanup_render_target();
            g_p_swap_chain->ResizeBuffers(0, (UINT)LOWORD(l_param), (UINT)HIWORD(l_param), DXGI_FORMAT_UNKNOWN, 0);
            create_render_target();
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

int APIENTRY wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prev_instance, _In_ LPWSTR cmd_line, _In_ int cmd_show) 
{
    process app(instance);

    app.run();

    return 0;
}