//************************************************************************
//
// LCDBase.h
//
// The CLCDBase class is the generic base class for all lcd ui objects
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#ifndef _LCDBASE_H_INCLUDED_
#define _LCDBASE_H_INCLUDED_

#include "LCDGfx.h"


typedef enum
{
    LG_SCROLLING_TEXT, LG_STATIC_TEXT, LG_ICON, LG_PROGRESS_BAR, LG_UNKNOWN
} LGObjectType;

class CLCDBase
{
public:
    CLCDBase(void);
    virtual ~CLCDBase(void);

public:
    virtual HRESULT Initialize(void);
    virtual void Shutdown(void);

    virtual void SetOrigin(POINT pt);
    virtual void SetOrigin(int nX, int nY);
    virtual POINT& GetOrigin(void);

    virtual void SetSize(SIZE& size);
    virtual void SetSize(int nCX, int nCY);
    virtual SIZE& GetSize(void);
    
    virtual int GetWidth(void) { return GetSize().cx; }
    virtual int GetHeight(void) { return GetSize().cy; };

    virtual void Show(BOOL bShow);
    virtual BOOL IsVisible();

    virtual void Invert(BOOL bEnable);
    virtual void ResetUpdate(void);

    // local coordinates
    virtual void SetLogicalOrigin(POINT& rLogical);
    virtual void SetLogicalOrigin(int nX, int nY);
    virtual POINT& GetLogicalOrigin(void);
    virtual void SetLogicalSize(SIZE& size);
    virtual void SetLogicalSize(int nCX, int nCY);
    virtual SIZE& GetLogicalSize(void);

    virtual void SetBackgroundMode(int nMode);
    virtual int  GetBackgroundMode();

    virtual const LGObjectType GetObjectType();
    virtual void SetObjectType(const LGObjectType type);
    
public:
    virtual void OnDraw(CLCDGfx &rGfx) = 0;
    virtual void OnUpdate(DWORD dwTimestamp);

protected:    
    SIZE m_Size;
    POINT m_Origin;
    BOOL m_bVisible;
    BOOL m_bInverted;

    POINT m_ptLogical;
    SIZE m_sizeLogical;
    int m_nBkMode;

    LGObjectType m_objectType;
};


#endif // !_LCDBASE_H_INCLUDED_

//** end of LCDBase.h ****************************************************
