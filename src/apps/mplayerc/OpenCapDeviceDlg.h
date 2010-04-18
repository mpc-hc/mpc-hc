/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "afxwin.h"
#include <atlcoll.h>


// COpenCapDeviceDlg dialog

class COpenCapDeviceDlg : public CResizableDialog
{
//	DECLARE_DYNAMIC(COpenCapDeviceDlg)

private:
    CAtlArray<CString> m_vidnames, m_audnames;

public:
    COpenCapDeviceDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~COpenCapDeviceDlg();

    CComboBox m_vidctrl;
    CComboBox m_audctrl;
    CComboBox m_countryctrl;

    CString m_vidstr, m_audstr;
    long m_country;

// Dialog Data
    enum { IDD = IDD_OPENCAPDEVICE_DLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedOk();
};
