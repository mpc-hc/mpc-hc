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

#pragma once

#include <d3dx9.h>
#include <dxva.h>
#include <dxva2api.h>
#include "..\BaseVideoFilter\BaseVideoFilter.h"

#include "IMPCVideoDecFilter.h"
#include "MPCVideoDecSettingsWnd.h"
#include "..\..\..\decss\DeCSSInputPin.h"


struct AVCodec;
struct AVCodecContext;
struct AVFrame;

class CCpuId;

[uuid("008BAC12-FBAF-497b-9670-BC6F6FBAE2C4")]
class CMPCVideoDecFilter 
	: public CBaseVideoFilter
	, public ISpecifyPropertyPages2
	, public IMPCVideoDecFilter
{
protected:


/*	CCritSec m_csReceive;

	CAtlArray<BYTE> m_buff;
	REFERENCE_TIME m_rtStart;
	bool m_fDiscontinuity;

	float m_sample_max;

	HRESULT GetDeliveryBuffer(IMediaSample** pSample, BYTE** pData);
	HRESULT ReconnectOutput(int nSamples, CMediaType& mt);*/

	static void LogLibAVCodec(void* par,int level,const char *fmt,va_list valist);

protected:
	AVCodec*			m_pAVCodec;
	AVCodecContext*		m_pAVCtx;
	AVFrame*			m_pFrame;
	int					m_nCodecNb;
	int					m_nWorkaroundBug;
	int					m_nErrorConcealment;
	int					m_nErrorResilience;
	int					m_nThreadNumber;

	CCpuId*				m_pCpuId;
	bool				m_bUseDXVA2;

	void				Cleanup();
	int					FindCodec(const CMediaType* mtIn);
	void				AllocExtradata(AVCodecContext* pAVCtx, const CMediaType* mt);


public:
	CMPCVideoDecFilter(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CMPCVideoDecFilter();

	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT Transform(IMediaSample* pIn);
	HRESULT CompleteConnect(PIN_DIRECTION direction,IPin *pReceivePin);

/*
    HRESULT EndOfStream();
	HRESULT BeginFlush();
	HRESULT EndFlush();
    HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
    HRESULT Receive(IMediaSample* pIn);

    HRESULT CheckInputType(const CMediaType* mtIn);
    HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
    HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
    HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);

	HRESULT StartStreaming();
	HRESULT StopStreaming();
	HRESULT CompleteConnect(PIN_DIRECTION direction,IPin *pReceivePin);
*/
	// ISpecifyPropertyPages2

	STDMETHODIMP	GetPages(CAUUID* pPages);
	STDMETHODIMP	CreatePage(const GUID& guid, IPropertyPage** ppPage);

	// === DXVA2 functions
	void			FillInVideoDescription(DXVA2_VideoDesc *pDesc);
	BOOL			IsSupportedDecoderConfig(const DXVA2_ConfigPictureDecode& config);
	BOOL			IsSupportedDecoderMode(const GUID& mode);
	HRESULT			ConfigureDXVA2(IPin *pPin);
	HRESULT			SetEVRForDXVA2(IPin *pPin);
	HRESULT			FindDecoderConfiguration(IDirectXVideoDecoderService *pDecoderService,
											 const GUID& guidDecoder, 
											 DXVA2_ConfigPictureDecode *pSelectedConfig,
											 BOOL *pbFoundDXVA2Configuration);

	bool			UseDXVA2()	{ return m_bUseDXVA2; };
	int				PictWidth();
	int				PictHeight();

	// === DXVA2 variables
	CComPtr<IDirect3DDeviceManager9>			m_pDeviceManager;
	CComPtr<IDirectXVideoDecoderService>		m_pDecoderService;
	CComPtr<IDirectXVideoAccelerationService>	m_pAccelerationService;
	CComPtr<IDirectXVideoDecoder>				m_pDecoder;
	DXVA2_ConfigPictureDecode					m_DecoderConfig;
	GUID										m_DecoderGuid;
	HANDLE										m_hDevice;
	DXVA2_VideoDesc								m_VideoDesc;

};
