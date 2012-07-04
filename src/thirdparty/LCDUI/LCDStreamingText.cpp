//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDStreamingText.cpp
//
// The CLCDStreamingText class draws streaming text onto the LCD.
// Streaming text is a single line of text that is repeatedly streamed
// horizontally across the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"

//************************************************************************
//
// CLCDStreamingText::CLCDStreamingText
//
//************************************************************************

CLCDStreamingText::CLCDStreamingText()
{
    m_sText.erase(m_sText.begin(), m_sText.end());
    m_sGapText.erase(m_sGapText.begin(), m_sGapText.end());

    m_sGapText.assign(_T("   "));
    m_hFont = NULL;
    m_nTextAlignment = DT_LEFT;

    m_bRedoColors = TRUE;
}


//************************************************************************
//
// CLCDStreamingText::~CLCDStreamingText
//
//************************************************************************

CLCDStreamingText::~CLCDStreamingText()
{
    RemoveAllText();
    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }
}


//************************************************************************
//
// CLCDStreamingText::Initialize
//
//************************************************************************

HRESULT CLCDStreamingText::Initialize(void)
{
    m_eState = STATE_DELAY;
    m_dwStartDelay  = 2000;
    m_dwSpeed       = 7;
	m_dwStepInPixels = 7;
    m_dwLastUpdate = 0;
    m_dwEllapsedTime = 0;
    m_dwLastUpdate = GetTickCount();
    m_bRecalcExtent = FALSE;
    m_pQueueHead = NULL;
    m_fFractDistance = 0.0f;

    m_hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
    if(NULL != m_hFont)
    {
        SetFontPointSize(DEFAULT_POINTSIZE);
    }

    //return CLCDCollection::Initialize();
    return ERROR_SUCCESS;
}


//************************************************************************
//
// CLCDStreamingText::ResetUpdate
//
//************************************************************************

void CLCDStreamingText::ResetUpdate(void)
{
    m_eState = STATE_DELAY;
    m_dwEllapsedTime = 0;
    m_dwLastUpdate = GetTickCount();
    m_pQueueHead = NULL;
    m_fFractDistance = 0.0f;
    
    // remove, re-add, and recalculate the text
    m_bRecalcExtent = TRUE;
    RemoveAllText();
    AddText(m_sText.c_str());
}


//************************************************************************
//
// CLCDStreamingText::Show
//
//************************************************************************

void CLCDStreamingText::Show(BOOL bShow)
{
    CLCDCollection::Show(bShow);
}


//************************************************************************
//
// CLCDStreamingText::SetText
//
//************************************************************************

void CLCDStreamingText::SetText(LPCTSTR szText)
{
    LCDUIASSERT(NULL != szText);
    if(szText && _tcscmp(m_sText.c_str(), szText))
    {
        m_sText.assign(szText);
        m_bRecalcExtent = TRUE;
        ResetUpdate();
    }
}


//************************************************************************
//
// CLCDStreamingText::SetOrigin
//
//************************************************************************

void CLCDStreamingText::SetOrigin(POINT pt)
{
	m_Origin = pt;
	SetOrigin(pt.x, pt.y);
}


//************************************************************************
//
// CLCDStreamingText::SetOrigin
//
//************************************************************************

void CLCDStreamingText::SetOrigin(int nX, int nY)
{
	m_Origin.x = nX;
	m_Origin.y = nY;

    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
	while(it != m_Objects.end())
	{
        CLCDBase *pObject = *it++;
        LCDUIASSERT(NULL != pObject);

		POINT ptOldOrigin = pObject->GetOrigin();
		pObject->SetOrigin(nX, nY);

		if ( (ptOldOrigin.x != nX) && (ptOldOrigin.y != nY) )
        {
            ResetUpdate();
            break;
        }
    }
}


//************************************************************************
//
// CLCDStreamingText::SetSize
//
//************************************************************************

void CLCDStreamingText::SetSize(SIZE& size)
{
    CLCDBase::SetSize(size);

    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
	if (it != m_Objects.end())
	{
        CLCDBase *pObject = *it;
		pObject->SetSize(size);
    }
}


//************************************************************************
//
// CLCDStreamingText::SetSize
//
//************************************************************************

void CLCDStreamingText::SetSize(int nCX, int nCY)
{
    SIZE size = { nCX, nCY };
    SetSize(size);  
}


//************************************************************************
//
// CLCDStreamingText::SetBackgroundMode
//
//************************************************************************

void CLCDStreamingText::SetBackgroundMode(int nMode)
{
    m_bRedoColors = TRUE;
    CLCDBase::SetBackgroundMode(nMode);
}


//************************************************************************
//
// CLCDStreamingText::SetForegroundColor
//
//************************************************************************

void CLCDStreamingText::SetForegroundColor(COLORREF crForeground)
{
    m_bRedoColors = TRUE;
    CLCDBase::SetForegroundColor(crForeground);
}


//************************************************************************
//
// CLCDStreamingText::SetBackgroundColor
//
//************************************************************************

void CLCDStreamingText::SetBackgroundColor(COLORREF crBackground)
{
    m_bRedoColors = TRUE;
    CLCDBase::SetBackgroundColor(crBackground);
}


//************************************************************************
//
// CLCDStreamingText::SetGapText
//
//************************************************************************

void CLCDStreamingText::SetGapText(LPCTSTR szGapText)
{
    LCDUIASSERT(NULL != szGapText);
    if(szGapText && _tcscmp(m_sGapText.c_str(), szGapText))
    {
        m_sGapText.assign(szGapText);
        m_bRecalcExtent = TRUE;
    }
}


//************************************************************************
//
// CLCDStreamingText::SetFont
//
//************************************************************************

void CLCDStreamingText::SetFont(LOGFONT& lf)
{
    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }

    m_hFont = CreateFontIndirect(&lf);
    m_bRecalcExtent = TRUE;
}


//************************************************************************
//
// CLCDStreamingText::GetFont
//
//************************************************************************

HFONT CLCDStreamingText::GetFont()
{
    return m_hFont;
}


//************************************************************************
//
// CLCDText::SetFontFaceName
//
//************************************************************************

void CLCDStreamingText::SetFontFaceName(LPCTSTR szFontName)
{
    // if NULL, uses the default gui font
    if (NULL == szFontName)
        return;

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    GetObject(m_hFont, sizeof(LOGFONT), &lf);

    DeleteObject(m_hFont);
    m_hFont = NULL;

    LCDUI_tcsncpy(lf.lfFaceName, szFontName, LF_FACESIZE);

    m_hFont = CreateFontIndirect(&lf);
}


//************************************************************************
//
// CLCDStreamingText::SetFontPointSize
//
//************************************************************************

void CLCDStreamingText::SetFontPointSize(int nPointSize)
{
    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    GetObject(m_hFont, sizeof(LOGFONT), &lf);

    DeleteObject(m_hFont);
    m_hFont = NULL;

    lf.lfHeight = -MulDiv(nPointSize, DEFAULT_DPI, 72);

    m_hFont = CreateFontIndirect(&lf);
}


//************************************************************************
//
// CLCDStreamingText::SetFontWeight
//
//************************************************************************

void CLCDStreamingText::SetFontWeight(int nWeight)
{
    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    GetObject(m_hFont, sizeof(LOGFONT), &lf);

    DeleteObject(m_hFont);
    m_hFont = NULL;

    lf.lfWeight = nWeight;

    m_hFont = CreateFontIndirect(&lf);
}


//************************************************************************
//
// CLCDStreamingText::SetFontColor
//
//************************************************************************

void CLCDStreamingText::SetFontColor(COLORREF color)
{
    SetForegroundColor(color);

    // Apply it to any existing text objects
    for (size_t i = 0; i < m_Objects.size(); i++)
    {
        m_Objects[i]->SetForegroundColor(color);
    }
}


//************************************************************************
//
// CLCDStreamingText::SetAlignment
//
// only relevant if no streaming
//************************************************************************

void CLCDStreamingText::SetAlignment(int nAlignment)
{
    m_nTextAlignment = nAlignment;
}


//************************************************************************
//
// CLCDStreamingText::AddText
//
//************************************************************************

int CLCDStreamingText::AddText(LPCTSTR szText)
{

    CLCDText* pText = new CLCDText;
    pText->Initialize();
    pText->SetText(szText);
    pText->SetOrigin(GetOrigin().x, GetOrigin().y);
    pText->SetLogicalOrigin(GetLogicalOrigin().x, GetLogicalOrigin().y);
    pText->SetSize(GetWidth(), GetHeight());
    pText->SetBackgroundMode(m_nBkMode);
    pText->SetBackgroundColor(m_crBackgroundColor);
    pText->SetFontColor(m_crForegroundColor);

    LOGFONT lf;
    GetObject(m_hFont, sizeof(LOGFONT), &lf);
    pText->SetFont(lf);

    m_bRecalcExtent = TRUE;

    AddObject(pText);


    if (NULL == m_pQueueHead)
    {
        m_pQueueHead = pText;
    }

    // return the zero-based index
    return (int)(m_Objects.size()-1);
}


//************************************************************************
//
// CLCDStreamingText::RemoveText
//
//************************************************************************

void CLCDStreamingText::RemoveText(int nIndex)
{
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
    int i = 0;
    while(it != m_Objects.end())
    {
        CLCDBase *pObject = *it;
        LCDUIASSERT(NULL != pObject);

        if (i == nIndex)
        {
            m_Objects.erase(it);
            if(NULL != pObject)
            {
                delete pObject;
            }
            break;
        }
        
        ++it;
        ++i;
    }
}


//************************************************************************
//
// CLCDStreamingText::RemoveAllText
//
//************************************************************************

void CLCDStreamingText::RemoveAllText()
{
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
    while(it != m_Objects.end())
    {
        // we need to delete all the text objects that were placed in to m_Objects
        CLCDBase *pObject = *it;
        LCDUIASSERT(NULL != pObject);

        if(NULL != pObject)
        {
            delete pObject;
        }
        ++it;
    }
    m_Objects.erase(m_Objects.begin(), m_Objects.end());
}


//************************************************************************
//
// CLCDStreamingText::SetStartDelay
//
//************************************************************************

void CLCDStreamingText::SetStartDelay(DWORD dwMilliseconds)
{
    m_dwStartDelay = dwMilliseconds;
}


//************************************************************************
//
// CLCDStreamingText::SetSpeed
//
//************************************************************************

void CLCDStreamingText::SetSpeed(DWORD dwSpeed)
{
    m_dwSpeed = dwSpeed;
}

//************************************************************************
//
// CLCDStreamingText::SetScrollingStep: sets the number of pixels the text 
// will jump when it scrolls
//
//************************************************************************

void CLCDStreamingText::SetScrollingStep(DWORD dwStepInPixels)
{
    m_dwStepInPixels = dwStepInPixels;
}

//************************************************************************
//
// CLCDStreamingText::OnUpdate
//
//************************************************************************

void CLCDStreamingText::OnUpdate(DWORD dwTimestamp)
{
    m_dwEllapsedTime = (dwTimestamp - m_dwLastUpdate);
}


//************************************************************************
//
// CLCDStreamingText::OnDraw
//
//************************************************************************

void CLCDStreamingText::OnDraw(CLCDGfxBase &rGfx)
{

    if (m_bRecalcExtent)
    {
        // this just recalculates the text extents
        RecalcTextBoxes(rGfx);
        m_bRecalcExtent = FALSE;
        return;
    }

    switch(m_eState)
    {
    case STATE_DELAY:
        if (m_dwEllapsedTime > m_dwStartDelay)
        {
            m_eState = STATE_SCROLL;
            m_dwEllapsedTime = 0;
            m_dwLastUpdate = GetTickCount();
        }
        break;
    case STATE_SCROLL:
        {
            // update the positions
            float fDistance = (float)(m_dwSpeed * m_dwEllapsedTime) / 1000.0f;
            m_dwLastUpdate = GetTickCount();

            if (m_Objects.size() > 1)
            {
                // extract any previous fractional remainder
                // and add it to the current distance
                float fTotDistance = (fDistance + m_fFractDistance);
                m_fFractDistance = (fTotDistance >= (float)m_dwStepInPixels) ? (float)(fTotDistance - (int)fTotDistance) : fTotDistance;

                if (fTotDistance < 0.0f)
                    fTotDistance = 0.0f;
                if (m_fFractDistance < 0.0f)
                    m_fFractDistance = 0.0f;

				if (fTotDistance >= (float)m_dwStepInPixels)
                    ApplyOrigins(-1 * (int)fTotDistance); // left
            }
        }
        break;
    default:
        break;
    }

    if( m_bRedoColors )
    {
        LCD_OBJECT_LIST::iterator it = m_Objects.begin();
        while(it != m_Objects.end())
        {
            // we need to delete all the text objects that were placed in to m_Objects
            CLCDBase *pObject = *it;
            LCDUIASSERT(NULL != pObject);

            if(NULL != pObject)
            {
                pObject->SetBackgroundMode(m_nBkMode);
                pObject->SetBackgroundColor(m_crBackgroundColor);
                pObject->SetForegroundColor(m_crForegroundColor);
            }
            ++it;
        }

        m_bRedoColors = FALSE;
    }

    CLCDCollection::OnDraw(rGfx);
}


//************************************************************************
//
// CLCDStreamingText::C
//
//************************************************************************

BOOL CLCDStreamingText::RecalcTextBoxes(CLCDGfxBase &rGfx)
{

    // check if we need to add another text box
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();

    if (it == m_Objects.end())
        return FALSE;

    CLCDBase* pObject = *it;
    CLCDText* pText = (CLCDText*)pObject;
    LCDUIASSERT(NULL != pObject);
    
    LOGFONT lf;
    GetObject(m_hFont, sizeof(LOGFONT), &lf);
    pText->SetFont(lf);

    // this will re-evaluate the main text object
    LCDUIASSERT(m_Objects.size() == 1);
    pText->CalculateExtent(TRUE);

    if (it != m_Objects.end())
    {
        if (pText->GetHExtent().cx > GetWidth())
        {

            pText->SetAlignment(DT_LEFT);

            // add a gap
            AddText(m_sGapText.c_str());
            // add another text
            AddText(m_sText.c_str());
            // add last gap
            AddText(m_sGapText.c_str());
        }
        else
        {
            pText->SetAlignment(m_nTextAlignment);
        }
    }

    // this will re-evaluate the other text objects
    CLCDCollection::OnDraw(rGfx);
    RecalcTextBoxOrigins();

    return TRUE;
}


//************************************************************************
//
// CLCDStreamingText::RecalcTextBoxOrigins
//
// Puts all the text boxes in order next to each other
//************************************************************************

void CLCDStreamingText::RecalcTextBoxOrigins()
{

    if (m_Objects.size() <= 1)
        return;

    // draw everyone to the left by the offset
    int nOrgOffset = 0;
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
    while(it != m_Objects.end())
    {
        CLCDBase* pObject = *it;
        CLCDText* pText = (CLCDText*)pObject;
        LCDUIASSERT(NULL != pObject);

        pText->SetLogicalSize(pText->GetHExtent().cx, pText->GetHExtent().cy);

        // string can be empty which generates zero logical space
        //LCDUIASSERT(pText->GetLogicalSize().cx);
        //LCDUIASSERT(pText->GetLogicalSize().cy);

        POINT& ptOrigin = pText->GetLogicalOrigin();

		if (nOrgOffset == 0)
		{
			nOrgOffset = pText->GetLogicalOrigin().x;
		}

        pText->SetLogicalOrigin(nOrgOffset, ptOrigin.y);
        nOrgOffset += pText->GetHExtent().cx;

        ++it;
    }

}


//************************************************************************
//
// CLCDStreamingText::ApplyOrigins
//
//************************************************************************

void CLCDStreamingText::ApplyOrigins(int nOffset)
{
    
    // draw everyone to the left by the offset
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
    while(it != m_Objects.end())
    {
        CLCDBase* pObject = *it;
        CLCDText* pText = (CLCDText*)pObject;
        LCDUIASSERT(NULL != pObject);

        POINT& ptOrigin = pText->GetLogicalOrigin();

        pText->SetLogicalOrigin(ptOrigin.x + nOffset, ptOrigin.y);

        ++it;
    }

    // If the active box is no longer visible, 
    // pop it off the push it to the end of the list
    if (abs(m_pQueueHead->GetLogicalOrigin().x) >= m_pQueueHead->GetHExtent().cx)
    {
        RemoveObject(0);
        AddObject(m_pQueueHead);
        RecalcTextBoxOrigins();
        m_pQueueHead = (CLCDText*)RetrieveObject(0);
    }
}


//** end of LCDStreamingText.cpp *******************************************
