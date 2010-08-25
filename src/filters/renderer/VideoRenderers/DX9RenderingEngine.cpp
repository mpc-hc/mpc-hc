/*
 * $Id$
 *
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
#include <algorithm>
#include <lcms2.h>
#include "../apps/mplayerc/resource.h"
#include "Dither.h"
#include "DX9RenderingEngine.h"

#pragma pack(push, 1)
template<int texcoords>
struct MYD3DVERTEX
{
	float x, y, z, rhw;
	struct
	{
		float u, v;
	} t[texcoords];
};
template<>
struct MYD3DVERTEX<0>
{
	float x, y, z, rhw;
	DWORD Diffuse;
};
#pragma pack(pop)

template<int texcoords>
static void AdjustQuad(MYD3DVERTEX<texcoords>* v, double dx, double dy)
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
static HRESULT TextureBlt(IDirect3DDevice9* pD3DDev, MYD3DVERTEX<texcoords> v[4], D3DTEXTUREFILTERTYPE filter = D3DTEXF_LINEAR)
{
	if(!pD3DDev)
		return E_POINTER;

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

	hr = pD3DDev->SetFVF(D3DFVF_XYZRHW | FVF);
	// hr = pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(v[0]));

	MYD3DVERTEX<texcoords> tmp = v[2];
	v[2] = v[3];
	v[3] = tmp;
	hr = pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(v[0]));

	//

	for(int i = 0; i < texcoords; i++)
	{
		pD3DDev->SetTexture(i, NULL);
	}

	return S_OK;
}


using namespace DSObjects;
using namespace std;

CDX9RenderingEngine::CDX9RenderingEngine(HWND hWnd, HRESULT& hr, CString *_pError)
	: CSubPicAllocatorPresenterImpl(hWnd, hr, _pError)
	, m_ScreenSize(0, 0)
	, m_nNbDXSurface(1)
	, m_nCurSurface(0)
{
	HINSTANCE hDll = GetRenderersData()->GetD3X9Dll();
	m_bD3DX = hDll != NULL;

	if (m_bD3DX)
	{
		(FARPROC&)m_pD3DXFloat32To16Array = GetProcAddress(hDll, "D3DXFloat32To16Array");
	}
}

void CDX9RenderingEngine::InitRenderingEngine()
{
	m_pPSC.Attach(DNew CPixelShaderCompiler(m_pD3DDev, true));

	// Detect supported StrechRect filter
	m_StretchRectFilter = D3DTEXF_NONE;
	if((m_Caps.StretchRectFilterCaps&D3DPTFILTERCAPS_MINFLINEAR)
			&& (m_Caps.StretchRectFilterCaps&D3DPTFILTERCAPS_MAGFLINEAR))
		m_StretchRectFilter = D3DTEXF_LINEAR;

	m_BicubicA = 0;

	m_bFinalPass = false;
	m_bFullFloatingPointProcessing = false;
	m_bColorManagement = false;
}

void CDX9RenderingEngine::CleanupRenderingEngine()
{
	m_pPSC.Free();

	for (int i = 0; i < 4; i++)
		m_pResizerPixelShaders[i] = NULL;

	CleanupFinalPass();

	POSITION pos = m_pCustomScreenSpacePixelShaders.GetHeadPosition();
	while(pos)
	{
		CExternalPixelShader &Shader = m_pCustomScreenSpacePixelShaders.GetNext(pos);
		Shader.m_pPixelShader = NULL;
	}
	pos = m_pCustomPixelShaders.GetHeadPosition();
	while(pos)
	{
		CExternalPixelShader &Shader = m_pCustomPixelShaders.GetNext(pos);
		Shader.m_pPixelShader = NULL;
	}

	for (int i = 0; i < 2; i++)
	{
		m_pTemporaryVideoTextures[i] = NULL;
		m_pTemporaryScreenSpaceTextures[i] = NULL;
	}
}

HRESULT CDX9RenderingEngine::CreateVideoSurfaces(D3DFORMAT format)
{
	CRenderersSettings& settings = GetRenderersSettings();

	for (int i = 0; i < m_nNbDXSurface; i++)
	{
		m_pVideoTexture[i] = NULL;
		m_pVideoSurface[i] = NULL;
	}

	m_SurfaceType = format;

	HRESULT hr;

	if (settings.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D || settings.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
	{
		int nTexturesNeeded = settings.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D ? m_nNbDXSurface : 1;

		for (int i = 0; i < nTexturesNeeded; i++)
		{
			if(FAILED(hr = m_pD3DDev->CreateTexture(
							   m_NativeVideoSize.cx, m_NativeVideoSize.cy, 1,
							   D3DUSAGE_RENDERTARGET, format,
							   D3DPOOL_DEFAULT, &m_pVideoTexture[i], NULL)))
				return hr;

			if (FAILED(hr = m_pVideoTexture[i]->GetSurfaceLevel(0, &m_pVideoSurface[i])))
				return hr;
		}

		if (settings.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D)
		{
			m_RenderingPath = RENDERING_PATH_STRETCHRECT;

			for (int i = 0; i < m_nNbDXSurface; i++)
				m_pVideoTexture[i] = NULL;
		}
		else
		{
			m_RenderingPath = RENDERING_PATH_DRAW;
		}
	}
	else
	{
		m_RenderingPath = RENDERING_PATH_STRETCHRECT;

		if (FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(
							m_NativeVideoSize.cx, m_NativeVideoSize.cy,
							format,
							D3DPOOL_DEFAULT, &m_pVideoSurface[m_nCurSurface], NULL)))
			return hr;
	}

	hr = m_pD3DDev->ColorFill(m_pVideoSurface[m_nCurSurface], NULL, 0);

	return S_OK;
}

void CDX9RenderingEngine::FreeVideoSurfaces()
{
	for (int i = 0; i < m_nNbDXSurface; i++)
	{
		m_pVideoTexture[i] = NULL;
		m_pVideoSurface[i] = NULL;
	}
}

HRESULT CDX9RenderingEngine::RenderVideo(IDirect3DSurface9* pRenderTarget, const CRect& srcRect, const CRect& destRect)
{
	if (m_RenderingPath == RENDERING_PATH_DRAW)
		return RenderVideoDrawPath(pRenderTarget, srcRect, destRect);
	else
		return RenderVideoStretchRectPath(pRenderTarget, srcRect, destRect);
}

HRESULT CDX9RenderingEngine::RenderVideoDrawPath(IDirect3DSurface9* pRenderTarget, const CRect& srcRect, const CRect& destRect)
{
	HRESULT hr;

	CRenderersSettings& settings = GetRenderersSettings();

	// Initialize the processing pipeline
	bool bCustomPixelShaders;
	bool bResizerShaders;
	bool bCustomScreenSpacePixelShaders;
	bool bFinalPass;

	int screenSpacePassCount = 0;
	DWORD iDX9Resizer = settings.iDX9Resizer;

	if (m_bD3DX)
	{
		// Final pass. Must be initialized first!
		hr = InitFinalPass();
		if (SUCCEEDED(hr))
			bFinalPass = m_bFinalPass;
		else
			bFinalPass = false;

		if (bFinalPass)
			++screenSpacePassCount;

		// Resizers
		float bicubicA = 0;
		switch (iDX9Resizer)
		{
		case 3:
			bicubicA = -0.60f;
			break;
		case 4:
			bicubicA = -0.751f;
			break;	// FIXME : 0.75 crash recent D3D, or eat CPU
		case 5:
			bicubicA = -1.00f;
			break;
		}

		hr = InitResizers(bicubicA);
		bResizerShaders = SUCCEEDED(hr);
		screenSpacePassCount += 1; // currently all resizers are 1-pass

		// Custom screen space pixel shaders
		bCustomScreenSpacePixelShaders = !m_pCustomScreenSpacePixelShaders.IsEmpty();

		if (bCustomScreenSpacePixelShaders)
			screenSpacePassCount += m_pCustomScreenSpacePixelShaders.GetCount();

		// Custom pixel shaders
		bCustomPixelShaders = !m_pCustomPixelShaders.IsEmpty();

		hr = InitTemporaryVideoTextures(min(m_pCustomPixelShaders.GetCount(), 2));
		if (FAILED(hr))
			bCustomPixelShaders = false;
	}
	else
	{
		bCustomPixelShaders = false;
		bResizerShaders = false;
		bCustomScreenSpacePixelShaders = false;
		bFinalPass = false;
	}

	hr = InitScreenSpacePipeline(screenSpacePassCount, pRenderTarget);
	if (FAILED(hr))
	{
		bCustomScreenSpacePixelShaders = false;
		bFinalPass = false;
	}

	// Apply the custom pixel shaders if there are any. Result: pVideoTexture
	CComPtr<IDirect3DTexture9> pVideoTexture = m_pVideoTexture[m_nCurSurface];

	if (bCustomPixelShaders)
	{
		static __int64 counter = 0;
		static long start = clock();

		long stop = clock();
		long diff = stop - start;

		if(diff >= 10*60*CLOCKS_PER_SEC) start = stop; // reset after 10 min (ps float has its limits in both range and accuracy)

#if 1
		D3DSURFACE_DESC desc;
		m_pVideoTexture[m_nCurSurface]->GetLevelDesc(0, &desc);

		float fConstData[][4] =
		{
			{(float)desc.Width, (float)desc.Height, (float)(counter++), (float)diff / CLOCKS_PER_SEC},
			{1.0f / desc.Width, 1.0f / desc.Height, 0, 0},
		};
#else
		CSize VideoSize = GetVisibleVideoSize();

		float fConstData[][4] =
		{
			{(float)VideoSize.cx, (float)VideoSize.cy, (float)(counter++), (float)diff / CLOCKS_PER_SEC},
			{1.0f / VideoSize.cx, 1.0f / VideoSize.cy, 0, 0},
		};
#endif

		hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));

		int src = 1;
		int dest = 0;
		bool first = true;

		POSITION pos = m_pCustomPixelShaders.GetHeadPosition();
		while(pos)
		{
			CComPtr<IDirect3DSurface9> pTemporarySurface;
			hr = m_pTemporaryVideoTextures[dest]->GetSurfaceLevel(0, &pTemporarySurface);
			hr = m_pD3DDev->SetRenderTarget(0, pTemporarySurface);

			CExternalPixelShader &Shader = m_pCustomPixelShaders.GetNext(pos);
			if (!Shader.m_pPixelShader)
				Shader.Compile(m_pPSC);
			hr = m_pD3DDev->SetPixelShader(Shader.m_pPixelShader);

			if (first)
			{
				TextureCopy(m_pVideoTexture[m_nCurSurface]);
				first = false;
			}
			else
			{
				TextureCopy(m_pTemporaryVideoTextures[src]);
			}

			swap(src, dest);
		}

		pVideoTexture = m_pTemporaryVideoTextures[src];
	}

	// Resize the frame
	Vector dst[4];
	Transform(destRect, dst);

	hr = BeginScreenSpacePass();

	if (m_ScreenSpacePassCount > 0)
		hr = m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);

	if (srcRect.Size() != destRect.Size())
	{
		if (iDX9Resizer == 0 || iDX9Resizer == 1 || !bResizerShaders)
		{
			D3DTEXTUREFILTERTYPE Filter = iDX9Resizer == 0 ? D3DTEXF_POINT : D3DTEXF_LINEAR;
			hr = TextureResize(pVideoTexture, dst, Filter, srcRect);
		}
		else if (iDX9Resizer == 2)
		{
			hr = TextureResizeBilinear(pVideoTexture, dst, srcRect);
		}
		else if (iDX9Resizer >= 3)
		{
			hr = TextureResizeBicubic1pass(pVideoTexture, dst, srcRect);
		}
	}
	else hr = TextureResize(pVideoTexture, dst, D3DTEXF_POINT, srcRect);

	// Apply the custom screen size pixel shaders
	if (bCustomScreenSpacePixelShaders)
	{
		static __int64 counter = 555;
		static long start = clock() + 333;

		long stop = clock() + 333;
		long diff = stop - start;

		if (diff >= 10*60*CLOCKS_PER_SEC) start = stop; // reset after 10 min (ps float has its limits in both range and accuracy)

		float fConstData[][4] =
		{
			{(float)m_TemporaryScreenSpaceTextureSize.cx, (float)m_TemporaryScreenSpaceTextureSize.cy, (float)(counter++), (float)diff / CLOCKS_PER_SEC},
			{1.0f / m_TemporaryScreenSpaceTextureSize.cx, 1.0f / m_TemporaryScreenSpaceTextureSize.cy, 0, 0},
		};

		hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));

		POSITION pos = m_pCustomScreenSpacePixelShaders.GetHeadPosition();
		while (pos)
		{
			BeginScreenSpacePass();

			CExternalPixelShader &Shader = m_pCustomScreenSpacePixelShaders.GetNext(pos);
			if (!Shader.m_pPixelShader)
				Shader.Compile(m_pPSC);
			hr = m_pD3DDev->SetPixelShader(Shader.m_pPixelShader);
			TextureCopy(m_pTemporaryScreenSpaceTextures[m_ScreenSpacePassSrc]);
		}
	}

	// Final pass
	if (bFinalPass)
	{
		hr = BeginScreenSpacePass();
		hr = FinalPass(m_pTemporaryScreenSpaceTextures[m_ScreenSpacePassSrc]);
	}

	hr = m_pD3DDev->SetPixelShader(NULL);

	return hr;
}

HRESULT CDX9RenderingEngine::RenderVideoStretchRectPath(IDirect3DSurface9* pRenderTarget, const CRect& srcRect, const CRect& destRect)
{
	HRESULT hr = S_OK;

	if (pRenderTarget)
	{
		CRect rSrcVid(srcRect);
		CRect rDstVid(destRect);

		ClipToSurface(pRenderTarget, rSrcVid, rDstVid); // grrr
		// IMPORTANT: rSrcVid has to be aligned on mod2 for yuy2->rgb conversion with StretchRect!!!
		rSrcVid.left &= ~1;
		rSrcVid.right &= ~1;
		rSrcVid.top &= ~1;
		rSrcVid.bottom &= ~1;
		hr = m_pD3DDev->StretchRect(m_pVideoSurface[m_nCurSurface], rSrcVid, pRenderTarget, rDstVid, m_StretchRectFilter);
	}

	return hr;
}

HRESULT CDX9RenderingEngine::InitTemporaryVideoTextures(int count)
{
	HRESULT hr = S_OK;

	for (int i = 0; i < count; i++)
	{
		if (m_pTemporaryVideoTextures[i] == NULL)
		{
			D3DFORMAT format;
			if (m_bFullFloatingPointProcessing)
			{
				format = D3DFMT_A16B16G16R16F;
			}
			else
			{
				if (m_bHighColorResolution || m_bForceInputHighColorResolution)
					format = D3DFMT_A2R10G10B10;
				else
					format = D3DFMT_A8R8G8B8;
			}

			hr = m_pD3DDev->CreateTexture(
					 m_NativeVideoSize.cx, m_NativeVideoSize.cy, 1, D3DUSAGE_RENDERTARGET, format,
					 D3DPOOL_DEFAULT, &m_pTemporaryVideoTextures[i], NULL);

			if (FAILED(hr))
			{
				// Free all textures
				for (int j = 0; j < 2; j++)
					m_pTemporaryVideoTextures[j] = NULL;

				return hr;
			}
		}
	}

	// Free unnecessary textures
	for (int i = count; i < 2; i++)
		m_pTemporaryVideoTextures[i] = NULL;

	return hr;
}

HRESULT CDX9RenderingEngine::InitScreenSpacePipeline(int passCount, IDirect3DSurface9* pRenderTarget)
{
	m_pRenderTarget = pRenderTarget;
	m_ScreenSpacePassCount = passCount;
	m_ScreenSpacePassSrc = 0;
	m_ScreenSpacePassDest = 1;

	HRESULT hr = InitTemporaryScreenSpaceTextures(min(passCount - 1, 2));

	// If the initialized have failed, disable the pipeline
	if (FAILED(hr))
		m_ScreenSpacePassCount = 1;

	return hr;
}

HRESULT CDX9RenderingEngine::InitTemporaryScreenSpaceTextures(int count)
{
	HRESULT hr = S_OK;

	for (int i = 0; i < count; i++)
	{
		if (m_pTemporaryScreenSpaceTextures[i] == NULL)
		{
			m_TemporaryScreenSpaceTextureSize = CSize(min(m_ScreenSize.cx, (int)m_Caps.MaxTextureWidth),
													  min(max(m_ScreenSize.cy, m_NativeVideoSize.cy), (int)m_Caps.MaxTextureHeight));

			D3DFORMAT format;
			if (m_bFullFloatingPointProcessing)
			{
				format = D3DFMT_A16B16G16R16F;
			}
			else
			{
				if (m_bHighColorResolution || m_bForceInputHighColorResolution)
					format = D3DFMT_A2R10G10B10;
				else
					format = D3DFMT_A8R8G8B8;
			}

			hr = m_pD3DDev->CreateTexture(
					 m_TemporaryScreenSpaceTextureSize.cx, m_TemporaryScreenSpaceTextureSize.cy, 1, D3DUSAGE_RENDERTARGET, format,
					 D3DPOOL_DEFAULT, &m_pTemporaryScreenSpaceTextures[i], NULL);

			if (FAILED(hr))
			{
				// Free all textures
				for (int j = 0; j < 2; j++)
					m_pTemporaryScreenSpaceTextures[j] = NULL;

				return hr;
			}
		}
	}

	// Free unnecessary textures
	for (int i = count; i < 2; i++)
		m_pTemporaryScreenSpaceTextures[i] = NULL;

	return hr;
}

HRESULT CDX9RenderingEngine::BeginScreenSpacePass()
{
	HRESULT hr;

	swap(m_ScreenSpacePassSrc, m_ScreenSpacePassDest);
	--m_ScreenSpacePassCount;

	if (m_ScreenSpacePassCount > 0)
	{
		CComPtr<IDirect3DSurface9> pTemporarySurface;
		hr = m_pTemporaryScreenSpaceTextures[m_ScreenSpacePassDest]->GetSurfaceLevel(0, &pTemporarySurface);

		if (SUCCEEDED(hr))
			hr = m_pD3DDev->SetRenderTarget(0, pTemporarySurface);
	}
	else
	{
		hr = m_pD3DDev->SetRenderTarget(0, m_pRenderTarget);
	}

	return hr;
}

HRESULT CDX9RenderingEngine::InitResizers(float bicubicA)
{
	HRESULT hr;

	// Check whether the resizer pixel shaders must be initialized
	bool bInitRequired = false;

	if (bicubicA)
	{
		for (int i = 0; i < 4; i++)
		{
			if (!m_pResizerPixelShaders[i])
				bInitRequired = true;
		}

		if (m_BicubicA != bicubicA)
			bInitRequired = true;
	}
	else
	{
		if (!m_pResizerPixelShaders[0])
			bInitRequired = true;
	}

	if (!bInitRequired)
		return S_OK;

	// Initialize the resizer pixel shaders
	m_BicubicA = bicubicA;

	for (int i = 0; i < countof(m_pResizerPixelShaders); i++)
		m_pResizerPixelShaders[i] = NULL;

	if (m_Caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return E_FAIL;

	LPCSTR pProfile = m_Caps.PixelShaderVersion >= D3DPS_VERSION(3, 0) ? "ps_3_0" : "ps_2_0";

	CStringA str;
	if (!LoadResource(IDF_SHADER_RESIZER, str, _T("FILE")))
		return E_FAIL;

	CStringA A;
	A.Format("(%f)", bicubicA);
	str.Replace("_The_Value_Of_A_Is_Set_Here_", A);

	LPCSTR pEntries[] = {"main_bilinear", "main_bicubic1pass", "main_bicubic2pass_pass1", "main_bicubic2pass_pass2"};

	ASSERT(countof(pEntries) == countof(m_pResizerPixelShaders));

	for (int i = 0; i < countof(pEntries); i++)
	{
		CString ErrorMessage;
		CString DissAssembly;
		hr = m_pPSC->CompileShader(str, pEntries[i], pProfile, 0, &m_pResizerPixelShaders[i], &DissAssembly, &ErrorMessage);
		if (FAILED(hr))
		{
			TRACE("%ws", ErrorMessage.GetString());
			ASSERT (0);
			return hr;
		}
		/*
		if (i == 2 || i == 3)
		{
			const wchar_t *pStr = DissAssembly.GetString();
			TRACE("DisAsm: %s\n", pEntries[i]);
			const wchar_t *pStrStart = pStr;
			while (*pStr)
			{
				while (*pStr && *pStr != '\n')
					++pStr;
				if (*pStr == '\n')
					++pStr;
				if (*pStr == '\r')
					++pStr;
				CString Test(pStrStart, pStr - pStrStart);
				TRACE("%ws", Test.GetString());
				pStrStart = pStr;
			}
		}
		*/
	}

	return S_OK;
}

HRESULT CDX9RenderingEngine::TextureResize(IDirect3DTexture9* pTexture, Vector dst[4], D3DTEXTUREFILTERTYPE filter, const CRect &srcRect)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if (!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
		return E_FAIL;

	float w = (float)desc.Width;
	float h = (float)desc.Height;

	float dx2 = 1.0/w;
	float dy2 = 1.0/h;

	MYD3DVERTEX<1> v[] =
	{
		{dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  srcRect.left * dx2, srcRect.top * dy2},
		{dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  srcRect.right * dx2, srcRect.top * dy2},
		{dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  srcRect.left * dx2, srcRect.bottom * dy2},
		{dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  srcRect.right * dx2, srcRect.bottom * dy2},
	};

	AdjustQuad(v, 0, 0);

	hr = m_pD3DDev->SetTexture(0, pTexture);
	hr = m_pD3DDev->SetPixelShader(NULL);
	hr = TextureBlt(m_pD3DDev, v, filter);

	return hr;
}

HRESULT CDX9RenderingEngine::TextureResizeBilinear(IDirect3DTexture9* pTexture, Vector dst[4], const CRect &srcRect)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if (!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
		return E_FAIL;

	// make const to give compiler a chance of optimising, also float faster than double and converted to float to sent to PS anyway
	const float dx = 1.0f/(float)desc.Width;
	const float dy = 1.0f/(float)desc.Height;
	const float tx0 = srcRect.left;
	const float tx1 = srcRect.right;
	const float ty0 = srcRect.top;
	const float ty1 = srcRect.bottom;

	MYD3DVERTEX<1> v[] =
	{
		{dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  tx0, ty0},
		{dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  tx1, ty0},
		{dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  tx0, ty1},
		{dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  tx1, ty1},
	};

	AdjustQuad(v, 1.0, 1.0);

	hr = m_pD3DDev->SetTexture(0, pTexture);

	float fConstData[][4] = {{dx*0.5f, dy*0.5f, 0, 0}, {dx, dy, 0, 0}, {dx, 0, 0, 0}, {0, dy, 0, 0}};
	hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));

	hr = m_pD3DDev->SetTexture(0, pTexture);
	hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShaders[0]);

	hr = TextureBlt(m_pD3DDev, v, D3DTEXF_POINT);
	return hr;
}

HRESULT CDX9RenderingEngine::TextureResizeBicubic1pass(IDirect3DTexture9* pTexture, Vector dst[4], const CRect &srcRect)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if (!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
		return E_FAIL;

	// make const to give compiler a chance of optimising, also float faster than double and converted to float to sent to PS anyway
	const float dx = 1.0f/(float)desc.Width;
	const float dy = 1.0f/(float)desc.Height;
	const float tx0 = srcRect.left;
	const float tx1 = srcRect.right;
	const float ty0 = srcRect.top;
	const float ty1 = srcRect.bottom;

	MYD3DVERTEX<1> v[] =
	{
		{dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z,  tx0, ty0},
		{dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z,  tx1, ty0},
		{dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z,  tx0, ty1},
		{dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z,  tx1, ty1},
	};

	AdjustQuad(v, 1.0, 1.0);

	hr = m_pD3DDev->SetTexture(0, pTexture);

	float fConstData[][4] = {{dx*0.5f, dy*0.5f, 0, 0}, {dx, dy, 0, 0}, {dx, 0, 0, 0}, {0, dy, 0, 0}};
	hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));

	hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShaders[1]);

	hr = TextureBlt(m_pD3DDev, v, D3DTEXF_POINT);
	return hr;
}

/*
// The 2 pass sampler is incorrect in that it only does bilinear resampling in the y direction.
HRESULT CDX9RenderingEngine::TextureResizeBicubic2pass(IDirect3DTexture9* pTexture, Vector dst[4], const CRect &srcRect)
{
	HRESULT hr;

	// rotated?
	if(dst[0].z != dst[1].z || dst[2].z != dst[3].z || dst[0].z != dst[3].z
	|| dst[0].y != dst[1].y || dst[0].x != dst[2].x || dst[2].y != dst[3].y || dst[1].x != dst[3].x)
	    return TextureResizeBicubic1pass(pTexture, dst, srcRect);

	D3DSURFACE_DESC desc;
	if(!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
	    return E_FAIL;

	float Tex0_Width = desc.Width;
	float Tex0_Height = desc.Height;

	double dx0 = 1.0/desc.Width;
	UNUSED_ALWAYS(dx0);
	double dy0 = 1.0/desc.Height;
	UNUSED_ALWAYS(dy0);

	CSize SrcTextSize = CSize(desc.Width, desc.Height);
	double w = (double)srcRect.Width();
	double h = (double)srcRect.Height();
	UNUSED_ALWAYS(w);

	CRect dst1(0, 0, (int)(dst[3].x - dst[0].x), (int)h);

	if(!m_pTemporaryScreenSpaceTextures[0] || FAILED(m_pTemporaryScreenSpaceTextures[0]->GetLevelDesc(0, &desc)))
	    return TextureResizeBicubic1pass(pTexture, dst, srcRect);

	float Tex1_Width = desc.Width;
	float Tex1_Height = desc.Height;

	double dx1 = 1.0/desc.Width;
	UNUSED_ALWAYS(dx1);
	double dy1 = 1.0/desc.Height;
	UNUSED_ALWAYS(dy1);

	double dw = (double)dst1.Width() / desc.Width;
	UNUSED_ALWAYS(dw);
	double dh = (double)dst1.Height() / desc.Height;
	UNUSED_ALWAYS(dh);

	float dx2 = 1.0f/SrcTextSize.cx;
	UNUSED_ALWAYS(dx2);
	float dy2 = 1.0f/SrcTextSize.cy;
	UNUSED_ALWAYS(dy2);

	float tx0 = srcRect.left;
	float tx1 = srcRect.right;
	float ty0 = srcRect.top;
	float ty1 = srcRect.bottom;

	float tx0_2 = 0;
	float tx1_2 = dst1.Width();
	float ty0_2 = 0;
	float ty1_2 = h;

	//	ASSERT(dst1.Height() == desc.Height);

	if(dst1.Width() > (int)desc.Width || dst1.Height() > (int)desc.Height)
	    // if(dst1.Width() != desc.Width || dst1.Height() != desc.Height)
	    return TextureResizeBicubic1pass(pTexture, dst, srcRect);

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


	AdjustQuad(vy, 0.0, 1.0);		// Casimir666 : bug ici, génére des bandes horizontales! TODO : pourquoi ??????

	hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShaders[2]);
	{
	    float fConstData[][4] = {{0.5f / Tex0_Width, 0.5f / Tex0_Height, 0, 0}, {1.0f / Tex0_Width, 1.0f / Tex0_Height, 0, 0}, {1.0f / Tex0_Width, 0, 0, 0}, {0, 1.0f / Tex0_Height, 0, 0}, {Tex0_Width, Tex0_Height, 0, 0}};
	    hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));
	}

	hr = m_pD3DDev->SetTexture(0, pTexture);

	CComPtr<IDirect3DSurface9> pRTOld;
	hr = m_pD3DDev->GetRenderTarget(0, &pRTOld);

	CComPtr<IDirect3DSurface9> pRT;
	hr = m_pTemporaryScreenSpaceTextures[0]->GetSurfaceLevel(0, &pRT);
	hr = m_pD3DDev->SetRenderTarget(0, pRT);

	hr = TextureBlt(m_pD3DDev, vx, D3DTEXF_POINT);

	hr = m_pD3DDev->SetPixelShader(m_pResizerPixelShaders[3]);
	{
	    float fConstData[][4] = {{0.5f / Tex1_Width, 0.5f / Tex1_Height, 0, 0}, {1.0f / Tex1_Width, 1.0f / Tex1_Height, 0, 0}, {1.0f / Tex1_Width, 0, 0, 0}, {0, 1.0f / Tex1_Height, 0, 0}, {Tex1_Width, Tex1_Height, 0, 0}};
	    hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));
	}

	hr = m_pD3DDev->SetTexture(0, m_pTemporaryScreenSpaceTextures[0]);

	hr = m_pD3DDev->SetRenderTarget(0, pRTOld);

	hr = TextureBlt(m_pD3DDev, vy, D3DTEXF_POINT);
	return hr;
}
*/

HRESULT CDX9RenderingEngine::InitFinalPass()
{
	HRESULT hr;

	CRenderersSettings& settings = GetRenderersSettings();
	CRenderersData* data = GetRenderersData();

	// Check whether the final pass must be initialized
	bool bFullFloatingPointProcessing = settings.m_RenderSettings.iVMR9FullFloatingPointProcessing;
	bool bColorManagement = settings.m_RenderSettings.iVMR9ColorManagementEnable;
	VideoSystem inputVideoSystem = static_cast<VideoSystem>(settings.m_RenderSettings.iVMR9ColorManagementInput);
	GammaCurve gamma = static_cast<GammaCurve>(settings.m_RenderSettings.iVMR9ColorManagementGamma);
	ColorRenderingIntent renderingIntent = static_cast<ColorRenderingIntent>(settings.m_RenderSettings.iVMR9ColorManagementIntent);

	bool bInitRequired = false;

	if ((m_bFullFloatingPointProcessing != bFullFloatingPointProcessing) ||
		(m_bColorManagement != bColorManagement))
	{
		bInitRequired = true;
	}

	if (m_bColorManagement && bColorManagement)
	{
		if ((m_InputVideoSystem != inputVideoSystem) ||
			(m_RenderingIntent != renderingIntent) ||
			(m_Gamma != gamma))
		{
			bInitRequired = true;
		}
	}

	if (!bInitRequired)
		return S_OK;

	// Cleanup
	CleanupFinalPass();

	// Check whether the final pass is supported by the hardware
	m_bFinalPass = data->m_bFP16Support;
	if (!m_bFinalPass)
		return S_OK;

	// Update the settings
	m_bFullFloatingPointProcessing = bFullFloatingPointProcessing;
	m_bColorManagement = bColorManagement;
	m_InputVideoSystem = inputVideoSystem;
	m_Gamma = gamma;
	m_RenderingIntent = renderingIntent;

	// Check whether the final pass is required
	m_bFinalPass = bFullFloatingPointProcessing || bColorManagement ||
				   (m_bForceInputHighColorResolution && !m_bHighColorResolution);

	if (!m_bFinalPass)
		return S_OK;

	// Create the dither texture
	hr = m_pD3DDev->CreateTexture(DITHER_MATRIX_SIZE, DITHER_MATRIX_SIZE,
								  1,
								  D3DUSAGE_DYNAMIC,
								  D3DFMT_A16B16G16R16F,
								  D3DPOOL_DEFAULT,
								  &m_pDitherTexture,
								  NULL);

	if (FAILED(hr))
	{
		CleanupFinalPass();
		return hr;
	}

	D3DLOCKED_RECT lockedRect;
	hr = m_pDitherTexture->LockRect(0, &lockedRect, NULL, D3DLOCK_DISCARD);
	if (FAILED(hr))
	{
		CleanupFinalPass();
		return hr;
	}

	char* outputRowIterator = static_cast<char*>(lockedRect.pBits);
	for (int y = 0; y < DITHER_MATRIX_SIZE; y++)
	{
		unsigned short* outputIterator = reinterpret_cast<unsigned short*>(outputRowIterator);
		for (int x = 0; x < DITHER_MATRIX_SIZE; x++)
		{
			for (int i = 0; i < 4; i++)
				*outputIterator++ = DITHER_MATRIX[y][x];
		}

		outputRowIterator += lockedRect.Pitch;
	}

	hr = m_pDitherTexture->UnlockRect(0);
	if (FAILED(hr))
	{
		CleanupFinalPass();
		return hr;
	}

	// Initialize the color management if necessary
	if (bColorManagement)
	{
		// Get the ICC profile path
		TCHAR* iccProfilePath = 0;
		HDC hDC = GetDC(m_hWnd);

		if (hDC != NULL)
		{
			DWORD icmProfilePathSize = 0;
			GetICMProfile(hDC, &icmProfilePathSize, NULL);
			iccProfilePath = new TCHAR[icmProfilePathSize];
			if (!GetICMProfile(hDC, &icmProfilePathSize, iccProfilePath))
			{
				delete[] iccProfilePath;
				iccProfilePath = 0;
			}

			ReleaseDC(m_hWnd, hDC);
		}

		// Create the 3D LUT texture
		m_Lut3DSize = 64; // 64x64x64 LUT is enough for high-quality color management
		m_Lut3DEntryCount = m_Lut3DSize * m_Lut3DSize * m_Lut3DSize;

		hr = m_pD3DDev->CreateVolumeTexture(m_Lut3DSize, m_Lut3DSize, m_Lut3DSize,
											1,
											D3DUSAGE_DYNAMIC,
											D3DFMT_A16B16G16R16F,
											D3DPOOL_DEFAULT,
											&m_pLut3DTexture,
											NULL);

		if (FAILED(hr))
		{
			delete[] iccProfilePath;
			CleanupFinalPass();
			return hr;
		}

		float* lut3DFloat32 = new float[m_Lut3DEntryCount * 3];
		hr = CreateIccProfileLut(iccProfilePath, lut3DFloat32);
		delete[] iccProfilePath;
		if (FAILED(hr))
		{
			delete[] lut3DFloat32;
			CleanupFinalPass();
			return hr;
		}

		D3DXFLOAT16* lut3DFloat16 = new D3DXFLOAT16[m_Lut3DEntryCount * 3];
		m_pD3DXFloat32To16Array(lut3DFloat16, lut3DFloat32, m_Lut3DEntryCount * 3);
		delete[] lut3DFloat32;

		const float oneFloat32 = 1.0f;
		D3DXFLOAT16 oneFloat16;
		m_pD3DXFloat32To16Array(&oneFloat16, &oneFloat32, 1);

		D3DLOCKED_BOX lockedBox;
		hr = m_pLut3DTexture->LockBox(0, &lockedBox, NULL, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			delete[] lut3DFloat16;
			CleanupFinalPass();
			return hr;
		}

		D3DXFLOAT16* lut3DFloat16Iterator = lut3DFloat16;
		char* outputSliceIterator = static_cast<char*>(lockedBox.pBits);
		for (int b = 0; b < m_Lut3DSize; b++)
		{
			char* outputRowIterator = outputSliceIterator;

			for (int g = 0; g < m_Lut3DSize; g++)
			{
				D3DXFLOAT16* outputIterator = reinterpret_cast<D3DXFLOAT16*>(outputRowIterator);

				for (int r = 0; r < m_Lut3DSize; r++)
				{
					// R, G, B
					for (int i = 0; i < 3; i++)
						*outputIterator++ = *lut3DFloat16Iterator++;

					// A
					*outputIterator++ = oneFloat16;
				}

				outputRowIterator += lockedBox.RowPitch;
			}

			outputSliceIterator += lockedBox.SlicePitch;
		}

		hr = m_pLut3DTexture->UnlockBox(0);
		delete[] lut3DFloat16;
		if (FAILED(hr))
		{
			CleanupFinalPass();
			return hr;
		}
	}

	// Compile the final pixel shader
	if (m_Caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
	{
		CleanupFinalPass();
		return E_FAIL;
	}

	LPCSTR pProfile = m_Caps.PixelShaderVersion >= D3DPS_VERSION(3, 0) ? "ps_3_0" : "ps_2_0";

	CStringA shaderSourceCode;
	if (!LoadResource(IDF_SHADER_FINAL, shaderSourceCode, _T("FILE")))
	{
		CleanupFinalPass();
		return E_FAIL;
	}

	int quantization;
	if (m_bHighColorResolution && (m_DisplayType == D3DFMT_A2R10G10B10 || m_DisplayType == D3DFMT_A2B10G10R10))
		quantization = 1023; // 10-bit
	else
		quantization = 255;  // 8-bit

	CStringA quantizationString;
	quantizationString.Format("%d.0f", quantization);
	shaderSourceCode.Replace("_QUANTIZATION_VALUE_", quantizationString);

	CStringA lut3DEnabledString;
	lut3DEnabledString.Format("%d", static_cast<int>(bColorManagement));
	shaderSourceCode.Replace("_LUT3D_ENABLED_VALUE_", lut3DEnabledString);

	if (bColorManagement)
	{
		CStringA lut3DSizeString;
		lut3DSizeString.Format("%d.0f", m_Lut3DSize);
		shaderSourceCode.Replace("_LUT3D_SIZE_VALUE_", lut3DSizeString);
	}

	CString ErrorMessage;
	CString DissAssembly;
	hr = m_pPSC->CompileShader(shaderSourceCode, "main", pProfile, 0, &m_pFinalPixelShader, &DissAssembly, &ErrorMessage);
	if (FAILED(hr))
	{
		TRACE("%ws", ErrorMessage.GetString());
		ASSERT (0);
		CleanupFinalPass();
		return hr;
	}

	return S_OK;
}

void CDX9RenderingEngine::CleanupFinalPass()
{
	m_bFinalPass = false;
	m_pDitherTexture = NULL;
	m_pLut3DTexture = NULL;
	m_pFinalPixelShader = NULL;
}

HRESULT CDX9RenderingEngine::CreateIccProfileLut(TCHAR* profilePath, float* lut3D)
{
	// Get the input video system
	VideoSystem videoSystem;

	if (m_InputVideoSystem == VIDEO_SYSTEM_UNKNOWN)
	{
		static const int ntscSizes[][2] = {{720, 480}, {720, 486}, {704, 480}};
		static const int palSizes[][2] = {{720, 576}, {704, 576}};

		videoSystem = VIDEO_SYSTEM_HDTV; // default

		for (int i = 0; i < countof(ntscSizes); i++)
		{
			if (m_NativeVideoSize.cx == ntscSizes[i][0] && m_NativeVideoSize.cy == ntscSizes[i][1])
				videoSystem = VIDEO_SYSTEM_SDTV_NTSC;
		}

		for (int i = 0; i < countof(palSizes); i++)
		{
			if (m_NativeVideoSize.cx == palSizes[i][0] && m_NativeVideoSize.cy == palSizes[i][1])
				videoSystem = VIDEO_SYSTEM_SDTV_PAL;
		}
	}
	else
	{
		videoSystem = m_InputVideoSystem;
	}

	// Get the gamma
	double gamma;

	switch (m_Gamma)
	{
	case GAMMA_CURVE_2_2:
		gamma = 2.2;
		break;

	case GAMMA_CURVE_2_3:
		gamma = 2.3;
		break;

	// Recommended by many (e.g., EBU, Poynton)
	case GAMMA_CURVE_2_35:
		gamma = 2.35;
		break;

	case GAMMA_CURVE_2_4:
		gamma = 2.4;
		break;

	default:
		return E_FAIL;
	}

	// Get the rendering intent
	cmsUInt32Number intent;

	switch (m_RenderingIntent)
	{
	case COLOR_RENDERING_INTENT_PERCEPTUAL:
		intent = INTENT_PERCEPTUAL;
		break;

	case COLOR_RENDERING_INTENT_RELATIVE_COLORIMETRIC:
		intent = INTENT_RELATIVE_COLORIMETRIC;
		break;

	case COLOR_RENDERING_INTENT_SATURATION:
		intent = INTENT_SATURATION;
		break;

	case COLOR_RENDERING_INTENT_ABSOLUTE_COLORIMETRIC:
		intent = INTENT_ABSOLUTE_COLORIMETRIC;
		break;

	default:
		return E_FAIL;
	}

	// Set the input white point. It's D65 in all cases.
	cmsCIExyY whitePoint;

	whitePoint.x = 0.3127;
	whitePoint.y = 0.3290;
	whitePoint.Y = 1.0;

	// Set the input primaries
	cmsCIExyYTRIPLE primaries;

	switch (videoSystem)
	{
	case VIDEO_SYSTEM_HDTV:
		// Rec. 709
		primaries.Red.x   = 0.64;
		primaries.Red.y   = 0.33;
		primaries.Green.x = 0.30;
		primaries.Green.y = 0.60;
		primaries.Blue.x  = 0.15;
		primaries.Blue.y  = 0.06;
		break;

	case VIDEO_SYSTEM_SDTV_NTSC:
		// SMPTE-C
		primaries.Red.x   = 0.630;
		primaries.Red.y   = 0.340;
		primaries.Green.x = 0.310;
		primaries.Green.y = 0.595;
		primaries.Blue.x  = 0.155;
		primaries.Blue.y  = 0.070;
		break;

	case VIDEO_SYSTEM_SDTV_PAL:
		// PAL/SECAM
		primaries.Red.x   = 0.64;
		primaries.Red.y   = 0.33;
		primaries.Green.x = 0.29;
		primaries.Green.y = 0.60;
		primaries.Blue.x  = 0.15;
		primaries.Blue.y  = 0.06;
		break;

	default:
		return E_FAIL;
	}

	primaries.Red.Y   = 1.0;
	primaries.Green.Y = 1.0;
	primaries.Blue.Y  = 1.0;

	// Set the input gamma, which is the gamma of a reference studio display we want to simulate
	// For more information, see the paper at http://www.poynton.com/notes/PU-PR-IS/Poynton-PU-PR-IS.pdf
	cmsToneCurve* transferFunction = cmsBuildGamma(0, gamma);

	cmsToneCurve* transferFunctionRGB[3];
	for (int i = 0; i < 3; i++)
		transferFunctionRGB[i] = transferFunction;

	// Create the input profile
	cmsHPROFILE hInputProfile = cmsCreateRGBProfile(&whitePoint, &primaries, transferFunctionRGB);
	cmsFreeToneCurve(transferFunction);

	if (hInputProfile == NULL)
		return E_FAIL;

	// Open the output profile
	cmsHPROFILE hOutputProfile;
	FILE* outputProfileStream;

	if (profilePath != 0)
	{
		if (_wfopen_s(&outputProfileStream, T2W(profilePath), L"rb") != 0)
		{
			cmsCloseProfile(hInputProfile);
			return E_FAIL;
		}

		hOutputProfile = cmsOpenProfileFromStream(outputProfileStream, "r");
	}
	else
	{
		hOutputProfile = cmsCreate_sRGBProfile();
	}

	if (hOutputProfile == NULL)
	{
		if (profilePath != 0)
			fclose(outputProfileStream);

		cmsCloseProfile(hInputProfile);
		return E_FAIL;
	}

	// Create the transform
	cmsHTRANSFORM hTransform = cmsCreateTransform(hInputProfile, TYPE_RGB_16, hOutputProfile, TYPE_RGB_16, intent, cmsFLAGS_HIGHRESPRECALC);

	cmsCloseProfile(hOutputProfile);

	if (profilePath != 0)
		fclose(outputProfileStream);

	cmsCloseProfile(hInputProfile);

	if (hTransform == NULL)
		return E_FAIL;

	// Create the 3D LUT input
	unsigned short* lut3DOutput = new unsigned short[m_Lut3DEntryCount * 3];
	unsigned short* lut3DInput  = new unsigned short[m_Lut3DEntryCount * 3];

	unsigned short* lut3DInputIterator = lut3DInput;

	for (int b = 0; b < m_Lut3DSize; b++)
	{
		for (int g = 0; g < m_Lut3DSize; g++)
		{
			for (int r = 0; r < m_Lut3DSize; r++)
			{
				*lut3DInputIterator++ = r * 65535 / (m_Lut3DSize - 1);
				*lut3DInputIterator++ = g * 65535 / (m_Lut3DSize - 1);
				*lut3DInputIterator++ = b * 65535 / (m_Lut3DSize - 1);
			}
		}
	}

	// Do the transform
	cmsDoTransform(hTransform, lut3DInput, lut3DOutput, m_Lut3DEntryCount);

	// Convert the output to floating point
	for (int i = 0; i < m_Lut3DEntryCount * 3; i++)
		lut3D[i] = static_cast<float>(lut3DOutput[i]) * (1.0f / 65535.0f);

	// Cleanup
	delete[] lut3DOutput;
	delete[] lut3DInput;
	cmsDeleteTransform(hTransform);

	return S_OK;
}

HRESULT CDX9RenderingEngine::FinalPass(IDirect3DTexture9* pTexture)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if (!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
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

	hr = m_pD3DDev->SetPixelShader(m_pFinalPixelShader);

	// Set sampler: image
	hr = m_pD3DDev->SetTexture(0, pTexture);

	// Set sampler: ditherMap
	hr = m_pD3DDev->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	hr = m_pD3DDev->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	hr = m_pD3DDev->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	hr = m_pD3DDev->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	hr = m_pD3DDev->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	hr = m_pD3DDev->SetTexture(1, m_pDitherTexture);

	if (m_bColorManagement)
	{
		hr = m_pD3DDev->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		hr = m_pD3DDev->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		hr = m_pD3DDev->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		hr = m_pD3DDev->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		hr = m_pD3DDev->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		hr = m_pD3DDev->SetSamplerState(2, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);

		hr = m_pD3DDev->SetTexture(2, m_pLut3DTexture);
	}

	// Set constants
	float fConstData[][4] = {{(float)w / DITHER_MATRIX_SIZE, (float)h / DITHER_MATRIX_SIZE, 0, 0}};
	hr = m_pD3DDev->SetPixelShaderConstantF(0, (float*)fConstData, countof(fConstData));

	hr = TextureBlt(m_pD3DDev, v, D3DTEXF_POINT);

	hr = m_pD3DDev->SetTexture(1, NULL);

	if (m_bColorManagement)
		hr = m_pD3DDev->SetTexture(2, NULL);

	return hr;
}

HRESULT CDX9RenderingEngine::TextureCopy(IDirect3DTexture9* pTexture)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if (!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc)))
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

	for (int i = 0; i < countof(v); i++)
	{
		v[i].x -= 0.5;
		v[i].y -= 0.5;
	}

	hr = m_pD3DDev->SetTexture(0, pTexture);

	return TextureBlt(m_pD3DDev, v, D3DTEXF_LINEAR);
}

bool CDX9RenderingEngine::ClipToSurface(IDirect3DSurface9* pSurface, CRect& s, CRect& d)
{
	D3DSURFACE_DESC d3dsd;
	ZeroMemory(&d3dsd, sizeof(d3dsd));
	if (FAILED(pSurface->GetDesc(&d3dsd)))
		return(false);

	int w = d3dsd.Width, h = d3dsd.Height;
	int sw = s.Width(), sh = s.Height();
	int dw = d.Width(), dh = d.Height();

	if (d.left >= w || d.right < 0 || d.top >= h || d.bottom < 0
			|| sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0)
	{
		s.SetRectEmpty();
		d.SetRectEmpty();
		return(true);
	}

	if (d.right > w)
	{
		s.right -= (d.right-w)*sw/dw;
		d.right = w;
	}
	if (d.bottom > h)
	{
		s.bottom -= (d.bottom-h)*sh/dh;
		d.bottom = h;
	}
	if (d.left < 0)
	{
		s.left += (0-d.left)*sw/dw;
		d.left = 0;
	}
	if (d.top < 0)
	{
		s.top += (0-d.top)*sh/dh;
		d.top = 0;
	}

	return(true);
}

HRESULT CDX9RenderingEngine::DrawRect(DWORD _Color, DWORD _Alpha, const CRect &_Rect)
{
	if (!m_pD3DDev)
		return E_POINTER;

	DWORD Color = D3DCOLOR_ARGB(_Alpha, GetRValue(_Color), GetGValue(_Color), GetBValue(_Color));
	MYD3DVERTEX<0> v[] =
	{
		{float(_Rect.left), float(_Rect.top), 0.5f, 2.0f, Color},
		{float(_Rect.right), float(_Rect.top), 0.5f, 2.0f, Color},
		{float(_Rect.left), float(_Rect.bottom), 0.5f, 2.0f, Color},
		{float(_Rect.right), float(_Rect.bottom), 0.5f, 2.0f, Color},
	};

	for (int i = 0; i < countof(v); i++)
	{
		v[i].x -= 0.5;
		v[i].y -= 0.5;
	}

	HRESULT hr = m_pD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	hr = m_pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = m_pD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
	hr = m_pD3DDev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	hr = m_pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	hr = m_pD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	hr = m_pD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//D3DRS_COLORVERTEX
	hr = m_pD3DDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	hr = m_pD3DDev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);


	hr = m_pD3DDev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED);

	hr = m_pD3DDev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX0 | D3DFVF_DIFFUSE);
	// hr = m_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(v[0]));

	MYD3DVERTEX<0> tmp = v[2];
	v[2] = v[3];
	v[3] = tmp;
	hr = m_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(v[0]));

	return S_OK;
}

HRESULT CDX9RenderingEngine::AlphaBlt(RECT* pSrc, RECT* pDst, IDirect3DTexture9* pTexture)
{
	if(!pSrc || !pDst)
		return E_POINTER;

	CRect src(*pSrc), dst(*pDst);

	HRESULT hr;

	D3DSURFACE_DESC d3dsd;
	ZeroMemory(&d3dsd, sizeof(d3dsd));
	if(FAILED(pTexture->GetLevelDesc(0, &d3dsd)) /*|| d3dsd.Type != D3DRTYPE_TEXTURE*/)
		return E_FAIL;

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

	hr = m_pD3DDev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	hr = m_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertices, sizeof(pVertices[0]));

	//

	m_pD3DDev->SetTexture(0, NULL);

	m_pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, abe);
	m_pD3DDev->SetRenderState(D3DRS_SRCBLEND, sb);
	m_pD3DDev->SetRenderState(D3DRS_DESTBLEND, db);

	return S_OK;
}

HRESULT CDX9RenderingEngine::SetCustomPixelShader(LPCSTR pSrcData, LPCSTR pTarget, bool bScreenSpace)
{
	CAtlList<CExternalPixelShader> *pPixelShaders;
	if (bScreenSpace)
		pPixelShaders = &m_pCustomScreenSpacePixelShaders;
	else
		pPixelShaders = &m_pCustomPixelShaders;

	if (!pSrcData && !pTarget)
	{
		pPixelShaders->RemoveAll();
		m_pD3DDev->SetPixelShader(NULL);
		return S_OK;
	}

	if (!pSrcData || !pTarget)
		return E_INVALIDARG;

	CExternalPixelShader Shader;
	Shader.m_SourceData = pSrcData;
	Shader.m_SourceTarget = pTarget;

	CComPtr<IDirect3DPixelShader9> pPixelShader;

	HRESULT hr = Shader.Compile(m_pPSC);
	if (FAILED(hr))
		return hr;

	pPixelShaders->AddTail(Shader);

	Paint(false);

	return S_OK;
}
