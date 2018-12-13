/*

* (C) 2017 see Authors.txt
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
#pragma once
#include <Mmsystem.h>
#include "IWAVTransferProperty.h"
#define EC_BBUFFER_READY EC_USER+100
#pragma warning(disable: 4097 4511 4512 4514 4705)

// {94221D9D-B49D-44AE-BC1F-EFD7E9E8E2C5}
static const GUID CLSID_WavTransfer =
{ 0x94221d9d, 0xb49d, 0x44ae,{ 0xbc, 0x1f, 0xef, 0xd7, 0xe9, 0xe8, 0xe2, 0xc5 } };


class CWavTransferOutputPin : public CTransformOutputPin
{
public:
    CWavTransferOutputPin(CTransformFilter *pFilter, HRESULT * phr);

    STDMETHODIMP EnumMediaTypes( IEnumMediaTypes **ppEnum );
    HRESULT CheckMediaType(const CMediaType* pmt);
};


class __declspec(uuid("94221D9D-B49D-44AE-BC1F-EFD7E9E8E2C5"))
CWavTransferFilter : public CTransformFilter, public IWavTransferProperties
{

public:

    DECLARE_IUNKNOWN;
  
    CWavTransferFilter(LPUNKNOWN pUnk, HRESULT *pHr);
    ~CWavTransferFilter();
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *pHr);

    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT Receive(IMediaSample *pSample);

    HRESULT CheckInputType(const CMediaType* mtIn) ;
    HRESULT CheckTransform(const CMediaType *mtIn,const CMediaType *mtOut);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType) ;

    HRESULT DecideBufferSize(IMemAllocator *pAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

    HRESULT StartStreaming();
    HRESULT StopStreaming();

    HRESULT CompleteConnect(PIN_DIRECTION direction,IPin *pReceivePin) { return S_OK; }

	STDMETHODIMP AddBuffer(LPWAVEHDR pwh,  UINT cbwh  ){m_wh=pwh; return S_OK;} 
	STDMETHODIMP EnablePreview(bool bpPreview){ bPreview= bpPreview ;return S_OK;}; 
	STDMETHODIMP set_2Bytes(bool b2Bytes){ m_b2Bytes= b2Bytes ;return S_OK;}; 
	STDMETHODIMP SetBufferEvent(){SetEvent(hBufferDone) ;return S_OK;}; 
	STDMETHODIMP GetCurrentLastTime(REFERENCE_TIME *pTime)	{ *pTime=m_CurrentTime;return S_OK;}; 
	STDMETHODIMP SetCurrentLastTime(REFERENCE_TIME pTime)	{ m_CurrentTime=pTime;return S_OK;}; 

	
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);   
 
	IWavTransferProperties *WP;
private:

    HRESULT Copy(IMediaSample *pSource, IMediaSample *pDest) ;//const;
    HRESULT Transform(IMediaSample *pMediaSample);
    HRESULT Transform(AM_MEDIA_TYPE *pType, const signed char ContrastLevel) const;

    ULONG m_cbWavData;
    ULONG m_cbHeader;
	CMediaType gMediaType;
	LPWAVEHDR m_wh;
	bool bPreview;
	bool m_bRunning;
	bool m_b2Bytes;
	HANDLE hBufferDone;
	REFERENCE_TIME m_CurrentTime;
};
