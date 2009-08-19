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

#include <initguid.h>
#include <moreuuids.h>
#include "..\..\..\DSUtil\DSUtil.h"

#include "MpcAudioRenderer.h"


#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM},
};

const AMOVIESETUP_PIN sudpPins[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CMpcAudioRenderer), L"MPC - Audio Renderer", MERIT_DO_NOT_USE/*0x40000001*/, countof(sudpPins), sudpPins},
};

CFactoryTemplate g_Templates[] =
{
    {sudFilter[0].strName, &__uuidof(CMpcAudioRenderer), CreateInstance<CMpcAudioRenderer>, NULL, &sudFilter[0]},
	//{L"CMpcAudioRendererPropertyPage", &__uuidof(CMpcAudioRendererSettingsWnd), CreateInstance<CInternalPropertyPageTempl<CMpcAudioRendererSettingsWnd> >},
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

CMpcAudioRenderer::CMpcAudioRenderer(LPUNKNOWN punk, HRESULT *phr)
: CBaseRenderer(__uuidof(this), NAME("MPC - Audio Renderer"), punk, phr)
, m_pDSBuffer		(NULL )
, m_pDS				(NULL )
, m_dwDSWriteOff	(0	  )
, m_nDSBufSize		(0	  )
, m_dRate			(1.0  )
, m_pReferenceClock	(NULL )
, m_pWaveFileFormat	(NULL )
{
	m_pSoundTouch	= new soundtouch::SoundTouch();
	*phr = DirectSoundCreate8 (NULL, &m_pDS, NULL);
}


CMpcAudioRenderer::~CMpcAudioRenderer()
{
	Stop();

	SAFE_DELETE  (m_pSoundTouch);
	SAFE_RELEASE (m_pDSBuffer);
	SAFE_RELEASE (m_pDS);
	
	if (m_pReferenceClock)
	{
		SetSyncSource(NULL);
		SAFE_RELEASE (m_pReferenceClock);
	}

	if (m_pWaveFileFormat)
	{
		BYTE *p = (BYTE *)m_pWaveFileFormat;
		SAFE_DELETE_ARRAY(p);
	}
}


HRESULT CMpcAudioRenderer::CheckInputType(const CMediaType *pmt)
{
	return CheckMediaType(pmt);
}

HRESULT	CMpcAudioRenderer::CheckMediaType(const CMediaType *pmt)
{
	if (pmt == NULL) return E_INVALIDARG;

	WAVEFORMATEX *pwfx = (WAVEFORMATEX *) pmt->Format();

	if (pwfx == NULL) return VFW_E_TYPE_NOT_ACCEPTED;

	if ((pmt->majortype		!= MEDIATYPE_Audio		) ||
		(pmt->formattype	!= FORMAT_WaveFormatEx	) ||
		(pwfx->wFormatTag	!= WAVE_FORMAT_PCM		))
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	return S_OK;
}

void CMpcAudioRenderer::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
	ClearBuffer();
}

BOOL CMpcAudioRenderer::ScheduleSample(IMediaSample *pMediaSample)
{
	REFERENCE_TIME		StartSample;
	REFERENCE_TIME		EndSample;

    // Is someone pulling our leg
    if (pMediaSample == NULL) return FALSE;


    // Get the next sample due up for rendering.  If there aren't any ready
    // then GetNextSampleTimes returns an error.  If there is one to be done
    // then it succeeds and yields the sample times. If it is due now then
    // it returns S_OK other if it's to be done when due it returns S_FALSE
    HRESULT hr = GetSampleTimes(pMediaSample, &StartSample, &EndSample);
    if (FAILED(hr)) return FALSE;

    // If we don't have a reference clock then we cannot set up the advise
    // time so we simply set the event indicating an image to render. This
    // will cause us to run flat out without any timing or synchronisation
    if (hr == S_OK) 
	{
		EXECUTE_ASSERT(SetEvent((HANDLE) m_RenderEvent));
		return TRUE;
    }

	if (m_dRate <= 1.1)
	{
		ASSERT(m_dwAdvise == 0);
		ASSERT(m_pClock);
		WaitForSingleObject((HANDLE)m_RenderEvent,0);

		hr = m_pClock->AdviseTime( (REFERENCE_TIME) m_tStart, StartSample, (HEVENT)(HANDLE) m_RenderEvent, &m_dwAdvise);
		if (SUCCEEDED(hr)) 	return TRUE;
	}
	else
		hr = DoRenderSample (pMediaSample);

    // We could not schedule the next sample for rendering despite the fact
    // we have a valid sample here. This is a fair indication that either
    // the system clock is wrong or the time stamp for the sample is duff
    ASSERT(m_dwAdvise == 0);
    return FALSE;
}

HRESULT	CMpcAudioRenderer::DoRenderSample(IMediaSample *pMediaSample)
{
	return DoRenderSampleDirectSound(pMediaSample);
}


STDMETHODIMP CMpcAudioRenderer::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IReferenceClock)
	{
		return GetReferenceClockInterface(riid, ppv);
	}

	return CBaseRenderer::NonDelegatingQueryInterface (riid, ppv);
}

HRESULT CMpcAudioRenderer::SetMediaType(const CMediaType *pmt)
{
	if (! pmt) return E_POINTER;

	WAVEFORMATEX	*pwf	= (WAVEFORMATEX *) pmt->Format();
	int				size	= sizeof(WAVEFORMATEX) + pwf->cbSize;

	if (m_pWaveFileFormat)
	{
		BYTE *p = (BYTE *)m_pWaveFileFormat;
		SAFE_DELETE_ARRAY(p);
	}

	m_pWaveFileFormat = (WAVEFORMATEX *)new BYTE[size];
	if (! m_pWaveFileFormat)
		return E_OUTOFMEMORY;

 	memcpy(m_pWaveFileFormat, pwf, size);

	if (m_pSoundTouch && (pwf->nChannels <= 2))
	{
		m_pSoundTouch->setSampleRate (pwf->nSamplesPerSec);
		m_pSoundTouch->setChannels (pwf->nChannels);
		m_pSoundTouch->setTempoChange (0);
		m_pSoundTouch->setPitchSemiTones(0);
	}

	return CBaseRenderer::SetMediaType (pmt);
}

HRESULT CMpcAudioRenderer::CompleteConnect(IPin *pReceivePin)
{
	HRESULT			hr = S_OK;

	if (! m_pDS) return E_FAIL;

	if (SUCCEEDED(hr)) hr = CBaseRenderer::CompleteConnect(pReceivePin);
	if (SUCCEEDED(hr)) hr = InitCoopLevel();
	if (SUCCEEDED(hr)) hr = CreateDSBuffer();

	return hr;

}

STDMETHODIMP CMpcAudioRenderer::Run(REFERENCE_TIME tStart)
{
	HRESULT		hr;

    if (m_State == State_Running) return NOERROR;

	if (m_pDSBuffer && 
		m_pPosition &&
		m_pWaveFileFormat && 
		SUCCEEDED(m_pPosition->GetRate(&m_dRate))) 
	{
		if (m_dRate < 1.0)
		{
			hr = m_pDSBuffer->SetFrequency ((long)(m_pWaveFileFormat->nSamplesPerSec * m_dRate));
			if (FAILED (hr)) return hr;
		}
		else
		{
			hr = m_pDSBuffer->SetFrequency ((long)m_pWaveFileFormat->nSamplesPerSec);
			m_pSoundTouch->setRateChange((float)(m_dRate-1.0)*100);
		}
	}

	ClearBuffer();
	hr = CBaseRenderer::Run(tStart);

	return hr;
}

STDMETHODIMP CMpcAudioRenderer::Stop() 
{
	if (m_pDSBuffer) m_pDSBuffer->Stop();

	HRESULT hr = CBaseRenderer::Stop(); 
	
	return hr;
};


STDMETHODIMP CMpcAudioRenderer::Pause()
{
	if (m_pDSBuffer) m_pDSBuffer->Stop();

	HRESULT hr = CBaseRenderer::Pause(); 

	return hr;
};


HRESULT CMpcAudioRenderer::GetReferenceClockInterface(REFIID riid, void **ppv)
{
	HRESULT hr = S_OK;

	if (m_pReferenceClock)
		return m_pReferenceClock->NonDelegatingQueryInterface(riid, ppv);

	m_pReferenceClock = new CBaseReferenceClock (NAME("Mpc Audio Clock"), NULL, &hr);
	if (! m_pReferenceClock)
		return E_OUTOFMEMORY;

	m_pReferenceClock->AddRef();

	hr = SetSyncSource(m_pReferenceClock);
	if (FAILED(hr)) 
	{
		SetSyncSource(NULL);
		return hr;
	}

	return GetReferenceClockInterface(riid, ppv);
}



HRESULT CMpcAudioRenderer::EndOfStream(void)
{
	m_pDSBuffer->Stop();

	return CBaseRenderer::EndOfStream();
}


#pragma region // ==== DirectSound

HRESULT CMpcAudioRenderer::CreateDSBuffer()
{
	if (! m_pWaveFileFormat) return E_POINTER;

	HRESULT					hr				= S_OK;
	LPDIRECTSOUNDBUFFER		pDSBPrimary		= NULL;
	DSBUFFERDESC			dsbd;
	DSBUFFERDESC			cDSBufferDesc;
	DSBCAPS					bufferCaps;
	DWORD					dwDSBufSize		= m_pWaveFileFormat->nAvgBytesPerSec * 4;

	ZeroMemory(&bufferCaps, sizeof(bufferCaps));
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));

	dsbd.dwSize        = sizeof(DSBUFFERDESC);
	dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
	dsbd.dwBufferBytes = 0;
	dsbd.lpwfxFormat   = NULL;
	if (SUCCEEDED (hr = m_pDS->CreateSoundBuffer( &dsbd, &pDSBPrimary, NULL )))
	{
		hr = pDSBPrimary->SetFormat(m_pWaveFileFormat);
		ATLASSERT(SUCCEEDED(hr));
		SAFE_RELEASE (pDSBPrimary);
	}


	SAFE_RELEASE (m_pDSBuffer);
	cDSBufferDesc.dwSize			= sizeof (DSBUFFERDESC);
	cDSBufferDesc.dwFlags			= DSBCAPS_GLOBALFOCUS			| 
									  DSBCAPS_GETCURRENTPOSITION2	| 
									  DSBCAPS_CTRLVOLUME 			|
									  DSBCAPS_CTRLPAN				|
									  DSBCAPS_CTRLFREQUENCY; 
	cDSBufferDesc.dwBufferBytes		= dwDSBufSize; 
	cDSBufferDesc.dwReserved		= 0; 
	cDSBufferDesc.lpwfxFormat		= m_pWaveFileFormat; 
   	cDSBufferDesc.guid3DAlgorithm	= GUID_NULL; 

	hr = m_pDS->CreateSoundBuffer (&cDSBufferDesc,  &m_pDSBuffer, NULL);

	m_nDSBufSize = 0;
	if (SUCCEEDED(hr))
	{
		bufferCaps.dwSize = sizeof(bufferCaps);
		hr = m_pDSBuffer->GetCaps(&bufferCaps);
	}
	if (SUCCEEDED (hr))
	{
		m_nDSBufSize = bufferCaps.dwBufferBytes;
		hr = ClearBuffer();
		m_pDSBuffer->SetFrequency ((long)(m_pWaveFileFormat->nSamplesPerSec * m_dRate));
	}

	return hr;
}



HRESULT CMpcAudioRenderer::ClearBuffer()
{
	HRESULT			hr				 = S_FALSE;
	VOID*			pDSLockedBuffer  = NULL;
	DWORD			dwDSLockedSize   = 0;

	if (m_pDSBuffer)
	{
		m_dwDSWriteOff = 0;
		m_pDSBuffer->SetCurrentPosition(0);

		hr = m_pDSBuffer->Lock (0, 0, &pDSLockedBuffer, &dwDSLockedSize, NULL, NULL, DSBLOCK_ENTIREBUFFER);
		if (SUCCEEDED (hr))
		{
			memset (pDSLockedBuffer, 0, dwDSLockedSize);
			hr = m_pDSBuffer->Unlock (pDSLockedBuffer, dwDSLockedSize, NULL, NULL);
		}
	}

	return hr;
}

HRESULT CMpcAudioRenderer::InitCoopLevel()
{
	HRESULT				hr				= S_OK;
	IVideoWindow*		pVideoWindow	= NULL;
	HWND				hWnd			= NULL;
	CComBSTR			bstrCaption;

	hr = m_pGraph->QueryInterface (__uuidof(IVideoWindow), (void**) &pVideoWindow);
	if (SUCCEEDED (hr))
	{
		pVideoWindow->get_Owner((long*)&hWnd);
		SAFE_RELEASE (pVideoWindow);
	}
	if (!hWnd) 
	{
		hWnd = GetTopWindow(NULL);
	}

	ATLASSERT(hWnd != NULL);
	hr = m_pDS->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);

	return hr;
}

HRESULT	CMpcAudioRenderer::DoRenderSampleDirectSound(IMediaSample *pMediaSample)
{
	HRESULT			hr					= S_OK;
	DWORD			dwStatus			= 0;
	const long		lSize				= pMediaSample->GetActualDataLength();
	DWORD			dwPlayCursor		= 0;
	DWORD			dwWriteCursor		= 0;

	hr = m_pDSBuffer->GetStatus(&dwStatus);

	if (SUCCEEDED(hr) && (dwStatus & DSBSTATUS_BUFFERLOST))
		hr = m_pDSBuffer->Restore();

	if ((SUCCEEDED(hr)) && ((dwStatus & DSBSTATUS_PLAYING) != DSBSTATUS_PLAYING))
	{
		hr = m_pDSBuffer->Play( 0, 0, DSBPLAY_LOOPING);
		ATLASSERT(SUCCEEDED(hr));
	}
	
	if (SUCCEEDED(hr)) hr = m_pDSBuffer->GetCurrentPosition(&dwPlayCursor, &dwWriteCursor);

	if (SUCCEEDED(hr))
	{
		if ( (  (dwPlayCursor < dwWriteCursor)		&&
				(
					((m_dwDSWriteOff >= dwPlayCursor) && (m_dwDSWriteOff <=  dwWriteCursor))
					||
					((m_dwDSWriteOff < dwPlayCursor) && (m_dwDSWriteOff + lSize >= dwPlayCursor))
				)
			 ) 
			 ||
			 (  (dwWriteCursor < dwPlayCursor) && 
				( 
					(m_dwDSWriteOff >= dwPlayCursor) || (m_dwDSWriteOff <  dwWriteCursor)
			    ) ) )
		{
			m_dwDSWriteOff = dwWriteCursor;
		}

		if (m_dwDSWriteOff >= (DWORD)m_nDSBufSize)
		{
			m_dwDSWriteOff = 0;
		}
	}

	if (SUCCEEDED(hr)) hr = WriteSampleToDSBuffer(pMediaSample, NULL);
	
	return hr;
}

HRESULT CMpcAudioRenderer::WriteSampleToDSBuffer(IMediaSample *pMediaSample, bool *looped)
{
	if (! m_pDSBuffer) return E_POINTER;

	REFERENCE_TIME	rtStart				= 0;
	REFERENCE_TIME	rtStop				= 0;
	HRESULT			hr					= S_OK;
	bool			loop				= false;
	VOID*			pDSLockedBuffers[2] = { NULL, NULL };
	DWORD			dwDSLockedSize[2]	= {    0,    0 };
	BYTE*			pMediaBuffer		= NULL;
	long			lSize				= pMediaSample->GetActualDataLength();

	hr = pMediaSample->GetPointer(&pMediaBuffer);

	if (m_dRate > 1.0)
	{
		int		nBytePerSample = m_pWaveFileFormat->nChannels * (m_pWaveFileFormat->wBitsPerSample/8);
		m_pSoundTouch->putSamples((const short*)pMediaBuffer, lSize / nBytePerSample);
		lSize = m_pSoundTouch->receiveSamples ((short*)pMediaBuffer, lSize / nBytePerSample) * nBytePerSample;
	}

	pMediaSample->GetTime (&rtStart, &rtStop);

	if (rtStart < 0)
	{
		DWORD		dwPercent	= (DWORD) ((-rtStart * 100) / (rtStop - rtStart));
		DWORD		dwRemove	= (lSize*dwPercent/100);

		dwRemove		 = (dwRemove / m_pWaveFileFormat->nBlockAlign) * m_pWaveFileFormat->nBlockAlign;
		pMediaBuffer	+= dwRemove;
		lSize			-= dwRemove;
	}

	if (SUCCEEDED (hr))
		hr = m_pDSBuffer->Lock (m_dwDSWriteOff, lSize, &pDSLockedBuffers[0], &dwDSLockedSize[0], &pDSLockedBuffers[1], &dwDSLockedSize[1], 0 );
	if (SUCCEEDED (hr))
	{
		if (pDSLockedBuffers [0] != NULL)
		{
			memcpy(pDSLockedBuffers[0], pMediaBuffer, dwDSLockedSize[0]);
			m_dwDSWriteOff += dwDSLockedSize[0];
		}

		if (pDSLockedBuffers [1] != NULL)
		{
			memcpy(pDSLockedBuffers[1], &pMediaBuffer[dwDSLockedSize[0]], dwDSLockedSize[1]);
			m_dwDSWriteOff = dwDSLockedSize[1];
			loop = true;
		}

		hr = m_pDSBuffer->Unlock(pDSLockedBuffers[0], dwDSLockedSize[0], pDSLockedBuffers[1], dwDSLockedSize[1]);
		ATLASSERT (dwDSLockedSize [0] + dwDSLockedSize [1] == (DWORD)lSize);
	}

	if (SUCCEEDED(hr) && looped) *looped = loop;

	return hr;
}

#pragma endregion