#pragma once
#include <algorithm>
namespace Gdiplus   //avoid GDIPLUS min/max issues when including atlimage
{
    using std::min;
    using std::max;
}
#include <atlimage.h>

class ColorProfileUtil
{
public:
    static TCHAR* getIccProfilePath(HWND wnd);
    static bool applyColorProfile(HWND wnd, CImage& image);
};

