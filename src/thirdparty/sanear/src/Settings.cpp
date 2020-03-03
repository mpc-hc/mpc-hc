#include "pch.h"
#include "Settings.h"

namespace SaneAudioRenderer
{
    Settings::Settings(IUnknown* pUnknown)
        : CUnknown("Audio Renderer Settings", pUnknown)
    {
    }

    STDMETHODIMP Settings::NonDelegatingQueryInterface(REFIID riid, void** ppv)
    {
        return (riid == __uuidof(ISettings)) ?
                   GetInterface(static_cast<ISettings*>(this), ppv) :
                   CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }

    STDMETHODIMP_(UINT32) Settings::GetSerial()
    {
        return m_serial;
    }

    STDMETHODIMP Settings::SetOuputDevice(LPCWSTR pDeviceId, BOOL bExclusive, UINT32 uBufferMS)
    {
        if (uBufferMS < OUTPUT_DEVICE_BUFFER_MIN_MS || uBufferMS > OUTPUT_DEVICE_BUFFER_MAX_MS)
            return E_INVALIDARG;

        CAutoLock lock(this);

        if (m_exclusive != bExclusive ||
            m_buffer != uBufferMS ||
            (pDeviceId && m_deviceId != pDeviceId) ||
            (!pDeviceId && !m_deviceId.empty()))
        {
            try
            {
                m_deviceId = pDeviceId ? pDeviceId : L"";
                m_exclusive = bExclusive;
                m_buffer = uBufferMS;
                m_serial++;
            }
            catch (std::bad_alloc&)
            {
                return E_OUTOFMEMORY;
            }
        }

        return S_OK;
    }

    STDMETHODIMP Settings::GetOuputDevice(LPWSTR* ppDeviceId, BOOL* pbExclusive, UINT32* puBufferMS)
    {
        CAutoLock lock(this);

        if (pbExclusive)
            *pbExclusive = m_exclusive;

        if (ppDeviceId)
        {
            size_t size = sizeof(wchar_t) * (m_deviceId.length() + 1);

            *ppDeviceId = static_cast<LPWSTR>(CoTaskMemAlloc(size));

            if (!*ppDeviceId)
                return E_OUTOFMEMORY;

            memcpy(*ppDeviceId, m_deviceId.c_str(), size);
        }

        if (puBufferMS)
            *puBufferMS = m_buffer;

        return S_OK;
    }

    STDMETHODIMP_(void) Settings::SetAllowBitstreaming(BOOL bAllowBitstreaming)
    {
        CAutoLock lock(this);

        if (m_allowBitstreaming != bAllowBitstreaming)
        {
            m_allowBitstreaming = bAllowBitstreaming;
            m_serial++;
        }
    }

    STDMETHODIMP_(BOOL) Settings::GetAllowBitstreaming()
    {
        CAutoLock lock(this);

        return m_allowBitstreaming;
    }

    STDMETHODIMP_(void) Settings::SetCrossfeedEnabled(BOOL bEnable)
    {
        CAutoLock lock(this);

        if (m_crossfeedEnabled != bEnable)
        {
            m_crossfeedEnabled = bEnable;
            m_serial++;
        }
    }

    STDMETHODIMP_(BOOL) Settings::GetCrossfeedEnabled()
    {
        CAutoLock lock(this);

        return m_crossfeedEnabled;
    }

    STDMETHODIMP Settings::SetCrossfeedSettings(UINT32 uCutoffFrequency, UINT32 uCrossfeedLevel)
    {
        if (uCutoffFrequency < CROSSFEED_CUTOFF_FREQ_MIN ||
            uCutoffFrequency > CROSSFEED_CUTOFF_FREQ_MAX ||
            uCrossfeedLevel < CROSSFEED_LEVEL_MIN ||
            uCrossfeedLevel > CROSSFEED_LEVEL_MAX)
        {
            return E_INVALIDARG;
        }

        CAutoLock lock(this);

        if (m_crossfeedCutoffFrequency != uCutoffFrequency ||
            m_crossfeedLevel != uCrossfeedLevel)
        {
            m_crossfeedCutoffFrequency = uCutoffFrequency;
            m_crossfeedLevel = uCrossfeedLevel;
            m_serial++;
        }

        return S_OK;
    }

    STDMETHODIMP_(void) Settings::GetCrossfeedSettings(UINT32* puCutoffFrequency, UINT32* puCrossfeedLevel)
    {
        CAutoLock lock(this);

        if (puCutoffFrequency)
            *puCutoffFrequency = m_crossfeedCutoffFrequency;

        if (puCrossfeedLevel)
            *puCrossfeedLevel = m_crossfeedLevel;
    }

    STDMETHODIMP_(void) Settings::SetIgnoreSystemChannelMixer(BOOL bEnable)
    {
        CAutoLock lock(this);

        if (m_ignoreSystemChannelMixer != bEnable)
        {
            m_ignoreSystemChannelMixer = bEnable;
            m_serial++;
        }
    }

    STDMETHODIMP_(BOOL) Settings::GetIgnoreSystemChannelMixer()
    {
        CAutoLock lock(this);

        return m_ignoreSystemChannelMixer;
    }

    STDMETHODIMP Settings::SetTimestretchSettings(UINT32 uTimestretchMethod)
    {
        if (uTimestretchMethod != TIMESTRETCH_METHOD_SOLA &&
            uTimestretchMethod != TIMESTRETCH_METHOD_PHASE_VOCODER)
        {
            return E_INVALIDARG;
        }

    #ifndef SANEAR_GPL_PHASE_VOCODER
        if (uTimestretchMethod == TIMESTRETCH_METHOD_PHASE_VOCODER)
            return E_NOTIMPL;
    #endif

        CAutoLock lock(this);

        if (uTimestretchMethod != m_timestretchMethod)
        {
            m_timestretchMethod = uTimestretchMethod;
            m_serial++;
        }

        return S_OK;
    }

    STDMETHODIMP_(void) Settings::GetTimestretchSettings(UINT32* puTimestretchMethod)
    {
        CAutoLock lock(this);

        if (puTimestretchMethod)
            *puTimestretchMethod = m_timestretchMethod;
    }
}
