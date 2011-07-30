/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
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

#include "stdafx.h"
#include "mplayerc.h"
#include "PPageOutput.h"
#include "WinAPIUtils.h"
#include <moreuuids.h>

#include "Monitors.h"

// CPPageOutput dialog

IMPLEMENT_DYNAMIC(CPPageOutput, CPPageBase)
CPPageOutput::CPPageOutput()
	: CPPageBase(CPPageOutput::IDD, CPPageOutput::IDD)
	, m_iDSVideoRendererType(0)
	, m_iRMVideoRendererType(0)
	, m_iQTVideoRendererType(0)
	, m_iAPSurfaceUsage(0)
	, m_iAudioRendererType(0)
	//	, m_fVMRSyncFix(FALSE)
	, m_iDX9Resizer(0)
	, m_fVMR9MixerMode(FALSE)
	, m_fVMR9MixerYUV(FALSE)
	, m_fVMR9AlterativeVSync(FALSE)
	, m_fResetDevice(FALSE)
	, m_iEvrBuffers(L"5")
	, m_fD3DFullscreen(FALSE)
	, m_fD3D9RenderDevice(FALSE)
	, m_iD3D9RenderDevice(-1)
{
}

CPPageOutput::~CPPageOutput()
{
}

void CPPageOutput::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_DSSYSDEF, m_iDSVideoRendererType);
	DDX_Radio(pDX, IDC_RMSYSDEF, m_iRMVideoRendererType);
	DDX_Radio(pDX, IDC_QTSYSDEF, m_iQTVideoRendererType);
	//	DDX_Radio(pDX, IDC_REGULARSURF, m_iAPSurfaceUsage);
	DDX_CBIndex(pDX, IDC_DX_SURFACE, m_iAPSurfaceUsage);
	DDX_CBIndex(pDX, IDC_COMBO1, m_iAudioRendererType);
	DDX_Control(pDX, IDC_COMBO1, m_iAudioRendererTypeCtrl);
	//	DDX_Check(pDX, IDC_CHECK1, m_fVMRSyncFix);
	DDX_CBIndex(pDX, IDC_DX9RESIZER_COMBO, m_iDX9Resizer);
	DDX_Check(pDX, IDC_DSVMR9LOADMIXER, m_fVMR9MixerMode);
	DDX_Check(pDX, IDC_DSVMR9YUVMIXER, m_fVMR9MixerYUV);
	DDX_Check(pDX, IDC_DSVMR9ALTERNATIVEVSYNC, m_fVMR9AlterativeVSync);
	DDX_Check(pDX, IDC_RESETDEVICE, m_fResetDevice);
	DDX_Check(pDX, IDC_FULLSCREEN_MONITOR_CHECK, m_fD3DFullscreen);

	DDX_CBString(pDX, IDC_EVR_BUFFERS, m_iEvrBuffers);

	DDX_Check(pDX, IDC_D3D9DEVICE, m_fD3D9RenderDevice);
	DDX_CBIndex(pDX, IDC_D3D9DEVICE_COMBO, m_iD3D9RenderDevice);
	DDX_Control(pDX, IDC_D3D9DEVICE_COMBO, m_iD3D9RenderDeviceCtrl);
}

BEGIN_MESSAGE_MAP(CPPageOutput, CPPageBase)
	ON_UPDATE_COMMAND_UI(IDC_DSVMR9YUVMIXER, OnUpdateMixerYUV)
	ON_CBN_SELCHANGE(IDC_DX_SURFACE, &CPPageOutput::OnSurfaceChange)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_DSSYSDEF, IDC_DSSYNC, &CPPageOutput::OnDSRendererChange)
	ON_BN_CLICKED(IDC_FULLSCREEN_MONITOR_CHECK, OnFullscreenCheck)
	ON_BN_CLICKED(IDC_D3D9DEVICE, OnD3D9DeviceCheck)
END_MESSAGE_MAP()

void CPPageOutput::DisableRadioButton(UINT nID, UINT nDefID)
{
	if (IsDlgButtonChecked(nID)) {
		CheckDlgButton(nID, BST_UNCHECKED);
		CheckDlgButton(nDefID, BST_CHECKED);
	}

	GetDlgItem(nID)->EnableWindow(FALSE);
}

// CPPageOutput message handlers

BOOL CPPageOutput::OnInitDialog()
{
	__super::OnInitDialog();

	SetHandCursor(m_hWnd, IDC_COMBO1);

	AppSettings& s = AfxGetAppSettings();

	CRenderersSettings& renderersSettings = s.m_RenderersSettings;
	m_iDSVideoRendererType	= s.iDSVideoRendererType;
	m_iRMVideoRendererType	= s.iRMVideoRendererType;
	m_iQTVideoRendererType	= s.iQTVideoRendererType;
	m_iAPSurfaceUsage		= renderersSettings.iAPSurfaceUsage;
	//	m_fVMRSyncFix			= renderersSettings.fVMRSyncFix;
	m_iDX9Resizer			= renderersSettings.iDX9Resizer;
	m_fVMR9MixerMode		= renderersSettings.fVMR9MixerMode;
	m_fVMR9MixerYUV			= renderersSettings.fVMR9MixerYUV;
	m_fVMR9AlterativeVSync	= renderersSettings.m_RenderSettings.fVMR9AlterativeVSync;
	m_fD3DFullscreen		= s.fD3DFullscreen;
	m_iEvrBuffers.Format(L"%d", renderersSettings.iEvrBuffers);

	m_fResetDevice = s.m_RenderersSettings.fResetDevice;
	m_AudioRendererDisplayNames.Add(_T(""));
	m_iAudioRendererTypeCtrl.AddString(_T("1: System Default"));
	m_iAudioRendererType = 0;

	int i=2;
	CString Cbstr;

	BeginEnumSysDev(CLSID_AudioRendererCategory, pMoniker) {
		LPOLESTR olestr = NULL;
		if (FAILED(pMoniker->GetDisplayName(0, 0, &olestr))) {
			continue;
		}

		CStringW str(olestr);
		CoTaskMemFree(olestr);

		m_AudioRendererDisplayNames.Add(CString(str));

		CComPtr<IPropertyBag> pPB;
		if (SUCCEEDED(pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPB))) {
			CComVariant var;
			pPB->Read(CComBSTR(_T("FriendlyName")), &var, NULL);

			CString fstr(var.bstrVal);

			var.Clear();
			if (SUCCEEDED(pPB->Read(CComBSTR(_T("FilterData")), &var, NULL))) {
				BSTR* pbstr;
				if (SUCCEEDED(SafeArrayAccessData(var.parray, (void**)&pbstr))) {
					fstr.Format(_T("%s (%08x)"), CString(fstr), *((DWORD*)pbstr + 1));
					SafeArrayUnaccessData(var.parray);
				}
			}
			Cbstr.Format(_T("%d: %s"), i, fstr);
		} else {
			Cbstr.Format(_T("%d: %s"), i, CString(str));
		}
		m_iAudioRendererTypeCtrl.AddString(Cbstr);

		if (s.strAudioRendererDisplayName == str && m_iAudioRendererType == 0) {
			m_iAudioRendererType = m_iAudioRendererTypeCtrl.GetCount()-1;
		}
		i++;
	}
	EndEnumSysDev

	Cbstr.Format(_T("%d: %s"), i++, AUDRNDT_NULL_COMP);
	m_AudioRendererDisplayNames.Add(AUDRNDT_NULL_COMP);
	m_iAudioRendererTypeCtrl.AddString(Cbstr);
	if (s.strAudioRendererDisplayName == AUDRNDT_NULL_COMP && m_iAudioRendererType == 0) {
		m_iAudioRendererType = m_iAudioRendererTypeCtrl.GetCount()-1;
	}

	Cbstr.Format(_T("%d: %s"), i++, AUDRNDT_NULL_UNCOMP);
	m_AudioRendererDisplayNames.Add(AUDRNDT_NULL_UNCOMP);
	m_iAudioRendererTypeCtrl.AddString(Cbstr);
	if (s.strAudioRendererDisplayName == AUDRNDT_NULL_UNCOMP && m_iAudioRendererType == 0) {
		m_iAudioRendererType = m_iAudioRendererTypeCtrl.GetCount()-1;
	}

	Cbstr.Format(_T("%d: %s"), i++, AUDRNDT_MPC);
	m_AudioRendererDisplayNames.Add(AUDRNDT_MPC);
	m_iAudioRendererTypeCtrl.AddString(Cbstr);
	if (s.strAudioRendererDisplayName == AUDRNDT_MPC && m_iAudioRendererType == 0) {
		m_iAudioRendererType = m_iAudioRendererTypeCtrl.GetCount()-1;
	}


	CorrectComboListWidth(m_iAudioRendererTypeCtrl, GetFont());

	//
	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (pD3D) {
		TCHAR		strGUID[50];
		CString cstrGUID;
		CString d3ddevice_str = _T("");
		CStringArray adapterList;

		D3DADAPTER_IDENTIFIER9 adapterIdentifier;

		for (UINT adp = 0, num_adp = pD3D->GetAdapterCount(); adp < num_adp; ++adp) {
			if (pD3D->GetAdapterIdentifier(adp, 0, &adapterIdentifier) == S_OK) {
				d3ddevice_str = adapterIdentifier.Description;
				d3ddevice_str += _T(" - ");
				d3ddevice_str += adapterIdentifier.DeviceName;
				cstrGUID = _T("");
				if (::StringFromGUID2(adapterIdentifier.DeviceIdentifier, strGUID, 50) > 0) {
					cstrGUID = strGUID;
				}
				if ((cstrGUID != _T(""))) {
					boolean m_find = false;
					for (i = 0; (!m_find) && (i < m_D3D9GUIDNames.GetCount()); i++) {
						if (m_D3D9GUIDNames.GetAt(i) == cstrGUID) {
							m_find = true;
						}
					}
					if (!m_find) {
						m_iD3D9RenderDeviceCtrl.AddString(d3ddevice_str);
						m_D3D9GUIDNames.Add(cstrGUID);
						if (renderersSettings.D3D9RenderDevice == cstrGUID) {
							m_iD3D9RenderDevice = m_iD3D9RenderDeviceCtrl.GetCount()-1;
						}
					}
				}
			}
		}
		pD3D->Release();
	}

	CorrectComboListWidth(m_iD3D9RenderDeviceCtrl, GetFont());

	UpdateData(FALSE);

	if (!IsCLSIDRegistered(CLSID_VideoMixingRenderer)) {
		DisableRadioButton(IDC_DSVMR7WIN, IDC_DSSYSDEF);
		DisableRadioButton(IDC_DSVMR7REN, IDC_DSSYSDEF);
	}

	if (!IsCLSIDRegistered(CLSID_VideoMixingRenderer9)) {
		DisableRadioButton(IDC_DSVMR9WIN, IDC_DSSYSDEF);
		DisableRadioButton(IDC_DSVMR9REN, IDC_DSSYSDEF);
		DisableRadioButton(IDC_RMDX9, IDC_RMSYSDEF);
		DisableRadioButton(IDC_QTDX9, IDC_QTSYSDEF);
	}

	if (!IsCLSIDRegistered(CLSID_EnhancedVideoRenderer)) {
		DisableRadioButton(IDC_EVR, IDC_DSSYSDEF);
		DisableRadioButton(IDC_EVR_CUSTOM, IDC_DSSYSDEF);
		DisableRadioButton(IDC_DSSYNC, IDC_DSSYSDEF); // EVR Sync
	}

	if (!IsCLSIDRegistered(CLSID_DXR)) {
		DisableRadioButton(IDC_DSDXR, IDC_DSSYSDEF);
	}

	if (!IsCLSIDRegistered(CLSID_madVR)) {
		DisableRadioButton(IDC_DSMADVR, IDC_DSSYSDEF);
	}

	// YUV mixing is not compatible with Vista
	if (IsWinVistaOrLater()) {
		GetDlgItem(IDC_DSVMR9YUVMIXER)->ShowWindow (SW_HIDE);
	}

	OnDSRendererChange (m_iDSVideoRendererType + IDC_DSSYSDEF);

	CheckDlgButton(IDC_D3D9DEVICE, BST_CHECKED);
	GetDlgItem(IDC_D3D9DEVICE)->EnableWindow(TRUE);
	GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(TRUE);

	if ((m_iDSVideoRendererType == 6 || m_iDSVideoRendererType == 11) && (m_iD3D9RenderDeviceCtrl.GetCount() > 1)) {
		GetDlgItem(IDC_D3D9DEVICE)->EnableWindow(TRUE);
		GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(FALSE);
		CheckDlgButton(IDC_D3D9DEVICE, BST_UNCHECKED);
		if (m_iD3D9RenderDevice != -1) {
			CheckDlgButton(IDC_D3D9DEVICE, BST_CHECKED);
			GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(TRUE);
		}
	} else {
		GetDlgItem(IDC_D3D9DEVICE)->EnableWindow(FALSE);
		GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(FALSE);
		if (m_iD3D9RenderDevice == -1) {
			CheckDlgButton(IDC_D3D9DEVICE, BST_UNCHECKED);
		}
	}
	UpdateData(TRUE);

	CreateToolTip();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageOutput::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	CRenderersSettings& renderersSettings = s.m_RenderersSettings;
	s.iDSVideoRendererType		= m_iDSVideoRendererType;
	s.iRMVideoRendererType		= m_iRMVideoRendererType;
	s.iQTVideoRendererType		= m_iQTVideoRendererType;
	renderersSettings.iAPSurfaceUsage			= m_iAPSurfaceUsage;
	//	renderersSettings.fVMRSyncFix				= !!m_fVMRSyncFix;
	renderersSettings.iDX9Resizer				= m_iDX9Resizer;
	renderersSettings.fVMR9MixerMode			= !!m_fVMR9MixerMode;
	renderersSettings.fVMR9MixerYUV				= !!m_fVMR9MixerYUV;
	renderersSettings.m_RenderSettings.fVMR9AlterativeVSync		= m_fVMR9AlterativeVSync != 0;
	s.strAudioRendererDisplayName	= m_AudioRendererDisplayNames[m_iAudioRendererType];
	s.fD3DFullscreen			= m_fD3DFullscreen ? true : false;

	renderersSettings.fResetDevice = !!m_fResetDevice;

	if (!m_iEvrBuffers.IsEmpty()) {
		int Temp = 5;
		swscanf(m_iEvrBuffers.GetBuffer(), L"%d", &Temp);
		renderersSettings.iEvrBuffers = Temp;
	} else {
		renderersSettings.iEvrBuffers = 5;
	}

	renderersSettings.D3D9RenderDevice = m_fD3D9RenderDevice ? m_D3D9GUIDNames[m_iD3D9RenderDevice] : _T("");

	return __super::OnApply();
}

void CPPageOutput::OnUpdateMixerYUV(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!!IsDlgButtonChecked(IDC_DSVMR9LOADMIXER) && IsDlgButtonChecked(IDC_DSVMR9REN));
}
void CPPageOutput::OnSurfaceChange()
{
	SetModified();
}

void CPPageOutput::OnDSRendererChange(UINT nIDbutton)
{
	GetDlgItem(IDC_DX_SURFACE)->EnableWindow(FALSE);
	GetDlgItem(IDC_DX9RESIZER_COMBO)->EnableWindow(FALSE);
	GetDlgItem(IDC_FULLSCREEN_MONITOR_CHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_DSVMR9LOADMIXER)->EnableWindow(FALSE);
	GetDlgItem(IDC_DSVMR9YUVMIXER)->EnableWindow(FALSE);
	GetDlgItem(IDC_DSVMR9ALTERNATIVEVSYNC)->EnableWindow(FALSE);
	GetDlgItem(IDC_RESETDEVICE)->EnableWindow(FALSE);
	//	GetDlgItem(IDC_CHECK1)->EnableWindow(FALSE);
	GetDlgItem(IDC_EVR_BUFFERS)->EnableWindow((nIDbutton - IDC_DSSYSDEF) == 11);
	GetDlgItem(IDC_EVR_BUFFERS_TXT)->EnableWindow((nIDbutton - IDC_DSSYSDEF) == 11);

	GetDlgItem(IDC_D3D9DEVICE)->EnableWindow(FALSE);
	GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(FALSE);

	switch (nIDbutton - IDC_DSSYSDEF) {
		case 5 :	// VMR7 renderless
			GetDlgItem(IDC_DX_SURFACE)->EnableWindow(TRUE);
			break;
		case 6 :	// VMR9 renderless
			if (m_iD3D9RenderDeviceCtrl.GetCount()>1) {
				GetDlgItem(IDC_D3D9DEVICE)->EnableWindow(TRUE);
				GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(IsDlgButtonChecked(IDC_D3D9DEVICE));
			}

			GetDlgItem(IDC_DSVMR9LOADMIXER)->EnableWindow(TRUE);
			GetDlgItem(IDC_DSVMR9YUVMIXER)->EnableWindow(TRUE);
			GetDlgItem(IDC_DSVMR9ALTERNATIVEVSYNC)->EnableWindow(TRUE);
			GetDlgItem(IDC_RESETDEVICE)->EnableWindow(TRUE);
		case 11 :	// EVR custom presenter
			if (m_iD3D9RenderDeviceCtrl.GetCount()>1) {
				GetDlgItem(IDC_D3D9DEVICE)->EnableWindow(TRUE);
				GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(IsDlgButtonChecked(IDC_D3D9DEVICE));
			}

			GetDlgItem(IDC_DX9RESIZER_COMBO)->EnableWindow(TRUE);
			GetDlgItem(IDC_FULLSCREEN_MONITOR_CHECK)->EnableWindow(TRUE);
			//		GetDlgItem(IDC_CHECK1)->EnableWindow(TRUE);		// Lock back buffer
			GetDlgItem(IDC_DSVMR9ALTERNATIVEVSYNC)->EnableWindow(TRUE);
			GetDlgItem(IDC_RESETDEVICE)->EnableWindow(TRUE);

			// Force 3D surface with EVR Custom
			if (nIDbutton - IDC_DSSYSDEF == 11) {
				GetDlgItem(IDC_DX_SURFACE)->EnableWindow(FALSE);
				((CComboBox*)GetDlgItem(IDC_DX_SURFACE))->SetCurSel(2);
			} else {
				GetDlgItem(IDC_DX_SURFACE)->EnableWindow(TRUE);
			}
			break;
		case 12 :	// madVR
			GetDlgItem(IDC_FULLSCREEN_MONITOR_CHECK)->EnableWindow(TRUE);
			break;
		case 13 :	// Sync Renderer
			GetDlgItem(IDC_EVR_BUFFERS)->EnableWindow(TRUE);
			GetDlgItem(IDC_EVR_BUFFERS_TXT)->EnableWindow(TRUE);
			GetDlgItem(IDC_DX9RESIZER_COMBO)->EnableWindow(TRUE);
			GetDlgItem(IDC_FULLSCREEN_MONITOR_CHECK)->EnableWindow(TRUE);
			GetDlgItem(IDC_RESETDEVICE)->EnableWindow(TRUE);
			GetDlgItem(IDC_DX_SURFACE)->EnableWindow(FALSE);
			((CComboBox*)GetDlgItem(IDC_DX_SURFACE))->SetCurSel(2);
			break;
	}

	SetModified();
}

void CPPageOutput::OnFullscreenCheck()
{
	UpdateData();
	if (m_fD3DFullscreen &&
			(MessageBox(ResStr(IDS_D3DFS_WARNING), NULL, MB_YESNO) == IDNO)) {
		m_fD3DFullscreen = false;
		UpdateData(FALSE);
	}
	SetModified();
}

void CPPageOutput::OnD3D9DeviceCheck()
{
	UpdateData();
	GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(m_fD3D9RenderDevice);
	SetModified();
}
