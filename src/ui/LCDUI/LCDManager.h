//************************************************************************
//
// LCDManager.h
//
// The CLCDManager class is the representation of a "Screen". LCD UI class
// objects are added here.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#ifndef _LCDMANAGER_H_INCLUDED_ 
#define _LCDMANAGER_H_INCLUDED_ 

#include "LCDCollection.h"


class CLCDManager : public CLCDCollection
{
public:
    CLCDManager();
    virtual ~CLCDManager();

    virtual HRESULT Initialize(void);
    virtual void Shutdown(void);

    lgLcdBitmap160x43x1 *GetLCDScreen(void);
    BITMAPINFO *GetBitmapInfo(void);

    virtual HRESULT Draw(void);
    virtual void Update(DWORD dwTimestamp);

    virtual void OnLCDButtonDown(int nButton);
    virtual void OnLCDButtonUp(int nButton);

    void SetExpiration(DWORD dwMilliseconds);
    virtual BOOL HasExpired(void);
    
private:
    CLCDGfx m_Gfx;

    DWORD m_dwStartTime, m_dwElapsedTime, m_dwExpirationTime;
};


#endif // !_LCDMANAGER_H_INCLUDED_ 

//** end of LCDManager.h *************************************************
