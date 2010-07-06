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

enum SampleFormat;

class __declspec(uuid("BF67339B-465E-4c5a-AE2D-DC4EE17EA272"))
	CMPCAudioDecFilter
	: public CTransformFilter
{
public:

	const static AMOVIESETUP_MEDIATYPE		sudPinTypesIn[];
	const static int						sudPinTypesInCount;
	const static AMOVIESETUP_MEDIATYPE		sudPinTypesOut[];
	const static int						sudPinTypesOutCount;

	CMPCAudioDecFilter(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CMPCAudioDecFilter();

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT Transform(IMediaSample* pIn);

	HRESULT GetMediaType(int iPosition, CMediaType* pmt);
	HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);

//	HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT CMPCAudioDecFilter::CompleteConnect(PIN_DIRECTION direction, IPin* pReceivePin);

	STDMETHODIMP_(SampleFormat) GetSampleFormat();
	CMediaType					CreateMediaType(SampleFormat sf, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask = 0);

protected :
	SampleFormat				m_iSampleFormat;

	AVCodec*					m_pAVCodec;
	AVCodecContext*				m_pAVCtx;
	AVFrame*					m_pFrame;
	int							m_nCodecNb;

	int							FindCodec(const CMediaType* mtIn);
	void						Cleanup();

	static void					LogLibAVCodec(void* par,int level,const char *fmt,va_list valist);
};
