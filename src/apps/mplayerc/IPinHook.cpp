/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2007 see AUTHORS
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

#include <d3dx9.h>
#include <dxva2api.h>

#include "IPinHook.h"
#include "DX9AllocatorPresenter.h"

#define LOG_FILE				_T("dxva.log")

REFERENCE_TIME g_tSegmentStart = 0;
REFERENCE_TIME g_tSampleStart = 0;

static HRESULT (STDMETHODCALLTYPE * NewSegmentOrg)(IPinC * This, /* [in] */ REFERENCE_TIME tStart, /* [in] */ REFERENCE_TIME tStop, /* [in] */ double dRate) = NULL;

static HRESULT STDMETHODCALLTYPE NewSegmentMine(IPinC * This, /* [in] */ REFERENCE_TIME tStart, /* [in] */ REFERENCE_TIME tStop, /* [in] */ double dRate)
{
	g_tSegmentStart = tStart;
	return NewSegmentOrg(This, tStart, tStop, dRate);
}

static HRESULT ( STDMETHODCALLTYPE *ReceiveOrg )( IMemInputPinC * This, IMediaSample *pSample) = NULL;

static HRESULT STDMETHODCALLTYPE ReceiveMineI(IMemInputPinC * This, IMediaSample *pSample)
{
	REFERENCE_TIME rtStart, rtStop;
	if(pSample && SUCCEEDED(pSample->GetTime(&rtStart, &rtStop)))
		g_tSampleStart = rtStart;
	return ReceiveOrg(This, pSample);
}

static HRESULT STDMETHODCALLTYPE ReceiveMine(IMemInputPinC * This, IMediaSample *pSample)
{
	// Support ffdshow queueing.
	// To avoid black out on pause, we have to lock g_ffdshowReceive to synchronize with CMainFrame::OnPlayPause.
	if(queueu_ffdshow_support)
	{
		CAutoLock lck(&g_ffdshowReceive);
		return ReceiveMineI(This,pSample);
	}
	return ReceiveMineI(This,pSample);
}

IPinCVtbl*				g_pPinCVtbl			= NULL;
IMemInputPinCVtbl*		g_pMemInputPinCVtbl = NULL;

bool HookNewSegmentAndReceive(IPinC* pPinC, IMemInputPinC* pMemInputPinC)
{
	if(!pPinC || !pMemInputPinC || (GetVersion()&0x80000000))
		return false;

	g_tSegmentStart = 0;
	g_tSampleStart = 0;

	BOOL res;
	DWORD flOldProtect = 0;

	// Casimir666 : unhook previous VTables
	if (g_pPinCVtbl && g_pMemInputPinCVtbl)
	{
		res = VirtualProtect(g_pPinCVtbl, sizeof(IPinCVtbl), PAGE_WRITECOPY, &flOldProtect);
		if (g_pPinCVtbl->NewSegment == NewSegmentMine)
			g_pPinCVtbl->NewSegment = NewSegmentOrg;
		res = VirtualProtect(g_pPinCVtbl, sizeof(IPinCVtbl), flOldProtect, &flOldProtect);

		res = VirtualProtect(g_pMemInputPinCVtbl, sizeof(IMemInputPinCVtbl), PAGE_WRITECOPY, &flOldProtect);
		if (g_pMemInputPinCVtbl->Receive == ReceiveMine)
			g_pMemInputPinCVtbl->Receive = ReceiveOrg;
		res = VirtualProtect(g_pMemInputPinCVtbl, sizeof(IMemInputPinCVtbl), flOldProtect, &flOldProtect);

		g_pPinCVtbl			= NULL;
		g_pMemInputPinCVtbl = NULL;
		NewSegmentOrg		= NULL;
		ReceiveOrg			= NULL;
	}

	// Casimir666 : change sizeof(IPinC) to sizeof(IPinCVtbl) to fix crash with EVR hack on Vista!
	res = VirtualProtect(pPinC->lpVtbl, sizeof(IPinCVtbl), PAGE_WRITECOPY, &flOldProtect);
	if(NewSegmentOrg == NULL) NewSegmentOrg = pPinC->lpVtbl->NewSegment;
	pPinC->lpVtbl->NewSegment = NewSegmentMine;
	res = VirtualProtect(pPinC->lpVtbl, sizeof(IPinCVtbl), flOldProtect, &flOldProtect);

	// Casimir666 : change sizeof(IMemInputPinC) to sizeof(IMemInputPinCVtbl) to fix crash with EVR hack on Vista!
	res = VirtualProtect(pMemInputPinC->lpVtbl, sizeof(IMemInputPinCVtbl), PAGE_WRITECOPY, &flOldProtect);
	if(ReceiveOrg == NULL) ReceiveOrg = pMemInputPinC->lpVtbl->Receive;
	pMemInputPinC->lpVtbl->Receive = ReceiveMine;
	res = VirtualProtect(pMemInputPinC->lpVtbl, sizeof(IMemInputPinCVtbl), flOldProtect, &flOldProtect);

	g_pPinCVtbl			= pPinC->lpVtbl;
	g_pMemInputPinCVtbl = pMemInputPinC->lpVtbl;

	return true;
}

static HRESULT ( STDMETHODCALLTYPE *GetVideoAcceleratorGUIDsOrg )( IAMVideoAcceleratorC * This,/* [out][in] */ LPDWORD pdwNumGuidsSupported,/* [out][in] */ LPGUID pGuidsSupported) = NULL;
static HRESULT ( STDMETHODCALLTYPE *GetUncompFormatsSupportedOrg )( IAMVideoAcceleratorC * This,/* [in] */ const GUID *pGuid,/* [out][in] */ LPDWORD pdwNumFormatsSupported,/* [out][in] */ LPDDPIXELFORMAT pFormatsSupported) = NULL;
static HRESULT ( STDMETHODCALLTYPE *GetInternalMemInfoOrg )( IAMVideoAcceleratorC * This,/* [in] */ const GUID *pGuid,/* [in] */ const AMVAUncompDataInfo *pamvaUncompDataInfo,/* [out][in] */ LPAMVAInternalMemInfo pamvaInternalMemInfo) = NULL;
static HRESULT ( STDMETHODCALLTYPE *GetCompBufferInfoOrg )( IAMVideoAcceleratorC * This,/* [in] */ const GUID *pGuid,/* [in] */ const AMVAUncompDataInfo *pamvaUncompDataInfo,/* [out][in] */ LPDWORD pdwNumTypesCompBuffers,/* [out] */ LPAMVACompBufferInfo pamvaCompBufferInfo) = NULL;
static HRESULT ( STDMETHODCALLTYPE *GetInternalCompBufferInfoOrg )( IAMVideoAcceleratorC * This,/* [out][in] */ LPDWORD pdwNumTypesCompBuffers,/* [out] */ LPAMVACompBufferInfo pamvaCompBufferInfo) = NULL;
static HRESULT ( STDMETHODCALLTYPE *BeginFrameOrg )( IAMVideoAcceleratorC * This,/* [in] */ const AMVABeginFrameInfo *amvaBeginFrameInfo) = NULL;        
static HRESULT ( STDMETHODCALLTYPE *EndFrameOrg )( IAMVideoAcceleratorC * This,/* [in] */ const AMVAEndFrameInfo *pEndFrameInfo) = NULL;
static HRESULT ( STDMETHODCALLTYPE *GetBufferOrg )( IAMVideoAcceleratorC * This,/* [in] */ DWORD dwTypeIndex,/* [in] */ DWORD dwBufferIndex,/* [in] */ BOOL bReadOnly,/* [out] */ LPVOID *ppBuffer,/* [out] */ LONG *lpStride) = NULL;
static HRESULT ( STDMETHODCALLTYPE *ReleaseBufferOrg )( IAMVideoAcceleratorC * This,/* [in] */ DWORD dwTypeIndex,/* [in] */ DWORD dwBufferIndex) = NULL;
static HRESULT ( STDMETHODCALLTYPE *ExecuteOrg )( IAMVideoAcceleratorC * This,/* [in] */ DWORD dwFunction,/* [in] */ LPVOID lpPrivateInputData,/* [in] */ DWORD cbPrivateInputData,/* [in] */ LPVOID lpPrivateOutputDat,/* [in] */ DWORD cbPrivateOutputData,/* [in] */ DWORD dwNumBuffers,/* [in] */ const AMVABUFFERINFO *pamvaBufferInfo) = NULL;
static HRESULT ( STDMETHODCALLTYPE *QueryRenderStatusOrg )( IAMVideoAcceleratorC * This,/* [in] */ DWORD dwTypeIndex,/* [in] */ DWORD dwBufferIndex,/* [in] */ DWORD dwFlags) = NULL;
static HRESULT ( STDMETHODCALLTYPE *DisplayFrameOrg )( IAMVideoAcceleratorC * This,/* [in] */ DWORD dwFlipToIndex,/* [in] */ IMediaSample *pMediaSample) = NULL;

static void LOG(LPCTSTR fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if(TCHAR* buff = new TCHAR[_vsctprintf(fmt, args) + 1])
	{
		_vstprintf(buff, fmt, args);
		if(FILE* f = _tfopen(LOG_FILE, _T("at")))
		{
			fseek(f, 0, 2);
			_ftprintf(f, _T("%s\n"), buff);
			fclose(f);
		}
		delete [] buff;
	}
	va_end(args);
}

static void LOGPF(LPCTSTR prefix, const DDPIXELFORMAT* p, int n)
{
	for(int i = 0; i < n; i++)
	{
		LOG(_T("%s[%d].dwSize = %d"), prefix, i, p[i].dwSize);
		LOG(_T("%s[%d].dwFlags = %08x"), prefix, i, p[i].dwFlags);
		LOG(_T("%s[%d].dwFourCC = %4.4hs"), prefix, i, &p[i].dwFourCC);
		LOG(_T("%s[%d].dwRGBBitCount = %08x"), prefix, i, &p[i].dwRGBBitCount);
		LOG(_T("%s[%d].dwRBitMask = %08x"), prefix, i, &p[i].dwRBitMask);
		LOG(_T("%s[%d].dwGBitMask = %08x"), prefix, i, &p[i].dwGBitMask);
		LOG(_T("%s[%d].dwBBitMask = %08x"), prefix, i, &p[i].dwBBitMask);
		LOG(_T("%s[%d].dwRGBAlphaBitMask = %08x"), prefix, i, &p[i].dwRGBAlphaBitMask);
	}
}

static void LOGUDI(LPCTSTR prefix, const AMVAUncompDataInfo* p, int n)
{
	for(int i = 0; i < n; i++)
	{
		LOG(_T("%s[%d].dwUncompWidth = %d"), prefix, i, p[i].dwUncompWidth);
		LOG(_T("%s[%d].dwUncompHeight = %d"), prefix, i, p[i].dwUncompHeight);

		CString prefix2;
		prefix2.Format(_T("%s[%d]"), prefix, i);
		LOGPF(prefix2, &p[i].ddUncompPixelFormat, 1);
	}
}

static HRESULT STDMETHODCALLTYPE GetVideoAcceleratorGUIDsMine(
	IAMVideoAcceleratorC * This,
	/* [out][in] */ LPDWORD pdwNumGuidsSupported,
	/* [out][in] */ LPGUID pGuidsSupported)
{
	LOG(_T("\nGetVideoAcceleratorGUIDs"));

	if(pdwNumGuidsSupported)
	{
		LOG(_T("[in] *pdwNumGuidsSupported = %d"), *pdwNumGuidsSupported);
	}

	HRESULT hr = GetVideoAcceleratorGUIDsOrg(This, pdwNumGuidsSupported, pGuidsSupported);

	LOG(_T("hr = %08x"), hr);

	if(pdwNumGuidsSupported)
	{
		LOG(_T("[out] *pdwNumGuidsSupported = %d"), *pdwNumGuidsSupported);

		if(pGuidsSupported)
		{
			for(int i = 0; i < *pdwNumGuidsSupported; i++)
			{
				LOG(_T("[out] pGuidsSupported[%d] = %s"), i, CStringFromGUID(pGuidsSupported[i]));
			}
		}
	}

	return hr;
}

static HRESULT STDMETHODCALLTYPE GetUncompFormatsSupportedMine(
	IAMVideoAcceleratorC * This,
	/* [in] */ const GUID *pGuid,
	/* [out][in] */ LPDWORD pdwNumFormatsSupported,
	/* [out][in] */ LPDDPIXELFORMAT pFormatsSupported)
{
	LOG(_T("\nGetUncompFormatsSupported"));

	if(pGuid)
	{
		LOG(_T("[in] *pGuid = %s"), CStringFromGUID(*pGuid));
	}

	if(pdwNumFormatsSupported)
	{
		LOG(_T("[in] *pdwNumFormatsSupported = %d"), *pdwNumFormatsSupported);
	}

	HRESULT hr = GetUncompFormatsSupportedOrg(This, pGuid, pdwNumFormatsSupported, pFormatsSupported);

	LOG(_T("hr = %08x"), hr);

	if(pdwNumFormatsSupported)
	{
		LOG(_T("[out] *pdwNumFormatsSupported = %d"), *pdwNumFormatsSupported);

		if(pFormatsSupported)
		{
			LOGPF(_T("[out] pFormatsSupported"), pFormatsSupported, *pdwNumFormatsSupported);
		}
	}

	return hr;
}

static HRESULT STDMETHODCALLTYPE GetInternalMemInfoMine(
	IAMVideoAcceleratorC * This,
	/* [in] */ const GUID *pGuid,
	/* [in] */ const AMVAUncompDataInfo *pamvaUncompDataInfo,
	/* [out][in] */ LPAMVAInternalMemInfo pamvaInternalMemInfo)
{
	LOG(_T("\nGetInternalMemInfo"));

	HRESULT hr = GetInternalMemInfoOrg(This, pGuid, pamvaUncompDataInfo, pamvaInternalMemInfo);

	LOG(_T("hr = %08x"), hr);

	return hr;
}

static HRESULT STDMETHODCALLTYPE GetCompBufferInfoMine(
	IAMVideoAcceleratorC * This,
	/* [in] */ const GUID *pGuid,
	/* [in] */ const AMVAUncompDataInfo *pamvaUncompDataInfo,
	/* [out][in] */ LPDWORD pdwNumTypesCompBuffers,
	/* [out] */ LPAMVACompBufferInfo pamvaCompBufferInfo)
{
	LOG(_T("\nGetCompBufferInfo"));

	if(pGuid)
	{
		LOG(_T("[in] *pGuid = %s"), CStringFromGUID(*pGuid));

		if(pdwNumTypesCompBuffers)
		{
			LOG(_T("[in] *pdwNumTypesCompBuffers = %d"), *pdwNumTypesCompBuffers);
		}
	}

	HRESULT hr = GetCompBufferInfoOrg(This, pGuid, pamvaUncompDataInfo, pdwNumTypesCompBuffers, pamvaCompBufferInfo);

	LOG(_T("hr = %08x"), hr);

	if(pdwNumTypesCompBuffers)
	{
		LOG(_T("[out] *pdwNumTypesCompBuffers = %d"), *pdwNumTypesCompBuffers);

		if(pamvaUncompDataInfo)
		{
			LOGUDI(_T("[out] pamvaUncompDataInfo"), pamvaUncompDataInfo, *pdwNumTypesCompBuffers);
		}
	}

	return hr;
}

static HRESULT STDMETHODCALLTYPE GetInternalCompBufferInfoMine(
	IAMVideoAcceleratorC * This,
	/* [out][in] */ LPDWORD pdwNumTypesCompBuffers,
	/* [out] */ LPAMVACompBufferInfo pamvaCompBufferInfo)
{
	LOG(_T("\nGetInternalCompBufferInfo"));

	HRESULT hr = GetInternalCompBufferInfoOrg(This, pdwNumTypesCompBuffers, pamvaCompBufferInfo);

	LOG(_T("hr = %08x"), hr);

	return hr;
}

static HRESULT STDMETHODCALLTYPE BeginFrameMine(
	IAMVideoAcceleratorC * This,
	/* [in] */ const AMVABeginFrameInfo *amvaBeginFrameInfo)
{
	LOG(_T("\nBeginFrame"));

	if(amvaBeginFrameInfo)
	{
		LOG(_T("[in] amvaBeginFrameInfo->dwDestSurfaceIndex = %08x"), amvaBeginFrameInfo->dwDestSurfaceIndex);
		LOG(_T("[in] amvaBeginFrameInfo->pInputData = %08x"), amvaBeginFrameInfo->pInputData);
		LOG(_T("[in] amvaBeginFrameInfo->dwSizeInputData = %08x"), amvaBeginFrameInfo->dwSizeInputData);
		LOG(_T("[in] amvaBeginFrameInfo->pOutputData = %08x"), amvaBeginFrameInfo->pOutputData);
		LOG(_T("[in] amvaBeginFrameInfo->dwSizeOutputData = %08x"), amvaBeginFrameInfo->dwSizeOutputData);
	}

	HRESULT hr = BeginFrameOrg(This, amvaBeginFrameInfo);

	LOG(_T("hr = %08x"), hr);

	if(amvaBeginFrameInfo && amvaBeginFrameInfo->pOutputData)
	{
		LOG(_T("[out] amvaBeginFrameInfo->pOutputData = %02x %02x %02x %02x..."), 
			((BYTE*)amvaBeginFrameInfo->pOutputData)[0],
			((BYTE*)amvaBeginFrameInfo->pOutputData)[1],
			((BYTE*)amvaBeginFrameInfo->pOutputData)[2],
			((BYTE*)amvaBeginFrameInfo->pOutputData)[3]);
	}

	return hr;
}
        
static HRESULT STDMETHODCALLTYPE EndFrameMine(
	IAMVideoAcceleratorC * This,
	/* [in] */ const AMVAEndFrameInfo *pEndFrameInfo)
{
	LOG(_T("\nEndFrame"));

	if(pEndFrameInfo)
	{
		LOG(_T("[in] pEndFrameInfo->dwSizeMiscData = %08x"), pEndFrameInfo->dwSizeMiscData);
		LOG(_T("[in] pEndFrameInfo->pMiscData = %08x"), pEndFrameInfo->pMiscData);
	}

	HRESULT hr = EndFrameOrg(This, pEndFrameInfo);

	LOG(_T("hr = %08x"), hr);

	return hr;
}

static HRESULT STDMETHODCALLTYPE GetBufferMine(
	IAMVideoAcceleratorC * This,
	/* [in] */ DWORD dwTypeIndex,
	/* [in] */ DWORD dwBufferIndex,
	/* [in] */ BOOL bReadOnly,
	/* [out] */ LPVOID *ppBuffer,
	/* [out] */ LONG *lpStride)
{
	LOG(_T("\nGetBuffer"));

	LOG(_T("[in] dwTypeIndex = %08x"), dwTypeIndex);
	LOG(_T("[in] dwBufferIndex = %08x"), dwBufferIndex);
	LOG(_T("[in] bReadOnly = %08x"), bReadOnly);	
	LOG(_T("[in] ppBuffer = %08x"), ppBuffer);
	LOG(_T("[in] lpStride = %08x"), lpStride);

	HRESULT hr = GetBufferOrg(This, dwTypeIndex, dwBufferIndex, bReadOnly, ppBuffer, lpStride);

	LOG(_T("hr = %08x"), hr);

	LOG(_T("[out] *ppBuffer = %02x %02x %02x %02x ..."), ((BYTE*)*ppBuffer)[0], ((BYTE*)*ppBuffer)[1], ((BYTE*)*ppBuffer)[2], ((BYTE*)*ppBuffer)[3]);
	LOG(_T("[out] *lpStride = %08x"), *lpStride);

	return hr;
}

static HRESULT STDMETHODCALLTYPE ReleaseBufferMine(
	IAMVideoAcceleratorC * This,
	/* [in] */ DWORD dwTypeIndex,
	/* [in] */ DWORD dwBufferIndex)
{
	LOG(_T("\nReleaseBuffer"));

	LOG(_T("[in] dwTypeIndex = %08x"), dwTypeIndex);
	LOG(_T("[in] dwBufferIndex = %08x"), dwBufferIndex);

	HRESULT hr = ReleaseBufferOrg(This, dwTypeIndex, dwBufferIndex);

	LOG(_T("hr = %08x"), hr);

	return hr;
}

static HRESULT STDMETHODCALLTYPE ExecuteMine(
	IAMVideoAcceleratorC * This,
	/* [in] */ DWORD dwFunction,
	/* [in] */ LPVOID lpPrivateInputData,
	/* [in] */ DWORD cbPrivateInputData,
	/* [in] */ LPVOID lpPrivateOutputData,
	/* [in] */ DWORD cbPrivateOutputData,
	/* [in] */ DWORD dwNumBuffers,
	/* [in] */ const AMVABUFFERINFO *pamvaBufferInfo)
{
	LOG(_T("\nExecute"));

	LOG(_T("[in] dwFunction = %08x"), dwFunction);
	if(lpPrivateInputData)
	{
		LOG(_T("[in] lpPrivateInputData = %02x %02x %02x %02x ..."), 
			((BYTE*)lpPrivateInputData)[0], 
			((BYTE*)lpPrivateInputData)[1], 
			((BYTE*)lpPrivateInputData)[2], 
			((BYTE*)lpPrivateInputData)[3]);
	}
	LOG(_T("[in] cbPrivateInputData = %08x"), cbPrivateInputData);
	LOG(_T("[in] lpPrivateOutputData = %08"), lpPrivateOutputData);
	LOG(_T("[in] cbPrivateOutputData = %08x"), cbPrivateOutputData);
	LOG(_T("[in] dwNumBuffers = %08x"), dwNumBuffers);
	if(pamvaBufferInfo)
	{
		LOG(_T("[in] pamvaBufferInfo->dwTypeIndex = %08x"), pamvaBufferInfo->dwTypeIndex);
		LOG(_T("[in] pamvaBufferInfo->dwBufferIndex = %08x"), pamvaBufferInfo->dwBufferIndex);
		LOG(_T("[in] pamvaBufferInfo->dwDataOffset = %08x"), pamvaBufferInfo->dwDataOffset);
		LOG(_T("[in] pamvaBufferInfo->dwDataSize = %08x"), pamvaBufferInfo->dwDataSize);
	}

	HRESULT hr = ExecuteOrg(This, dwFunction, lpPrivateInputData, cbPrivateInputData, lpPrivateOutputData, cbPrivateOutputData, dwNumBuffers, pamvaBufferInfo);

	LOG(_T("hr = %08x"), hr);

	if(lpPrivateOutputData)
	{
		LOG(_T("[out] *lpPrivateOutputData = %08"), lpPrivateOutputData);

		if(cbPrivateOutputData)
		{
			LOG(_T("[out] cbPrivateOutputData = %02x %02x %02x %02x ..."), 
				((BYTE*)lpPrivateOutputData)[0], 
				((BYTE*)lpPrivateOutputData)[1], 
				((BYTE*)lpPrivateOutputData)[2], 
				((BYTE*)lpPrivateOutputData)[3]);
		}
	}

	return hr;
}

static HRESULT STDMETHODCALLTYPE QueryRenderStatusMine(
	IAMVideoAcceleratorC * This,
	/* [in] */ DWORD dwTypeIndex,
	/* [in] */ DWORD dwBufferIndex,
	/* [in] */ DWORD dwFlags)
{
	LOG(_T("\nQueryRenderStatus"));

	HRESULT hr = QueryRenderStatusOrg(This, dwTypeIndex, dwBufferIndex, dwFlags);

	LOG(_T("hr = %08x"), hr);

	return hr;
}

static HRESULT STDMETHODCALLTYPE DisplayFrameMine(
	IAMVideoAcceleratorC * This,
	/* [in] */ DWORD dwFlipToIndex,
	/* [in] */ IMediaSample *pMediaSample)
{
	LOG(_T("\nDisplayFrame"));

	HRESULT hr = DisplayFrameOrg(This, dwFlipToIndex, pMediaSample);

	LOG(_T("hr = %08x"), hr);

	return hr;
}

void HookAMVideoAccelerator(IAMVideoAcceleratorC* pAMVideoAcceleratorC)
{
	BOOL res;
	DWORD flOldProtect = 0;
	res = VirtualProtect(pAMVideoAcceleratorC->lpVtbl, sizeof(IAMVideoAcceleratorC), PAGE_WRITECOPY, &flOldProtect);

	if(GetVideoAcceleratorGUIDsOrg == NULL) GetVideoAcceleratorGUIDsOrg = pAMVideoAcceleratorC->lpVtbl->GetVideoAcceleratorGUIDs;
	if(GetUncompFormatsSupportedOrg == NULL) GetUncompFormatsSupportedOrg = pAMVideoAcceleratorC->lpVtbl->GetUncompFormatsSupported;
	if(GetInternalMemInfoOrg == NULL) GetInternalMemInfoOrg = pAMVideoAcceleratorC->lpVtbl->GetInternalMemInfo;
	if(GetCompBufferInfoOrg == NULL) GetCompBufferInfoOrg = pAMVideoAcceleratorC->lpVtbl->GetCompBufferInfo;
	if(GetInternalCompBufferInfoOrg == NULL) GetInternalCompBufferInfoOrg = pAMVideoAcceleratorC->lpVtbl->GetInternalCompBufferInfo;
	if(BeginFrameOrg == NULL) BeginFrameOrg = pAMVideoAcceleratorC->lpVtbl->BeginFrame;
	if(EndFrameOrg == NULL) EndFrameOrg = pAMVideoAcceleratorC->lpVtbl->EndFrame;
	if(GetBufferOrg == NULL) GetBufferOrg = pAMVideoAcceleratorC->lpVtbl->GetBuffer;
	if(ReleaseBufferOrg == NULL) ReleaseBufferOrg = pAMVideoAcceleratorC->lpVtbl->ReleaseBuffer;
	if(ExecuteOrg == NULL) ExecuteOrg = pAMVideoAcceleratorC->lpVtbl->Execute;
	if(QueryRenderStatusOrg == NULL) QueryRenderStatusOrg = pAMVideoAcceleratorC->lpVtbl->QueryRenderStatus;
	if(DisplayFrameOrg == NULL) DisplayFrameOrg = pAMVideoAcceleratorC->lpVtbl->DisplayFrame;

	pAMVideoAcceleratorC->lpVtbl->GetVideoAcceleratorGUIDs = GetVideoAcceleratorGUIDsMine;
	pAMVideoAcceleratorC->lpVtbl->GetUncompFormatsSupported = GetUncompFormatsSupportedMine;
	pAMVideoAcceleratorC->lpVtbl->GetInternalMemInfo = GetInternalMemInfoMine;
	pAMVideoAcceleratorC->lpVtbl->GetCompBufferInfo = GetCompBufferInfoMine;
	pAMVideoAcceleratorC->lpVtbl->GetInternalCompBufferInfo = GetInternalCompBufferInfoMine;
	pAMVideoAcceleratorC->lpVtbl->BeginFrame = BeginFrameMine;
	pAMVideoAcceleratorC->lpVtbl->EndFrame = EndFrameMine;
	pAMVideoAcceleratorC->lpVtbl->GetBuffer = GetBufferMine;
	pAMVideoAcceleratorC->lpVtbl->ReleaseBuffer = ReleaseBufferMine;
	pAMVideoAcceleratorC->lpVtbl->Execute = ExecuteMine;
	pAMVideoAcceleratorC->lpVtbl->QueryRenderStatus = QueryRenderStatusMine;
	pAMVideoAcceleratorC->lpVtbl->DisplayFrame = DisplayFrameMine;

	res = VirtualProtect(pAMVideoAcceleratorC->lpVtbl, sizeof(IAMVideoAcceleratorC), PAGE_EXECUTE, &flOldProtect);
}



// === Hook for DXVA2

typedef struct
{
  const GUID*			Guid;
  const LPCTSTR			Description;
} DXVA2_DECODER;

							   
EXTERN_C const GUID DXVA2_UnknownMode1 = { 0x1720AC81, 0x9D1B,0x4F63,0x9A,0x37,0x4A,0x88,0x48,0x3D,0x0B,0x87};	// nVidia 6600GT

const DXVA2_DECODER DXVA2Decoder[] = 
{
	{ &GUID_NULL,				_T("Not using DXVA2") },
	{ &DXVA2_ModeH264_A,		_T("H.264 motion compensation, no FGT") },
	{ &DXVA2_ModeH264_B,		_T("H.264 motion compensation, FGT") },
	{ &DXVA2_ModeH264_C,		_T("H.264 IDCT, no FGT") },
	{ &DXVA2_ModeH264_D,		_T("H.264 IDCT, FGT") },
	{ &DXVA2_ModeH264_E,		_T("H.264 VLD, no FGT") },
	{ &DXVA2_ModeH264_F,		_T("H.264 variable-length decoder, FGT") },
	{ &DXVA2_ModeMPEG2_IDCT,	_T("MPEG-2 IDCT") },
	{ &DXVA2_ModeMPEG2_MoComp,	_T("MPEG-2 motion compensation") },
	{ &DXVA2_ModeMPEG2_VLD,		_T("MPEG-2 variable-length decoder") },
	{ &DXVA2_ModeVC1_A,			_T("VC-1 post processing") },
	{ &DXVA2_ModeVC1_B,			_T("VC-1 motion compensation") },
	{ &DXVA2_ModeVC1_C,			_T("VC-1 IDCT") },
	{ &DXVA2_ModeVC1_D,			_T("VC-1 VLD") },
	{ &DXVA2_ModeWMV8_A,		_T("WMV 8 post processing.") }, 
	{ &DXVA2_ModeWMV8_B,		_T("WMV8 motion compensation") },
	{ &DXVA2_ModeWMV9_A,		_T("WMV9 post processing") },
	{ &DXVA2_ModeWMV9_B,		_T("WMV9 motion compensation") },
	{ &DXVA2_ModeWMV9_C,		_T("WMV9 IDCT.") },

	{ &DXVA2_UnknownMode1,		_T("Unknown : 1720AC81-9D1B-4F63-9A37-4A88483D0B87") },
};
 
int		m_nCurrentDXVA2Decoder = 0;

int FindDXVA2Decoder(const GUID& DecoderGuid)
{
	for (int i=0; i<countof(DXVA2Decoder); i++)
		if (DecoderGuid == *DXVA2Decoder[i].Guid)
			return i;

	return 0;
}

LPCTSTR GetDXVA2DecoderDescription()
{
	return DXVA2Decoder[m_nCurrentDXVA2Decoder].Description;
}


interface IDirectXVideoDecoderServiceC;
typedef struct IDirectXVideoDecoderServiceCVtbl
{
	BEGIN_INTERFACE
	HRESULT ( STDMETHODCALLTYPE *QueryInterface )( IDirectXVideoDecoderServiceC* pThis, /* [in] */ REFIID riid, /* [iid_is][out] */ void **ppvObject );
	ULONG ( STDMETHODCALLTYPE *AddRef )(IDirectXVideoDecoderServiceC* pThis);
	ULONG ( STDMETHODCALLTYPE *Release )(IDirectXVideoDecoderServiceC*	pThis);
	HRESULT (STDMETHODCALLTYPE* CreateSurface)(IDirectXVideoDecoderServiceC* pThis, __in  UINT Width, __in  UINT Height, __in  UINT BackBuffers, __in  D3DFORMAT Format, __in  D3DPOOL Pool, __in  DWORD Usage, __in  DWORD DxvaType, __out_ecount(BackBuffers+1)  IDirect3DSurface9 **ppSurface, __inout_opt  HANDLE *pSharedHandle);

    HRESULT (STDMETHODCALLTYPE* GetDecoderDeviceGuids)(
		IDirectXVideoDecoderServiceC*					pThis,
        __out  UINT*									pCount,
        __deref_out_ecount_opt(*pCount)  GUID**			pGuids);
    
    HRESULT (STDMETHODCALLTYPE* GetDecoderRenderTargets)( 
		IDirectXVideoDecoderServiceC*					pThis,
        __in  REFGUID									Guid,
        __out  UINT*									pCount,
        __deref_out_ecount_opt(*pCount)  D3DFORMAT**	pFormats);
    
    HRESULT (STDMETHODCALLTYPE* GetDecoderConfigurations)( 
		IDirectXVideoDecoderServiceC*					pThis,
        __in  REFGUID									Guid,
        __in  const DXVA2_VideoDesc*					pVideoDesc,
        __reserved  void*								pReserved,
        __out  UINT*									pCount,
        __deref_out_ecount_opt(*pCount)  DXVA2_ConfigPictureDecode **ppConfigs);
    
    HRESULT (STDMETHODCALLTYPE* CreateVideoDecoder)( 
		IDirectXVideoDecoderServiceC*					pThis,
        __in  REFGUID									Guid,
        __in  const DXVA2_VideoDesc*					pVideoDesc,
        __in  const DXVA2_ConfigPictureDecode*			pConfig,
        __in_ecount(NumRenderTargets)  IDirect3DSurface9 **ppDecoderRenderTargets,
        __in  UINT										NumRenderTargets,
        __deref_out  IDirectXVideoDecoder**				ppDecode);

	END_INTERFACE
};

interface IDirectXVideoDecoderServiceC
{
	CONST_VTBL struct IDirectXVideoDecoderServiceCVtbl *lpVtbl;
};


IDirectXVideoDecoderServiceCVtbl*	g_pIDirectXVideoDecoderServiceCVtbl;
static HRESULT (STDMETHODCALLTYPE* CreateVideoDecoderOrg )  (IDirectXVideoDecoderServiceC* pThis, __in  REFGUID Guid, __in  const DXVA2_VideoDesc* pVideoDesc, __in  const DXVA2_ConfigPictureDecode* pConfig, __in_ecount(NumRenderTargets)  IDirect3DSurface9 **ppDecoderRenderTargets, __in  UINT NumRenderTargets, __deref_out  IDirectXVideoDecoder** ppDecode) = NULL;
static HRESULT (STDMETHODCALLTYPE* GetDecoderDeviceGuidsOrg)(IDirectXVideoDecoderServiceC* pThis, __out  UINT* pCount, __deref_out_ecount_opt(*pCount)  GUID** pGuids) = NULL;


static void LogDXVA2Config (const DXVA2_ConfigPictureDecode* pConfig)
{
	LOG(_T("Config"));
	LOG(_T("	- Config4GroupedCoefs               %d"), pConfig->Config4GroupedCoefs);
	LOG(_T("	- ConfigBitstreamRaw                %d"), pConfig->ConfigBitstreamRaw);
	LOG(_T("	- ConfigDecoderSpecific             %d"), pConfig->ConfigDecoderSpecific);
	LOG(_T("	- ConfigHostInverseScan             %d"), pConfig->ConfigHostInverseScan);
	LOG(_T("	- ConfigIntraResidUnsigned          %d"), pConfig->ConfigIntraResidUnsigned);
	LOG(_T("	- ConfigMBcontrolRasterOrder        %d"), pConfig->ConfigMBcontrolRasterOrder);
	LOG(_T("	- ConfigMinRenderTargetBuffCount    %d"), pConfig->ConfigMinRenderTargetBuffCount);
	LOG(_T("	- ConfigResid8Subtraction           %d"), pConfig->ConfigResid8Subtraction);
	LOG(_T("	- ConfigResidDiffAccelerator        %d"), pConfig->ConfigResidDiffAccelerator);
	LOG(_T("	- ConfigResidDiffHost               %d"), pConfig->ConfigResidDiffHost);
	LOG(_T("	- ConfigSpatialHost8or9Clipping     %d"), pConfig->ConfigSpatialHost8or9Clipping);
	LOG(_T("	- ConfigSpatialResid8               %d"), pConfig->ConfigSpatialResid8);
	LOG(_T("	- ConfigSpatialResidInterleaved     %d"), pConfig->ConfigSpatialResidInterleaved);
	LOG(_T("	- ConfigSpecificIDCT                %d"), pConfig->ConfigSpecificIDCT);
	LOG(_T("	- guidConfigBitstreamEncryption     %s"), CStringFromGUID(pConfig->guidConfigBitstreamEncryption));
	LOG(_T("	- guidConfigMBcontrolEncryption     %s"), CStringFromGUID(pConfig->guidConfigMBcontrolEncryption));
	LOG(_T("	- guidConfigResidDiffEncryption     %s"), CStringFromGUID(pConfig->guidConfigResidDiffEncryption));
}

static void LogDXVA2VideoDesc (const DXVA2_VideoDesc* pVideoDesc)
{
	LOG(_T("VideoDesc"));
	LOG(_T("	- Format                            %d"), pVideoDesc->Format);
	LOG(_T("	- InputSampleFreq                   %d/%d"), pVideoDesc->InputSampleFreq.Numerator, pVideoDesc->InputSampleFreq.Denominator);
	LOG(_T("	- OutputFrameFreq                   %d/%d"), pVideoDesc->OutputFrameFreq.Numerator, pVideoDesc->OutputFrameFreq.Denominator);
	LOG(_T("	- SampleFormat                      %d"), pVideoDesc->SampleFormat);
	LOG(_T("	- SampleHeight                      %d"), pVideoDesc->SampleHeight);
	LOG(_T("	- SampleWidth                       %d"), pVideoDesc->SampleWidth);
	LOG(_T("	- UABProtectionLevel                %d"), pVideoDesc->UABProtectionLevel);
}

static void LogVideoCardCaps(IDirectXVideoDecoderService* pDecoderService)
{
	HRESULT			hr;
    UINT			cDecoderGuids	= 0;
    GUID*			pDecoderGuids	= NULL;

	hr = pDecoderService->GetDecoderDeviceGuids(&cDecoderGuids, &pDecoderGuids);
    if (SUCCEEDED(hr))
    {
        // Look for the decoder GUIDs we want.
        for (UINT iGuid = 0; iGuid < cDecoderGuids; iGuid++)
        {
			int		nSupportedMode = FindDXVA2Decoder (pDecoderGuids[iGuid]);
			
			LOG (_T("=== New mode : %s"), DXVA2Decoder[nSupportedMode].Description);

            // Find a configuration that we support. 
			UINT						cFormats = 0;
			UINT						cConfigurations = 0;
			D3DFORMAT*					pFormats = NULL;
			DXVA2_ConfigPictureDecode*	pConfig = NULL;
			DXVA2_VideoDesc				m_VideoDesc;

			hr = pDecoderService->GetDecoderRenderTargets(pDecoderGuids[iGuid], &cFormats, &pFormats);

			if (SUCCEEDED(hr))
			{
				// Look for a format that matches our output format.
				for (UINT iFormat = 0; iFormat < cFormats;  iFormat++)
				{
					LOG (_T("Direct 3D format : %c%c%c%c"),	TCHAR((pFormats[iFormat]>>0)&0xff), 
															TCHAR((pFormats[iFormat]>>8)&0xff), 
															TCHAR((pFormats[iFormat]>>16)&0xff),
															TCHAR((pFormats[iFormat]>>24)&0xff));
					// Fill in the video description. Set the width, height, format, and frame rate.
					memset(&m_VideoDesc, 0, sizeof(m_VideoDesc));
					m_VideoDesc.SampleWidth		= 1280;
					m_VideoDesc.SampleHeight	= 720;
					m_VideoDesc.Format			= pFormats[iFormat];

					// Get the available configurations.
					hr = pDecoderService->GetDecoderConfigurations(pDecoderGuids[iGuid], &m_VideoDesc, NULL, &cConfigurations, &pConfig);

					if (SUCCEEDED(hr))
					{

						// Find a supported configuration.
						for (UINT iConfig = 0; iConfig < cConfigurations; iConfig++)
						{
							LogDXVA2Config (&pConfig[iConfig]);
						}

						CoTaskMemFree(pConfig);
					}
				}
			}

			LOG(_T("\n"));
			CoTaskMemFree(pFormats);
        }
	}
}

static HRESULT STDMETHODCALLTYPE CreateVideoDecoderMine( 
		IDirectXVideoDecoderServiceC*					pThis,
        __in  REFGUID									Guid,
        __in  const DXVA2_VideoDesc*					pVideoDesc,
        __in  const DXVA2_ConfigPictureDecode*			pConfig,
        __in_ecount(NumRenderTargets)  IDirect3DSurface9 **ppDecoderRenderTargets,
        __in  UINT										NumRenderTargets,
        __deref_out  IDirectXVideoDecoder**				ppDecode)
{
	m_nCurrentDXVA2Decoder = FindDXVA2Decoder (Guid);

	LOG(_T("IDirectXVideoDecoderService::CreateVideoDecoder  %s  (%d render targets)"), DXVA2Decoder[m_nCurrentDXVA2Decoder].Description, NumRenderTargets);

	LogDXVA2VideoDesc(pVideoDesc);
	LogDXVA2Config(pConfig);

	HRESULT hr = CreateVideoDecoderOrg(pThis, Guid, pVideoDesc, pConfig, ppDecoderRenderTargets, NumRenderTargets, ppDecode);

	if (FAILED (hr)) m_nCurrentDXVA2Decoder = 0;
	LOG(_T("hr = %08x\n"), hr);

	return hr;
}



static HRESULT STDMETHODCALLTYPE GetDecoderDeviceGuidsMine (IDirectXVideoDecoderServiceC*					pThis,
														__out  UINT*									pCount,
														__deref_out_ecount_opt(*pCount)  GUID**			pGuids)
{
	LOG(_T("IDirectXVideoDecoderService::GetDecoderDeviceGuids"));

	HRESULT hr = GetDecoderDeviceGuidsOrg(pThis, pCount, pGuids);

	LOG(_T("hr = %08x\n"), hr);

	return hr;
}


void HookDirectXVideoDecoderService(void* pIDirectXVideoDecoderService)
{
	IDirectXVideoDecoderServiceC*	pIDirectXVideoDecoderServiceC = (IDirectXVideoDecoderServiceC*) pIDirectXVideoDecoderService;

	BOOL res;
	DWORD flOldProtect = 0;

	// Casimir666 : unhook previous VTables
	if (g_pIDirectXVideoDecoderServiceCVtbl)
	{
		res = VirtualProtect(g_pIDirectXVideoDecoderServiceCVtbl, sizeof(g_pIDirectXVideoDecoderServiceCVtbl), PAGE_WRITECOPY, &flOldProtect);
		if (g_pIDirectXVideoDecoderServiceCVtbl->CreateVideoDecoder == CreateVideoDecoderMine)
			g_pIDirectXVideoDecoderServiceCVtbl->CreateVideoDecoder = CreateVideoDecoderOrg;
		//if (g_pIDirectXVideoDecoderServiceCVtbl->GetDecoderDeviceGuids == GetDecoderDeviceGuidsMine)
		//	g_pIDirectXVideoDecoderServiceCVtbl->GetDecoderDeviceGuids = GetDecoderDeviceGuidsOrg;
		res = VirtualProtect(g_pIDirectXVideoDecoderServiceCVtbl, sizeof(g_pIDirectXVideoDecoderServiceCVtbl), flOldProtect, &flOldProtect);

		g_pIDirectXVideoDecoderServiceCVtbl	= NULL;
		CreateVideoDecoderOrg				= NULL;
		m_nCurrentDXVA2Decoder				= 0;
	}

	// TODO : remove log file !!
	::DeleteFile (LOG_FILE);
//	LogVideoCardCaps((IDirectXVideoDecoderService*) pIDirectXVideoDecoderService);

	if (pIDirectXVideoDecoderService)
	{
		res = VirtualProtect(pIDirectXVideoDecoderServiceC->lpVtbl, sizeof(IDirectXVideoDecoderServiceCVtbl), PAGE_WRITECOPY, &flOldProtect);

		CreateVideoDecoderOrg = pIDirectXVideoDecoderServiceC->lpVtbl->CreateVideoDecoder;
		pIDirectXVideoDecoderServiceC->lpVtbl->CreateVideoDecoder = CreateVideoDecoderMine;

		//GetDecoderDeviceGuidsOrg = pIDirectXVideoDecoderServiceC->lpVtbl->GetDecoderDeviceGuids;
		//pIDirectXVideoDecoderServiceC->lpVtbl->GetDecoderDeviceGuids = GetDecoderDeviceGuidsMine;

		res = VirtualProtect(pIDirectXVideoDecoderServiceC->lpVtbl, sizeof(IDirectXVideoDecoderServiceCVtbl), flOldProtect, &flOldProtect);

		g_pIDirectXVideoDecoderServiceCVtbl			= pIDirectXVideoDecoderServiceC->lpVtbl;
	}
}