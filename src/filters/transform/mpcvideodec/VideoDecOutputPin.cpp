/* 
 * $Id$
 *
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
#include "VideoDecOutputPin.h"
#include "VideoDecDXVAAllocator.h"
#include "MPCVideoDecFilter.h"
#include "..\..\..\DSUtil\DSUtil.h"

CVideoDecOutputPin::CVideoDecOutputPin(TCHAR* pObjectName, CBaseVideoFilter* pFilter, HRESULT* phr, LPCWSTR pName)
				  : CBaseVideoOutputPin(pObjectName, pFilter, phr, pName)
{
	m_pVideoDecFilter		= (CMPCVideoDecFilter*) pFilter;
	m_pDXVA2Allocator		= NULL;
	m_dwDXVA1SurfaceCount	= 0;
}

CVideoDecOutputPin::~CVideoDecOutputPin(void)
{
}


HRESULT CVideoDecOutputPin::InitAllocator(IMemAllocator **ppAlloc)
{
	TRACE("CVideoDecOutputPin::InitAllocator");
	if (m_pVideoDecFilter->UseDXVA2())
	{
		HRESULT hr = S_FALSE;
		m_pDXVA2Allocator = new CVideoDecDXVAAllocator(m_pVideoDecFilter, &hr);
		if (!m_pDXVA2Allocator)
		{
			return E_OUTOFMEMORY;
		}
		if (FAILED(hr))
		{
			delete m_pDXVA2Allocator;
			return hr;
		}
		// Return the IMemAllocator interface.
		return m_pDXVA2Allocator->QueryInterface(__uuidof(IMemAllocator), (void **)ppAlloc);
	}
	else
		return __super::InitAllocator(ppAlloc);
}



HRESULT CVideoDecOutputPin::Deliver(IMediaSample* pMediaSample)
{
	if (!m_pVideoDecFilter->UseDXVA2())
	{
		if(!m_pOutputQueue) return NOERROR;
		pMediaSample->AddRef();
		return m_pOutputQueue->Receive(pMediaSample);
	}
	else
		return __super::Deliver (pMediaSample);
}


HRESULT CVideoDecOutputPin::Active()
{
	CAutoLock cAutoLock(m_pLock);

	if(m_Connected && !m_pOutputQueue)
	{
		HRESULT hr = NOERROR;

		m_pOutputQueue.Attach(new COutputQueue(m_Connected, &hr));
		if(!m_pOutputQueue) hr = E_OUTOFMEMORY;

		if(FAILED(hr))
		{
			m_pOutputQueue.Free();
			return hr;
		}
	}

	return __super::Active();
}

HRESULT CVideoDecOutputPin::Inactive()
{
	CAutoLock cAutoLock(m_pLock);
	m_pOutputQueue.Free();
	return __super::Inactive();
}


#define CallQueue(call) \
	if(!m_pOutputQueue) return NOERROR; \
	m_pOutputQueue->##call; \
	return NOERROR; \

HRESULT CVideoDecOutputPin::DeliverEndOfStream()
{
	CallQueue(EOS());
}

HRESULT CVideoDecOutputPin::DeliverBeginFlush()
{
	CallQueue(BeginFlush());
}

HRESULT CVideoDecOutputPin::DeliverEndFlush()
{
	CallQueue(EndFlush());
}

HRESULT CVideoDecOutputPin::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	CallQueue(NewSegment(tStart, tStop, dRate));
}



STDMETHODIMP CVideoDecOutputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI(IAMVideoAcceleratorNotify)
		 __super::NonDelegatingQueryInterface(riid, ppv);
}


// === IAMVideoAcceleratorNotify

STDMETHODIMP CVideoDecOutputPin::GetUncompSurfacesInfo(const GUID *pGuid, LPAMVAUncompBufferInfo pUncompBufferInfo)
{
	HRESULT			hr = E_INVALIDARG;
	
	if (SUCCEEDED (m_pVideoDecFilter->CheckDXVA1Decoder (pGuid)))
	{
		CComQIPtr<IAMVideoAccelerator>		pAMVideoAccelerator	= GetConnected();

		if (pAMVideoAccelerator)
		{
			pUncompBufferInfo->dwMaxNumSurfaces		= m_pVideoDecFilter->GetPicEntryNumber();
			pUncompBufferInfo->dwMinNumSurfaces		= m_pVideoDecFilter->GetPicEntryNumber();

			hr = m_pVideoDecFilter->FindDXVA1DecoderConfiguration (pAMVideoAccelerator, pGuid, &pUncompBufferInfo->ddUncompPixelFormat);
			if (SUCCEEDED (hr)) m_pVideoDecFilter->SetDXVA1Params (pGuid, &pUncompBufferInfo->ddUncompPixelFormat);
		}
	}
	return hr;
}

STDMETHODIMP CVideoDecOutputPin::SetUncompSurfacesInfo(DWORD dwActualUncompSurfacesAllocated)      
{
	m_dwDXVA1SurfaceCount = dwActualUncompSurfacesAllocated;
	return S_OK;
}

STDMETHODIMP CVideoDecOutputPin::GetCreateVideoAcceleratorData(const GUID *pGuid, LPDWORD pdwSizeMiscData, LPVOID *ppMiscData)
{
	HRESULT								hr						= E_UNEXPECTED;
	AMVAUncompDataInfo					UncompInfo;
	AMVACompBufferInfo					CompInfo[30];
	DWORD								dwNumTypesCompBuffers	= countof(CompInfo);
	CComQIPtr<IAMVideoAccelerator>		pAMVideoAccelerator		= GetConnected();
	DXVA_ConnectMode*					pConnectMode;	

	if (pAMVideoAccelerator)
	{
		memcpy (&UncompInfo.ddUncompPixelFormat, m_pVideoDecFilter->GetPixelFormat(), sizeof (DDPIXELFORMAT));
		UncompInfo.dwUncompHeight		= m_pVideoDecFilter->PictHeightRounded();
		UncompInfo.dwUncompWidth		= m_pVideoDecFilter->PictWidth();
		hr = pAMVideoAccelerator->GetCompBufferInfo(m_pVideoDecFilter->GetDXVADecoderGuid(), &UncompInfo, &dwNumTypesCompBuffers, CompInfo);

		if (SUCCEEDED (hr))
		{
			hr = m_pVideoDecFilter->CreateDXVA1Decoder (pAMVideoAccelerator, pGuid, m_dwDXVA1SurfaceCount);

			if (SUCCEEDED (hr))
			{
				pConnectMode					= (DXVA_ConnectMode*)CoTaskMemAlloc (sizeof(DXVA_ConnectMode));
				pConnectMode->guidMode			= *m_pVideoDecFilter->GetDXVADecoderGuid();
				pConnectMode->wRestrictedMode	= m_pVideoDecFilter->GetDXVA1RestrictedMode();
				*pdwSizeMiscData				= sizeof(DXVA_ConnectMode);
				*ppMiscData						= pConnectMode;
			}
		}
	}


	return hr;
}
