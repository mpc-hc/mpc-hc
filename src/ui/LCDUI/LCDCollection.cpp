//************************************************************************
//
// LCDCollection.cpp
//
// The CLCDCollection class is a generic collection of CLCDBase objects.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#include "LCDCollection.h"


//************************************************************************
//
// CLCDCollection::CLCDCollection
//
//************************************************************************

CLCDCollection::CLCDCollection(void)
{
}


//************************************************************************
//
// CLCDCollection::~CLCDCollection
//
//************************************************************************

CLCDCollection::~CLCDCollection(void)
{
}


//************************************************************************
//
// CLCDCollection::AddObject
//
//************************************************************************

BOOL CLCDCollection::AddObject(CLCDBase* pObject)
{
    //TODO: handle addition of same object twice...
    m_Objects.push_back(pObject);
    return TRUE;
}


//************************************************************************
//
// CLCDCollection::RemoveObject
//
//************************************************************************

BOOL CLCDCollection::RemoveObject(CLCDBase* pObject)
{
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
    while(it != m_Objects.end())
    {
        if (*it == pObject)
        {
            m_Objects.erase(it);
            break;
        }
        ++it;
    }
    return FALSE;
}


//************************************************************************
//
// CLCDCollection::OnDraw
//
//************************************************************************

void CLCDCollection::OnDraw(CLCDGfx &rGfx)
{
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
    while(it != m_Objects.end())
    {
        CLCDBase *pObject = *it;
        LCDUIASSERT(NULL != pObject);

        if (!pObject->IsVisible())
        {
            ++it;
            continue;
        }

        // create the clip region
        HRGN hRgn = CreateRectRgn(pObject->GetOrigin().x, pObject->GetOrigin().y,
                                  pObject->GetOrigin().x + pObject->GetWidth(),
                                  pObject->GetOrigin().y + pObject->GetHeight());
        
        // ensure that controls only draw within their specified region
        SelectClipRgn(rGfx.GetHDC(), hRgn);

        // free the region (a copy is used in the call above)
        DeleteObject(hRgn);

        // offset the control at its origin so controls use (0,0)
        POINT ptPrevViewportOrg = { 0, 0 };
        SetViewportOrgEx(rGfx.GetHDC(),
                         pObject->GetOrigin().x,
                         pObject->GetOrigin().y,
                         &ptPrevViewportOrg);

        // allow controls to supply additional translation
        // this allows controls to move freely within the confined viewport
        OffsetViewportOrgEx(rGfx.GetHDC(),
                            pObject->GetLogicalOrigin().x,
                            pObject->GetLogicalOrigin().y,
                            NULL);

        pObject->OnDraw(rGfx);

        // set the clipping region to nothing
        SelectClipRgn(rGfx.GetHDC(), NULL);

        // restore the viewport origin
        SetViewportOrgEx(rGfx.GetHDC(),
            ptPrevViewportOrg.x,
            ptPrevViewportOrg.y,
            NULL);

        // restore the viewport origin offset
        OffsetViewportOrgEx(rGfx.GetHDC(), 0, 0, NULL);

        ++it;
    }
}


//************************************************************************
//
// CLCDCollection::OnUpdate
//
//************************************************************************

void CLCDCollection::OnUpdate(DWORD dwTimestamp)
{
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
    while(it != m_Objects.end())
    {
        CLCDBase *pObject = *it;
        LCDUIASSERT(NULL != pObject);
        pObject->OnUpdate(dwTimestamp);
        ++it;
    }
}


//************************************************************************
//
// CLCDCollection::ResetUpdate
//
//************************************************************************

void CLCDCollection::ResetUpdate(void)
{
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
    while(it != m_Objects.end())
    {
        CLCDBase *pObject = *it;
        LCDUIASSERT(NULL != pObject);
        pObject->ResetUpdate();
        ++it;
    }
}


//************************************************************************
//
// CLCDCollection::Show
//
//************************************************************************

void CLCDCollection::Show(BOOL bShow)
{
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
    while(it != m_Objects.end())
    {
        CLCDBase *pObject = *it;
        LCDUIASSERT(NULL != pObject);
        pObject->Show(bShow);
        ++it;
    }

    CLCDBase::Show(bShow);
}


//** end of LCDCollection.cpp ********************************************
