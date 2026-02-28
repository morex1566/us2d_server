#include "pch.h"
#include "app.h"

int APIENTRY wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prev_instance, _In_ LPWSTR cmd_line, _In_ int cmd_show) 
{
    net::admin::app app(instance);

    if (app.initialize())
    {
        app.run();
    }

    return 0;
}
