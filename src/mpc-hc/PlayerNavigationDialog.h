/*
 * (C) 2010-2013 see Authors.txt
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
#include <afxcmn.h>
#include "../filters/transform/BufferFilter/BufferFilter.h"
#include "FloatEdit.h"
#include "DVBChannel.h"
#include "resource.h"
#include "ResizableLib/ResizableDialog.h"

#define MAX_CHANNELS_ALLOWED 200

class CMainFrame;

class CPlayerNavigationDialog : public CResizableDialog
{
public:
    CPlayerNavigationDialog(CMainFrame* pMainFrame);
    virtual ~CPlayerNavigationDialog();

    BOOL Create(CWnd* pParent = nullptr);
    void UpdateElementList();
    void UpdatePos(int nID);
    int p_nItems[MAX_CHANNELS_ALLOWED];
    bool m_bTVStations;

    // Dialog Data
    enum { IDD = IDD_NAVIGATION_DLG };

    CMainFrame* m_pMainFrame;

    CListBox m_ChannelList;
    CButton m_ButtonInfo;
    CButton m_ButtonScan;
    CButton m_ButtonFilterStations;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnDestroy();
    afx_msg void OnChangeChannel();
    afx_msg void OnTunerScan();
    afx_msg void OnButtonInfo();
    afx_msg void OnTvRadioStations();
};
