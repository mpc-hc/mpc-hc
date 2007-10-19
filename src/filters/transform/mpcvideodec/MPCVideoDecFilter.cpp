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
#include <math.h>
#include <atlbase.h>
#include <mmreg.h>

#include "PODtypes.h"
#include "avcodec.h"

#include "MPCVideoDecFilter.h"
#include "VideoDecOutputPin.h"
#include "CpuId.h"

#include "..\..\..\DSUtil\DSUtil.h"

#include <initguid.h>
#include <moreuuids.h>

#undef free
#include <malloc.h>


/////
typedef struct FFMPEG_CODECS
{
  const CLSID*			clsMinorType;
  const enum CodecID	nFFCodec;
  const int				fourcc;
  const	bool			bSupportDXVA;
};


const FFMPEG_CODECS		ffCodecs[] =
{
	// Flash video
	{ &MEDIASUBTYPE_FLV1, CODEC_ID_FLV1, MAKEFOURCC('F','L','V','1'),	false },
	{ &MEDIASUBTYPE_VP50, CODEC_ID_VP5,  MAKEFOURCC('V','P','5','0'),	false },
	{ &MEDIASUBTYPE_VP60, CODEC_ID_VP6,  MAKEFOURCC('V','P','6','0'),	false },
	{ &MEDIASUBTYPE_VP61, CODEC_ID_VP6,  MAKEFOURCC('V','P','6','1'),	false },
	{ &MEDIASUBTYPE_VP62, CODEC_ID_VP6,  MAKEFOURCC('V','P','6','2'),	false },
	{ &MEDIASUBTYPE_FLV4, CODEC_ID_VP6F, MAKEFOURCC('F','L','V','4'),	false },
	{ &MEDIASUBTYPE_VP6F, CODEC_ID_VP6F, MAKEFOURCC('V','P','6','F'),	false },
	{ &MEDIASUBTYPE_VP6A, CODEC_ID_VP6A, MAKEFOURCC('V','P','6','A'),	false },

	{ &MEDIASUBTYPE_H264, CODEC_ID_H264, MAKEFOURCC('H','2','6','4'),	true },
	{ &MEDIASUBTYPE_X264, CODEC_ID_H264, MAKEFOURCC('X','2','6','4'),	true },
	{ &MEDIASUBTYPE_VSSH, CODEC_ID_H264, MAKEFOURCC('V','S','S','H'),	true },
	{ &MEDIASUBTYPE_DAVC, CODEC_ID_H264, MAKEFOURCC('D','A','V','C'),	true },
	{ &MEDIASUBTYPE_PAVC, CODEC_ID_H264, MAKEFOURCC('P','A','V','C'),	true },
	{ &MEDIASUBTYPE_AVC1, CODEC_ID_H264, MAKEFOURCC('A','V','C','1'),	true },
};

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	// Flash video
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FLV1   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP50   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP60   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP61   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP62   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FLV4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP6F   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP6A   },

	// H264
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_H264   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_X264   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VSSH   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DAVC   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_PAVC   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_AVC1   },
};


#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] =
{
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM},
};

const AMOVIESETUP_PIN sudpPins[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn),  sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CMPCVideoDecFilter), L"MPC - Video decoder", /*MERIT_DO_NOT_USE*/0x40000001, countof(sudpPins), sudpPins},
};

CFactoryTemplate g_Templates[] =
{
    {sudFilter[0].strName, &__uuidof(CMPCVideoDecFilter), CreateInstance<CMPCVideoDecFilter>, NULL, &sudFilter[0]},
	{L"CMPCVideoDecPropertyPage", &__uuidof(CMPCVideoDecSettingsWnd), CreateInstance<CInternalPropertyPageTempl<CMPCVideoDecSettingsWnd> >},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

//

#include "..\..\FilterApp.h"

CFilterApp theApp;

#endif


CMPCVideoDecFilter::CMPCVideoDecFilter(LPUNKNOWN lpunk, HRESULT* phr) 
	: CBaseVideoFilter(NAME("MPC - Video decoder"), lpunk, phr, __uuidof(this))
{
	if(phr) *phr = S_OK;

	if (m_pOutput)	delete m_pOutput;
	if(!(m_pOutput = new CVideoDecOutputPin(NAME("CVideoDecOutputPin"), this, phr, L"Output"))) *phr = E_OUTOFMEMORY;

/*	CRegKey key;
	if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\MPEG Audio Decoder"), KEY_READ))
	{
		DWORD dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("SampleFormat"), dw)) m_iSampleFormat = (SampleFormat)dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Normalize"), dw)) m_fNormalize = !!dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Boost"), dw)) m_boost = *(float*)&dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Ac3SpeakerConfig"), dw)) m_iSpeakerConfig[ac3] = (int)dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("DtsSpeakerConfig"), dw)) m_iSpeakerConfig[dts] = (int)dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("AacSpeakerConfig"), dw)) m_iSpeakerConfig[aac] = (int)dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Ac3DynamicRangeControl"), dw)) m_fDynamicRangeControl[ac3] = !!dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("DtsDynamicRangeControl"), dw)) m_fDynamicRangeControl[dts] = !!dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("AacDynamicRangeControl"), dw)) m_fDynamicRangeControl[aac] = !!dw;
	}*/

	m_pCpuId			= new CCpuId();
	m_pAVCodec			= NULL;
	m_pAVCtx			= NULL;
	m_pFrame			= NULL;
	m_nCodecNb			= -1;

	m_nWorkaroundBug	= 1;		// TODO : add config in property page
	m_nErrorConcealment	= 3;
	m_nErrorResilience	= 1;
	m_nThreadNumber		= 1; //m_CpuId.GetProcessorNumber();

	m_bUseDXVA2			= false;

	avcodec_init();
	avcodec_register_all();
	av_log_set_callback(LogLibAVCodec);
}



CMPCVideoDecFilter::~CMPCVideoDecFilter()
{
	Cleanup();

	delete m_pCpuId;
	/*
	CRegKey key;
	if(ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\MPEG Audio Decoder")))
	{
		key.SetDWORDValue(_T("SampleFormat"), m_iSampleFormat);
		key.SetDWORDValue(_T("Normalize"), m_fNormalize);
		key.SetDWORDValue(_T("Boost"), *(DWORD*)&m_boost);
		key.SetDWORDValue(_T("Ac3SpeakerConfig"), m_iSpeakerConfig[ac3]);
		key.SetDWORDValue(_T("DtsSpeakerConfig"), m_iSpeakerConfig[dts]);
		key.SetDWORDValue(_T("AacSpeakerConfig"), m_iSpeakerConfig[aac]);
		key.SetDWORDValue(_T("Ac3DynamicRangeControl"), m_fDynamicRangeControl[ac3]);
		key.SetDWORDValue(_T("DtsDynamicRangeControl"), m_fDynamicRangeControl[dts]);
		key.SetDWORDValue(_T("AacDynamicRangeControl"), m_fDynamicRangeControl[aac]);
	}*/
}


int CMPCVideoDecFilter::PictWidth()
{
	return m_pAVCtx ? m_pAVCtx->width  : 0; 
}


int CMPCVideoDecFilter::PictHeight()
{
	return m_pAVCtx ? m_pAVCtx->height : 0;
}

int CMPCVideoDecFilter::FindCodec(const CMediaType* mtIn)
{
	for (int i=0; i<countof(ffCodecs); i++)
		if (mtIn->subtype == *ffCodecs[i].clsMinorType)
			return i;

	return -1;
}

void CMPCVideoDecFilter::Cleanup()
{
	if (m_pAVCtx)
	{
		if (m_pAVCtx->intra_matrix)			free(m_pAVCtx->intra_matrix);
		if (m_pAVCtx->inter_matrix)			free(m_pAVCtx->inter_matrix);
		if (m_pAVCtx->intra_matrix_luma)	free(m_pAVCtx->intra_matrix_luma);
		if (m_pAVCtx->intra_matrix_chroma)	free(m_pAVCtx->intra_matrix_chroma);
		if (m_pAVCtx->inter_matrix_luma)	free(m_pAVCtx->inter_matrix_luma);
		if (m_pAVCtx->inter_matrix_chroma)	free(m_pAVCtx->inter_matrix_chroma);

		if (m_pAVCtx->slice_offset) av_free(m_pAVCtx->slice_offset);
		if (m_pAVCodec) avcodec_close(m_pAVCtx);
		avcodec_thread_free (m_pAVCtx);
		av_free(m_pAVCtx);
	}
	if (m_pFrame)	av_free(m_pFrame);

	m_pAVCodec	= NULL;
	m_pAVCtx	= NULL;
	m_pFrame	= NULL;
	m_nCodecNb	= -1;
}

void CMPCVideoDecFilter::LogLibAVCodec(void* par,int level,const char *fmt,va_list valist)
{
	AVCodecContext*	m_pAVCtx = (AVCodecContext*) par;
	TRACE(fmt,valist);
}


STDMETHODIMP CMPCVideoDecFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI(IMPCVideoDecFilter)
		QI(ISpecifyPropertyPages)
		QI(ISpecifyPropertyPages2)
		 __super::NonDelegatingQueryInterface(riid, ppv);
}




HRESULT CMPCVideoDecFilter::CheckInputType(const CMediaType* mtIn)
{
	return S_OK;
	for (int i=0; i<sizeof(sudPinTypesIn)/sizeof(AMOVIESETUP_MEDIATYPE); i++)
	{
		if ((mtIn->majortype == *sudPinTypesIn[i].clsMajorType) && 
			(mtIn->subtype == *sudPinTypesIn[i].clsMinorType))
			return S_OK;
	}

	return VFW_E_TYPE_NOT_ACCEPTED;
}


HRESULT CMPCVideoDecFilter::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
	int		nNewCodec;

	nNewCodec = FindCodec(pmt);
	if ((direction == PINDIR_INPUT) && (nNewCodec != -1) && (nNewCodec != m_nCodecNb))
	{
		m_nCodecNb	= nNewCodec;
		m_pAVCodec	= avcodec_find_decoder(ffCodecs[nNewCodec].nFFCodec);
		m_pAVCtx	= avcodec_alloc_context();
		avcodec_thread_init(m_pAVCtx, m_nThreadNumber);
		m_pFrame = avcodec_alloc_frame();

		if(pmt->formattype == FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER*	vih = (VIDEOINFOHEADER*)pmt->pbFormat;
			m_pAVCtx->width		= vih->bmiHeader.biWidth;
			m_pAVCtx->height	= abs(vih->bmiHeader.biHeight);
			m_pAVCtx->codec_tag	= vih->bmiHeader.biCompression;
		}
		else if(pmt->formattype == FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2*	vih2 = (VIDEOINFOHEADER2*)pmt->pbFormat;
			m_pAVCtx->width		= vih2->bmiHeader.biWidth;
			m_pAVCtx->height	= abs(vih2->bmiHeader.biHeight);
			m_pAVCtx->codec_tag	= vih2->bmiHeader.biCompression;
		}
		else if(pmt->formattype == FORMAT_MPEGVideo)
		{
			MPEG1VIDEOINFO*		mpgv = (MPEG1VIDEOINFO*)pmt->pbFormat;
			m_pAVCtx->width		= mpgv->hdr.bmiHeader.biWidth;
			m_pAVCtx->height	= abs(mpgv->hdr.bmiHeader.biHeight);
			m_pAVCtx->codec_tag	= mpgv->hdr.bmiHeader.biCompression;
		}
		else if(pmt->formattype == FORMAT_MPEG2Video)
		{
			MPEG2VIDEOINFO*		mpg2v = (MPEG2VIDEOINFO*)pmt->pbFormat;
			m_pAVCtx->width		= mpg2v->hdr.bmiHeader.biWidth;
			m_pAVCtx->height	= abs(mpg2v->hdr.bmiHeader.biHeight);
			m_pAVCtx->codec_tag	= mpg2v->hdr.bmiHeader.biCompression;

			if (m_pAVCtx->codec_tag == MAKEFOURCC('a','v','c','1'))
			{
				m_pAVCtx->nal_length_size = mpg2v->dwFlags;
			}
		}
		
		m_pAVCtx->intra_matrix			= (uint16_t*)calloc(sizeof(uint16_t),64);
		m_pAVCtx->inter_matrix			= (uint16_t*)calloc(sizeof(uint16_t),64);
		m_pAVCtx->intra_matrix_luma		= (uint16_t*)calloc(sizeof(uint16_t),16);
		m_pAVCtx->intra_matrix_chroma	= (uint16_t*)calloc(sizeof(uint16_t),16);
		m_pAVCtx->inter_matrix_luma		= (uint16_t*)calloc(sizeof(uint16_t),16);
		m_pAVCtx->inter_matrix_chroma	= (uint16_t*)calloc(sizeof(uint16_t),16);
		m_pAVCtx->codec_tag				= ffCodecs[nNewCodec].fourcc;
		m_pAVCtx->workaround_bugs		= m_nWorkaroundBug;	// TODO !! 
		m_pAVCtx->error_concealment		= m_nErrorConcealment;
		m_pAVCtx->error_resilience		= m_nErrorResilience;
		m_pAVCtx->dsp_mask				= FF_MM_FORCE | m_pCpuId->GetFeatures();

		m_pAVCtx->postgain				= 1.0f;
		m_pAVCtx->scenechange_factor	= 1;
		m_pAVCtx->debug_mv				= 1;

		AllocExtradata (m_pAVCtx, pmt);		

		if (avcodec_open(m_pAVCtx, m_pAVCodec)<0)
			return VFW_E_INVALIDMEDIATYPE;
	}

	return __super::SetMediaType(direction, pmt);
}


void CMPCVideoDecFilter::AllocExtradata(AVCodecContext* pAVCtx, const CMediaType* pmt)
{
	const BYTE*		data = NULL;
	unsigned int	size = 0;

	if (pmt->formattype==FORMAT_VideoInfo)
	{
		size = pmt->cbFormat-sizeof(VIDEOINFOHEADER);
		data = size?pmt->pbFormat+sizeof(VIDEOINFOHEADER):NULL;
	}
	else if (pmt->formattype==FORMAT_VideoInfo2)
	{
		size = pmt->cbFormat-sizeof(VIDEOINFOHEADER2);
		data = size?pmt->pbFormat+sizeof(VIDEOINFOHEADER2):NULL;
	}
	else if (pmt->formattype==FORMAT_MPEGVideo)
	{
		MPEG1VIDEOINFO*		mpeg1info = (MPEG1VIDEOINFO*)pmt->pbFormat;
		if (mpeg1info->cbSequenceHeader)
		{
			size = mpeg1info->cbSequenceHeader;
			data = mpeg1info->bSequenceHeader;
		}
	}
	else if (pmt->formattype==FORMAT_MPEG2Video)
	{
		MPEG2VIDEOINFO*		mpeg2info = (MPEG2VIDEOINFO*)pmt->pbFormat;
		if (mpeg2info->cbSequenceHeader)
		{
			size = mpeg2info->cbSequenceHeader;
			data = (const uint8_t*)mpeg2info->dwSequenceHeader;
		}
	}
	else if (pmt->formattype==FORMAT_VorbisFormat2)
	{
		const VORBISFORMAT2 *vf2=(const VORBISFORMAT2*)pmt->pbFormat;
		size=pmt->cbFormat-sizeof(VORBISFORMAT2);
		data=size?pmt->pbFormat+sizeof(VORBISFORMAT2):NULL;
	}

	if (size)
	{
		pAVCtx->extradata_size	= size;
		pAVCtx->extradata		= (const unsigned char*)calloc(1,size+FF_INPUT_BUFFER_PADDING_SIZE);
		memcpy((void*)pAVCtx->extradata, data, size);
	}
}


HRESULT CMPCVideoDecFilter::CompleteConnect(PIN_DIRECTION direction, IPin* pReceivePin)
{
	if ( (direction==PINDIR_OUTPUT) &&
		 ffCodecs[m_nCodecNb].bSupportDXVA &&
		 SUCCEEDED (ConfigureDXVA2 (pReceivePin)) &&
		 SUCCEEDED (SetEVRForDXVA2 (pReceivePin)) )
	{
		m_bUseDXVA2 = true;
	}

	return __super::CompleteConnect (direction, pReceivePin);
}


HRESULT CMPCVideoDecFilter::Transform(IMediaSample* pIn)
{
	HRESULT			hr;
	BYTE*			pDataIn = NULL;
	long			nSize;
	int				got_picture;
	REFERENCE_TIME	rtStart = _I64_MIN, rtStop = _I64_MIN;

	if(FAILED(hr = pIn->GetPointer(&pDataIn)))
		return hr;

	nSize	= pIn->GetActualDataLength();
	hr		= pIn->GetTime(&rtStart, &rtStop);

	m_pAVCtx->parserRtStart=&rtStart;
	int	xx = avcodec_decode_video (m_pAVCtx, m_pFrame, &got_picture, pDataIn, nSize);
	if (!got_picture || !m_pFrame->data[0]) return S_FALSE;


	if(pIn->IsPreroll() == S_OK || rtStart < 0)
		return S_OK;

	CComPtr<IMediaSample>	pOut;
	BYTE*					pDataOut = NULL;
	if(FAILED(hr = GetDeliveryBuffer(m_pAVCtx->width, m_pAVCtx->height, &pOut)) || FAILED(hr = pOut->GetPointer(&pDataOut)))
		return hr;

	pOut->SetTime(&rtStart, &rtStop);
	pOut->SetDiscontinuity(pIn->IsDiscontinuity() == S_OK);

	CopyBuffer(pDataOut, m_pFrame->data, m_pAVCtx->width, m_pAVCtx->height, m_pFrame->linesize[0], MEDIASUBTYPE_I420, false);

	return m_pOutput->Deliver(pOut);
}

void CMPCVideoDecFilter::FillInVideoDescription(DXVA2_VideoDesc *pDesc)
{
	pDesc->SampleWidth	= m_pAVCtx->width;
	pDesc->SampleHeight	= m_pAVCtx->height;
	pDesc->Format       = D3DFMT_YUY2;
}

BOOL CMPCVideoDecFilter::IsSupportedDecoderMode(const GUID& mode)
{
	// TODO !!
	return true;
}

BOOL CMPCVideoDecFilter::IsSupportedDecoderConfig(const DXVA2_ConfigPictureDecode& config)
{
	// TODO !!!
	return true;
}

HRESULT CMPCVideoDecFilter::FindDecoderConfiguration(IDirectXVideoDecoderService *pDecoderService,
													 const GUID& guidDecoder, 
													 DXVA2_ConfigPictureDecode *pSelectedConfig,
													 BOOL *pbFoundDXVA2Configuration)
{
    HRESULT hr = S_OK;
    UINT cFormats = 0;
    UINT cConfigurations = 0;

    D3DFORMAT                   *pFormats = NULL;           // size = cFormats
    DXVA2_ConfigPictureDecode   *pConfig = NULL;            // size = cConfigurations

    // Find the valid render target formats for this decoder GUID.
    hr = pDecoderService->GetDecoderRenderTargets(
        guidDecoder,
        &cFormats,
        &pFormats
        );


    if (SUCCEEDED(hr))
    {
        // Look for a format that matches our output format.
        for (UINT iFormat = 0; iFormat < cFormats;  iFormat++)
        {
			char_t name[256] = _l("");
//			fourcc2str((FOURCC) pFormats[iFormat], name, 256);
//			DPRINTF(_l("TffdshowDecVideo::FindDecoderConfiguration Format supported by the renderer : %s (%d)"), name, pFormats[iFormat]);
			
			// Albain : not sure about these tests (mapping between supported formats by DXVA driver, and by ffdshow)
			//if (!(pFormats[iFormat] == D3DFMT_YUY2 && presetSettings->output->yuy2) &&
			//	!(pFormats[iFormat] == D3DFMT_UYVY && presetSettings->output->uyvy) &&
			//	!((pFormats[iFormat] == D3DFMT_X1R5G5B5 || pFormats[iFormat] == D3DFMT_A1R5G5B5) && presetSettings->output->rgb555) &&
			//	!(pFormats[iFormat] == D3DFMT_R5G6B5 && presetSettings->output->rgb565) &&
			//	!(pFormats[iFormat] == D3DFMT_R8G8B8 && presetSettings->output->rgb24) &&
			//	!(pFormats[iFormat] == D3DFMT_X8B8G8R8 && presetSettings->output->rgb32) &&
			//	!(pFormats[iFormat] == MAKEFOURCC('I', 'M', 'C', '4') && presetSettings->output->yv12))
   //         {
   //             continue;
   //         }

            // Fill in the video description. Set the width, height, format, and frame rate.
            FillInVideoDescription(&m_VideoDesc); // Private helper function.
            m_VideoDesc.Format = pFormats[iFormat];

            // Get the available configurations.
            hr = pDecoderService->GetDecoderConfigurations(
                guidDecoder,
                &m_VideoDesc,
                NULL, // Reserved.
                &cConfigurations,
                &pConfig
                );

            if (FAILED(hr))
            {
                break;
            }

            // Find a supported configuration.
            for (UINT iConfig = 0; iConfig < cConfigurations; iConfig++)
            {
                if (IsSupportedDecoderConfig(pConfig[iConfig]))
                {
                    // This configuration is good.
                    *pbFoundDXVA2Configuration = TRUE;
                    *pSelectedConfig = pConfig[iConfig];
                    break;
                }
            }

            CoTaskMemFree(pConfig);
            break;

        } // End of formats loop.
    }

    CoTaskMemFree(pFormats);

    // Note: It is possible to return S_OK without finding a configuration.
    return hr;
}


HRESULT CMPCVideoDecFilter::ConfigureDXVA2(IPin *pPin)
{
    HRESULT hr						 = S_OK;
    UINT    cDecoderGuids			 = 0;
    BOOL    bFoundDXVA2Configuration = FALSE;
    GUID    guidDecoder				 = GUID_NULL;

    DXVA2_ConfigPictureDecode config;
    ZeroMemory(&config, sizeof(config));

    CComPtr<IMFGetService>					pGetService;
    CComPtr<IDirect3DDeviceManager9>		pDeviceManager;
    CComPtr<IDirectXVideoDecoderService>	pDecoderService;
    GUID*									pDecoderGuids = NULL;
    HANDLE									hDevice = INVALID_HANDLE_VALUE;

    // Query the pin for IMFGetService.
    hr = pPin->QueryInterface(__uuidof(IMFGetService), (void**)&pGetService);

    // Get the Direct3D device manager.
    if (SUCCEEDED(hr))
    {
        hr = pGetService->GetService(
            MR_VIDEO_ACCELERATION_SERVICE,
            __uuidof(IDirect3DDeviceManager9),
            (void**)&pDeviceManager);
    }

    // Open a new device handle.
    if (SUCCEEDED(hr))
    {
        hr = pDeviceManager->OpenDeviceHandle(&hDevice);
    } 

    // Get the video decoder service.
    if (SUCCEEDED(hr))
    {
        hr = pDeviceManager->GetVideoService(
                hDevice, 
                __uuidof(IDirectXVideoDecoderService), 
                (void**)&pDecoderService);
    }

    // Get the decoder GUIDs.
    if (SUCCEEDED(hr))
    {
        hr = pDecoderService->GetDecoderDeviceGuids(&cDecoderGuids, &pDecoderGuids);
    }

    if (SUCCEEDED(hr))
    {
        // Look for the decoder GUIDs we want.
        for (UINT iGuid = 0; iGuid < cDecoderGuids; iGuid++)
        {
            // Do we support this mode?
            if (!IsSupportedDecoderMode(pDecoderGuids[iGuid]))
            {
                continue;
            }

            // Find a configuration that we support. 
            hr = FindDecoderConfiguration(pDecoderService, pDecoderGuids[iGuid], &config, &bFoundDXVA2Configuration);

            if (FAILED(hr))
            {
                break;
            }

            if (bFoundDXVA2Configuration)
            {
                // Found a good configuration. Save the GUID.
                guidDecoder = pDecoderGuids[iGuid];
            }
        }
    }

    if (!bFoundDXVA2Configuration)
    {
        hr = E_FAIL; // Unable to find a configuration.
    }

    if (SUCCEEDED(hr))
    {
        // Store the things we will need later.
		m_pDeviceManager	= pDeviceManager;
        m_pDecoderService	= pDecoderService;

        m_DecoderConfig		= config;
        m_DecoderGuid		= guidDecoder;
        m_hDevice			= hDevice;
    }

    if (FAILED(hr))
    {
        if (hDevice != INVALID_HANDLE_VALUE)
        {
            pDeviceManager->CloseDeviceHandle(hDevice);
        }
    }

    return hr;
}


HRESULT CMPCVideoDecFilter::SetEVRForDXVA2(IPin *pPin)
{
    HRESULT hr = S_OK;

    CComPtr<IMFGetService>						pGetService;
    CComPtr<IDirectXVideoMemoryConfiguration>	pVideoConfig;

    // Query the pin for IMFGetService.
    hr = pPin->QueryInterface(__uuidof(IMFGetService), (void**)&pGetService);

    // Get the IDirectXVideoMemoryConfiguration interface.
    if (SUCCEEDED(hr))
    {
        hr = pGetService->GetService(
            MR_VIDEO_ACCELERATION_SERVICE,
            __uuidof(IDirectXVideoMemoryConfiguration),
            (void**)&pVideoConfig);
    }

    // Notify the EVR. 
    if (SUCCEEDED(hr))
    {
        DXVA2_SurfaceType surfaceType;

        for (DWORD iTypeIndex = 0; ; iTypeIndex++)
        {
            hr = pVideoConfig->GetAvailableSurfaceTypeByIndex(iTypeIndex, &surfaceType);
            
            if (FAILED(hr))
            {
                break;
            }

            if (surfaceType == DXVA2_SurfaceType_DecoderRenderTarget)
            {
                hr = pVideoConfig->SetSurfaceType(DXVA2_SurfaceType_DecoderRenderTarget);
                break;
            }
        }
    }

    return hr;
}


// ISpecifyPropertyPages2

STDMETHODIMP CMPCVideoDecFilter::GetPages(CAUUID* pPages)
{
	CheckPointer(pPages, E_POINTER);

	pPages->cElems		= 1;
	pPages->pElems		= (GUID*)CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
	pPages->pElems[0]	= __uuidof(CMPCVideoDecSettingsWnd);

	return S_OK;
}

STDMETHODIMP CMPCVideoDecFilter::CreatePage(const GUID& guid, IPropertyPage** ppPage)
{
	CheckPointer(ppPage, E_POINTER);

	if(*ppPage != NULL) return E_INVALIDARG;

	HRESULT hr;

	if(guid == __uuidof(CMPCVideoDecSettingsWnd))
	{
		(*ppPage = new CInternalPropertyPageTempl<CMPCVideoDecSettingsWnd>(NULL, &hr))->AddRef();
	}

	return *ppPage ? S_OK : E_FAIL;
}


// IFfmpegDecFilter


