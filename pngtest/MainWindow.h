#pragma once
#include "BaseWindow.h"
#include "PngUtil.h"
#include <tchar.h>

class MainWindow : public BaseWindow<MainWindow>
{
public:
    LRESULT  HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    LPCTSTR   ClassName() const override
    {
        return _T("PNG TEST");
    }

private:
    PngUtil     png;    
};