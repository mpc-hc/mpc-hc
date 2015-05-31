#pragma once

#include <atlimage.h>

class CMPCPngImage : public CBitmap
{
    // Construction/Destruction
public:
    CMPCPngImage() = default;
    virtual ~CMPCPngImage() = default;

    // Attributes:
protected:
    static ATL::CImage* m_pImage;

    // Operations:
public:
    BOOL Load(UINT uiResID, HINSTANCE hinstRes = nullptr);
    BOOL Load(LPCTSTR lpszResourceName, HINSTANCE hinstRes = nullptr);

    BOOL LoadFromFile(LPCTSTR lpszPath);
    BOOL LoadFromBuffer(const LPBYTE lpBuffer, UINT uiSize);

    CSize GetSize();

    static void __stdcall CleanUp() {
        SAFE_DELETE(m_pImage);
    }
};
