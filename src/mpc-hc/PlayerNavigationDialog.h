/*
 * $Id$
 *
 * (C) 2006-2012 see Authors.txt
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
#include <ResizableLib/ResizableDialog.h>


#define MAX_CHANNELS_ALLOWED 200

// CPlayerNavigationDialog dialog

class CPlayerNavigationDialog : public CResizableDialog //CDialog
{

public:
    CPlayerNavigationDialog();   // standard constructor
    virtual ~CPlayerNavigationDialog();

    BOOL Create(CWnd* pParent = NULL);
    void UpdateElementList();
    void UpdatePos(int nID);
    void SetupAudioSwitcherSubMenu(CDVBChannel* Channel = NULL);
    int p_nItems[MAX_CHANNELS_ALLOWED];
    DVBStreamInfo m_audios[DVB_MAX_AUDIO];
    bool m_bTVStations;

    // Dialog Data
    enum { IDD = IDD_NAVIGATION_DLG };

    CListBox m_ChannelList;
    CComboBox m_ComboAudio;
    CButton m_ButtonInfo;
    CButton m_ButtonScan;
    CButton m_ButtonFilterStations;
    CWnd* m_pParent;
    //  CMenu m_subtitles, m_audios;


protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnDestroy();
    afx_msg void OnChangeChannel();
    afx_msg void OnTunerScan();
    afx_msg void OnSelChangeComboAudio();
    afx_msg void OnButtonInfo();
    afx_msg void OnTvRadioStations();
};
