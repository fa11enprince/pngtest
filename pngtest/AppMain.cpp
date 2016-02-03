#include <windows.h>
#include <tchar.h>
#include "MainWindow.h"

INT APIENTRY _tWinMain(
    _In_        HINSTANCE   hInstance,
    _In_opt_    HINSTANCE   hPrevInstance,
    _In_        LPTSTR      lpCmdLine,
    _In_        INT         nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MainWindow win;

    if ( !win.Create(
        _T("PNG TEST"),
        WS_OVERLAPPEDWINDOW
        )
    )
    {
        return 0;
    }
    ShowWindow( win.Window(), nCmdShow );
    UpdateWindow( win.Window() );

    MSG msg = { 0 };
    while ( GetMessage(&msg, NULL, 0, 0) )
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}