#include "stdafx.h"
#include "ColorProfileUtil.h"
#include "lcms2/library/include/lcms2.h"
#include "MainFrm.h"
#include "Icm.h"

TCHAR* ColorProfileUtil::getIccProfilePath(HWND wnd)
{
    TCHAR* iccProfilePath = 0;

    HMONITOR hMonitor = MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFOEX miex;
    miex.cbSize = sizeof(miex);
    GetMonitorInfo(hMonitor, &miex);

    HDC hDC = CreateDC(_T("DISPLAY"), miex.szDevice, nullptr, nullptr);

    if (hDC != nullptr) {
        DWORD icmProfilePathSize = 0;
        GetICMProfile(hDC, &icmProfilePathSize, nullptr);
        iccProfilePath = DEBUG_NEW TCHAR[icmProfilePathSize];
        if (!GetICMProfile(hDC, &icmProfilePathSize, iccProfilePath)) {
            delete[] iccProfilePath;
            iccProfilePath = 0;
        }
        DeleteDC(hDC);
    }
    return iccProfilePath;
}

bool ColorProfileUtil::applyColorProfile(HWND wnd, CImage& image)
{
    cmsHPROFILE hInputProfile = cmsCreate_sRGBProfile();

    cmsHPROFILE hOutputProfile = nullptr;
    FILE* outputProfileStream = nullptr;

    TCHAR* iccProfilePath = getIccProfilePath(wnd);

    if (iccProfilePath != 0) {
        if (_wfopen_s(&outputProfileStream, T2W(iccProfilePath), L"rb") != 0) {
            cmsCloseProfile(hInputProfile);
            return false;
        }
        hOutputProfile = cmsOpenProfileFromStream(outputProfileStream, "r");
        delete[] iccProfilePath;
    }

    if (!hOutputProfile) {
        hOutputProfile = cmsCreate_sRGBProfile();
    }

    cmsHTRANSFORM hTransform;
    cmsUInt32Number type = image.GetBPP() == 32 ? TYPE_BGRA_8 : TYPE_BGR_8;
    hTransform = cmsCreateTransform(hInputProfile, type, hOutputProfile, type, INTENT_PERCEPTUAL, 0);

    if (hTransform) {
        BYTE* bits = static_cast<BYTE*>(image.GetBits());
        for (int y = 0; y < image.GetHeight(); y++, bits += image.GetPitch()) {
            RGBQUAD* p = reinterpret_cast<RGBQUAD*>(bits);
            cmsDoTransform(hTransform, p, p, image.GetWidth());
        }
        cmsDeleteTransform(hTransform);
    }

    cmsCloseProfile(hInputProfile);
    cmsCloseProfile(hOutputProfile);

    return true;
}
