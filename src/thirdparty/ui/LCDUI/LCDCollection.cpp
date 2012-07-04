//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDCollection.cpp
//
// Holds a collection of base items.  Its draw will draw everything
// in its list.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


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
// CLCDCollection::GetObjectCount
//
//************************************************************************

int CLCDCollection::GetObjectCount(void)
{
    return (int)m_Objects.size();
}


//************************************************************************
//
// CLCDCollection::AddObject
//
//************************************************************************

bool CLCDCollection::AddObject(CLCDBase* pObject)
{
    m_Objects.push_back(pObject);
    return true;
}


//************************************************************************
//
// CLCDCollection::RemoveObject
//
//************************************************************************

bool CLCDCollection::RemoveObject(CLCDBase* pObject)
{
    LCD_OBJECT_LIST::iterator it = 
        std::find(m_Objects.begin(), m_Objects.end(), pObject);
    if(it != m_Objects.end())
    {
        m_Objects.erase(it);
        return true;
    }

    return false;
}


//************************************************************************
//
// CLCDCollection::RemoveObject
//
//************************************************************************

bool CLCDCollection::RemoveObject(int objpos)
{
    if(!m_Objects.empty())
    {
        if((0 <= objpos) && (objpos < (int) m_Objects.size()))
        {
            m_Objects.erase(m_Objects.begin() + objpos);
            return true;
        }
    }
    return false;
}


//************************************************************************
//
// CLCDCollection::RemoveAll
//
//************************************************************************

void CLCDCollection::RemoveAll()
{
    m_Objects.clear();
}


//************************************************************************
//
// CLCDCollection::RetrieveObject
//
//************************************************************************

CLCDBase* CLCDCollection::RetrieveObject(int objpos)
{
    if(!m_Objects.empty())
    {
        if((0 <= objpos) && (objpos < (int) m_Objects.size()))
        {
            return m_Objects[objpos];
        }
    }
    return NULL;
}


//************************************************************************
//
// CLCDCollection::OnDraw
//
//************************************************************************

void CLCDCollection::OnDraw(CLCDGfxBase &rGfx)
{
    if(!IsVisible())
    {
        return;
    }

    //iterate through your objects and draw them
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
    while(it != m_Objects.end())
    {
        CLCDBase *pObject = *it++;
        LCDUIASSERT(NULL != pObject);

        if (pObject->IsVisible())
        {
            pObject->OnPrepareDraw(rGfx);

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
        }
    }
}


//************************************************************************
//
// CLCDCollection::OnUpdate
//
//************************************************************************

void CLCDCollection::OnUpdate(DWORD dwTimestamp)
{
    //iterate through your objects and update them
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
    while(it != m_Objects.end())
    {
        CLCDBase *pObject = *it++;
        LCDUIASSERT(NULL != pObject);

        pObject->OnUpdate(dwTimestamp);
    }

    //and update yourself
    CLCDBase::OnUpdate(dwTimestamp);
}


//** end of LCDCollection.cpp ********************************************
