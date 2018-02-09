/*
* (C) 2009-2014 see Authors.txt
* (C) 2009-2013 see Authors.txt
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

#include "PPageBase.h"
#include <afxcmn.h>
#include <afxwin.h>

// CPPageDigitalTV dialog

class CPPageDigitalTV : public CPPageBase
{
    DECLARE_DYNAMIC(CPPageDigitalTV)

    CAtlArray<CString> m_vidnames, m_audnames, m_providernames, m_tunernames, m_receivernames;

private:
    BOOL m_bEnableDVB;
    CComboBox m_cbDigitalNetworkProvider;
    CComboBox m_cbDigitalTuner;
    CComboBox m_cbDigitalReceiver;
    CComboBox m_cbRebuildFilterGraph;
    CComboBox m_cbStopFilterGraph;

    void FindAnalogDevices();
    void FindDigitalDevices();
    void SaveFoundDevices();
    BOOL m_bEnableIPTV;
    BOOL m_bUseIGMPMembership;

public:
    CPPageDigitalTV();   // standard constructor
    virtual ~CPPageDigitalTV();

    // Dialog Data
    enum { IDD = IDD_PPAGEDIGITALTV };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();

    DECLARE_MESSAGE_MAP()

public:

    afx_msg void OnUpdateDVB(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDigitalReciver(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDigitalStopFilterGraph(CCmdUI* pCmdUI);
    afx_msg void OnSelchangeRebuildFilterGraph();
    afx_msg void OnSelchangeStopFilterGraph();
    afx_msg void OnUpdateIPTV(CCmdUI* pCmdUI);
    afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pNMH, LRESULT* pResult);
    afx_msg void OnClickedDVBChannels();
    afx_msg void OnClickedIPTVChannels();
};
