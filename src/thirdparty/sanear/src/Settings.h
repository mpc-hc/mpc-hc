#pragma once

#include "Interfaces.h"

namespace SaneAudioRenderer
{
    class Settings final
        : public CUnknown
        , public ISettings
        , private CCritSec
    {
    public:

        DECLARE_IUNKNOWN

        Settings(IUnknown* pUnknown);
        Settings(const Settings&) = delete;
        Settings& operator=(const Settings&) = delete;

        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;

        STDMETHODIMP_(UINT32) GetSerial() override;

        STDMETHODIMP SetOuputDevice(LPCWSTR pDeviceId, BOOL bExclusive, UINT32 uBufferMS) override;
        STDMETHODIMP GetOuputDevice(LPWSTR* ppDeviceId, BOOL* pbExclusive, UINT32* puBufferMS) override;

        STDMETHODIMP_(void) SetAllowBitstreaming(BOOL bAllowBitstreaming) override;
        STDMETHODIMP_(BOOL) GetAllowBitstreaming() override;

        STDMETHODIMP_(void) SetCrossfeedEnabled(BOOL bEnable) override;
        STDMETHODIMP_(BOOL) GetCrossfeedEnabled() override;

        STDMETHODIMP SetCrossfeedSettings(UINT32 uCutoffFrequency, UINT32 uCrossfeedLevel) override;
        STDMETHODIMP_(void) GetCrossfeedSettings(UINT32* puCutoffFrequency, UINT32* puCrossfeedLevel) override;

        STDMETHODIMP_(void) SetIgnoreSystemChannelMixer(BOOL bEnable) override;
        STDMETHODIMP_(BOOL) GetIgnoreSystemChannelMixer() override;

        STDMETHODIMP SetTimestretchSettings(UINT32 uTimestretchMethod) override;
        STDMETHODIMP_(void) GetTimestretchSettings(UINT32* puTimestretchMethod) override;

    private:

        std::atomic<UINT32> m_serial = 0;

        std::wstring m_deviceId;
        BOOL m_exclusive = FALSE;
        UINT32 m_buffer = OUTPUT_DEVICE_BUFFER_DEFAULT_MS;

        BOOL m_allowBitstreaming = TRUE;

        BOOL m_sharedModePeakLimiterEnabled = FALSE;

        BOOL m_crossfeedEnabled = FALSE;
        UINT32 m_crossfeedCutoffFrequency = CROSSFEED_CUTOFF_FREQ_CMOY;
        UINT32 m_crossfeedLevel = CROSSFEED_LEVEL_CMOY;

        BOOL m_ignoreSystemChannelMixer = TRUE;

        UINT32 m_timestretchMethod =
    #ifdef SANEAR_GPL_PHASE_VOCODER
                   TIMESTRETCH_METHOD_PHASE_VOCODER;
    #else
                   TIMESTRETCH_METHOD_SOLA;
    #endif
    };
}
