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
    HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);

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
	HRESULT			CreateDXVA2Decoder(UINT nNumRenderTargets, IDirect3DSurface9** pDecoderRenderTargets);

	bool			UseDXVA2()	{ return m_bUseDXVA2; };
	int				PictWidth();
	int				PictHeight();

	// === DXVA2 variables
	CComPtr<IDirect3DDeviceManager9>			m_pDeviceManager;
	CComPtr<IDirectXVideoDecoderService>		m_pDecoderService;
	CComPtr<IDirectXVideoDecoder>				m_pDecoder;
	DXVA2_ConfigPictureDecode					m_DecoderConfig;
	GUID										m_DecoderGuid;
	HANDLE										m_hDevice;
	DXVA2_VideoDesc								m_VideoDesc;
	DXVA2_DecodeExecuteParams					m_ExecuteParams;
	CComPtr<IDirect3DSurface9>					m_pDecoderRenderTarget;
};
