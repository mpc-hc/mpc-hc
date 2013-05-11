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

#include "stdafx.h"
#include "mplayerc.h"
#include "PPageOutput.h"
#include "SysVersion.h"
#include "moreuuids.h"
#include "Monitors.h"
#include "MPCPngImage.h"

// CPPageOutput dialog

IMPLEMENT_DYNAMIC(CPPageOutput, CPPageBase)
CPPageOutput::CPPageOutput()
    : CPPageBase(CPPageOutput::IDD, CPPageOutput::IDD)
    , m_iDSVideoRendererType(VIDRNDT_DS_DEFAULT)
    , m_iRMVideoRendererType(VIDRNDT_RM_DEFAULT)
    , m_iQTVideoRendererType(VIDRNDT_QT_DEFAULT)
    , m_iAPSurfaceUsage(0)
    , m_iAudioRendererType(0)
    , m_iDX9Resizer(0)
    , m_fVMR9MixerMode(FALSE)
    , m_fVMR9MixerYUV(FALSE)
    , m_fVMR9AlterativeVSync(FALSE)
    , m_fResetDevice(FALSE)
    , m_iEvrBuffers(_T("5"))
    , m_fD3DFullscreen(FALSE)
    , m_fD3D9RenderDevice(FALSE)
    , m_iD3D9RenderDevice(-1)
    , m_tick(nullptr)
    , m_cross(nullptr)
{
}

CPPageOutput::~CPPageOutput()
{
    DestroyIcon(m_tick);
    DestroyIcon(m_cross);
}

void CPPageOutput::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_VIDRND_COMBO, m_iDSVideoRendererTypeCtrl);
    DDX_Control(pDX, IDC_RMRND_COMBO, m_iRMVideoRendererTypeCtrl);
    DDX_Control(pDX, IDC_QTRND_COMBO, m_iQTVideoRendererTypeCtrl);
    DDX_Control(pDX, IDC_AUDRND_COMBO, m_iAudioRendererTypeCtrl);
    DDX_Control(pDX, IDC_D3D9DEVICE_COMBO, m_iD3D9RenderDeviceCtrl);
    DDX_Control(pDX, IDC_DX_SURFACE, m_APSurfaceUsageCtrl);
    DDX_Control(pDX, IDC_DX9RESIZER_COMBO, m_DX9ResizerCtrl);
    DDX_Control(pDX, IDC_EVR_BUFFERS, m_EVRBuffersCtrl);
    DDX_Control(pDX, IDC_VIDRND_DXVA_SUPPORT, m_iDSDXVASupport);
    DDX_Control(pDX, IDC_VIDRND_SUBTITLE_SUPPORT, m_iDSSubtitleSupport);
    DDX_Control(pDX, IDC_VIDRND_SAVEIMAGE_SUPPORT, m_iDSSaveImageSupport);
    DDX_Control(pDX, IDC_VIDRND_SHADER_SUPPORT, m_iDSShaderSupport);
    DDX_Control(pDX, IDC_VIDRND_ROTATION_SUPPORT, m_iDSRotationSupport);
    DDX_Control(pDX, IDC_RMRND_SUBTITLE_SUPPORT, m_iRMSubtitleSupport);
    DDX_Control(pDX, IDC_RMRND_SAVEIMAGE_SUPPORT, m_iRMSaveImageSupport);
    DDX_Control(pDX, IDC_QTRND_SUBTITLE_SUPPORT, m_iQTSubtitleSupport);
    DDX_Control(pDX, IDC_QTRND_SAVEIMAGE_SUPPORT, m_iQTSaveImageSupport);
    DDX_CBIndex(pDX, IDC_RMRND_COMBO, m_iRMVideoRendererType);
    DDX_CBIndex(pDX, IDC_QTRND_COMBO, m_iQTVideoRendererType);
    DDX_CBIndex(pDX, IDC_AUDRND_COMBO, m_iAudioRendererType);
    DDX_CBIndex(pDX, IDC_DX_SURFACE, m_iAPSurfaceUsage);
    DDX_CBIndex(pDX, IDC_DX9RESIZER_COMBO, m_iDX9Resizer);
    DDX_CBIndex(pDX, IDC_D3D9DEVICE_COMBO, m_iD3D9RenderDevice);
    DDX_Check(pDX, IDC_D3D9DEVICE, m_fD3D9RenderDevice);
    DDX_Check(pDX, IDC_RESETDEVICE, m_fResetDevice);
    DDX_Check(pDX, IDC_FULLSCREEN_MONITOR_CHECK, m_fD3DFullscreen);
    DDX_Check(pDX, IDC_DSVMR9ALTERNATIVEVSYNC, m_fVMR9AlterativeVSync);
    DDX_Check(pDX, IDC_DSVMR9LOADMIXER, m_fVMR9MixerMode);
    DDX_Check(pDX, IDC_DSVMR9YUVMIXER, m_fVMR9MixerYUV);
    DDX_CBString(pDX, IDC_EVR_BUFFERS, m_iEvrBuffers);
}

BEGIN_MESSAGE_MAP(CPPageOutput, CPPageBase)
    ON_CBN_SELCHANGE(IDC_VIDRND_COMBO, &CPPageOutput::OnDSRendererChange)
    ON_CBN_SELCHANGE(IDC_RMRND_COMBO, &CPPageOutput::OnRMRendererChange)
    ON_CBN_SELCHANGE(IDC_QTRND_COMBO, &CPPageOutput::OnQTRendererChange)
    ON_CBN_SELCHANGE(IDC_DX_SURFACE, &CPPageOutput::OnSurfaceChange)
    ON_BN_CLICKED(IDC_D3D9DEVICE, OnD3D9DeviceCheck)
    ON_BN_CLICKED(IDC_FULLSCREEN_MONITOR_CHECK, OnFullscreenCheck)
    ON_UPDATE_COMMAND_UI(IDC_DSVMR9YUVMIXER, OnUpdateMixerYUV)
END_MESSAGE_MAP()

// CPPageOutput message handlers

BOOL CPPageOutput::OnInitDialog()
{
    __super::OnInitDialog();

    SetHandCursor(m_hWnd, IDC_AUDRND_COMBO);

    const CAppSettings& s = AfxGetAppSettings();
    const CRenderersSettings& r = s.m_RenderersSettings;

    m_iDSVideoRendererType  = s.iDSVideoRendererType;
    m_iRMVideoRendererType  = s.iRMVideoRendererType;
    m_iQTVideoRendererType  = s.iQTVideoRendererType;

    m_APSurfaceUsageCtrl.AddString(ResStr(IDS_PPAGE_OUTPUT_SURF_OFFSCREEN));
    m_APSurfaceUsageCtrl.AddString(ResStr(IDS_PPAGE_OUTPUT_SURF_2D));
    m_APSurfaceUsageCtrl.AddString(ResStr(IDS_PPAGE_OUTPUT_SURF_3D));
    CorrectComboListWidth(m_APSurfaceUsageCtrl);
    m_iAPSurfaceUsage       = r.iAPSurfaceUsage;

    m_DX9ResizerCtrl.AddString(ResStr(IDS_PPAGE_OUTPUT_RESIZE_NN));
    m_DX9ResizerCtrl.AddString(ResStr(IDS_PPAGE_OUTPUT_RESIZER_BILIN));
    m_DX9ResizerCtrl.AddString(ResStr(IDS_PPAGE_OUTPUT_RESIZER_BIL_PS));
    m_DX9ResizerCtrl.AddString(ResStr(IDS_PPAGE_OUTPUT_RESIZER_BICUB1));
    m_DX9ResizerCtrl.AddString(ResStr(IDS_PPAGE_OUTPUT_RESIZER_BICUB2));
    m_DX9ResizerCtrl.AddString(ResStr(IDS_PPAGE_OUTPUT_RESIZER_BICUB3));
    m_iDX9Resizer           = r.iDX9Resizer;

    m_fVMR9MixerMode        = r.fVMR9MixerMode;
    m_fVMR9MixerYUV         = r.fVMR9MixerYUV;
    m_fVMR9AlterativeVSync  = r.m_AdvRendSets.fVMR9AlterativeVSync;
    m_fD3DFullscreen        = s.fD3DFullscreen;

    int EVRBuffers[] = { 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 25, 30, 35, 40, 45, 50, 55, 60 };
    CString EVRBuffer;
    for (size_t i = 0; i < _countof(EVRBuffers); i++) {
        EVRBuffer.Format(_T("%d"), EVRBuffers[i]);
        m_EVRBuffersCtrl.AddString(EVRBuffer);
    }
    m_iEvrBuffers.Format(_T("%d"), r.iEvrBuffers);

    m_iAudioRendererTypeCtrl.SetRedraw(FALSE);
    m_fResetDevice = s.m_RenderersSettings.fResetDevice;
    m_AudioRendererDisplayNames.Add(_T(""));
    m_iAudioRendererTypeCtrl.AddString(_T("1: ") + ResStr(IDS_PPAGE_OUTPUT_SYS_DEF));
    m_iAudioRendererType = 0;

    int i = 2;
    CString Cbstr;

    BeginEnumSysDev(CLSID_AudioRendererCategory, pMoniker) {
        LPOLESTR olestr = nullptr;
        if (FAILED(pMoniker->GetDisplayName(0, 0, &olestr))) {
            continue;
        }

        CStringW str(olestr);
        CoTaskMemFree(olestr);

        m_AudioRendererDisplayNames.Add(CString(str));

        CComPtr<IPropertyBag> pPB;
        if (SUCCEEDED(pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPB))) {
            CComVariant var;
            pPB->Read(CComBSTR(_T("FriendlyName")), &var, nullptr);

            CString fstr(var.bstrVal);

            var.Clear();
            if (SUCCEEDED(pPB->Read(CComBSTR(_T("FilterData")), &var, nullptr))) {
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
            m_iAudioRendererType = m_iAudioRendererTypeCtrl.GetCount() - 1;
        }
        i++;
    }
    EndEnumSysDev;

    Cbstr.Format(_T("%d: %s"), i++, AUDRNDT_NULL_COMP);
    m_AudioRendererDisplayNames.Add(AUDRNDT_NULL_COMP);
    m_iAudioRendererTypeCtrl.AddString(Cbstr);
    if (s.strAudioRendererDisplayName == AUDRNDT_NULL_COMP && m_iAudioRendererType == 0) {
        m_iAudioRendererType = m_iAudioRendererTypeCtrl.GetCount() - 1;
    }

    Cbstr.Format(_T("%d: %s"), i++, AUDRNDT_NULL_UNCOMP);
    m_AudioRendererDisplayNames.Add(AUDRNDT_NULL_UNCOMP);
    m_iAudioRendererTypeCtrl.AddString(Cbstr);
    if (s.strAudioRendererDisplayName == AUDRNDT_NULL_UNCOMP && m_iAudioRendererType == 0) {
        m_iAudioRendererType = m_iAudioRendererTypeCtrl.GetCount() - 1;
    }

    Cbstr.Format(_T("%d: %s"), i++, AUDRNDT_MPC);
    m_AudioRendererDisplayNames.Add(AUDRNDT_MPC);
    m_iAudioRendererTypeCtrl.AddString(Cbstr);
    if (s.strAudioRendererDisplayName == AUDRNDT_MPC && m_iAudioRendererType == 0) {
        m_iAudioRendererType = m_iAudioRendererTypeCtrl.GetCount() - 1;
    }

    CorrectComboListWidth(m_iAudioRendererTypeCtrl);
    m_iAudioRendererTypeCtrl.SetRedraw(TRUE);
    m_iAudioRendererTypeCtrl.Invalidate();
    m_iAudioRendererTypeCtrl.UpdateWindow();

    IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (pD3D) {
        TCHAR strGUID[50];
        CString cstrGUID;
        CString d3ddevice_str = _T("");
        CStringArray adapterList;

        D3DADAPTER_IDENTIFIER9 adapterIdentifier;

        for (UINT adp = 0; adp < pD3D->GetAdapterCount(); ++adp) {
            if (SUCCEEDED(pD3D->GetAdapterIdentifier(adp, 0, &adapterIdentifier))) {
                d3ddevice_str = adapterIdentifier.Description;
                d3ddevice_str += _T(" - ");
                d3ddevice_str += adapterIdentifier.DeviceName;
                cstrGUID = _T("");
                if (::StringFromGUID2(adapterIdentifier.DeviceIdentifier, strGUID, 50) > 0) {
                    cstrGUID = strGUID;
                }
                if (!cstrGUID.IsEmpty()) {
                    boolean m_find = false;
                    for (INT_PTR j = 0; !m_find && (j < m_D3D9GUIDNames.GetCount()); j++) {
                        if (m_D3D9GUIDNames.GetAt(j) == cstrGUID) {
                            m_find = true;
                        }
                    }
                    if (!m_find) {
                        m_iD3D9RenderDeviceCtrl.AddString(d3ddevice_str);
                        m_D3D9GUIDNames.Add(cstrGUID);
                        if (r.D3D9RenderDevice == cstrGUID) {
                            m_iD3D9RenderDevice = m_iD3D9RenderDeviceCtrl.GetCount() - 1;
                        }
                    }
                }
            }
        }
        pD3D->Release();
    }
    CorrectComboListWidth(m_iD3D9RenderDeviceCtrl);

    CComboBox& m_iDSVRTC = m_iDSVideoRendererTypeCtrl;
    m_iDSVRTC.SetRedraw(FALSE); // Do not draw the control while we are filling it with items
    m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_SYS_DEF)), VIDRNDT_DS_DEFAULT);
    m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_OLDRENDERER)), VIDRNDT_DS_OLDRENDERER);
    m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_OVERLAYMIXER)), VIDRNDT_DS_OVERLAYMIXER);
    m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_VMR7WINDOWED)), VIDRNDT_DS_VMR7WINDOWED);
    m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_VMR9WINDOWED)), VIDRNDT_DS_VMR9WINDOWED);
    m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_VMR7RENDERLESS)), VIDRNDT_DS_VMR7RENDERLESS);
    m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_VMR9RENDERLESS)), VIDRNDT_DS_VMR9RENDERLESS);
    if (IsCLSIDRegistered(CLSID_EnhancedVideoRenderer)) {
        m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_EVR)), VIDRNDT_DS_EVR);
        m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_EVR_CUSTOM)), VIDRNDT_DS_EVR_CUSTOM);
        m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_SYNC)), VIDRNDT_DS_SYNC);
    } else {
        CString str;
        str.Format(_T("%s %s"), ResStr(IDS_PPAGE_OUTPUT_EVR), ResStr(IDS_PPAGE_OUTPUT_UNAVAILABLE));
        m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(str), VIDRNDT_DS_EVR);
        str.Format(_T("%s %s"), ResStr(IDS_PPAGE_OUTPUT_EVR_CUSTOM), ResStr(IDS_PPAGE_OUTPUT_UNAVAILABLE));
        m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(str), VIDRNDT_DS_EVR_CUSTOM);
        str.Format(_T("%s %s"), ResStr(IDS_PPAGE_OUTPUT_SYNC), ResStr(IDS_PPAGE_OUTPUT_UNAVAILABLE));
        m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(str), VIDRNDT_DS_SYNC);
    }
    if (IsCLSIDRegistered(CLSID_DXR)) {
        m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_DXR)), VIDRNDT_DS_DXR);
    } else {
        CString str;
        str.Format(_T("%s %s"), ResStr(IDS_PPAGE_OUTPUT_DXR), ResStr(IDS_PPAGE_OUTPUT_UNAVAILABLE));
        m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(str), VIDRNDT_DS_DXR);
    }
    m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_NULL_COMP)), VIDRNDT_DS_NULL_COMP);
    m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_NULL_UNCOMP)), VIDRNDT_DS_NULL_UNCOMP);
    if (IsCLSIDRegistered(CLSID_madVR)) {
        m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_MADVR)), VIDRNDT_DS_MADVR);
    } else {
        CString str;
        str.Format(_T("%s %s"), ResStr(IDS_PPAGE_OUTPUT_MADVR), ResStr(IDS_PPAGE_OUTPUT_UNAVAILABLE));
        m_iDSVRTC.SetItemData(m_iDSVRTC.AddString(str), VIDRNDT_DS_MADVR);
    }

    for (int j = 0; j < m_iDSVRTC.GetCount(); ++j) {
        if (m_iDSVideoRendererType == m_iDSVRTC.GetItemData(j)) {
            m_iDSVRTC.SetCurSel(j);
            break;
        }
    }

    m_iDSVRTC.SetRedraw(TRUE);
    m_iDSVRTC.Invalidate();
    m_iDSVRTC.UpdateWindow();

    CComboBox& m_iQTVRTC = m_iQTVideoRendererTypeCtrl;
    m_iQTVRTC.SetItemData(m_iQTVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_SYS_DEF)), VIDRNDT_QT_DEFAULT);
    m_iQTVRTC.SetItemData(m_iQTVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_VMR7RENDERLESS)), VIDRNDT_QT_DX7);
    m_iQTVRTC.SetItemData(m_iQTVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_VMR9RENDERLESS)), VIDRNDT_QT_DX9);
    m_iQTVRTC.SetCurSel(m_iQTVideoRendererType);
    CorrectComboListWidth(m_iQTVRTC);

    CComboBox& m_iRMVRTC = m_iRMVideoRendererTypeCtrl;
    m_iRMVideoRendererTypeCtrl.SetItemData(m_iRMVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_SYS_DEF)), VIDRNDT_RM_DEFAULT);
    m_iRMVRTC.SetItemData(m_iRMVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_VMR7RENDERLESS)), VIDRNDT_RM_DX7);
    m_iRMVRTC.SetItemData(m_iRMVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_VMR9RENDERLESS)), VIDRNDT_RM_DX9);
    m_iRMVRTC.SetCurSel(m_iRMVideoRendererType);
    CorrectComboListWidth(m_iRMVRTC);

    UpdateData(FALSE);

    m_tickcross.Create(16, 16, ILC_COLOR32, 2, 0);
    CMPCPngImage tickcross;
    tickcross.Load(IDF_TICKCROSS);
    m_tickcross.Add(&tickcross, (CBitmap*)nullptr);
    m_tick = m_tickcross.ExtractIcon(0);
    m_cross = m_tickcross.ExtractIcon(1);

    CreateToolTip();

    m_wndToolTip.AddTool(GetDlgItem(IDC_RMRND_COMBO), ResStr(IDC_RMSYSDEF));
    m_wndToolTip.AddTool(GetDlgItem(IDC_QTRND_COMBO), ResStr(IDC_QTSYSDEF));
    m_wndToolTip.AddTool(GetDlgItem(IDC_DX_SURFACE), ResStr(IDC_REGULARSURF));

    OnDSRendererChange();
    OnRMRendererChange();
    OnQTRendererChange();
    OnSurfaceChange();

    // YUV mixing is incompatible with Vista+
    if (SysVersion::IsVistaOrLater()) {
        GetDlgItem(IDC_DSVMR9YUVMIXER)->EnableWindow(FALSE);
    }

    CheckDlgButton(IDC_D3D9DEVICE, BST_CHECKED);
    GetDlgItem(IDC_D3D9DEVICE)->EnableWindow(TRUE);
    GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(TRUE);

    if ((m_iDSVideoRendererType == VIDRNDT_DS_VMR9RENDERLESS || m_iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM) && (m_iD3D9RenderDeviceCtrl.GetCount() > 1)) {
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

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageOutput::OnApply()
{
    UpdateData();

    CAppSettings& s = AfxGetAppSettings();

    if (!IsRenderTypeAvailable(m_iDSVideoRendererType)) {
        ((CPropertySheet*)GetParent())->SetActivePage(this);
        AfxMessageBox(IDS_PPAGE_OUTPUT_UNAVAILABLEMSG, MB_ICONEXCLAMATION | MB_OK, 0);

        // revert to the renderer in the settings
        m_iDSVideoRendererTypeCtrl.SetCurSel(0);
        for (int i = 0; i < m_iDSVideoRendererTypeCtrl.GetCount(); ++i) {
            if (s.iDSVideoRendererType == m_iDSVideoRendererTypeCtrl.GetItemData(i)) {
                m_iDSVideoRendererTypeCtrl.SetCurSel(i);
                break;
            }
        }
        OnDSRendererChange();

        return FALSE;
    }

    CRenderersSettings& r                   = s.m_RenderersSettings;
    s.iDSVideoRendererType                  = m_iDSVideoRendererType;
    s.iRMVideoRendererType                  = m_iRMVideoRendererType;
    s.iQTVideoRendererType                  = m_iQTVideoRendererType;
    r.iAPSurfaceUsage                       = m_iAPSurfaceUsage;
    r.iDX9Resizer                           = m_iDX9Resizer;
    r.fVMR9MixerMode                        = !!m_fVMR9MixerMode;
    r.fVMR9MixerYUV                         = !!m_fVMR9MixerYUV;
    r.m_AdvRendSets.fVMR9AlterativeVSync    = m_fVMR9AlterativeVSync != 0;
    s.strAudioRendererDisplayName           = m_AudioRendererDisplayNames[m_iAudioRendererType];
    s.fD3DFullscreen                        = m_fD3DFullscreen ? true : false;

    r.fResetDevice = !!m_fResetDevice;

    if (m_iEvrBuffers.IsEmpty() || _stscanf_s(m_iEvrBuffers, _T("%d"), &r.iEvrBuffers) != 1) {
        r.iEvrBuffers = 5;
    }

    r.D3D9RenderDevice = m_fD3D9RenderDevice ? m_D3D9GUIDNames[m_iD3D9RenderDevice] : _T("");

    return __super::OnApply();
}

void CPPageOutput::OnUpdateMixerYUV(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!!IsDlgButtonChecked(IDC_DSVMR9LOADMIXER) && (m_iDSVideoRendererType == VIDRNDT_DS_VMR9RENDERLESS) && !SysVersion::IsVistaOrLater());
}

void CPPageOutput::OnSurfaceChange()
{
    UpdateData();

    switch (m_iAPSurfaceUsage) {
        case VIDRNDT_AP_SURFACE:
            m_iDSShaderSupport.SetIcon(m_cross);
            m_iDSRotationSupport.SetIcon(m_cross);
            m_wndToolTip.UpdateTipText(ResStr(IDC_REGULARSURF), GetDlgItem(IDC_DX_SURFACE));
            break;
        case VIDRNDT_AP_TEXTURE2D:
            m_iDSShaderSupport.SetIcon(m_cross);
            m_iDSRotationSupport.SetIcon(m_cross);
            m_wndToolTip.UpdateTipText(ResStr(IDC_TEXTURESURF2D), GetDlgItem(IDC_DX_SURFACE));
            break;
        case VIDRNDT_AP_TEXTURE3D:
            if (m_iDSVideoRendererType == VIDRNDT_DS_VMR9RENDERLESS || m_iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM) {
                m_iDSShaderSupport.SetIcon(m_tick);
                m_iDSRotationSupport.SetIcon(m_tick);
            } else if (m_iDSVideoRendererType == VIDRNDT_DS_MADVR) {
                m_iDSShaderSupport.SetIcon(m_tick);
                m_iDSRotationSupport.SetIcon(m_cross);
            } else {
                m_iDSShaderSupport.SetIcon(m_cross);
                m_iDSRotationSupport.SetIcon(m_cross);
            }
            m_wndToolTip.UpdateTipText(ResStr(IDC_TEXTURESURF3D), GetDlgItem(IDC_DX_SURFACE));
            break;
    }

    SetModified();
}

void CPPageOutput::OnDSRendererChange()
{
    UpdateData();
    m_iDSVideoRendererType = (int)m_iDSVideoRendererTypeCtrl.GetItemData(m_iDSVideoRendererTypeCtrl.GetCurSel());

    GetDlgItem(IDC_DX_SURFACE)->EnableWindow(FALSE);
    GetDlgItem(IDC_DX9RESIZER_COMBO)->EnableWindow(FALSE);
    GetDlgItem(IDC_FULLSCREEN_MONITOR_CHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_DSVMR9LOADMIXER)->EnableWindow(FALSE);
    GetDlgItem(IDC_DSVMR9YUVMIXER)->EnableWindow(FALSE);
    GetDlgItem(IDC_DSVMR9ALTERNATIVEVSYNC)->EnableWindow(FALSE);
    GetDlgItem(IDC_RESETDEVICE)->EnableWindow(FALSE);
    GetDlgItem(IDC_EVR_BUFFERS)->EnableWindow(m_iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM);
    GetDlgItem(IDC_EVR_BUFFERS_TXT)->EnableWindow(m_iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM);

    GetDlgItem(IDC_D3D9DEVICE)->EnableWindow(FALSE);
    GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(FALSE);

    m_iDSDXVASupport.SetRedraw(FALSE);
    m_iDSSubtitleSupport.SetRedraw(FALSE);
    m_iDSSaveImageSupport.SetRedraw(FALSE);
    m_iDSShaderSupport.SetRedraw(FALSE);
    m_iDSRotationSupport.SetRedraw(FALSE);

    m_iDSDXVASupport.SetIcon(m_cross);
    m_iDSSubtitleSupport.SetIcon(m_cross);
    m_iDSSaveImageSupport.SetIcon(m_cross);
    m_iDSShaderSupport.SetIcon(m_cross);
    m_iDSRotationSupport.SetIcon(m_cross);

    m_wndToolTip.UpdateTipText(ResStr(IDC_VIDRND_COMBO), GetDlgItem(IDC_VIDRND_COMBO));

    switch (m_iDSVideoRendererType) {
        case VIDRNDT_DS_DEFAULT:
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSSYSDEF), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_OLDRENDERER:
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSOLD), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_OVERLAYMIXER:
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSOVERLAYMIXER), GetDlgItem(IDC_VIDRND_COMBO));
            if (!SysVersion::IsVistaOrLater()) {
                m_iDSDXVASupport.SetIcon(m_tick);
            }
            break;
        case VIDRNDT_DS_VMR7WINDOWED:
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSVMR7WIN), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_VMR9WINDOWED:
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSVMR9WIN), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_EVR:
            if (SysVersion::IsVistaOrLater()) {
                m_iDSDXVASupport.SetIcon(m_tick);
            }
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSEVR), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_NULL_COMP:
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSNULL_COMP), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_NULL_UNCOMP:
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSNULL_UNCOMP), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_VMR7RENDERLESS:
            GetDlgItem(IDC_DX_SURFACE)->EnableWindow(TRUE);

            if (!SysVersion::IsVistaOrLater()) {
                m_iDSDXVASupport.SetIcon(m_tick);
            }
            m_iDSSubtitleSupport.SetIcon(m_tick);
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSVMR7REN), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_VMR9RENDERLESS:
            GetDlgItem(IDC_DSVMR9LOADMIXER)->EnableWindow(TRUE);
            GetDlgItem(IDC_DSVMR9YUVMIXER)->EnableWindow(TRUE);
            GetDlgItem(IDC_DSVMR9ALTERNATIVEVSYNC)->EnableWindow(TRUE);
            GetDlgItem(IDC_RESETDEVICE)->EnableWindow(TRUE);

            m_wndToolTip.UpdateTipText(ResStr(IDC_DSVMR9REN), GetDlgItem(IDC_VIDRND_COMBO));
        case VIDRNDT_DS_EVR_CUSTOM:
            if (m_iD3D9RenderDeviceCtrl.GetCount() > 1) {
                GetDlgItem(IDC_D3D9DEVICE)->EnableWindow(TRUE);
                GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(IsDlgButtonChecked(IDC_D3D9DEVICE));
            }

            GetDlgItem(IDC_DX9RESIZER_COMBO)->EnableWindow(TRUE);
            GetDlgItem(IDC_FULLSCREEN_MONITOR_CHECK)->EnableWindow(TRUE);
            GetDlgItem(IDC_DSVMR9ALTERNATIVEVSYNC)->EnableWindow(TRUE);
            GetDlgItem(IDC_RESETDEVICE)->EnableWindow(TRUE);

            // Force 3D surface with EVR Custom
            if (m_iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM) {
                GetDlgItem(IDC_DX_SURFACE)->EnableWindow(FALSE);
                ((CComboBox*)GetDlgItem(IDC_DX_SURFACE))->SetCurSel(2);

                if (SysVersion::IsVistaOrLater()) {
                    m_iDSDXVASupport.SetIcon(m_tick);
                }
                m_iDSShaderSupport.SetIcon(m_tick);
                m_iDSRotationSupport.SetIcon(m_tick);
                m_wndToolTip.UpdateTipText(ResStr(IDC_DSEVR_CUSTOM), GetDlgItem(IDC_VIDRND_COMBO));
            } else {
                GetDlgItem(IDC_DX_SURFACE)->EnableWindow(TRUE);

                if (!SysVersion::IsVistaOrLater()) {
                    m_iDSDXVASupport.SetIcon(m_tick);
                }
                if (m_iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D) {
                    m_iDSShaderSupport.SetIcon(m_tick);
                    m_iDSRotationSupport.SetIcon(m_tick);
                }
            }

            m_iDSSubtitleSupport.SetIcon(m_tick);
            m_iDSSaveImageSupport.SetIcon(m_tick);
            break;
        case VIDRNDT_DS_SYNC:
            GetDlgItem(IDC_EVR_BUFFERS)->EnableWindow(TRUE);
            GetDlgItem(IDC_EVR_BUFFERS_TXT)->EnableWindow(TRUE);
            GetDlgItem(IDC_DX9RESIZER_COMBO)->EnableWindow(TRUE);
            GetDlgItem(IDC_FULLSCREEN_MONITOR_CHECK)->EnableWindow(TRUE);
            GetDlgItem(IDC_RESETDEVICE)->EnableWindow(TRUE);
            GetDlgItem(IDC_DX_SURFACE)->EnableWindow(FALSE);
            ((CComboBox*)GetDlgItem(IDC_DX_SURFACE))->SetCurSel(2);

            m_iDSDXVASupport.SetIcon(m_tick);
            m_iDSSubtitleSupport.SetIcon(m_tick);
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_iDSShaderSupport.SetIcon(m_tick);
            m_iDSRotationSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSSYNC), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_MADVR:
            GetDlgItem(IDC_FULLSCREEN_MONITOR_CHECK)->EnableWindow(TRUE);
            ((CComboBox*)GetDlgItem(IDC_DX_SURFACE))->SetCurSel(2);

            m_iDSDXVASupport.SetIcon(m_tick);
            m_iDSSubtitleSupport.SetIcon(m_tick);
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_iDSShaderSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSMADVR), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_DXR:
            m_iDSSubtitleSupport.SetIcon(m_tick);
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSDXR), GetDlgItem(IDC_VIDRND_COMBO));
            break;
    }

    m_iDSDXVASupport.SetRedraw(TRUE);
    m_iDSDXVASupport.Invalidate();
    m_iDSDXVASupport.UpdateWindow();
    m_iDSSubtitleSupport.SetRedraw(TRUE);
    m_iDSSubtitleSupport.Invalidate();
    m_iDSSubtitleSupport.UpdateWindow();
    m_iDSSaveImageSupport.SetRedraw(TRUE);
    m_iDSSaveImageSupport.Invalidate();
    m_iDSSaveImageSupport.UpdateWindow();
    m_iDSShaderSupport.SetRedraw(TRUE);
    m_iDSShaderSupport.Invalidate();
    m_iDSShaderSupport.UpdateWindow();
    m_iDSRotationSupport.SetRedraw(TRUE);
    m_iDSRotationSupport.Invalidate();
    m_iDSRotationSupport.UpdateWindow();

    SetModified();
}

void CPPageOutput::OnRMRendererChange()
{
    UpdateData();

    switch (m_iRMVideoRendererType) {
        case VIDRNDT_RM_DEFAULT:
            m_iRMSaveImageSupport.SetIcon(m_cross);
            m_iRMSubtitleSupport.SetIcon(m_cross);

            m_wndToolTip.UpdateTipText(ResStr(IDC_RMSYSDEF), GetDlgItem(IDC_RMRND_COMBO));
            break;
        case VIDRNDT_RM_DX7:
            m_iRMSaveImageSupport.SetIcon(m_tick);
            m_iRMSubtitleSupport.SetIcon(m_tick);

            m_wndToolTip.UpdateTipText(ResStr(IDC_RMDX7), GetDlgItem(IDC_RMRND_COMBO));
            break;
        case VIDRNDT_RM_DX9:
            m_iRMSaveImageSupport.SetIcon(m_tick);
            m_iRMSubtitleSupport.SetIcon(m_tick);

            m_wndToolTip.UpdateTipText(ResStr(IDC_RMDX9), GetDlgItem(IDC_RMRND_COMBO));
            break;
    }

    SetModified();
}

void CPPageOutput::OnQTRendererChange()
{
    UpdateData();

    switch (m_iQTVideoRendererType) {
        case VIDRNDT_QT_DEFAULT:
            m_iQTSaveImageSupport.SetIcon(m_cross);
            m_iQTSubtitleSupport.SetIcon(m_cross);

            m_wndToolTip.UpdateTipText(ResStr(IDC_QTSYSDEF), GetDlgItem(IDC_QTRND_COMBO));
            break;
        case VIDRNDT_QT_DX7:
            m_iQTSaveImageSupport.SetIcon(m_tick);
            m_iQTSubtitleSupport.SetIcon(m_tick);

            m_wndToolTip.UpdateTipText(ResStr(IDC_QTDX7), GetDlgItem(IDC_QTRND_COMBO));
            break;
        case VIDRNDT_QT_DX9:
            m_iQTSaveImageSupport.SetIcon(m_tick);
            m_iQTSubtitleSupport.SetIcon(m_tick);

            m_wndToolTip.UpdateTipText(ResStr(IDC_QTDX9), GetDlgItem(IDC_QTRND_COMBO));
            break;
    }

    SetModified();
}

void CPPageOutput::OnFullscreenCheck()
{
    UpdateData();
    if (m_fD3DFullscreen &&
            (MessageBox(ResStr(IDS_D3DFS_WARNING), nullptr, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDNO)) {
        m_fD3DFullscreen = false;
        UpdateData(FALSE);
    } else {
        SetModified();
    }
}

void CPPageOutput::OnD3D9DeviceCheck()
{
    UpdateData();
    GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(m_fD3D9RenderDevice);
    SetModified();
}

bool CPPageOutput::IsRenderTypeAvailable(int VideoRendererType)
{
    switch (m_iDSVideoRendererType) {
        case VIDRNDT_DS_EVR:
        case VIDRNDT_DS_EVR_CUSTOM:
        case VIDRNDT_DS_SYNC:
            return IsCLSIDRegistered(CLSID_EnhancedVideoRenderer);
        case VIDRNDT_DS_DXR:
            return IsCLSIDRegistered(CLSID_DXR);
        case VIDRNDT_DS_MADVR:
            return IsCLSIDRegistered(CLSID_madVR);
        default:
            return true;
    }
}
