//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDConnection.cpp
//
// The CLCDConnection class manages connections to all LCD devices
// including color and monochrome
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"

// Add the lgLcd.lib to the linker

// #pragma comment(lib, "lgLcd.lib")

// to keep track of clients that use multiple CLCDOutput instances
// within the same app
LONG CLCDConnection::g_lInitCount = 0;

//************************************************************************
//
// CLCDConnection::CLCDConnection
//
//************************************************************************

CLCDConnection::CLCDConnection(void)
{
    m_hConnection = LGLCD_INVALID_CONNECTION;

    ZeroMemory(&m_AppletState, sizeof(m_AppletState));

    m_plcdSoftButtonsChangedCtx = NULL;

    InitializeCriticalSection(&m_csCallback);
}


//************************************************************************
//
// CLCDConnection::~CLCDConnection
//
//************************************************************************

CLCDConnection::~CLCDConnection(void)
{
    Disconnect();

    FreeMonoOutput();
    FreeColorOutput();

    if (m_AppletState.Mono.pGfx)
    {
        delete m_AppletState.Mono.pGfx;
        m_AppletState.Mono.pGfx = NULL;
    }
    if (m_AppletState.Color.pGfx)
    {
        delete m_AppletState.Color.pGfx;
        m_AppletState.Color.pGfx = NULL;
    }
    DeleteCriticalSection(&m_csCallback);
}


//************************************************************************
//
// CLCDConnection::Initialize
//
//************************************************************************

BOOL CLCDConnection::Initialize(lgLcdConnectContext & ConnectContext,
                                lgLcdSoftbuttonsChangedContext* pSoftButtonChangedContext)
{
    // Assume only BW is supported
    m_AppletState.Mono.pOutput = new CLCDOutput();
    m_AppletState.Mono.pGfx = new CLCDGfxMono();
    m_AppletState.Mono.pGfx->Initialize();
    m_AppletState.Mono.pOutput->SetGfx(m_AppletState.Mono.pGfx);

    lgLcdConnectContextEx ConnectContextEx;
    memset(&ConnectContextEx, 0, sizeof(ConnectContextEx));
    ConnectContextEx.appFriendlyName = ConnectContext.appFriendlyName;
    ConnectContextEx.isPersistent = ConnectContext.isPersistent;
    ConnectContextEx.isAutostartable = ConnectContext.isAutostartable;
    ConnectContextEx.connection = LGLCD_INVALID_CONNECTION;
    ConnectContextEx.onConfigure = ConnectContext.onConfigure;
    ConnectContextEx.dwAppletCapabilitiesSupported = LGLCD_APPLET_CAP_BW;

    return Initialize(ConnectContextEx, pSoftButtonChangedContext);
}


//************************************************************************
//
// CLCDConnection::Initialize
//
//************************************************************************

BOOL CLCDConnection::Initialize(lgLcdConnectContextEx & ConnectContext,
                                lgLcdSoftbuttonsChangedContext* pSoftButtonChangedContext)
{
    DWORD res = ERROR_SUCCESS;

    if ((LGLCD_APPLET_CAP_BASIC == ConnectContext.dwAppletCapabilitiesSupported) ||
         ConnectContext.dwAppletCapabilitiesSupported & LGLCD_APPLET_CAP_BW)
    {
        m_AppletState.Mono.pOutput = AllocMonoOutput();
        m_AppletState.Mono.pGfx = new CLCDGfxMono();
        m_AppletState.Mono.pGfx->Initialize();
        m_AppletState.Mono.pOutput->SetGfx(m_AppletState.Mono.pGfx);
    }

    if (ConnectContext.dwAppletCapabilitiesSupported & LGLCD_APPLET_CAP_QVGA)
    {
        m_AppletState.Color.pOutput = AllocColorOutput();
        m_AppletState.Color.pGfx = new CLCDGfxColor();
        m_AppletState.Color.pGfx->Initialize();
        m_AppletState.Color.pOutput->SetGfx(m_AppletState.Color.pGfx);
    }

    //Assure we only call the lib's init once
    LCDUIASSERT(g_lInitCount >= 0);
    if(1 == InterlockedIncrement(&g_lInitCount))
    {
        // need to call lgLcdInit once
        res = lgLcdInit();
        if (ERROR_SUCCESS != res)
        {
            InterlockedDecrement(&g_lInitCount);
            LCDUITRACE(_T("WARNING: lgLcdInit failed\n"));
            return FALSE;
        }
    }

    memset(&m_lcdConnectCtxEx, 0, sizeof(m_lcdConnectCtxEx));
    m_lcdConnectCtxEx.appFriendlyName = NULL;
    m_lcdConnectCtxEx.isPersistent = FALSE;
    m_lcdConnectCtxEx.isAutostartable = FALSE;
    m_lcdConnectCtxEx.connection = LGLCD_INVALID_CONNECTION;

    // Initialize the added version 3.0 API fields
    m_lcdConnectCtxEx.dwAppletCapabilitiesSupported = LGLCD_APPLET_CAP_BASIC;
    m_lcdConnectCtxEx.dwReserved1 = 0;
    m_lcdConnectCtxEx.onNotify.notificationCallback = NULL;
    m_lcdConnectCtxEx.onNotify.notifyContext = NULL;

    memcpy(&m_lcdConnectCtxEx, &ConnectContext, sizeof(lgLcdConnectContextEx));

    m_plcdSoftButtonsChangedCtx = pSoftButtonChangedContext;

    // Add internal callback handlers if not specified
    if (NULL == m_lcdConnectCtxEx.onNotify.notificationCallback)
    {
        m_lcdConnectCtxEx.onNotify.notificationCallback = _OnNotificationCallback;
        m_lcdConnectCtxEx.onNotify.notifyContext = this;
    }

    Connect();

    if (LGLCD_INVALID_CONNECTION != m_hConnection)
    {
        // At this point, the asynchronous events have already been sent.
        // Let's allow a small amount of time for the callback thread to insert the events 
        // into the shared queue.
        // If you don't care about knowing about devices after Initialize
        // completes, you can remove these 2 lines
        Sleep(50);
        Update();
    }

    return TRUE;
}


//************************************************************************
//
// CLCDConnection::Shutdown
//
//************************************************************************

void CLCDConnection::Shutdown(void)
{
    Disconnect();

    if(0 == InterlockedDecrement(&g_lInitCount))
    {
        lgLcdDeInit();
    }

    m_plcdSoftButtonsChangedCtx = NULL;
}


//************************************************************************
//
// CLCDConnection::Connect
// 
// This will attempt a connection to LCDMon
//************************************************************************

void CLCDConnection::Connect(void)
{
    //Close previous connections
    Disconnect();

    if (LGLCD_INVALID_CONNECTION == m_hConnection)
    {
        DWORD retval = lgLcdConnectEx(&m_lcdConnectCtxEx);
        if (ERROR_SUCCESS == retval)
        {
            m_hConnection = m_lcdConnectCtxEx.connection;
        }
        else
        {
            if( ERROR_SERVICE_NOT_ACTIVE == retval )
            {
                LCDUITRACE( _T("lgLcdConnectEx ---> ERROR_INVALID_PARAMETER\n") );
                m_hConnection = LGLCD_INVALID_CONNECTION;
            }

            m_hConnection = LGLCD_INVALID_CONNECTION;
        }
    }
}


//************************************************************************
//
// CLCDConnection::Disconnect
//
//************************************************************************

void CLCDConnection::Disconnect(void)
{   
    //Close your devices, too
    if (m_AppletState.Color.pOutput)
    {
        m_AppletState.Color.pOutput->Close();
    }
    if (m_AppletState.Mono.pOutput)
    {
        m_AppletState.Mono.pOutput->Close();
    }

    if( LGLCD_INVALID_CONNECTION != m_hConnection )
    {
        lgLcdDisconnect(m_hConnection);
        m_hConnection = LGLCD_INVALID_CONNECTION;
    }
}


//************************************************************************
//
// CLCDConnection::IsConnected
//
//************************************************************************

BOOL CLCDConnection::IsConnected(void)
{
    return LGLCD_INVALID_CONNECTION != m_hConnection;
}


//************************************************************************
//
// CLCDConnection::GetConnectionId
//
//************************************************************************

int CLCDConnection::GetConnectionId(void)
{
    return m_hConnection;
}


//************************************************************************
//
// CLCDConnection::ColorOutput
//
//************************************************************************

CLCDOutput *CLCDConnection::ColorOutput(void)
{
    return m_AppletState.Color.pOutput;
}


//************************************************************************
//
// CLCDConnection::MonoOutput
//
//************************************************************************

CLCDOutput *CLCDConnection::MonoOutput(void)
{
    return m_AppletState.Mono.pOutput;
}


//************************************************************************
//
// CLCDConnection::_OnNotificationCallback
//
//************************************************************************

DWORD CALLBACK CLCDConnection::_OnNotificationCallback(
    IN int connection,
    IN const PVOID pContext,
    IN DWORD notificationCode,
    IN DWORD notifyParm1,
    IN DWORD notifyParm2,
    IN DWORD notifyParm3,
    IN DWORD notifyParm4)
{

    UNREFERENCED_PARAMETER(connection);
    UNREFERENCED_PARAMETER(notifyParm3);
    UNREFERENCED_PARAMETER(notifyParm4);

    CLCDConnection* pThis = (CLCDConnection*)pContext;
    
    CB_EVENT Event;
    memset(&Event, 0, sizeof(Event));
    EnterCriticalSection(&pThis->m_csCallback);
    Event.Type = CBT_NOTIFICATION;
    Event.CallbackCode = notificationCode;
    Event.CallbackParam1 = notifyParm1;
    Event.CallbackParam2 = notifyParm2;
    pThis->m_CallbackEventQueue.push(Event);
    LeaveCriticalSection(&pThis->m_csCallback);

    pThis->OnCallbackEvent();

    return 0;
}


//************************************************************************
//
// CLCDConnection::_OnSoftButtonsCallback
//
//************************************************************************

DWORD CALLBACK CLCDConnection::_OnSoftButtonsCallback(
    IN int device,
    IN DWORD dwButtons,
    IN const PVOID pContext)
{
    CLCDConnection* pThis = (CLCDConnection*)pContext;

    CB_EVENT Event;
    memset(&Event, 0, sizeof(Event));
    EnterCriticalSection(&pThis->m_csCallback);
    Event.Type = CBT_BUTTON;
    Event.CallbackCode = device;
    Event.CallbackParam1 = dwButtons;
    pThis->m_CallbackEventQueue.push(Event);
    LeaveCriticalSection(&pThis->m_csCallback);

    pThis->OnCallbackEvent();

    return 0;
}


//************************************************************************
//
// CLCDConnection::_OnConfigureCallback
//
//************************************************************************

DWORD CALLBACK CLCDConnection::_OnConfigureCallback(
    IN int connection,
    IN const PVOID pContext)
{
    UNREFERENCED_PARAMETER(connection);

    CLCDConnection* pThis = (CLCDConnection*)pContext;

    CB_EVENT Event;
    memset(&Event, 0, sizeof(Event));
    EnterCriticalSection(&pThis->m_csCallback);
    Event.Type = CBT_CONFIG;
    pThis->m_CallbackEventQueue.push(Event);
    LeaveCriticalSection(&pThis->m_csCallback);

    return 0;
}


//************************************************************************
//
// CLCDConnection::GetNextCallbackEvent
//
//************************************************************************

BOOL CLCDConnection::GetNextCallbackEvent(CB_EVENT& rEvent)
{
    memset(&rEvent, 0, sizeof(rEvent));
    EnterCriticalSection(&m_csCallback);
    if(0 < m_CallbackEventQueue.size())
    {
        std::swap(rEvent, m_CallbackEventQueue.front());
        m_CallbackEventQueue.pop();
        LeaveCriticalSection(&m_csCallback);
        return TRUE;
    }
    LeaveCriticalSection(&m_csCallback);
    return FALSE;
}


//************************************************************************
//
// CLCDConnection::Update
//
//************************************************************************

void CLCDConnection::Update(void)
{
    // If we're not connected, connect
    if (LGLCD_INVALID_CONNECTION == m_hConnection)
    {
        Connect();
    }

    // Get events
    CB_EVENT Event;
    while(GetNextCallbackEvent(Event))
    {
        switch(Event.Type)
        {
        case CBT_NOTIFICATION:
            OnNotification(Event.CallbackCode, Event.CallbackParam1);
            break;

        case CBT_CONFIG:
            OnConfigure();
            break;

        case CBT_BUTTON:
            OnSoftButtonEvent(Event.CallbackCode, Event.CallbackParam1);
            break;

        default:
            break;
        }
    }

    // For each display type
    for (int i = 0; i < 2; i++)
    {
        LCD_DEVICE_STATE* pDevice = (i == 0) ? &m_AppletState.Mono : &m_AppletState.Color;

        if (NULL == pDevice->pOutput)
        {
            // Output not supported
            continue;
        }

        if (pDevice->pOutput->IsOpened())
        {
            pDevice->pOutput->OnUpdate(GetTickCount());
            pDevice->pOutput->OnDraw();
        }

        // If the device is closed, but it was opened by OpenByType(),
        // we can try to open it again...
        if (!pDevice->pOutput->IsOpened() && pDevice->pOutput->HasBeenOpenedByDeviceType())
        {
            pDevice->pOutput->ReOpenDeviceType();
        }
    }
}


//************************************************************************
//
// CLCDConnection::OnConfigure
//
//************************************************************************

void CLCDConnection::OnConfigure(void)
{
    LCDUITRACE(_T("OnConfigure\n"));
}


//************************************************************************
//
// CLCDConnection::OnDeviceArrival
//
//************************************************************************

void CLCDConnection::OnDeviceArrival(DWORD dwDisplayType)
{
    LCD_DEVICE_STATE* pDevice = NULL;

    switch(dwDisplayType)
    {
    case LGLCD_DEVICE_BW:
        pDevice = &m_AppletState.Mono;
        break;

    case LGLCD_DEVICE_QVGA:
        pDevice = &m_AppletState.Color;
        break;
        
    default:
        LCDUITRACE(_T("Unhandled DisplayType in OnDeviceArrival()!\n"));
        return;
    }

    // Ensure that we have a valid output
    if (NULL == pDevice->pOutput)
    {
        LCDUITRACE(_T("Device arrival on unsupported device\n"));
        return;
    }

    lgLcdOpenByTypeContext OpenCtx;
    memset(&OpenCtx, 0, sizeof(OpenCtx));

    OpenCtx.connection = m_hConnection;
    OpenCtx.deviceType = dwDisplayType;
    OpenCtx.onSoftbuttonsChanged.softbuttonsChangedCallback = _OnSoftButtonsCallback;
    OpenCtx.onSoftbuttonsChanged.softbuttonsChangedContext = this;
    OpenCtx.device = LGLCD_INVALID_DEVICE;

    // If user has specified the soft button context, allow user to override
    if (NULL != m_plcdSoftButtonsChangedCtx)
    {
        OpenCtx.onSoftbuttonsChanged = *m_plcdSoftButtonsChangedCtx;
    }

    pDevice->pOutput->OpenByType(OpenCtx);
}


//************************************************************************
//
// CLCDConnection::OnDeviceRemoval
//
//************************************************************************

void CLCDConnection::OnDeviceRemoval(DWORD dwDisplayType)
{
    LCD_DEVICE_STATE* pDevice = NULL;

    switch(dwDisplayType)
    {
    case LGLCD_DEVICE_BW:
        pDevice = &m_AppletState.Mono;
        break;

    case LGLCD_DEVICE_QVGA:
        pDevice = &m_AppletState.Color;
        break;

    default:
        LCDUITRACE(_T("Unhandled DisplayType in OnDeviceArrival()!\n"));
        return;
    }

    if (pDevice->pOutput)
    {
        pDevice->pOutput->StopOpeningByDeviceType();
        pDevice->pOutput->Close();
    }
}


//************************************************************************
//
// CLCDConnection::OnAppletEnabled
//
//************************************************************************

void CLCDConnection::OnAppletEnabled(void)
{
}


//************************************************************************
//
// CLCDConnection::OnAppletDisabled
//
//************************************************************************

void CLCDConnection::OnAppletDisabled(void)
{
}


//************************************************************************
//
// CLCDConnection::OnNotification
//
//************************************************************************

void CLCDConnection::OnNotification(DWORD dwNotification, DWORD dwParam1)
{
    switch(dwNotification)
    {
    case LGLCD_NOTIFICATION_DEVICE_ARRIVAL:
        LCDUITRACE(_T("LGLCD_NOTIFICATION_DEVICE_ARRIVAL\n"));
        OnDeviceArrival(dwParam1);
        break;

    case LGLCD_NOTIFICATION_DEVICE_REMOVAL:
        LCDUITRACE(_T("LGLCD_NOTIFICATION_DEVICE_REMOVAL\n"));
        OnDeviceRemoval(dwParam1);
        break;

    case LGLCD_NOTIFICATION_CLOSE_CONNECTION:
        LCDUITRACE(_T("LGLCD_NOTIFICATION_CLOSE_CONNECTION\n"));
        Disconnect();
        break;

    case LGLCD_NOTIFICATION_APPLET_DISABLED:
        LCDUITRACE(_T("LGLCD_NOTIFICATION_APPLET_DISABLED\n"));
        OnAppletDisabled();
        break;

    case LGLCD_NOTIFICATION_APPLET_ENABLED:
        LCDUITRACE(_T("LGLCD_NOTIFICATION_APPLET_ENABLED\n"));
        OnAppletEnabled();
        break;

    case LGLCD_NOTIFICATION_TERMINATE_APPLET:
        break;
    }
}


//************************************************************************
//
// CLCDConnection::OnSoftButtonEvent
//
//************************************************************************

void CLCDConnection::OnSoftButtonEvent(int nDeviceId, DWORD dwButtonState)
{
    // Forward this to the appropriate display
    if (m_AppletState.Mono.pOutput && (nDeviceId == m_AppletState.Mono.pOutput->GetDeviceId()))
    {
        m_AppletState.Mono.pOutput->OnSoftButtonEvent(dwButtonState);
    }
    else if (m_AppletState.Color.pOutput && (nDeviceId == m_AppletState.Color.pOutput->GetDeviceId()))
    {
        m_AppletState.Color.pOutput->OnSoftButtonEvent(dwButtonState);
    }
}


//************************************************************************
//
// CLCDConnection::AllocMonoOutput
//
//************************************************************************

CLCDOutput* CLCDConnection::AllocMonoOutput(void)
{
    return new CLCDOutput();
}


//************************************************************************
//
// CLCDConnection::AllocColorOutput
//
//************************************************************************

CLCDOutput* CLCDConnection::AllocColorOutput(void)
{
    return new CLCDOutput();
}


//************************************************************************
//
// CLCDConnection::FreeMonoOutput
//
//************************************************************************

void CLCDConnection::FreeMonoOutput(void)
{
    if (NULL != m_AppletState.Mono.pOutput)
    {
        delete m_AppletState.Mono.pOutput;
        m_AppletState.Mono.pOutput = NULL;
    }
}


//************************************************************************
//
// CLCDConnection::FreeColorOutput
//
//************************************************************************

void CLCDConnection::FreeColorOutput(void)
{
    if (NULL != m_AppletState.Color.pOutput)
    {
        delete m_AppletState.Color.pOutput;
        m_AppletState.Color.pOutput = NULL;
    }
}

//** end of LCDConnection.cpp ********************************************
