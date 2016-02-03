#pragma once
#include <windows.h>

template <class Derived>
class BaseWindow
{
public:
    BaseWindow() : hWnd_(NULL) {}
    virtual ~BaseWindow() {}
    static LRESULT CALLBACK WindowProc(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam );
    BOOL Create(
        LPCTSTR lpWindowName,
        DWORD dwStyle,
        DWORD dwExStyle = 0,
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = CW_USEDEFAULT,
        int nHeight = CW_USEDEFAULT,
        HWND hWndParent = 0,
        HMENU hMenu = 0 );
    HWND Window() const;
protected:
    HWND    hWnd_;
    virtual LPCTSTR  ClassName() const = 0;
    virtual LRESULT HandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam ) = 0;
};

// Details
template<class Derived>
LRESULT CALLBACK BaseWindow<Derived>::WindowProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    Derived *pDerived = NULL;
    // Sent prior to the WM_CREATE message when a window is first created.
    if ( uMsg == WM_NCCREATE )
    {
        CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
        pDerived = reinterpret_cast<Derived *>(pCreate->lpCreateParams);
        // Changes an attribute of the specified window. 
        // The function also sets a value at the specified offset
        // in the extra window memory. 
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDerived));
        pDerived->hWnd_ = hWnd;
    }
    else
    {
        // Retrieves information about the specified window.
        // The function also retrieves the value at a specified offset
        // into the extra window memory. 
        pDerived = reinterpret_cast<Derived *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if ( pDerived )
    {
        return pDerived->HandleMessage(uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

template<class Derived>
BOOL BaseWindow<Derived>::Create(
    LPCTSTR lpWindowName,
    DWORD dwStyle,
    DWORD dwExStyle = 0,
    int x = CW_USEDEFAULT,
    int y = CW_USEDEFAULT,
    int nWidth = CW_USEDEFAULT,
    int nHeight = CW_USEDEFAULT,
    HWND hWndParent = 0,
    HMENU hMenu = 0 )
{
    WNDCLASSEX wc = { 0 };
    // register window procedure
    wc.lpfnWndProc   = Derived::WindowProc; 

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;  
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = ClassName();

    RegisterClassEx(&wc);
    // The CreateWindowEx function sends
    // WM_NCCREATE, WM_NCCALCSIZE, and WM_CREATE messages
    // to the window being created. 

    hWnd_ = CreateWindowEx(
        dwExStyle,
        ClassName(),
        lpWindowName,
        dwStyle,
        x,
        y,
        nWidth,
        nHeight,
        hWndParent,
        hMenu,
        GetModuleHandle(NULL),
        this
        );

    return ( hWnd_ ? TRUE : FALSE );
}

template<class Derived>
HWND BaseWindow<Derived>::Window() const
{
    return hWnd_;
}