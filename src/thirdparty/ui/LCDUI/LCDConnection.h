//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDConnection.h
//
// The CLCDConnection class manages connections to all LCD devices
// including color and monochrome
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#ifndef __LCDCONNECTION_H__
#define __LCDCONNECTION_H__

#include "LCDOutput.h"
#include "LCDGfxMono.h"
#include "LCDGfxColor.h"
#include <queue>

// LCD device state
typedef struct LCD_DEVICE_STATE
{
    CLCDOutput* pOutput;
    CLCDGfxBase* pGfx;

}LCD_DEVICE_STATE;

// Applet state
typedef struct APPLET_STATE
{
    LCD_DEVICE_STATE Color;
    LCD_DEVICE_STATE Mono;
    BOOL isEnabled;

}APPLET_STATE;

#define LGLCD_INVALID_DEVICE_COOKIE     (0)


class CLCDConnection
{
public:
    CLCDConnection(void);
    virtual ~CLCDConnection(void);

    //This will attempt to open the library
    //Call this function before Connect()
    virtual BOOL Initialize(lgLcdConnectContext & ConnectContext,
                            lgLcdSoftbuttonsChangedContext* pSoftButtonChangedContext = NULL);
    virtual BOOL Initialize(lgLcdConnectContextEx & ConnectContext,
                            lgLcdSoftbuttonsChangedContext* pSoftButtonChangedContext = NULL);

    //This will close your library
    virtual void Shutdown(void);
    
    //Checks if there is a valid connection
    virtual BOOL IsConnected(void);
    virtual int  GetConnectionId(void);

    // Call this function every game frame
    virtual void Update(void);

    // Add your controls to the appropriate display
    CLCDOutput *ColorOutput(void);
    CLCDOutput *MonoOutput(void);

    BOOL HasColorDevice() { return m_AppletState.Color.pOutput && m_AppletState.Color.pOutput->IsOpened(); }
    BOOL HasMonochromeDevice() { return m_AppletState.Mono.pOutput && m_AppletState.Mono.pOutput->IsOpened(); }

protected:
    // dwDisplayType = LGLCD_DEVICE_BW or LGLCD_DEVICE_QVGA
    virtual void OnDeviceArrival(DWORD dwDisplayType);
    virtual void OnDeviceRemoval(DWORD dwDisplayType);
    virtual void OnAppletEnabled(void);
    virtual void OnAppletDisabled(void);
    virtual void OnNotification(DWORD dwNotification, DWORD dwParam1 = 0);
    virtual void OnConfigure(void);
    virtual void OnSoftButtonEvent(int nDeviceId, DWORD dwButtonState);
    virtual void OnCallbackEvent(void) { }

protected:
    virtual void Connect(void);
    virtual void Disconnect(void);

protected:
    // Override only if you need to create custom output classes
    virtual CLCDOutput* AllocMonoOutput(void);
    virtual CLCDOutput* AllocColorOutput(void);
    virtual void FreeMonoOutput(void);
    virtual void FreeColorOutput(void);

protected:
    // Internal threaded event handling
    enum CB_TYPE { CBT_BUTTON, CBT_CONFIG, CBT_NOTIFICATION };
    typedef struct CB_EVENT
    {
        CB_TYPE Type;
        DWORD CallbackCode;
        DWORD CallbackParam1;
        DWORD CallbackParam2;
        DWORD CallbackParam3;
        DWORD CallbackParam;

    } CB_EVENT;

    int m_hConnection;

    APPLET_STATE m_AppletState;

    
    BOOL GetNextCallbackEvent(CB_EVENT& rEvent);

private:
    lgLcdConnectContextEx m_lcdConnectCtxEx;

    lgLcdSoftbuttonsChangedContext* m_plcdSoftButtonsChangedCtx;



private:
    typedef std::queue<CB_EVENT> LCD_CB_EVENT_QUEUE;

    CRITICAL_SECTION m_csCallback;
    LCD_CB_EVENT_QUEUE m_CallbackEventQueue;
    


private:
    static LONG g_lInitCount;

    static DWORD CALLBACK _OnNotificationCallback(
        IN int connection,
        IN const PVOID pContext,
        IN DWORD notificationCode,
        IN DWORD notifyParm1,
        IN DWORD notifyParm2,
        IN DWORD notifyParm3,
        IN DWORD notifyParm4);

    static DWORD CALLBACK _OnSoftButtonsCallback(
        IN int device,
        IN DWORD dwButtons,
        IN const PVOID pContext);

    static DWORD CALLBACK _OnConfigureCallback(
        IN int connection,
        IN const PVOID pContext);
};

#endif

//** end of LCDConnection.h **********************************************
