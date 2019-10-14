/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2017 see Authors.txt
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
#include "moreuuids.h"
#include "Monitors.h"
#include "MPCPngImage.h"
#include <WinapiFunc.h>

// CPPageOutput dialog

IMPLEMENT_DYNAMIC(CPPageOutput, CMPCThemePPageBase)
CPPageOutput::CPPageOutput()
    : CMPCThemePPageBase(CPPageOutput::IDD, CPPageOutput::IDD)
    , m_tick(nullptr)
    , m_cross(nullptr)
    , m_iDSVideoRendererType(VIDRNDT_DS_DEFAULT)
    , m_iRMVideoRendererType(VIDRNDT_RM_DEFAULT)
    , m_iQTVideoRendererType(VIDRNDT_QT_DEFAULT)
    , m_iAPSurfaceUsage(0)
    , m_iAudioRendererType(0)
    , m_lastSubrenderer{false, CAppSettings::SubtitleRenderer::INTERNAL}
    , m_iDX9Resizer(0)
    , m_fVMR9MixerMode(FALSE)
    , m_fD3DFullscreen(FALSE)
    , m_fVMR9AlterativeVSync(FALSE)
    , m_fResetDevice(FALSE)
    , m_fCacheShaders(FALSE)
    , m_iEvrBuffers(_T("5"))
    , m_fD3D9RenderDevice(FALSE)
    , m_iD3D9RenderDevice(-1)
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
    DDX_Control(pDX, IDC_COMBO1, m_SubtitleRendererCtrl);
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
    DDX_CBIndex(pDX, IDC_AUDRND_COMBO, m_iAudioRendererType);
    DDX_CBIndex(pDX, IDC_DX_SURFACE, m_iAPSurfaceUsage);
    DDX_CBIndex(pDX, IDC_DX9RESIZER_COMBO, m_iDX9Resizer);
    DDX_CBIndex(pDX, IDC_D3D9DEVICE_COMBO, m_iD3D9RenderDevice);
    DDX_Check(pDX, IDC_D3D9DEVICE, m_fD3D9RenderDevice);
    DDX_Check(pDX, IDC_RESETDEVICE, m_fResetDevice);
    DDX_Check(pDX, IDC_CACHESHADERS, m_fCacheShaders);
    DDX_Check(pDX, IDC_FULLSCREEN_MONITOR_CHECK, m_fD3DFullscreen);
    DDX_Check(pDX, IDC_DSVMR9ALTERNATIVEVSYNC, m_fVMR9AlterativeVSync);
    DDX_Check(pDX, IDC_DSVMR9LOADMIXER, m_fVMR9MixerMode);
    DDX_CBString(pDX, IDC_EVR_BUFFERS, m_iEvrBuffers);
}

BEGIN_MESSAGE_MAP(CPPageOutput, CMPCThemePPageBase)
    ON_CBN_SELCHANGE(IDC_VIDRND_COMBO, &CPPageOutput::OnDSRendererChange)
    ON_CBN_SELCHANGE(IDC_RMRND_COMBO, &CPPageOutput::OnRMRendererChange)
    ON_CBN_SELCHANGE(IDC_QTRND_COMBO, &CPPageOutput::OnQTRendererChange)
    ON_CBN_SELCHANGE(IDC_COMBO1, &CPPageOutput::OnSubtitleRendererChange)
    ON_CBN_SELCHANGE(IDC_DX_SURFACE, &CPPageOutput::OnSurfaceChange)
    ON_BN_CLICKED(IDC_D3D9DEVICE, OnD3D9DeviceCheck)
    ON_BN_CLICKED(IDC_FULLSCREEN_MONITOR_CHECK, OnFullscreenCheck)
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
    m_fVMR9AlterativeVSync  = r.m_AdvRendSets.bVMR9AlterativeVSync;
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
    m_fCacheShaders = s.m_RenderersSettings.m_AdvRendSets.bCacheShaders;
    m_AudioRendererDisplayNames.Add(_T(""));
    m_iAudioRendererTypeCtrl.AddString(_T("1: ") + ResStr(IDS_PPAGE_OUTPUT_SYS_DEF));
    m_iAudioRendererType = 0;

    int i = 2;
    CString Cbstr;

    BeginEnumSysDev(CLSID_AudioRendererCategory, pMoniker) {
        CComHeapPtr<OLECHAR> olestr;
        if (FAILED(pMoniker->GetDisplayName(0, 0, &olestr))) {
            continue;
        }

        CStringW str(olestr);

        m_AudioRendererDisplayNames.Add(CString(str));

        CComPtr<IPropertyBag> pPB;
        if (SUCCEEDED(pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPB)))) {
            CComVariant var;
            if (SUCCEEDED(pPB->Read(_T("FriendlyName"), &var, nullptr))) {
                CString fstr(var.bstrVal);

                var.Clear();
                if (SUCCEEDED(pPB->Read(_T("FilterData"), &var, nullptr))) {
                    BSTR* pbstr;
                    if (SUCCEEDED(SafeArrayAccessData(var.parray, (void**)&pbstr))) {
                        fstr.AppendFormat(_T(" (%08x)"), *((DWORD*)pbstr + 1));
                        SafeArrayUnaccessData(var.parray);
                    }
                }
                Cbstr.Format(_T("%d: %s"), i, fstr.GetString());
            }
        } else {
            Cbstr.Format(_T("%d: %s"), i, str.GetString());
        }
        m_iAudioRendererTypeCtrl.AddString(Cbstr);

        if (s.strAudioRendererDisplayName == str && m_iAudioRendererType == 0) {
            m_iAudioRendererType = m_iAudioRendererTypeCtrl.GetCount() - 1;
        }
        i++;
    }
    EndEnumSysDev;

    Cbstr.Format(_T("%d: %s"), i++, ResStr(IDS_PPAGE_OUTPUT_AUD_NULL_COMP).GetString());
    m_AudioRendererDisplayNames.Add(AUDRNDT_NULL_COMP);
    m_iAudioRendererTypeCtrl.AddString(Cbstr);
    if (s.strAudioRendererDisplayName == AUDRNDT_NULL_COMP && m_iAudioRendererType == 0) {
        m_iAudioRendererType = m_iAudioRendererTypeCtrl.GetCount() - 1;
    }

    Cbstr.Format(_T("%d: %s"), i++, ResStr(IDS_PPAGE_OUTPUT_AUD_NULL_UNCOMP).GetString());
    m_AudioRendererDisplayNames.Add(AUDRNDT_NULL_UNCOMP);
    m_iAudioRendererTypeCtrl.AddString(Cbstr);
    if (s.strAudioRendererDisplayName == AUDRNDT_NULL_UNCOMP && m_iAudioRendererType == 0) {
        m_iAudioRendererType = m_iAudioRendererTypeCtrl.GetCount() - 1;
    }

    Cbstr.Format(_T("%d: %s"), i++, ResStr(IDS_PPAGE_OUTPUT_AUD_INTERNAL_REND).GetString());
    m_AudioRendererDisplayNames.Add(AUDRNDT_INTERNAL);
    m_iAudioRendererTypeCtrl.AddString(Cbstr);
    if (s.strAudioRendererDisplayName == AUDRNDT_INTERNAL && m_iAudioRendererType == 0) {
        m_iAudioRendererType = m_iAudioRendererTypeCtrl.GetCount() - 1;
    }

    CorrectComboListWidth(m_iAudioRendererTypeCtrl);
    m_iAudioRendererTypeCtrl.SetRedraw(TRUE);
    m_iAudioRendererTypeCtrl.Invalidate();
    m_iAudioRendererTypeCtrl.UpdateWindow();

    UpdateSubtitleRendererList();

    IDirect3D9* pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (pD3D9) {
        TCHAR strGUID[50];
        CString cstrGUID;
        CString d3ddevice_str = _T("");

        D3DADAPTER_IDENTIFIER9 adapterIdentifier;

        for (UINT adp = 0; adp < pD3D9->GetAdapterCount(); ++adp) {
            if (SUCCEEDED(pD3D9->GetAdapterIdentifier(adp, 0, &adapterIdentifier))) {
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
        pD3D9->Release();
    }
    CorrectComboListWidth(m_iD3D9RenderDeviceCtrl);

    auto addRenderer = [&](int nID) {
        WORD resName;

        switch (nID) {
            case VIDRNDT_DS_DEFAULT:
                resName = IDS_PPAGE_OUTPUT_SYS_DEF;
                break;
            case VIDRNDT_DS_OLDRENDERER:
                resName = IDS_PPAGE_OUTPUT_OLDRENDERER;
                break;
            case VIDRNDT_DS_OVERLAYMIXER:
                resName = IDS_PPAGE_OUTPUT_OVERLAYMIXER;
                break;
            case VIDRNDT_DS_VMR9WINDOWED:
                resName = IDS_PPAGE_OUTPUT_VMR9WINDOWED;
                break;
            case VIDRNDT_DS_VMR9RENDERLESS:
                resName = IDS_PPAGE_OUTPUT_VMR9RENDERLESS;
                break;
            case VIDRNDT_DS_DXR:
                resName = IDS_PPAGE_OUTPUT_DXR;
                break;
            case VIDRNDT_DS_NULL_COMP:
                resName = IDS_PPAGE_OUTPUT_NULL_COMP;
                break;
            case VIDRNDT_DS_NULL_UNCOMP:
                resName = IDS_PPAGE_OUTPUT_NULL_UNCOMP;
                break;
            case VIDRNDT_DS_EVR:
                resName = IDS_PPAGE_OUTPUT_EVR;
                break;
            case VIDRNDT_DS_EVR_CUSTOM:
                resName = IDS_PPAGE_OUTPUT_EVR_CUSTOM;
                break;
            case VIDRNDT_DS_MADVR:
                resName = IDS_PPAGE_OUTPUT_MADVR;
                break;
            case VIDRNDT_DS_SYNC:
                resName = IDS_PPAGE_OUTPUT_SYNC;
                break;
            default:
                ASSERT(FALSE);
                return;
        }

        CString sName(StrRes(resName));
        if (!s.IsVideoRendererAvailable(nID)) {
            sName.AppendFormat(_T(" %s"), ResStr(IDS_PPAGE_OUTPUT_UNAVAILABLE).GetString());
        }

        m_iDSVideoRendererTypeCtrl.SetItemData(m_iDSVideoRendererTypeCtrl.AddString(sName), nID);
    };

    CComboBox& m_iDSVRTC = m_iDSVideoRendererTypeCtrl;
    m_iDSVRTC.SetRedraw(FALSE); // Do not draw the control while we are filling it with items
    addRenderer(VIDRNDT_DS_DEFAULT);
    addRenderer(VIDRNDT_DS_OLDRENDERER);
    addRenderer(VIDRNDT_DS_OVERLAYMIXER);
    addRenderer(VIDRNDT_DS_VMR9WINDOWED);
    addRenderer(VIDRNDT_DS_VMR9RENDERLESS);
    addRenderer(VIDRNDT_DS_EVR);
    addRenderer(VIDRNDT_DS_EVR_CUSTOM);
    addRenderer(VIDRNDT_DS_SYNC);
    addRenderer(VIDRNDT_DS_DXR);
    addRenderer(VIDRNDT_DS_MADVR);
    addRenderer(VIDRNDT_DS_NULL_COMP);
    addRenderer(VIDRNDT_DS_NULL_UNCOMP);

    for (int j = 0; j < m_iDSVRTC.GetCount(); ++j) {
        if ((UINT)m_iDSVideoRendererType == m_iDSVRTC.GetItemData(j)) {
            m_iDSVRTC.SetCurSel(j);
            break;
        }
    }

    m_iDSVRTC.SetRedraw(TRUE);
    m_iDSVRTC.Invalidate();
    m_iDSVRTC.UpdateWindow();

    CComboBox& m_iQTVRTC = m_iQTVideoRendererTypeCtrl;
    m_iQTVRTC.SetItemData(m_iQTVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_SYS_DEF)), VIDRNDT_QT_DEFAULT);
    m_iQTVRTC.SetItemData(m_iQTVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_VMR9RENDERLESS)), VIDRNDT_QT_DX9);
    for (int j = 0; j < m_iQTVRTC.GetCount(); ++j) {
        if ((UINT)m_iQTVideoRendererType == m_iQTVRTC.GetItemData(j)) {
            m_iQTVRTC.SetCurSel(j);
            break;
        }
    }
    CorrectComboListWidth(m_iQTVRTC);

    CComboBox& m_iRMVRTC = m_iRMVideoRendererTypeCtrl;
    m_iRMVideoRendererTypeCtrl.SetItemData(m_iRMVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_SYS_DEF)), VIDRNDT_RM_DEFAULT);
    m_iRMVRTC.SetItemData(m_iRMVRTC.AddString(ResStr(IDS_PPAGE_OUTPUT_VMR9RENDERLESS)), VIDRNDT_RM_DX9);
    for (int j = 0; j < m_iRMVRTC.GetCount(); ++j) {
        if ((UINT)m_iRMVideoRendererType == m_iRMVRTC.GetItemData(j)) {
            m_iRMVRTC.SetCurSel(j);
            break;
        }
    }
    CorrectComboListWidth(m_iRMVRTC);

    UpdateData(FALSE);

    m_tickcross.Create(16, 16, ILC_COLOR32, 2, 0);
    CMPCPngImage tickcross;
    tickcross.Load(IDF_TICKCROSS);
    m_tickcross.Add(&tickcross, (CBitmap*)nullptr);
    m_tick = m_tickcross.ExtractIcon(0);
    m_cross = m_tickcross.ExtractIcon(1);

    CreateToolTip();

    m_wndToolTip.AddTool(GetDlgItem(IDC_VIDRND_COMBO), ResStr(IDC_DSSYSDEF));
    m_wndToolTip.AddTool(GetDlgItem(IDC_RMRND_COMBO), ResStr(IDC_RMSYSDEF));
    m_wndToolTip.AddTool(GetDlgItem(IDC_QTRND_COMBO), ResStr(IDC_QTSYSDEF));
    m_wndToolTip.AddTool(GetDlgItem(IDC_DX_SURFACE), ResStr(IDC_REGULARSURF));

    OnDSRendererChange();
    OnRMRendererChange();
    OnQTRendererChange();
    OnSurfaceChange();

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

    if (!s.IsVideoRendererAvailable(m_iDSVideoRendererType)) {
        ((CPropertySheet*)GetParent())->SetActivePage(this);
        AfxMessageBox(IDS_PPAGE_OUTPUT_UNAVAILABLEMSG, MB_ICONEXCLAMATION | MB_OK, 0);

        // revert to the renderer in the settings
        m_iDSVideoRendererTypeCtrl.SetCurSel(0);
        for (int i = 0; i < m_iDSVideoRendererTypeCtrl.GetCount(); ++i) {
            if ((UINT)s.iDSVideoRendererType == m_iDSVideoRendererTypeCtrl.GetItemData(i)) {
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
    r.m_AdvRendSets.bVMR9AlterativeVSync    = m_fVMR9AlterativeVSync != FALSE;
    s.strAudioRendererDisplayName           = m_AudioRendererDisplayNames[m_iAudioRendererType];
    s.fD3DFullscreen                        = m_fD3DFullscreen ? true : false;

    if (m_SubtitleRendererCtrl.IsWindowEnabled()) {
        auto subrenderer = static_cast<CAppSettings::SubtitleRenderer>(m_SubtitleRendererCtrl.GetItemData(m_SubtitleRendererCtrl.GetCurSel()));
        m_lastSubrenderer.first = true;
        m_lastSubrenderer.second = subrenderer;
        s.SetSubtitleRenderer(subrenderer);
    }

    r.fResetDevice = !!m_fResetDevice;
    r.m_AdvRendSets.bCacheShaders = !!m_fCacheShaders;

    if (m_iEvrBuffers.IsEmpty() || _stscanf_s(m_iEvrBuffers, _T("%d"), &r.iEvrBuffers) != 1) {
        r.iEvrBuffers = 5;
    }

    if (m_fD3D9RenderDevice) {
        r.D3D9RenderDevice = m_D3D9GUIDNames[m_iD3D9RenderDevice];
    } else {
        r.D3D9RenderDevice.Empty();
    }

    return __super::OnApply();
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
            if (m_iDSVideoRendererType == VIDRNDT_DS_VMR9RENDERLESS
                    || m_iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM || m_iDSVideoRendererType == VIDRNDT_DS_SYNC) {
                m_iDSShaderSupport.SetIcon(m_tick);
                m_iDSRotationSupport.SetIcon(m_tick);
            } else if (m_iDSVideoRendererType == VIDRNDT_DS_MADVR) {
                m_iDSShaderSupport.SetIcon(m_tick);
                m_iDSRotationSupport.SetIcon(m_tick);
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
    GetDlgItem(IDC_DSVMR9ALTERNATIVEVSYNC)->EnableWindow(FALSE);
    GetDlgItem(IDC_RESETDEVICE)->EnableWindow(FALSE);
    GetDlgItem(IDC_CACHESHADERS)->EnableWindow(FALSE);
    GetDlgItem(IDC_EVR_BUFFERS)->EnableWindow(m_iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM);
    GetDlgItem(IDC_EVR_BUFFERS_TXT)->EnableWindow(m_iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM);

    GetDlgItem(IDC_D3D9DEVICE)->EnableWindow(FALSE);
    GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(FALSE);

    m_iDSDXVASupport.SetRedraw(FALSE);
    m_iDSSaveImageSupport.SetRedraw(FALSE);
    m_iDSShaderSupport.SetRedraw(FALSE);
    m_iDSRotationSupport.SetRedraw(FALSE);

    m_iDSDXVASupport.SetIcon(m_cross);
    m_iDSSaveImageSupport.SetIcon(m_cross);
    m_iDSShaderSupport.SetIcon(m_cross);
    m_iDSRotationSupport.SetIcon(m_cross);

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
            break;
        case VIDRNDT_DS_VMR9WINDOWED:
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSVMR9WIN), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_EVR:
            m_iDSDXVASupport.SetIcon(m_tick);
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(_T(""), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_NULL_COMP:
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSNULL_COMP), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_NULL_UNCOMP:
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSNULL_UNCOMP), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_VMR9RENDERLESS:
            GetDlgItem(IDC_DSVMR9LOADMIXER)->EnableWindow(TRUE);
            GetDlgItem(IDC_DSVMR9ALTERNATIVEVSYNC)->EnableWindow(TRUE);
            GetDlgItem(IDC_RESETDEVICE)->EnableWindow(TRUE);
            GetDlgItem(IDC_CACHESHADERS)->EnableWindow(TRUE);
            GetDlgItem(IDC_DX_SURFACE)->EnableWindow(TRUE);
            GetDlgItem(IDC_DX9RESIZER_COMBO)->EnableWindow(TRUE);
            GetDlgItem(IDC_FULLSCREEN_MONITOR_CHECK)->EnableWindow(TRUE);

            if (m_iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D) {
                m_iDSShaderSupport.SetIcon(m_tick);
                m_iDSRotationSupport.SetIcon(m_tick);
            }
            m_iDSSaveImageSupport.SetIcon(m_tick);

            m_wndToolTip.UpdateTipText(ResStr(IDC_DSVMR9REN), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_EVR_CUSTOM:
            if (m_iD3D9RenderDeviceCtrl.GetCount() > 1) {
                GetDlgItem(IDC_D3D9DEVICE)->EnableWindow(TRUE);
                GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(IsDlgButtonChecked(IDC_D3D9DEVICE));
            }

            GetDlgItem(IDC_DX9RESIZER_COMBO)->EnableWindow(TRUE);
            GetDlgItem(IDC_FULLSCREEN_MONITOR_CHECK)->EnableWindow(TRUE);
            GetDlgItem(IDC_DSVMR9ALTERNATIVEVSYNC)->EnableWindow(TRUE);
            GetDlgItem(IDC_RESETDEVICE)->EnableWindow(TRUE);
            GetDlgItem(IDC_CACHESHADERS)->EnableWindow(TRUE);

            // Force 3D surface with EVR Custom
            GetDlgItem(IDC_DX_SURFACE)->EnableWindow(FALSE);
            ((CComboBox*)GetDlgItem(IDC_DX_SURFACE))->SetCurSel(2);

            m_iDSDXVASupport.SetIcon(m_tick);
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_iDSShaderSupport.SetIcon(m_tick);
            m_iDSRotationSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSEVR_CUSTOM), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_SYNC:
            GetDlgItem(IDC_EVR_BUFFERS)->EnableWindow(TRUE);
            GetDlgItem(IDC_EVR_BUFFERS_TXT)->EnableWindow(TRUE);
            GetDlgItem(IDC_DX9RESIZER_COMBO)->EnableWindow(TRUE);
            GetDlgItem(IDC_FULLSCREEN_MONITOR_CHECK)->EnableWindow(TRUE);
            GetDlgItem(IDC_RESETDEVICE)->EnableWindow(TRUE);
            GetDlgItem(IDC_CACHESHADERS)->EnableWindow(TRUE);

            // Force 3D surface with EVR Sync
            GetDlgItem(IDC_DX_SURFACE)->EnableWindow(FALSE);
            ((CComboBox*)GetDlgItem(IDC_DX_SURFACE))->SetCurSel(2);

            m_iDSDXVASupport.SetIcon(m_tick);
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_iDSShaderSupport.SetIcon(m_tick);
            m_iDSRotationSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSSYNC), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_MADVR:
            ((CComboBox*)GetDlgItem(IDC_DX_SURFACE))->SetCurSel(2);

            m_iDSDXVASupport.SetIcon(m_tick);
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_iDSShaderSupport.SetIcon(m_tick);
            m_iDSRotationSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSMADVR), GetDlgItem(IDC_VIDRND_COMBO));
            break;
        case VIDRNDT_DS_DXR:
            m_iDSSaveImageSupport.SetIcon(m_tick);
            m_wndToolTip.UpdateTipText(ResStr(IDC_DSDXR), GetDlgItem(IDC_VIDRND_COMBO));
            break;
    }

    m_iDSDXVASupport.SetRedraw(TRUE);
    m_iDSDXVASupport.Invalidate();
    m_iDSDXVASupport.UpdateWindow();
    m_iDSSaveImageSupport.SetRedraw(TRUE);
    m_iDSSaveImageSupport.Invalidate();
    m_iDSSaveImageSupport.UpdateWindow();
    m_iDSShaderSupport.SetRedraw(TRUE);
    m_iDSShaderSupport.Invalidate();
    m_iDSShaderSupport.UpdateWindow();
    m_iDSRotationSupport.SetRedraw(TRUE);
    m_iDSRotationSupport.Invalidate();
    m_iDSRotationSupport.UpdateWindow();

    UpdateSubtitleRendererList();
    UpdateSubtitleSupport();
    SetModified();
}

void CPPageOutput::OnRMRendererChange()
{
    UpdateData();
    m_iRMVideoRendererType = (int)m_iRMVideoRendererTypeCtrl.GetItemData(m_iRMVideoRendererTypeCtrl.GetCurSel());

    switch (m_iRMVideoRendererType) {
        case VIDRNDT_RM_DEFAULT:
            m_iRMSaveImageSupport.SetIcon(m_cross);

            m_wndToolTip.UpdateTipText(ResStr(IDC_RMSYSDEF), GetDlgItem(IDC_RMRND_COMBO));
            break;
        case VIDRNDT_RM_DX9:
            m_iRMSaveImageSupport.SetIcon(m_tick);

            m_wndToolTip.UpdateTipText(ResStr(IDC_RMDX9), GetDlgItem(IDC_RMRND_COMBO));
            break;
    }

    UpdateSubtitleSupport();
    SetModified();
}

void CPPageOutput::OnQTRendererChange()
{
    UpdateData();
    m_iQTVideoRendererType = (int)m_iQTVideoRendererTypeCtrl.GetItemData(m_iQTVideoRendererTypeCtrl.GetCurSel());

    switch (m_iQTVideoRendererType) {
        case VIDRNDT_QT_DEFAULT:
            m_iQTSaveImageSupport.SetIcon(m_cross);

            m_wndToolTip.UpdateTipText(ResStr(IDC_QTSYSDEF), GetDlgItem(IDC_QTRND_COMBO));
            break;
        case VIDRNDT_QT_DX9:
            m_iQTSaveImageSupport.SetIcon(m_tick);

            m_wndToolTip.UpdateTipText(ResStr(IDC_QTDX9), GetDlgItem(IDC_QTRND_COMBO));
            break;
    }

    UpdateSubtitleSupport();
    SetModified();
}

void CPPageOutput::OnSubtitleRendererChange()
{
    UpdateData();
    UpdateSubtitleSupport();
    SetModified();

    m_lastSubrenderer.first = true;
    m_lastSubrenderer.second = static_cast<CAppSettings::SubtitleRenderer>(m_SubtitleRendererCtrl.GetItemData(m_SubtitleRendererCtrl.GetCurSel()));
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

void CPPageOutput::UpdateSubtitleSupport()
{
    auto subrenderer = static_cast<CAppSettings::SubtitleRenderer>(m_SubtitleRendererCtrl.GetItemData(m_SubtitleRendererCtrl.GetCurSel()));

    bool supported = (m_iDSVideoRendererType != VIDRNDT_DS_NULL_COMP &&
                      m_iDSVideoRendererType != VIDRNDT_DS_NULL_UNCOMP &&
                      CAppSettings::IsSubtitleRendererRegistered(subrenderer) &&
                      CAppSettings::IsSubtitleRendererSupported(subrenderer, m_iDSVideoRendererType));

    m_iDSSubtitleSupport.SetIcon(supported ? m_tick : m_cross);

    m_iQTSubtitleSupport.SetIcon((m_iQTVideoRendererType != VIDRNDT_QT_DEFAULT && subrenderer == CAppSettings::SubtitleRenderer::INTERNAL) ? m_tick : m_cross);
    m_iRMSubtitleSupport.SetIcon((m_iRMVideoRendererType != VIDRNDT_RM_DEFAULT && subrenderer == CAppSettings::SubtitleRenderer::INTERNAL) ? m_tick : m_cross);
}

void CPPageOutput::UpdateSubtitleRendererList()
{
    const auto& s = AfxGetAppSettings();

    auto addSubtitleRenderer = [&](CAppSettings::SubtitleRenderer nID) {
        if (!CAppSettings::IsSubtitleRendererSupported(nID, m_iDSVideoRendererType)) {
            return;
        }

        CString sName;
        switch (nID) {
            case CAppSettings::SubtitleRenderer::INTERNAL:
                sName = ResStr(IDS_SUBTITLE_RENDERER_INTERNAL);
                break;
            case CAppSettings::SubtitleRenderer::VS_FILTER:
                sName = ResStr(IDS_SUBTITLE_RENDERER_VS_FILTER);
                break;
            case CAppSettings::SubtitleRenderer::XY_SUB_FILTER:
                sName = ResStr(IDS_SUBTITLE_RENDERER_XY_SUB_FILTER);
                break;
            case CAppSettings::SubtitleRenderer::ASS_FILTER:
                sName = ResStr(IDS_SUBTITLE_RENDERER_ASS_FILTER);
                break;
            default:
                ASSERT(FALSE);
                break;
        }

        if (!CAppSettings::IsSubtitleRendererRegistered(nID)) {
            sName.AppendFormat(_T(" %s"), ResStr(IDS_PPAGE_OUTPUT_UNAVAILABLE).GetString());
        }

        m_SubtitleRendererCtrl.SetItemData(m_SubtitleRendererCtrl.AddString(sName), static_cast<int>(nID));
    };

    m_SubtitleRendererCtrl.SetRedraw(FALSE);
    while (m_SubtitleRendererCtrl.DeleteString(0) != CB_ERR);
    addSubtitleRenderer(CAppSettings::SubtitleRenderer::INTERNAL);
    addSubtitleRenderer(CAppSettings::SubtitleRenderer::VS_FILTER);
    addSubtitleRenderer(CAppSettings::SubtitleRenderer::XY_SUB_FILTER);
    addSubtitleRenderer(CAppSettings::SubtitleRenderer::ASS_FILTER);
    m_SubtitleRendererCtrl.SetCurSel(0);
    auto subrenderer = m_lastSubrenderer.first ? m_lastSubrenderer.second : s.GetSubtitleRenderer();
    for (int j = 0; j < m_SubtitleRendererCtrl.GetCount(); ++j) {
        if ((UINT)subrenderer == m_SubtitleRendererCtrl.GetItemData(j)) {
            m_SubtitleRendererCtrl.SetCurSel(j);
            break;
        }
    }
    m_SubtitleRendererCtrl.EnableWindow(m_SubtitleRendererCtrl.GetCount() > 1);
    CorrectComboListWidth(m_SubtitleRendererCtrl);
    m_SubtitleRendererCtrl.SetRedraw(TRUE);
    m_SubtitleRendererCtrl.Invalidate();
    m_SubtitleRendererCtrl.UpdateWindow();
}

void CPPageOutput::OnD3D9DeviceCheck()
{
    UpdateData();
    GetDlgItem(IDC_D3D9DEVICE_COMBO)->EnableWindow(m_fD3D9RenderDevice);
    SetModified();
}
