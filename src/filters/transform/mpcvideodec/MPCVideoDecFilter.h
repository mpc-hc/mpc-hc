/* 
 * $Id: MPCVideoDecFilter.h 249 2007-09-26 11:07:22Z casimir666 $
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

#pragma once

#include <d3dx9.h>
#include <Videoacc.h>		// DXVA1
#include <dxva.h>
#include <dxva2api.h>		// DXVA2
#include "..\BaseVideoFilter\BaseVideoFilter.h"

#include "IMPCVideoDecFilter.h"
#include "MPCVideoDecSettingsWnd.h"
#include "..\..\..\decss\DeCSSInputPin.h"
#include "DXVADecoder.h"
#include "TlibavcodecExt.h"

struct AVCodec;
struct AVCodecContext;
struct AVFrame;

class CCpuId;




// === FFMpeg extern function
typedef unsigned char uint8_t;
typedef void			(*FUNC_AVCODEC_INIT)();
typedef void			(*FUNC_AVCODEC_REGISTER_ALL)();
typedef AVCodec*		(*FUNC_AVCODEC_FIND_DECODER)(enum CodecID id);
typedef AVCodecContext* (*FUNC_AVCODEC_ALLOC_CONTEXT)(void);
typedef AVFrame*		(*FUNC_AVCODEC_ALLOC_FRAME)(void);
typedef int				(*FUNC_AVCODEC_OPEN)(AVCodecContext *avctx, AVCodec *codec);
typedef int				(*FUNC_AVCODEC_DECODE_VIDEO)(AVCodecContext *avctx, AVFrame *picture, int *got_picture_ptr, uint8_t *buf, int buf_size);
typedef void			(*FUNC_AV_LOG_SET_CALLBACK)(void (*callback)(void*, int, const char*, va_list));
typedef int				(*FUNC_AVCODEC_CLOSE)(AVCodecContext *avctx);
typedef void			(*FUNC_AVCODEC_THREAD_FREE)(AVCodecContext *s);
typedef int				(*FUNC_AVCODEC_THREAD_INIT)(AVCodecContext *s, int thread_count);
typedef void			(*FUNC_AV_FREE)(void *ptr);


typedef enum
{
	MODE_SOFTWARE,
	MODE_DXVA1,
	MODE_DXVA2
} DXVA_MODE;

[uuid("008BAC12-FBAF-497b-9670-BC6F6FBAE2C4")]
class CMPCVideoDecFilter 
	: public CBaseVideoFilter
	, public TlibavcodecExt
	, public ISpecifyPropertyPages2
	, public IMPCVideoDecFilter
{
protected:

	// === FFMpeg callbacks
	static void		LogLibAVCodec(void* par,int level,const char *fmt,va_list valist);
	virtual void	OnGetBuffer(AVFrame *pic);

protected:
	friend class CVideoDecDXVAAllocator;

	CCpuId*									m_pCpuId;
	CCritSec								m_csProps;

	// === Persistants parameters (registry)
	int										m_nThreadNumber;
	int										m_nDiscardMode;
	int										m_nErrorResilience;
	int										m_nIDCTAlgo;
	bool									m_bEnableDXVA;

	// === FFMpeg variables
	AVCodec*								m_pAVCodec;
	AVCodecContext*							m_pAVCtx;
	AVFrame*								m_pFrame;
	int										m_nCodecNb;
	int										m_nWorkaroundBug;
	int										m_nErrorConcealment;
	REFERENCE_TIME							m_rtStart;				// Ref. time for last decoded frame (use for Ffmpeg callback)
	REFERENCE_TIME							m_rtAvrTimePerFrame;

	FUNC_AVCODEC_INIT						ff_avcodec_init;
	FUNC_AVCODEC_REGISTER_ALL				ff_avcodec_register_all;
	FUNC_AVCODEC_FIND_DECODER				ff_avcodec_find_decoder;
	FUNC_AVCODEC_ALLOC_CONTEXT				ff_avcodec_alloc_context;
	FUNC_AVCODEC_ALLOC_FRAME				ff_avcodec_alloc_frame;
	FUNC_AVCODEC_OPEN						ff_avcodec_open;
	FUNC_AVCODEC_DECODE_VIDEO				ff_avcodec_decode_video;
	FUNC_AV_LOG_SET_CALLBACK				ff_av_log_set_callback;
	FUNC_AVCODEC_CLOSE						ff_avcodec_close;
	FUNC_AVCODEC_THREAD_FREE				ff_avcodec_thread_free;
	FUNC_AVCODEC_THREAD_INIT				ff_avcodec_thread_init;
	FUNC_AV_FREE							ff_av_free;

	// === DXVA common variables
	VIDEO_OUTPUT_FORMATS*					m_pVideoOutputFormat;
	int										m_nVideoOutputCount;
	DXVA_MODE								m_nDXVAMode;
	CDXVADecoder*							m_pDXVADecoder;
	GUID									m_DXVADecoderGUID;

	// === DXVA1 variables
	DDPIXELFORMAT							m_PixelFormat;
	//CComPtr<IAMVideoAccelerator>			m_pAMVideoAccelerator;

	// === DXVA2 variables
	CComPtr<IDirect3DDeviceManager9>		m_pDeviceManager;
	CComPtr<IDirectXVideoDecoderService>	m_pDecoderService;
	CComPtr<IDirect3DSurface9>				m_pDecoderRenderTarget;
	DXVA2_ConfigPictureDecode				m_DXVA2Config;
	HANDLE									m_hDevice;
	DXVA2_VideoDesc							m_VideoDesc;

	// === Private functions
	void				Cleanup();
	int					FindCodec(const CMediaType* mtIn);
	void				AllocExtradata(AVCodecContext* pAVCtx, const CMediaType* mt);
	bool				IsMultiThreadSupported(int nCodec);
	void				GetOutputFormats (int& nNumber, VIDEO_OUTPUT_FORMATS** ppFormats);
	void				CalcAvgTimePerFrame();

	HRESULT				TransformSoftware(IMediaSample* pIn, BYTE* pDataIn, int nSize, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop);
	HRESULT				TransformDXVA(IMediaSample* pIn,    BYTE* pDataIn, int nSize, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop);

public:

	const static AMOVIESETUP_MEDIATYPE		sudPinTypesIn[];
	const static int						sudPinTypesInCount;
	const static AMOVIESETUP_MEDIATYPE		sudPinTypesOut[];
	const static int						sudPinTypesOutCount;

	CMPCVideoDecFilter(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CMPCVideoDecFilter();

	DECLARE_IUNKNOWN
    STDMETHODIMP			NonDelegatingQueryInterface(REFIID riid, void** ppv);
	virtual HRESULT			IsVideoInterlaced();
	CTransformOutputPin*	GetOutputPin() { return m_pOutput; }

	// === Overriden DirectShow functions
	HRESULT			SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);
	HRESULT			CheckInputType(const CMediaType* mtIn);
//	HRESULT			GetMediaType(int iPosition, CMediaType* pmt);
	HRESULT			Transform(IMediaSample* pIn);
	HRESULT			CompleteConnect(PIN_DIRECTION direction,IPin *pReceivePin);
    HRESULT			DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT			NewSegment(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, double dRate);


	// === ISpecifyPropertyPages2

	STDMETHODIMP	GetPages(CAUUID* pPages);
	STDMETHODIMP	CreatePage(const GUID& guid, IPropertyPage** ppPage);

	// === IMPCVideoDecFilter
	STDMETHODIMP SetThreadNumber(int nValue);
	STDMETHODIMP_(int) GetThreadNumber();
	STDMETHODIMP SetEnableDXVA(bool fValue);
	STDMETHODIMP_(bool) GetEnableDXVA();
	STDMETHOD(SetDiscardMode(int nValue));
	STDMETHOD_(int, GetDiscardMode());
	STDMETHOD(SetErrorResilience(int nValue));
	STDMETHOD_(int, GetErrorResilience());
	STDMETHOD(SetIDCTAlgo(int nValue));
	STDMETHOD_(int, GetIDCTAlgo());
	STDMETHOD(SetH264QuantMatrix(int nValue));
	STDMETHOD_(int, GetH264QuantMatrix());


	// === DXVA common functions
	BOOL			IsSupportedDecoderConfig(const D3DFORMAT nD3DFormat, const DXVA2_ConfigPictureDecode& config);
	BOOL			IsSupportedDecoderMode(const GUID& mode);
	void			BuildDXVAOutputFormat();
	int				GetPicEntryNumber();
	int				PictWidth();
	int				PictHeight();
	bool			UseDXVA2()	{ return (m_nDXVAMode == MODE_DXVA2); };
	void*			GetAVContextPrivateData();
	void			DecodeData (BYTE* pDataIn, int nSize);
	GUID*			GetDXVADecoderGUID() { return &m_DXVADecoderGUID; }
	void			FlushDXVADecoder()	 { if (m_pDXVADecoder) m_pDXVADecoder->Flush(); }


	// === DXVA1 functions
	DDPIXELFORMAT*	GetPixelFormat() { return &m_PixelFormat; }
	HRESULT			FindDXVA1DecoderConfiguration(IAMVideoAccelerator* pAMVideoAccelerator, const GUID* guidDecoder, DDPIXELFORMAT* pPixelFormat);
	HRESULT			CheckDXVA1Decoder(const GUID *pGuid);
	void			SetDXVA1Params(const GUID* pGuid, DDPIXELFORMAT* pPixelFormat);
	WORD			GetDXVA1RestrictedMode();
	HRESULT			CreateDXVA1Decoder(IAMVideoAccelerator* pAMVideoAccelerator, const GUID* pDecoderGuid, DWORD dwSurfaceCount);


	// === DXVA2 functions
	void			FillInVideoDescription(DXVA2_VideoDesc *pDesc);
	HRESULT			ConfigureDXVA2(IPin *pPin);
	HRESULT			SetEVRForDXVA2(IPin *pPin);
	HRESULT			FindDXVA2DecoderConfiguration(IDirectXVideoDecoderService *pDecoderService,
												  const GUID& guidDecoder, 
												  DXVA2_ConfigPictureDecode *pSelectedConfig,
											      BOOL *pbFoundDXVA2Configuration);
	HRESULT			CreateDXVA2Decoder(UINT nNumRenderTargets, IDirect3DSurface9** pDecoderRenderTargets);

};
