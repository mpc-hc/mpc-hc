#pragma once
#include "stdafx.h"
#include "CMPCTheme.h"
class CMPCThemeFrameUtil
{

public:
    CWnd* self;
    CMPCThemeFrameUtil(CWnd* self);
    bool IsWindowForeground() { return self == self->GetActiveWindow(); };
    bool IsWindowZoomed() { return self->IsZoomed(); };
    BOOL PostWindowMessage(UINT Msg, WPARAM wParam, LPARAM lParam) { return self->PostMessage(Msg, wParam, lParam); };
};

