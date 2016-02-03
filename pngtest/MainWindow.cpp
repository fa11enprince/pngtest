#include "MainWindow.h"
#include <assert.h>

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
    case WM_PAINT:
        {
            png.Load(hWnd_, "v01.png");
            return TRUE;
        }

    case WM_CLOSE:
        {
            PostQuitMessage(0);
            return TRUE;
        }

    default:
        return DefWindowProc(hWnd_, uMsg, wParam, lParam);
    }

    return TRUE;
}