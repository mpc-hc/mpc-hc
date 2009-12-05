#pragma once

#include "PPageBase.h"
#include "afxcmn.h"
#include "afxwin.h"

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
