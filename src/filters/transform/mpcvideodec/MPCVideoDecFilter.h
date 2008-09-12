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




typedef enum
{
	MODE_SOFTWARE,
	MODE_DXVA1,
	MODE_DXVA2
} DXVA_MODE;


typedef struct
{
	REFERENCE_TIME	rtStart;
	REFERENCE_TIME	rtStop;
} B_FRAME;

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

	friend class CVideoDecDXVAAllocator;

	CCpuId*									m_pCpuId;
	CCritSec								m_csProps;

	// === Persistants parameters (registry)
	int										m_nThreadNumber;
	int										m_nDiscardMode;
	int										m_nErrorRecognition;
	int										m_nIDCTAlgo;
	bool									m_bDXVACompatible;
	int										m_nCompatibilityMode;
	int										m_nActiveCodecs;
	int										m_nARMode;

	// === FFMpeg variables
	AVCodec*								m_pAVCodec;
	AVCodecContext*							m_pAVCtx;
	AVFrame*								m_pFrame;
	int										m_nCodecNb;
	int										m_nWorkaroundBug;
	int										m_nErrorConcealment;
	REFERENCE_TIME							m_rtStart;				// Ref. time for last decoded frame (use for Ffmpeg callback)
	REFERENCE_TIME							m_rtAvrTimePerFrame;
	bool									m_bReorderBFrame;
	B_FRAME									m_BFrames[2];
	int										m_nPosB;
	BYTE*									m_pFFBuffer;
	int										m_nFFBufferSize;
	int										m_nWidth;				// Frame width give to input pin
	int										m_nHeight;				// Frame height give to input pin
	
	bool									m_bUseDXVA;
	bool									m_bUseFFmpeg;				

	// === DXVA common variables
	VIDEO_OUTPUT_FORMATS*					m_pVideoOutputFormat;
	int										m_nVideoOutputCount;
	DXVA_MODE								m_nDXVAMode;
	CDXVADecoder*							m_pDXVADecoder;
	GUID									m_DXVADecoderGUID;

	int										m_nPCIVendor;
	int										m_nPCIDevice;
	CString									m_strDeviceDescription;

	// === DXVA1 variables
	DDPIXELFORMAT							m_PixelFormat;

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
	void				DetectVideoCard();


	HRESULT				SoftwareDecode(IMediaSample* pIn, BYTE* pDataIn, int nSize, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop);

public:

	const static AMOVIESETUP_MEDIATYPE		sudPinTypesIn[];
	const static int						sudPinTypesInCount;
	const static AMOVIESETUP_MEDIATYPE		sudPinTypesOut[];
	const static int						sudPinTypesOutCount;

	static UINT								FFmpegFilters;
	static UINT								DXVAFilters;

	CMPCVideoDecFilter(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CMPCVideoDecFilter();

	DECLARE_IUNKNOWN
    STDMETHODIMP			NonDelegatingQueryInterface(REFIID riid, void** ppv);
	virtual HRESULT			IsVideoInterlaced();
	virtual void			GetOutputSize(int& w, int& h, int& arx, int& ary);
	CTransformOutputPin*	GetOutputPin() { return m_pOutput; }

	// === Overriden DirectShow functions
	HRESULT			SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);
	HRESULT			CheckInputType(const CMediaType* mtIn);
	HRESULT			Transform(IMediaSample* pIn);
	HRESULT			CompleteConnect(PIN_DIRECTION direction,IPin *pReceivePin);
    HRESULT			DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT			NewSegment(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, double dRate);
	HRESULT			BreakConnect(PIN_DIRECTION dir);


	// === ISpecifyPropertyPages2

	STDMETHODIMP	GetPages(CAUUID* pPages);
	STDMETHODIMP	CreatePage(const GUID& guid, IPropertyPage** ppPage);

	// === IMPCVideoDecFilter
	STDMETHODIMP Apply();
	STDMETHODIMP SetThreadNumber(int nValue);
	STDMETHODIMP_(int) GetThreadNumber();
	STDMETHOD(SetDiscardMode(int nValue));
	STDMETHOD_(int, GetDiscardMode());
	STDMETHOD(SetErrorRecognition(int nValue));
	STDMETHOD_(int, GetErrorRecognition());
	STDMETHOD(SetIDCTAlgo(int nValue));
	STDMETHOD_(int, GetIDCTAlgo());
	STDMETHOD_(GUID*, GetDXVADecoderGuid());
	STDMETHOD(SetActiveCodecs(MPC_VIDEO_CODEC nValue));
	STDMETHOD_(MPC_VIDEO_CODEC, GetActiveCodecs());
	STDMETHODIMP_(LPCTSTR) GetVideoCardDescription();
	
	STDMETHOD(SetARMode(int nValue));
	STDMETHOD_(int, GetARMode());



	// === DXVA common functions
	BOOL			IsSupportedDecoderConfig(const D3DFORMAT nD3DFormat, const DXVA2_ConfigPictureDecode& config);
	BOOL			IsSupportedDecoderMode(const GUID& mode);
	void			BuildDXVAOutputFormat();
	int				GetPicEntryNumber();
	int				PictWidth();
	int				PictHeight();
	int				PictWidthRounded();
	int				PictHeightRounded();
	bool			UseDXVA2()	{ return (m_nDXVAMode == MODE_DXVA2); };
	void			FlushDXVADecoder()	 { if (m_pDXVADecoder) m_pDXVADecoder->Flush(); }
	AVCodecContext*	GetAVCtx()		 { return m_pAVCtx; };
	bool			IsDXVASupported();
	bool			ReorderBFrame() { return m_bReorderBFrame; };
	int				GetPCIVendor()  { return m_nPCIVendor; };

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
	
	// === aspect ratio
	double			m_sar;
};
