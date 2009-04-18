//************************************************************************
//
// LCDOutput.cpp
//
// The CLCDOutput class manages LCD hardware enumeration and screen 
// management.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#include "../../DSUtil/SharedInclude.h"
#include <Afx.h>
#include "LCDOutput.h"

#pragma comment(lib, "lgLcd.lib")

// to keep track of clients that use multiple CLCDOutput instances
// within the same app
static LONG lInitCount = 0;

//************************************************************************
//
// CLCDOutput::CLCDOutput
//
//************************************************************************

CLCDOutput::CLCDOutput()
{
    m_pActiveScreen = NULL;
    m_bLocked = FALSE;
    m_hDevice = LGLCD_INVALID_DEVICE;
    m_hConnection = LGLCD_INVALID_CONNECTION;
    m_nPriority = LGLCD_PRIORITY_NORMAL;
    ZeroMemory(&m_lcdConnectCtxEx, sizeof(m_lcdConnectCtxEx));
    m_bDisplayLocked = FALSE;
    m_bSetAsForeground = FALSE;
    
    // Setup default device families
    m_dwDeviceFamiliesSupported = LGLCD_DEVICE_FAMILY_KEYBOARD_G15 | 
                        LGLCD_DEVICE_FAMILY_JACKBOX |
                        LGLCD_DEVICE_FAMILY_SPEAKERS_Z10;
    m_dwDeviceFamiliesSupportedReserved1 = 0; 

    m_pLastBitmap = DNew lgLcdBitmap160x43x1;
    ClearBitmap(m_pLastBitmap);
    // Allow the first update to go through
    m_bPriorityHasChanged = TRUE;
}

//************************************************************************
//
// CLCDOutput::~CLCDOutput
//
//************************************************************************

CLCDOutput::~CLCDOutput()
{
    delete m_pLastBitmap;
    m_pLastBitmap = NULL;
}


//************************************************************************
//
// CLCDOutput::Initialize
//
//************************************************************************

HRESULT CLCDOutput::Initialize()
{
    return Initialize((lgLcdConnectContext *)NULL, FALSE);
}


//************************************************************************
//
// CLCDOutput::Initialize
//
// NOTE: Initialize should always return S_OK
//************************************************************************

HRESULT CLCDOutput::Initialize(lgLcdConnectContext* pContext, BOOL bUseWindow)
{    

    UNREFERENCED_PARAMETER(bUseWindow);

    DWORD res = ERROR_SUCCESS;

    CLCDManager::Initialize();

    // initialize our screens
    LCD_MGR_LIST::iterator it = m_LCDMgrList.begin();
    while(it != m_LCDMgrList.end())
    {
        CLCDManager *pMgr = *it;
        LCDUIASSERT(NULL != pMgr);

        pMgr->Initialize();
        ++it;
    }

    // LCD Stuff
    LCDUIASSERT(lInitCount >= 0);
    if(1 == InterlockedIncrement(&lInitCount))
    {
        // need to call lgLcdInit once
        res = lgLcdInit();
        if (ERROR_SUCCESS != res)
        {
            InterlockedDecrement(&lInitCount);
            LCDUITRACE(_T("WARNING: lgLcdInit failed\n"));
            return E_FAIL;
        }
    }

    m_lcdConnectCtxEx.appFriendlyName = _T("My App");
    m_lcdConnectCtxEx.isPersistent = FALSE;
    m_lcdConnectCtxEx.isAutostartable = FALSE;
    m_lcdConnectCtxEx.connection = LGLCD_INVALID_CONNECTION;

    // Initialize the added version 3.0 API fields
    m_lcdConnectCtxEx.dwAppletCapabilitiesSupported = LGLCD_APPLET_CAP_BASIC;
    m_lcdConnectCtxEx.dwReserved1 = 0;
    m_lcdConnectCtxEx.onNotify.notificationCallback = NULL;
    m_lcdConnectCtxEx.onNotify.notifyContext = NULL;

    // if user passed in the context, fill it up
    if (NULL != pContext)
    {
        memcpy(&m_lcdConnectCtxEx, pContext, sizeof(lgLcdConnectContext));
    }

    return S_OK;
}

//************************************************************************
//
// CLCDOutput::Initialize
//
// NOTE: Initialize should always return S_OK
//************************************************************************

HRESULT CLCDOutput::Initialize(lgLcdConnectContextEx* pContextEx, BOOL bUseWindow)
{    

    UNREFERENCED_PARAMETER(bUseWindow);

    DWORD res = ERROR_SUCCESS;

    CLCDManager::Initialize();

    // initialize our screens
    LCD_MGR_LIST::iterator it = m_LCDMgrList.begin();
    while(it != m_LCDMgrList.end())
    {
        CLCDManager *pMgr = *it;
        LCDUIASSERT(NULL != pMgr);

        pMgr->Initialize();
        ++it;
    }

    // LCD Stuff
    LCDUIASSERT(lInitCount >= 0);
    if(1 == InterlockedIncrement(&lInitCount))
    {
        // need to call lgLcdInit once
        res = lgLcdInit();
        if (ERROR_SUCCESS != res)
        {
            InterlockedDecrement(&lInitCount);
            LCDUITRACE(_T("WARNING: lgLcdInit failed\n"));
            return E_FAIL;
        }
    }

    
    m_lcdConnectCtxEx.appFriendlyName = _T("My App");
    m_lcdConnectCtxEx.isPersistent = FALSE;
    m_lcdConnectCtxEx.isAutostartable = FALSE;
    m_lcdConnectCtxEx.connection = LGLCD_INVALID_CONNECTION;

    // if user passed in the context, fill it up
    if (NULL != pContextEx)
    {
        memcpy(&m_lcdConnectCtxEx, pContextEx, sizeof(lgLcdConnectContextEx));
    }

    return S_OK;
}

//************************************************************************
//
// CLCDOutput::Shutdown
//
//************************************************************************

void CLCDOutput::Shutdown(void)
{
    CloseAndDisconnect();
    if(0 == InterlockedDecrement(&lInitCount))
    {
        lgLcdDeInit();
    }
    LCDUIASSERT(lInitCount >= 0);
}


//************************************************************************
//
// CLCDOutput::Draw
//
//************************************************************************

HRESULT CLCDOutput::Draw()
{
    DWORD dwPriorityToUse = LGLCD_ASYNC_UPDATE(m_nPriority);
    
    if ( (NULL == m_pActiveScreen)              ||
         (LGLCD_INVALID_DEVICE == m_hDevice)    ||
         (LGLCD_PRIORITY_IDLE_NO_SHOW == dwPriorityToUse) )
    {
        // don't submit the bitmap
        return S_OK;
    }

    // Render the active screen
    m_pActiveScreen->Draw();
    
    // Get the active bitmap
    lgLcdBitmap160x43x1* pScreen = m_pActiveScreen->GetLCDScreen();

    // Only submit if the bitmap needs to be updated
    // (If the priority or bitmap have changed)
    DWORD res = ERROR_SUCCESS;
    if (DoesBitmapNeedUpdate(pScreen))
    {
        res = lgLcdUpdateBitmap(m_hDevice, &pScreen->hdr, dwPriorityToUse);
        HandleErrorFromAPI(res);
    }

    // read the soft buttons
    ReadButtons();

    return S_OK;
}


//************************************************************************
//
// CLCDOutput::Update
//
//************************************************************************

void CLCDOutput::Update(DWORD dwTimestamp)
{
    if (m_pActiveScreen)
    {
        m_pActiveScreen->Update(dwTimestamp);
    }

    // check for expiration
    if (m_pActiveScreen && m_pActiveScreen->HasExpired())
    {
        m_pActiveScreen = NULL;
        //m_nPriority = LGLCD_PRIORITY_FYI; -> needs to go so that if a 
		// program sets priority to LGLCD_PRIORITY_BACKGROUND, that 
		// priority sticks.

        OnScreenExpired(m_pActiveScreen);

        // Clear the bitmap
        ClearBitmap(m_pLastBitmap);

        // find the next active screen
        LCD_MGR_LIST::iterator it = m_LCDMgrList.begin();
        while(it != m_LCDMgrList.end())
        {
            CLCDManager *pMgr = *it;
            LCDUIASSERT(NULL != pMgr);

            if (!pMgr->HasExpired())
            {
                ActivateScreen(pMgr);
                //m_nPriority = LGLCD_PRIORITY_FYI;  -> needs to go so that if a 
				// program sets priority to LGLCD_PRIORITY_BACKGROUND, that 
				// priority sticks.
                break;
            }

            ++it;
        }

        // if no screen found, empty the screen at idle priority
        if (NULL == m_pActiveScreen)
        {
            if (LGLCD_INVALID_DEVICE != m_hDevice)
            {
                lgLcdUpdateBitmap(m_hDevice, &CLCDManager::GetLCDScreen()->hdr,
                    LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_IDLE_NO_SHOW));
            }
        }
    }

    // check for lcd devices
    if (LGLCD_INVALID_DEVICE == m_hDevice)
    {
        EnumerateDevices();
    }
}


//************************************************************************
//
// CLCDOutput::HasHardwareChanged
//
//************************************************************************

BOOL CLCDOutput::HasHardwareChanged(void)
{
    if(LGLCD_INVALID_DEVICE != m_hDevice)
    {
        // ping to see whether we're still alive
        DWORD dwButtonState = 0;
        
        DWORD res = lgLcdReadSoftButtons(m_hDevice, &dwButtonState);

        HandleErrorFromAPI(res);
    }

    // check for lcd devices
    if (LGLCD_INVALID_DEVICE == m_hDevice)
    {
        EnumerateDevices();
    }
    else
    {
        // we still have our device;
        return FALSE;
    }

    // we got a new device
    return LGLCD_INVALID_DEVICE != m_hDevice;
}


//************************************************************************
//
// CLCDOutput::GetLCDScreen
//
//************************************************************************

lgLcdBitmap160x43x1 *CLCDOutput::GetLCDScreen(void)
{
    return m_pActiveScreen ? m_pActiveScreen->GetLCDScreen() : CLCDManager::GetLCDScreen();
}


//************************************************************************
//
// CLCDOutput::GetBitmapInfo
//
//************************************************************************

BITMAPINFO *CLCDOutput::GetBitmapInfo(void)
{
    return m_pActiveScreen ? m_pActiveScreen->GetBitmapInfo() : CLCDManager::GetBitmapInfo();
}


//************************************************************************
//
// CLCDOutput::ReadButtons
//
//************************************************************************

void CLCDOutput::ReadButtons()
{
    if(IsOpened())
    {
        DWORD dwButtonState = 0;
        
        DWORD res = lgLcdReadSoftButtons(m_hDevice, &dwButtonState);
        if (ERROR_SUCCESS != res)
        {
            LCDUITRACE(_T("lgLcdReadSoftButtons failed: unplug?\n"));
            HandleErrorFromAPI(res);
        }
        
        if (m_dwButtonState == dwButtonState)
            return;
        
        // handle the buttons
        HandleButtonState(dwButtonState, LGLCDBUTTON_BUTTON0);
        HandleButtonState(dwButtonState, LGLCDBUTTON_BUTTON1);
        HandleButtonState(dwButtonState, LGLCDBUTTON_BUTTON2);
        HandleButtonState(dwButtonState, LGLCDBUTTON_BUTTON3);
        
        m_dwButtonState = dwButtonState;
    }
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
    if (m_pActiveScreen)
    {
        m_pActiveScreen->OnLCDButtonDown(nButton);
    }
}


//************************************************************************
//
// CLCDOutput::OnLCDButtonUp
//
//************************************************************************

void CLCDOutput::OnLCDButtonUp(int nButton)
{
    if (m_pActiveScreen)
    {
        m_pActiveScreen->OnLCDButtonUp(nButton);
    }
}


//************************************************************************
//
// CLCDOutput::ActivateScreen
//
//************************************************************************

void CLCDOutput::ActivateScreen(CLCDManager* pScreen)
{
    if (m_bLocked)
        return;
    m_pActiveScreen = pScreen;
}


//************************************************************************
//
// CLCDOutput::LockScreen
//
//************************************************************************

void CLCDOutput::LockScreen(CLCDManager* pScreen)
{
    if (m_bLocked)
        return;

    m_pActiveScreen = pScreen;
    m_bLocked = TRUE;
}


//************************************************************************
//
// CLCDOutput::UnlockScreen
//
//************************************************************************

void CLCDOutput::UnlockScreen()
{
    m_bLocked = FALSE;
    m_pActiveScreen = NULL;
}


//************************************************************************
//
// CLCDOutput::IsLocked
//
//************************************************************************

BOOL CLCDOutput::IsLocked()
{
    return m_bLocked;
}


//************************************************************************
//
// CLCDOutput::AddScreen
//
//************************************************************************

void CLCDOutput::AddScreen(CLCDManager* pScreen)
{
    m_LCDMgrList.push_back(pScreen);
}


//************************************************************************
//
// CLCDOutput::AnyDeviceOfThisFamilyPresent
//
//************************************************************************

BOOL CLCDOutput::AnyDeviceOfThisFamilyPresent(DWORD dwDeviceFamilyWanted, DWORD dwReserved1)
{
    lgLcdDeviceDescEx   descEx;

    if (LGLCD_INVALID_CONNECTION == m_hConnection)
    {
        if (ERROR_SUCCESS == lgLcdConnectEx(&m_lcdConnectCtxEx))
        {
            // make sure we don't work with a stale device handle
            m_hConnection = m_lcdConnectCtxEx.connection;
            m_hDevice = LGLCD_INVALID_CONNECTION;
        }
        else
        {
            return FALSE;
        }
    }

    // Setup the device family to use next time
    lgLcdSetDeviceFamiliesToUse(m_hConnection, dwDeviceFamilyWanted, dwReserved1);

    ZeroMemory(&descEx, sizeof(lgLcdDeviceDescEx));
    DWORD res = ERROR_SUCCESS;
    
    res = lgLcdEnumerateEx(m_hConnection, 0, &descEx);

    if (ERROR_SUCCESS != res)
    {
        if(ERROR_NO_MORE_ITEMS != res)
        {
            // something happened. Let's close this.
            CloseAndDisconnect();
            return FALSE;
        }

        // Go back to the previous device family we were using
        lgLcdSetDeviceFamiliesToUse(m_hConnection, m_dwDeviceFamiliesSupported, 
                                    m_dwDeviceFamiliesSupportedReserved1);

        return FALSE;
    }

    // Go back to what was being used
    lgLcdSetDeviceFamiliesToUse(m_hConnection, m_dwDeviceFamiliesSupported, 
                                m_dwDeviceFamiliesSupportedReserved1);

    return TRUE;
}

//************************************************************************
//
// CLCDOutput::SetDeviceFamiliesSupported
//
//************************************************************************

void CLCDOutput::SetDeviceFamiliesSupported(DWORD dwDeviceFamiliesSupported, DWORD dwReserved1)
{
    m_dwDeviceFamiliesSupported = dwDeviceFamiliesSupported;
    m_dwDeviceFamiliesSupportedReserved1 = dwReserved1;

    if (LGLCD_INVALID_CONNECTION == m_hConnection)
    {
        if (ERROR_SUCCESS == lgLcdConnectEx(&m_lcdConnectCtxEx))
        {
            // make sure we don't work with a stale device handle
            m_hConnection = m_lcdConnectCtxEx.connection;
            m_hDevice = LGLCD_INVALID_CONNECTION;
        }
        else
        {
            return;
        }
    }

    // close the lcd device before we open up another
    if (LGLCD_INVALID_DEVICE != m_hDevice)
    {
        lgLcdClose(m_hDevice);
        m_hDevice = LGLCD_INVALID_DEVICE;

    }
    
    // Setup the device family to use next time
    lgLcdSetDeviceFamiliesToUse(m_hConnection, m_dwDeviceFamiliesSupported, m_dwDeviceFamiliesSupportedReserved1);
}

//************************************************************************
//
// CLCDOutput::EnumerateDevices
//
//************************************************************************

void CLCDOutput::EnumerateDevices()
{
    lgLcdDeviceDescEx descEx;

    if (LGLCD_INVALID_CONNECTION == m_hConnection)
    {
        if (ERROR_SUCCESS == lgLcdConnectEx(&m_lcdConnectCtxEx))
        {
            // make sure we don't work with a stale device handle
            m_hConnection = m_lcdConnectCtxEx.connection;
            m_hDevice = LGLCD_INVALID_CONNECTION;
        }
        else
        {
            return;
        }
    }

    // close the lcd device before we open up another
    if (LGLCD_INVALID_DEVICE != m_hDevice)
    {
        lgLcdClose(m_hDevice);
        m_hDevice = LGLCD_INVALID_DEVICE;

    }
    
    // Setup the device family to use next time
    lgLcdSetDeviceFamiliesToUse(m_hConnection, m_dwDeviceFamiliesSupported, m_dwDeviceFamiliesSupportedReserved1);
 
    ZeroMemory(&descEx, sizeof(lgLcdDeviceDescEx));
    DWORD res = ERROR_SUCCESS;
    
    res = lgLcdEnumerateEx(m_hConnection, 0, &descEx);
    if (ERROR_SUCCESS != res)
    {
        if(ERROR_NO_MORE_ITEMS != res)
        {
            // something happened. Let's close this.
            CloseAndDisconnect();
        }
        return;
    }
// ERROR_NO_MORE_ITEMS
    lgLcdOpenContext open_ctx;
    ZeroMemory(&open_ctx, sizeof(open_ctx));

    open_ctx.connection = m_hConnection;
    open_ctx.index = 0;
    res = lgLcdOpen(&open_ctx);
    if (ERROR_SUCCESS != res)
        return;
    m_hDevice = open_ctx.device;
    m_dwButtonState = 0;

    // restores
    SetAsForeground(m_bSetAsForeground);
}


//************************************************************************
//
// CLCDOutput::HandleErrorFromAPI
//
//************************************************************************
void CLCDOutput::HandleErrorFromAPI(DWORD dwRes)
{
    switch(dwRes)
    {
        // all is well
    case ERROR_SUCCESS:
        break;
        // we lost our device
    case ERROR_DEVICE_NOT_CONNECTED:
        LCDUITRACE(_T("lgLcdAPI returned with ERROR_DEVICE_NOT_CONNECTED, closing device\n"));
        OnClosingDevice(m_hDevice);
        break;
    default:
        LCDUITRACE(_T("lgLcdAPI returned with other error (0x%08x) closing device and connection\n"));
        OnClosingDevice(m_hDevice);
        OnDisconnecting(m_hConnection);
        // something else happened, such as LCDMon that was terminated
        break;
    }
}

//************************************************************************
//
// CLCDOutput::SetScreenPriority
//
//************************************************************************
void CLCDOutput::SetScreenPriority(DWORD priority)
{
    if (priority == m_nPriority)
    {
        // Nothing to do
        return;
    }

    // Clear the bitmap
    ClearBitmap(m_pLastBitmap);

    m_nPriority = priority;
    m_bPriorityHasChanged = TRUE;

    if (LGLCD_PRIORITY_IDLE_NO_SHOW == m_nPriority)
    {
        // send an empty bitmap at idle priority
        if (LGLCD_INVALID_DEVICE != m_hDevice)
        {
            lgLcdUpdateBitmap(m_hDevice, &m_pLastBitmap->hdr,
                LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_IDLE_NO_SHOW));
        }
    }
}

//************************************************************************
//
// CLCDOutput::GetScreenPriority
//
//************************************************************************
DWORD CLCDOutput::GetScreenPriority()
{
    return m_nPriority;
}

//************************************************************************
//
// CLCDOutput::GetDeviceHandle
//
//************************************************************************
INT CLCDOutput::GetDeviceHandle()
{
    return m_hDevice;
}

//************************************************************************
//
// CLCDOutput::CloseAndDisconnect
//
//************************************************************************

void CLCDOutput::CloseAndDisconnect()
{
    OnClosingDevice(m_hDevice);
    OnDisconnecting(m_hConnection);
}


//************************************************************************
//
// CLCDOutput::OnScreenExpired
//
//************************************************************************

void CLCDOutput::OnScreenExpired(CLCDManager* pScreen)
{
    UNREFERENCED_PARAMETER(pScreen);
    UnlockScreen();
}


//************************************************************************
//
// CLCDOutput::OnClosingDevice
//
//************************************************************************

void CLCDOutput::OnClosingDevice(int hDevice)
{
    UNREFERENCED_PARAMETER(hDevice);
    LCDUITRACE(_T("CLCDOutput::OnClosingDevice\n"));
    if (IsOpened())
    {
        lgLcdClose(m_hDevice);
        m_hDevice = LGLCD_INVALID_DEVICE;
    }
    ClearBitmap(m_pLastBitmap);
}

//************************************************************************
//
// CLCDOutput::OnDisconnecting
//
//************************************************************************

void CLCDOutput::OnDisconnecting(int hConnection)
{
    UNREFERENCED_PARAMETER(hConnection);
    LCDUITRACE(_T("CLCDOutput::OnDisconnecting\n"));
    // let's hope our device is already gone
    LCDUIASSERT(!IsOpened());

    if (LGLCD_INVALID_CONNECTION != m_hConnection)
    {
        lgLcdDisconnect(m_hConnection);
        m_hConnection = LGLCD_INVALID_CONNECTION;
        ZeroMemory(m_pLastBitmap, sizeof(lgLcdBitmap160x43x1));
    }
}

//************************************************************************
//
// CLCDOutput::IsOpened
//
//************************************************************************

BOOL CLCDOutput::IsOpened()
{
	return (LGLCD_INVALID_DEVICE != m_hDevice);
}


//************************************************************************
//
// CLCDOutput::SetAsForeground
//
//************************************************************************

void CLCDOutput::SetAsForeground(BOOL bSetAsForeground)
{
    /*DWORD dwSet = */bSetAsForeground ? LGLCD_LCD_FOREGROUND_APP_YES : LGLCD_LCD_FOREGROUND_APP_NO;
    m_bSetAsForeground = bSetAsForeground;
    if (LGLCD_INVALID_DEVICE != m_hDevice)
    {
        lgLcdSetAsLCDForegroundApp(m_hDevice, bSetAsForeground);
    }
}


//************************************************************************
//
// CLCDOutput::DoesBitmapNeedUpdate
//
//************************************************************************

BOOL CLCDOutput::DoesBitmapNeedUpdate(lgLcdBitmap160x43x1* pCurrentBitmap)
{
    // The bitmap is different from the last one sent
    // send the bitmap
    BOOL bBitmapChanged = 0 != memcmp(pCurrentBitmap, m_pLastBitmap, sizeof(lgLcdBitmap160x43x1));
    BOOL bPriorityChanged = m_bPriorityHasChanged;

    if (bBitmapChanged)
    {
        LCDUITRACE(_T("Resubmitting bitmap (bitmap changed)\n"));
    }
    else if (bPriorityChanged)
    {
        LCDUITRACE(_T("Resubmitting bitmap (priority changed)\n"));
    }

    // Save the current bitmap
    memcpy(m_pLastBitmap, pCurrentBitmap, sizeof(lgLcdBitmap160x43x1));
    // Reset the priority change
    m_bPriorityHasChanged = FALSE;

    return (bBitmapChanged || bPriorityChanged);
}


//************************************************************************
//
// CLCDOutput::ClearBitmap
//
//************************************************************************

void CLCDOutput::ClearBitmap(lgLcdBitmap160x43x1* pCurrentBitmap)
{
    LCDUIASSERT(NULL != pCurrentBitmap);
    if (pCurrentBitmap)
    {
        pCurrentBitmap->hdr.Format = LGLCD_BMP_FORMAT_160x43x1;
        ZeroMemory(pCurrentBitmap->pixels, sizeof(pCurrentBitmap->pixels));
    }
}

//** end of LCDOutput.cpp ************************************************
