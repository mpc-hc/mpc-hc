/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of vsrip.
 *
 * Vsrip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vsrip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <afxtempl.h>
#include "../../subtitles/VobSubFileRipper.h"

// CVSRipPage dialog

class CVSRipPage : public CDialog, public IVSFRipperCallbackImpl
{
    DECLARE_DYNAMIC(CVSRipPage)

protected:
    CComPtr<IVSFRipper> m_pVSFRipper;

public:
    CVSRipPage(IVSFRipper* pVSFRipper, UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor
    virtual ~CVSRipPage();

//	static bool ParseParamFile(CString fn);

    virtual void OnPrev() {}
    virtual void OnNext() {}
    virtual void OnClose() {}
    virtual bool CanGoPrev()
    {
        return(false);
    }
    virtual bool CanGoNext()
    {
        return(false);
    }
    virtual bool CanClose()
    {
        return(true);
    }
    virtual CString GetPrevText()
    {
        return(_T("< &Back"));
    }
    virtual CString GetNextText()
    {
        return(_T("&Next >"));
    }
    virtual CString GetCloseText()
    {
        return(_T("&Cancel"));
    }
    virtual CString GetHeaderText()
    {
        return(_T("Header Text"));
    }
    virtual CString GetDescText()
    {
        return(_T("Hello World"));
    }

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
