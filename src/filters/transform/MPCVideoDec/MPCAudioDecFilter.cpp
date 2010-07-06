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
#include <math.h>
#include <atlbase.h>
#include <mmreg.h>

#include "PODtypes.h"
#include "avcodec.h"

#include "MPCAudioDecFilter.h"

#include "../../../DSUtil/DSUtil.h"

#include <initguid.h>
#include <moreuuids.h>


typedef struct
{
	const CLSID*			clsMinorType;
	const enum CodecID		nFFCodec;
	const int				fourcc;
} FFMPEG_CODECS;


const FFMPEG_CODECS		ffCodecs[] =
{
	// AMVA
	{ &MEDIASUBTYPE_IMA_AMV, CODEC_ID_ADPCM_IMA_AMV, MAKEFOURCC('A','M','V','A') },
};

const AMOVIESETUP_MEDIATYPE CMPCAudioDecFilter::sudPinTypesIn[] =
{
	{ &MEDIATYPE_Audio, &MEDIASUBTYPE_IMA_AMV },
};
const int CMPCAudioDecFilter::sudPinTypesInCount = countof(CMPCAudioDecFilter::sudPinTypesIn);


const AMOVIESETUP_MEDIATYPE CMPCAudioDecFilter::sudPinTypesOut[] =
{
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM}
};
const int CMPCAudioDecFilter::sudPinTypesOutCount = countof(CMPCAudioDecFilter::sudPinTypesOut);


CMPCAudioDecFilter::CMPCAudioDecFilter(LPUNKNOWN lpunk, HRESULT* phr)
	: CTransformFilter(NAME("CMPCAudioDecFilter"), lpunk, __uuidof(this))
{
	if(!(m_pInput = new CTransformInputPin(NAME("CAudioDecInputPin"), this, phr, L"In"))) *phr = E_OUTOFMEMORY;
	if(FAILED(*phr)) return;

	if(!(m_pOutput = new CTransformOutputPin(NAME("CAudioDecOutputPin"), this, phr, L"Out"))) *phr = E_OUTOFMEMORY;
	if(FAILED(*phr))  {
		delete m_pInput, m_pInput = NULL;
		return;
	}

	m_iSampleFormat		= SAMPLE_FMT_S16;

	m_pAVCodec			= NULL;
	m_pAVCtx			= NULL;
	m_pFrame			= NULL;
	m_nCodecNb			= -1;

	avcodec_init();
	avcodec_register_all();
	av_log_set_callback(LogLibAVCodec);

}

CMPCAudioDecFilter::~CMPCAudioDecFilter(void)
{
	Cleanup();
}

void CMPCAudioDecFilter::Cleanup()
{
	if (m_pAVCtx)
	{
		avcodec_thread_free (m_pAVCtx);
		av_free(m_pAVCtx);
	}
	if (m_pFrame)	av_free(m_pFrame);

	m_pAVCodec	= NULL;
	m_pAVCtx	= NULL;
	m_pFrame	= NULL;
	m_nCodecNb	= -1;
}

void CMPCAudioDecFilter::LogLibAVCodec(void* par,int level,const char *fmt,va_list valist)
{
#ifdef _DEBUG
	//AVCodecContext*	m_pAVCtx = (AVCodecContext*) par;

	//char		Msg [500];
	//snprintf (Msg, sizeof(Msg), fmt, valist);
	//TRACE("AVLIB : %s", Msg);
#endif
}

STDMETHODIMP CMPCAudioDecFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
//		QI(IMPCVideoDecFilter)
//		QI(ISpecifyPropertyPages)
//		QI(ISpecifyPropertyPages2)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CMPCAudioDecFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	return SUCCEEDED(CheckInputType(mtIn))
		   && mtOut->majortype == MEDIATYPE_Audio && mtOut->subtype == MEDIASUBTYPE_PCM
		   || mtOut->majortype == MEDIATYPE_Audio && mtOut->subtype == MEDIASUBTYPE_IEEE_FLOAT
		   ? S_OK
		   : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CMPCAudioDecFilter::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
	return S_OK;
}

HRESULT CMPCAudioDecFilter::CheckInputType(const CMediaType* mtIn)
{
	for (int i=0; i<sizeof(sudPinTypesIn)/sizeof(AMOVIESETUP_MEDIATYPE); i++)
	{
		if ((mtIn->majortype == *sudPinTypesIn[i].clsMajorType) &&
			(mtIn->subtype == *sudPinTypesIn[i].clsMinorType))
			return S_OK;
	}

	return VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CMPCAudioDecFilter::GetMediaType(int iPosition, CMediaType* pmt)
{
	if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

	if(iPosition < 0) return E_INVALIDARG;
	if(iPosition > 0) return VFW_S_NO_MORE_ITEMS;

	CMediaType mt = m_pInput->CurrentMediaType();
	const GUID& subtype = mt.subtype;
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();

	//if(GetSpeakerConfig(ac3) < 0 && (subtype == MEDIASUBTYPE_DOLBY_AC3 || subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3)
	//|| GetSpeakerConfig(dts) < 0 && (subtype == MEDIASUBTYPE_DTS || subtype == MEDIASUBTYPE_WAVE_DTS))
	//{
	//	*pmt = CreateMediaTypeSPDIF();
	//}
	//else if(subtype == MEDIASUBTYPE_Vorbis2)
	//{
	//	*pmt = CreateMediaType(GetSampleFormat(), m_vorbis.vi.rate, m_vorbis.vi.channels);
	//}
	//else
	//{
		*pmt = CreateMediaType(GetSampleFormat(), wfe->nSamplesPerSec, min(2, wfe->nChannels));
	//}

	return S_OK;
}

HRESULT CMPCAudioDecFilter::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
	return __super::SetMediaType(direction, pmt);
}

HRESULT CMPCAudioDecFilter::CompleteConnect(PIN_DIRECTION direction, IPin* pReceivePin)
{
	int			nNewCodec;

	if (direction == PINDIR_OUTPUT)
	{
		CMediaType&		mt = m_pInput->CurrentMediaType();
		nNewCodec = FindCodec(&mt);
		if ((direction == PINDIR_OUTPUT) && (nNewCodec != -1) && (nNewCodec != m_nCodecNb))
		{
			WAVEFORMATEX*		wfex = (WAVEFORMATEX*) mt.pbFormat;
			Cleanup();

			m_nCodecNb	= nNewCodec;
			m_pInput->CurrentMediaType();

			m_pAVCodec	= avcodec_find_decoder(ffCodecs[nNewCodec].nFFCodec);
			CheckPointer (m_pAVCodec, VFW_E_UNSUPPORTED_AUDIO);

			m_pAVCtx	= avcodec_alloc_context();
			CheckPointer (m_pAVCtx,	  E_POINTER);

			m_pAVCtx->sample_rate	= wfex->nSamplesPerSec;
			m_pAVCtx->channels		= wfex->nChannels;

			m_pAVCtx->bit_rate			= wfex->nAvgBytesPerSec*8;
			m_pAVCtx->bits_per_coded_sample	= wfex->wBitsPerSample;
			m_pAVCtx->block_align		= wfex->nBlockAlign;

			/*if (codecId==CODEC_ID_FLAC && extradata.size>=4 && *(FOURCC*)extradata.data==mmioFOURCC('f','L','a','C')) // HACK
			{
				avctx->extradata=extradata.data+8;
				avctx->extradata_size=34;
			}
			else if (codecId==CODEC_ID_COOK && mt.formattype==FORMAT_WaveFormatEx && mt.pbFormat)
			{
				avctx->extradata=mt.pbFormat+sizeof(WAVEFORMATEX);
				avctx->extradata_size=mt.cbFormat-sizeof(WAVEFORMATEX);
				for (;avctx->extradata_size;avctx->extradata=(uint8_t*)avctx->extradata+1,avctx->extradata_size--)
				if (memcmp(avctx->extradata,"cook",4)==0)
				{
					avctx->extradata=(uint8_t*)avctx->extradata+12;
					avctx->extradata_size-=12;
					break;
				}
			}
			else
			{
				avctx->extradata=extradata.data;
				avctx->extradata_size=(int)extradata.size;
			}
			if (codecId==CODEC_ID_VORBIS && mt.formattype==FORMAT_VorbisFormat2)
			{
				const VORBISFORMAT2 *vf2=(const VORBISFORMAT2*)mt.pbFormat;
				avctx->vorbis_header_size[0]=vf2->HeaderSize[0];
				avctx->vorbis_header_size[1]=vf2->HeaderSize[1];
				avctx->vorbis_header_size[2]=vf2->HeaderSize[2];
			}
			if (libavcodec->avcodec_open(avctx,avcodec)<0) return false;
			codecinited=true;
			switch (avctx->sample_fmt)
			{
				case SAMPLE_FMT_S16:fmt.sf=TsampleFormat::SF_PCM16;break;
				case SAMPLE_FMT_FLT:fmt.sf=TsampleFormat::SF_FLOAT32;break;
			}
			*/

			if (avcodec_open(m_pAVCtx, m_pAVCodec)<0)
				return VFW_E_INVALIDMEDIATYPE;
		}
	}

	return __super::CompleteConnect (direction, pReceivePin);
}

HRESULT CMPCAudioDecFilter::Transform(IMediaSample* pIn)
{
	HRESULT			hr = S_OK;
	//BYTE*			pDataIn = NULL;
	//UINT			nSize;
	//int				got_picture;
	//REFERENCE_TIME	rtStart = _I64_MIN, rtStop = _I64_MIN;

	//if(FAILED(hr = pIn->GetPointer(&pDataIn)))
	//	return hr;

	//nSize	= pIn->GetActualDataLength();
	//hr		= pIn->GetTime(&rtStart, &rtStop);

	////FILE*	File = fopen ("e:\\temp\\h264.bin", "wb");
	////fwrite (pDataIn, nSize, 1, File);
	////fclose (File);

	//int		used_bytes;

	//m_pAVCtx->parserRtStart=&rtStart;
	//
	//while (nSize > 0)
	//{
	//	used_bytes = avcodec_decode_video (m_pAVCtx, m_pFrame, &got_picture, pDataIn, nSize);
	//	if (!got_picture || !m_pFrame->data[0]) return S_FALSE;

	//	if(pIn->IsPreroll() == S_OK || rtStart < 0)
	//		return S_OK;

	//	CComPtr<IMediaSample>	pOut;
	//	BYTE*					pDataOut = NULL;
	//	if(FAILED(hr = GetDeliveryBuffer(m_pAVCtx->width, m_pAVCtx->height, &pOut)) || FAILED(hr = pOut->GetPointer(&pDataOut)))
	//		return hr;

	//	TRACE ("CMPCAudioDecFilter::Transform  %I64d - %I64d\n", rtStart, rtStop);

	//	//rtStart = m_pFrame->rtStart;
	//	//rtStop = m_pFrame->rtStart + 1;

	//	pOut->SetTime(&rtStart, &rtStop);
	//	pOut->SetDiscontinuity(pIn->IsDiscontinuity() == S_OK);

	//	CopyBuffer(pDataOut, m_pFrame->data, m_pAVCtx->width, m_pAVCtx->height, m_pFrame->linesize[0], MEDIASUBTYPE_I420, false);

	//	hr = m_pOutput->Deliver(pOut);

	//	nSize	-= used_bytes;
	//	pDataIn += used_bytes;
	//}

	return hr;
}

int CMPCAudioDecFilter::FindCodec(const CMediaType* mtIn)
{
	for (int i=0; i<countof(ffCodecs); i++)
		if (mtIn->subtype == *ffCodecs[i].clsMinorType)
			return i;

	return -1;
}

STDMETHODIMP_(SampleFormat) CMPCAudioDecFilter::GetSampleFormat()
{
//	CAutoLock cAutoLock(&m_csProps);
	return m_iSampleFormat;
}

CMediaType CMPCAudioDecFilter::CreateMediaType(SampleFormat sf, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask)
{
	CMediaType mt;

	mt.majortype = MEDIATYPE_Audio;
	mt.subtype = sf == SAMPLE_FMT_FLT ? MEDIASUBTYPE_IEEE_FLOAT : MEDIASUBTYPE_PCM;
	mt.formattype = FORMAT_WaveFormatEx;

	WAVEFORMATEXTENSIBLE wfex;
	memset(&wfex, 0, sizeof(wfex));
	WAVEFORMATEX* wfe = &wfex.Format;
	wfe->wFormatTag = (WORD)mt.subtype.Data1;
	wfe->nChannels = nChannels;
	wfe->nSamplesPerSec = nSamplesPerSec;
	switch(sf)
	{
	default:
	case SAMPLE_FMT_S16:
		wfe->wBitsPerSample = 16;
		break;
	case SAMPLE_FMT_S32:
	case SAMPLE_FMT_FLT:
		wfe->wBitsPerSample = 32;
		break;
	}
	wfe->nBlockAlign = wfe->nChannels*wfe->wBitsPerSample/8;
	wfe->nAvgBytesPerSec = wfe->nSamplesPerSec*wfe->nBlockAlign;

	// FIXME: 32 bit only seems to work with WAVE_FORMAT_EXTENSIBLE
	if(dwChannelMask == 0 && (sf == SAMPLE_FMT_S32))
		dwChannelMask = nChannels == 2 ? (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT) : SPEAKER_FRONT_CENTER;

	if(dwChannelMask)
	{
		wfex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wfex.Format.cbSize = sizeof(wfex) - sizeof(wfex.Format);
		wfex.dwChannelMask = dwChannelMask;
		wfex.Samples.wValidBitsPerSample = wfex.Format.wBitsPerSample;
		wfex.SubFormat = mt.subtype;
	}

	mt.SetFormat((BYTE*)&wfex, sizeof(wfex.Format) + wfex.Format.cbSize);

	return mt;
}
