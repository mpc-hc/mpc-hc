/*
 * (C) 2009-2014 see Authors.txt
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

#include "CMPCThemePPageBase.h"
#include <afxcmn.h>
#include <afxwin.h>
#include "CMPCThemeComboBox.h"

// CPPageCapture dialog

class CPPageCapture : public CMPCThemePPageBase
{
    DECLARE_DYNAMIC(CPPageCapture)

private:
    CAtlArray<CString> m_vidnames, m_audnames, m_providernames, m_tunernames, m_receivernames;

    CMPCThemeComboBox m_cbAnalogVideo;
    CMPCThemeComboBox m_cbAnalogAudio;
    CMPCThemeComboBox m_cbAnalogCountry;
    CMPCThemeComboBox m_cbDigitalNetworkProvider;
    CMPCThemeComboBox m_cbDigitalTuner;
    CMPCThemeComboBox m_cbDigitalReceiver;
    int m_iDefaultDevice;
    CMPCThemeComboBox m_cbRebuildFilterGraph;
    CMPCThemeComboBox m_cbStopFilterGraph;

    void FindAnalogDevices();
    void FindDigitalDevices();
    void SaveFoundDevices();

public:
    CPPageCapture();
    virtual ~CPPageCapture();

    // Dialog Data
    enum { IDD = IDD_PPAGECAPTURE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnUpdateAnalog(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDigital(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDigitalReciver(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDigitalStopFilterGraph(CCmdUI* pCmdUI);
    afx_msg void OnSelChangeRebuildFilterGraph();
    afx_msg void OnSelChangeStopFilterGraph();
    afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pNMH, LRESULT* pResult);
};
