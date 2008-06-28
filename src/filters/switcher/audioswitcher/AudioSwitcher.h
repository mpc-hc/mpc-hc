// Copyright 2003 Gabest.
// http://www.gabest.org
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html

#pragma once

#include "StreamSwitcher.h"

[uuid("CEDB2890-53AE-4231-91A3-B0AAFCD1DBDE")]
interface IAudioSwitcherFilter : public IUnknown
{
	STDMETHOD(GetInputSpeakerConfig) (DWORD* pdwChannelMask) = 0;
    STDMETHOD(GetSpeakerConfig) (bool* pfCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18]) = 0;
    STDMETHOD(SetSpeakerConfig) (bool fCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18]) = 0;
    STDMETHOD_(int, GetNumberOfInputChannels) () = 0;
	STDMETHOD_(bool, IsDownSamplingTo441Enabled) () = 0;
	STDMETHOD(EnableDownSamplingTo441) (bool fEnable) = 0;
	STDMETHOD_(REFERENCE_TIME, GetAudioTimeShift) () = 0;
	STDMETHOD(SetAudioTimeShift) (REFERENCE_TIME rtAudioTimeShift) = 0;
	STDMETHOD(GetNormalizeBoost) (bool& fNormalize, bool& fNormalizeRecover, float& boost) = 0;
	STDMETHOD(SetNormalizeBoost) (bool fNormalize, bool fNormalizeRecover, float boost) = 0;
};

class AudioStreamResampler;

[uuid("18C16B08-6497-420e-AD14-22D21C2CEAB7")]
class CAudioSwitcherFilter : public CStreamSwitcherFilter, public IAudioSwitcherFilter
{
	typedef struct {DWORD Speaker, Channel;} ChMap;
	CAtlArray<ChMap> m_chs[18];

	bool m_fCustomChannelMapping;
	DWORD m_pSpeakerToChannelMap[18][18];
	bool m_fDownSampleTo441;
	REFERENCE_TIME m_rtAudioTimeShift;
	CAutoPtrArray<AudioStreamResampler> m_pResamplers;
	double m_sample_max;
	bool m_fNormalize, m_fNormalizeRecover;
	float m_boost;

	REFERENCE_TIME m_rtNextStart, m_rtNextStop;

public:
	CAudioSwitcherFilter(LPUNKNOWN lpunk, HRESULT* phr);

	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	HRESULT CheckMediaType(const CMediaType* pmt);
	HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut);
	CMediaType CreateNewOutputMediaType(CMediaType mt, long& cbBuffer);
	void OnNewOutputMediaType(const CMediaType& mtIn, const CMediaType& mtOut);

	HRESULT DeliverEndFlush();
	HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	// IAudioSwitcherFilter
	STDMETHODIMP GetInputSpeakerConfig(DWORD* pdwChannelMask);
    STDMETHODIMP GetSpeakerConfig(bool* pfCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18]);
    STDMETHODIMP SetSpeakerConfig(bool fCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18]);
    STDMETHODIMP_(int) GetNumberOfInputChannels();
	STDMETHODIMP_(bool) IsDownSamplingTo441Enabled();
	STDMETHODIMP EnableDownSamplingTo441(bool fEnable);
	STDMETHODIMP_(REFERENCE_TIME) GetAudioTimeShift();
	STDMETHODIMP SetAudioTimeShift(REFERENCE_TIME rtAudioTimeShift);
	STDMETHODIMP GetNormalizeBoost(bool& fNormalize, bool& fNormalizeRecover, float& boost);
	STDMETHODIMP SetNormalizeBoost(bool fNormalize, bool fNormalizeRecover, float boost);

	// IAMStreamSelect
	STDMETHODIMP Enable(long lIndex, DWORD dwFlags);
};
