/*
* $Id: SyncRenderer.cpp 1293 2009-10-04 07:50:53Z ar-jar $
*
* (C) 2003-2006 Gabest
* (C) 2006-2010 see AUTHORS
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
#include "../../filters/misc/SyncClock/Interfaces.h"
#include <atlbase.h>
#include <atlcoll.h>
#include "../apps/mplayerc/resource.h"
#include "../DSUtil/DSUtil.h"
#include <strsafe.h> // Required in CGenlock
#include <Videoacc.h>
#include <initguid.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <Vmr9.h>
#include <evr.h>
#include <mfapi.h>
#include <Mferror.h>
#include <vector>
#include "../SubPic/DX9SubPic.h"
#include <moreuuids.h>
#include "MacrovisionKicker.h"
#include "IPinHook.h"
#include "PixelShaderCompiler.h"
#include "SyncRenderer.h"

// only for debugging
//#define DISABLE_USING_D3D9EX

using namespace GothSync;
using namespace std;

extern bool LoadResource(UINT resid, CStringA& str, LPCTSTR restype);

CString GothSyncErrorMessage(HRESULT _Error, HMODULE _Module)
{

    switch (_Error)
    {
    case D3DERR_WRONGTEXTUREFORMAT               :
        return _T("D3DERR_WRONGTEXTUREFORMAT");
    case D3DERR_UNSUPPORTEDCOLOROPERATION        :
        return _T("D3DERR_UNSUPPORTEDCOLOROPERATION");
    case D3DERR_UNSUPPORTEDCOLORARG              :
        return _T("D3DERR_UNSUPPORTEDCOLORARG");
    case D3DERR_UNSUPPORTEDALPHAOPERATION        :
        return _T("D3DERR_UNSUPPORTEDALPHAOPERATION");
    case D3DERR_UNSUPPORTEDALPHAARG              :
        return _T("D3DERR_UNSUPPORTEDALPHAARG");
    case D3DERR_TOOMANYOPERATIONS                :
        return _T("D3DERR_TOOMANYOPERATIONS");
    case D3DERR_CONFLICTINGTEXTUREFILTER         :
        return _T("D3DERR_CONFLICTINGTEXTUREFILTER");
    case D3DERR_UNSUPPORTEDFACTORVALUE           :
        return _T("D3DERR_UNSUPPORTEDFACTORVALUE");
    case D3DERR_CONFLICTINGRENDERSTATE           :
        return _T("D3DERR_CONFLICTINGRENDERSTATE");
    case D3DERR_UNSUPPORTEDTEXTUREFILTER         :
        return _T("D3DERR_UNSUPPORTEDTEXTUREFILTER");
    case D3DERR_CONFLICTINGTEXTUREPALETTE        :
        return _T("D3DERR_CONFLICTINGTEXTUREPALETTE");
    case D3DERR_DRIVERINTERNALERROR              :
        return _T("D3DERR_DRIVERINTERNALERROR");
    case D3DERR_NOTFOUND                         :
        return _T("D3DERR_NOTFOUND");
    case D3DERR_MOREDATA                         :
        return _T("D3DERR_MOREDATA");
    case D3DERR_DEVICELOST                       :
        return _T("D3DERR_DEVICELOST");
    case D3DERR_DEVICENOTRESET                   :
        return _T("D3DERR_DEVICENOTRESET");
    case D3DERR_NOTAVAILABLE                     :
        return _T("D3DERR_NOTAVAILABLE");
    case D3DERR_OUTOFVIDEOMEMORY                 :
        return _T("D3DERR_OUTOFVIDEOMEMORY");
    case D3DERR_INVALIDDEVICE                    :
        return _T("D3DERR_INVALIDDEVICE");
    case D3DERR_INVALIDCALL                      :
        return _T("D3DERR_INVALIDCALL");
    case D3DERR_DRIVERINVALIDCALL                :
        return _T("D3DERR_DRIVERINVALIDCALL");
    case D3DERR_WASSTILLDRAWING                  :
        return _T("D3DERR_WASSTILLDRAWING");
    case D3DOK_NOAUTOGEN                         :
        return _T("D3DOK_NOAUTOGEN");
    case D3DERR_DEVICEREMOVED                    :
        return _T("D3DERR_DEVICEREMOVED");
    case S_NOT_RESIDENT                          :
        return _T("S_NOT_RESIDENT");
    case S_RESIDENT_IN_SHARED_MEMORY             :
        return _T("S_RESIDENT_IN_SHARED_MEMORY");
    case S_PRESENT_MODE_CHANGED                  :
        return _T("S_PRESENT_MODE_CHANGED");
    case S_PRESENT_OCCLUDED                      :
        return _T("S_PRESENT_OCCLUDED");
    case D3DERR_DEVICEHUNG                       :
		return _T("D3DERR_DEVICEHUNG");
	case E_UNEXPECTED						     :
		return _T("E_UNEXPECTED");
    }

    CString errmsg;
    LPVOID lpMsgBuf;
    if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_HMODULE,
                     _Module, _Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL))
    {
        errmsg = (LPCTSTR)lpMsgBuf;
        LocalFree(lpMsgBuf);
    }
    CString Temp;
    Temp.Format(L"0x%08x ", _Error);
    return Temp + errmsg;
}

CBaseAP::CBaseAP(HWND hWnd, bool bFullscreen, HRESULT& hr, CString &_Error):
    ISubPicAllocatorPresenterImpl(hWnd, hr, &_Error),
    m_ScreenSize(0, 0),
    m_bicubicA(0),
    m_nDXSurface(1),
    m_nVMR9Surfaces(0),
    m_iVMR9Surface(0),
    m_nCurSurface(0),
    m_bSnapToVSync(false),
    m_bInterlaced(0),
	m_nUsedBuffer(0),
    m_TextScale(1.0),
    m_dMainThreadId(0),
    m_bNeedCheckSample(true),
	m_hEvtQuit(INVALID_HANDLE_VALUE),
	m_bIsFullscreen(bFullscreen),
    m_uSyncGlitches(0),
    m_pGenlock(NULL),
    m_lAudioLag(0),
    m_lAudioLagMin(10000),
    m_lAudioLagMax(-10000),
    m_pAudioStats(NULL),
    m_nNextJitter(0),
    m_nNextSyncOffset(0),
    m_llLastSyncTime(0),
    m_fAvrFps(0.0),
    m_fJitterStdDev(0.0),
    m_fSyncOffsetStdDev(0.0),
    m_fSyncOffsetAvr(0.0),
    m_llHysteresis(0),
    m_uD3DRefreshRate(0),
    m_dD3DRefreshCycle(0),
    m_dDetectedScanlineTime(0.0),
    m_dEstRefreshCycle(0.0),
    m_dFrameCycle(0.0),
    m_dOptimumDisplayCycle(0.0),
    m_dCycleDifference(1.0),
	m_llEstVBlankTime(0),
	m_CurrentAdapter(0)
{
    if(FAILED(hr))
    {
        _Error += L"ISubPicAllocatorPresenterImpl failed\n";
        return;
    }

    HINSTANCE hDll;
    m_pD3DXLoadSurfaceFromMemory = NULL;
    m_pD3DXCreateLine = NULL;
    m_pD3DXCreateFont = NULL;
    m_pD3DXCreateSprite = NULL;
    hDll = GetRenderersData()->GetD3X9Dll();
    if(hDll)
    {
        (FARPROC &)m_pD3DXLoadSurfaceFromMemory = GetProcAddress(hDll, "D3DXLoadSurfaceFromMemory");
        (FARPROC &)m_pD3DXCreateLine = GetProcAddress(hDll, "D3DXCreateLine");
        (FARPROC &)m_pD3DXCreateFont = GetProcAddress(hDll, "D3DXCreateFontW");
        (FARPROC &)m_pD3DXCreateSprite = GetProcAddress(hDll, "D3DXCreateSprite");
    }
    else
    {
        _Error += L"No D3DX9 dll found. To enable stats, shaders and complex resizers, please install the latest DirectX End-User Runtime.\n";
    }

    m_pDwmIsCompositionEnabled = NULL;
    m_pDwmEnableComposition = NULL;
    m_hDWMAPI = LoadLibrary(L"dwmapi.dll");
    if (m_hDWMAPI)
    {
        (FARPROC &)m_pDwmIsCompositionEnabled = GetProcAddress(m_hDWMAPI, "DwmIsCompositionEnabled");
        (FARPROC &)m_pDwmEnableComposition = GetProcAddress(m_hDWMAPI, "DwmEnableComposition");
    }

    m_pDirect3DCreate9Ex = NULL;
    m_hD3D9 = LoadLibrary(L"d3d9.dll");
#ifndef DISABLE_USING_D3D9EX
	if (m_hD3D9)
		(FARPROC &)m_pDirect3DCreate9Ex = GetProcAddress(m_hD3D9, "Direct3DCreate9Ex");
#endif

	if (m_pDirect3DCreate9Ex)
	{
		_tprintf(_T("m_pDirect3DCreate9Ex\n"));
		m_pDirect3DCreate9Ex(D3D_SDK_VERSION, &m_pD3DEx);
		if(!m_pD3DEx)
		{
			m_pDirect3DCreate9Ex(D3D9b_SDK_VERSION, &m_pD3DEx);
		}
	}
	if(!m_pD3DEx)
	{
		m_pD3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
		if(!m_pD3D)
		{
			m_pD3D.Attach(Direct3DCreate9(D3D9b_SDK_VERSION));
		}
		if(m_pD3D)
			_tprintf(_T("m_pDirect3DCreate9\n"));
	}
	else
		m_pD3D = m_pD3DEx;

    ZeroMemory(&m_VMR9AlphaBitmap, sizeof(m_VMR9AlphaBitmap));

    CRenderersSettings& s = GetRenderersSettings();
    if (s.m_RenderSettings.iVMRDisableDesktopComposition)
    {
        m_bDesktopCompositionDisabled = true;
        if (m_pDwmEnableComposition) m_pDwmEnableComposition(0);
    }
    else
    {
        m_bDesktopCompositionDisabled = false;
    }

    m_pGenlock = new CGenlock(s.m_RenderSettings.fTargetSyncOffset, s.m_RenderSettings.fControlLimit, s.m_RenderSettings.iLineDelta, s.m_RenderSettings.iColumnDelta, s.m_RenderSettings.fCycleDelta, 0); // Must be done before CreateDXDevice
    hr = CreateDXDevice(_Error);
    memset (m_pllJitter, 0, sizeof(m_pllJitter));
    memset (m_pllSyncOffset, 0, sizeof(m_pllSyncOffset));
}

CBaseAP::~CBaseAP()
{
    if (m_bDesktopCompositionDisabled)
    {
        m_bDesktopCompositionDisabled = false;
        if (m_pDwmEnableComposition)
            m_pDwmEnableComposition(1);
    }

    m_pFont = NULL;
    m_pLine = NULL;
    m_pD3DDev = NULL;
    m_pD3DDevEx = NULL;
    m_pPSC.Free();
    m_pD3D = NULL;
    m_pD3DEx = NULL;
    if (m_hDWMAPI)
    {
        FreeLibrary(m_hDWMAPI);
        m_hDWMAPI = NULL;
    }
    if (m_hD3D9)
    {
        FreeLibrary(m_hD3D9);
        m_hD3D9 = NULL;
    }
    m_pAudioStats = NULL;
    if (m_pGenlock)
    {
        delete m_pGenlock;
        m_pGenlock = NULL;
    }
}

template<int texcoords>
void CBaseAP::AdjustQuad(MYD3DVERTEX<texcoords>* v, double dx, double dy)
{
    double offset = 0.5;

    for(int i = 0; i < 4; i++)
    {
        v[i].x -= offset;
        v[i].y -= offset;

        for(int j = 0; j < max(texcoords-1, 1); j++)
        {
            v[i].t[j].u -= offset*dx;
            v[i].t[j].v -= offset*dy;
        }

        if(texcoords > 1)
        {
            v[i].t[texcoords-1].u -= offset;
            v[i].t[texcoords-1].v -= offset;
        }
    }
}

template<int texcoords>
HRESULT CBaseAP::TextureBlt(IDirect3DDevice9* pD3DDev, MYD3DVERTEX<texcoords> v[4], D3DTEXTUREFILTERTYPE filter = D3DTEXF_LINEAR)
{
    if(!pD3DDev) return E_POINTER;

    DWORD FVF = 0;
    switch(texcoords)
    {
    case 1:
        FVF = D3DFVF_TEX1;
        break;
    case 2:
        FVF = D3DFVF_TEX2;
        break;
    case 3:
        FVF = D3DFVF_TEX3;
        break;
    case 4:
        FVF = D3DFVF_TEX4;
        break;
    case 5:
        FVF = D3DFVF_TEX5;
        break;
    case 6:
        FVF = D3DFVF_TEX6;
        break;
    case 7:
        FVF = D3DFVF_TEX7;
        break;
    case 8:
        FVF = D3DFVF_TEX8;
        break;
    default:
        return E_FAIL;
    }

    HRESULT hr;
    do
    {
        hr = pD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        hr = pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
        hr = pD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
        hr = pD3DDev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
        hr = pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        hr = pD3DDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        hr = pD3DDev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        hr = pD3DDev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED);

        for(int i = 0; i < texcoords; i++)
        {
            hr = pD3DDev->SetSamplerState(i, D3DSAMP_MAGFILTER, filter);
            hr = pD3DDev->SetSamplerState(i, D3DSAMP_MINFILTER, filter);
            hr = pD3DDev->SetSamplerState(i, D3DSAMP_MIPFILTER, filter);

            hr = pD3DDev->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            hr = pD3DDev->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        }

        hr = pD3DDev->SetFVF(D3DFVF_XYZRHW | FVF);

        MYD3DVERTEX<texcoords> tmp = v[2];
        v[2] = v[3];
        v[3] = tmp;
        hr = pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(v[0]));

        for(int i = 0; i < texcoords; i++)
        {
            pD3DDev->SetTexture(i, NULL);
        }

        return S_OK;
    }
    while(0);
    return E_FAIL;
}

HRESULT CBaseAP::DrawRectBase(IDirect3DDevice9* pD3DDev, MYD3DVERTEX<0> v[4])
{
    if(!pD3DDev) return E_POINTER;

    do
    {
        HRESULT hr = pD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        hr = pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
        hr = pD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
        hr = pD3DDev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
        hr = pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        hr = pD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        hr = pD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        hr = pD3DDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        hr = pD3DDev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

        hr = pD3DDev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED);

        hr = pD3DDev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX0 | D3DFVF_DIFFUSE);

        MYD3DVERTEX<0> tmp = v[2];
        v[2] = v[3];
        v[3] = tmp;
        hr = pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(v[0]));

        return S_OK;
    }
    while(0);
    return E_FAIL;
}

MFOffset CBaseAP::GetOffset(float v)
{
    MFOffset offset;
    offset.value = short(v);
    offset.fract = WORD(65536 * (v-offset.value));
    return offset;
}

MFVideoArea CBaseAP::GetArea(float x, float y, DWORD width, DWORD height)
{
    MFVideoArea area;
    area.OffsetX = GetOffset(x);
    area.OffsetY = GetOffset(y);
    area.Area.cx = width;
    area.Area.cy = height;
    return area;
}

void CBaseAP::ResetStats()
{
    m_pGenlock->ResetStats();
    m_lAudioLag = 0;
    m_lAudioLagMin = 10000;
    m_lAudioLagMax = -10000;
    m_MinJitter = MAXLONG64;
    m_MaxJitter = MINLONG64;
    m_MinSyncOffset = MAXLONG64;
    m_MaxSyncOffset = MINLONG64;
    m_uSyncGlitches = 0;
    m_pcFramesDropped = 0;
}

bool CBaseAP::SettingsNeedResetDevice()
{
    CRenderersSettings& s = GetRenderersSettings();
	CRenderersSettings::CRendererSettingsEVR & New = s.m_RenderSettings;
    CRenderersSettings::CRendererSettingsEVR & Current = m_LastRendererSettings;

    bool bRet = false;
    if (!m_bIsFullscreen)
    {
        if (Current.iVMRDisableDesktopComposition)
        {
            if (!m_bDesktopCompositionDisabled)
            {
                m_bDesktopCompositionDisabled = true;
                if (m_pDwmEnableComposition)
                    m_pDwmEnableComposition(0);
            }
        }
        else
        {
            if (m_bDesktopCompositionDisabled)
            {
                m_bDesktopCompositionDisabled = false;
                if (m_pDwmEnableComposition)
                    m_pDwmEnableComposition(1);
            }
        }
    }
    bRet = bRet || New.iEVRHighColorResolution != Current.iEVRHighColorResolution;
    m_LastRendererSettings = s.m_RenderSettings;
    return bRet;
}

HRESULT CBaseAP::CreateDXDevice(CString &_Error)
{
    TRACE("--> CBaseAP::CreateDXDevice on thread: %d\n", GetCurrentThreadId());
    CRenderersSettings& s = GetRenderersSettings();
    m_LastRendererSettings = s.m_RenderSettings;
    HRESULT hr = E_FAIL;

    m_pFont = NULL;
    m_pSprite = NULL;
    m_pLine = NULL;

    m_pPSC.Free();
    m_pD3DDev = NULL;
    m_pD3DDevEx = NULL;

    m_pResizerPixelShader[0] = 0;
    m_pResizerPixelShader[1] = 0;
    m_pResizerPixelShader[2] = 0;
    m_pResizerPixelShader[3] = 0;

    POSITION pos = m_pPixelShadersScreenSpace.GetHeadPosition();
    while(pos)
    {
        CExternalPixelShader &Shader = m_pPixelShadersScreenSpace.GetNext(pos);
        Shader.m_pPixelShader = NULL;
    }
    pos = m_pPixelShaders.GetHeadPosition();
    while(pos)
    {
        CExternalPixelShader &Shader = m_pPixelShaders.GetNext(pos);
        Shader.m_pPixelShader = NULL;
    }

	if(!m_pD3D)
	{
		_Error += L"Failed to create Direct3D device\n";
		return E_UNEXPECTED;
	}

    D3DDISPLAYMODE d3ddm;
    ZeroMemory(&d3ddm, sizeof(d3ddm));
	m_CurrentAdapter = GetAdapter(m_pD3D);
    if(FAILED(m_pD3D->GetAdapterDisplayMode(m_CurrentAdapter, &d3ddm)))
    {
        _Error += L"Can not retrieve display mode data\n";
        return E_UNEXPECTED;
    }

    if FAILED(m_pD3D->GetDeviceCaps(m_CurrentAdapter, D3DDEVTYPE_HAL, &m_caps))
        if ((m_caps.Caps & D3DCAPS_READ_SCANLINE) == 0)
        {
            _Error += L"Video card does not have scanline access. Display synchronization is not possible.\n";
            return E_UNEXPECTED;
        }

    m_uD3DRefreshRate = d3ddm.RefreshRate;
    m_dD3DRefreshCycle = 1000.0 / (double)m_uD3DRefreshRate; // In ms
    m_ScreenSize.SetSize(d3ddm.Width, d3ddm.Height);
    m_pGenlock->SetDisplayResolution(d3ddm.Width, d3ddm.Height);

    BOOL bCompositionEnabled = false;
    if (m_pDwmIsCompositionEnabled) m_pDwmIsCompositionEnabled(&bCompositionEnabled);
    m_bCompositionEnabled = bCompositionEnabled != 0;

    ZeroMemory(&pp, sizeof(pp));
    if (m_bIsFullscreen) // Exclusive mode fullscreen
    {
        pp.Windowed = FALSE;
        pp.BackBufferWidth = d3ddm.Width;
        pp.BackBufferHeight = d3ddm.Height;
        pp.hDeviceWindow = m_hWnd;
        _tprintf(_T("Wnd in CreateDXDevice: %d\n"), m_hWnd);
        pp.BackBufferCount = 3;
        pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
        pp.Flags = D3DPRESENTFLAG_VIDEO;
        m_bHighColorResolution = s.m_RenderSettings.iEVRHighColorResolution;
        if (m_bHighColorResolution)
        {
            if(FAILED(m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DFMT_A2R10G10B10, false)))
            {
                m_strStatsMsg[MSG_ERROR].Format (L"10 bit RGB is not supported by this graphics device in this resolution.");
                m_bHighColorResolution = false;
            }
        }

        if (m_bHighColorResolution)
            pp.BackBufferFormat = D3DFMT_A2R10G10B10;
        else
            pp.BackBufferFormat = d3ddm.Format;

        if (m_pD3DEx)
        {
            D3DDISPLAYMODEEX DisplayMode;
            ZeroMemory(&DisplayMode, sizeof(DisplayMode));
            DisplayMode.Size = sizeof(DisplayMode);
            m_pD3DEx->GetAdapterDisplayModeEx(m_CurrentAdapter, &DisplayMode, NULL);

            DisplayMode.Format = pp.BackBufferFormat;
            pp.FullScreen_RefreshRateInHz = DisplayMode.RefreshRate;

            if FAILED(m_pD3DEx->CreateDeviceEx(m_CurrentAdapter, D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED, &pp, &DisplayMode, &m_pD3DDevEx))
            {
                _Error += GothSyncErrorMessage(hr, m_hD3D9);
                return hr;
            }
            if (m_pD3DDevEx)
            {
                m_pD3DDev = m_pD3DDevEx;
                m_BackbufferType = pp.BackBufferFormat;
                m_DisplayType = DisplayMode.Format;
            }
        }
        else
        {
            if FAILED(m_pD3D->CreateDevice(m_CurrentAdapter, D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED, &pp, &m_pD3DDev))
            {
                _Error += GothSyncErrorMessage(hr, m_hD3D9);
                return hr;
            }
            _tprintf(_T("Created full-screen device\n"));
            if (m_pD3DDev)
            {
                m_BackbufferType = pp.BackBufferFormat;
                m_DisplayType = d3ddm.Format;
            }
        }
    }
    else // Windowed
    {
        pp.Windowed = TRUE;
        pp.hDeviceWindow = m_hWnd;
        pp.SwapEffect = D3DSWAPEFFECT_COPY;
        pp.Flags = D3DPRESENTFLAG_VIDEO;
        pp.BackBufferCount = 1;
        pp.BackBufferWidth = d3ddm.Width;
        pp.BackBufferHeight = d3ddm.Height;
        m_BackbufferType = d3ddm.Format;
        m_DisplayType = d3ddm.Format;
        m_bHighColorResolution = s.m_RenderSettings.iEVRHighColorResolution;
        if (m_bHighColorResolution)
        {
            if(FAILED(m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DFMT_A2R10G10B10, false)))
            {
                m_strStatsMsg[MSG_ERROR].Format (L"10 bit RGB is not supported by this graphics device in this resolution.");
                m_bHighColorResolution = false;
            }
        }

        if (m_bHighColorResolution)
        {
            m_BackbufferType = D3DFMT_A2R10G10B10;
            pp.BackBufferFormat = D3DFMT_A2R10G10B10;
        }
        if (bCompositionEnabled)
        {
            // Desktop composition presents the whole desktop
            pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
        }
        else
        {
            pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
        }
        if (m_pD3DEx)
        {
            if FAILED(m_pD3DEx->CreateDeviceEx(m_CurrentAdapter, D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED, &pp, NULL, &m_pD3DDevEx))
            {
                _Error += GothSyncErrorMessage(hr, m_hD3D9);
                return hr;
            }
            if (m_pD3DDevEx) m_pD3DDev = m_pD3DDevEx;
        }
        else
        {
            if FAILED(m_pD3D->CreateDevice(m_CurrentAdapter, D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED, &pp, &m_pD3DDev))
            {
                _Error += GothSyncErrorMessage(hr, m_hD3D9);
                return hr;
            }
            _tprintf(_T("Created windowed device\n"));
        }
    }

    if (m_pD3DDevEx)
    {
        m_pD3DDevEx->SetGPUThreadPriority(7);
    }

    m_pPSC.Attach(DNew CPixelShaderCompiler(m_pD3DDev, true));
    m_filter = D3DTEXF_NONE;

    if(m_caps.StretchRectFilterCaps&D3DPTFILTERCAPS_MINFLINEAR && m_caps.StretchRectFilterCaps&D3DPTFILTERCAPS_MAGFLINEAR)
        m_filter = D3DTEXF_LINEAR;

    m_bicubicA = 0;

    CSize size;
    switch(GetRenderersSettings().nSPCMaxRes)
    {
    case 0:
    default:
        size = m_ScreenSize;
        break;
    case 1:
        size.SetSize(1024, 768);
        break;
    case 2:
        size.SetSize(800, 600);
        break;
    case 3:
        size.SetSize(640, 480);
        break;
    case 4:
        size.SetSize(512, 384);
        break;
    case 5:
        size.SetSize(384, 288);
        break;
    case 6:
        size.SetSize(2560, 1600);
        break;
    case 7:
        size.SetSize(1920, 1080);
        break;
    case 8:
        size.SetSize(1320, 900);
        break;
    case 9:
        size.SetSize(1280, 720);
        break;
    }

    if(m_pAllocator)
    {
        m_pAllocator->ChangeDevice(m_pD3DDev);
    }
    else
    {
        m_pAllocator = DNew CDX9SubPicAllocator(m_pD3DDev, size, GetRenderersSettings().fSPCPow2Tex);
        if(!m_pAllocator)
        {
            _Error += L"CDX9SubPicAllocator failed\n";
            return E_FAIL;
        }
    }

    hr = S_OK;

    CComPtr<ISubPicProvider> pSubPicProvider;
    if(m_pSubPicQueue)
    {
        _tprintf(_T("m_pSubPicQueue != NULL\n"));
        m_pSubPicQueue->GetSubPicProvider(&pSubPicProvider);
    }

    m_pSubPicQueue = NULL;
    m_pSubPicQueue = GetRenderersSettings().nSPCSize > 0
                     ? (ISubPicQueue*)DNew CSubPicQueue(GetRenderersSettings().nSPCSize, !GetRenderersSettings().fSPCAllowAnimationWhenBuffering, m_pAllocator, &hr)
                     : (ISubPicQueue*)DNew CSubPicQueueNoThread(m_pAllocator, &hr);
    if(!m_pSubPicQueue || FAILED(hr))
    {
        _Error += L"m_pSubPicQueue failed\n";
        return E_FAIL;
    }

    if(pSubPicProvider) m_pSubPicQueue->SetSubPicProvider(pSubPicProvider);

    if (m_pD3DXCreateFont)
    {
        int MinSize = 1600;
        int CurrentSize = min(m_ScreenSize.cx, MinSize);
        double Scale = double(CurrentSize) / double(MinSize);
        m_TextScale = Scale;
        m_pD3DXCreateFont(m_pD3DDev, -24.0*Scale, -11.0*Scale, CurrentSize < 800 ? FW_NORMAL : FW_BOLD, 0, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FIXED_PITCH | FF_DONTCARE, L"Lucida Console", &m_pFont);
    }
    if (m_pD3DXCreateSprite) m_pD3DXCreateSprite(m_pD3DDev, &m_pSprite);
	if (m_pD3DXCreateLine) m_pD3DXCreateLine (m_pD3DDev, &m_pLine);
	m_LastAdapterCheck = GetRenderersData()->GetPerfCounter();
    return S_OK;
}

HRESULT CBaseAP::ResetDXDevice(CString &_Error)
{
    CRenderersSettings& s = GetRenderersSettings();
    m_LastRendererSettings = s.m_RenderSettings;
    HRESULT hr = E_FAIL;

    hr = m_pD3DDev->TestCooperativeLevel();
    if ((hr != D3DERR_DEVICENOTRESET) && (hr != D3D_OK)) return hr;

    CComPtr<IEnumPins> rendererInputEnum;
    vector<CComPtr<IPin>> decoderOutput;
    vector<CComPtr<IPin>> rendererInput;
    FILTER_INFO filterInfo;

    bool disconnected = FALSE;

    // Disconnect all pins to release video memory resources
    if (m_pD3DDev)
    {
        ZeroMemory(&filterInfo, sizeof(filterInfo));
        m_pOuterEVR->QueryFilterInfo(&filterInfo); // This addref's the pGraph member
        if (SUCCEEDED(m_pOuterEVR->EnumPins(&rendererInputEnum)))
        {
            CComPtr<IPin> input;
            CComPtr<IPin> output;
            while (hr = rendererInputEnum->Next(1, &input.p, 0), hr == S_OK) // Must have .p here
            {
                _tprintf(_T("Pin found\n"));
                input->ConnectedTo(&output.p);
                if (output != NULL)
                {
                    rendererInput.push_back(input);
                    decoderOutput.push_back(output);
                }
                input.Release();
                output.Release();
            }
        }
        else return hr;
        for (DWORD i = 0; i < decoderOutput.size(); i++)
        {
            _tprintf(_T("Disconnecting pin\n"));
            filterInfo.pGraph->Disconnect(decoderOutput.at(i).p);
            filterInfo.pGraph->Disconnect(rendererInput.at(i).p);
            _tprintf(_T("Pin disconnected\n"));
        }
        disconnected = true;
    }

    // Release more resources
    m_pSubPicQueue = NULL;
    m_pFont = NULL;
    m_pSprite = NULL;
    m_pLine = NULL;
    m_pPSC.Free();

    m_pResizerPixelShader[0] = 0;
    m_pResizerPixelShader[1] = 0;
    m_pResizerPixelShader[2] = 0;
    m_pResizerPixelShader[3] = 0;

    POSITION pos = m_pPixelShadersScreenSpace.GetHeadPosition();
    while(pos)
    {
        CExternalPixelShader &Shader = m_pPixelShadersScreenSpace.GetNext(pos);
        Shader.m_pPixelShader = NULL;
    }
    pos = m_pPixelShaders.GetHeadPosition();
    while(pos)
    {
        CExternalPixelShader &Shader = m_pPixelShaders.GetNext(pos);
        Shader.m_pPixelShader = NULL;
    }

    D3DDISPLAYMODE d3ddm;
    ZeroMemory(&d3ddm, sizeof(d3ddm));
    if(FAILED(m_pD3D->GetAdapterDisplayMode(GetAdapter(m_pD3D), &d3ddm)))
    {
        _Error += L"Can not retrieve display mode data\n";
        return E_UNEXPECTED;
    }

    m_uD3DRefreshRate = d3ddm.RefreshRate;
    m_dD3DRefreshCycle = 1000.0 / (double)m_uD3DRefreshRate; // In ms
    m_ScreenSize.SetSize(d3ddm.Width, d3ddm.Height);
    m_pGenlock->SetDisplayResolution(d3ddm.Width, d3ddm.Height);

    D3DPRESENT_PARAMETERS pp;
    ZeroMemory(&pp, sizeof(pp));

    BOOL bCompositionEnabled = false;
    if (m_pDwmIsCompositionEnabled) m_pDwmIsCompositionEnabled(&bCompositionEnabled);
    m_bCompositionEnabled = bCompositionEnabled != 0;
    m_bHighColorResolution = s.m_RenderSettings.iEVRHighColorResolution;

    if (m_bIsFullscreen) // Exclusive mode fullscreen
    {
        pp.BackBufferWidth = d3ddm.Width;
        pp.BackBufferHeight = d3ddm.Height;
        if (m_bHighColorResolution)
            pp.BackBufferFormat = D3DFMT_A2R10G10B10;
        else
            pp.BackBufferFormat = d3ddm.Format;
        if(FAILED(m_pD3DEx->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pp.BackBufferFormat, pp.BackBufferFormat, false)))
        {
            _Error += L"10 bit RGB is not supported by this graphics device in exclusive mode fullscreen.\n";
            return hr;
        }

        D3DDISPLAYMODEEX DisplayMode;
        ZeroMemory(&DisplayMode, sizeof(DisplayMode));
        DisplayMode.Size = sizeof(DisplayMode);
        if (m_pD3DDevEx)
        {
            m_pD3DEx->GetAdapterDisplayModeEx(GetAdapter(m_pD3DEx), &DisplayMode, NULL);
            DisplayMode.Format = pp.BackBufferFormat;
            pp.FullScreen_RefreshRateInHz = DisplayMode.RefreshRate;
            if FAILED(m_pD3DDevEx->Reset(&pp))
            {
                _Error += GothSyncErrorMessage(hr, m_hD3D9);
                return hr;
            }
        }
        else if (m_pD3DDev)
        {
            if FAILED(m_pD3DDev->Reset(&pp))
            {
                _Error += GothSyncErrorMessage(hr, m_hD3D9);
                return hr;
            }
        }
        else
        {
            _Error += L"No device.\n";
            return hr;
        }
        m_BackbufferType = pp.BackBufferFormat;
        m_DisplayType = d3ddm.Format;
    }
    else // Windowed
    {
        pp.BackBufferWidth = d3ddm.Width;
        pp.BackBufferHeight = d3ddm.Height;
        m_BackbufferType = d3ddm.Format;
        m_DisplayType = d3ddm.Format;
        if (m_bHighColorResolution)
        {
            m_BackbufferType = D3DFMT_A2R10G10B10;
            pp.BackBufferFormat = D3DFMT_A2R10G10B10;
        }
        if(FAILED(m_pD3DEx->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pp.BackBufferFormat, pp.BackBufferFormat, false)))
        {
            _Error += L"10 bit RGB is not supported by this graphics device in windowed mode.\n";
            return hr;
        }
        if (bCompositionEnabled)
        {
            // Desktop composition presents the whole desktop
            pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
        }
        else
        {
            pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
        }
        if (m_pD3DDevEx)
            if FAILED(m_pD3DDevEx->Reset(&pp))
            {
                _Error += GothSyncErrorMessage(hr, m_hD3D9);
                return hr;
            }
            else if (m_pD3DDev)
                if FAILED(m_pD3DDevEx->Reset(&pp))
                {
                    _Error += GothSyncErrorMessage(hr, m_hD3D9);
                    return hr;
                }
                else
                {
                    _Error += L"No device.\n";
                    return hr;
                }
    }

    if (disconnected)
    {
        for (DWORD i = 0; i < decoderOutput.size(); i++)
        {
            if (FAILED(filterInfo.pGraph->ConnectDirect(decoderOutput.at(i).p, rendererInput.at(i).p, NULL)))
            {
                return hr;
            }
        }

        if (filterInfo.pGraph != NULL)
        {
            filterInfo.pGraph->Release();
        }
    }

    m_pPSC.Attach(DNew CPixelShaderCompiler(m_pD3DDev, true));
    m_filter = D3DTEXF_NONE;

    if((m_caps.StretchRectFilterCaps&D3DPTFILTERCAPS_MINFLINEAR)
       && (m_caps.StretchRectFilterCaps&D3DPTFILTERCAPS_MAGFLINEAR))
        m_filter = D3DTEXF_LINEAR;

    m_bicubicA = 0;

    CComPtr<ISubPicProvider> pSubPicProvider;
    if(m_pSubPicQueue) m_pSubPicQueue->GetSubPicProvider(&pSubPicProvider);
    CSize size;
    switch(GetRenderersSettings().nSPCMaxRes)
    {
    case 0:
    default:
        size = m_ScreenSize;
        break;
    case 1:
        size.SetSize(1024, 768);
        break;
    case 2:
        size.SetSize(800, 600);
        break;
    case 3:
        size.SetSize(640, 480);
        break;
    case 4:
        size.SetSize(512, 384);
        break;
    case 5:
        size.SetSize(384, 288);
        break;
    case 6:
        size.SetSize(2560, 1600);
        break;
    case 7:
        size.SetSize(1920, 1080);
        break;
    case 8:
        size.SetSize(1320, 900);
        break;
    case 9:
        size.SetSize(1280, 720);
        break;
    }

    if(m_pAllocator)
    {
        m_pAllocator->ChangeDevice(m_pD3DDev);
    }
    else
    {
        m_pAllocator = DNew CDX9SubPicAllocator(m_pD3DDev, size, GetRenderersSettings().fSPCPow2Tex);
        if(!m_pAllocator)
        {
            _Error += L"CDX9SubPicAllocator failed\n";

            return E_FAIL;
        }
    }

    hr = S_OK;
    m_pSubPicQueue = GetRenderersSettings().nSPCSize > 0
                     ? (ISubPicQueue*)DNew CSubPicQueue(GetRenderersSettings().nSPCSize, !GetRenderersSettings().fSPCAllowAnimationWhenBuffering, m_pAllocator, &hr)
                     : (ISubPicQueue*)DNew CSubPicQueueNoThread(m_pAllocator, &hr);
    if(!m_pSubPicQueue || FAILED(hr))
    {
        _Error += L"m_pSubPicQueue failed\n";

        return E_FAIL;
    }

    if(pSubPicProvider) m_pSubPicQueue->SetSubPicProvider(pSubPicProvider);

    m_pFont = NULL;
    if (m_pD3DXCreateFont)
    {
        int MinSize = 1600;
        int CurrentSize = min(m_ScreenSize.cx, MinSize);
        double Scale = double(CurrentSize) / double(MinSize);
        m_TextScale = Scale;
        m_pD3DXCreateFont(m_pD3DDev, -24.0*Scale, -11.0*Scale, CurrentSize < 800 ? FW_NORMAL : FW_BOLD, 0, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FIXED_PITCH | FF_DONTCARE, L"Lucida Console", &m_pFont);
    }
    m_pSprite = NULL;
    if (m_pD3DXCreateSprite) m_pD3DXCreateSprite(m_pD3DDev, &m_pSprite);
    m_pLine = NULL;
    if (m_pD3DXCreateLine) m_pD3DXCreateLine (m_pD3DDev, &m_pLine);
    return S_OK;
}

HRESULT CBaseAP::AllocSurfaces(D3DFORMAT Format)
{
    CAutoLock cAutoLock(this);
    CAutoLock cRenderLock(&m_allocatorLock);

    CRenderersSettings& s = GetRenderersSettings();

    for(int i = 0; i < m_nDXSurface+2; i++)
    {
        m_pVideoTexture[i] = NULL;
        m_pVideoSurface[i] = NULL;
    }

    m_pScreenSizeTemporaryTexture[0] = NULL;
    m_pScreenSizeTemporaryTexture[1] = NULL;
    m_SurfaceType = Format;

    HRESULT hr;
    if(s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D || s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
    {
        int nTexturesNeeded = s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D ? m_nDXSurface+2 : 1;

        for(int i = 0; i < nTexturesNeeded; i++)
        {
            if(FAILED(hr = m_pD3DDev->CreateTexture(
                               m_NativeVideoSize.cx, m_NativeVideoSize.cy, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pVideoTexture[i], NULL)))
                return hr;

            if(FAILED(hr = m_pVideoTexture[i]->GetSurfaceLevel(0, &m_pVideoSurface[i])))
                return hr;
        }
        if(s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D)
        {
            for(int i = 0; i < m_nDXSurface+2; i++)
            {
                m_pVideoTexture[i] = NULL;
            }
        }
    }
    else
    {
        if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(m_NativeVideoSize.cx, m_NativeVideoSize.cy, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pVideoSurface[m_nCurSurface], NULL)))
            return hr;
    }

    hr = m_pD3DDev->ColorFill(m_pVideoSurface[m_nCurSurface], NULL, 0);
    return S_OK;
}

void CBaseAP::DeleteSurfaces()
{
    CAutoLock cAutoLock(this);
    CAutoLock cRenderLock(&m_allocatorLock);

    for(int i = 0; i < m_nDXSurface+2; i++)
    {
        m_pVideoTexture[i] = NULL;
        m_pVideoSurface[i] = NULL;
    }
}

UINT CBaseAP::GetAdapter(IDirect3D9* pD3D)
{
    if(m_hWnd == NULL || pD3D == NULL) return D3DADAPTER_DEFAULT;

    HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
    if(hMonitor == NULL) return D3DADAPTER_DEFAULT;

    for(UINT adp = 0, num_adp = pD3D->GetAdapterCount(); adp < num_adp; ++adp)
    {
        HMONITOR hAdpMon = pD3D->GetAdapterMonitor(adp);
        if(hAdpMon == hMonitor) return adp;
    }
    return D3DADAPTER_DEFAULT;
}

// ISubPicAllocatorPresenter

STDMETHODIMP CBaseAP::CreateRenderer(IUnknown** ppRenderer)
{
    return E_NOTIMPL;
}

bool CBaseAP::ClipToSurface(IDirect3DSurface9* pSurface, CRect& s, CRect& d)
{
    D3DSURFACE_DESC d3dsd;
    ZeroMemory(&d3dsd, sizeof(d3dsd));
    if(FAILED(pSurface->GetDesc(&d3dsd)))
        return(false);

    int w = d3dsd.Width, h = d3dsd.Height;
    int sw = s.Width(), sh = s.Height();
    int dw = d.Width(), dh = d.Height();

    if(d.left >= w || d.right < 0 || d.top >= h || d.bottom < 0
       || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0)
    {
        s.SetRectEmpty();
        d.SetRectEmpty();
        return(true);
    }
    if(d.right > w)
    {
        s.right -= (d.right-w)*sw/dw;
        d.right = w;
    }
    if(d.bottom > h)
    {
        s.bottom -= (d.bottom-h)*sh/dh;
        d.bottom = h;
    }
    if(d.left < 0)
    {
        s.left += (0-d.left)*sw/dw;
        d.left = 0;
    }
    if(d.top < 0)
    {
        s.top += (0-d.top)*sh/dh;
        d.top = 0;
    }
    return(true);
}

HRESULT CBaseAP::InitResizers(float bicubicA, bool bNeedScreenSizeTexture)
{
    HRESULT hr;
    do
    {
        if (bicubicA)
        {
            if (!m_pResizerPixelShader[0])
                break;
            if (!m_pResizerPixelShader[1])
                break;
            if (!m_pResizerPixelShader[2])
                break;
            if (!m_pResizerPixelShader[3])
                break;
            if (m_bicubicA != bicubicA)
                break;
            if (!m_pScreenSizeTemporaryTexture[0])
                break;
            if (bNeedScreenSizeTexture)
            {
                if (!m_pScreenSizeTemporaryTexture[1])
                    break;
            }
        }
        else
        {
            if (!m_pResizerPixelShader[0])
                break;
            if (bNeedScreenSizeTexture)
            {
                if (!m_pScreenSizeTemporaryTexture[0])
                    break;
                if (!m_pScreenSizeTemporaryTexture[1])
                    break;
            }
        }
        return S_OK;
    }
    while (0);

    m_bicubicA = bicubicA;
    m_pScreenSizeTemporaryTexture[0] = NULL;
    m_pScreenSizeTemporaryTexture[1] = NULL;

    for(int i = 0; i < countof(m_pResizerPixelShader); i++)
        m_pResizerPixelShader[i] = NULL;

    if(m_caps.PixelShaderVersion < D3DPS_VERSION(2, 0)) return E_FAIL;

    LPCSTR pProfile = m_caps.PixelShaderVersion >= D3DPS_VERSION(3, 0) ? "ps_3_0" : "ps_2_0";

    CStringA str;
    if(!LoadResource(IDF_SHADER_RESIZER, str, _T("FILE"))) return E_FAIL;

    CStringA A;
    A.Format("(%f)", bicubicA);
    str.Replace("_The_Value_Of_A_Is_Set_Here_", A);

    LPCSTR pEntries[] = {"main_bilinear", "main_bicubic1pass", "main_bicubic2pass_pass1", "main_bicubic2pass_pass2"};

    ASSERT(countof(pEntries) == countof(m_pResizerPixelShader));
    for(int i = 0; i < countof(pEntries); i++)
    {
        CString ErrorMessage;
        CString DissAssembly;
        hr = m_pPSC->CompileShader(str, pEntries[i], pProfile, 0, &m_pResizerPixelShader[i], &DissAssembly, &ErrorMessage);
        if(FAILED(hr))
        {
            TRACE("%ws", ErrorMessage.GetString());
            ASSERT (0);
            return hr;
        }
    }
    if(m_bicubicA || bNeedScreenSizeTexture)
    {
        if(FAILED(m_pD3DDev->CreateTexture(
                      min(m_ScreenSize.cx, (int)m_caps.MaxTextureWidth), min(max(m_ScreenSize.cy, m_NativeVideoSize.cy), (int)m_caps.MaxTextureHeight), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
                      D3DPOOL_DEFAULT, &m_pScreenSizeTemporaryTexture[0], NULL)))
        {
            ASSERT(0);
            m_pScreenSizeTemporaryTexture[0] = NULL; // will do 1 pass then
        }
    }
    if(m_bicubicA || bNeedScreenSizeTexture)
    {
        if(FAILED(m_pD3DDev->CreateTexture(
                      min(m_ScreenSize.cx, (int)m_caps.MaxTextureWidth), min(max(m_ScreenSize.cy, m_NativeVideoSize.cy), (int)m_caps.MaxTextureHeight), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
                      D3DPOOL_DEFAULT, &m_pScreenSizeTemporaryTexture[1], NULL)))
        {
            ASSERT(0);
            m_pScreenSizeTemporaryTexture[1] = NULL; // will do 1 pass then
        }
    }
    return S_OK;
}

HRESULT CBaseAP::TextureCopy(IDirect3DTexture9* pTexture)
{
    HRESULT hr;

    D3DSURFACE_DESC desc;
    if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
        return E_FAIL;

    float w = (float)desc.Width;
    float h = (float)desc.Height;
    MYD3DVERTEX<1> v[] =
    {
        {0, 0, 0.5f, 2.0f, 0, 0},
        {w, 0, 0.5f, 2.0f, 1, 0},
        {0, h, 0.5f, 2.0f, 0, 1},
        {w, h, 0.5f, 2.0f, 1, 1},
    };
    for(int i = 0; i < countof(v); i++)
    {
        v[i].x -= 0.5;
        v[i].y -= 0.5;
    }
    hr = m_pD3DDev->SetTexture(0, pTexture);
    return TextureBlt(m_pD3DDev, v, D3DTEXF_LINEAR);
}

HRESULT CBaseAP::DrawRect(DWORD _Color, DWORD _Alpha, const CRect &_Rect)
{
    DWORD Color = D3DCOLOR_ARGB(_Alpha, GetRValue(_Color), GetGValue(_Color), GetBValue(_Color));
    MYD3DVERTEX<0> v[] =
    {
        {float(_Rect.left), float(_Rect.top), 0.5f, 2.0f, Color},
        {float(_Rect.right), float(_Rect.top), 0.5f, 2.0f, Color},
        {float(_Rect.left), float(_Rect.bottom), 0.5f, 2.0f, Color},
        {float(_Rect.right), float(_Rect.bottom), 0.5f, 2.0f, Color},
    };
    for(int i = 0; i < countof(v); i++)
    {
        v[i].x -= 0.5;
        v[i].y -= 0.5;
    }
    return DrawRectBase(m_pD3DDev, v);
}

HRESULT CBaseAP::TextureResize(IDirect3DTexture9* pTexture, Vector dst[4], D3DTEXTUREFILTERTYPE filter, const CRect &SrcRect)
{
    HRESULT hr;

    D3DSURFACE_DESC desc;
    if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
        return E_FAIL;

    float w = (float)desc.Width;
    float h = (float)desc.Height;

    float dx = 1.0f/w;
    float dy = 1.0f/h;
    float dx2 = 1.0/w;
    float dy2 = 1.0/h;

    MYD3DVERTEX<1> v[] =
    {
        {dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  SrcRect.left * dx2, SrcRect.top * dy2},
        {dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  SrcRect.right * dx2, SrcRect.top * dy2},
        {dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  SrcRect.left * dx2, SrcRect.bottom * dy2},
        {dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  SrcRect.right * dx2, SrcRect.bottom * dy2},
    };
    AdjustQuad(v, 0, 0);
    hr = m_pD3DDev->SetTexture(0, pTexture);
    hr = m_pD3DDev->SetPixelShader(NULL);
    hr = TextureBlt(m_pD3DDev, v, filter);
    return hr;
}

HRESULT CBaseAP::TextureResizeBilinear(IDirect3DTexture9* pTexture, Vector dst[4], const CRect &SrcRect)
{
    HRESULT hr;

    D3DSURFACE_DESC desc;
    if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
        return E_FAIL;

    float w = (float)desc.Width;
    float h = (float)desc.Height;

    float dx = 1.0f/w;
    float dy = 1.0f/h;
    float tx0 = SrcRect.left;
    float tx1 = SrcRect.right;
    float ty0 = SrcRect.top;
    float ty1 = SrcRect.bottom;

    MYD3DVERTEX<1> v[] =
    {
        {dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  tx0, ty0},
        {dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  tx1, ty0},
        {dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  tx0, ty1},
        {dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  tx1, ty1},
    };
    AdjustQuad(v, 1.0, 1.0);
    float fConstData[][4] = {{0.5f / w, 0.5f / h, 0, 0}, {1.0f / w, 1.0f / h, 0, 0}, {1.0f / w, 0, 0, 0}, {0, 1.0f / h, 0, 0}, {w, h, 0, 0}};
    hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));
    hr = m_pD3DDev->SetTexture(0, pTexture);
    hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShader[0]);
    hr = TextureBlt(m_pD3DDev, v, D3DTEXF_POINT);
    m_pD3DDev->SetPixelShader(NULL);
    return hr;
}

HRESULT CBaseAP::TextureResizeBicubic1pass(IDirect3DTexture9* pTexture, Vector dst[4], const CRect &SrcRect)
{
    HRESULT hr;

    D3DSURFACE_DESC desc;
    if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
        return E_FAIL;

    double w = (double)desc.Width;
    double h = (double)desc.Height;

    double sw = SrcRect.Width();
    double sh = SrcRect.Height();

    double dx = 1.0f/w;
    double dy = 1.0f/h;

    float dx2 = 1.0f/w;
    float dy2 = 1.0f/h;
    float tx0 = SrcRect.left;
    float tx1 = SrcRect.right;
    float ty0 = SrcRect.top;
    float ty1 = SrcRect.bottom;

    MYD3DVERTEX<1> v[] =
    {
        {dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  tx0, ty0},
        {dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  tx1, ty0},
        {dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  tx0, ty1},
        {dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  tx1, ty1},
    };
    AdjustQuad(v, 1.0, 1.0);
    hr = m_pD3DDev->SetTexture(0, pTexture);
    float fConstData[][4] = {{0.5f / w, 0.5f / h, 0, 0}, {1.0f / w, 1.0f / h, 0, 0}, {1.0f / w, 0, 0, 0}, {0, 1.0f / h, 0, 0}, {w, h, 0, 0}};
    hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));
    hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShader[1]);
    hr = TextureBlt(m_pD3DDev, v, D3DTEXF_POINT);
    m_pD3DDev->SetPixelShader(NULL);
    return hr;
}

HRESULT CBaseAP::TextureResizeBicubic2pass(IDirect3DTexture9* pTexture, Vector dst[4], const CRect &SrcRect)
{
    // The 2 pass sampler is incorrect in that it only does bilinear resampling in the y direction.
    return TextureResizeBicubic1pass(pTexture, dst, SrcRect);

    HRESULT hr;

    // rotated?
    if(dst[0].z != dst[1].z || dst[2].z != dst[3].z || dst[0].z != dst[3].z
    || dst[0].y != dst[1].y || dst[0].x != dst[2].x || dst[2].y != dst[3].y || dst[1].x != dst[3].x)
        return TextureResizeBicubic1pass(pTexture, dst, SrcRect);

    D3DSURFACE_DESC desc;
    if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
        return E_FAIL;

    float Tex0_Width = desc.Width;
    float Tex0_Height = desc.Height;

    double dx0 = 1.0/desc.Width;
    double dy0 = 1.0/desc.Height;

    CSize SrcTextSize = CSize(desc.Width, desc.Height);
    double w = (double)SrcRect.Width();
    double h = (double)SrcRect.Height();

    CRect dst1(0, 0, (int)(dst[3].x - dst[0].x), (int)h);

    if(!m_pScreenSizeTemporaryTexture[0] || FAILED(m_pScreenSizeTemporaryTexture[0]->GetLevelDesc(0, &desc)))
        return TextureResizeBicubic1pass(pTexture, dst, SrcRect);

    float Tex1_Width = desc.Width;
    float Tex1_Height = desc.Height;

    double dx1 = 1.0/desc.Width;
    double dy1 = 1.0/desc.Height;

    double dw = (double)dst1.Width() / desc.Width;
    double dh = (double)dst1.Height() / desc.Height;

    float dx2 = 1.0f/SrcTextSize.cx;
    float dy2 = 1.0f/SrcTextSize.cy;
    float tx0 = SrcRect.left;
    float tx1 = SrcRect.right;
    float ty0 = SrcRect.top;
    float ty1 = SrcRect.bottom;

    float tx0_2 = 0;
    float tx1_2 = dst1.Width();
    float ty0_2 = 0;
    float ty1_2 = h;

    if(dst1.Width() > (int)desc.Width || dst1.Height() > (int)desc.Height)
        return TextureResizeBicubic1pass(pTexture, dst, SrcRect);

    MYD3DVERTEX<1> vx[] =
    {
        {(float)dst1.left, (float)dst1.top,		0.5f, 2.0f, tx0, ty0},
        {(float)dst1.right, (float)dst1.top,	0.5f, 2.0f, tx1, ty0},
        {(float)dst1.left, (float)dst1.bottom,	0.5f, 2.0f, tx0, ty1},
        {(float)dst1.right, (float)dst1.bottom, 0.5f, 2.0f, tx1, ty1},
    };
    AdjustQuad(vx, 1.0, 0.0);		// Casimir666 : bug ici, génére des bandes verticales! TODO : pourquoi ??????
    MYD3DVERTEX<1> vy[] =
    {
        {dst[0].x, dst[0].y, dst[0].z, 1.0/dst[0].z, tx0_2, ty0_2},
        {dst[1].x, dst[1].y, dst[1].z, 1.0/dst[1].z, tx1_2, ty0_2},
        {dst[2].x, dst[2].y, dst[2].z, 1.0/dst[2].z, tx0_2, ty1_2},
        {dst[3].x, dst[3].y, dst[3].z, 1.0/dst[3].z, tx1_2, ty1_2},
    };
    AdjustQuad(vy, 0.0, 1.0);
    hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShader[2]);
    {
        float fConstData[][4] = {{0.5f / Tex0_Width, 0.5f / Tex0_Height, 0, 0}, {1.0f / Tex0_Width, 1.0f / Tex0_Height, 0, 0}, {1.0f / Tex0_Width, 0, 0, 0}, {0, 1.0f / Tex0_Height, 0, 0}, {Tex0_Width, Tex0_Height, 0, 0}};
        hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));
    }
    hr = m_pD3DDev->SetTexture(0, pTexture);
    CComPtr<IDirect3DSurface9> pRTOld;
    hr = m_pD3DDev->GetRenderTarget(0, &pRTOld);
    CComPtr<IDirect3DSurface9> pRT;
    hr = m_pScreenSizeTemporaryTexture[0]->GetSurfaceLevel(0, &pRT);
    hr = m_pD3DDev->SetRenderTarget(0, pRT);
    hr = TextureBlt(m_pD3DDev, vx, D3DTEXF_POINT);
    hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShader[3]);
    {
        float fConstData[][4] = {{0.5f / Tex1_Width, 0.5f / Tex1_Height, 0, 0}, {1.0f / Tex1_Width, 1.0f / Tex1_Height, 0, 0}, {1.0f / Tex1_Width, 0, 0, 0}, {0, 1.0f / Tex1_Height, 0, 0}, {Tex1_Width, Tex1_Height, 0, 0}};
        hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));
    }
    hr = m_pD3DDev->SetTexture(0, m_pScreenSizeTemporaryTexture[0]);
    hr = m_pD3DDev->SetRenderTarget(0, pRTOld);
    hr = TextureBlt(m_pD3DDev, vy, D3DTEXF_POINT);
    m_pD3DDev->SetPixelShader(NULL);
    return hr;
}

HRESULT CBaseAP::AlphaBlt(RECT* pSrc, RECT* pDst, IDirect3DTexture9* pTexture)
{
    if(!pSrc || !pDst)
        return E_POINTER;

    CRect src(*pSrc), dst(*pDst);

    HRESULT hr;

    do
    {
        D3DSURFACE_DESC d3dsd;
        ZeroMemory(&d3dsd, sizeof(d3dsd));
        if(FAILED(pTexture->GetLevelDesc(0, &d3dsd)) /*|| d3dsd.Type != D3DRTYPE_TEXTURE*/)
            break;

        float w = (float)d3dsd.Width;
        float h = (float)d3dsd.Height;

        struct
        {
            float x, y, z, rhw;
            float tu, tv;
        }
        pVertices[] =
        {
            {(float)dst.left, (float)dst.top, 0.5f, 2.0f, (float)src.left / w, (float)src.top / h},
            {(float)dst.right, (float)dst.top, 0.5f, 2.0f, (float)src.right / w, (float)src.top / h},
            {(float)dst.left, (float)dst.bottom, 0.5f, 2.0f, (float)src.left / w, (float)src.bottom / h},
            {(float)dst.right, (float)dst.bottom, 0.5f, 2.0f, (float)src.right / w, (float)src.bottom / h},
        };

        hr = m_pD3DDev->SetTexture(0, pTexture);

        DWORD abe, sb, db;
        hr = m_pD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &abe);
        hr = m_pD3DDev->GetRenderState(D3DRS_SRCBLEND, &sb);
        hr = m_pD3DDev->GetRenderState(D3DRS_DESTBLEND, &db);

        hr = m_pD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        hr = m_pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
        hr = m_pD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
        hr = m_pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        hr = m_pD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // pre-multiplied src and ...
        hr = m_pD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA); // ... inverse alpha channel for dst

        hr = m_pD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        hr = m_pD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        hr = m_pD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

        hr = m_pD3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        hr = m_pD3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        hr = m_pD3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

        hr = m_pD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        hr = m_pD3DDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

        hr = m_pD3DDev->SetPixelShader(NULL);

        hr = m_pD3DDev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
        hr = m_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertices, sizeof(pVertices[0]));

        m_pD3DDev->SetTexture(0, NULL);

        m_pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, abe);
        m_pD3DDev->SetRenderState(D3DRS_SRCBLEND, sb);
        m_pD3DDev->SetRenderState(D3DRS_DESTBLEND, db);

        return S_OK;
    }
    while(0);
    return E_FAIL;
}

// Update the array m_pllJitter with a new vsync period. Calculate min, max and stddev.
void CBaseAP::SyncStats(LONGLONG syncTime)
{
    m_nNextJitter = (m_nNextJitter+1) % NB_JITTER;
    m_pllJitter[m_nNextJitter] = syncTime - m_llLastSyncTime;
    double syncDeviation = ((double)m_pllJitter[m_nNextJitter] - m_fJitterMean) / 10000.0;
    if (abs(syncDeviation) > (GetDisplayCycle() / 2))
        m_uSyncGlitches++;

    LONGLONG llJitterSum = 0;
    LONGLONG llJitterSumAvg = 0;
    for (int i=0; i<NB_JITTER; i++)
    {
        LONGLONG Jitter = m_pllJitter[i];
        llJitterSum += Jitter;
        llJitterSumAvg += Jitter;
    }
    m_fJitterMean = double(llJitterSumAvg) / NB_JITTER;
    double DeviationSum = 0;

    for (int i=0; i<NB_JITTER; i++)
    {
        LONGLONG DevInt = m_pllJitter[i] - m_fJitterMean;
        double Deviation = DevInt;
        DeviationSum += Deviation*Deviation;
        m_MaxJitter = max(m_MaxJitter, DevInt);
        m_MinJitter = min(m_MinJitter, DevInt);
    }

    m_fJitterStdDev = sqrt(DeviationSum/NB_JITTER);
    m_fAvrFps = 10000000.0/(double(llJitterSum)/NB_JITTER);
    m_llLastSyncTime = syncTime;
}

// Collect the difference between periodEnd and periodStart in an array, calculate mean and stddev.
void CBaseAP::SyncOffsetStats(LONGLONG syncOffset)
{
    m_nNextSyncOffset = (m_nNextSyncOffset+1) % NB_JITTER;
    m_pllSyncOffset[m_nNextSyncOffset] = syncOffset;

    LONGLONG AvrageSum = 0;
    for (int i=0; i<NB_JITTER; i++)
    {
        LONGLONG Offset = m_pllSyncOffset[i];
        AvrageSum += Offset;
        m_MaxSyncOffset = max(m_MaxSyncOffset, Offset);
        m_MinSyncOffset = min(m_MinSyncOffset, Offset);
    }
    double MeanOffset = double(AvrageSum)/NB_JITTER;
    double DeviationSum = 0;
    for (int i=0; i<NB_JITTER; i++)
    {
        double Deviation = double(m_pllSyncOffset[i]) - MeanOffset;
        DeviationSum += Deviation*Deviation;
    }
    double StdDev = sqrt(DeviationSum/NB_JITTER);

    m_fSyncOffsetAvr = MeanOffset;
    m_fSyncOffsetStdDev = StdDev;
}

void CBaseAP::UpdateAlphaBitmap()
{
    m_VMR9AlphaBitmapData.Free();

    if ((m_VMR9AlphaBitmap.dwFlags & VMRBITMAP_DISABLE) == 0)
    {
        HBITMAP hBitmap = (HBITMAP)GetCurrentObject (m_VMR9AlphaBitmap.hdc, OBJ_BITMAP);
        if (!hBitmap)
            return;
        DIBSECTION info = {0};
        if (!::GetObject(hBitmap, sizeof( DIBSECTION ), &info ))
            return;

        m_VMR9AlphaBitmapRect = CRect(0, 0, info.dsBm.bmWidth, info.dsBm.bmHeight);
        m_VMR9AlphaBitmapWidthBytes = info.dsBm.bmWidthBytes;

        if (m_VMR9AlphaBitmapData.Allocate(info.dsBm.bmWidthBytes * info.dsBm.bmHeight))
        {
            memcpy((BYTE *)m_VMR9AlphaBitmapData, info.dsBm.bmBits, info.dsBm.bmWidthBytes * info.dsBm.bmHeight);
        }
    }
}

// Present a sample (frame) using DirectX.
STDMETHODIMP_(bool) CBaseAP::Paint(bool fAll)
{
	if (m_bPendingResetDevice)
	{
		SendResetRequest();
		return false;
	}

    CRenderersSettings& s = GetRenderersSettings();
    CRenderersData * pApp = GetRenderersData();
    D3DRASTER_STATUS rasterStatus;
    REFERENCE_TIME llCurRefTime = 0;
    REFERENCE_TIME llSyncOffset = 0;
    double dSyncOffset = 0.0;

    CAutoLock cRenderLock(&m_allocatorLock);

    // Estimate time for next vblank based on number of remaining lines in this frame. This algorithm seems to be
    // accurate within one ms why there should not be any need for a more accurate one. The wiggly line seen
    // when using sync to nearest and sync display is most likely due to inaccuracies in the audio-card-based
    // reference clock. The wiggles are not seen with the perfcounter-based reference clock of the sync to video option.
    m_pD3DDev->GetRasterStatus(0, &rasterStatus);
    m_uScanLineEnteringPaint = rasterStatus.ScanLine;
    if (m_pRefClock) m_pRefClock->GetTime(&llCurRefTime);
    dSyncOffset = (m_ScreenSize.cy - m_uScanLineEnteringPaint) * m_dDetectedScanlineTime; // ms
    llSyncOffset = REFERENCE_TIME(10000.0 * dSyncOffset); // Reference time units (100 ns)
    m_llEstVBlankTime = llCurRefTime + llSyncOffset; // Estimated time for the start of next vblank

    if(m_WindowRect.right <= m_WindowRect.left || m_WindowRect.bottom <= m_WindowRect.top
       || m_NativeVideoSize.cx <= 0 || m_NativeVideoSize.cy <= 0
       || !m_pVideoSurface)
    {
        return(false);
    }

    HRESULT hr;
    CRect rSrcVid(CPoint(0, 0), m_NativeVideoSize);
    CRect rDstVid(m_VideoRect);
    CRect rSrcPri(CPoint(0, 0), m_WindowRect.Size());
    CRect rDstPri(m_WindowRect);

    m_pD3DDev->BeginScene();
    CComPtr<IDirect3DSurface9> pBackBuffer;
    m_pD3DDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
    m_pD3DDev->SetRenderTarget(0, pBackBuffer);
    hr = m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
    if(!rDstVid.IsRectEmpty())
    {
        if(m_pVideoTexture[m_nCurSurface])
        {
            CComPtr<IDirect3DTexture9> pVideoTexture = m_pVideoTexture[m_nCurSurface];

            if(m_pVideoTexture[m_nDXSurface] && m_pVideoTexture[m_nDXSurface+1] && !m_pPixelShaders.IsEmpty())
            {
                static __int64 counter = 0;
                static long start = clock();

                long stop = clock();
                long diff = stop - start;

                if(diff >= 10*60*CLOCKS_PER_SEC) start = stop; // reset after 10 min (ps float has its limits in both range and accuracy)

                int src = m_nCurSurface, dst = m_nDXSurface;

                D3DSURFACE_DESC desc;
                m_pVideoTexture[src]->GetLevelDesc(0, &desc);

                float fConstData[][4] =
                {
                    {(float)desc.Width, (float)desc.Height, (float)(counter++), (float)diff / CLOCKS_PER_SEC},
                    {1.0f / desc.Width, 1.0f / desc.Height, 0, 0},
                };

                hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));

                CComPtr<IDirect3DSurface9> pRT;
                hr = m_pD3DDev->GetRenderTarget(0, &pRT);

                POSITION pos = m_pPixelShaders.GetHeadPosition();
                while(pos)
                {
                    pVideoTexture = m_pVideoTexture[dst];

                    hr = m_pD3DDev->SetRenderTarget(0, m_pVideoSurface[dst]);
                    CExternalPixelShader &Shader = m_pPixelShaders.GetNext(pos);
                    if (!Shader.m_pPixelShader)
                        Shader.Compile(m_pPSC);
                    hr = m_pD3DDev->SetPixelShader(Shader.m_pPixelShader);
                    TextureCopy(m_pVideoTexture[src]);

                    src		= dst;
                    if(++dst >= m_nDXSurface+2) dst = m_nDXSurface;
                }

                hr = m_pD3DDev->SetRenderTarget(0, pRT);
                hr = m_pD3DDev->SetPixelShader(NULL);
            }

            Vector dst[4];
            Transform(rDstVid, dst);

            DWORD iDX9Resizer = s.iDX9Resizer;

            float A = 0;

            switch(iDX9Resizer)
            {
            case 3:
                A = -0.60f;
                break;
            case 4:
                A = -0.751f;
                break;	// FIXME : 0.75 crash recent D3D, or eat CPU
            case 5:
                A = -1.00f;
                break;
            }
            bool bScreenSpacePixelShaders = !m_pPixelShadersScreenSpace.IsEmpty();

            hr = InitResizers(A, bScreenSpacePixelShaders);

            if (!m_pScreenSizeTemporaryTexture[0] || !m_pScreenSizeTemporaryTexture[1])
                bScreenSpacePixelShaders = false;

            if (bScreenSpacePixelShaders)
            {
                CComPtr<IDirect3DSurface9> pRT;
                hr = m_pScreenSizeTemporaryTexture[1]->GetSurfaceLevel(0, &pRT);
                if (hr != S_OK)
                    bScreenSpacePixelShaders = false;
                if (bScreenSpacePixelShaders)
                {
                    hr = m_pD3DDev->SetRenderTarget(0, pRT);
                    if (hr != S_OK)
                        bScreenSpacePixelShaders = false;
                    hr = m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
                }
            }

            if(rSrcVid.Size() != rDstVid.Size())
            {
                if(iDX9Resizer == 0 || iDX9Resizer == 1)
                {
                    D3DTEXTUREFILTERTYPE Filter = iDX9Resizer == 0 ? D3DTEXF_POINT : D3DTEXF_LINEAR;
                    hr = TextureResize(pVideoTexture, dst, Filter, rSrcVid);
                }
                else if(iDX9Resizer == 2)
                {
                    hr = TextureResizeBilinear(pVideoTexture, dst, rSrcVid);
                }
                else if(iDX9Resizer >= 3)
                {
                    hr = TextureResizeBicubic2pass(pVideoTexture, dst, rSrcVid);
                }
            }
            else hr = TextureResize(pVideoTexture, dst, D3DTEXF_POINT, rSrcVid);

            if (bScreenSpacePixelShaders)
            {
                static __int64 counter = 555;
                static long start = clock() + 333;

                long stop = clock() + 333;
                long diff = stop - start;

                if(diff >= 10*60*CLOCKS_PER_SEC) start = stop; // reset after 10 min (ps float has its limits in both range and accuracy)

                D3DSURFACE_DESC desc;
                m_pScreenSizeTemporaryTexture[0]->GetLevelDesc(0, &desc);

                float fConstData[][4] =
                {
                    {(float)desc.Width, (float)desc.Height, (float)(counter++), (float)diff / CLOCKS_PER_SEC},
                    {1.0f / desc.Width, 1.0f / desc.Height, 0, 0},
                };

                hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));

                int src = 1, dst = 0;

                POSITION pos = m_pPixelShadersScreenSpace.GetHeadPosition();
                while(pos)
                {
                    if (m_pPixelShadersScreenSpace.GetTailPosition() == pos)
                    {
                        m_pD3DDev->SetRenderTarget(0, pBackBuffer);
                    }
                    else
                    {
                        CComPtr<IDirect3DSurface9> pRT;
                        hr = m_pScreenSizeTemporaryTexture[dst]->GetSurfaceLevel(0, &pRT);
                        m_pD3DDev->SetRenderTarget(0, pRT);
                    }

                    CExternalPixelShader &Shader = m_pPixelShadersScreenSpace.GetNext(pos);
                    if (!Shader.m_pPixelShader)
                        Shader.Compile(m_pPSC);
                    hr = m_pD3DDev->SetPixelShader(Shader.m_pPixelShader);
                    TextureCopy(m_pScreenSizeTemporaryTexture[src]);

                    swap(src, dst);
                }

                hr = m_pD3DDev->SetPixelShader(NULL);
            }
        }
        else
        {
            if(pBackBuffer)
            {
                ClipToSurface(pBackBuffer, rSrcVid, rDstVid);
                // rSrcVid has to be aligned on mod2 for yuy2->rgb conversion with StretchRect
                rSrcVid.left &= ~1;
                rSrcVid.right &= ~1;
                rSrcVid.top &= ~1;
                rSrcVid.bottom &= ~1;
                hr = m_pD3DDev->StretchRect(m_pVideoSurface[m_nCurSurface], rSrcVid, pBackBuffer, rDstVid, m_filter);
                if(FAILED(hr)) return false;
            }
        }
    }
    AlphaBltSubPic(rSrcPri.Size());
    if (m_VMR9AlphaBitmap.dwFlags & VMRBITMAP_UPDATE)
    {
        CAutoLock BitMapLock(&m_VMR9AlphaBitmapLock);
        CRect		rcSrc (m_VMR9AlphaBitmap.rSrc);
        m_pOSDTexture	= NULL;
        m_pOSDSurface	= NULL;
        if ((m_VMR9AlphaBitmap.dwFlags & VMRBITMAP_DISABLE) == 0 && (BYTE *)m_VMR9AlphaBitmapData)
        {
            if( (m_pD3DXLoadSurfaceFromMemory != NULL) &&
                SUCCEEDED(hr = m_pD3DDev->CreateTexture(rcSrc.Width(), rcSrc.Height(), 1,
                               D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
                               D3DPOOL_DEFAULT, &m_pOSDTexture, NULL)) )
            {
                if (SUCCEEDED (hr = m_pOSDTexture->GetSurfaceLevel(0, &m_pOSDSurface)))
                {
                    hr = m_pD3DXLoadSurfaceFromMemory (m_pOSDSurface, NULL, NULL, (BYTE *)m_VMR9AlphaBitmapData, D3DFMT_A8R8G8B8, m_VMR9AlphaBitmapWidthBytes,
                                                       NULL, &m_VMR9AlphaBitmapRect, D3DX_FILTER_NONE, m_VMR9AlphaBitmap.clrSrcKey);
                }
                if (FAILED (hr))
                {
                    m_pOSDTexture	= NULL;
                    m_pOSDSurface	= NULL;
                }
            }
        }
        m_VMR9AlphaBitmap.dwFlags ^= VMRBITMAP_UPDATE;
    }
    if (pApp->m_fDisplayStats) DrawStats();
    if (m_pOSDTexture) AlphaBlt(rSrcPri, rDstPri, m_pOSDTexture);
    m_pD3DDev->EndScene();

    if (m_pD3DDevEx)
    {
        if (m_bIsFullscreen)
            hr = m_pD3DDevEx->PresentEx(NULL, NULL, NULL, NULL, NULL);
        else
            hr = m_pD3DDevEx->PresentEx(rSrcPri, rDstPri, NULL, NULL, NULL);
    }
    else
    {
        if (m_bIsFullscreen)
            hr = m_pD3DDev->Present(NULL, NULL, NULL, NULL);
        else
            hr = m_pD3DDev->Present(rSrcPri, rDstPri, NULL, NULL);
    }
    if(FAILED(hr)) _tprintf(_T("Device lost or something\n"));
    // Calculate timing statistics
    if (m_pRefClock) m_pRefClock->GetTime(&llCurRefTime); // To check if we called Present too late to hit the right vsync
    m_llEstVBlankTime = max(m_llEstVBlankTime, llCurRefTime); // Sometimes the real value is larger than the estimated value (but never smaller)
    if (pApp->m_fDisplayStats < 3) // Partial on-screen statistics
        SyncStats(m_llEstVBlankTime); // Max of estimate and real. Sometimes Present may actually return immediately so we need the estimate as a lower bound
    if (pApp->m_fDisplayStats == 1) // Full on-screen statistics
        SyncOffsetStats(-llSyncOffset); // Minus because we want time to flow downward in the graph in DrawStats

    // Adjust sync
    double frameCycle = (double)((m_llSampleTime - m_llLastSampleTime) / 10000.0);
    if (frameCycle < 0) frameCycle = 0.0; // Happens when searching.

    if (s.m_RenderSettings.bSynchronizeVideo) m_pGenlock->ControlClock(dSyncOffset, frameCycle);
    else if (s.m_RenderSettings.bSynchronizeDisplay) m_pGenlock->ControlDisplay(dSyncOffset, frameCycle);
    else m_pGenlock->UpdateStats(dSyncOffset, frameCycle); // No sync or sync to nearest neighbor

    m_dFrameCycle = m_pGenlock->frameCycleAvg;
    if (m_dFrameCycle > 0.0) m_fps = 1000.0 / m_dFrameCycle;
    m_dCycleDifference = GetCycleDifference();
    if (abs(m_dCycleDifference) < 0.05) // If less than 5% speed difference
        m_bSnapToVSync = true;
    else
        m_bSnapToVSync = false;

    // Check how well audio is matching rate (if at all)
    DWORD tmp;
    if (m_pAudioStats != NULL)
    {
        m_pAudioStats->GetStatParam(AM_AUDREND_STAT_PARAM_SLAVE_ACCUMERROR, &m_lAudioLag, &tmp);
        m_lAudioLagMin = min((long)m_lAudioLag, m_lAudioLagMin);
        m_lAudioLagMax = max((long)m_lAudioLag, m_lAudioLagMax);
        m_pAudioStats->GetStatParam(AM_AUDREND_STAT_PARAM_SLAVE_MODE, &m_lAudioSlaveMode, &tmp);
    }

    if (pApp->m_bResetStats)
    {
        ResetStats();
        pApp->m_bResetStats = false;
    }

    bool fResetDevice = m_bPendingResetDevice;
    if(hr == D3DERR_DEVICELOST && m_pD3DDev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET || hr == S_PRESENT_MODE_CHANGED)
        fResetDevice = true;
    if (SettingsNeedResetDevice())
        fResetDevice = true;

    BOOL bCompositionEnabled = false;
    if (m_pDwmIsCompositionEnabled) m_pDwmIsCompositionEnabled(&bCompositionEnabled);
    if ((bCompositionEnabled != 0) != m_bCompositionEnabled)
    {
        if (m_bIsFullscreen)
        {
            m_bCompositionEnabled = (bCompositionEnabled != 0);
        }
        else
            fResetDevice = true;
    }

    if(s.fResetDevice)
	{
		LONGLONG time = GetRenderersData()->GetPerfCounter();
		if (time > m_LastAdapterCheck + 20000000) // check every 2 sec.
		{
			m_LastAdapterCheck = time;
#ifdef _DEBUG
			D3DDEVICE_CREATION_PARAMETERS Parameters;
			if(SUCCEEDED(m_pD3DDev->GetCreationParameters(&Parameters)))
			{
				ASSERT(Parameters.AdapterOrdinal == m_CurrentAdapter);
			}
#endif
			if(m_CurrentAdapter != GetAdapter(m_pD3D))
			{
				fResetDevice = true;
			}
#ifdef _DEBUG
			else
			{
				ASSERT(m_pD3D->GetAdapterMonitor(m_CurrentAdapter) == m_pD3D->GetAdapterMonitor(GetAdapter(m_pD3D)));
			}
#endif
		}
	}

    if(fResetDevice)
	{
		m_bPendingResetDevice = true;
		SendResetRequest();
    }
    return(true);
}

void CBaseAP::SendResetRequest()
{
	if (!m_bDeviceResetRequested)
	{
		m_bDeviceResetRequested = true;
		AfxGetApp()->m_pMainWnd->PostMessage(WM_RESET_DEVICE);
	}
}

STDMETHODIMP_(bool) CBaseAP::ResetDevice()
{
    DeleteSurfaces();
    HRESULT hr;
    CString Error;
    if(FAILED(hr = CreateDXDevice(Error)) || FAILED(hr = AllocSurfaces()))
	{
		m_bDeviceResetRequested = false;
		return false;
	}
    m_pGenlock->SetMonitor(GetAdapter(m_pD3D));
    m_pGenlock->GetTiming();
	OnResetDevice();
	m_bDeviceResetRequested = false;
	m_bPendingResetDevice = false;
    return true;
}

void CBaseAP::DrawText(const RECT &rc, const CString &strText, int _Priority)
{
    if (_Priority < 1) return;
    int Quality = 1;
    D3DXCOLOR Color1(1.0f, 0.2f, 0.2f, 1.0f );
    D3DXCOLOR Color0(0.0f, 0.0f, 0.0f, 1.0f );
    RECT Rect1 = rc;
    RECT Rect2 = rc;
    if (Quality == 1)
        OffsetRect(&Rect2 , 2, 2);
    else
        OffsetRect(&Rect2 , -1, -1);
    if (Quality > 0)
        m_pFont->DrawText(m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
    OffsetRect (&Rect2 , 1, 0);
    if (Quality > 3)
        m_pFont->DrawText(m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
    OffsetRect (&Rect2 , 1, 0);
    if (Quality > 2)
        m_pFont->DrawText(m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
    OffsetRect (&Rect2 , 0, 1);
    if (Quality > 3)
        m_pFont->DrawText(m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
    OffsetRect (&Rect2 , 0, 1);
    if (Quality > 1)
        m_pFont->DrawText(m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
    OffsetRect (&Rect2 , -1, 0);
    if (Quality > 3)
        m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
    OffsetRect (&Rect2 , -1, 0);
    if (Quality > 2)
        m_pFont->DrawText(m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
    OffsetRect (&Rect2 , 0, -1);
    if (Quality > 3)
        m_pFont->DrawText(m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
    m_pFont->DrawText( m_pSprite, strText, -1, &Rect1, DT_NOCLIP, Color1);
}

void CBaseAP::DrawStats()
{
    CRenderersSettings& s = GetRenderersSettings();
    CRenderersData * pApp = GetRenderersData();

    LONGLONG llMaxJitter = m_MaxJitter;
    LONGLONG llMinJitter = m_MinJitter;
    LONGLONG llMaxSyncOffset = m_MaxSyncOffset;
    LONGLONG llMinSyncOffset = m_MinSyncOffset;

    RECT rc = {20, 20, 520, 520 };
    // pApp->m_fDisplayStats = 1 for full stats, 2 for little less, 3 for basic, 0 for no stats
    if (m_pFont && m_pSprite)
    {
        m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
        CString	strText;
        int TextHeight = 25.0*m_TextScale + 0.5;

        strText.Format(L"Frames drawn from stream start: %d | Sample time stamp: %d ms", m_pcFramesDrawn, (LONG)(m_llSampleTime / 10000));
        DrawText(rc, strText, 1);
        OffsetRect(&rc, 0, TextHeight);

        if (pApp->m_fDisplayStats == 1)
        {
            strText.Format(L"Frame cycle: %.3f ms [%.3f ms, %.3f ms] | Frame rate: %.3f fps", m_dFrameCycle, m_pGenlock->minFrameCycle, m_pGenlock->maxFrameCycle, m_fps);
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);

            strText.Format(L"Measured closest match display cycle: %.3f ms | Measured base display cycle: %.3f ms", m_dOptimumDisplayCycle, m_dEstRefreshCycle);
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);

            strText.Format(L"Display cycle - frame cycle mismatch: %.3f %%", 100 * m_dCycleDifference);
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);

            strText.Format(L"Actual frame cycle: %+5.3f ms [%+.3f ms, %+.3f ms] | Actual frame rate: %.3f fps", m_fJitterMean / 10000.0, (double(llMinJitter)/10000.0), (double(llMaxJitter)/10000.0), 10000000.0 / m_fJitterMean);
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);

            strText.Format(L"Display cycle from Windows: %.3f ms | Display refresh rate from Windows: %d Hz", m_dD3DRefreshCycle, m_uD3DRefreshRate);
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);

            if (m_pGenlock->powerstripTimingExists)
            {
                strText.Format(L"Display cycle from Powerstrip: %.3f ms | Display refresh rate from Powerstrip: %.3f Hz", 1000.0 / m_pGenlock->curDisplayFreq, m_pGenlock->curDisplayFreq);
                DrawText(rc, strText, 1);
                OffsetRect(&rc, 0, TextHeight);
            }

            if ((m_caps.Caps & D3DCAPS_READ_SCANLINE) == 0)
            {
                strText.Format(L"Graphics device does not support scan line access. No sync is possible");
                DrawText(rc, strText, 1);
                OffsetRect(&rc, 0, TextHeight);
            }

            strText.Format(L"Video resolution: %d x %d | Aspect ratio: %d x %d", m_NativeVideoSize.cx, m_NativeVideoSize.cy, m_AspectRatio.cx, m_AspectRatio.cy);
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);

            strText.Format(L"Display resolution: %d x %d", m_ScreenSize.cx, m_ScreenSize.cy);
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);

            if (s.m_RenderSettings.bSynchronizeDisplay || s.m_RenderSettings.bSynchronizeVideo)
            {
                if (s.m_RenderSettings.bSynchronizeDisplay && !m_pGenlock->PowerstripRunning())
                {
                    strText.Format(L"PowerStrip is not running. No display sync is possible.");
                    DrawText(rc, strText, 1);
                    OffsetRect(&rc, 0, TextHeight);
                }
                else
                {
                    strText.Format(L"Sync adjustment: %d | # of adjustments: %d", m_pGenlock->adjDelta, (m_pGenlock->clockAdjustmentsMade + m_pGenlock->displayAdjustmentsMade) / 2);
                    DrawText(rc, strText, 1);
                    OffsetRect(&rc, 0, TextHeight);
                }
            }
        }

        strText.Format(L"Average sync offset: %3.1f ms [%.1f ms, %.1f ms] | Target sync offset: %3.1f ms", m_pGenlock->syncOffsetAvg, m_pGenlock->minSyncOffset, m_pGenlock->maxSyncOffset, s.m_RenderSettings.fTargetSyncOffset);
        DrawText(rc, strText, 1);
        OffsetRect(&rc, 0, TextHeight);

        if (pApp->m_fDisplayStats < 3)
        {
            strText.Format(L"# of sync glitches: %d", m_uSyncGlitches);
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);

            strText.Format(L"# of frames dropped: %d", m_pcFramesDropped);
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);
        }

        if (pApp->m_fDisplayStats == 1)
        {
            if (m_pAudioStats && s.m_RenderSettings.bSynchronizeVideo)
            {
                strText.Format(L"Audio lag: %3d ms [%d ms, %d ms] | %s", m_lAudioLag, m_lAudioLagMin, m_lAudioLagMax, (m_lAudioSlaveMode == 4) ? _T("Audio renderer is matching rate (for analog sound output)") : _T("Audio renderer is not matching rate"));
                DrawText(rc, strText, 1);
                OffsetRect(&rc, 0, TextHeight);
            }

            strText.Format(L"Sample waiting time: %d ms", m_lNextSampleWait);
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);
            if (s.m_RenderSettings.bSynchronizeNearest)
            {
                strText.Format(L"Sample paint time correction: %2d ms | Hysteresis: %d", m_lShiftToNearest, m_llHysteresis /10000);
                DrawText(rc, strText, 1);
                OffsetRect(&rc, 0, TextHeight);
            }

            strText.Format(L"Buffered: %3d | Free: %3d | Current surface: %3d", m_nUsedBuffer, m_nDXSurface - m_nUsedBuffer, m_nCurSurface, m_nVMR9Surfaces, m_iVMR9Surface);
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);

            strText.Format(L"Settings: ");

            if (m_bIsFullscreen)
                strText += "D3DFS ";
            if (s.m_RenderSettings.iVMRDisableDesktopComposition)
                strText += "DisDC ";
            if (s.m_RenderSettings.bSynchronizeVideo)
                strText += "SyncVideo ";
            if (s.m_RenderSettings.bSynchronizeDisplay)
                strText += "SyncDisplay ";
            if (s.m_RenderSettings.bSynchronizeNearest)
                strText += "SyncNearest ";
            if (m_bHighColorResolution)
                strText += "10 bit ";
            if (s.m_RenderSettings.iEVROutputRange == 0)
                strText += "0-255 ";
            else if (s.m_RenderSettings.iEVROutputRange == 1)
                strText += "16-235 ";

            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);

            strText.Format(L"%s: %s", GetDXVAVersion(), GetDXVADecoderDescription());
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);

            strText.Format(L"DirectX SDK: %d", GetRenderersData()->GetDXSdkRelease());
            DrawText(rc, strText, 1);
            OffsetRect(&rc, 0, TextHeight);

            for (int i=0; i<6; i++)
            {
                if (m_strStatsMsg[i][0])
                {
                    DrawText(rc, m_strStatsMsg[i], 1);
                    OffsetRect(&rc, 0, TextHeight);
                }
            }
        }
        OffsetRect(&rc, 0, TextHeight); // Extra "line feed"
        m_pSprite->End();
    }

    if (m_pLine && (pApp->m_fDisplayStats < 3))
    {
        D3DXVECTOR2	Points[NB_JITTER];
        int nIndex;

        int DrawWidth = 625;
        int DrawHeight = 250;
        int Alpha = 80;
        int StartX = rc.left;
        int StartY = rc.top;

        DrawRect(RGB(0, 0, 0), Alpha, CRect(StartX, StartY, StartX + DrawWidth, StartY + DrawHeight));
        m_pLine->SetWidth(2.5);
        m_pLine->SetAntialias(1);
        m_pLine->Begin();

        for (int i = 0; i <= DrawHeight; i += 5)
        {
            Points[0].x = (FLOAT)StartX;
            Points[0].y = (FLOAT)(StartY + i);
            Points[1].x = (FLOAT)(StartX + ((i + 25) % 25 ? 50 : 625));
            Points[1].y = (FLOAT)(StartY + i);
            m_pLine->Draw(Points, 2, D3DCOLOR_XRGB(100, 100, 255));
        }

        for (int i = 0; i < DrawWidth; i += 125) // Every 25:th sample
        {
            Points[0].x = (FLOAT)(StartX + i);
            Points[0].y = (FLOAT)(StartY + DrawHeight / 2);
            Points[1].x = (FLOAT)(StartX + i);
            Points[1].y = (FLOAT)(StartY + DrawHeight / 2 + 10);
            m_pLine->Draw(Points, 2, D3DCOLOR_XRGB(100, 100, 255));
        }

        for (int i = 0; i < NB_JITTER; i++)
        {
            nIndex = (m_nNextJitter + 1 + i) % NB_JITTER;
            if (nIndex < 0)
                nIndex += NB_JITTER;
            double Jitter = m_pllJitter[nIndex] - m_fJitterMean;
            Points[i].x  = (FLOAT)(StartX + (i * 5));
            Points[i].y  = (FLOAT)(StartY + (Jitter / 2000.0 + 125.0));
        }
        m_pLine->Draw(Points, NB_JITTER, D3DCOLOR_XRGB(255, 100, 100));

        if (pApp->m_fDisplayStats == 1) // Full on-screen statistics
        {
            for (int i = 0; i < NB_JITTER; i++)
            {
                nIndex = (m_nNextSyncOffset + 1 + i) % NB_JITTER;
                if (nIndex < 0)
                    nIndex += NB_JITTER;
                Points[i].x  = (FLOAT)(StartX + (i * 5));
                Points[i].y  = (FLOAT)(StartY + ((m_pllSyncOffset[nIndex]) / 2000 + 125));
            }
            m_pLine->Draw(Points, NB_JITTER, D3DCOLOR_XRGB(100, 200, 100));
        }
        m_pLine->End();
    }
}

double CBaseAP::GetRefreshRate()
{
    if (m_pGenlock->powerstripTimingExists) return m_pGenlock->curDisplayFreq;
    else return (double)m_uD3DRefreshRate;
}

double CBaseAP::GetDisplayCycle()
{
    if (m_pGenlock->powerstripTimingExists) return 1000.0 / m_pGenlock->curDisplayFreq;
    else return (double)m_dD3DRefreshCycle;
}

double CBaseAP::GetCycleDifference()
{
    double dBaseDisplayCycle = GetDisplayCycle();
    UINT i;
    double minDiff = 1.0;
    if (dBaseDisplayCycle == 0.0 || m_dFrameCycle == 0.0)
        return 1.0;
    else
    {
        for (i = 1; i <= 8; i++) // Try a lot of multiples of the display frequency
        {
            double dDisplayCycle = i * dBaseDisplayCycle;
            double diff = (dDisplayCycle - m_dFrameCycle) / m_dFrameCycle;
            if (abs(diff) < abs(minDiff))
            {
                minDiff = diff;
                m_dOptimumDisplayCycle = dDisplayCycle;
            }
        }
    }
    return minDiff;
}

void CBaseAP::EstimateRefreshTimings()
{
    if (m_pD3DDev)
    {
        CRenderersData *pApp = GetRenderersData();
        D3DRASTER_STATUS rasterStatus;
        m_pD3DDev->GetRasterStatus(0, &rasterStatus);
        while (rasterStatus.ScanLine != 0) m_pD3DDev->GetRasterStatus(0, &rasterStatus);
        while (rasterStatus.ScanLine == 0) m_pD3DDev->GetRasterStatus(0, &rasterStatus);
        m_pD3DDev->GetRasterStatus(0, &rasterStatus);
        LONGLONG startTime = pApp->GetPerfCounter();
        UINT startLine = rasterStatus.ScanLine;
        LONGLONG endTime = 0;
        LONGLONG time = 0;
        UINT endLine = 0;
        UINT line = 0;
        bool done = false;
        while (!done) // Estimate time for one scan line
        {
            m_pD3DDev->GetRasterStatus(0, &rasterStatus);
            line = rasterStatus.ScanLine;
            time = pApp->GetPerfCounter();
            if (line > 0)
            {
                endLine = line;
                endTime = time;
            }
            else
                done = true;
        }
        m_dDetectedScanlineTime = (double)(endTime - startTime) / (double)((endLine - startLine) * 10000.0);

        // Estimate the display refresh rate from the vsyncs
        m_pD3DDev->GetRasterStatus(0, &rasterStatus);
        while (rasterStatus.ScanLine != 0) m_pD3DDev->GetRasterStatus(0, &rasterStatus);
        // Now we're at the start of a vsync
        startTime = pApp->GetPerfCounter();
        UINT i;
        for (i = 1; i <= 50; i++)
        {
            m_pD3DDev->GetRasterStatus(0, &rasterStatus);
            while (rasterStatus.ScanLine == 0) m_pD3DDev->GetRasterStatus(0, &rasterStatus);
            while (rasterStatus.ScanLine != 0) m_pD3DDev->GetRasterStatus(0, &rasterStatus);
            // Now we're at the next vsync
        }
        endTime = pApp->GetPerfCounter();
        m_dEstRefreshCycle = (double)(endTime - startTime) / ((i - 1) * 10000.0);
    }
}

bool CBaseAP::ExtractInterlaced(const AM_MEDIA_TYPE* pmt)
{
    if (pmt->formattype==FORMAT_VideoInfo)
        return false;
    else if (pmt->formattype==FORMAT_VideoInfo2)
        return (((VIDEOINFOHEADER2*)pmt->pbFormat)->dwInterlaceFlags & AMINTERLACE_IsInterlaced) != 0;
    else if (pmt->formattype==FORMAT_MPEGVideo)
        return false;
    else if (pmt->formattype==FORMAT_MPEG2Video)
        return (((MPEG2VIDEOINFO*)pmt->pbFormat)->hdr.dwInterlaceFlags & AMINTERLACE_IsInterlaced) != 0;
    else
        return false;
}

STDMETHODIMP CBaseAP::GetDIB(BYTE* lpDib, DWORD* size)
{
    CheckPointer(size, E_POINTER);

    HRESULT hr;

    D3DSURFACE_DESC desc;
    memset(&desc, 0, sizeof(desc));
    m_pVideoSurface[m_nCurSurface]->GetDesc(&desc);

    DWORD required = sizeof(BITMAPINFOHEADER) + (desc.Width * desc.Height * 32 >> 3);
    if(!lpDib)
    {
        *size = required;
        return S_OK;
    }
    if(*size < required) return E_OUTOFMEMORY;
    *size = required;

    CComPtr<IDirect3DSurface9> pSurface = m_pVideoSurface[m_nCurSurface];
    D3DLOCKED_RECT r;
    if(FAILED(hr = pSurface->LockRect(&r, NULL, D3DLOCK_READONLY)))
    {
        pSurface = NULL;
        if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &pSurface, NULL))
           || FAILED(hr = m_pD3DDev->GetRenderTargetData(m_pVideoSurface[m_nCurSurface], pSurface))
           || FAILED(hr = pSurface->LockRect(&r, NULL, D3DLOCK_READONLY)))
            return hr;
    }

    BITMAPINFOHEADER* bih = (BITMAPINFOHEADER*)lpDib;
    memset(bih, 0, sizeof(BITMAPINFOHEADER));
    bih->biSize = sizeof(BITMAPINFOHEADER);
    bih->biWidth = desc.Width;
    bih->biHeight = desc.Height;
    bih->biBitCount = 32;
    bih->biPlanes = 1;
    bih->biSizeImage = bih->biWidth * bih->biHeight * bih->biBitCount >> 3;

    BitBltFromRGBToRGB(
        bih->biWidth, bih->biHeight,
        (BYTE*)(bih + 1), bih->biWidth*bih->biBitCount>>3, bih->biBitCount,
        (BYTE*)r.pBits + r.Pitch*(desc.Height-1), -(int)r.Pitch, 32);

    pSurface->UnlockRect();

    return S_OK;
}

STDMETHODIMP CBaseAP::SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget)
{
    return SetPixelShader2(pSrcData, pTarget, false);
}

STDMETHODIMP CBaseAP::SetPixelShader2(LPCSTR pSrcData, LPCSTR pTarget, bool bScreenSpace)
{
    CAutoLock cRenderLock(&m_allocatorLock);

    CAtlList<CExternalPixelShader> *pPixelShaders;
    if (bScreenSpace)
        pPixelShaders = &m_pPixelShadersScreenSpace;
    else
        pPixelShaders = &m_pPixelShaders;

    if(!pSrcData && !pTarget)
    {
        pPixelShaders->RemoveAll();
        m_pD3DDev->SetPixelShader(NULL);
        return S_OK;
    }

    if(!pSrcData || !pTarget)
        return E_INVALIDARG;

    CExternalPixelShader Shader;
    Shader.m_SourceData = pSrcData;
    Shader.m_SourceTarget = pTarget;

    CComPtr<IDirect3DPixelShader9> pPixelShader;

    HRESULT hr = Shader.Compile(m_pPSC);
    if(FAILED(hr))
        return hr;

    pPixelShaders->AddTail(Shader);
    Paint(true);
    return S_OK;
}


CSyncAP::CSyncAP(HWND hWnd, bool bFullscreen, HRESULT& hr, CString &_Error): CBaseAP(hWnd, bFullscreen, hr, _Error)
{
    HMODULE		hLib;
    CRenderersSettings& s = GetRenderersSettings();

    m_nResetToken = 0;
    m_hRenderThread  = INVALID_HANDLE_VALUE;
    m_hMixerThread= INVALID_HANDLE_VALUE;
	m_hEvtFlush = INVALID_HANDLE_VALUE;
	m_hEvtQuit = INVALID_HANDLE_VALUE;
	m_hEvtSkip = INVALID_HANDLE_VALUE;
    m_bEvtQuit = 0;
    m_bEvtFlush = 0;

    if (FAILED (hr))
    {
        _Error += L"SyncAP failed\n";
        return;
    }

    // Load EVR specifics DLLs
    hLib = LoadLibrary (L"dxva2.dll");
    pfDXVA2CreateDirect3DDeviceManager9	= hLib ? (PTR_DXVA2CreateDirect3DDeviceManager9) GetProcAddress (hLib, "DXVA2CreateDirect3DDeviceManager9") : NULL;

    // Load EVR functions
    hLib = LoadLibrary (L"evr.dll");
    pfMFCreateDXSurfaceBuffer = hLib ? (PTR_MFCreateDXSurfaceBuffer)GetProcAddress (hLib, "MFCreateDXSurfaceBuffer") : NULL;
    pfMFCreateVideoSampleFromSurface = hLib ? (PTR_MFCreateVideoSampleFromSurface)GetProcAddress (hLib, "MFCreateVideoSampleFromSurface") : NULL;
    pfMFCreateVideoMediaType = hLib ? (PTR_MFCreateVideoMediaType)GetProcAddress (hLib, "MFCreateVideoMediaType") : NULL;

    if (!pfDXVA2CreateDirect3DDeviceManager9 || !pfMFCreateDXSurfaceBuffer || !pfMFCreateVideoSampleFromSurface || !pfMFCreateVideoMediaType)
    {
        if (!pfDXVA2CreateDirect3DDeviceManager9)
            _Error += L"Could not find DXVA2CreateDirect3DDeviceManager9 (dxva2.dll)\n";
        if (!pfMFCreateDXSurfaceBuffer)
            _Error += L"Could not find MFCreateDXSurfaceBuffer (evr.dll)\n";
        if (!pfMFCreateVideoSampleFromSurface)
            _Error += L"Could not find MFCreateVideoSampleFromSurface (evr.dll)\n";
        if (!pfMFCreateVideoMediaType)
            _Error += L"Could not find MFCreateVideoMediaType (evr.dll)\n";
        hr = E_FAIL;
        return;
    }

    // Load Vista specific DLLs
    hLib = LoadLibrary (L"avrt.dll");
    pfAvSetMmThreadCharacteristicsW = hLib ? (PTR_AvSetMmThreadCharacteristicsW) GetProcAddress (hLib, "AvSetMmThreadCharacteristicsW") : NULL;
    pfAvSetMmThreadPriority = hLib ? (PTR_AvSetMmThreadPriority) GetProcAddress (hLib, "AvSetMmThreadPriority") : NULL;
    pfAvRevertMmThreadCharacteristics = hLib ? (PTR_AvRevertMmThreadCharacteristics) GetProcAddress (hLib, "AvRevertMmThreadCharacteristics") : NULL;

    // Init DXVA manager
    hr = pfDXVA2CreateDirect3DDeviceManager9(&m_nResetToken, &m_pD3DManager);
    if (SUCCEEDED (hr))
    {
        hr = m_pD3DManager->ResetDevice(m_pD3DDev, m_nResetToken);
        if (!SUCCEEDED (hr))
        {
            _Error += L"m_pD3DManager->ResetDevice failed\n";
        }
    }
    else
        _Error += L"DXVA2CreateDirect3DDeviceManager9 failed\n";

    CComPtr<IDirectXVideoDecoderService> pDecoderService;
    HANDLE hDevice;
    if (SUCCEEDED (m_pD3DManager->OpenDeviceHandle(&hDevice)) &&
        SUCCEEDED (m_pD3DManager->GetVideoService (hDevice, __uuidof(IDirectXVideoDecoderService), (void**)&pDecoderService)))
    {
        HookDirectXVideoDecoderService (pDecoderService);
        m_pD3DManager->CloseDeviceHandle (hDevice);
    }

    // Bufferize frame only with 3D texture
    if (s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
        m_nDXSurface = max(min (s.iEvrBuffers, MAX_PICTURE_SLOTS-2), 4);
    else
        m_nDXSurface = 1;

    m_nRenderState = Shutdown;
    m_bStepping = false;
    m_bUseInternalTimer = false;
    m_LastSetOutputRange = -1;
    m_bPendingRenegotiate = false;
    m_bPendingMediaFinished = false;
    m_pCurrentDisplaydSample = NULL;
    m_nStepCount = 0;
    m_dwVideoAspectRatioMode = MFVideoARMode_PreservePicture;
    m_dwVideoRenderPrefs = (MFVideoRenderPrefs)0;
    m_BorderColor = RGB (0,0,0);
    m_pOuterEVR = NULL;
    m_bPrerolled = false;
    m_lShiftToNearest = -1; // Illegal value to start with
}

CSyncAP::~CSyncAP(void)
{
    StopWorkerThreads();
    m_pMediaType = NULL;
    m_pClock = NULL;
    m_pD3DManager = NULL;
}

HRESULT CSyncAP::CheckShutdown() const
{
    if (m_nRenderState == Shutdown) return MF_E_SHUTDOWN;
    else return S_OK;
}

void CSyncAP::StartWorkerThreads()
{
    DWORD dwThreadId;
    if (m_nRenderState == Shutdown)
    {
        m_hEvtQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
        m_hEvtFlush = CreateEvent(NULL, TRUE, FALSE, NULL);
        m_hEvtSkip = CreateEvent(NULL, TRUE, FALSE, NULL);
        m_hMixerThread = ::CreateThread(NULL, 0, MixerThreadStatic, (LPVOID)this, 0, &dwThreadId);
        SetThreadPriority(m_hMixerThread, THREAD_PRIORITY_HIGHEST);
        m_hRenderThread = ::CreateThread(NULL, 0, RenderThreadStatic, (LPVOID)this, 0, &dwThreadId);
        SetThreadPriority(m_hRenderThread, THREAD_PRIORITY_TIME_CRITICAL);
        m_nRenderState = Stopped;
    }
}

void CSyncAP::StopWorkerThreads()
{
    if (m_nRenderState != Shutdown)
    {
        SetEvent(m_hEvtFlush);
        m_bEvtFlush = true;
        SetEvent(m_hEvtQuit);
        m_bEvtQuit = true;
        SetEvent(m_hEvtSkip);
        m_bEvtSkip = true;
        if ((m_hRenderThread != INVALID_HANDLE_VALUE) && (WaitForSingleObject (m_hRenderThread, 10000) == WAIT_TIMEOUT))
        {
            ASSERT (FALSE);
            TerminateThread (m_hRenderThread, 0xDEAD);
        }
        if (m_hRenderThread != INVALID_HANDLE_VALUE) CloseHandle (m_hRenderThread);
        if ((m_hMixerThread != INVALID_HANDLE_VALUE) && (WaitForSingleObject (m_hMixerThread, 10000) == WAIT_TIMEOUT))
        {
            ASSERT (FALSE);
            TerminateThread (m_hMixerThread, 0xDEAD);
        }
        if (m_hMixerThread != INVALID_HANDLE_VALUE) CloseHandle (m_hMixerThread);

        if (m_hEvtFlush != INVALID_HANDLE_VALUE) CloseHandle(m_hEvtFlush);
        if (m_hEvtQuit != INVALID_HANDLE_VALUE) CloseHandle(m_hEvtQuit);
        if (m_hEvtSkip != INVALID_HANDLE_VALUE) CloseHandle(m_hEvtSkip);

        m_bEvtFlush = false;
        m_bEvtQuit = false;
        m_bEvtSkip = false;
    }
    m_nRenderState = Shutdown;
}

STDMETHODIMP CSyncAP::CreateRenderer(IUnknown** ppRenderer)
{
    CheckPointer(ppRenderer, E_POINTER);
    *ppRenderer = NULL;
    HRESULT hr = E_FAIL;

    do
    {
        CMacrovisionKicker* pMK = DNew CMacrovisionKicker(NAME("CMacrovisionKicker"), NULL);
        CComPtr<IUnknown> pUnk = (IUnknown*)(INonDelegatingUnknown*)pMK;

        CSyncRenderer *pOuterEVR = DNew CSyncRenderer(NAME("CSyncRenderer"), pUnk, hr, &m_VMR9AlphaBitmap, this);
        m_pOuterEVR = pOuterEVR;

        pMK->SetInner((IUnknown*)(INonDelegatingUnknown*)pOuterEVR);
        CComQIPtr<IBaseFilter> pBF = pUnk;

        if (FAILED(hr)) break;

        // Set EVR custom presenter
        CComPtr<IMFVideoPresenter> pVP;
        CComPtr<IMFVideoRenderer> pMFVR;
        CComQIPtr<IMFGetService, &__uuidof(IMFGetService)> pMFGS = pBF;

        hr = pMFGS->GetService (MR_VIDEO_RENDER_SERVICE, IID_IMFVideoRenderer, (void**)&pMFVR);

        if(SUCCEEDED(hr)) hr = QueryInterface(__uuidof(IMFVideoPresenter), (void**)&pVP);
        if(SUCCEEDED(hr)) hr = pMFVR->InitializeRenderer(NULL, pVP);

        CComPtr<IPin> pPin = GetFirstPin(pBF);
        CComQIPtr<IMemInputPin> pMemInputPin = pPin;

        m_bUseInternalTimer = HookNewSegmentAndReceive((IPinC*)(IPin*)pPin, (IMemInputPinC*)(IMemInputPin*)pMemInputPin);
        if(FAILED(hr))
            *ppRenderer = NULL;
        else
            *ppRenderer = pBF.Detach();
    }
    while (0);

    return hr;
}

STDMETHODIMP_(bool) CSyncAP::Paint(bool fAll)
{
    return __super::Paint(fAll);
}

STDMETHODIMP CSyncAP::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    HRESULT		hr;
    if(riid == __uuidof(IMFClockStateSink))
        hr = GetInterface((IMFClockStateSink*)this, ppv);
    else if(riid == __uuidof(IMFVideoPresenter))
        hr = GetInterface((IMFVideoPresenter*)this, ppv);
    else if(riid == __uuidof(IMFTopologyServiceLookupClient))
        hr = GetInterface((IMFTopologyServiceLookupClient*)this, ppv);
    else if(riid == __uuidof(IMFVideoDeviceID))
        hr = GetInterface((IMFVideoDeviceID*)this, ppv);
    else if(riid == __uuidof(IMFGetService))
        hr = GetInterface((IMFGetService*)this, ppv);
    else if(riid == __uuidof(IMFAsyncCallback))
        hr = GetInterface((IMFAsyncCallback*)this, ppv);
    else if(riid == __uuidof(IMFVideoDisplayControl))
        hr = GetInterface((IMFVideoDisplayControl*)this, ppv);
    else if(riid == __uuidof(IEVRTrustedVideoPlugin))
        hr = GetInterface((IEVRTrustedVideoPlugin*)this, ppv);
    else if(riid == IID_IQualProp)
        hr = GetInterface((IQualProp*)this, ppv);
    else if(riid == __uuidof(IMFRateSupport))
        hr = GetInterface((IMFRateSupport*)this, ppv);
    else if(riid == __uuidof(IDirect3DDeviceManager9))
		hr = m_pD3DManager->QueryInterface (__uuidof(IDirect3DDeviceManager9), (void**) ppv);
	else if(riid == __uuidof(ISyncClockAdviser))
		hr = GetInterface((ISyncClockAdviser*)this, ppv);
    else
        hr = __super::NonDelegatingQueryInterface(riid, ppv);

    return hr;
}

// IMFClockStateSink
STDMETHODIMP CSyncAP::OnClockStart(MFTIME hnsSystemTime,  LONGLONG llClockStartOffset)
{
    m_nRenderState = Started;
    return S_OK;
}

STDMETHODIMP CSyncAP::OnClockStop(MFTIME hnsSystemTime)
{
    m_nRenderState = Stopped;
    return S_OK;
}

STDMETHODIMP CSyncAP::OnClockPause(MFTIME hnsSystemTime)
{
    m_nRenderState = Paused;
    return S_OK;
}

STDMETHODIMP CSyncAP::OnClockRestart(MFTIME hnsSystemTime)
{
    m_nRenderState	= Started;
    return S_OK;
}

STDMETHODIMP CSyncAP::OnClockSetRate(MFTIME hnsSystemTime, float flRate)
{
    return E_NOTIMPL;
}

// IBaseFilter delegate
bool CSyncAP::GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *State, HRESULT &_ReturnValue)
{
    CAutoLock lock(&m_SampleQueueLock);
    switch(m_nRenderState)
    {
    case Started:
        *State = State_Running;
        break;
    case Paused:
        *State = State_Paused;
        break;
    case Stopped:
        *State = State_Stopped;
        break;
    default:
        *State = State_Stopped;
        _ReturnValue = E_FAIL;
    }
    _ReturnValue = S_OK;
    return true;
}

// IQualProp
STDMETHODIMP CSyncAP::get_FramesDroppedInRenderer(int *pcFrames)
{
    *pcFrames = m_pcFramesDropped;
    return S_OK;
}
STDMETHODIMP CSyncAP::get_FramesDrawn(int *pcFramesDrawn)
{
    *pcFramesDrawn = m_pcFramesDrawn;
    return S_OK;
}
STDMETHODIMP CSyncAP::get_AvgFrameRate(int *piAvgFrameRate)
{
    *piAvgFrameRate = (int)(m_fAvrFps * 100);
    return S_OK;
}
STDMETHODIMP CSyncAP::get_Jitter(int *iJitter)
{
    *iJitter = (int)((m_fJitterStdDev/10000.0) + 0.5);
    return S_OK;
}
STDMETHODIMP CSyncAP::get_AvgSyncOffset(int *piAvg)
{
    *piAvg = (int)((m_fSyncOffsetAvr/10000.0) + 0.5);
    return S_OK;
}
STDMETHODIMP CSyncAP::get_DevSyncOffset(int *piDev)
{
    *piDev = (int)((m_fSyncOffsetStdDev/10000.0) + 0.5);
    return S_OK;
}

// IMFRateSupport
STDMETHODIMP CSyncAP::GetSlowestRate(MFRATE_DIRECTION eDirection, BOOL fThin, float *pflRate)
{
    *pflRate = 0;
    return S_OK;
}

STDMETHODIMP CSyncAP::GetFastestRate(MFRATE_DIRECTION eDirection, BOOL fThin, float *pflRate)
{
    HRESULT		hr = S_OK;
    float		fMaxRate = 0.0f;

    CAutoLock lock(this);

    CheckPointer(pflRate, E_POINTER);
    CheckHR(CheckShutdown());

    // Get the maximum forward rate.
    fMaxRate = GetMaxRate(fThin);

    // For reverse playback, swap the sign.
    if (eDirection == MFRATE_REVERSE)
        fMaxRate = -fMaxRate;

    *pflRate = fMaxRate;
    return hr;
}

STDMETHODIMP CSyncAP::IsRateSupported(BOOL fThin, float flRate, float *pflNearestSupportedRate)
{
    // fRate can be negative for reverse playback.
    // pfNearestSupportedRate can be NULL.
    CAutoLock lock(this);
    HRESULT hr = S_OK;
    float   fMaxRate = 0.0f;
    float   fNearestRate = flRate;   // Default.

    CheckPointer (pflNearestSupportedRate, E_POINTER);
    CheckHR(hr = CheckShutdown());

    // Find the maximum forward rate.
    fMaxRate = GetMaxRate(fThin);

    if (fabsf(flRate) > fMaxRate)
    {
        // The (absolute) requested rate exceeds the maximum rate.
        hr = MF_E_UNSUPPORTED_RATE;

        // The nearest supported rate is fMaxRate.
        fNearestRate = fMaxRate;
        if (flRate < 0)
        {
            // For reverse playback, swap the sign.
            fNearestRate = -fNearestRate;
        }
    }
    // Return the nearest supported rate if the caller requested it.
    if (pflNearestSupportedRate != NULL) *pflNearestSupportedRate = fNearestRate;
    return hr;
}

float CSyncAP::GetMaxRate(BOOL bThin)
{
    float fMaxRate = FLT_MAX;  // Default.
    UINT32 fpsNumerator = 0, fpsDenominator = 0;
    UINT MonitorRateHz = 0;

    if (!bThin && (m_pMediaType != NULL))
    {
        // Non-thinned: Use the frame rate and monitor refresh rate.

        // Frame rate:
        MFGetAttributeRatio(m_pMediaType, MF_MT_FRAME_RATE,
                            &fpsNumerator, &fpsDenominator);

        // Monitor refresh rate:
        MonitorRateHz = m_uD3DRefreshRate; // D3DDISPLAYMODE

        if (fpsDenominator && fpsNumerator && MonitorRateHz)
        {
            // Max Rate = Refresh Rate / Frame Rate
            fMaxRate = (float)MulDiv(MonitorRateHz, fpsDenominator, fpsNumerator);
        }
    }
    return fMaxRate;
}

void CSyncAP::CompleteFrameStep(bool bCancel)
{
    if (m_nStepCount > 0)
    {
        if (bCancel || (m_nStepCount == 1))
        {
            m_pSink->Notify(EC_STEP_COMPLETE, bCancel ? TRUE : FALSE, 0);
            m_nStepCount = 0;
        }
        else
            m_nStepCount--;
    }
}

// IMFVideoPresenter
STDMETHODIMP CSyncAP::ProcessMessage(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam)
{
    HRESULT hr = S_OK;
    CRenderersSettings& s = GetRenderersSettings();

    switch (eMessage)
    {
    case MFVP_MESSAGE_BEGINSTREAMING:
        hr = BeginStreaming();
        m_llHysteresis = 0;
        m_lShiftToNearest = 0;
        m_bStepping = false;
        break;

    case MFVP_MESSAGE_CANCELSTEP:
        m_bStepping = false;
        CompleteFrameStep(true);
        break;

    case MFVP_MESSAGE_ENDOFSTREAM:
        m_bPendingMediaFinished = true;
        break;

    case MFVP_MESSAGE_ENDSTREAMING:
        m_pGenlock->ResetTiming();
        m_pRefClock = NULL;
		break;

    case MFVP_MESSAGE_FLUSH:
        SetEvent(m_hEvtFlush);
        m_bEvtFlush = true;
        while (WaitForSingleObject(m_hEvtFlush, 1) == WAIT_OBJECT_0);
        break;

    case MFVP_MESSAGE_INVALIDATEMEDIATYPE:
        m_bPendingRenegotiate = true;
        while (*((volatile bool *)&m_bPendingRenegotiate)) Sleep(1);
        break;

    case MFVP_MESSAGE_PROCESSINPUTNOTIFY:
        break;

    case MFVP_MESSAGE_STEP:
        m_nStepCount = ulParam;
        m_bStepping = true;
        break;

    default :
        ASSERT(FALSE);
        break;
    }
    return hr;
}

HRESULT CSyncAP::IsMediaTypeSupported(IMFMediaType* pMixerType)
{
    HRESULT hr;
    AM_MEDIA_TYPE* pAMMedia;
    UINT nInterlaceMode;

    CheckHR (pMixerType->GetRepresentation(FORMAT_VideoInfo2, (void**)&pAMMedia));
    CheckHR (pMixerType->GetUINT32 (MF_MT_INTERLACE_MODE, &nInterlaceMode));

    if ( (pAMMedia->majortype != MEDIATYPE_Video)) hr = MF_E_INVALIDMEDIATYPE;
    pMixerType->FreeRepresentation(FORMAT_VideoInfo2, (void*)pAMMedia);
    return hr;
}

HRESULT CSyncAP::CreateProposedOutputType(IMFMediaType* pMixerType, IMFMediaType** pType)
{
    HRESULT hr;
    AM_MEDIA_TYPE *pAMMedia = NULL;
    LARGE_INTEGER i64Size;
    MFVIDEOFORMAT *VideoFormat;

    CheckHR(pMixerType->GetRepresentation(FORMAT_MFVideoFormat, (void**)&pAMMedia));

    VideoFormat = (MFVIDEOFORMAT*)pAMMedia->pbFormat;
    hr = pfMFCreateVideoMediaType(VideoFormat, &m_pMediaType);

    m_AspectRatio.cx = VideoFormat->videoInfo.PixelAspectRatio.Numerator;
    m_AspectRatio.cy = VideoFormat->videoInfo.PixelAspectRatio.Denominator;

    if (SUCCEEDED (hr))
    {
        i64Size.HighPart = VideoFormat->videoInfo.dwWidth;
        i64Size.LowPart	 = VideoFormat->videoInfo.dwHeight;
        m_pMediaType->SetUINT64(MF_MT_FRAME_SIZE, i64Size.QuadPart);
        m_pMediaType->SetUINT32(MF_MT_PAN_SCAN_ENABLED, 0);
        CRenderersSettings& s = GetRenderersSettings();

        if (s.m_RenderSettings.iEVROutputRange == 1)
            m_pMediaType->SetUINT32(MF_MT_VIDEO_NOMINAL_RANGE, MFNominalRange_16_235);
        else
            m_pMediaType->SetUINT32(MF_MT_VIDEO_NOMINAL_RANGE, MFNominalRange_0_255);

        m_LastSetOutputRange = s.m_RenderSettings.iEVROutputRange;
        i64Size.HighPart = m_AspectRatio.cx;
        i64Size.LowPart  = m_AspectRatio.cy;
        m_pMediaType->SetUINT64(MF_MT_PIXEL_ASPECT_RATIO, i64Size.QuadPart);

        MFVideoArea Area = GetArea(0, 0, VideoFormat->videoInfo.dwWidth, VideoFormat->videoInfo.dwHeight);
        m_pMediaType->SetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*)&Area, sizeof(MFVideoArea));
    }

    m_AspectRatio.cx *= VideoFormat->videoInfo.dwWidth;
    m_AspectRatio.cy *= VideoFormat->videoInfo.dwHeight;

    bool bDoneSomething = true;

    if(m_AspectRatio.cx >= 1 && m_AspectRatio.cy >= 1)
    {
        while (bDoneSomething)
        {
            bDoneSomething = false;
            INT MinNum = min(m_AspectRatio.cx, m_AspectRatio.cy);
            INT i;
            for (i = 2; i < MinNum+1; ++i)
            {
                if (m_AspectRatio.cx%i == 0 && m_AspectRatio.cy%i ==0)
                    break;
            }
            if (i != MinNum + 1)
            {
                m_AspectRatio.cx = m_AspectRatio.cx / i;
                m_AspectRatio.cy = m_AspectRatio.cy / i;
                bDoneSomething = true;
            }
        }
    }

    pMixerType->FreeRepresentation(FORMAT_MFVideoFormat, (void*)pAMMedia);
    m_pMediaType->QueryInterface(__uuidof(IMFMediaType), (void**) pType);

    return hr;
}

HRESULT CSyncAP::SetMediaType(IMFMediaType* pType)
{
    HRESULT hr;
    AM_MEDIA_TYPE* pAMMedia = NULL;
    CString strTemp;

    CheckPointer(pType, E_POINTER);
    CheckHR(pType->GetRepresentation(FORMAT_VideoInfo2, (void**)&pAMMedia));

    hr = InitializeDevice(pAMMedia);
    if (SUCCEEDED(hr))
    {
        strTemp = GetMediaTypeName(pAMMedia->subtype);
        strTemp.Replace(L"MEDIASUBTYPE_", L"");
        m_strStatsMsg[MSG_MIXEROUT].Format (L"Mixer output: %s", strTemp);
    }

    pType->FreeRepresentation(FORMAT_VideoInfo2, (void*)pAMMedia);

    return hr;
}

typedef struct
{
    const int Format;
    const LPCTSTR Description;
} D3DFORMAT_TYPE;

LONGLONG CSyncAP::GetMediaTypeMerit(IMFMediaType *pMediaType)
{
    AM_MEDIA_TYPE *pAMMedia = NULL;
    MFVIDEOFORMAT *VideoFormat;

    HRESULT hr;
    CheckHR(pMediaType->GetRepresentation  (FORMAT_MFVideoFormat, (void**)&pAMMedia));
    VideoFormat = (MFVIDEOFORMAT*)pAMMedia->pbFormat;

    LONGLONG Merit = 0;
    switch (VideoFormat->surfaceInfo.Format)
    {
    case FCC('NV12'):
        Merit = 90000000;
        break;
    case FCC('YV12'):
        Merit = 80000000;
        break;
    case FCC('YUY2'):
        Merit = 70000000;
        break;
    case FCC('UYVY'):
        Merit = 60000000;
        break;

    case D3DFMT_X8R8G8B8: // Never opt for RGB
    case D3DFMT_A8R8G8B8:
    case D3DFMT_R8G8B8:
    case D3DFMT_R5G6B5:
        Merit = 0;
        break;
    default:
        Merit = 1000;
        break;
    }
    pMediaType->FreeRepresentation(FORMAT_MFVideoFormat, (void*)pAMMedia);
    return Merit;
}

HRESULT CSyncAP::RenegotiateMediaType()
{
    HRESULT hr = S_OK;

    CComPtr<IMFMediaType> pMixerType;
    CComPtr<IMFMediaType> pType;

    if (!m_pMixer) return MF_E_INVALIDREQUEST;

    CInterfaceArray<IMFMediaType> ValidMixerTypes;
    // Loop through all of the mixer's proposed output types.
    DWORD iTypeIndex = 0;
    while ((hr != MF_E_NO_MORE_TYPES))
    {
        pMixerType  = NULL;
        pType = NULL;
        m_pMediaType = NULL;

        // Step 1. Get the next media type supported by mixer.
        hr = m_pMixer->GetOutputAvailableType(0, iTypeIndex++, &pMixerType);
        if (FAILED(hr))
        {
            break;
        }
        // Step 2. Check if we support this media type.
        if (SUCCEEDED(hr))
            hr = IsMediaTypeSupported(pMixerType);
        if (SUCCEEDED(hr))
            hr = CreateProposedOutputType(pMixerType, &pType);
        // Step 4. Check if the mixer will accept this media type.
        if (SUCCEEDED(hr))
            hr = m_pMixer->SetOutputType(0, pType, MFT_SET_TYPE_TEST_ONLY);
        if (SUCCEEDED(hr))
        {
            LONGLONG Merit = GetMediaTypeMerit(pType);

            int nTypes = ValidMixerTypes.GetCount();
            int iInsertPos = 0;
            for (int i = 0; i < nTypes; ++i)
            {
                LONGLONG ThisMerit = GetMediaTypeMerit(ValidMixerTypes[i]);
                if (Merit > ThisMerit)
                {
                    iInsertPos = i;
                    break;
                }
                else
                    iInsertPos = i+1;
            }
            ValidMixerTypes.InsertAt(iInsertPos, pType);
        }
    }

    int nValidTypes = ValidMixerTypes.GetCount();
    for (int i = 0; i < nValidTypes; ++i)
    {
        pType = ValidMixerTypes[i];
    }

    for (int i = 0; i < nValidTypes; ++i)
    {
        pType = ValidMixerTypes[i];
        hr = SetMediaType(pType);
        if (SUCCEEDED(hr))
        {
            hr = m_pMixer->SetOutputType(0, pType, 0);
            // If something went wrong, clear the media type.
            if (FAILED(hr))
            {
                SetMediaType(NULL);
            }
            else
                break;
        }
    }

    pMixerType = NULL;
    pType = NULL;
    return hr;
}

bool CSyncAP::GetSampleFromMixer()
{
    MFT_OUTPUT_DATA_BUFFER Buffer;
    HRESULT hr = S_OK;
    DWORD dwStatus;
    LONGLONG llClockBefore = 0;
    LONGLONG llClockAfter  = 0;
    LONGLONG llMixerLatency;

    UINT dwSurface;
    bool newSample = false;

    while(SUCCEEDED(hr)) // Get as many frames as there are and that we have samples for
    {
        CComPtr<IMFSample> pSample;
        CComPtr<IMFSample> pNewSample;
        if (FAILED(GetFreeSample(&pSample))) // All samples are taken for the moment. Better luck next time
        {
            break;
        }

        memset(&Buffer, 0, sizeof(Buffer));
        Buffer.pSample = pSample;
        pSample->GetUINT32(GUID_SURFACE_INDEX, &dwSurface);
        {
            llClockBefore = GetRenderersData()->GetPerfCounter();
            hr = m_pMixer->ProcessOutput(0 , 1, &Buffer, &dwStatus);
            llClockAfter = GetRenderersData()->GetPerfCounter();
        }

        if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT) // There are no samples left in the mixer
        {
            MoveToFreeList(pSample, false);
            break;
        }
        if (m_pSink)
        {
            llMixerLatency = llClockAfter - llClockBefore;
            m_pSink->Notify (EC_PROCESSING_LATENCY, (LONG_PTR)&llMixerLatency, 0);
        }

        newSample = true;

        if (GetRenderersData()->m_fTearingTest)
        {
            RECT rcTearing;

            rcTearing.left = m_nTearingPos;
            rcTearing.top = 0;
            rcTearing.right	= rcTearing.left + 4;
            rcTearing.bottom = m_NativeVideoSize.cy;
            m_pD3DDev->ColorFill(m_pVideoSurface[dwSurface], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

            rcTearing.left = (rcTearing.right + 15) % m_NativeVideoSize.cx;
            rcTearing.right	= rcTearing.left + 4;
            m_pD3DDev->ColorFill(m_pVideoSurface[dwSurface], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));
            m_nTearingPos = (m_nTearingPos + 7) % m_NativeVideoSize.cx;
        }
        MoveToScheduledList(pSample, false); // Schedule, then go back to see if there is more where that came from
    }
    return newSample;
}

STDMETHODIMP CSyncAP::GetCurrentMediaType(__deref_out  IMFVideoMediaType **ppMediaType)
{
    HRESULT hr = S_OK;
    CAutoLock lock(this);
    CheckPointer(ppMediaType, E_POINTER);
    CheckHR(CheckShutdown());

    if (m_pMediaType == NULL)
        CheckHR(MF_E_NOT_INITIALIZED);

    CheckHR(m_pMediaType->QueryInterface(__uuidof(IMFVideoMediaType), (void**)&ppMediaType));
    return hr;
}

// IMFTopologyServiceLookupClient
STDMETHODIMP CSyncAP::InitServicePointers(__in IMFTopologyServiceLookup *pLookup)
{
    HRESULT hr;
    DWORD dwObjects = 1;
    hr = pLookup->LookupService(MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_MIXER_SERVICE, __uuidof (IMFTransform), (void**)&m_pMixer, &dwObjects);
    hr = pLookup->LookupService(MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_RENDER_SERVICE, __uuidof (IMediaEventSink ), (void**)&m_pSink, &dwObjects);
    hr = pLookup->LookupService(MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_RENDER_SERVICE, __uuidof (IMFClock ), (void**)&m_pClock, &dwObjects);
    StartWorkerThreads();
    return S_OK;
}

STDMETHODIMP CSyncAP::ReleaseServicePointers()
{
    StopWorkerThreads();
    m_pMixer = NULL;
    m_pSink = NULL;
    m_pClock = NULL;
    return S_OK;
}

// IMFVideoDeviceID
STDMETHODIMP CSyncAP::GetDeviceID( __out  IID *pDeviceID)
{
    CheckPointer(pDeviceID, E_POINTER);
    *pDeviceID = IID_IDirect3DDevice9;
    return S_OK;
}

// IMFGetService
STDMETHODIMP CSyncAP::GetService( __RPC__in REFGUID guidService, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID *ppvObject)
{
    if (guidService == MR_VIDEO_RENDER_SERVICE)
        return NonDelegatingQueryInterface (riid, ppvObject);
    else if (guidService == MR_VIDEO_ACCELERATION_SERVICE)
        return m_pD3DManager->QueryInterface (__uuidof(IDirect3DDeviceManager9), (void**) ppvObject);

    return E_NOINTERFACE;
}


// IMFAsyncCallback
STDMETHODIMP CSyncAP::GetParameters( __RPC__out DWORD *pdwFlags, __RPC__out DWORD *pdwQueue)
{
    return E_NOTIMPL;
}

STDMETHODIMP CSyncAP::Invoke( __RPC__in_opt IMFAsyncResult *pAsyncResult)
{
    return E_NOTIMPL;
}

// IMFVideoDisplayControl
STDMETHODIMP CSyncAP::GetNativeVideoSize(SIZE *pszVideo, SIZE *pszARVideo)
{
    if (pszVideo)
    {
        pszVideo->cx	= m_NativeVideoSize.cx;
        pszVideo->cy	= m_NativeVideoSize.cy;
    }
    if (pszARVideo)
    {
        pszARVideo->cx	= m_NativeVideoSize.cx * m_AspectRatio.cx;
        pszARVideo->cy	= m_NativeVideoSize.cy * m_AspectRatio.cy;
    }
    return S_OK;
}

STDMETHODIMP CSyncAP::GetIdealVideoSize(SIZE *pszMin, SIZE *pszMax)
{
    if (pszMin)
    {
        pszMin->cx	= 1;
        pszMin->cy	= 1;
    }

    if (pszMax)
    {
        D3DDISPLAYMODE	d3ddm;

        ZeroMemory(&d3ddm, sizeof(d3ddm));
        if(SUCCEEDED(m_pD3D->GetAdapterDisplayMode(GetAdapter(m_pD3D), &d3ddm)))
        {
            pszMax->cx	= d3ddm.Width;
            pszMax->cy	= d3ddm.Height;
        }
    }
    return S_OK;
}
STDMETHODIMP CSyncAP::SetVideoPosition(const MFVideoNormalizedRect *pnrcSource, const LPRECT prcDest)
{
    return S_OK;
}
STDMETHODIMP CSyncAP::GetVideoPosition(MFVideoNormalizedRect *pnrcSource, LPRECT prcDest)
{
    if (pnrcSource)
    {
        pnrcSource->left	= 0.0;
        pnrcSource->top		= 0.0;
        pnrcSource->right	= 1.0;
        pnrcSource->bottom	= 1.0;
    }
    if (prcDest)
        memcpy (prcDest, &m_VideoRect, sizeof(m_VideoRect));//GetClientRect (m_hWnd, prcDest);
    return S_OK;
}
STDMETHODIMP CSyncAP::SetAspectRatioMode(DWORD dwAspectRatioMode)
{
    m_dwVideoAspectRatioMode = (MFVideoAspectRatioMode)dwAspectRatioMode;
    return S_OK;
}
STDMETHODIMP CSyncAP::GetAspectRatioMode(DWORD *pdwAspectRatioMode)
{
    CheckPointer (pdwAspectRatioMode, E_POINTER);
    *pdwAspectRatioMode = m_dwVideoAspectRatioMode;
    return S_OK;
}
STDMETHODIMP CSyncAP::SetVideoWindow(HWND hwndVideo)
{
    ASSERT (m_hWnd == hwndVideo);
    return S_OK;
}
STDMETHODIMP CSyncAP::GetVideoWindow(HWND *phwndVideo)
{
    CheckPointer(phwndVideo, E_POINTER);
    *phwndVideo = m_hWnd;
    return S_OK;
}
STDMETHODIMP CSyncAP::RepaintVideo()
{
    Paint(true);
    return S_OK;
}
STDMETHODIMP CSyncAP::GetCurrentImage(BITMAPINFOHEADER *pBih, BYTE **pDib, DWORD *pcbDib, LONGLONG *pTimeStamp)
{
    ASSERT (FALSE);
    return E_NOTIMPL;
}
STDMETHODIMP CSyncAP::SetBorderColor(COLORREF Clr)
{
    m_BorderColor = Clr;
    return S_OK;
}
STDMETHODIMP CSyncAP::GetBorderColor(COLORREF *pClr)
{
    CheckPointer (pClr, E_POINTER);
    *pClr = m_BorderColor;
    return S_OK;
}
STDMETHODIMP CSyncAP::SetRenderingPrefs(DWORD dwRenderFlags)
{
    m_dwVideoRenderPrefs = (MFVideoRenderPrefs)dwRenderFlags;
    return S_OK;
}
STDMETHODIMP CSyncAP::GetRenderingPrefs(DWORD *pdwRenderFlags)
{
    CheckPointer(pdwRenderFlags, E_POINTER);
    *pdwRenderFlags = m_dwVideoRenderPrefs;
    return S_OK;
}
STDMETHODIMP CSyncAP::SetFullscreen(BOOL fFullscreen)
{
    ASSERT (FALSE);
    return E_NOTIMPL;
}
STDMETHODIMP CSyncAP::GetFullscreen(BOOL *pfFullscreen)
{
    ASSERT (FALSE);
    return E_NOTIMPL;
}

// IEVRTrustedVideoPlugin
STDMETHODIMP CSyncAP::IsInTrustedVideoMode(BOOL *pYes)
{
    CheckPointer(pYes, E_POINTER);
    *pYes = TRUE;
    return S_OK;
}

STDMETHODIMP CSyncAP::CanConstrict(BOOL *pYes)
{
    CheckPointer(pYes, E_POINTER);
    *pYes = TRUE;
    return S_OK;
}

STDMETHODIMP CSyncAP::SetConstriction(DWORD dwKPix)
{
    return S_OK;
}

STDMETHODIMP CSyncAP::DisableImageExport(BOOL bDisable)
{
    return S_OK;
}

// IDirect3DDeviceManager9
STDMETHODIMP CSyncAP::ResetDevice(IDirect3DDevice9 *pDevice,UINT resetToken)
{
    HRESULT		hr = m_pD3DManager->ResetDevice (pDevice, resetToken);
    return hr;
}

STDMETHODIMP CSyncAP::OpenDeviceHandle(HANDLE *phDevice)
{
    HRESULT		hr = m_pD3DManager->OpenDeviceHandle (phDevice);
    return hr;
}

STDMETHODIMP CSyncAP::CloseDeviceHandle(HANDLE hDevice)
{
    HRESULT		hr = m_pD3DManager->CloseDeviceHandle(hDevice);
    return hr;
}

STDMETHODIMP CSyncAP::TestDevice(HANDLE hDevice)
{
    HRESULT		hr = m_pD3DManager->TestDevice(hDevice);
    return hr;
}

STDMETHODIMP CSyncAP::LockDevice(HANDLE hDevice, IDirect3DDevice9 **ppDevice, BOOL fBlock)
{
    HRESULT		hr = m_pD3DManager->LockDevice(hDevice, ppDevice, fBlock);
    return hr;
}

STDMETHODIMP CSyncAP::UnlockDevice(HANDLE hDevice, BOOL fSaveState)
{
    HRESULT		hr = m_pD3DManager->UnlockDevice(hDevice, fSaveState);
    return hr;
}

STDMETHODIMP CSyncAP::GetVideoService(HANDLE hDevice, REFIID riid, void **ppService)
{
    HRESULT		hr = m_pD3DManager->GetVideoService(hDevice, riid, ppService);

    if (riid == __uuidof(IDirectXVideoDecoderService))
    {
        UINT		nNbDecoder = 5;
        GUID*		pDecoderGuid;
        IDirectXVideoDecoderService*		pDXVAVideoDecoder = (IDirectXVideoDecoderService*) *ppService;
        pDXVAVideoDecoder->GetDecoderDeviceGuids (&nNbDecoder, &pDecoderGuid);
    }
    else if (riid == __uuidof(IDirectXVideoProcessorService))
    {
        IDirectXVideoProcessorService*		pDXVAProcessor = (IDirectXVideoProcessorService*) *ppService;
    }

    return hr;
}

STDMETHODIMP CSyncAP::GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
{
    // This function should be called...
    ASSERT (FALSE);

    if(lpWidth)		*lpWidth	= m_NativeVideoSize.cx;
    if(lpHeight)	*lpHeight	= m_NativeVideoSize.cy;
    if(lpARWidth)	*lpARWidth	= m_AspectRatio.cx;
    if(lpARHeight)	*lpARHeight	= m_AspectRatio.cy;
    return S_OK;
}

STDMETHODIMP CSyncAP::InitializeDevice(AM_MEDIA_TYPE* pMediaType)
{
    HRESULT hr;
    CAutoLock lock(this);
    CAutoLock lock2(&m_ImageProcessingLock);
    CAutoLock cRenderLock(&m_allocatorLock);

    RemoveAllSamples();
    DeleteSurfaces();

    VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*) pMediaType->pbFormat;
    int w = vih2->bmiHeader.biWidth;
    int h = abs(vih2->bmiHeader.biHeight);

    m_NativeVideoSize = CSize(w, h);
    if (m_bHighColorResolution)
        hr = AllocSurfaces(D3DFMT_A2R10G10B10);
    else
        hr = AllocSurfaces(D3DFMT_X8R8G8B8);

    for(int i = 0; i < m_nDXSurface; i++)
    {
        CComPtr<IMFSample> pMFSample;
        hr = pfMFCreateVideoSampleFromSurface(m_pVideoSurface[i], &pMFSample);
        if (SUCCEEDED (hr))
        {
            pMFSample->SetUINT32(GUID_SURFACE_INDEX, i);
            m_FreeSamples.AddTail (pMFSample);
        }
        ASSERT (SUCCEEDED (hr));
    }
    return hr;
}

DWORD WINAPI CSyncAP::MixerThreadStatic(LPVOID lpParam)
{
    CSyncAP *pThis = (CSyncAP*) lpParam;
    pThis->MixerThread();
    return 0;
}

void CSyncAP::MixerThread()
{
    HANDLE hAvrt;
    HANDLE hEvts[] = {m_hEvtQuit};
    bool bQuit = false;
    TIMECAPS tc;
    DWORD dwResolution;
    DWORD dwUser = 0;
    DWORD dwTaskIndex = 0;

    timeGetDevCaps(&tc, sizeof(TIMECAPS));
    dwResolution = min(max(tc.wPeriodMin, 0), tc.wPeriodMax);
    dwUser = timeBeginPeriod(dwResolution);

    while (!bQuit)
    {
        DWORD dwObject = WaitForMultipleObjects (countof(hEvts), hEvts, FALSE, 1);
        switch (dwObject)
        {
        case WAIT_OBJECT_0 :
            bQuit = true;
            break;
        case WAIT_TIMEOUT :
        {
            bool bNewSample = false;
            {
                CAutoLock AutoLock(&m_ImageProcessingLock);
                bNewSample = GetSampleFromMixer();
            }
            if(m_bUseInternalTimer && m_pSubPicQueue)
            {
                m_pSubPicQueue->SetFPS(m_fps);
            }
        }
        break;
        }
    }
    timeEndPeriod (dwResolution);
}

DWORD WINAPI CSyncAP::RenderThreadStatic(LPVOID lpParam)
{
    CSyncAP *pThis = (CSyncAP*)lpParam;
    pThis->RenderThread();
    return 0;
}

// Get samples that have been received and queued up by MixerThread() and present them at the correct time by calling Paint().
void CSyncAP::RenderThread()
{
    HANDLE hAvrt;
    DWORD dwTaskIndex = 0;
    HANDLE hEvts[] = {m_hEvtQuit, m_hEvtFlush, m_hEvtSkip};
    bool bQuit = false;
    TIMECAPS tc;
    DWORD dwResolution;
    LONGLONG llRefClockTime;
    double dTargetSyncOffset;
    MFTIME llSystemTime;
    DWORD dwUser = 0;
    DWORD dwObject;
    int nSamplesLeft;
    CComPtr<IMFSample>pNewSample = NULL; // The sample next in line to be presented

    // Tell Vista Multimedia Class Scheduler we are doing threaded playback (increase priority)
    if (pfAvSetMmThreadCharacteristicsW) hAvrt = pfAvSetMmThreadCharacteristicsW (L"Playback", &dwTaskIndex);
    if (pfAvSetMmThreadPriority) pfAvSetMmThreadPriority (hAvrt, AVRT_PRIORITY_HIGH);

    CRenderersSettings& s = GetRenderersSettings();

    // Set timer resolution
    timeGetDevCaps(&tc, sizeof(TIMECAPS));
    dwResolution = min(max(tc.wPeriodMin, 0), tc.wPeriodMax);
    dwUser = timeBeginPeriod(dwResolution);
    pNewSample = NULL;

    while (!bQuit)
    {
        m_lNextSampleWait = 1; // Default value for running this loop
        nSamplesLeft = 0;
        bool stepForward = false;
        LONG lDisplayCycle = (LONG)(GetDisplayCycle());
        LONG lDisplayCycle2 = (LONG)(GetDisplayCycle() / 2.0); // These are a couple of empirically determined constants used the control the "snap" function
        LONG lDisplayCycle4 = (LONG)(GetDisplayCycle() / 4.0);

        CRenderersSettings& s = GetRenderersSettings();
        dTargetSyncOffset = s.m_RenderSettings.fTargetSyncOffset;

        if ((m_nRenderState == Started || !m_bPrerolled) && !pNewSample) // If either streaming or the pre-roll sample and no sample yet fetched
        {
            if (SUCCEEDED(GetScheduledSample(&pNewSample, nSamplesLeft))) // Get the next sample
            {
                m_llLastSampleTime = m_llSampleTime;
                if (!m_bPrerolled)
                {
                    m_bPrerolled = true; // m_bPrerolled is a ticket to show one (1) frame immediately
                    m_lNextSampleWait = 0; // Present immediately
                }
                else if (SUCCEEDED(pNewSample->GetSampleTime(&m_llSampleTime))) // Get zero-based sample due time
                {
                    if (m_llLastSampleTime == m_llSampleTime) // In the rare case there are duplicate frames in the movie. There really shouldn't be but it happens.
                    {
                        MoveToFreeList(pNewSample, true);
                        pNewSample = NULL;
                        m_lNextSampleWait = 0;
                    }
                    else
                    {
                        m_pClock->GetCorrelatedTime(0, &llRefClockTime, &llSystemTime); // Get zero-based reference clock time. llSystemTime is not used for anything here
                        m_lNextSampleWait = (LONG)((m_llSampleTime - llRefClockTime) / 10000); // Time left until sample is due, in ms
                        if (m_bStepping)
                        {
                            m_lNextSampleWait = 0;
                        }
                        else if (s.m_RenderSettings.bSynchronizeNearest) // Present at the closest "safe" occasion at dTargetSyncOffset ms before vsync to avoid tearing
                        {
                            if (m_lNextSampleWait < -lDisplayCycle) // We have to allow slightly negative numbers at this stage. Otherwise we get "choking" when frame rate > refresh rate
                            {
                                SetEvent(m_hEvtSkip);
                                m_bEvtSkip = true;
                            }
                            REFERENCE_TIME rtRefClockTimeNow;
                            if (m_pRefClock) m_pRefClock->GetTime(&rtRefClockTimeNow); // Reference clock time now
                            LONG lLastVsyncTime = (LONG)((m_llEstVBlankTime - rtRefClockTimeNow) / 10000); // Last vsync time relative to now
                            if (abs(lLastVsyncTime) > lDisplayCycle) lLastVsyncTime = - lDisplayCycle; // To even out glitches in the beginning

                            LONGLONG llNextSampleWait = (LONGLONG)(((double)lLastVsyncTime + GetDisplayCycle() - dTargetSyncOffset) * 10000); // Time from now util next safe time to Paint()
                            while ((llRefClockTime + llNextSampleWait) < (m_llSampleTime + m_llHysteresis)) // While the proposed time is in the past of sample presentation time
                            {
                                llNextSampleWait = llNextSampleWait + (LONGLONG)(GetDisplayCycle() * 10000); // Try the next possible time, one display cycle ahead
                            }
                            m_lNextSampleWait = (LONG)(llNextSampleWait / 10000);
                            m_lShiftToNearestPrev = m_lShiftToNearest;
                            m_lShiftToNearest = (LONG)((llRefClockTime + llNextSampleWait - m_llSampleTime) / 10000); // The adjustment made to get to the sweet point in time, in ms

                            // If m_lShiftToNearest is pushed a whole cycle into the future, then we are getting more frames
                            // than we can chew and we need to throw one away. We don't want to wait many cycles and skip many
                            // frames.
                            if (m_lShiftToNearest > (lDisplayCycle + 1))
                            {
                                SetEvent(m_hEvtSkip);
                                m_bEvtSkip = true;
                            }

                            // We need to add a hysteresis to the control of the timing adjustment to avoid judder when
                            // presentation time is close to vsync and the renderer couldn't otherwise make up its mind
                            // whether to present before the vsync or after. That kind of indecisiveness leads to judder.
                            if (m_bSnapToVSync)
                            {

                                if ((m_lShiftToNearestPrev - m_lShiftToNearest) > lDisplayCycle2) // If a step down in the m_lShiftToNearest function. Display slower than video.
                                {
                                    m_bVideoSlowerThanDisplay = false;
                                    m_llHysteresis = -(LONGLONG)(10000 * lDisplayCycle4);
                                }
                                else if ((m_lShiftToNearest - m_lShiftToNearestPrev) > lDisplayCycle2) // If a step up
                                {
                                    m_bVideoSlowerThanDisplay = true;
                                    m_llHysteresis = (LONGLONG)(10000 * lDisplayCycle4);
                                }
                                else if ((m_lShiftToNearest < (3 * lDisplayCycle4)) && (m_lShiftToNearest > lDisplayCycle4))
                                    m_llHysteresis = 0; // Reset when between 1/4 and 3/4 of the way either way

                                if ((m_lShiftToNearest < lDisplayCycle2) && (m_llHysteresis > 0)) m_llHysteresis = 0; // Should never really be in this territory.
                                if (m_lShiftToNearest < 0) m_llHysteresis = 0; // A glitch might get us to a sticky state where both these numbers are negative.
                                if ((m_lShiftToNearest > lDisplayCycle2) && (m_llHysteresis < 0)) m_llHysteresis = 0;
                            }
                        }

                        if (m_lNextSampleWait < 0) // Skip late or duplicate sample.
                        {
                            SetEvent(m_hEvtSkip);
                            m_bEvtSkip = true;
                        }

                        if (m_lNextSampleWait > 1000)
                        {
                            m_lNextSampleWait = 1000; // So as to avoid full a full stop when sample is far in the future (shouldn't really happen).
                        }
                    }
                } // if got new sample
            }
        }
        // Wait for the next presentation time (m_lNextSampleWait) or some other event.
        dwObject = WaitForMultipleObjects(countof(hEvts), hEvts, FALSE, (DWORD)m_lNextSampleWait);
        switch (dwObject)
        {
        case WAIT_OBJECT_0: // Quit
            bQuit = true;
            break;

        case WAIT_OBJECT_0 + 1: // Flush
            if (pNewSample) MoveToFreeList(pNewSample, true);
            pNewSample = NULL;
            FlushSamples();
            m_bEvtFlush = false;
            ResetEvent(m_hEvtFlush);
            m_bPrerolled = false;
            m_lShiftToNearest = 0;
            stepForward = true;
            break;

        case WAIT_OBJECT_0 + 2: // Skip sample
            m_pcFramesDropped++;
            m_llSampleTime = m_llLastSampleTime; // This sample will never be shown
            m_bEvtSkip = false;
            ResetEvent(m_hEvtSkip);
            stepForward = true;
            break;

        case WAIT_TIMEOUT: // Time to show the sample or something
            if (m_LastSetOutputRange != -1 && m_LastSetOutputRange != s.m_RenderSettings.iEVROutputRange || m_bPendingRenegotiate)
            {
                if (pNewSample) MoveToFreeList(pNewSample, true);
                pNewSample = NULL;
                FlushSamples();
                RenegotiateMediaType();
                m_bPendingRenegotiate = false;
            }

            if (m_bPendingResetDevice)
			{
				if (pNewSample)
					MoveToFreeList(pNewSample, true);
				pNewSample = NULL;
				SendResetRequest();
			}
            else if (m_nStepCount < 0)
            {
                m_nStepCount = 0;
                m_pcFramesDropped++;
                stepForward = true;
            }
            else if (pNewSample && (m_nStepCount > 0))
            {
                pNewSample->GetUINT32(GUID_SURFACE_INDEX, (UINT32 *)&m_nCurSurface);
                if (!g_bExternalSubtitleTime) __super::SetTime (g_tSegmentStart + m_llSampleTime);
                Paint(true);
                CompleteFrameStep(false);
                m_pcFramesDrawn++;
                stepForward = true;
            }
            else if (pNewSample && !m_bStepping) // When a stepped frame is shown, a new one is fetched that we don't want to show here while stepping
            {
                pNewSample->GetUINT32(GUID_SURFACE_INDEX, (UINT32*)&m_nCurSurface);
                if (!g_bExternalSubtitleTime) __super::SetTime (g_tSegmentStart + m_llSampleTime);
                Paint(true);
                m_pcFramesDrawn++;
                stepForward = true;
            }
            break;
        } // switch
        if (pNewSample && stepForward)
        {
            MoveToFreeList(pNewSample, true);
            pNewSample = NULL;
        }
    } // while
    if (pNewSample)
    {
        MoveToFreeList(pNewSample, true);
        pNewSample = NULL;
    }
    timeEndPeriod (dwResolution);
    if (pfAvRevertMmThreadCharacteristics) pfAvRevertMmThreadCharacteristics(hAvrt);
}

STDMETHODIMP_(bool) CSyncAP::ResetDevice()
{
	CAutoLock lock(this);
	CAutoLock lock2(&m_ImageProcessingLock);
	CAutoLock cRenderLock(&m_allocatorLock);
	RemoveAllSamples();
	
	bool bResult = __super::ResetDevice();

	for(int i = 0; i < m_nDXSurface; i++)
	{
		CComPtr<IMFSample> pMFSample;
		HRESULT hr = pfMFCreateVideoSampleFromSurface (m_pVideoSurface[i], &pMFSample);
		if (SUCCEEDED (hr))
		{
			pMFSample->SetUINT32(GUID_SURFACE_INDEX, i);
			m_FreeSamples.AddTail(pMFSample);
		}
		ASSERT(SUCCEEDED (hr));
	}
	return bResult;
}

void CSyncAP::OnResetDevice()
{
    TRACE("--> CSyncAP::OnResetDevice on thread: %d\n", GetCurrentThreadId());
    HRESULT hr;
    hr = m_pD3DManager->ResetDevice(m_pD3DDev, m_nResetToken);
    if (m_pSink) m_pSink->Notify(EC_DISPLAY_CHANGED, 0, 0);
	CSize videoSize = GetVisibleVideoSize();
    if (m_pSink) m_pSink->Notify(EC_VIDEO_SIZE_CHANGED, MAKELPARAM(videoSize.cx, videoSize.cy), 0);
}

void CSyncAP::RemoveAllSamples()
{
    CAutoLock AutoLock(&m_ImageProcessingLock);
    FlushSamples();
    m_ScheduledSamples.RemoveAll();
    m_FreeSamples.RemoveAll();
    m_nUsedBuffer = 0;
}

HRESULT CSyncAP::GetFreeSample(IMFSample** ppSample)
{
    CAutoLock lock(&m_SampleQueueLock);
    HRESULT		hr = S_OK;

    if (m_FreeSamples.GetCount() > 1)	// Cannot use first free buffer (can be currently displayed)
    {
        InterlockedIncrement(&m_nUsedBuffer);
        *ppSample = m_FreeSamples.RemoveHead().Detach();
    }
    else
        hr = MF_E_SAMPLEALLOCATOR_EMPTY;

    return hr;
}

HRESULT CSyncAP::GetScheduledSample(IMFSample** ppSample, int &_Count)
{
    CAutoLock lock(&m_SampleQueueLock);
    HRESULT		hr = S_OK;

    _Count = m_ScheduledSamples.GetCount();
    if (_Count > 0)
    {
        *ppSample = m_ScheduledSamples.RemoveHead().Detach();
        --_Count;
    }
    else
        hr = MF_E_SAMPLEALLOCATOR_EMPTY;

    return hr;
}

void CSyncAP::MoveToFreeList(IMFSample* pSample, bool bTail)
{
    CAutoLock lock(&m_SampleQueueLock);
    InterlockedDecrement(&m_nUsedBuffer);
    if (m_bPendingMediaFinished && m_nUsedBuffer == 0)
    {
        m_bPendingMediaFinished = false;
        m_pSink->Notify(EC_COMPLETE, 0, 0);
    }
    if (bTail)
        m_FreeSamples.AddTail(pSample);
    else
        m_FreeSamples.AddHead(pSample);
}

void CSyncAP::MoveToScheduledList(IMFSample* pSample, bool _bSorted)
{
    if (_bSorted)
    {
        CAutoLock lock(&m_SampleQueueLock);
        m_ScheduledSamples.AddHead(pSample);
    }
    else
    {
        CAutoLock lock(&m_SampleQueueLock);
        m_ScheduledSamples.AddTail(pSample);
    }
}

void CSyncAP::FlushSamples()
{
    CAutoLock lock(this);
    CAutoLock lock2(&m_SampleQueueLock);
    FlushSamplesInternal();
}

void CSyncAP::FlushSamplesInternal()
{
    m_bPrerolled = false;
    while (m_ScheduledSamples.GetCount() > 0)
    {
        CComPtr<IMFSample> pMFSample;
        pMFSample = m_ScheduledSamples.RemoveHead();
        MoveToFreeList(pMFSample, true);
    }
}

HRESULT CSyncAP::AdviseSyncClock(ISyncClock* sC)
{
	return m_pGenlock->AdviseSyncClock(sC);
}

HRESULT CSyncAP::BeginStreaming()
{
    CRenderersSettings& s = GetRenderersSettings();
    m_pcFramesDropped = 0;
    m_pcFramesDrawn = 0;

    CComPtr<IBaseFilter> pEVR;
    FILTER_INFO filterInfo;
    ZeroMemory(&filterInfo, sizeof(filterInfo));
    m_pOuterEVR->QueryInterface (__uuidof(IBaseFilter), (void**)&pEVR);
    pEVR->QueryFilterInfo(&filterInfo); // This addref's the pGraph member

    BeginEnumFilters(filterInfo.pGraph, pEF, pBF)
    if(CComQIPtr<IAMAudioRendererStats> pAS = pBF)
    {
        m_pAudioStats = pAS;
    };
    EndEnumFilters

    pEVR->GetSyncSource(&m_pRefClock);
    if (filterInfo.pGraph) filterInfo.pGraph->Release();
    m_pGenlock->SetMonitor(GetAdapter(m_pD3D));
    m_pGenlock->GetTiming();

    ResetStats();
    EstimateRefreshTimings();
    if (m_dFrameCycle > 0.0) m_dCycleDifference = GetCycleDifference(); // Might have moved to another display
    return S_OK;
}

HRESULT CreateSyncRenderer(const CLSID& clsid, HWND hWnd, bool bFullscreen, ISubPicAllocatorPresenter** ppAP)
{
    HRESULT		hr = E_FAIL;
    if (clsid == CLSID_SyncAllocatorPresenter)
    {
		CString Error;
        *ppAP	= DNew CSyncAP(hWnd, bFullscreen, hr, Error);
        (*ppAP)->AddRef();

        if(FAILED(hr))
        {
            Error += L"\n";
            Error += GothSyncErrorMessage(hr, NULL);
            MessageBox(hWnd, Error, L"Error creating Sync Renderer", MB_OK | MB_ICONERROR);
            (*ppAP)->Release();
            *ppAP = NULL;
        }
        else if (!Error.IsEmpty())
        {
            MessageBox(hWnd, Error, L"Warning creating Sync Renderer", MB_OK|MB_ICONWARNING);
        }
    }
    return hr;
}

CSyncRenderer::CSyncRenderer(const TCHAR* pName, LPUNKNOWN pUnk, HRESULT& hr, VMR9AlphaBitmap* pVMR9AlphaBitmap, CSyncAP *pAllocatorPresenter): CUnknown(pName, pUnk)
{
    hr = m_pEVR.CoCreateInstance(CLSID_EnhancedVideoRenderer, GetOwner());
    m_pVMR9AlphaBitmap = pVMR9AlphaBitmap;
    m_pAllocatorPresenter = pAllocatorPresenter;
}

CSyncRenderer::~CSyncRenderer()
{
}

HRESULT STDMETHODCALLTYPE CSyncRenderer::GetState(DWORD dwMilliSecsTimeout, __out  FILTER_STATE *State)
{
    HRESULT ReturnValue;
    CComPtr<IBaseFilter> pEVRBase;
    if (m_pEVR)
        m_pEVR->QueryInterface(&pEVRBase);
    if (pEVRBase)
        return pEVRBase->GetState(dwMilliSecsTimeout, State);
    return E_NOTIMPL;
}

STDMETHODIMP CSyncRenderer::EnumPins(__out IEnumPins **ppEnum)
{
    CComPtr<IBaseFilter> pEVRBase;
    if (m_pEVR)
        m_pEVR->QueryInterface(&pEVRBase);
    if (pEVRBase)
        return pEVRBase->EnumPins(ppEnum);
    return E_NOTIMPL;
}

STDMETHODIMP CSyncRenderer::FindPin(LPCWSTR Id, __out  IPin **ppPin)
{
    CComPtr<IBaseFilter> pEVRBase;
    if (m_pEVR)
        m_pEVR->QueryInterface(&pEVRBase);
    if (pEVRBase)
        return pEVRBase->FindPin(Id, ppPin);
    return E_NOTIMPL;
}

STDMETHODIMP CSyncRenderer::QueryFilterInfo(__out  FILTER_INFO *pInfo)
{
    CComPtr<IBaseFilter> pEVRBase;
    if (m_pEVR)
        m_pEVR->QueryInterface(&pEVRBase);
    if (pEVRBase)
        return pEVRBase->QueryFilterInfo(pInfo);
    return E_NOTIMPL;
}

STDMETHODIMP CSyncRenderer::JoinFilterGraph(__in_opt  IFilterGraph *pGraph, __in_opt  LPCWSTR pName)
{
    CComPtr<IBaseFilter> pEVRBase;
    if (m_pEVR)
        m_pEVR->QueryInterface(&pEVRBase);
    if (pEVRBase)
        return pEVRBase->JoinFilterGraph(pGraph, pName);
    return E_NOTIMPL;
}

STDMETHODIMP CSyncRenderer::QueryVendorInfo(__out  LPWSTR *pVendorInfo)
{
    CComPtr<IBaseFilter> pEVRBase;
    if (m_pEVR)
        m_pEVR->QueryInterface(&pEVRBase);
    if (pEVRBase)
        return pEVRBase->QueryVendorInfo(pVendorInfo);
    return E_NOTIMPL;
}

STDMETHODIMP CSyncRenderer::Stop( void)
{
    CComPtr<IBaseFilter> pEVRBase;
    if (m_pEVR)
        m_pEVR->QueryInterface(&pEVRBase);
    if (pEVRBase)
        return pEVRBase->Stop();
    return E_NOTIMPL;
}

STDMETHODIMP CSyncRenderer::Pause(void)
{
    CComPtr<IBaseFilter> pEVRBase;
    if (m_pEVR)
        m_pEVR->QueryInterface(&pEVRBase);
    if (pEVRBase)
        return pEVRBase->Pause();
    return E_NOTIMPL;
}

STDMETHODIMP CSyncRenderer::Run(REFERENCE_TIME tStart)
{
    CComPtr<IBaseFilter> pEVRBase;
    if (m_pEVR)
        m_pEVR->QueryInterface(&pEVRBase);
    if (pEVRBase)
        return pEVRBase->Run(tStart);
    return E_NOTIMPL;
}

STDMETHODIMP CSyncRenderer::SetSyncSource(__in_opt IReferenceClock *pClock)
{
    CComPtr<IBaseFilter> pEVRBase;
    if (m_pEVR)
        m_pEVR->QueryInterface(&pEVRBase);
    if (pEVRBase)
        return pEVRBase->SetSyncSource(pClock);
    return E_NOTIMPL;
}

STDMETHODIMP CSyncRenderer::GetSyncSource(__deref_out_opt IReferenceClock **pClock)
{
    CComPtr<IBaseFilter> pEVRBase;
    if (m_pEVR)
        m_pEVR->QueryInterface(&pEVRBase);
    if (pEVRBase)
        return pEVRBase->GetSyncSource(pClock);
    return E_NOTIMPL;
}

STDMETHODIMP CSyncRenderer::GetClassID(__RPC__out CLSID *pClassID)
{
    CComPtr<IBaseFilter> pEVRBase;
    if (m_pEVR)
        m_pEVR->QueryInterface(&pEVRBase);
    if (pEVRBase)
        return pEVRBase->GetClassID(pClassID);
    return E_NOTIMPL;
}

STDMETHODIMP CSyncRenderer::GetAlphaBitmapParameters(VMR9AlphaBitmap* pBmpParms)
{
    CheckPointer(pBmpParms, E_POINTER);
    CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
    memcpy (pBmpParms, m_pVMR9AlphaBitmap, sizeof(VMR9AlphaBitmap));
    return S_OK;
}

STDMETHODIMP CSyncRenderer::SetAlphaBitmap(const VMR9AlphaBitmap*  pBmpParms)
{
    CheckPointer(pBmpParms, E_POINTER);
    CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
    memcpy (m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
    m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
    m_pAllocatorPresenter->UpdateAlphaBitmap();
    return S_OK;
}

STDMETHODIMP CSyncRenderer::UpdateAlphaBitmapParameters(const VMR9AlphaBitmap* pBmpParms)
{
    CheckPointer(pBmpParms, E_POINTER);
    CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
    memcpy (m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
    m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
    m_pAllocatorPresenter->UpdateAlphaBitmap();
    return S_OK;
}

STDMETHODIMP CSyncRenderer::support_ffdshow()
{
    queue_ffdshow_support = true;
    return S_OK;
}

STDMETHODIMP CSyncRenderer::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr;

    if(riid == __uuidof(IVMRMixerBitmap9))
        return GetInterface((IVMRMixerBitmap9*)this, ppv);

    if (riid == __uuidof(IBaseFilter))
    {
        return GetInterface((IBaseFilter*)this, ppv);
    }

    if (riid == __uuidof(IMediaFilter))
    {
        return GetInterface((IMediaFilter*)this, ppv);
    }
    if (riid == __uuidof(IPersist))
    {
        return GetInterface((IPersist*)this, ppv);
    }
    if (riid == __uuidof(IBaseFilter))
    {
        return GetInterface((IBaseFilter*)this, ppv);
    }

    hr = m_pEVR ? m_pEVR->QueryInterface(riid, ppv) : E_NOINTERFACE;
    if(m_pEVR && FAILED(hr))
    {
        if(riid == __uuidof(IVMRffdshow9)) // Support ffdshow queueing. We show ffdshow that this is patched Media Player Classic.
            return GetInterface((IVMRffdshow9*)this, ppv);
    }
    return SUCCEEDED(hr) ? hr : __super::NonDelegatingQueryInterface(riid, ppv);
}

CGenlock::CGenlock(DOUBLE target, DOUBLE limit, INT lineD, INT colD, DOUBLE clockD, UINT mon):
    targetSyncOffset(target), // Target sync offset, typically around 10 ms
    controlLimit(limit), // How much sync offset is allowed to drift from target sync offset before control kicks in
    lineDelta(lineD), // Number of rows used in display frequency adjustment, typically 1 (one)
    columnDelta(colD),  // Number of columns used in display frequency adjustment, typically 1 - 2
    cycleDelta(clockD),  // Delta used in clock speed adjustment. In fractions of 1.0. Typically around 0.001
    monitor(mon) // The monitor to be adjusted if the display refresh rate is the controlled parameter
{
    lowSyncOffset = targetSyncOffset - controlLimit;
    highSyncOffset = targetSyncOffset + controlLimit;
    adjDelta = 0;
    displayAdjustmentsMade = 0;
    clockAdjustmentsMade = 0;
    displayFreqCruise = 0;
    displayFreqFaster = 0;
    displayFreqSlower = 0;
    curDisplayFreq = 0;
    psWnd = NULL;
    liveSource = FALSE;
    powerstripTimingExists = FALSE;
    syncOffsetFifo = new MovingAverage(64);
    frameCycleFifo = new MovingAverage(4);
}

CGenlock::~CGenlock()
{
    ResetTiming();
    if(syncOffsetFifo != NULL)
    {
        delete syncOffsetFifo;
        syncOffsetFifo = NULL;
    }
    if(frameCycleFifo != NULL)
    {
        delete frameCycleFifo;
        frameCycleFifo = NULL;
    }
    syncClock = NULL;
};

BOOL CGenlock::PowerstripRunning()
{
    psWnd = FindWindow(_T("TPShidden"), NULL);
    if (!psWnd) return FALSE; // Powerstrip is not running
    else return TRUE;
}

// Get the display timing parameters through PowerStrip (if running).
HRESULT CGenlock::GetTiming()
{
    ATOM getTiming;
    LPARAM lParam = NULL;
    WPARAM wParam = monitor;
    INT i = 0;
    INT j = 0;
    INT params = 0;
    BOOL done = FALSE;
    TCHAR tmpStr[MAX_LOADSTRING];

    CAutoLock lock(&csGenlockLock);
    if (!PowerstripRunning()) return E_FAIL;

    getTiming = static_cast<ATOM>(SendMessage(psWnd, UM_GETTIMING, wParam, lParam));
    GlobalGetAtomName(getTiming, savedTiming, MAX_LOADSTRING);

    while (params < TIMING_PARAM_CNT)
    {
        while (savedTiming[i] != ',' && savedTiming[i] != '\0')
        {
            tmpStr[j++] = savedTiming[i];
            tmpStr[j] = '\0';
            i++;
        }
        i++; // Skip trailing comma
        j = 0;
        displayTiming[params] = _ttoi(tmpStr);
        displayTimingSave[params] = displayTiming[params];
        params++;
    }

    // The display update frequency is controlled by adding and subtracting pixels form the
    // image. This is done by either subtracting columns or rows or both. Some displays like
    // row adjustments and some column adjustments. One should probably not do both.
    StringCchPrintf(faster, MAX_LOADSTRING, TEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\0"),
                    displayTiming[0],
                    displayTiming[HFRONTPORCH] - columnDelta,
                    displayTiming[2],
                    displayTiming[3],
                    displayTiming[4],
                    displayTiming[VFRONTPORCH] - lineDelta,
                    displayTiming[6],
                    displayTiming[7],
                    displayTiming[PIXELCLOCK],
                    displayTiming[9]
                   );

    // Nominal update frequency
    StringCchPrintf(cruise, MAX_LOADSTRING, TEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\0"),
                    displayTiming[0],
                    displayTiming[HFRONTPORCH],
                    displayTiming[2],
                    displayTiming[3],
                    displayTiming[4],
                    displayTiming[VFRONTPORCH],
                    displayTiming[6],
                    displayTiming[7],
                    displayTiming[PIXELCLOCK],
                    displayTiming[9]
                   );

    // Lower than nominal update frequency
    StringCchPrintf(slower, MAX_LOADSTRING, TEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\0"),
                    displayTiming[0],
                    displayTiming[HFRONTPORCH] + columnDelta,
                    displayTiming[2],
                    displayTiming[3],
                    displayTiming[4],
                    displayTiming[VFRONTPORCH] + lineDelta,
                    displayTiming[6],
                    displayTiming[7],
                    displayTiming[PIXELCLOCK],
                    displayTiming[9]
                   );

    totalColumns = displayTiming[HACTIVE] + displayTiming[HFRONTPORCH] + displayTiming[HSYNCWIDTH] + displayTiming[HBACKPORCH];
    totalLines = displayTiming[VACTIVE] + displayTiming[VFRONTPORCH] + displayTiming[VSYNCWIDTH] + displayTiming[VBACKPORCH];
    pixelClock = 1000 * displayTiming[PIXELCLOCK]; // Pixels/s
    displayFreqCruise = (DOUBLE)pixelClock / (totalLines * totalColumns); // Frames/s
    displayFreqSlower = (DOUBLE)pixelClock / ((totalLines + lineDelta) * (totalColumns + columnDelta));
    displayFreqFaster = (DOUBLE)pixelClock / ((totalLines - lineDelta) * (totalColumns - columnDelta));
    curDisplayFreq = displayFreqCruise;
    GlobalDeleteAtom(getTiming);
    adjDelta = 0;
    powerstripTimingExists = TRUE;
    return S_OK;
}

// Reset display timing parameters to nominal.
HRESULT CGenlock::ResetTiming()
{
    LPARAM lParam = NULL;
    WPARAM wParam = monitor;
    ATOM setTiming;
    LRESULT ret;
    CAutoLock lock(&csGenlockLock);

    if (!PowerstripRunning()) return E_FAIL;

    if (displayAdjustmentsMade > 0)
    {
        setTiming = GlobalAddAtom(cruise);
        lParam = setTiming;
        ret = SendMessage(psWnd, UM_SETCUSTOMTIMINGFAST, wParam, lParam);
        GlobalDeleteAtom(setTiming);
        curDisplayFreq = displayFreqCruise;
    }
    adjDelta = 0;
    return S_OK;
}

// Reset reference clock speed to nominal.
HRESULT CGenlock::ResetClock()
{
    adjDelta = 0;
    if (syncClock == NULL) return E_FAIL;
    else return syncClock->AdjustClock(1.0);
    return S_OK;
}

HRESULT CGenlock::SetTargetSyncOffset(DOUBLE targetD)
{
    targetSyncOffset = targetD;
    lowSyncOffset = targetD - controlLimit;
    highSyncOffset = targetD + controlLimit;
    return S_OK;
}

HRESULT CGenlock::GetTargetSyncOffset(DOUBLE *targetD)
{
    *targetD = targetSyncOffset;
    return S_OK;
}

HRESULT CGenlock::SetControlLimit(DOUBLE cL)
{
    controlLimit = cL;
    return S_OK;
}

HRESULT CGenlock::GetControlLimit(DOUBLE *cL)
{
    *cL = controlLimit;
    return S_OK;
}

HRESULT CGenlock::SetDisplayResolution(UINT columns, UINT lines)
{
    visibleColumns = columns;
    visibleLines = lines;
    return S_OK;
}

HRESULT CGenlock::AdviseSyncClock(ISyncClock* sC)
{
    if (!sC) return E_FAIL;
    if (syncClock) syncClock = NULL; // Release any outstanding references if this is called repeatedly
    syncClock = sC;
    return S_OK;
}

// Set the monitor to control. This is best done manually as not all monitors can be controlled
// so automatic detection of monitor to control might have unintended effects.
// The PowerStrip API uses zero-based monitor numbers, i.e. the default monitor is 0.
HRESULT CGenlock::SetMonitor(UINT mon)
{
    monitor = mon;
    return S_OK;
}

HRESULT CGenlock::ResetStats()
{
    CAutoLock lock(&csGenlockLock);
    minSyncOffset = 1000000.0;
    maxSyncOffset = -1000000.0;
    minFrameCycle = 1000000.0;
    maxFrameCycle = -1000000.0;
    displayAdjustmentsMade = 0;
    clockAdjustmentsMade = 0;
    return S_OK;
}

// Synchronize by adjusting display refresh rate
HRESULT CGenlock::ControlDisplay(double syncOffset, double frameCycle)
{
    LPARAM lParam = NULL;
    WPARAM wParam = monitor;
    ATOM setTiming;

    CRenderersSettings& s = GetRenderersSettings();
    targetSyncOffset = s.m_RenderSettings.fTargetSyncOffset;
    lowSyncOffset = targetSyncOffset - s.m_RenderSettings.fControlLimit;
    highSyncOffset = targetSyncOffset + s.m_RenderSettings.fControlLimit;

    syncOffsetAvg = syncOffsetFifo->Average(syncOffset);
    minSyncOffset = min(minSyncOffset, syncOffset);
    maxSyncOffset = max(maxSyncOffset, syncOffset);
    frameCycleAvg = frameCycleFifo->Average(frameCycle);
    minFrameCycle = min(minFrameCycle, frameCycle);
    maxFrameCycle = max(maxFrameCycle, frameCycle);

    if (!PowerstripRunning() || !powerstripTimingExists) return E_FAIL;
    // Adjust as seldom as possible by checking the current controlState before changing it.
    if ((syncOffsetAvg > highSyncOffset) && (adjDelta != 1))
        // Speed up display refresh rate by subtracting pixels from the image.
    {
        adjDelta = 1; // Increase refresh rate
        curDisplayFreq = displayFreqFaster;
        setTiming = GlobalAddAtom(faster);
        lParam = setTiming;
        SendMessage(psWnd, UM_SETCUSTOMTIMINGFAST, wParam, lParam);
        GlobalDeleteAtom(setTiming);
        displayAdjustmentsMade++;
    }
    else
        // Slow down display refresh rate by adding pixels to the image.
        if ((syncOffsetAvg < lowSyncOffset) && (adjDelta != -1))
        {
            adjDelta = -1;
            curDisplayFreq = displayFreqSlower;
            setTiming = GlobalAddAtom(slower);
            lParam = setTiming;
            SendMessage(psWnd, UM_SETCUSTOMTIMINGFAST, wParam, lParam);
            GlobalDeleteAtom(setTiming);
            displayAdjustmentsMade++;
        }
        else
            // Cruise.
            if ((syncOffsetAvg < targetSyncOffset) && (adjDelta == 1))
            {
                adjDelta = 0;
                curDisplayFreq = displayFreqCruise;
                setTiming = GlobalAddAtom(cruise);
                lParam = setTiming;
                SendMessage(psWnd, UM_SETCUSTOMTIMINGFAST, wParam, lParam);
                GlobalDeleteAtom(setTiming);
                displayAdjustmentsMade++;
            }
            else if ((syncOffsetAvg > targetSyncOffset) && (adjDelta == -1))
            {
                adjDelta = 0;
                curDisplayFreq = displayFreqCruise;
                setTiming = GlobalAddAtom(cruise);
                lParam = setTiming;
                SendMessage(psWnd, UM_SETCUSTOMTIMINGFAST, wParam, lParam);
                GlobalDeleteAtom(setTiming);
                displayAdjustmentsMade++;
            }
    return S_OK;
}

// Synchronize by adjusting reference clock rate (and therefore video FPS).
// Todo: check so that we don't have a live source
HRESULT CGenlock::ControlClock(double syncOffset, double frameCycle)
{
    CRenderersSettings& s = GetRenderersSettings();
    targetSyncOffset = s.m_RenderSettings.fTargetSyncOffset;
    lowSyncOffset = targetSyncOffset - s.m_RenderSettings.fControlLimit;
    highSyncOffset = targetSyncOffset + s.m_RenderSettings.fControlLimit;

    syncOffsetAvg = syncOffsetFifo->Average(syncOffset);
    minSyncOffset = min(minSyncOffset, syncOffset);
    maxSyncOffset = max(maxSyncOffset, syncOffset);
    frameCycleAvg = frameCycleFifo->Average(frameCycle);
    minFrameCycle = min(minFrameCycle, frameCycle);
    maxFrameCycle = max(maxFrameCycle, frameCycle);

    if (!syncClock) return E_FAIL;
    // Adjust as seldom as possible by checking the current controlState before changing it.
    if ((syncOffsetAvg > highSyncOffset) && (adjDelta != 1))
        // Slow down video stream.
    {
        adjDelta = 1;
        syncClock->AdjustClock(1.0 - cycleDelta); // Makes the clock move slower by providing smaller increments
        clockAdjustmentsMade++;
    }
    else
        // Speed up video stream.
        if ((syncOffsetAvg < lowSyncOffset) && (adjDelta != -1))
        {
            adjDelta = -1;
            syncClock->AdjustClock(1.0 + cycleDelta);
            clockAdjustmentsMade++;
        }
        else
            // Cruise.
            if ((syncOffsetAvg < targetSyncOffset) && (adjDelta == 1))
            {
                adjDelta = 0;
                syncClock->AdjustClock(1.0);
                clockAdjustmentsMade++;
            }
            else if ((syncOffsetAvg > targetSyncOffset) && (adjDelta == -1))
            {
                adjDelta = 0;
                syncClock->AdjustClock(1.0);
                clockAdjustmentsMade++;
            }
    return S_OK;
}

// Don't adjust anything, just update the syncOffset stats
HRESULT CGenlock::UpdateStats(double syncOffset, double frameCycle)
{
    syncOffsetAvg = syncOffsetFifo->Average(syncOffset);
    minSyncOffset = min(minSyncOffset, syncOffset);
    maxSyncOffset = max(maxSyncOffset, syncOffset);
    frameCycleAvg = frameCycleFifo->Average(frameCycle);
    minFrameCycle = min(minFrameCycle, frameCycle);
    maxFrameCycle = max(maxFrameCycle, frameCycle);
    return S_OK;
}