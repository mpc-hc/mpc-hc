#include "pch.h"
#include "MyClock.h"

#include "AudioRenderer.h"

namespace SaneAudioRenderer
{
    MyClock::MyClock(IUnknown* pUnknown, const std::unique_ptr<AudioRenderer>& renderer, HRESULT& result)
        : CBaseReferenceClock(L"SaneAudioRenderer::MyClock", pUnknown, &result)
        , m_renderer(renderer)
        , m_performanceFrequency(GetPerformanceFrequency())
    {
    }

    STDMETHODIMP MyClock::NonDelegatingQueryInterface(REFIID riid, void** ppv)
    {
        if (riid == __uuidof(IGuidedReclock))
            return GetInterface(static_cast<IGuidedReclock*>(this), ppv);

        return CBaseReferenceClock::NonDelegatingQueryInterface(riid, ppv);
    }

    REFERENCE_TIME MyClock::GetPrivateTime()
    {
        CAutoLock lock(this);

    #ifndef NDEBUG
        const int64_t oldCounterOffset = m_counterOffset;
    #endif

        if (m_guidedReclockSlaving && !CanDoGuidedReclock())
            UnslaveClock();

        REFERENCE_TIME audioClockTime, counterTime, clockTime;

        if (m_guidedReclockSlaving)
        {
            auto getGuidedReclockTime = [this](int64_t counterTime)
            {
                int64_t progress = (int64_t)((counterTime - m_guidedReclockStartTime) * m_guidedReclockMultiplier);
                return m_guidedReclockStartClock + progress;
            };

            if (SUCCEEDED(GetAudioClockTime(&audioClockTime, &counterTime)))
            {
                clockTime = getGuidedReclockTime(counterTime);
                int64_t diff = clockTime - audioClockTime;
                m_audioOffset += diff;
                m_renderer->TakeGuidedReclock(diff);
            }
            else
            {
                counterTime = GetCounterTime();
                clockTime = getGuidedReclockTime(counterTime);
            }

            m_counterOffset = clockTime - counterTime;
        }
        else if (SUCCEEDED(GetAudioClockTime(&audioClockTime, &counterTime)))
        {
            clockTime = audioClockTime;
            m_counterOffset = audioClockTime - counterTime;
        }
        else
        {
            clockTime = m_counterOffset + GetCounterTime();
        }

    #ifndef NDEBUG
        int64_t counterOffsetDiff = m_counterOffset - oldCounterOffset;
        if (std::abs(counterOffsetDiff) > OneMillisecond / 2)
            DebugOut(ClassName(this), "observed clock warp of", counterOffsetDiff / 10000., "ms");
    #endif

        return clockTime;
    }

    void MyClock::SlaveClockToAudio(IAudioClock* pAudioClock, int64_t audioStart)
    {
        assert(pAudioClock);

        CAutoLock lock(this);

        DebugOut(ClassName(this), "slave clock to audio device (delayed until it progresses)");

        m_audioClock = pAudioClock;
        m_audioStart = audioStart;
        m_audioInitialPosition = 0;
        m_audioClock->GetPosition(&m_audioInitialPosition, nullptr);
        m_audioOffset = 0;
    }

    void MyClock::UnslaveClockFromAudio()
    {
        CAutoLock lock(this);

        DebugOut(ClassName(this), "unslave clock from audio device");

        m_audioClock = nullptr;
    }

    void MyClock::OffsetAudioClock(REFERENCE_TIME offsetTime)
    {
        CAutoLock lock(this);

        m_audioOffset += offsetTime;
    }

    HRESULT MyClock::GetAudioClockTime(REFERENCE_TIME* pAudioTime, REFERENCE_TIME* pCounterTime)
    {
        CheckPointer(pAudioTime, E_POINTER);

        CAutoLock lock(this);

        if (m_audioClock)
        {
            uint64_t audioFrequency, audioPosition, audioTime;
            if (SUCCEEDED(m_audioClock->GetFrequency(&audioFrequency)) &&
                SUCCEEDED(m_audioClock->GetPosition(&audioPosition, &audioTime)) &&
                audioPosition > m_audioInitialPosition)
            {
                int64_t counterTime = GetCounterTime();
                int64_t clockTime = llMulDiv(audioPosition, OneSecond, audioFrequency, 0) +
                                    m_audioStart + m_audioOffset + counterTime - audioTime;

                *pAudioTime = clockTime;

                if (pCounterTime)
                    *pCounterTime = counterTime;

                return S_OK;
            }

            return E_FAIL;
        }

        return E_FAIL;
    }

    HRESULT MyClock::GetAudioClockStartTime(REFERENCE_TIME* pStartTime)
    {
        CheckPointer(pStartTime, E_POINTER);

        CAutoLock lock(this);

        if (m_audioClock)
        {
            *pStartTime = m_audioStart;
            return S_OK;
        }

        return E_FAIL;
    }

    STDMETHODIMP MyClock::SlaveClock(DOUBLE multiplier)
    {
        CAutoLock lock(this);

        if (!CanDoGuidedReclock())
            return E_FAIL;

        int64_t time;
        ReturnIfFailed(GetTime(&time));

        m_guidedReclockSlaving = true;
        m_guidedReclockMultiplier = multiplier;
        m_guidedReclockStartTime = GetCounterTime();
        m_guidedReclockStartClock = time;

        return S_OK;
    }

    STDMETHODIMP MyClock::UnslaveClock()
    {
        CAutoLock lock(this);

        if (!m_guidedReclockSlaving)
            return S_FALSE;

        GetPrivateTime();
        m_guidedReclockSlaving = false;

        return S_OK;
    }

    STDMETHODIMP MyClock::OffsetClock(LONGLONG offset)
    {
        CAutoLock lock(this);

        if (!CanDoGuidedReclock())
            return E_FAIL;

        m_audioOffset += offset;
        m_counterOffset += offset;
        m_guidedReclockStartClock += offset;

        m_renderer->TakeGuidedReclock(offset);

        return S_OK;
    }

    STDMETHODIMP MyClock::GetImmediateTime(LONGLONG* pTime)
    {
        CheckPointer(pTime, E_POINTER);

        *pTime = GetPrivateTime();

        return S_OK;
    }

    bool MyClock::CanDoGuidedReclock()
    {
        return !m_renderer->IsBitstreaming() &&
               !m_renderer->OnExternalClock() &&
               !m_renderer->IsLive();
    }
}
