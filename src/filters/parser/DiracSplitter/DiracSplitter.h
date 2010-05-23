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

#include "../BaseSplitter/BaseSplitter.h"
#include "DiracSplitterFile.h"

class __declspec(uuid("5899CFB9-948F-4869-A999-5544ECB38BA5"))
CDiracSplitterFilter : public CBaseSplitterFilter
{
protected:
	CAutoPtr<CDiracSplitterFile> m_pFile;
	HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

	bool DemuxInit();
	void DemuxSeek(REFERENCE_TIME rt);
	bool DemuxLoop();

public:
	CDiracSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);

	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
};

class __declspec(uuid("09E7F58E-71A1-419D-B0A0-E524AE1454A9"))
CDiracSourceFilter : public CDiracSplitterFilter
{
public:
	CDiracSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};

class __declspec(uuid("F78CF248-180E-4713-B107-B13F7B5C31E1"))
CDiracVideoDecoder : public CTransformFilter
{
    void* m_decoder; // dirac_decoder_t*
	void InitDecoder(), FreeDecoder();
	BYTE* m_pYUV[4];
	bool m_fDropFrames;
	REFERENCE_TIME m_tStart, m_rtAvgTimePerFrame;

	HRESULT Deliver(IMediaSample* pIn, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop);
	void Copy(BYTE* pOut);

public:
	CDiracVideoDecoder(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CDiracVideoDecoder();

	HRESULT Receive(IMediaSample* pIn);
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);

	HRESULT StartStreaming();
	HRESULT StopStreaming();

    HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	HRESULT AlterQuality(Quality q);
};