/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "mplayerc.h"
#include <atlbase.h>
#include <atlcoll.h>
#include "..\..\DSUtil\DSUtil.h"

#include <Videoacc.h>

#include <initguid.h>
#include "DX9AllocatorPresenter.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <Vmr9.h>
#include "..\..\SubPic\DX9SubPic.h"
#include "..\..\..\include\RealMedia\pntypes.h"
#include "..\..\..\include\RealMedia\pnwintyp.h"
#include "..\..\..\include\RealMedia\pncom.h"
#include "..\..\..\include\RealMedia\rmavsurf.h"
#include "IQTVideoSurface.h"
#include "..\..\..\include\moreuuids.h"

#include "MacrovisionKicker.h"
#include "IPinHook.h"

#include "PixelShaderCompiler.h"
#include "MainFrm.h"

#include "AllocatorCommon.h"

#define VMRBITMAP_UPDATE            0x80000000
#define FRAMERATE_MAX_DELTA			3000

CCritSec g_ffdshowReceive;
bool queueu_ffdshow_support = false;

bool IsVMR9InGraph(IFilterGraph* pFG)
{
	BeginEnumFilters(pFG, pEF, pBF)
		if(CComQIPtr<IVMRWindowlessControl9>(pBF)) return(true);
	EndEnumFilters
	return(false);
}

using namespace DSObjects;

//

HRESULT CreateAP9(const CLSID& clsid, HWND hWnd, ISubPicAllocatorPresenter** ppAP)
{
	CheckPointer(ppAP, E_POINTER);

	*ppAP = NULL;

	HRESULT hr = E_FAIL;
	if(clsid == CLSID_VMR9AllocatorPresenter && !(*ppAP = new CVMR9AllocatorPresenter(hWnd, hr))
	|| clsid == CLSID_RM9AllocatorPresenter && !(*ppAP = new CRM9AllocatorPresenter(hWnd, hr))
	|| clsid == CLSID_QT9AllocatorPresenter && !(*ppAP = new CQT9AllocatorPresenter(hWnd, hr))
	|| clsid == CLSID_DXRAllocatorPresenter && !(*ppAP = new CDXRAllocatorPresenter(hWnd, hr)))
		return E_OUTOFMEMORY;

	if(*ppAP == NULL)
		return E_FAIL;

	(*ppAP)->AddRef();

	if(FAILED(hr))
	{
		(*ppAP)->Release();
		*ppAP = NULL;
	}

	return hr;
}

//

#pragma pack(push, 1)
template<int texcoords>
struct MYD3DVERTEX {float x, y, z, rhw; struct {float u, v;} t[texcoords];};
#pragma pack(pop)

template<int texcoords>
static void AdjustQuad(MYD3DVERTEX<texcoords>* v, float dx, float dy)
{
	float offset = 0.5f;

	for(int i = 0; i < 4; i++)
	{
		v[i].x -= offset;
		v[i].y -= offset;
		
		for(int j = 0; j < texcoords-1; j++)
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
static HRESULT TextureBlt(CComPtr<IDirect3DDevice9> pD3DDev, MYD3DVERTEX<texcoords> v[4], D3DTEXTUREFILTERTYPE filter = D3DTEXF_LINEAR)
{
	if(!pD3DDev)
		return E_POINTER;

	DWORD FVF = 0;

	switch(texcoords)
	{
	case 1: FVF = D3DFVF_TEX1; break;
	case 2: FVF = D3DFVF_TEX2; break;
	case 3: FVF = D3DFVF_TEX3; break;
	case 4: FVF = D3DFVF_TEX4; break;
	case 5: FVF = D3DFVF_TEX5; break;
	case 6: FVF = D3DFVF_TEX6; break;
	case 7: FVF = D3DFVF_TEX7; break;
	case 8: FVF = D3DFVF_TEX8; break;
	default: return E_FAIL;
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

		//

		if(FAILED(hr = pD3DDev->BeginScene()))
			break;

        hr = pD3DDev->SetFVF(D3DFVF_XYZRHW | FVF);
		// hr = pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(v[0]));

		MYD3DVERTEX<texcoords> tmp = v[2]; v[2] = v[3]; v[3] = tmp;
		hr = pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(v[0]));	

		hr = pD3DDev->EndScene();

        //

		for(int i = 0; i < texcoords; i++)
		{
			pD3DDev->SetTexture(i, NULL);
		}

		return S_OK;
    }
	while(0);

    return E_FAIL;
}

// CDX9AllocatorPresenter

CDX9AllocatorPresenter::CDX9AllocatorPresenter(HWND hWnd, HRESULT& hr) 
	: ISubPicAllocatorPresenterImpl(hWnd, hr)
	, m_ScreenSize(0, 0)
	, m_bicubicA(0)
	, m_nTearingPos(0)
	, m_nPictureSlots(1)
	, m_nCurPicture(0)
	, m_rtCandidate(0)
{
	if(FAILED(hr)) return;
	m_pD3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
	if(!m_pD3D) m_pD3D.Attach(Direct3DCreate9(D3D9b_SDK_VERSION));
	if(!m_pD3D) {hr = E_FAIL; return;}

	CString d3dx9_dll;
	d3dx9_dll.Format(_T("d3dx9_%d.dll"), D3DX_SDK_VERSION);

	m_pD3DXLoadSurfaceFromMemory	= NULL;
	m_pD3DXCreateLine				= NULL;
	m_pD3DXCreateFont				= NULL;
	m_hDll = LoadLibrary(d3dx9_dll);
	if(m_hDll)
	{
		m_pD3DXLoadSurfaceFromMemory = (D3DXLoadSurfaceFromMemoryPtr)GetProcAddress(m_hDll, "D3DXLoadSurfaceFromMemory");
		m_pD3DXCreateLine			 = (D3DXCreateLinePtr)			 GetProcAddress(m_hDll, "D3DXCreateLine");
		m_pD3DXCreateFont			 = (D3DXCreateFontPtr)			 GetProcAddress(m_hDll, "D3DXCreateFontW");
	}

	ZeroMemory(&m_VMR9AlphaBitmap, sizeof(m_VMR9AlphaBitmap));
	hr = CreateDevice();

	memset (m_pllJitter, 0, sizeof(m_pllJitter));
	m_nNextJitter		= 0;
	m_llLastPerf		= 0;
	m_rtTimePerFrame	= 0;
}

CDX9AllocatorPresenter::~CDX9AllocatorPresenter() 
{
	m_pFont		= NULL;
	m_pLine		= NULL;
    m_pD3DDev	= NULL;
	m_pPSC.Free();
	m_pD3D.Detach();

	if(m_hDll) FreeLibrary(m_hDll);
}

HRESULT CDX9AllocatorPresenter::CreateDevice()
{
	m_pPSC.Free();
    m_pD3DDev = NULL;

	D3DDISPLAYMODE d3ddm;
	HRESULT hr;
	ZeroMemory(&d3ddm, sizeof(d3ddm));
	if(FAILED(m_pD3D->GetAdapterDisplayMode(GetAdapter(m_pD3D), &d3ddm)))
		return E_UNEXPECTED;

	m_ScreenSize.SetSize(d3ddm.Width, d3ddm.Height);

    D3DPRESENT_PARAMETERS pp;
    ZeroMemory(&pp, sizeof(pp));

	
	if (AfxGetAppSettings().fD3DFullscreen)
	{
		pp.Windowed = false; 
		pp.BackBufferWidth = d3ddm.Width; 
		pp.BackBufferHeight = d3ddm.Height; 
		pp.hDeviceWindow = m_hWnd;
		pp.BackBufferCount = 1; 
		pp.SwapEffect = D3DSWAPEFFECT_FLIP;		// Ne pas mettre D3DSWAPEFFECT_COPY car cela entraine une desynchro audio sur les MKV !
		pp.Flags = D3DPRESENTFLAG_VIDEO;
		pp.BackBufferFormat = d3ddm.Format; 
		pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		hr = m_pD3D->CreateDevice(
							GetAdapter(m_pD3D), D3DDEVTYPE_HAL, m_hWnd,
							D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED, //D3DCREATE_MANAGED 
							&pp, &m_pD3DDev);
		ASSERT (SUCCEEDED (hr));
	}
	else
	{
		pp.Windowed = TRUE;
		pp.hDeviceWindow = m_hWnd;
		pp.SwapEffect = D3DSWAPEFFECT_COPY;
		pp.Flags = D3DPRESENTFLAG_VIDEO;
		pp.BackBufferWidth = d3ddm.Width;
		pp.BackBufferHeight = d3ddm.Height;
		pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		if(m_fVMRSyncFix = AfxGetMyApp()->m_s.fVMRSyncFix)
			pp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

		hr = m_pD3D->CreateDevice(
							GetAdapter(m_pD3D), D3DDEVTYPE_HAL, m_hWnd,
							D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED, //D3DCREATE_MANAGED 
							&pp, &m_pD3DDev);
	}
/*
	HRESULT hr = m_pD3D->CreateDevice(
						m_pD3D->GetAdapterCount()-1, D3DDEVTYPE_REF, m_hWnd,
						D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED, //D3DCREATE_MANAGED 
						&pp, &m_pD3DDev);
*/
	if(FAILED(hr))
		return hr;

	m_pPSC.Attach(new CPixelShaderCompiler(m_pD3DDev, true));

	//

	m_filter = D3DTEXF_NONE;

    ZeroMemory(&m_caps, sizeof(m_caps));
	m_pD3DDev->GetDeviceCaps(&m_caps);

	if((m_caps.StretchRectFilterCaps&D3DPTFILTERCAPS_MINFLINEAR)
	&& (m_caps.StretchRectFilterCaps&D3DPTFILTERCAPS_MAGFLINEAR))
		m_filter = D3DTEXF_LINEAR;

	//

	m_bicubicA = 0;

	//

	CComPtr<ISubPicProvider> pSubPicProvider;
	if(m_pSubPicQueue) m_pSubPicQueue->GetSubPicProvider(&pSubPicProvider);

	CSize size;
	switch(AfxGetAppSettings().nSPCMaxRes)
	{
	case 0: default: size = m_ScreenSize; break;
	case 1: size.SetSize(1024, 768); break;
	case 2: size.SetSize(800, 600); break;
	case 3: size.SetSize(640, 480); break;
	case 4: size.SetSize(512, 384); break;
	case 5: size.SetSize(384, 288); break;
	}

	if(m_pAllocator)
	{
		m_pAllocator->ChangeDevice(m_pD3DDev);
	}
	else
	{
		m_pAllocator = new CDX9SubPicAllocator(m_pD3DDev, size, AfxGetAppSettings().fSPCPow2Tex);
		if(!m_pAllocator)
			return E_FAIL;
	}

	hr = S_OK;
	m_pSubPicQueue = AfxGetAppSettings().nSPCSize > 0 
		? (ISubPicQueue*)new CSubPicQueue(AfxGetAppSettings().nSPCSize, m_pAllocator, &hr)
		: (ISubPicQueue*)new CSubPicQueueNoThread(m_pAllocator, &hr);
	if(!m_pSubPicQueue || FAILED(hr))
		return E_FAIL;

	if(pSubPicProvider) m_pSubPicQueue->SetSubPicProvider(pSubPicProvider);

	m_pFont = NULL;
	if (m_pD3DXCreateFont)
		m_pD3DXCreateFont( m_pD3DDev,            // D3D device
							 -20,               // Height
							 0,                     // Width
							 FW_BOLD,               // Weight
							 1,                     // MipLevels, 0 = autogen mipmaps
							 FALSE,                 // Italic
							 DEFAULT_CHARSET,       // CharSet
							 OUT_DEFAULT_PRECIS,    // OutputPrecision
							 DEFAULT_QUALITY,       // Quality
							 DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
							 L"Arial",              // pFaceName
							 &m_pFont);              // ppFont

	m_pLine = NULL;
	if (m_pD3DXCreateLine)
		m_pD3DXCreateLine (m_pD3DDev, &m_pLine);

	return S_OK;
} 

HRESULT CDX9AllocatorPresenter::AllocSurfaces()
{
    CAutoLock cAutoLock(this);

	AppSettings& s = AfxGetAppSettings();

	for(int i = 0; i < m_nPictureSlots+2; i++)
	{
		m_pVideoTexture[i] = NULL;
		m_pVideoSurface[i] = NULL;
	}

	m_pResizerBicubic1stPass = NULL;

	HRESULT hr;

	if(s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D || s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
	{
		int nTexturesNeeded = s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D ? m_nPictureSlots+2 : 1;

		for(int i = 0; i < nTexturesNeeded; i++)
		{
			if(FAILED(hr = m_pD3DDev->CreateTexture(
				m_NativeVideoSize.cx, m_NativeVideoSize.cy, 1, 
				D3DUSAGE_RENDERTARGET, /*D3DFMT_X8R8G8B8*/D3DFMT_A8R8G8B8, 
				D3DPOOL_DEFAULT, &m_pVideoTexture[i], NULL)))
				return hr;

			if(FAILED(hr = m_pVideoTexture[i]->GetSurfaceLevel(0, &m_pVideoSurface[i])))
				return hr;
		}

		if(s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D)
		{
			for(int i = 0; i < m_nPictureSlots+2; i++)
			{
				m_pVideoTexture[i] = NULL;
			}
		}
	}
	else
	{
		if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(
			m_NativeVideoSize.cx, m_NativeVideoSize.cy, 
			D3DFMT_X8R8G8B8/*D3DFMT_A8R8G8B8*/, 
			D3DPOOL_DEFAULT, &m_pVideoSurface[m_nCurPicture], NULL)))
			return hr;
	}

	hr = m_pD3DDev->ColorFill(m_pVideoSurface[m_nCurPicture], NULL, 0);

	return S_OK;
}

void CDX9AllocatorPresenter::DeleteSurfaces()
{
    CAutoLock cAutoLock(this);

	for(int i = 0; i < m_nPictureSlots+2; i++)
	{
		m_pVideoTexture[i] = NULL;
		m_pVideoSurface[i] = NULL;
	}
}

UINT CDX9AllocatorPresenter::GetAdapter(IDirect3D9* pD3D)
{
	if(m_hWnd == NULL || pD3D == NULL)
		return D3DADAPTER_DEFAULT;

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

STDMETHODIMP CDX9AllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
	return E_NOTIMPL;
}

static bool ClipToSurface(IDirect3DSurface9* pSurface, CRect& s, CRect& d)   
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

	if(d.right > w) {s.right -= (d.right-w)*sw/dw; d.right = w;}   
	if(d.bottom > h) {s.bottom -= (d.bottom-h)*sh/dh; d.bottom = h;}   
	if(d.left < 0) {s.left += (0-d.left)*sw/dw; d.left = 0;}   
	if(d.top < 0) {s.top += (0-d.top)*sh/dh; d.top = 0;}   

	return(true);
}

HRESULT CDX9AllocatorPresenter::InitResizers(float bicubicA)
{
	HRESULT hr;

	if(m_pResizerPixelShader[0] && m_bicubicA == 0 && bicubicA == 0
	|| m_pResizerPixelShader[1] && m_pResizerPixelShader[2] && m_bicubicA == bicubicA && m_pResizerBicubic1stPass)
		return S_OK;

	m_bicubicA = bicubicA;
	m_pResizerBicubic1stPass = NULL;

	for(int i = 0; i < countof(m_pResizerPixelShader); i++)
		m_pResizerPixelShader[i] = NULL;

	if(m_caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return E_FAIL;

	LPCSTR pProfile = m_caps.PixelShaderVersion >= D3DPS_VERSION(3, 0) ? "ps_3_0" : "ps_2_0";

	CStringA str;
	if(!LoadResource(IDF_SHADER_RESIZER, str, _T("FILE")))
		return E_FAIL;

	CStringA A;
	A.Format("(%f)", bicubicA);
	str.Replace("_The_Value_Of_A_Is_Set_Here_", A);

	LPCSTR pEntries[] = {"main_bilinear", "main_bicubic1pass", "main_bicubic2pass"};

	ASSERT(countof(pEntries) == countof(m_pResizerPixelShader));

	for(int i = 0; i < countof(pEntries); i++)
	{
		hr = m_pPSC->CompileShader(str, pEntries[i], pProfile, 0, &m_pResizerPixelShader[i]);
		ASSERT (SUCCEEDED (hr));
		if(FAILED(hr)) return hr;
	}

	if(m_bicubicA)
	{
		if(FAILED(m_pD3DDev->CreateTexture(
			min(max(2048, m_ScreenSize.cx), m_caps.MaxTextureWidth), m_NativeVideoSize.cy, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
			D3DPOOL_DEFAULT, &m_pResizerBicubic1stPass, NULL)))
		{
			ASSERT(0);
			m_pResizerBicubic1stPass = NULL; // will do 1 pass then
		}
	}

	return S_OK;
}

HRESULT CDX9AllocatorPresenter::TextureCopy(CComPtr<IDirect3DTexture9> pTexture)
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

HRESULT CDX9AllocatorPresenter::TextureResize(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], D3DTEXTUREFILTERTYPE filter)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
		return E_FAIL;

	float w = (float)desc.Width;
	float h = (float)desc.Height;

	float dx = 1.0f/w;
	float dy = 1.0f/h;

	MYD3DVERTEX<1> v[] =
	{
		{dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  0, 0},
		{dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  1, 0},
		{dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  0, 1},
		{dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  1, 1},
	};

	AdjustQuad(v, dx, dy);

	hr = m_pD3DDev->SetTexture(0, pTexture);

	hr = m_pD3DDev->SetPixelShader(NULL);

	hr = TextureBlt(m_pD3DDev, v, filter);

	return hr;
}

HRESULT CDX9AllocatorPresenter::TextureResizeBilinear(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4])
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
		return E_FAIL;

	float w = (float)desc.Width;
	float h = (float)desc.Height;

	float dx = 1.0f/w;
	float dy = 1.0f/h;

	MYD3DVERTEX<5> v[] =
	{
		{dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  0, 0,  0+dx, 0,  0, 0+dy,  0+dx, 0+dy,  0, 0},
		{dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  1, 0,  1+dx, 0,  1, 0+dy,  1+dx, 0+dy,  w, 0},
		{dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  0, 1,  0+dx, 1,  0, 1+dy,  0+dx, 1+dy,  0, h},
		{dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  1, 1,  1+dx, 1,  1, 1+dy,  1+dx, 1+dy,  w, h},
	};

	AdjustQuad(v, dx, dy);

	hr = m_pD3DDev->SetTexture(0, pTexture);
	hr = m_pD3DDev->SetTexture(1, pTexture);
	hr = m_pD3DDev->SetTexture(2, pTexture);
	hr = m_pD3DDev->SetTexture(3, pTexture);

	hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShader[0]);

	hr = TextureBlt(m_pD3DDev, v, D3DTEXF_POINT);

	//

	m_pD3DDev->SetPixelShader(NULL);

	return hr;
}

HRESULT CDX9AllocatorPresenter::TextureResizeBicubic1pass(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4])
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
		return E_FAIL;

	float w = (float)desc.Width;
	float h = (float)desc.Height;

	float dx = 1.0f/w;
	float dy = 1.0f/h;

	MYD3DVERTEX<2> v[] =
	{
		{dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  0, 0, 0, 0},
		{dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  1, 0, w, 0},
		{dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  0, 1, 0, h},
		{dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  1, 1, w, h},
	};

	AdjustQuad(v, dx, dy);

	hr = m_pD3DDev->SetTexture(0, pTexture);

	float fConstData[][4] = {{w, h, 0, 0}, {1.0f / w, 1.0f / h, 0, 0}, {1.0f / w, 0, 0, 0}, {0, 1.0f / h, 0, 0}};
	hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));

	hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShader[1]);

	hr = TextureBlt(m_pD3DDev, v, D3DTEXF_POINT);

	m_pD3DDev->SetPixelShader(NULL);

	return hr;
}

HRESULT CDX9AllocatorPresenter::TextureResizeBicubic2pass(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4])
{
	// return TextureResizeBicubic1pass(pTexture, dst);

	HRESULT hr;

	// rotated?
	if(dst[0].z != dst[1].z || dst[2].z != dst[3].z || dst[0].z != dst[3].z
	|| dst[0].y != dst[1].y || dst[0].x != dst[2].x || dst[2].y != dst[3].y || dst[1].x != dst[3].x)
		return TextureResizeBicubic1pass(pTexture, dst);

	D3DSURFACE_DESC desc;
	if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
		return E_FAIL;

	float dx = 1.0f/desc.Width;

	float w = (float)desc.Width;
	float h = (float)desc.Height;

	CRect dst1(0, 0, (int)(dst[3].x - dst[0].x), (int)h);

	if(!m_pResizerBicubic1stPass || FAILED(m_pResizerBicubic1stPass->GetLevelDesc(0, &desc)))
		return TextureResizeBicubic1pass(pTexture, dst);

	float dy = 1.0f/desc.Height;

	float dw = (float)dst1.Width() / desc.Width;
	float dh = (float)dst1.Height() / desc.Height;

	ASSERT(dst1.Height() == desc.Height);

	if(dst1.Width() > desc.Width || dst1.Height() > desc.Height)
	// if(dst1.Width() != desc.Width || dst1.Height() != desc.Height)
		return TextureResizeBicubic1pass(pTexture, dst);

	MYD3DVERTEX<5> vx[] =
	{
		{(float)dst1.left, (float)dst1.top, 0.5f, 2.0f, 0-dx, 0,  0, 0,  0+dx, 0,  0+dx*2, 0,  0, 0},
		{(float)dst1.right, (float)dst1.top, 0.5f, 2.0f,  1-dx, 0,  1, 0,  1+dx, 0,  1+dx*2, 0,  w, 0},
		{(float)dst1.left, (float)dst1.bottom, 0.5f, 2.0f,  0-dx, 1,  0, 1,  0+dx, 1,  0+dx*2, 1,  0, 0},
		{(float)dst1.right, (float)dst1.bottom, 0.5f, 2.0f,  1-dx, 1,  1, 1,  1+dx, 1,  1+dx*2, 1,  w, 0},
	};

	AdjustQuad(vx, dx, 0);		// Casimir666 : bug ici, génére des bandes verticales! TODO : pourquoi ??????

	MYD3DVERTEX<5> vy[] =
	{
		{dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  0, 0-dy,  0, 0,  0, 0+dy,  0, 0+dy*2,  0, 0},
		{dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  dw, 0-dy,  dw, 0,  dw, 0+dy,  dw, 0+dy*2,  0, 0},
		{dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  0, dh-dy,  0, dh,  0, dh+dy,  0, dh+dy*2,  h, 0},
		{dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  dw, dh-dy,  dw, dh,  dw, dh+dy,  dw, dh+dy*2,  h, 0},
	};

	AdjustQuad(vy, 0, dy);		// Casimir666 : bug ici, génére des bandes horizontales! TODO : pourquoi ??????

	hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShader[2]);

	hr = m_pD3DDev->SetTexture(0, pTexture);
	hr = m_pD3DDev->SetTexture(1, pTexture);
	hr = m_pD3DDev->SetTexture(2, pTexture);
	hr = m_pD3DDev->SetTexture(3, pTexture);

	CComPtr<IDirect3DSurface9> pRTOld;
	hr = m_pD3DDev->GetRenderTarget(0, &pRTOld);

	CComPtr<IDirect3DSurface9> pRT;
	hr = m_pResizerBicubic1stPass->GetSurfaceLevel(0, &pRT);
	hr = m_pD3DDev->SetRenderTarget(0, pRT);

	hr = TextureBlt(m_pD3DDev, vx, D3DTEXF_POINT);

	hr = m_pD3DDev->SetTexture(0, m_pResizerBicubic1stPass);
	hr = m_pD3DDev->SetTexture(1, m_pResizerBicubic1stPass);
	hr = m_pD3DDev->SetTexture(2, m_pResizerBicubic1stPass);
	hr = m_pD3DDev->SetTexture(3, m_pResizerBicubic1stPass);

	hr = m_pD3DDev->SetRenderTarget(0, pRTOld);

	hr = TextureBlt(m_pD3DDev, vy, D3DTEXF_POINT);

	m_pD3DDev->SetPixelShader(NULL);

	return hr;
}

HRESULT CDX9AllocatorPresenter::AlphaBlt(RECT* pSrc, RECT* pDst, CComPtr<IDirect3DTexture9> pTexture)
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
/*
		for(int i = 0; i < countof(pVertices); i++)
		{
			pVertices[i].x -= 0.5;
			pVertices[i].y -= 0.5;
		}
*/

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

		/*//

		D3DCAPS9 d3dcaps9;
		hr = m_pD3DDev->GetDeviceCaps(&d3dcaps9);
		if(d3dcaps9.AlphaCmpCaps & D3DPCMPCAPS_LESS)
		{
			hr = m_pD3DDev->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x000000FE);
			hr = m_pD3DDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE); 
			hr = m_pD3DDev->SetRenderState(D3DRS_ALPHAFUNC, D3DPCMPCAPS_LESS);
		}

		*///

        hr = m_pD3DDev->SetPixelShader(NULL);

		if(FAILED(hr = m_pD3DDev->BeginScene()))
			break;

        hr = m_pD3DDev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
		hr = m_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertices, sizeof(pVertices[0]));

		hr = m_pD3DDev->EndScene();

        //

		m_pD3DDev->SetTexture(0, NULL);

    	m_pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, abe);
        m_pD3DDev->SetRenderState(D3DRS_SRCBLEND, sb);
        m_pD3DDev->SetRenderState(D3DRS_DESTBLEND, db);

		return S_OK;
    }
	while(0);

    return E_FAIL;
}


STDMETHODIMP_(bool) CDX9AllocatorPresenter::Paint(bool fAll)
{
	AppSettings& s = AfxGetAppSettings();

	CAutoLock cAutoLock(this);

	if(m_WindowRect.right <= m_WindowRect.left || m_WindowRect.bottom <= m_WindowRect.top
	|| m_NativeVideoSize.cx <= 0 || m_NativeVideoSize.cy <= 0
	|| !m_pVideoSurface)
		return(false);

	HRESULT hr;

	CRect rSrcVid(CPoint(0, 0), m_NativeVideoSize);
	CRect rDstVid(m_VideoRect);

	CRect rSrcPri(CPoint(0, 0), m_WindowRect.Size());
	CRect rDstPri(m_WindowRect);

	CComPtr<IDirect3DSurface9> pBackBuffer;
	m_pD3DDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);

	m_pD3DDev->SetRenderTarget(0, pBackBuffer);

	if(fAll)
	{
		// clear the backbuffer

		hr = m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);

		// paint the video on the backbuffer

		if(!rDstVid.IsRectEmpty())
		{
			if(m_pVideoTexture[m_nCurPicture])
			{
				CComPtr<IDirect3DTexture9> pVideoTexture = m_pVideoTexture[m_nCurPicture];

				if(m_pVideoTexture[m_nPictureSlots] && m_pVideoTexture[m_nPictureSlots+1] && !m_pPixelShaders.IsEmpty())
				{
					static __int64 counter = 0;
					static long start = clock();

					long stop = clock();
					long diff = stop - start;

					if(diff >= 10*60*CLOCKS_PER_SEC) start = stop; // reset after 10 min (ps float has its limits in both range and accuracy)

					float fConstData[][4] = 
					{
						{(float)m_NativeVideoSize.cx, (float)m_NativeVideoSize.cy, (float)(counter++), (float)diff / CLOCKS_PER_SEC},
						{1.0f / m_NativeVideoSize.cx, 1.0f / m_NativeVideoSize.cy, 0, 0},
					};

					hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));

					CComPtr<IDirect3DSurface9> pRT;
					hr = m_pD3DDev->GetRenderTarget(0, &pRT);

					int src = m_nCurPicture, dst = m_nPictureSlots;

					POSITION pos = m_pPixelShaders.GetHeadPosition();
					while(pos)
					{
						pVideoTexture = m_pVideoTexture[dst];

						hr = m_pD3DDev->SetRenderTarget(0, m_pVideoSurface[dst]);
						hr = m_pD3DDev->SetPixelShader(m_pPixelShaders.GetNext(pos));
						TextureCopy(m_pVideoTexture[src]);

						//if(++src > 2) src = 1;
						//if(++dst > 2) dst = 1;
						src		= dst;
						if(++dst >= m_nPictureSlots+2) dst = m_nPictureSlots;
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
					case 3: A = -0.60f; break;
					case 4: A = -0.75f; break;
					case 5: A = -1.00f; break;
				}

				hr = InitResizers(A);

				if(iDX9Resizer == 0 || iDX9Resizer == 1 || rSrcVid.Size() == rDstVid.Size() || FAILED(hr))
				{
					hr = TextureResize(pVideoTexture, dst, iDX9Resizer == 0 ? D3DTEXF_POINT : D3DTEXF_LINEAR);
				}
				else if(iDX9Resizer == 2)
				{
					hr = TextureResizeBilinear(pVideoTexture, dst);
				}
				else if(iDX9Resizer >= 3)
				{
					hr = TextureResizeBicubic2pass(pVideoTexture, dst);
				}

			}
			else
			{
				if(pBackBuffer)
				{
					ClipToSurface(pBackBuffer, rSrcVid, rDstVid); // grrr
					// IMPORTANT: rSrcVid has to be aligned on mod2 for yuy2->rgb conversion with StretchRect!!!
					rSrcVid.left &= ~1; rSrcVid.right &= ~1;
					rSrcVid.top &= ~1; rSrcVid.bottom &= ~1;
					hr = m_pD3DDev->StretchRect(m_pVideoSurface[m_nCurPicture], rSrcVid, pBackBuffer, rDstVid, m_filter);

					// Support ffdshow queueing
					// m_pD3DDev->StretchRect may fail if ffdshow is using queue output samples.
					// Here we don't want to show the black buffer.
					if(FAILED(hr)) return false;
				}
			}
		}

		// paint the text on the backbuffer

		AlphaBltSubPic(rSrcPri.Size());
	}

	// Casimir666 : affichage de l'OSD
	if (m_VMR9AlphaBitmap.dwFlags & VMRBITMAP_UPDATE)
	{
		CRect		rcSrc (m_VMR9AlphaBitmap.rSrc);
		m_pOSDTexture	= NULL;
		m_pOSDSurface	= NULL;
		if ((m_VMR9AlphaBitmap.dwFlags & VMRBITMAP_DISABLE) == 0)
		{
			if(SUCCEEDED(hr = m_pD3DDev->CreateTexture(rcSrc.Width(), rcSrc.Height(), 1, 
												D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
												D3DPOOL_DEFAULT, &m_pOSDTexture, NULL)))
			{
				if (SUCCEEDED (hr = m_pOSDTexture->GetSurfaceLevel(0, &m_pOSDSurface)))
				{
					HBITMAP			hBitmap = (HBITMAP)GetCurrentObject (m_VMR9AlphaBitmap.hdc, OBJ_BITMAP);
					DIBSECTION		info = {0};

					::GetObject(hBitmap, sizeof( DIBSECTION ), &info );
					CRect		rcBitmap (0, 0, info.dsBm.bmWidth, info.dsBm.bmHeight);

					if (m_pD3DXLoadSurfaceFromMemory)
						hr = m_pD3DXLoadSurfaceFromMemory (m_pOSDSurface,
													NULL,
													NULL,
													info.dsBm.bmBits,
													D3DFMT_A8R8G8B8,
													info.dsBm.bmWidthBytes,
													NULL,
													&rcBitmap,
													D3DX_FILTER_NONE,
													m_VMR9AlphaBitmap.clrSrcKey);
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

	if (AfxGetMyApp()->m_fDisplayStats) DrawStats();

	if (m_pOSDTexture) AlphaBlt(rSrcPri, rDstPri, m_pOSDTexture);

	if(m_fVMRSyncFix)
	{
		D3DLOCKED_RECT lr;
		if(SUCCEEDED(pBackBuffer->LockRect(&lr, NULL, 0)))
			pBackBuffer->UnlockRect();
	}

	if ((AfxGetApp()->m_pMainWnd != NULL) && (((CMainFrame*)AfxGetApp()->m_pMainWnd)->IsD3DFullScreenMode()) )
		hr = m_pD3DDev->Present(NULL, NULL, NULL, NULL);
	else
		hr = m_pD3DDev->Present(rSrcPri, rDstPri, NULL, NULL);

	// Calculate the jitter!
	LONGLONG	llPerf = AfxGetMyApp()->GetPerfCounter();
	if ((m_rtTimePerFrame != 0) && (labs (llPerf - m_llLastPerf) < m_rtTimePerFrame/2))
	{
		m_pllJitter[m_nNextJitter] = llPerf - m_llLastPerf - m_rtTimePerFrame/10;
		m_nNextJitter = (m_nNextJitter+1) % NB_JITTER;
	}
	m_llLastPerf = llPerf;

	bool fResetDevice = false;

	if(hr == D3DERR_DEVICELOST && m_pD3DDev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		fResetDevice = true;
	}

	D3DDEVICE_CREATION_PARAMETERS Parameters;
	if(SUCCEEDED(m_pD3DDev->GetCreationParameters(&Parameters))
	&& m_pD3D->GetAdapterMonitor(Parameters.AdapterOrdinal) != m_pD3D->GetAdapterMonitor(GetAdapter(m_pD3D)))
	{
		fResetDevice = true;
	}

	if(fResetDevice)
	{
		DeleteSurfaces();
		if(FAILED(hr = CreateDevice()) || FAILED(hr = AllocSurfaces()))
			return false;
		OnResetDevice();
	}

	return(true);
}


void CDX9AllocatorPresenter::DrawStats()
{
	if (m_pLine && m_pFont)
	{
		D3DXVECTOR2		Points[NB_JITTER];
		int				nIndex;
		RECT			rc = {700, 40, 0, 0 };

		// === Jitter Graduation
		m_pLine->SetWidth(1.0);          // Width 
		for (int i=10; i<500; i+= 20)
		{
			Points[0].x = 0;
			Points[0].y = i;
			Points[1].x = (i-10)%80 ? 50 : 625;
			Points[1].y = i;
			if (i == 250) Points[1].x += 50;
			m_pLine->SetWidth(i == 250 ? 2.0 : 1.0);          // Width 
			m_pLine->Begin();
			m_pLine->Draw (Points, 2, D3DCOLOR_XRGB(0,0,255));
			m_pLine->End();
		}

		// === Jitter curve
		if (m_rtTimePerFrame)
		{
			for (int i=0; i<NB_JITTER; i++)
			{
				nIndex = (m_nNextJitter+i) % NB_JITTER;
				Points[i].x = i*5+5;
				Points[i].y = m_pllJitter[nIndex]/500 + 250;
			}		
			m_pLine->Begin();
			m_pLine->Draw (Points, NB_JITTER, D3DCOLOR_XRGB(255,0,0));
			m_pLine->End();
		}

		// === Text
		CString		strText;
		strText.Format(L"Frame rate : %.03f  (%I64d µs)", m_fps, m_rtTimePerFrame / 10);
		m_pFont->DrawText( NULL, strText, -1, &rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ));

		OffsetRect (&rc, 0, 30);
		strText.Format(L"Candidate frame rate : %.03f  (%I64d µs)", 10000000.0 / m_rtCandidate, m_rtCandidate / 10);
//		m_pFont->DrawText( NULL, strText, -1, &rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ));
	}
}

STDMETHODIMP CDX9AllocatorPresenter::GetDIB(BYTE* lpDib, DWORD* size)
{
	CheckPointer(size, E_POINTER);

	HRESULT hr;

	D3DSURFACE_DESC desc;
	memset(&desc, 0, sizeof(desc));
	m_pVideoSurface[m_nCurPicture]->GetDesc(&desc);

	DWORD required = sizeof(BITMAPINFOHEADER) + (desc.Width * desc.Height * 32 >> 3);
	if(!lpDib) {*size = required; return S_OK;}
	if(*size < required) return E_OUTOFMEMORY;
	*size = required;

	CComPtr<IDirect3DSurface9> pSurface = m_pVideoSurface[m_nCurPicture];
	D3DLOCKED_RECT r;
	if(FAILED(hr = pSurface->LockRect(&r, NULL, D3DLOCK_READONLY)))
	{
		pSurface = NULL;
		if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &pSurface, NULL))
		|| FAILED(hr = m_pD3DDev->GetRenderTargetData(m_pVideoSurface[m_nCurPicture], pSurface))
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

STDMETHODIMP CDX9AllocatorPresenter::SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget)
{
	CAutoLock cAutoLock(this);

	if(!pSrcData && !pTarget)
	{
		m_pPixelShaders.RemoveAll();
		m_pD3DDev->SetPixelShader(NULL);
		return S_OK;
	}

	if(!pSrcData || !pTarget)
		return E_INVALIDARG;

	CComPtr<IDirect3DPixelShader9> pPixelShader;

	HRESULT hr = m_pPSC->CompileShader(pSrcData, "main", pTarget, 0, &pPixelShader);
	if(FAILED(hr)) return hr;

	m_pPixelShaders.AddTail(pPixelShader);

	Paint(true);

	return S_OK;
}

void CDX9AllocatorPresenter::EstimateFrameRate (REFERENCE_TIME rtStart)
{
	static REFERENCE_TIME	rtLast = 0;
	static int				nCount	= 0;

	REFERENCE_TIME		rtCurDuration = rtStart - rtLast;

	if (labs (rtCurDuration - m_rtCandidate) <= 20000)
	{
		if (nCount <= 6) nCount++;
		if (nCount == 5)
		{
			if ( labs(rtCurDuration - 417080) < FRAMERATE_MAX_DELTA)
			{
				m_rtTimePerFrame	= 417080;
				m_fps				= 23.976;
			}
			else if ( labs(rtCurDuration - 400000) < FRAMERATE_MAX_DELTA)
			{
				m_rtTimePerFrame	= 400000;
				m_fps				= 25.000;
			}
			else if ( labs(rtCurDuration - 333600) < FRAMERATE_MAX_DELTA)
			{
				m_rtTimePerFrame	= 333600;
				m_fps				= 29.976;
			}
			else
			{
				m_rtTimePerFrame = rtCurDuration;
				m_fps = 10000000.0 / m_rtTimePerFrame;
			}
		}
	}
	else
	{
		nCount = max (nCount-1, 0);
		if (nCount == 0) m_rtCandidate = rtCurDuration;
	}

	rtLast = rtStart;
}

//
// CVMR9AllocatorPresenter
//

#define MY_USER_ID 0x6ABE51

CVMR9AllocatorPresenter::CVMR9AllocatorPresenter(HWND hWnd, HRESULT& hr) 
	: CDX9AllocatorPresenter(hWnd, hr)
	, m_fUseInternalTimer(false)
	, m_rtPrevStart(-1)
{
}

STDMETHODIMP CVMR9AllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

	return 
		QI(IVMRSurfaceAllocator9)
		QI(IVMRImagePresenter9)
		QI(IVMRWindowlessControl9)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CVMR9AllocatorPresenter::CreateDevice()
{
	HRESULT hr = __super::CreateDevice();
	if(FAILED(hr)) return hr;

	if(m_pIVMRSurfAllocNotify)
	{
		HMONITOR hMonitor = m_pD3D->GetAdapterMonitor(GetAdapter(m_pD3D));
		if(FAILED(hr = m_pIVMRSurfAllocNotify->ChangeD3DDevice(m_pD3DDev, hMonitor)))
			return(false);
	}

	return hr;
}

void CVMR9AllocatorPresenter::DeleteSurfaces()
{
    CAutoLock cAutoLock(this);

	m_pSurfaces.RemoveAll();

	return __super::DeleteSurfaces();
}

// ISubPicAllocatorPresenter

class COuterVMR9
	: public CUnknown
	, public IVideoWindow
	, public IBasicVideo2
	, public IVMRWindowlessControl
	, public IVMRffdshow9
	, public IVMRMixerBitmap9
{
	CComPtr<IUnknown>	m_pVMR;
	VMR9AlphaBitmap*	m_pVMR9AlphaBitmap;

public:

	COuterVMR9(const TCHAR* pName, LPUNKNOWN pUnk, VMR9AlphaBitmap* pVMR9AlphaBitmap) : CUnknown(pName, pUnk)
	{
		m_pVMR.CoCreateInstance(CLSID_VideoMixingRenderer9, GetOwner());
		m_pVMR9AlphaBitmap = pVMR9AlphaBitmap;
	}

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv)
	{
		HRESULT hr;

		// Casimir666 : en mode Renderless faire l'incrustation à la place du VMR
		if(riid == __uuidof(IVMRMixerBitmap9))
			return GetInterface((IVMRMixerBitmap9*)this, ppv);

		hr = m_pVMR ? m_pVMR->QueryInterface(riid, ppv) : E_NOINTERFACE;
		if(m_pVMR && FAILED(hr))
		{
			if(riid == __uuidof(IVideoWindow))
				return GetInterface((IVideoWindow*)this, ppv);
			if(riid == __uuidof(IBasicVideo))
				return GetInterface((IBasicVideo*)this, ppv);
			if(riid == __uuidof(IBasicVideo2))
				return GetInterface((IBasicVideo2*)this, ppv);
			if(riid == __uuidof(IVMRffdshow9)) // Support ffdshow queueing. We show ffdshow that this is patched Media Player Classic.
				return GetInterface((IVMRffdshow9*)this, ppv);
/*			if(riid == __uuidof(IVMRWindowlessControl))
				return GetInterface((IVMRWindowlessControl*)this, ppv);
*/
		}

		return SUCCEEDED(hr) ? hr : __super::NonDelegatingQueryInterface(riid, ppv);
	}

	// IVMRWindowlessControl

	STDMETHODIMP GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			return pWC9->GetNativeVideoSize(lpWidth, lpHeight, lpARWidth, lpARHeight);
		}

		return E_NOTIMPL;
	}
	STDMETHODIMP GetMinIdealVideoSize(LONG* lpWidth, LONG* lpHeight) {return E_NOTIMPL;}
	STDMETHODIMP GetMaxIdealVideoSize(LONG* lpWidth, LONG* lpHeight) {return E_NOTIMPL;}
	STDMETHODIMP SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect) {return E_NOTIMPL;}
    STDMETHODIMP GetVideoPosition(LPRECT lpSRCRect, LPRECT lpDSTRect)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			return pWC9->GetVideoPosition(lpSRCRect, lpDSTRect);
		}

		return E_NOTIMPL;
	}
	STDMETHODIMP GetAspectRatioMode(DWORD* lpAspectRatioMode)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			*lpAspectRatioMode = VMR_ARMODE_NONE;
			return S_OK;
		}

		return E_NOTIMPL;
	}
	STDMETHODIMP SetAspectRatioMode(DWORD AspectRatioMode) {return E_NOTIMPL;}
	STDMETHODIMP SetVideoClippingWindow(HWND hwnd) {return E_NOTIMPL;}
	STDMETHODIMP RepaintVideo(HWND hwnd, HDC hdc) {return E_NOTIMPL;}
	STDMETHODIMP DisplayModeChanged() {return E_NOTIMPL;}
	STDMETHODIMP GetCurrentImage(BYTE** lpDib) {return E_NOTIMPL;}
	STDMETHODIMP SetBorderColor(COLORREF Clr) {return E_NOTIMPL;}
	STDMETHODIMP GetBorderColor(COLORREF* lpClr) {return E_NOTIMPL;}
	STDMETHODIMP SetColorKey(COLORREF Clr) {return E_NOTIMPL;}
	STDMETHODIMP GetColorKey(COLORREF* lpClr) {return E_NOTIMPL;}

	// IVideoWindow
	STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) {return E_NOTIMPL;}
	STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) {return E_NOTIMPL;}
	STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {return E_NOTIMPL;}
	STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {return E_NOTIMPL;}
    STDMETHODIMP put_Caption(BSTR strCaption) {return E_NOTIMPL;}
    STDMETHODIMP get_Caption(BSTR* strCaption) {return E_NOTIMPL;}
	STDMETHODIMP put_WindowStyle(long WindowStyle) {return E_NOTIMPL;}
	STDMETHODIMP get_WindowStyle(long* WindowStyle) {return E_NOTIMPL;}
	STDMETHODIMP put_WindowStyleEx(long WindowStyleEx) {return E_NOTIMPL;}
	STDMETHODIMP get_WindowStyleEx(long* WindowStyleEx) {return E_NOTIMPL;}
	STDMETHODIMP put_AutoShow(long AutoShow) {return E_NOTIMPL;}
	STDMETHODIMP get_AutoShow(long* AutoShow) {return E_NOTIMPL;}
	STDMETHODIMP put_WindowState(long WindowState) {return E_NOTIMPL;}
	STDMETHODIMP get_WindowState(long* WindowState) {return E_NOTIMPL;}
	STDMETHODIMP put_BackgroundPalette(long BackgroundPalette) {return E_NOTIMPL;}
	STDMETHODIMP get_BackgroundPalette(long* pBackgroundPalette) {return E_NOTIMPL;}
	STDMETHODIMP put_Visible(long Visible) {return E_NOTIMPL;}
	STDMETHODIMP get_Visible(long* pVisible) {return E_NOTIMPL;}
	STDMETHODIMP put_Left(long Left) {return E_NOTIMPL;}
	STDMETHODIMP get_Left(long* pLeft) {return E_NOTIMPL;}
	STDMETHODIMP put_Width(long Width) {return E_NOTIMPL;}
	STDMETHODIMP get_Width(long* pWidth)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			CRect s, d;
			HRESULT hr = pWC9->GetVideoPosition(&s, &d);
			*pWidth = d.Width();
			return hr;
		}

		return E_NOTIMPL;
	}
	STDMETHODIMP put_Top(long Top) {return E_NOTIMPL;}
	STDMETHODIMP get_Top(long* pTop) {return E_NOTIMPL;}
	STDMETHODIMP put_Height(long Height) {return E_NOTIMPL;}
	STDMETHODIMP get_Height(long* pHeight)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			CRect s, d;
			HRESULT hr = pWC9->GetVideoPosition(&s, &d);
			*pHeight = d.Height();
			return hr;
		}

		return E_NOTIMPL;
	}
	STDMETHODIMP put_Owner(OAHWND Owner) {return E_NOTIMPL;}
	STDMETHODIMP get_Owner(OAHWND* Owner) {return E_NOTIMPL;}
	STDMETHODIMP put_MessageDrain(OAHWND Drain) {return E_NOTIMPL;}
	STDMETHODIMP get_MessageDrain(OAHWND* Drain) {return E_NOTIMPL;}
	STDMETHODIMP get_BorderColor(long* Color) {return E_NOTIMPL;}
	STDMETHODIMP put_BorderColor(long Color) {return E_NOTIMPL;}
	STDMETHODIMP get_FullScreenMode(long* FullScreenMode) {return E_NOTIMPL;}
	STDMETHODIMP put_FullScreenMode(long FullScreenMode) {return E_NOTIMPL;}
    STDMETHODIMP SetWindowForeground(long Focus) {return E_NOTIMPL;}
    STDMETHODIMP NotifyOwnerMessage(OAHWND hwnd, long uMsg, LONG_PTR wParam, LONG_PTR lParam) {return E_NOTIMPL;}
    STDMETHODIMP SetWindowPosition(long Left, long Top, long Width, long Height) {return E_NOTIMPL;}
	STDMETHODIMP GetWindowPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight) {return E_NOTIMPL;}
	STDMETHODIMP GetMinIdealImageSize(long* pWidth, long* pHeight) {return E_NOTIMPL;}
	STDMETHODIMP GetMaxIdealImageSize(long* pWidth, long* pHeight) {return E_NOTIMPL;}
	STDMETHODIMP GetRestorePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight) {return E_NOTIMPL;}
	STDMETHODIMP HideCursor(long HideCursor) {return E_NOTIMPL;}
	STDMETHODIMP IsCursorHidden(long* CursorHidden) {return E_NOTIMPL;}

	// IBasicVideo2
    STDMETHODIMP get_AvgTimePerFrame(REFTIME* pAvgTimePerFrame) {return E_NOTIMPL;}
    STDMETHODIMP get_BitRate(long* pBitRate) {return E_NOTIMPL;}
    STDMETHODIMP get_BitErrorRate(long* pBitErrorRate) {return E_NOTIMPL;}
    STDMETHODIMP get_VideoWidth(long* pVideoWidth) {return E_NOTIMPL;}
    STDMETHODIMP get_VideoHeight(long* pVideoHeight) {return E_NOTIMPL;}
    STDMETHODIMP put_SourceLeft(long SourceLeft) {return E_NOTIMPL;}
    STDMETHODIMP get_SourceLeft(long* pSourceLeft) {return E_NOTIMPL;}
    STDMETHODIMP put_SourceWidth(long SourceWidth) {return E_NOTIMPL;}
    STDMETHODIMP get_SourceWidth(long* pSourceWidth) {return E_NOTIMPL;}
    STDMETHODIMP put_SourceTop(long SourceTop) {return E_NOTIMPL;}
    STDMETHODIMP get_SourceTop(long* pSourceTop) {return E_NOTIMPL;}
    STDMETHODIMP put_SourceHeight(long SourceHeight) {return E_NOTIMPL;}
    STDMETHODIMP get_SourceHeight(long* pSourceHeight) {return E_NOTIMPL;}
    STDMETHODIMP put_DestinationLeft(long DestinationLeft) {return E_NOTIMPL;}
    STDMETHODIMP get_DestinationLeft(long* pDestinationLeft) {return E_NOTIMPL;}
    STDMETHODIMP put_DestinationWidth(long DestinationWidth) {return E_NOTIMPL;}
    STDMETHODIMP get_DestinationWidth(long* pDestinationWidth) {return E_NOTIMPL;}
    STDMETHODIMP put_DestinationTop(long DestinationTop) {return E_NOTIMPL;}
    STDMETHODIMP get_DestinationTop(long* pDestinationTop) {return E_NOTIMPL;}
    STDMETHODIMP put_DestinationHeight(long DestinationHeight) {return E_NOTIMPL;}
    STDMETHODIMP get_DestinationHeight(long* pDestinationHeight) {return E_NOTIMPL;}
    STDMETHODIMP SetSourcePosition(long Left, long Top, long Width, long Height) {return E_NOTIMPL;}
    STDMETHODIMP GetSourcePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight)
	{
		// DVD Nav. bug workaround fix
		{
			*pLeft = *pTop = 0;
			return GetVideoSize(pWidth, pHeight);
		}
/*
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			CRect s, d;
			HRESULT hr = pWC9->GetVideoPosition(&s, &d);
			*pLeft = s.left;
			*pTop = s.top;
			*pWidth = s.Width();
			*pHeight = s.Height();
			return hr;
		}
*/
		return E_NOTIMPL;
	}
    STDMETHODIMP SetDefaultSourcePosition() {return E_NOTIMPL;}
    STDMETHODIMP SetDestinationPosition(long Left, long Top, long Width, long Height) {return E_NOTIMPL;}
    STDMETHODIMP GetDestinationPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			CRect s, d;
			HRESULT hr = pWC9->GetVideoPosition(&s, &d);
			*pLeft = d.left;
			*pTop = d.top;
			*pWidth = d.Width();
			*pHeight = d.Height();
			return hr;
		}

		return E_NOTIMPL;
	}
    STDMETHODIMP SetDefaultDestinationPosition() {return E_NOTIMPL;}
    STDMETHODIMP GetVideoSize(long* pWidth, long* pHeight)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			LONG aw, ah;
//			return pWC9->GetNativeVideoSize(pWidth, pHeight, &aw, &ah);
			// DVD Nav. bug workaround fix
			HRESULT hr = pWC9->GetNativeVideoSize(pWidth, pHeight, &aw, &ah);
			*pWidth = *pHeight * aw / ah;
			return hr;
		}

		return E_NOTIMPL;
	}
	// IVMRffdshow9
	STDMETHODIMP support_ffdshow()
	{
		queueu_ffdshow_support = true;
		return S_OK;
	}

    STDMETHODIMP GetVideoPaletteEntries(long StartIndex, long Entries, long* pRetrieved, long* pPalette) {return E_NOTIMPL;}
    STDMETHODIMP GetCurrentImage(long* pBufferSize, long* pDIBImage) {return E_NOTIMPL;}
    STDMETHODIMP IsUsingDefaultSource() {return E_NOTIMPL;}
    STDMETHODIMP IsUsingDefaultDestination() {return E_NOTIMPL;}

	STDMETHODIMP GetPreferredAspectRatio(long* plAspectX, long* plAspectY)
	{
		if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
		{
			LONG w, h;
			return pWC9->GetNativeVideoSize(&w, &h, plAspectX, plAspectY);
		}

		return E_NOTIMPL;
	}

	// IVMRMixerBitmap9
	STDMETHODIMP GetAlphaBitmapParameters(VMR9AlphaBitmap* pBmpParms)
	{
		CheckPointer(pBmpParms, E_POINTER);
		memcpy (pBmpParms, m_pVMR9AlphaBitmap, sizeof(VMR9AlphaBitmap));
		return S_OK;
	}
	
	STDMETHODIMP SetAlphaBitmap(const VMR9AlphaBitmap*  pBmpParms)
	{
		CheckPointer(pBmpParms, E_POINTER);
		memcpy (m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
		m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
		return S_OK;
	}

	STDMETHODIMP UpdateAlphaBitmapParameters(const VMR9AlphaBitmap* pBmpParms)
	{
		CheckPointer(pBmpParms, E_POINTER);
		memcpy (m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
		m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
		return S_OK;
	}
};

STDMETHODIMP CVMR9AllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
    CheckPointer(ppRenderer, E_POINTER);

	*ppRenderer = NULL;

	HRESULT hr;

	do
	{
		CMacrovisionKicker* pMK = new CMacrovisionKicker(NAME("CMacrovisionKicker"), NULL);
		CComPtr<IUnknown> pUnk = (IUnknown*)(INonDelegatingUnknown*)pMK;
		pMK->SetInner((IUnknown*)(INonDelegatingUnknown*)new COuterVMR9(NAME("COuterVMR9"), pUnk, &m_VMR9AlphaBitmap));
		CComQIPtr<IBaseFilter> pBF = pUnk;
/*
		CComQIPtr<IBaseFilter> pBF = (IUnknown*)(INonDelegatingUnknown*)new COuterVMR9(NAME("COuterVMR9"), NULL);
		if(!pBF) pBF.CoCreateInstance(CLSID_VideoMixingRenderer9);
*/

		CComPtr<IPin> pPin = GetFirstPin(pBF);
		CComQIPtr<IMemInputPin> pMemInputPin = pPin;
		m_fUseInternalTimer = HookNewSegmentAndReceive((IPinC*)(IPin*)pPin, (IMemInputPinC*)(IMemInputPin*)pMemInputPin);
/*
if(CComQIPtr<IAMVideoAccelerator> pAMVA = pPin)
	HookAMVideoAccelerator((IAMVideoAcceleratorC*)(IAMVideoAccelerator*)pAMVA);
*/
		CComQIPtr<IVMRFilterConfig9> pConfig = pBF;
		if(!pConfig)
			break;

		AppSettings& s = AfxGetAppSettings();

		if(s.fVMR9MixerMode)
		{
			if(FAILED(hr = pConfig->SetNumberOfStreams(1)))
				break;

			if(s.fVMR9MixerYUV)
			{
				if(CComQIPtr<IVMRMixerControl9> pMC = pBF)
				{
					DWORD dwPrefs;
					pMC->GetMixingPrefs(&dwPrefs);  
					dwPrefs &= ~MixerPref9_RenderTargetMask; 
					dwPrefs |= MixerPref9_RenderTargetYUV;
					pMC->SetMixingPrefs(dwPrefs);
				}
			}
		}

		if(FAILED(hr = pConfig->SetRenderingMode(VMR9Mode_Renderless)))
			break;

		CComQIPtr<IVMRSurfaceAllocatorNotify9> pSAN = pBF;
		if(!pSAN)
			break;

		if(FAILED(hr = pSAN->AdviseSurfaceAllocator(MY_USER_ID, static_cast<IVMRSurfaceAllocator9*>(this)))
		|| FAILED(hr = AdviseNotify(pSAN)))
			break;

		*ppRenderer = (IUnknown*)pBF.Detach();

		return S_OK;
	}
	while(0);

    return E_FAIL;
}

STDMETHODIMP_(void) CVMR9AllocatorPresenter::SetTime(REFERENCE_TIME rtNow)
{
	__super::SetTime(rtNow);
	m_fUseInternalTimer = false;
}

// IVMRSurfaceAllocator9

STDMETHODIMP CVMR9AllocatorPresenter::InitializeDevice(DWORD_PTR dwUserID, VMR9AllocationInfo* lpAllocInfo, DWORD* lpNumBuffers)
{
	if(!lpAllocInfo || !lpNumBuffers)
		return E_POINTER;

	if(!m_pIVMRSurfAllocNotify)
		return E_FAIL;

	if((GetAsyncKeyState(VK_CONTROL)&0x80000000))
	if(lpAllocInfo->Format == '21VY' || lpAllocInfo->Format == '024I')
		return E_FAIL;

	DeleteSurfaces();

	m_pSurfaces.SetCount(*lpNumBuffers);

	int w = lpAllocInfo->dwWidth;
	int h = abs((int)lpAllocInfo->dwHeight);

	HRESULT hr;

	if(lpAllocInfo->dwFlags & VMR9AllocFlag_3DRenderTarget)
		lpAllocInfo->dwFlags |= VMR9AllocFlag_TextureSurface;

	hr = m_pIVMRSurfAllocNotify->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, &m_pSurfaces[0]);
	if(FAILED(hr)) return hr;

	m_NativeVideoSize = m_AspectRatio = CSize(w, h);
	int arx = lpAllocInfo->szAspectRatio.cx, ary = lpAllocInfo->szAspectRatio.cy;
	if(arx > 0 && ary > 0) m_AspectRatio.SetSize(arx, ary);

	if(FAILED(hr = AllocSurfaces()))
		return hr;

	if(!(lpAllocInfo->dwFlags & VMR9AllocFlag_TextureSurface))
	{
		// test if the colorspace is acceptable
		if(FAILED(hr = m_pD3DDev->StretchRect(m_pSurfaces[0], NULL, m_pVideoSurface[m_nCurPicture], NULL, D3DTEXF_NONE)))
		{
			DeleteSurfaces();
			return E_FAIL;
		}
	}

	hr = m_pD3DDev->ColorFill(m_pVideoSurface[m_nCurPicture], NULL, 0);

	return hr;
}

STDMETHODIMP CVMR9AllocatorPresenter::TerminateDevice(DWORD_PTR dwUserID)
{
    DeleteSurfaces();
    return S_OK;
}

STDMETHODIMP CVMR9AllocatorPresenter::GetSurface(DWORD_PTR dwUserID, DWORD SurfaceIndex, DWORD SurfaceFlags, IDirect3DSurface9** lplpSurface)
{
    if(!lplpSurface)
		return E_POINTER;

	if(SurfaceIndex >= m_pSurfaces.GetCount()) 
        return E_FAIL;

    CAutoLock cAutoLock(this);

	(*lplpSurface = m_pSurfaces[SurfaceIndex])->AddRef();

	return S_OK;
}

STDMETHODIMP CVMR9AllocatorPresenter::AdviseNotify(IVMRSurfaceAllocatorNotify9* lpIVMRSurfAllocNotify)
{
    CAutoLock cAutoLock(this);
	
	m_pIVMRSurfAllocNotify = lpIVMRSurfAllocNotify;

	HRESULT hr;
    HMONITOR hMonitor = m_pD3D->GetAdapterMonitor(GetAdapter(m_pD3D));
    if(FAILED(hr = m_pIVMRSurfAllocNotify->SetD3DDevice(m_pD3DDev, hMonitor)))
		return hr;

    return S_OK;
}

// IVMRImagePresenter9

STDMETHODIMP CVMR9AllocatorPresenter::StartPresenting(DWORD_PTR dwUserID)
{
    CAutoLock cAutoLock(this);

    ASSERT(m_pD3DDev);

	return m_pD3DDev ? S_OK : E_FAIL;
}

STDMETHODIMP CVMR9AllocatorPresenter::StopPresenting(DWORD_PTR dwUserID)
{
	return S_OK;
}


STDMETHODIMP CVMR9AllocatorPresenter::PresentImage(DWORD_PTR dwUserID, VMR9PresentationInfo* lpPresInfo)
{
	CheckPointer(m_pIVMRSurfAllocNotify, E_UNEXPECTED);

    HRESULT hr;

	if(!lpPresInfo || !lpPresInfo->lpSurf)
		return E_POINTER;

	CAutoLock cAutoLock(this);

	CComPtr<IDirect3DTexture9> pTexture;
	lpPresInfo->lpSurf->GetContainer(IID_IDirect3DTexture9, (void**)&pTexture);

	if(pTexture)
	{
		m_pVideoSurface[m_nCurPicture] = lpPresInfo->lpSurf;
		if(m_pVideoTexture[m_nCurPicture]) m_pVideoTexture[m_nCurPicture] = pTexture;
	}
	else
	{
		hr = m_pD3DDev->StretchRect(lpPresInfo->lpSurf, NULL, m_pVideoSurface[m_nCurPicture], NULL, D3DTEXF_NONE);
	}

	if(lpPresInfo->rtEnd > lpPresInfo->rtStart)
	{
		EstimateFrameRate (lpPresInfo->rtStart);

		if(m_pSubPicQueue)
		{
			m_pSubPicQueue->SetFPS(m_fps);

			if(m_fUseInternalTimer)
			{
				__super::SetTime(g_tSegmentStart + g_tSampleStart);
			}
		}
	}

	CSize VideoSize = m_NativeVideoSize;
	int arx = lpPresInfo->szAspectRatio.cx, ary = lpPresInfo->szAspectRatio.cy;
	if(arx > 0 && ary > 0) VideoSize.cx = VideoSize.cy*arx/ary;
	if(VideoSize != GetVideoSize())
	{
		m_AspectRatio.SetSize(arx, ary);
		AfxGetApp()->m_pMainWnd->PostMessage(WM_REARRANGERENDERLESS);
	}

	// Tear test bars
	if (AfxGetMyApp()->m_fTearingTest)
	{
		RECT		rcTearing;
		
		rcTearing.left		= m_nTearingPos;
		rcTearing.top		= 0;
		rcTearing.right		= rcTearing.left + 4;
		rcTearing.bottom	= m_NativeVideoSize.cy;
		m_pD3DDev->ColorFill (m_pVideoSurface[m_nCurPicture], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

		rcTearing.left	= (rcTearing.right + 15) % m_NativeVideoSize.cx;
		rcTearing.right	= rcTearing.left + 4;
		m_pD3DDev->ColorFill (m_pVideoSurface[m_nCurPicture], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

		m_nTearingPos = (m_nTearingPos + 7) % m_NativeVideoSize.cx;
	}

	Paint(true);

    return S_OK;
}

// IVMRWindowlessControl9
//
// It is only implemented (partially) for the dvd navigator's 
// menu handling, which needs to know a few things about the 
// location of our window.

STDMETHODIMP CVMR9AllocatorPresenter::GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
{
	if(lpWidth) *lpWidth = m_NativeVideoSize.cx;
	if(lpHeight) *lpHeight = m_NativeVideoSize.cy;
	if(lpARWidth) *lpARWidth = m_AspectRatio.cx;
	if(lpARHeight) *lpARHeight = m_AspectRatio.cy;
	return S_OK;
}
STDMETHODIMP CVMR9AllocatorPresenter::GetMinIdealVideoSize(LONG* lpWidth, LONG* lpHeight) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::GetMaxIdealVideoSize(LONG* lpWidth, LONG* lpHeight) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect) {return E_NOTIMPL;} // we have our own method for this
STDMETHODIMP CVMR9AllocatorPresenter::GetVideoPosition(LPRECT lpSRCRect, LPRECT lpDSTRect)
{
	CopyRect(lpSRCRect, CRect(CPoint(0, 0), m_NativeVideoSize));
	CopyRect(lpDSTRect, &m_VideoRect);
	return S_OK;
}
STDMETHODIMP CVMR9AllocatorPresenter::GetAspectRatioMode(DWORD* lpAspectRatioMode)
{
	if(lpAspectRatioMode) *lpAspectRatioMode = AM_ARMODE_STRETCHED;
	return S_OK;
}
STDMETHODIMP CVMR9AllocatorPresenter::SetAspectRatioMode(DWORD AspectRatioMode) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::SetVideoClippingWindow(HWND hwnd) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::RepaintVideo(HWND hwnd, HDC hdc) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::DisplayModeChanged() {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::GetCurrentImage(BYTE** lpDib) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::SetBorderColor(COLORREF Clr) {return E_NOTIMPL;}
STDMETHODIMP CVMR9AllocatorPresenter::GetBorderColor(COLORREF* lpClr)
{
	if(lpClr) *lpClr = 0;
	return S_OK;
}

//
// CRM9AllocatorPresenter
//

CRM9AllocatorPresenter::CRM9AllocatorPresenter(HWND hWnd, HRESULT& hr) 
	: CDX9AllocatorPresenter(hWnd, hr)
{
}

STDMETHODIMP CRM9AllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

	return 
		QI2(IRMAVideoSurface)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CRM9AllocatorPresenter::AllocSurfaces()
{
    CAutoLock cAutoLock(this);

	m_pVideoSurfaceOff = NULL;
	m_pVideoSurfaceYUY2 = NULL;

	HRESULT hr;

	if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(
		m_NativeVideoSize.cx, m_NativeVideoSize.cy, D3DFMT_X8R8G8B8, 
		D3DPOOL_DEFAULT, &m_pVideoSurfaceOff, NULL)))
		return hr;

	m_pD3DDev->ColorFill(m_pVideoSurfaceOff, NULL, 0);

	if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(
		m_NativeVideoSize.cx, m_NativeVideoSize.cy, D3DFMT_YUY2, 
		D3DPOOL_DEFAULT, &m_pVideoSurfaceYUY2, NULL)))
		m_pVideoSurfaceYUY2 = NULL;

	if(m_pVideoSurfaceYUY2)
	{
		m_pD3DDev->ColorFill(m_pVideoSurfaceOff, NULL, 0x80108010);
	}

	return __super::AllocSurfaces();
}

void CRM9AllocatorPresenter::DeleteSurfaces()
{
    CAutoLock cAutoLock(this);
	m_pVideoSurfaceOff = NULL;
	m_pVideoSurfaceYUY2 = NULL;
	__super::DeleteSurfaces();
}

// IRMAVideoSurface

STDMETHODIMP CRM9AllocatorPresenter::Blt(UCHAR* pImageData, RMABitmapInfoHeader* pBitmapInfo, REF(PNxRect) inDestRect, REF(PNxRect) inSrcRect)
{
	if(!m_pVideoSurface || !m_pVideoSurfaceOff)
		return E_FAIL;

	bool fRGB = false;
	bool fYUY2 = false;

	CRect src((RECT*)&inSrcRect), dst((RECT*)&inDestRect), src2(CPoint(0,0), src.Size());
	if(src.Width() > dst.Width() || src.Height() > dst.Height())
		return E_FAIL;

	D3DSURFACE_DESC d3dsd;
	ZeroMemory(&d3dsd, sizeof(d3dsd));
	if(FAILED(m_pVideoSurfaceOff->GetDesc(&d3dsd)))
		return E_FAIL;

	int dbpp = 
		d3dsd.Format == D3DFMT_R8G8B8 || d3dsd.Format == D3DFMT_X8R8G8B8 || d3dsd.Format == D3DFMT_A8R8G8B8 ? 32 : 
		d3dsd.Format == D3DFMT_R5G6B5 ? 16 : 0;

	if(pBitmapInfo->biCompression == '024I')
	{
		DWORD pitch = pBitmapInfo->biWidth;
		DWORD size = pitch*abs(pBitmapInfo->biHeight);

		BYTE* y = pImageData					+ src.top*pitch + src.left;
		BYTE* u = pImageData + size				+ src.top*(pitch/2) + src.left/2;
		BYTE* v = pImageData + size + size/4	+ src.top*(pitch/2) + src.left/2;

		if(m_pVideoSurfaceYUY2)
		{
			D3DLOCKED_RECT r;
			if(SUCCEEDED(m_pVideoSurfaceYUY2->LockRect(&r, src2, 0)))
			{
				BitBltFromI420ToYUY2(src.Width(), src.Height(), (BYTE*)r.pBits, r.Pitch, y, u, v, pitch);
				m_pVideoSurfaceYUY2->UnlockRect();
				fYUY2 = true;
			}
		}
		else
		{
			D3DLOCKED_RECT r;
			if(SUCCEEDED(m_pVideoSurfaceOff->LockRect(&r, src2, 0)))
			{
				BitBltFromI420ToRGB(src.Width(), src.Height(), (BYTE*)r.pBits, r.Pitch, dbpp, y, u, v, pitch);
				m_pVideoSurfaceOff->UnlockRect();
				fRGB = true;
			}
		}
	}
	else if(pBitmapInfo->biCompression == '2YUY')
	{
		DWORD w = pBitmapInfo->biWidth;
		DWORD h = abs(pBitmapInfo->biHeight);
		DWORD pitch = pBitmapInfo->biWidth*2;

		BYTE* yvyu = pImageData + src.top*pitch + src.left*2;

		if(m_pVideoSurfaceYUY2)
		{
			D3DLOCKED_RECT r;
			if(SUCCEEDED(m_pVideoSurfaceYUY2->LockRect(&r, src2, 0)))
			{
				BitBltFromYUY2ToYUY2(src.Width(), src.Height(), (BYTE*)r.pBits, r.Pitch, yvyu, pitch);
				m_pVideoSurfaceYUY2->UnlockRect();
				fYUY2 = true;
			}
		}
		else
		{
			D3DLOCKED_RECT r;
			if(SUCCEEDED(m_pVideoSurfaceOff->LockRect(&r, src2, 0)))
			{
				BitBltFromYUY2ToRGB(src.Width(), src.Height(), (BYTE*)r.pBits, r.Pitch, dbpp, yvyu, pitch);
				m_pVideoSurfaceOff->UnlockRect();
				fRGB = true;
			}
		}
	}
	else if(pBitmapInfo->biCompression == 0 || pBitmapInfo->biCompression == 3
		 || pBitmapInfo->biCompression == 'BGRA')
	{
		DWORD w = pBitmapInfo->biWidth;
		DWORD h = abs(pBitmapInfo->biHeight);
		DWORD pitch = pBitmapInfo->biWidth*pBitmapInfo->biBitCount>>3;

		BYTE* rgb = pImageData + src.top*pitch + src.left*(pBitmapInfo->biBitCount>>3);

		D3DLOCKED_RECT r;
		if(SUCCEEDED(m_pVideoSurfaceOff->LockRect(&r, src2, 0)))
		{
			BYTE* pBits = (BYTE*)r.pBits;
			if(pBitmapInfo->biHeight > 0) {pBits += r.Pitch*(src.Height()-1); r.Pitch = -r.Pitch;}
			BitBltFromRGBToRGB(src.Width(), src.Height(), pBits, r.Pitch, dbpp, rgb, pitch, pBitmapInfo->biBitCount);
			m_pVideoSurfaceOff->UnlockRect();
			fRGB = true;
		}
	}

	if(!fRGB && !fYUY2)
	{
		m_pD3DDev->ColorFill(m_pVideoSurfaceOff, NULL, 0);

		HDC hDC;
		if(SUCCEEDED(m_pVideoSurfaceOff->GetDC(&hDC)))
		{
			CString str;
			str.Format(_T("Sorry, this format is not supported"));

			SetBkColor(hDC, 0);
			SetTextColor(hDC, 0x404040);
			TextOut(hDC, 10, 10, str, str.GetLength());

			m_pVideoSurfaceOff->ReleaseDC(hDC);

			fRGB = true;
		}
	}

	HRESULT hr;
	
	if(fRGB)
		hr = m_pD3DDev->StretchRect(m_pVideoSurfaceOff, src2, m_pVideoSurface[m_nCurPicture], dst, D3DTEXF_NONE);
	if(fYUY2)
		hr = m_pD3DDev->StretchRect(m_pVideoSurfaceYUY2, src2, m_pVideoSurface[m_nCurPicture], dst, D3DTEXF_NONE);

	Paint(true);

	return PNR_OK;
}

STDMETHODIMP CRM9AllocatorPresenter::BeginOptimizedBlt(RMABitmapInfoHeader* pBitmapInfo)
{
    CAutoLock cAutoLock(this);
	DeleteSurfaces();
	m_NativeVideoSize = m_AspectRatio = CSize(pBitmapInfo->biWidth, abs(pBitmapInfo->biHeight));
	if(FAILED(AllocSurfaces())) return E_FAIL;
	return PNR_NOTIMPL;
}

STDMETHODIMP CRM9AllocatorPresenter::OptimizedBlt(UCHAR* pImageBits, REF(PNxRect) rDestRect, REF(PNxRect) rSrcRect)
{
	return PNR_NOTIMPL;
}

STDMETHODIMP CRM9AllocatorPresenter::EndOptimizedBlt()
{
	return PNR_NOTIMPL;
}

STDMETHODIMP CRM9AllocatorPresenter::GetOptimizedFormat(REF(RMA_COMPRESSION_TYPE) ulType)
{
	return PNR_NOTIMPL;
}

STDMETHODIMP CRM9AllocatorPresenter::GetPreferredFormat(REF(RMA_COMPRESSION_TYPE) ulType)
{
	ulType = RMA_I420;
	return PNR_OK;
}

//
// CQT9AllocatorPresenter
//

CQT9AllocatorPresenter::CQT9AllocatorPresenter(HWND hWnd, HRESULT& hr) 
	: CDX9AllocatorPresenter(hWnd, hr)
{
}

STDMETHODIMP CQT9AllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

	return 
		QI(IQTVideoSurface)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CQT9AllocatorPresenter::AllocSurfaces()
{
	HRESULT hr;

	m_pVideoSurfaceOff = NULL;

	if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(
		m_NativeVideoSize.cx, m_NativeVideoSize.cy, D3DFMT_X8R8G8B8, 
		D3DPOOL_DEFAULT, &m_pVideoSurfaceOff, NULL)))
		return hr;

	return __super::AllocSurfaces();
}

void CQT9AllocatorPresenter::DeleteSurfaces()
{
	m_pVideoSurfaceOff = NULL;

	__super::DeleteSurfaces();
}

// IQTVideoSurface

STDMETHODIMP CQT9AllocatorPresenter::BeginBlt(const BITMAP& bm)
{
    CAutoLock cAutoLock(this);
	DeleteSurfaces();
	m_NativeVideoSize = m_AspectRatio = CSize(bm.bmWidth, abs(bm.bmHeight));
	if(FAILED(AllocSurfaces())) return E_FAIL;
	return S_OK;
}

STDMETHODIMP CQT9AllocatorPresenter::DoBlt(const BITMAP& bm)
{
	if(!m_pVideoSurface || !m_pVideoSurfaceOff)
		return E_FAIL;

	bool fOk = false;

	D3DSURFACE_DESC d3dsd;
	ZeroMemory(&d3dsd, sizeof(d3dsd));
	if(FAILED(m_pVideoSurfaceOff->GetDesc(&d3dsd)))
		return E_FAIL;

	int w = bm.bmWidth;
	int h = abs(bm.bmHeight);
	int bpp = bm.bmBitsPixel;
	int dbpp = 
		d3dsd.Format == D3DFMT_R8G8B8 || d3dsd.Format == D3DFMT_X8R8G8B8 || d3dsd.Format == D3DFMT_A8R8G8B8 ? 32 : 
		d3dsd.Format == D3DFMT_R5G6B5 ? 16 : 0;

	if((bpp == 16 || bpp == 24 || bpp == 32) && w == d3dsd.Width && h == d3dsd.Height)
	{
		D3DLOCKED_RECT r;
		if(SUCCEEDED(m_pVideoSurfaceOff->LockRect(&r, NULL, 0)))
		{
			BitBltFromRGBToRGB(
				w, h,
				(BYTE*)r.pBits, r.Pitch, dbpp,
				(BYTE*)bm.bmBits, bm.bmWidthBytes, bm.bmBitsPixel);
			m_pVideoSurfaceOff->UnlockRect();
			fOk = true;
		}
	}

	if(!fOk)
	{
		m_pD3DDev->ColorFill(m_pVideoSurfaceOff, NULL, 0);

		HDC hDC;
		if(SUCCEEDED(m_pVideoSurfaceOff->GetDC(&hDC)))
		{
			CString str;
			str.Format(_T("Sorry, this color format is not supported"));

			SetBkColor(hDC, 0);
			SetTextColor(hDC, 0x404040);
			TextOut(hDC, 10, 10, str, str.GetLength());

			m_pVideoSurfaceOff->ReleaseDC(hDC);
		}
	}

	m_pD3DDev->StretchRect(m_pVideoSurfaceOff, NULL, m_pVideoSurface[m_nCurPicture], NULL, D3DTEXF_NONE);

	Paint(true);

	return S_OK;
}

//
// CDXRAllocatorPresenter
//

CDXRAllocatorPresenter::CDXRAllocatorPresenter(HWND hWnd, HRESULT& hr)
	: ISubPicAllocatorPresenterImpl(hWnd, hr)
{
	if(FAILED(hr)) return;

	hr = S_OK;
}

CDXRAllocatorPresenter::~CDXRAllocatorPresenter()
{
	if(m_pSRCB)
	{
		// nasty, but we have to let it know about our death somehow
		((CSubRenderCallback*)(ISubRenderCallback*)m_pSRCB)->SetDXRAP(NULL);
	}

	// the order is important here
	m_pSubPicQueue = NULL;
	m_pAllocator = NULL;
	m_pDXR = NULL;
}

STDMETHODIMP CDXRAllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
/*
	if(riid == __uuidof(IVideoWindow))
		return GetInterface((IVideoWindow*)this, ppv);
	if(riid == __uuidof(IBasicVideo))
		return GetInterface((IBasicVideo*)this, ppv);
	if(riid == __uuidof(IBasicVideo2))
		return GetInterface((IBasicVideo2*)this, ppv);
*/
/*
	if(riid == __uuidof(IVMRWindowlessControl))
		return GetInterface((IVMRWindowlessControl*)this, ppv);
*/

	if(riid != IID_IUnknown && m_pDXR)
	{
		if(SUCCEEDED(m_pDXR->QueryInterface(riid, ppv)))
			return S_OK;
	}

	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CDXRAllocatorPresenter::SetDevice(IDirect3DDevice9* pD3DDev)
{
	CheckPointer(pD3DDev, E_POINTER);

	CSize size;
	switch(AfxGetAppSettings().nSPCMaxRes)
	{
	// TODO: m_ScreenSize ? 
	// case 0: default: size = m_ScreenSize; break;
	default: 
	case 1: size.SetSize(1024, 768); break;
	case 2: size.SetSize(800, 600); break;
	case 3: size.SetSize(640, 480); break;
	case 4: size.SetSize(512, 384); break;
	case 5: size.SetSize(384, 288); break;
	}

	if(m_pAllocator)
	{
		m_pAllocator->ChangeDevice(pD3DDev);
	}
	else
	{
		m_pAllocator = new CDX9SubPicAllocator(pD3DDev, size, AfxGetAppSettings().fSPCPow2Tex);
		if(!m_pAllocator)
			return E_FAIL;
	}

	HRESULT hr = S_OK;

	m_pSubPicQueue = AfxGetAppSettings().nSPCSize > 0 
		? (ISubPicQueue*)new CSubPicQueue(AfxGetAppSettings().nSPCSize, m_pAllocator, &hr)
		: (ISubPicQueue*)new CSubPicQueueNoThread(m_pAllocator, &hr);
	if(!m_pSubPicQueue || FAILED(hr))
		return E_FAIL;

	if(m_SubPicProvider) m_pSubPicQueue->SetSubPicProvider(m_SubPicProvider);

	return S_OK;
}

HRESULT CDXRAllocatorPresenter::Render(
	REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, REFERENCE_TIME atpf,
	int left, int top, int right, int bottom, int width, int height)
{	
	__super::SetPosition(CRect(0, 0, width, height), CRect(left, top, right, bottom)); // needed? should be already set by the player
	SetTime(rtStart);
	if(atpf > 0 && m_pSubPicQueue) m_pSubPicQueue->SetFPS(10000000.0 / atpf);
	AlphaBltSubPic(CSize(width, height));
	return S_OK;
}

// ISubPicAllocatorPresenter

STDMETHODIMP CDXRAllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
    CheckPointer(ppRenderer, E_POINTER);

	if(m_pDXR) return E_UNEXPECTED;
	m_pDXR.CoCreateInstance(CLSID_DXR, GetOwner());
	if(!m_pDXR) return E_FAIL;

	CComQIPtr<ISubRender> pSR = m_pDXR;
	if(!pSR) {m_pDXR = NULL; return E_FAIL;}

	m_pSRCB = new CSubRenderCallback(this);
	if(FAILED(pSR->SetCallback(m_pSRCB))) {m_pDXR = NULL; return E_FAIL;}

	(*ppRenderer = this)->AddRef();

	return S_OK;
}

STDMETHODIMP_(void) CDXRAllocatorPresenter::SetPosition(RECT w, RECT v)
{
	if(CComQIPtr<IBasicVideo> pBV = m_pDXR)
	{
		pBV->SetDefaultSourcePosition();
		pBV->SetDestinationPosition(v.left, v.top, v.right - v.left, v.bottom - v.top);
	}

	if(CComQIPtr<IVideoWindow> pVW = m_pDXR)
	{
		pVW->SetWindowPosition(w.left, w.top, w.right - w.left, w.bottom - w.top);
	}
}

STDMETHODIMP_(SIZE) CDXRAllocatorPresenter::GetVideoSize(bool fCorrectAR)
{
	SIZE size = {0, 0};

	if(!fCorrectAR)
	{
		if(CComQIPtr<IBasicVideo> pBV = m_pDXR)
			pBV->GetVideoSize(&size.cx, &size.cy);
	}
	else
	{
		if(CComQIPtr<IBasicVideo2> pBV2 = m_pDXR)
			pBV2->GetPreferredAspectRatio(&size.cx, &size.cy);
	}

	return size;
}

STDMETHODIMP_(bool) CDXRAllocatorPresenter::Paint(bool fAll)
{
	return false; // TODO
}

STDMETHODIMP CDXRAllocatorPresenter::GetDIB(BYTE* lpDib, DWORD* size)
{
	HRESULT hr = E_NOTIMPL;
	if(CComQIPtr<IBasicVideo> pBV = m_pDXR)
		hr = pBV->GetCurrentImage((long*)size, (long*)lpDib);
	return hr;
}

STDMETHODIMP CDXRAllocatorPresenter::SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget)
{
	return E_NOTIMPL; // TODO
}

