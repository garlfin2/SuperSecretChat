#include <Server/ServerWindow.h>

#include <windows.h>
#include <commctrl.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmd)
{
    INITCOMMONCONTROLSEX controls;
    controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    controls.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&controls);

    Secretest::SocketContext context{};

    Secretest::ServerWindow window{ uvec2(980, 720), 3283 };
    Secretest::Window::RunWindows();

    return 0;
}
