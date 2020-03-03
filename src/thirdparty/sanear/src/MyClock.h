#pragma once

#include "../IGuidedReclock.h"

namespace SaneAudioRenderer
{
    class AudioRenderer;

    class MyClock final
        : public CBaseReferenceClock
        , public IGuidedReclock
    {
    public:

        MyClock(IUnknown* pUnknown, const std::unique_ptr<AudioRenderer>& renderer, HRESULT& result);
        MyClock(const MyClock&) = delete;
        MyClock& operator=(const MyClock&) = delete;

        DECLARE_IUNKNOWN

        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;

        REFERENCE_TIME GetPrivateTime() override;

        void SlaveClockToAudio(IAudioClock* pAudioClock, int64_t audioStart);
        void UnslaveClockFromAudio();
        void OffsetAudioClock(REFERENCE_TIME offsetTime);
        HRESULT GetAudioClockTime(REFERENCE_TIME* pAudioTime, REFERENCE_TIME* pCounterTime);
        HRESULT GetAudioClockStartTime(REFERENCE_TIME* pStartTime);

        STDMETHODIMP SlaveClock(DOUBLE multiplier) override;
        STDMETHODIMP UnslaveClock() override;
        STDMETHODIMP OffsetClock(LONGLONG offset) override;
        STDMETHODIMP GetImmediateTime(LONGLONG* pTime) override;

    private:

        bool CanDoGuidedReclock();

        int64_t GetCounterTime() { return llMulDiv(GetPerformanceCounter(), OneSecond, m_performanceFrequency, 0); }

        const std::unique_ptr<AudioRenderer>& m_renderer;

        const int64_t m_performanceFrequency;

        IAudioClockPtr m_audioClock;
        int64_t m_audioStart = 0;
        uint64_t m_audioInitialPosition = 0;
        int64_t m_audioOffset = 0;
        int64_t m_counterOffset = 0;

        bool m_guidedReclockSlaving = false;
        double m_guidedReclockMultiplier = 1.0;
        int64_t m_guidedReclockStartTime = 0;
        int64_t m_guidedReclockStartClock = 0;
    };
}
