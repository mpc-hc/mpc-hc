//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDOutput.cpp
//
// The CLCDOutput manages the actual device and the various pages that
// are sent to that device
// 
// This class is now managed by CLCDConnection. You no longer need to 
// derive or instantiate this class yourself
//
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDOutput::CLCDOutput
//
//************************************************************************

CLCDOutput::CLCDOutput(void)
:   m_pActivePage(NULL),
    m_hDevice(LGLCD_INVALID_DEVICE),
    m_bSetAsForeground(FALSE),
    m_dwButtonState(0),
    m_nPriority(LGLCD_PRIORITY_NORMAL),
    m_pGfx(NULL)
{
    ZeroMemory(&m_OpenByTypeContext, sizeof(m_OpenByTypeContext));
}


//************************************************************************
//
// CLCDOutput::~CLCDOutput
//
//************************************************************************

CLCDOutput::~CLCDOutput(void)
{
    Shutdown();
}


//************************************************************************
//
// CLCDOutput::SetGfx
//
//************************************************************************

void CLCDOutput::SetGfx(CLCDGfxBase *gfx)
{
    m_pGfx = gfx;
}


//************************************************************************
//
// CLCDOutput::Open
//
//************************************************************************

BOOL CLCDOutput::Open(lgLcdOpenContext & OpenContext)
{
    //Close the old device if there is one
    Close();

    DWORD res = lgLcdOpen(&OpenContext);
    if (ERROR_SUCCESS != res)
    {
        if( res == ERROR_INVALID_PARAMETER )
        {
            LCDUITRACE( _T("Open failed: invalid parameter.\n") );
            return FALSE;
        }
        else if( res == ERROR_ALREADY_EXISTS )
        {
            LCDUITRACE( _T("Open failed: already exists.\n") );
            return FALSE;
        }
        return FALSE;
    }

    m_hDevice = OpenContext.device;
    m_dwButtonState = 0;

    // restores
    SetAsForeground(m_bSetAsForeground);

    OnOpenedDevice(m_hDevice);

    return TRUE;
}


//************************************************************************
//
// CLCDOutput::OpenByType
//
//************************************************************************

BOOL CLCDOutput::OpenByType(lgLcdOpenByTypeContext &OpenContext)
{
    //Close the old device if there is one
    Close();

    DWORD res = lgLcdOpenByType(&OpenContext);
    if (ERROR_SUCCESS != res)
    {
        if( res == ERROR_INVALID_PARAMETER )
        {
            LCDUITRACE( _T("Open failed: invalid parameter.\n") );
            return FALSE;
        }
        else if( res == ERROR_ALREADY_EXISTS )
        {
            LCDUITRACE( _T("Open failed: already exists.\n") );
            return FALSE;
        }
        return FALSE;
    }

    m_hDevice = OpenContext.device;
    m_dwButtonState = 0;

    // restores
    SetAsForeground(m_bSetAsForeground);

    m_OpenByTypeContext = OpenContext;

    OnOpenedDevice(m_hDevice);

    return TRUE;
}


//************************************************************************
//
// CLCDOutput::ReOpenDeviceType
//
//************************************************************************

BOOL CLCDOutput::ReOpenDeviceType(void)
{
    // The device type must be active
    if (!HasBeenOpenedByDeviceType())
    {
        return FALSE;
    }

    return OpenByType(m_OpenByTypeContext);
}


//************************************************************************
//
// CLCDOutput::Close
//
//************************************************************************

void CLCDOutput::Close(void)
{
    if( LGLCD_INVALID_DEVICE != m_hDevice )
    {
        OnClosingDevice(m_hDevice);
        lgLcdClose(m_hDevice);
        m_hDevice = LGLCD_INVALID_DEVICE;
    }
}


//************************************************************************
//
// CLCDOutput::Shutdown
//
//************************************************************************

void CLCDOutput::Shutdown(void)
{
    Close();
}


//************************************************************************
//
// CLCDOutput::AddPage
//
//************************************************************************

void CLCDOutput::AddPage(CLCDPage *pPage)
{
    pPage->Initialize();
    AddObject(pPage);
}


//************************************************************************
//
// CLCDOutput::RemovePage
//
//************************************************************************

void CLCDOutput::RemovePage(CLCDPage *pPage)
{
    RemoveObject(pPage);
}


//************************************************************************
//
// CLCDOutput::ShowPage
//
//************************************************************************

void CLCDOutput::ShowPage(CLCDPage *pPage, BOOL bShow)
{
    LCDUIASSERT(NULL != pPage);

    if (bShow)
    {
        m_pActivePage = pPage;

        OnPageShown(pPage);
    }
    else
    {
        // Expire it and update
        pPage->SetExpiration(0);
        OnUpdate(GetTickCount());
    }
}


//************************************************************************
//
// CLCDOutput::GetShowingPage
//
//************************************************************************

CLCDPage* CLCDOutput::GetShowingPage(void)
{
    return m_pActivePage;
}


//************************************************************************
//
// CLCDOutput::SetScreenPriority
//
//************************************************************************

void CLCDOutput::SetScreenPriority(DWORD priority)
{
    // Priority has changed
    // If we're going into idle, send an idle frame
    if (LGLCD_PRIORITY_IDLE_NO_SHOW == priority)
    {
        lgLcdUpdateBitmap(m_hDevice, &m_pGfx->GetLCDScreen()->bmp_mono.hdr,
            LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_IDLE_NO_SHOW));
    }

    m_nPriority = priority;
}


//************************************************************************
//
// CLCDOutput::GetScreenPriority
//
//************************************************************************

DWORD CLCDOutput::GetScreenPriority(void)
{
    return m_nPriority;
}


//************************************************************************
//
// CLCDOutput::IsOpened
//
//************************************************************************

BOOL CLCDOutput::IsOpened(void)
{
    return (LGLCD_INVALID_DEVICE != m_hDevice);
}


//************************************************************************
//
// CLCDOutput::SetAsForeground
//
//************************************************************************

HRESULT CLCDOutput::SetAsForeground(BOOL bSetAsForeground)
{
    m_bSetAsForeground = bSetAsForeground;
    if (LGLCD_INVALID_DEVICE != m_hDevice)
    {
        DWORD dwRes = lgLcdSetAsLCDForegroundApp(m_hDevice, bSetAsForeground);
        if(ERROR_SUCCESS != dwRes)
        {
            return MAKE_HRESULT(SEVERITY_ERROR, 0, dwRes);
        }
    }
    return E_FAIL;
}


//************************************************************************
//
// CLCDOutput::OnDraw
//
//************************************************************************

BOOL CLCDOutput::OnDraw(void)
{
    DWORD dwPriorityToUse = LGLCD_ASYNC_UPDATE(m_nPriority);

    if ( (NULL == m_pActivePage) ||
        (LGLCD_INVALID_DEVICE == m_hDevice) ||
        (LGLCD_PRIORITY_IDLE_NO_SHOW == dwPriorityToUse) )
    {
        // don't submit the bitmap
        return TRUE;
    }

    // Render the active screen
    m_pGfx->BeginDraw();
    m_pGfx->ClearScreen();
    m_pActivePage->OnDraw(*m_pGfx);
    m_pGfx->EndDraw(); 

    // Get the active bitmap
    lgLcdBitmap* pBitmap = m_pGfx->GetLCDScreen();

    // Only submit if the bitmap needs to be updated
    // (If the priority or bitmap have changed)
    DWORD res = ERROR_SUCCESS;
    if (DoesBitmapNeedUpdate(pBitmap))
    {
        res = lgLcdUpdateBitmap(m_hDevice, &pBitmap->bmp_mono.hdr, dwPriorityToUse);
        HandleErrorFromAPI(res);
    }

    return (LGLCD_INVALID_DEVICE != m_hDevice);
}


//************************************************************************
//
// CLCDOutput::OnUpdate
//
//************************************************************************

void CLCDOutput::OnUpdate(DWORD dwTimestamp)
{
    if (m_pActivePage)
    {
        m_pActivePage->OnUpdate(dwTimestamp);
    }

    // check for expiration
    if (m_pActivePage && m_pActivePage->HasExpired())
    {
        m_pActivePage = NULL;
        //m_nPriority = LGLCD_PRIORITY_FYI; -> needs to go so that if a 
        // program sets priority to LGLCD_PRIORITY_BACKGROUND, that 
        // priority sticks.

        OnPageExpired(m_pActivePage);

        // find the next active screen
        for (size_t i = 0; i < m_Objects.size(); i++)
        {
            CLCDPage *pPage = dynamic_cast<CLCDPage*>(m_Objects[i]);
            LCDUIASSERT(NULL != pPage);

            if (!pPage->HasExpired())
            {
                ShowPage(pPage);
                //m_nPriority = LGLCD_PRIORITY_FYI;  -> needs to go so that if a 
                // program sets priority to LGLCD_PRIORITY_BACKGROUND, that 
                // priority sticks.
                break;
            }
        }

        // if no screen found, empty the screen at idle priority
        if (NULL == m_pActivePage)
        {
            OnEnteringIdle();
            if (LGLCD_INVALID_DEVICE != m_hDevice)
            {
                m_pGfx->ClearScreen();
                lgLcdUpdateBitmap(m_hDevice, &m_pGfx->GetLCDScreen()->bmp_mono.hdr,
                    LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_IDLE_NO_SHOW));
            }
        }
    }
}


//************************************************************************
//
// CLCDOutput::GetSoftButtonState
//
//************************************************************************

DWORD CLCDOutput::GetSoftButtonState(void)
{
    return IsOpened() ? m_dwButtonState : 0;
}


//************************************************************************
//
// CLCDOutput::HandleErrorFromAPI
//
//************************************************************************

HRESULT CLCDOutput::HandleErrorFromAPI(DWORD dwRes)
{
    switch(dwRes)
    {
        // all is well
    case ERROR_SUCCESS:
    case RPC_S_PROTOCOL_ERROR:
        break;
        // we lost our device
    case ERROR_DEVICE_NOT_CONNECTED:
        LCDUITRACE(_T("lgLcdAPI returned with ERROR_DEVICE_NOT_CONNECTED, closing device\n"));
        Close();
        break;
    default:
        LCDUITRACE(_T("lgLcdAPI returned with other error (0x%08x) closing device\n"));
        Close();
        // something else happened, such as LCDMon that was terminated
        break;
    }

    if(ERROR_SUCCESS != dwRes)
    {
        return MAKE_HRESULT(SEVERITY_ERROR, 0, dwRes);
    }

    return S_OK;
}


//************************************************************************
//
// CLCDOutput::DoesBitmapNeedUpdate
//
//************************************************************************

BOOL CLCDOutput::DoesBitmapNeedUpdate(lgLcdBitmap* pBitmap)
{
    UNREFERENCED_PARAMETER(pBitmap);
    // For now, always update
    return TRUE;
}


//************************************************************************
//
// CLCDOutput::OnPageExpired
//
//************************************************************************

void CLCDOutput::OnPageExpired(CLCDCollection* pScreen)
{
    UNREFERENCED_PARAMETER(pScreen);
}


//************************************************************************
//
// CLCDOutput::OnPageShown
//
//************************************************************************

void CLCDOutput::OnPageShown(CLCDCollection* pScreen)
{
    UNREFERENCED_PARAMETER(pScreen);
}


//************************************************************************
//
// CLCDOutput::OnEnteringIdle
//
//************************************************************************

void CLCDOutput::OnEnteringIdle(void)
{
}


//************************************************************************
//
// CLCDOutput::OnClosingDevice
//
//************************************************************************

void CLCDOutput::OnClosingDevice(int hDevice)
{
    UNREFERENCED_PARAMETER(hDevice);
}


//************************************************************************
//
// CLCDOutput::OnOpenedDevice
//
//************************************************************************

void CLCDOutput::OnOpenedDevice(int hDevice)
{
    UNREFERENCED_PARAMETER(hDevice);
}


//************************************************************************
//
// CLCDOutput::OnSoftButtonEvent
//
//************************************************************************

void CLCDOutput::OnSoftButtonEvent(DWORD dwButtonState)
{
    if (LGLCD_DEVICE_FAMILY_QVGA_BASIC == m_pGfx->GetFamily())
    {
        HandleButtonState(dwButtonState, LGLCDBUTTON_LEFT);
        HandleButtonState(dwButtonState, LGLCDBUTTON_RIGHT);
        HandleButtonState(dwButtonState, LGLCDBUTTON_OK);
        HandleButtonState(dwButtonState, LGLCDBUTTON_CANCEL);
        HandleButtonState(dwButtonState, LGLCDBUTTON_UP);
        HandleButtonState(dwButtonState, LGLCDBUTTON_DOWN);
        HandleButtonState(dwButtonState, LGLCDBUTTON_MENU);
    }
    else
    {
        HandleButtonState(dwButtonState, LGLCDBUTTON_BUTTON0);
        HandleButtonState(dwButtonState, LGLCDBUTTON_BUTTON1);
        HandleButtonState(dwButtonState, LGLCDBUTTON_BUTTON2);
        HandleButtonState(dwButtonState, LGLCDBUTTON_BUTTON3);
    }

    m_dwButtonState = dwButtonState;
}


//************************************************************************
//
// CLCDOutput::HandleButtonState
//
//************************************************************************

void CLCDOutput::HandleButtonState(DWORD dwButtonState, DWORD dwButton)
{
    if ( (m_dwButtonState & dwButton) && !(dwButtonState & dwButton) )
    {
        LCDUITRACE(_T("Button 0x%x released\n"), dwButton);
        OnLCDButtonUp(dwButton);
    }
    if ( !(m_dwButtonState & dwButton) && (dwButtonState & dwButton) )
    {
        LCDUITRACE(_T("Button 0x%x pressed\n"), dwButton);
        OnLCDButtonDown(dwButton);
    }
}


//************************************************************************
//
// CLCDOutput::OnLCDButtonDown
//
//************************************************************************

void CLCDOutput::OnLCDButtonDown(int nButton)
{
    if (m_pActivePage)
    {
        m_pActivePage->OnLCDButtonDown(nButton);
    }
}


//************************************************************************
//
// CLCDOutput::OnLCDButtonUp
//
//************************************************************************

void CLCDOutput::OnLCDButtonUp(int nButton)
{
    if (m_pActivePage)
    {
        m_pActivePage->OnLCDButtonUp(nButton);
    }
}


//************************************************************************
//
// CLCDOutput::GetDeviceId
//
//************************************************************************

int CLCDOutput::GetDeviceId(void)
{
    return m_hDevice;
}


//************************************************************************
//
// CLCDOutput::StopOpeningByDeviceType
//
//************************************************************************

void CLCDOutput::StopOpeningByDeviceType(void)
{
    m_OpenByTypeContext.deviceType = 0;
}


//************************************************************************
//
// CLCDOutput::HasBeenOpenedByDeviceType
//
//************************************************************************

BOOL CLCDOutput::HasBeenOpenedByDeviceType(void)
{
    return (0 != m_OpenByTypeContext.deviceType);
}


//** end of LCDOutput.cpp ************************************************
