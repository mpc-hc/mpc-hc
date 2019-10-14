/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <afxwin.h>
#include <atlcoll.h>
#include "IGraphBuilder2.h"
#include "CMPCThemeResizableDialog.h"
#include "CMPCThemeEdit.h"
#include "CMPCThemeComboBox.h"


// CMediaTypesDlg dialog

class CMediaTypesDlg : public CMPCThemeResizableDialog
{
    //  DECLARE_DYNAMIC(CMediaTypesDlg)

private:
    CComPtr<IGraphBuilderDeadEnd> m_pGBDE;
    enum { UNKNOWN, VIDEO, AUDIO } m_type;
    GUID m_subtype;
    void AddLine(CString str = _T("\n"));
    void AddMediaType(const AM_MEDIA_TYPE* pmt);

public:
    CMediaTypesDlg(IGraphBuilderDeadEnd* pGBDE, CWnd* pParent = nullptr);   // standard constructor
    virtual ~CMediaTypesDlg();

    // Dialog Data
    enum { IDD = IDD_MEDIATYPES_DLG };
    CMPCThemeComboBox m_pins;
    CMPCThemeEdit m_report;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnCbnSelchangeCombo1();
};
