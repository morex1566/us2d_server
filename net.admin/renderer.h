#pragma once
#include <d3d11.h>
#include <dxgi.h>

namespace net::admin
{
    class renderer
    {
    public:
        renderer();
        ~renderer();

        bool initialize(HWND window_handle);
        void cleanup();

        void begin_frame();
        void end_frame();

        void create_render_target();
        void cleanup_render_target();

        void handle_resize(UINT width, UINT height);

        ID3D11Device* get_device() const { return pd3d_device; }

        ID3D11DeviceContext* get_device_context() const { return pd3d_device_context; }

        ID3D11RenderTargetView* get_render_target_view() const { return main_render_target_view; }

        IDXGISwapChain* get_swap_chain() const { return p_swap_chain; }

    private:
        ID3D11Device* pd3d_device = nullptr;

        ID3D11DeviceContext* pd3d_device_context = nullptr;

        IDXGISwapChain* p_swap_chain = nullptr;

        ID3D11RenderTargetView* main_render_target_view = nullptr;
    };
}
