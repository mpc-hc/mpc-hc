//************************************************************************
//
// LCDCollection.h
//
// The CLCDCollection class is a generic collection of CLCDBase objects.
//
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#ifndef _LCDCOLLECTION_H_INCLUDED_
#define _LCDCOLLECTION_H_INCLUDED_

#include "LCDBase.h"


#include <list>
using namespace std;
typedef list <CLCDBase*> LCD_OBJECT_LIST;
typedef LCD_OBJECT_LIST::iterator LCD_OBJECT_LIST_ITER;


class CLCDCollection : public CLCDBase
{
public:
    CLCDCollection();
    virtual ~CLCDCollection();

    // collection objects use relative origin
    BOOL AddObject(CLCDBase* pObject);
    BOOL RemoveObject(CLCDBase* pObject);

    virtual void ResetUpdate(void);
    virtual void Show(BOOL bShow);

public:
    virtual void OnDraw(CLCDGfx &rGfx);
    virtual void OnUpdate(DWORD dwTimestamp);

protected:
    LCD_OBJECT_LIST m_Objects;
};


#endif // !_LCDCOLLECTION_H_INCLUDED_ 

//** end of LCDCollection.h **********************************************
