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

#include "PPageBase.h"
#include <afxcmn.h>
#include <afxwin.h>


// CPPageCapture dialog

class CPPageCapture : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageCapture)

	CAtlArray<CString> m_vidnames, m_audnames, m_providernames, m_tunernames, m_receivernames;

public:
	CPPageCapture();   // standard constructor
	virtual ~CPPageCapture();

	// Dialog Data
	enum { IDD = IDD_PPAGECAPTURE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	void FindAnalogDevices();
	void FindDigitalDevices();

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_cbAnalogVideo;
	CComboBox m_cbAnalogAudio;
	CComboBox m_cbAnalogCountry;
	CComboBox m_cbDigitalNetworkProvider;
	CComboBox m_cbDigitalTuner;
	CComboBox m_cbDigitalReceiver;
	int m_iDefaultDevice;
};
