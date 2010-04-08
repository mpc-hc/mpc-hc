//************************************************************************
//
// LCDManager.cpp
//
// The CLCDManager class is the representation of a "Screen". LCD UI class
// objects are added here.
//
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#include "LCDManager.h"


//************************************************************************
//
// CLCDManager::CLCDManager
//
//************************************************************************

CLCDManager::CLCDManager(void)
{
    m_dwElapsedTime = 0;
    m_dwExpirationTime = 0;
    m_dwStartTime = 0;
}


//************************************************************************
//
// CLCDManager::~CLCDManager
//
//************************************************************************

CLCDManager::~CLCDManager(void)
{
    Shutdown();
}


//************************************************************************
//
// CLCDManager::Initialize
//
//************************************************************************

HRESULT CLCDManager::Initialize(void)
{
    HRESULT hRes = CLCDCollection::Initialize();
    if(FAILED(hRes))
    {
        LCDUITRACE(_T("CLCDManager::Initialize(): failed to initialize base class.\n"));
        Shutdown();
        return hRes;
    }

    SetOrigin(0, 0);
    SetSize(LGLCD_BMP_WIDTH, LGLCD_BMP_HEIGHT);

    hRes = m_Gfx.Initialize(GetWidth(), GetHeight());
    if(FAILED(hRes))
    {
        LCDUITRACE(_T("CLCDManager::Initialize(): failed to initialize graphics component.\n"));
        Shutdown();
        return hRes;
    }

    return S_OK;
}


//************************************************************************
//
// CLCDManager::Shutdown
//
//************************************************************************

void CLCDManager::Shutdown(void)
{
    m_Gfx.Shutdown();
}


//************************************************************************
//
// CLCDManager::GetLCDScreen
//
//************************************************************************

lgLcdBitmap160x43x1 *CLCDManager::GetLCDScreen(void)
{
    return m_Gfx.GetLCDScreen();
}


//************************************************************************
//
// CLCDManager::GetBitmapInfo
//
//************************************************************************

BITMAPINFO *CLCDManager::GetBitmapInfo(void)
{
    return m_Gfx.GetBitmapInfo();
}


//************************************************************************
//
// CLCDManager::Draw
//
//************************************************************************

HRESULT CLCDManager::Draw(void)
{
    LCDUIASSERT(NULL != m_Gfx.GetHDC());
    LCDUIASSERT(NULL != m_Gfx.GetHBITMAP());
    if((NULL == m_Gfx.GetHDC()) || (NULL == m_Gfx.GetHBITMAP()))
    {
        LCDUITRACE(_T("CLCDManager::Draw(): trying to draw without successful Initialize() first.!\n"));
        return E_FAIL;
    }

    // select our bitmap into the Gfx DC
    m_Gfx.BeginDraw();

    m_Gfx.ClearScreen();

    // invoke LCD UI Elements
    OnDraw(m_Gfx);

    // select it back out of it
    m_Gfx.EndDraw();

    return S_OK;
}


//************************************************************************
//
// CLCDManager::Update
//
//************************************************************************

void CLCDManager::Update(DWORD dwTimestamp)
{
    LCDUIASSERT(m_dwStartTime != 0);
    m_dwElapsedTime = (dwTimestamp - m_dwStartTime);
    CLCDCollection::OnUpdate(dwTimestamp);
}


//************************************************************************
//
// CLCDManager::SetExpiration
//
//************************************************************************

void CLCDManager::SetExpiration(DWORD dwMilliseconds)
{
    m_dwStartTime = GetTickCount();
    m_dwElapsedTime = 0;
    m_dwExpirationTime = dwMilliseconds;
}


//************************************************************************
//
// CLCDManager::HasExpired
//
//************************************************************************

BOOL CLCDManager::HasExpired(void)
{
    return (!m_dwStartTime || (m_dwElapsedTime > m_dwExpirationTime));
}


//************************************************************************
//
// CLCDManager::OnLCDButtonDown
//
//************************************************************************

void CLCDManager::OnLCDButtonDown(int nButton)
{
    UNREFERENCED_PARAMETER(nButton);
}


//************************************************************************
//
// CLCDManager::OnLCDButtonUp
//
//************************************************************************

void CLCDManager::OnLCDButtonUp(int nButton)
{
    UNREFERENCED_PARAMETER(nButton);
}


//** end of LCDManager.cpp ***********************************************
