#include <Windowing/Window.h>
#include <Networking/Socket.h>

#include <windows.h>
#include <commctrl.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmd)
{
    INITCOMMONCONTROLSEX controls;
    controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    controls.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&controls);

    Secretest::SocketContext context{};
    Secretest::Server server{ 2831 };
    server.Listen();

    Secretest::Window window{ uvec2(980, 720) };
    Secretest::Window::RunWindows();

    return 0;
}
