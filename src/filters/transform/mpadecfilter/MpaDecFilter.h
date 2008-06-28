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

#include <atlcoll.h>
#include <stdint.h>
#include "libmad-0.15.0b\msvc++\mad.h"
#include "a52dec-0.7.4\include\a52.h"
#include "dtsdec-0.0.1\include\dts.h"
//#include "faad2\include\neaacdec.h" // conflicts with dxtrans.h
#include "libvorbisidec\ivorbiscodec.h"
#include "..\..\..\decss\DeCSSInputPin.h"
#include "IMpaDecFilter.h"
#include "MpaDecSettingsWnd.h"

struct aac_state_t
{
	void* h; // NeAACDecHandle h;
	DWORD freq;
	BYTE channels;

	aac_state_t();
	~aac_state_t();
	bool open();
	void close();
	bool init(const CMediaType& mt);
};

struct ps2_state_t
{
	bool sync;
	double a[2], b[2];
	ps2_state_t() {reset();}
	void reset() {sync = false; a[0] = a[1] = b[0] = b[1] = 0;}
};

struct vorbis_state_t
{
	vorbis_info vi;
	vorbis_comment vc;
	vorbis_block vb;
	vorbis_dsp_state vd;
	ogg_packet op;
	int packetno;
	double postgain;

	vorbis_state_t();
	~vorbis_state_t();
	void clear();
	bool init(const CMediaType& mt);
};

[uuid("3D446B6F-71DE-4437-BE15-8CE47174340F")]
class CMpaDecFilter 
	: public CTransformFilter
	, public IMpaDecFilter
	, public ISpecifyPropertyPages2
{
protected:
	CCritSec m_csReceive;

	a52_state_t* m_a52_state;
	dts_state_t* m_dts_state;
	aac_state_t m_aac_state;
	mad_stream m_stream;
	mad_frame m_frame;
	mad_synth m_synth;
	ps2_state_t m_ps2_state;
	vorbis_state_t m_vorbis;

	CAtlArray<BYTE> m_buff;
	REFERENCE_TIME m_rtStart;
	bool m_fDiscontinuity;

	float m_sample_max;

	HRESULT ProcessLPCM();
	HRESULT ProcessHdmvLPCM();
	HRESULT ProcessAC3();
	HRESULT ProcessDTS();
	HRESULT ProcessAAC();
	HRESULT ProcessPS2PCM();
	HRESULT ProcessPS2ADPCM();
	HRESULT ProcessVorbis();
	HRESULT ProcessMPA();

	HRESULT GetDeliveryBuffer(IMediaSample** pSample, BYTE** pData);
	HRESULT Deliver(CAtlArray<float>& pBuff, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask = 0);
	HRESULT Deliver(BYTE* pBuff, int size, int bit_rate, BYTE type);
	HRESULT ReconnectOutput(int nSamples, CMediaType& mt);
	CMediaType CreateMediaType(SampleFormat sf, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask = 0);
	CMediaType CreateMediaTypeSPDIF();

protected:
	CCritSec m_csProps;
	SampleFormat m_iSampleFormat;
	bool m_fNormalize;
	int m_iSpeakerConfig[etlast];
	bool m_fDynamicRangeControl[etlast];
	float m_boost;

public:
	CMpaDecFilter(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CMpaDecFilter();

	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

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

	// ISpecifyPropertyPages2

	STDMETHODIMP GetPages(CAUUID* pPages);
	STDMETHODIMP CreatePage(const GUID& guid, IPropertyPage** ppPage);

	// IMpaDecFilter

	STDMETHODIMP SetSampleFormat(SampleFormat sf);
	STDMETHODIMP_(SampleFormat) GetSampleFormat();
	STDMETHODIMP SetNormalize(bool fNormalize);
	STDMETHODIMP_(bool) GetNormalize();
	STDMETHODIMP SetSpeakerConfig(enctype et, int sc);
	STDMETHODIMP_(int) GetSpeakerConfig(enctype et);
	STDMETHODIMP SetDynamicRangeControl(enctype et, bool fDRC);
	STDMETHODIMP_(bool) GetDynamicRangeControl(enctype et);
	STDMETHODIMP SetBoost(float boost);
	STDMETHODIMP_(float) GetBoost();
};

class CMpaDecInputPin : public CDeCSSInputPin
{
public:
    CMpaDecInputPin(CTransformFilter* pFilter, HRESULT* phr, LPWSTR pName);
};
