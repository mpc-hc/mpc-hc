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
#include "IPinHook.h"

REFERENCE_TIME g_tSegmentStart = 0;
REFERENCE_TIME g_tSampleStart = 0;

static HRESULT (STDMETHODCALLTYPE * NewSegmentOrg)(IPinC * This, /* [in] */ REFERENCE_TIME tStart, /* [in] */ REFERENCE_TIME tStop, /* [in] */ double dRate) = NULL;

static HRESULT STDMETHODCALLTYPE NewSegmentMine(IPinC * This, /* [in] */ REFERENCE_TIME tStart, /* [in] */ REFERENCE_TIME tStop, /* [in] */ double dRate)
{
	g_tSegmentStart = tStart;
	return NewSegmentOrg(This, tStart, tStop, dRate);
}

static HRESULT ( STDMETHODCALLTYPE *ReceiveOrg )( IMemInputPinC * This, IMediaSample *pSample) = NULL;

static HRESULT STDMETHODCALLTYPE ReceiveMine(IMemInputPinC * This, IMediaSample *pSample)
{
	REFERENCE_TIME rtStart, rtStop;
	if(pSample && SUCCEEDED(pSample->GetTime(&rtStart, &rtStop)))
		g_tSampleStart = rtStart;
	return ReceiveOrg(This, pSample);
}

bool HookNewSegmentAndReceive(IPinC* pPinC, IMemInputPinC* pMemInputPinC)
{
	if(!pPinC || !pMemInputPinC || (GetVersion()&0x80000000))
		return false;

	g_tSegmentStart = 0;
	g_tSampleStart = 0;

	BOOL res;
	DWORD flOldProtect = 0;

	res = VirtualProtect(pPinC->lpVtbl, sizeof(IPinC), PAGE_WRITECOPY, &flOldProtect);
	if(NewSegmentOrg == NULL) NewSegmentOrg = pPinC->lpVtbl->NewSegment;
	pPinC->lpVtbl->NewSegment = NewSegmentMine;
	res = VirtualProtect(pPinC->lpVtbl, sizeof(IPinC), PAGE_EXECUTE, &flOldProtect);

	res = VirtualProtect(pMemInputPinC->lpVtbl, sizeof(IMemInputPinC), PAGE_WRITECOPY, &flOldProtect);
	if(ReceiveOrg == NULL) ReceiveOrg = pMemInputPinC->lpVtbl->Receive;
	pMemInputPinC->lpVtbl->Receive = ReceiveMine;
	res = VirtualProtect(pMemInputPinC->lpVtbl, sizeof(IMemInputPinC), PAGE_EXECUTE, &flOldProtect);

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
		if(FILE* f = _tfopen(_T("c:\\dxva.log"), _T("at")))
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
/*
*/
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
/*
*/
/*
*/
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
/*
*/
	res = VirtualProtect(pAMVideoAcceleratorC->lpVtbl, sizeof(IAMVideoAcceleratorC), PAGE_EXECUTE, &flOldProtect);
}
