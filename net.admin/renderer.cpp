#include "pch.h"
#include "renderer.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace net::admin
{
    renderer::renderer() = default;

    renderer::~renderer()
    {
        cleanup();
    }

    bool renderer::initialize(HWND window_handle)
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

        HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_device_flags, feature_level_array, 2, D3D11_SDK_VERSION, &sd, &p_swap_chain, &pd3d_device, &feature_level, &pd3d_device_context);

        if (res != S_OK) return false;

        create_render_target();
        return true;
    }

    void renderer::cleanup()
    {
        cleanup_render_target();
        if (p_swap_chain) { p_swap_chain->Release(); p_swap_chain = nullptr; }
        if (pd3d_device_context) { pd3d_device_context->Release(); pd3d_device_context = nullptr; }
        if (pd3d_device) { pd3d_device->Release(); pd3d_device = nullptr; }
    }

    void renderer::begin_frame()
    {
        const float clear_color_with_alpha[4] = { 0.15f, 0.15f, 0.15f, 1.00f };
        pd3d_device_context->OMSetRenderTargets(1, &main_render_target_view, nullptr);
        pd3d_device_context->ClearRenderTargetView(main_render_target_view, clear_color_with_alpha);
    }

    void renderer::end_frame()
    {
        p_swap_chain->Present(1, 0); // VSync enabled
    }

    void renderer::create_render_target()
    {
        ID3D11Texture2D* p_back_buffer;
        p_swap_chain->GetBuffer(0, IID_PPV_ARGS(&p_back_buffer));
        pd3d_device->CreateRenderTargetView(p_back_buffer, nullptr, &main_render_target_view);
        p_back_buffer->Release();
    }

    void renderer::cleanup_render_target()
    {
        if (main_render_target_view) { main_render_target_view->Release(); main_render_target_view = nullptr; }
    }

    void renderer::handle_resize(UINT width, UINT height)
    {
        if (pd3d_device != nullptr)
        {
            cleanup_render_target();
            p_swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
            create_render_target();
        }
    }
}
