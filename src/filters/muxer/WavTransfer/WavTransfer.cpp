/*

* *
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
 #define POINTER_64 __ptr64
#include "stdafx.h"
#include <windows.h>
#include "BaseClasses/streams.h"
#include <initguid.h>

#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif


#include "WavTransfer.h"
#include <aviriff.h>
#include <malloc.h>
#include <stdio.h>

//#define EC_BBUFFER_READY EC_USER+100

#ifdef STANDALONE_FILTER
const AMOVIESETUP_FILTER sudWavTrans3 =
{
    &CLSID_WavTransfer,           // clsID
    L"WAV Transfer",              // strName
    MERIT_DO_NOT_USE,         // dwMerit
    0,                        // nPins
    0                         // lpPin
};


// Global data
CFactoryTemplate g_Templates[]= {
    {L"WAV Transfer", &CLSID_WavTransfer, CWavTransferFilter::CreateInstance, NULL, &sudWavTrans3},
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;
#endif
// ------------------------------------------------------------------------
// filter constructor

#pragma warning(disable:4355)


CWavTransferFilter::CWavTransferFilter(LPUNKNOWN pUnk, HRESULT *phr) :
                CTransformFilter(NAME("WavTransfer filter"), pUnk, CLSID_WavTransfer)
{
    ASSERT(m_pOutput == 0);
    ASSERT(phr);

	m_wh=NULL;
	bPreview = 0;
	m_bRunning = false ;
	m_b2Bytes = true;

	hBufferDone = CreateEvent(0, FALSE, FALSE, 0);
	ResetEvent(hBufferDone);

    if(SUCCEEDED(*phr))
    {
        // Create an output pin so we can have control over the connection
        // media type.
        CWavTransferOutputPin *pOut = new CWavTransferOutputPin(this, phr);

        if(pOut)
        {
            if(SUCCEEDED(*phr))
            {
                m_pOutput = pOut;
            }
            else
            {
                delete pOut;
            }
        }
        else
        {
            *phr = E_OUTOFMEMORY;
        }

        //
        // NOTE!: If we've created our own output pin we must also create
        // the input pin ourselves because the CTransformFilter base class 
        // will create an extra output pin if the input pin wasn't created.        
        //
        CTransformInputPin *pIn = new CTransformInputPin(NAME("Transform input pin"),
                                        this,              // Owner filter
                                        phr,               // Result code
                                        L"In");            // Pin name
        // a failed return code should delete the object
        if(pIn)
        {
            if(SUCCEEDED(*phr))
            {
                m_pInput = pIn;
            }
            else
            {
                delete pIn;
            }
        }
        else
        {
            *phr = E_OUTOFMEMORY;
        }
    }
}


// ------------------------------------------------------------------------
// destructor

CWavTransferFilter::~CWavTransferFilter()
{
}


CUnknown * WINAPI CWavTransferFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    return new CWavTransferFilter(pUnk, phr);
}


// To be able to transform, the formats must be identical
HRESULT CWavTransferFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    HRESULT hr;


    if(FAILED(hr = CheckInputType(mtIn)))
    {
        return hr;
    }

    return NOERROR;

} // CheckTransform


// overridden because we need to know if Deliver() failed.

HRESULT CWavTransferFilter::Receive(IMediaSample *pSample)
{
    ULONG cbOld = m_cbWavData;
    HRESULT hr = CTransformFilter::Receive(pSample);

    // don't update the count if Deliver() downstream fails.
    if(hr != S_OK)
    {
        m_cbWavData = cbOld;
    }

    return hr;
}


STDMETHODIMP CWavTransferFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{

if (riid == IID_IWavTransferProperties)
        return GetInterface((IWavTransferProperties*) this, ppv);

    return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
}


HRESULT CWavTransferFilter::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
    REFERENCE_TIME rtStart, rtEnd;

    // First just copy the data to the output sample
    HRESULT hr = Copy(pIn, pOut);
    if(FAILED(hr))
    {
        return hr;
    }

    // Prepare it for writing    
    LONG lActual = pOut->GetActualDataLength();

    rtStart = m_cbWavData + m_cbHeader;
    rtEnd   = rtStart + lActual;
    m_cbWavData += lActual;

   EXECUTE_ASSERT(pOut->SetTime(&rtStart, &rtEnd) == S_OK);

    return S_OK;
}



//
// Make destination an identical copy of source
//
HRESULT CWavTransferFilter::Copy(IMediaSample *pSource, IMediaSample *pDest) //const
{
    CheckPointer(pSource,E_POINTER);
    CheckPointer(pDest,E_POINTER);

    // Copy the sample data

    BYTE *pSourceBuffer, *pDestBuffer;
    long lSourceSize = pSource->GetActualDataLength();
	long lActualSize,lRemainSize,lOffset;
long lDestSize = pDest->GetSize();
#ifdef DEBUG    
    
  //  ASSERT(lDestSize >= lSourceSize);
#endif

    pSource->GetPointer(&pSourceBuffer);
    pDest->GetPointer(&pDestBuffer);


	lRemainSize = lSourceSize;
	lOffset =0L;
	if (m_wh!=NULL)
	{

		while (lRemainSize>m_wh->dwBufferLength)
		{
			lActualSize = m_wh->dwBufferLength;
			CopyMemory((PVOID) m_wh->lpData,(PVOID) (pSourceBuffer+lOffset),lActualSize);
			m_wh->dwBytesRecorded = lActualSize;
			CBaseFilter::NotifyEvent(EC_BBUFFER_READY,0,(LONG_PTR)m_wh);

			lRemainSize -= lActualSize;
			lOffset+=lActualSize;
			DWORD EventResult;
			EventResult=WaitForSingleObject(hBufferDone, 1000);//INFINITE);
			int i=0;
			/*
			switch (EventResult)
			{
			case WAIT_ABANDONED:
				i=1;
				break;
			case WAIT_OBJECT_0:
				i=2;
				break;
			case WAIT_TIMEOUT:
				i=3;
				break;
			case WAIT_FAILED:
				i=4;
				break;
			}*/
			ResetEvent(hBufferDone);
 

		}


		//lActualSize=(m_wh->dwBufferLength>lSourceSize)?lSourceSize:m_wh->dwBufferLength;

		lActualSize = lRemainSize;

		CopyMemory((PVOID) m_wh->lpData,(PVOID) (pSourceBuffer+lOffset),lActualSize);
		//CopyMemory((PVOID) m_wh->lpData,(PVOID) pSourceBuffer,lActualSize);
		m_wh->dwBytesRecorded = lActualSize;
		CBaseFilter::NotifyEvent(EC_BBUFFER_READY,0,(LONG_PTR)m_wh);

		DWORD EventResult=WaitForSingleObject(hBufferDone, 1000);//INFINITE);
		ResetEvent(hBufferDone);

	}

	bPreview=true;

	lActualSize = (lDestSize >= lSourceSize)?lSourceSize:lDestSize;

	if (bPreview)
		CopyMemory((PVOID) pDestBuffer,(PVOID) pSourceBuffer,lActualSize);
	else
	{
	
		if (m_b2Bytes)
			ZeroMemory((PVOID) pDestBuffer,lActualSize);
		else
			memset((PVOID) pDestBuffer ,0x80,lActualSize);
	}

	
    // Copy the sample times

    REFERENCE_TIME TimeStart, TimeEnd;
    if(NOERROR == pSource->GetTime(&TimeStart, &TimeEnd))
    {
        pDest->SetTime(&TimeStart, &TimeEnd);
		m_CurrentTime = TimeEnd;
    }

    LONGLONG MediaStart, MediaEnd;
    if(pSource->GetMediaTime(&MediaStart,&MediaEnd) == NOERROR)
    {
        pDest->SetMediaTime(&MediaStart,&MediaEnd);
    }
 
    // Copy the media type
    AM_MEDIA_TYPE *pMediaType;
    pSource->GetMediaType(&pMediaType);
    pDest->SetMediaType(pMediaType);
    DeleteMediaType(pMediaType);

    // Copy the actual data length
    long lDataLength = pSource->GetActualDataLength();
    pDest->SetActualDataLength(lDataLength);

    return NOERROR;

} // Copy


//
// CheckInputType
//
HRESULT CWavTransferFilter::CheckInputType(const CMediaType* mtIn)
{
    gMediaType = *mtIn;

	if(mtIn->formattype == FORMAT_WaveFormatEx)
    {
        return S_OK;
    }
    return S_FALSE;
}

//
// GetMediaType
//
HRESULT CWavTransferFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    ASSERT(iPosition == 0 || iPosition == 1);

    if(iPosition == 0)
    {
        CheckPointer(pMediaType,E_POINTER);

		*pMediaType=gMediaType;
        //pMediaType->SetType(&MEDIATYPE_Stream);
        //pMediaType->SetSubtype(&MEDIASUBTYPE_WAVE);
        return S_OK;
    }

    return VFW_S_NO_MORE_ITEMS;
}

//
// DecideBufferSize
//
// Tell the output pin's allocator what size buffers we
// require. Can only do this when the input is connected
//
HRESULT CWavTransferFilter::DecideBufferSize(IMemAllocator *pAlloc,
                                         ALLOCATOR_PROPERTIES *pProperties)
{
    HRESULT hr = NOERROR;

    // Is the input pin connected
    if(m_pInput->IsConnected() == FALSE)
    {
        return E_UNEXPECTED;
    }

    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);

    pProperties->cBuffers = 1;
    pProperties->cbAlign  = 1;

    // Get input pin's allocator size and use that
    ALLOCATOR_PROPERTIES InProps;
    IMemAllocator * pInAlloc = NULL;

    hr = m_pInput->GetAllocator(&pInAlloc);
    if(SUCCEEDED(hr))
    {
        hr = pInAlloc->GetProperties(&InProps);
        if(SUCCEEDED(hr))
        {
            pProperties->cbBuffer = InProps.cbBuffer;
        }
        pInAlloc->Release();
    }

    if(FAILED(hr))
        return hr;

    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if(FAILED(hr))
    {
        return hr;
    }

    ASSERT(Actual.cBuffers == 1);

    if(pProperties->cBuffers > Actual.cBuffers ||
        pProperties->cbBuffer > Actual.cbBuffer)
    {
        return E_FAIL;
    }

    return NOERROR;

} // DecideBufferSize



HRESULT CWavTransferFilter::StartStreaming()
{
	m_cbHeader =0;
    m_cbWavData = 0;
	m_CurrentTime=0;
    return S_OK;
}


//
// StopStreaming
//
// Write out the header
//
HRESULT CWavTransferFilter::StopStreaming()
{
m_CurrentTime=0;
	  return S_OK;
 
}

//
// CWavTrans3OutputPin::CWavTrans3OutputPin 
//
CWavTransferOutputPin::CWavTransferOutputPin(CTransformFilter *pFilter, HRESULT * phr) :
        CTransformOutputPin(NAME("WavTransfer output pin"), pFilter, phr, L"Out")
{
    // Empty
}


//
// CWavTrans3OutputPin::EnumMediaTypes
//
STDMETHODIMP CWavTransferOutputPin::EnumMediaTypes( IEnumMediaTypes **ppEnum )
{
    return CBaseOutputPin::EnumMediaTypes(ppEnum);
}



// Make sure it's our default type

HRESULT CWavTransferOutputPin::CheckMediaType(const CMediaType* pmt)
{
    CheckPointer(pmt,E_POINTER);
   //  if(pmt->majortype == MEDIATYPE_Stream && pmt->subtype == MEDIASUBTYPE_WAVE)
    return S_OK;
}


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////


//
// DllEntryPoint
//
//extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
//
//BOOL APIENTRY DllMain(HANDLE hModule, 
//                      DWORD  dwReason, 
//                      LPVOID lpReserved)
//{
//	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
//}
//

