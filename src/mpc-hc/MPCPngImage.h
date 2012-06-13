#pragma once

#include <atlimage.h>

class CMPCPngImage : public CBitmap
{
    // Construction/Destruction
public:
    CMPCPngImage();
    virtual ~CMPCPngImage();

    // Attributes:
protected:
    static ATL::CImage* m_pImage;

    // Operations:
public:
    BOOL Load(UINT uiResID, HINSTANCE hinstRes = NULL);
    BOOL Load(LPCTSTR lpszResourceName, HINSTANCE hinstRes = NULL);

    BOOL LoadFromFile(LPCTSTR lpszPath);
    BOOL LoadFromBuffer(LPBYTE lpBuffer, UINT uiSize);

    static void __stdcall CleanUp() {
        if (m_pImage != NULL) {
            delete m_pImage;
            m_pImage = NULL;
        }
    }
};
