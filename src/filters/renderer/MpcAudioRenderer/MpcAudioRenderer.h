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

#include <streams.h>
#include <dsound.h>

#include <MMReg.h>  //must be before other Wasapi headers
#include <strsafe.h>
#include <mmdeviceapi.h>
#include <Avrt.h>
#include <audioclient.h>
#include <Endpointvolume.h>


#include "SoundTouch\Include\SoundTouch.h"

[uuid("601D2A2B-9CDE-40bd-8650-0485E3522727")]
class CMpcAudioRenderer : public CBaseRenderer
{
public:
	CMpcAudioRenderer(LPUNKNOWN punk, HRESULT *phr);
	virtual ~CMpcAudioRenderer();

    static const AMOVIESETUP_FILTER sudASFilter;

	HRESULT					CheckInputType			(const CMediaType* mtIn);
	virtual HRESULT			CheckMediaType			(const CMediaType *pmt);
	virtual HRESULT			DoRenderSample			(IMediaSample *pMediaSample);
	virtual void			OnReceiveFirstSample	(IMediaSample *pMediaSample);
	BOOL					ScheduleSample			(IMediaSample *pMediaSample);
	virtual HRESULT			SetMediaType			(const CMediaType *pmt);
    virtual HRESULT			CompleteConnect			(IPin *pReceivePin);

	HRESULT EndOfStream(void);


    DECLARE_IUNKNOWN

	STDMETHODIMP			NonDelegatingQueryInterface(REFIID riid, void **ppv);

	// === IMediaFilter
	STDMETHOD(Run)				(REFERENCE_TIME tStart);
	STDMETHOD(Stop)				();
	STDMETHOD(Pause)			();

private:

	HRESULT					DoRenderSampleDirectSound(IMediaSample *pMediaSample);

	HRESULT					InitCoopLevel();
	HRESULT					ClearBuffer();
	HRESULT					CreateDSBuffer();
	HRESULT					GetReferenceClockInterface(REFIID riid, void **ppv);
	HRESULT					WriteSampleToDSBuffer(IMediaSample *pMediaSample, bool *looped);
 HRESULT     GetDefaultAudioDevice(IAudioClient **ppAudioClient);
 HRESULT     InitDevice(IAudioClient *pAudioClient, WAVEFORMATEX *pWaveFormatEx, IAudioRenderClient **ppRenderClient);

	LPDIRECTSOUND8			m_pDS;
	LPDIRECTSOUNDBUFFER		m_pDSBuffer;
	DWORD					m_dwDSWriteOff;
	WAVEFORMATEX			*m_pWaveFileFormat;
	int						m_nDSBufSize;
	CBaseReferenceClock*	m_pReferenceClock;
	double					m_dRate;
 soundtouch::SoundTouch*	m_pSoundTouch;

 // WASAPI
 HRESULT	DoRenderSampleWasapi(IMediaSample *pMediaSample);

 bool               useWASAPI;
 IAudioClient       *pAudioClient;
 IAudioRenderClient *pRenderClient;
 //HANDLE             hEvent;
 UINT32             bufferFrameCount;
 REFERENCE_TIME     hnsRequestedDuration;
 HANDLE             hTask;

};

