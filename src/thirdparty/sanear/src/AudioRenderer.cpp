#include "pch.h"
#include "AudioRenderer.h"

#include "MyClock.h"

namespace SaneAudioRenderer
{
    AudioRenderer::AudioRenderer(ISettings* pSettings, MyClock& clock, HRESULT& result)
        : m_deviceManager(result)
        , m_myClock(clock)
        , m_flush(TRUE/*manual reset*/)
        , m_dspVolume(*this)
        , m_dspBalance(*this)
        , m_settings(pSettings)
    {
        if (FAILED(result))
            return;

        try
        {
            if (!m_settings)
                throw E_UNEXPECTED;

            if (static_cast<HANDLE>(m_flush) == NULL)
            {
                throw E_OUTOFMEMORY;
            }
        }
        catch (HRESULT ex)
        {
            result = ex;
        }
    }

    AudioRenderer::~AudioRenderer()
    {
        // Just in case.
        if (m_state != State_Stopped)
            Stop();
    }

    void AudioRenderer::SetClock(IReferenceClock* pClock)
    {
        CAutoLock objectLock(this);

        m_graphClock = pClock;

        if (m_graphClock && !IsEqualObject(m_graphClock, m_myClock.GetOwner()))
        {
            if (!m_externalClock)
                ClearDevice();

            m_externalClock = true;
        }
        else
        {
            if (m_externalClock)
                ClearDevice();

            m_externalClock = false;
        }

        m_guidedReclockActive = false;
    }

    bool AudioRenderer::Push(IMediaSample* pSample, AM_SAMPLE2_PROPERTIES& sampleProps, CAMEvent* pFilledEvent)
    {
        DspChunk chunk;

        {
            CAutoLock objectLock(this);
            assert(m_inputFormat);
            assert(m_state != State_Stopped);

            try
            {
                // Clear the device if related settings were changed.
                CheckDeviceSettings();

                // Create the device if needed.
                if (!m_device)
                    CreateDevice();

                // Establish time/frame relation.
                chunk = m_sampleCorrection.ProcessSample(pSample, sampleProps, m_live || m_externalClock);

                // Drop frames if requested.
                if (m_dropNextFrames > 0 && !chunk.IsEmpty())
                {
                    assert(!IsBitstreaming());

                    const size_t chunkFrames = chunk.GetFrameCount();

                    if (m_dropNextFrames >= chunkFrames)
                    {
                        DebugOut(ClassName(this), "dropping", chunkFrames, "frames");
                        m_dropNextFrames -= chunkFrames;
                        chunk = DspChunk();
                        assert(chunk.IsEmpty());
                    }
                    else
                    {
                        DebugOut(ClassName(this), "dropping", m_dropNextFrames, "frames");
                        chunk.ShrinkHead(chunkFrames - m_dropNextFrames);
                        m_dropNextFrames = 0;
                    }
                }

                // Apply clock corrections.
                if (!m_live && m_device && m_state == State_Running)
                    ApplyClockCorrection();

                // Apply dsp chain.
                if (m_device && !IsBitstreaming())
                {
                    auto f = [&](DspBase* pDsp)
                    {
                        pDsp->Process(chunk);
                    };

                    EnumerateProcessors(f);

                    DspChunk::ToFormat(m_device->GetDspFormat(), chunk);
                }

                if (m_device && !IsBitstreaming() && m_state == State_Running)
                {
                    if (m_live || m_externalClock)
                    {
                        // Apply rate corrections (rate matching and clock slaving).
                        ApplyRateCorrection(chunk);
                    }
                    else if (REFERENCE_TIME offset = std::atomic_exchange(&m_guidedReclockOffset, 0))
                    {
                        // Apply guided reclock adjustment.
                        m_dspRate.Adjust(-offset);
                        m_guidedReclockActive = true;
                    }
                }
            }
            catch (HRESULT)
            {
                ClearDevice();
            }
            catch (std::bad_alloc&)
            {
                ClearDevice();
                chunk = DspChunk();
            }
        }

        // Send processed sample to the device.
        return PushToDevice(chunk, pFilledEvent);
    }

    bool AudioRenderer::Finish(bool blockUntilEnd, CAMEvent* pFilledEvent)
    {
        DspChunk chunk;

        {
            CAutoLock objectLock(this);
            assert(m_state != State_Stopped);

            // No device - nothing to block on.
            if (!m_device)
                blockUntilEnd = false;

            try
            {
                // Apply dsp chain.
                if (m_device && !IsBitstreaming())
                {
                    auto f = [&](DspBase* pDsp)
                    {
                        pDsp->Finish(chunk);
                    };

                    EnumerateProcessors(f);

                    DspChunk::ToFormat(m_device->GetDspFormat(), chunk);
                }
            }
            catch (std::bad_alloc&)
            {
                chunk = DspChunk();
                assert(chunk.IsEmpty());
            }
        }

        auto doBlock = [&]
        {
            // Increase system timer resolution.
            TimePeriodHelper timePeriodHelper(1);

            for (;;)
            {
                REFERENCE_TIME remaining = 0;

                {
                    CAutoLock objectLock(this);

                    if (m_device)
                    {
                        try
                        {
                            remaining = m_device->Finish(pFilledEvent);
                        }
                        catch (HRESULT)
                        {
                            ClearDevice();
                        }
                    }
                }

                // The end of stream is reached.
                if (remaining <= 0)
                    return true;

                // Sleep until predicted end of stream.
                if (m_flush.Wait(std::max(1, (int32_t)(remaining / OneMillisecond))))
                    return false;
            }
        };

        // Send processed sample to the device, and block until the end of stream (if requested).
        return PushToDevice(chunk, pFilledEvent) && (!blockUntilEnd || doBlock());
    }

    void AudioRenderer::BeginFlush()
    {
        m_flush.Set();
    }

    void AudioRenderer::EndFlush()
    {
        CAutoLock objectLock(this);

        if (m_device)
        {
            if (m_state == State_Running)
            {
                m_myClock.UnslaveClockFromAudio();
                m_device->Stop();
                m_device->Reset();
                m_dropNextFrames = 0;
                m_sampleCorrection.NewDeviceBuffer();
                InitializeProcessors();
                m_startClockOffset = m_sampleCorrection.GetLastFrameEnd();
                PushReslavingJitter();
                StartDevice();
            }
            else
            {
                m_device->Reset();
                m_dropNextFrames = 0;
                m_sampleCorrection.NewDeviceBuffer();
                InitializeProcessors();
            }
        }

        m_flush.Reset();
    }

    bool AudioRenderer::CheckFormat(SharedWaveFormat inputFormat, bool live)
    {
        assert(inputFormat);

        if (inputFormat->nChannels == 0 ||
            inputFormat->nSamplesPerSec == 0 ||
            inputFormat->wBitsPerSample == 0 ||
            inputFormat->nBlockAlign * 8 != inputFormat->nChannels * inputFormat->wBitsPerSample ||
            inputFormat->nChannels > 18)
        {
            return false;
        }

        if (DspFormatFromWaveFormat(*inputFormat) != DspFormat::Unknown)
            return true;

        BOOL exclusive;
        m_settings->GetOuputDevice(nullptr, &exclusive, nullptr);
        BOOL bitstreamingAllowed = m_settings->GetAllowBitstreaming();

        if (!exclusive || !bitstreamingAllowed || live)
            return false;

        CAutoLock objectLock(this);

        return m_deviceManager.BitstreamFormatSupported(inputFormat, m_settings) && !m_externalClock;
    }

    void AudioRenderer::SetFormat(SharedWaveFormat inputFormat, bool live)
    {
        CAutoLock objectLock(this);

        m_inputFormat = inputFormat;
        m_live = live;

        m_sampleCorrection.NewFormat(inputFormat);

        ClearDevice();

        m_bitstreaming = (DspFormatFromWaveFormat(*inputFormat) == DspFormat::Unknown);
    }

    void AudioRenderer::NewSegment(double rate)
    {
        CAutoLock objectLock(this);

        if (m_rate != rate)
        {
            m_rate = rate;

            if (m_device)
                (m_device->GetEnd() > 0) ? ClearDevice() : InitializeProcessors();
        }

        m_startClockOffset = 0;

        m_clockCorrection += m_sampleCorrection.GetLastFrameEnd();

        m_sampleCorrection.NewSegment(m_rate);
    }

    void AudioRenderer::Play(REFERENCE_TIME startTime)
    {
        CAutoLock objectLock(this);

        CheckDeviceSettings();

        assert(m_state != State_Running);
        m_state = State_Running;

        m_startTime = startTime;
        StartDevice();
    }

    void AudioRenderer::Pause()
    {
        CAutoLock objectLock(this);

        if (m_device)
        {
            m_myClock.UnslaveClockFromAudio();
            m_device->Stop();
        }

        assert(m_state != State_Paused);
        m_state = State_Paused;
    }

    void AudioRenderer::Stop()
    {
        CAutoLock objectLock(this);

        ClearDevice();

        assert(m_state != State_Stopped);
        m_state = State_Stopped;
    }

    SharedWaveFormat AudioRenderer::GetInputFormat()
    {
        CAutoLock objectLock(this);

        return m_inputFormat;
    }

    const AudioDevice* AudioRenderer::GetAudioDevice()
    {
        assert(CritCheckIn(this));

        return m_device.get();
    }

    std::vector<std::wstring> AudioRenderer::GetActiveProcessors()
    {
        CAutoLock objectLock(this);

        std::vector<std::wstring> ret;

        if (m_inputFormat && m_device && !IsBitstreaming())
        {
            auto f = [&](DspBase* pDsp)
            {
                if (pDsp->Active())
                    ret.emplace_back(pDsp->Name());
            };

            EnumerateProcessors(f);
        }

        return ret;
    }

    bool AudioRenderer::OnGuidedReclock()
    {
        CAutoLock objectLock(this);

        return m_guidedReclockActive;
    }

    void AudioRenderer::CheckDeviceSettings()
    {
        CAutoLock objectLock(this);

        UINT32 newSettingsSerial = m_settings->GetSerial();
        uint32_t newDefaultDeviceSerial = m_deviceManager.GetDefaultDeviceSerial();

        if (m_device && (m_deviceSettingsSerial != newSettingsSerial ||
                         m_defaultDeviceSerial != newDefaultDeviceSerial))
        {
            bool settingsDeviceDefault;
            std::unique_ptr<WCHAR, CoTaskMemFreeDeleter> settingsDeviceId;
            BOOL settingsDeviceExclusive;
            UINT32 settingsDeviceBuffer;

            {
                LPWSTR pDeviceId = nullptr;

                if (FAILED(m_settings->GetOuputDevice(&pDeviceId, &settingsDeviceExclusive, &settingsDeviceBuffer)))
                    return;

                settingsDeviceDefault = (!pDeviceId || !*pDeviceId);
                settingsDeviceId.reset(pDeviceId);
            }

            bool clearForSystemChannelMixer = false;
            bool clearForCrossfeed = false;

            if (!m_device->IsExclusive())
            {
                BOOL crossfeedEnabled = m_settings->GetCrossfeedEnabled();
                BOOL ignoreSystemChannelMixer = m_settings->GetIgnoreSystemChannelMixer();

                SharedWaveFormat mixFormat = m_device->GetMixFormat();
                assert(mixFormat);

                bool mixFormatIsStereo = DspMatrix::IsStereoFormat(*mixFormat);
                bool inputIsStereo = DspMatrix::IsStereoFormat(*m_inputFormat);
                bool outputIsStereo = DspMatrix::IsStereoFormat(*m_device->GetWaveFormat());

                if (m_device->IgnoredSystemChannelMixer() && !ignoreSystemChannelMixer && !crossfeedEnabled)
                {
                    clearForSystemChannelMixer = true;
                }
                else if (mixFormatIsStereo && crossfeedEnabled && !outputIsStereo)
                {
                    clearForCrossfeed = true;
                }
            }

            bool clearForTimestretch = false;
            {
            #ifdef SANEAR_GPL_PHASE_VOCODER
                UINT32 timestretchMethod;
                m_settings->GetTimestretchSettings(&timestretchMethod);
                const bool usePhaseVocoder = (timestretchMethod == ISettings::TIMESTRETCH_METHOD_PHASE_VOCODER);

                if ((usePhaseVocoder && m_dspTempo1.Active()) ||
                    (!usePhaseVocoder && m_dspTempo2.Active()))
                {
                    clearForTimestretch = true;
                }
            #endif
            }

            m_deviceSettingsSerial = newSettingsSerial;

            std::unique_ptr<WCHAR, CoTaskMemFreeDeleter> systemDeviceId;;

            if (settingsDeviceDefault)
            {
                systemDeviceId = m_deviceManager.GetDefaultDeviceId();

                if (!systemDeviceId)
                    return;
            }

            m_defaultDeviceSerial = newDefaultDeviceSerial;

            if ((clearForSystemChannelMixer) ||
                (clearForCrossfeed) ||
                (clearForTimestretch) ||
                (m_device->IsExclusive() != !!settingsDeviceExclusive) ||
                (m_device->GetBufferDuration() != settingsDeviceBuffer) ||
                (!settingsDeviceDefault && *m_device->GetId() != settingsDeviceId.get()) ||
                (settingsDeviceDefault && *m_device->GetId() != systemDeviceId.get()))
            {
                ClearDevice();
                assert(!m_device);
            }
        }
    }

    void AudioRenderer::StartDevice()
    {
        CAutoLock objectLock(this);
        assert(m_state == State_Running);

        if (m_device)
        {
            try
            {
                // Renew (effectively recreate) the device if it's been freed because of inactivity.
                int64_t deviceRenewPosition = 0;
                if (!m_deviceManager.RenewInactiveDevice(*m_device, deviceRenewPosition))
                {
                    DebugOut(ClassName(this), "failed to renew device");
                    ClearDevice();
                    return;
                }

                // Try to minimize clock slaving initial jitter.
                {
                    REFERENCE_TIME jitter = EstimateSlavingJitter();

                    if (jitter > OneMillisecond &&
                        jitter < 200 * OneMillisecond)
                    {
                        int32_t sleepTime = (int32_t)(jitter / OneMillisecond);
                        jitter -= sleepTime * OneMillisecond;

                        TimePeriodHelper timePeriodHelper(1);
                        DebugOut(ClassName(this), "sleep for", sleepTime, "ms to minimize slaving jitter");
                        Sleep(sleepTime);

                        jitter = EstimateSlavingJitter();
                    }
                    else if (m_sampleCorrection.GetLastFrameEnd() == 0)
                    {
                        DebugOut(ClassName(this), "empty start");

                        if (!m_bitstreaming && !m_device->IsRealtime())
                        {
                            size_t silenceFrames = m_device->GetRate() * m_device->GetBufferDuration() / 5000; // Buffer / 5

                            DspChunk chunk = DspChunk(m_device->GetDspFormat(), m_device->GetChannelCount(),
                                                      silenceFrames, m_device->GetRate());
                            ZeroMemory(chunk.GetData(), chunk.GetSize());

                            DebugOut(ClassName(this), "pushing", silenceFrames, "frames of silence");
                            m_device->Push(chunk, nullptr);

                            m_startClockOffset -= FramesToTime(silenceFrames, m_device->GetRate());

                            jitter = EstimateSlavingJitter();
                        }

                        if (!m_bitstreaming && jitter < 0)
                        {
                            m_dropNextFrames = TimeToFrames(-jitter, m_device->GetRate());

                            if (m_dropNextFrames > 0)
                            {
                                DebugOut(ClassName(this), "will be dropping next", m_dropNextFrames, "frames");
                                m_startClockOffset += FramesToTime(m_dropNextFrames, m_device->GetRate());
                                jitter = EstimateSlavingJitter();
                            }
                        }
                    }

                    DebugOut(ClassName(this), "predicting approx", jitter / 10000., "ms slaving jitter");
                }

                m_guidedReclockOffset = 0;
                m_myClock.SlaveClockToAudio(m_device->GetClock(), m_startTime + m_startClockOffset + deviceRenewPosition);
                m_clockCorrection = 0;
                m_device->Start();
            }
            catch (HRESULT)
            {
                ClearDevice();
            }
        }
    }

    void AudioRenderer::CreateDevice()
    {
        CAutoLock objectLock(this);

        assert(!m_device);
        assert(m_inputFormat);

        m_deviceSettingsSerial = m_settings->GetSerial();
        m_defaultDeviceSerial = m_deviceManager.GetDefaultDeviceSerial();
        m_device = m_deviceManager.CreateDevice(m_inputFormat, m_live || m_externalClock, m_settings);

        if (m_device)
        {
            m_sampleCorrection.NewDeviceBuffer();

            InitializeProcessors();

            m_startClockOffset = m_sampleCorrection.GetLastFrameEnd();

            if (m_state == State_Running)
            {
                try
                {
                    PushReslavingJitter();
                }
                catch (HRESULT)
                {
                    ClearDevice();
                }

                StartDevice();
            }
        }
    }

    void AudioRenderer::ClearDevice()
    {
        CAutoLock objectLock(this);

        if (m_device)
        {
            if (m_state == State_Running)
            {
                m_myClock.UnslaveClockFromAudio();
                m_device->Stop();
            }
            m_device = nullptr;
        }

        m_dropNextFrames = 0;
    }

    REFERENCE_TIME AudioRenderer::EstimateSlavingJitter()
    {
        CAutoLock objectLock(this);
        assert(m_device);

        REFERENCE_TIME startTimeOffset;
        if (m_externalClock) {
	        startTimeOffset = 0;
        } else {
	        startTimeOffset = m_myClock.GetPrivateTime() - m_startTime;
        }

        REFERENCE_TIME jitter = m_startClockOffset - (startTimeOffset) + m_device->GetPosition();

        if (m_device->IsExclusive())
        {
            jitter -= 3 * OneMillisecond; // Taking USB cards into account, could have been lower otherwise.
        }
        else
        {
            jitter -= m_device->GetStreamLatency() - OneMillisecond; // Experimental guesswork.
        }

        return jitter;
    }

    void AudioRenderer::PushReslavingJitter()
    {
        CAutoLock objectLock(this);

        assert(m_device);
        assert(m_state == State_Running);
        assert(m_device->GetEnd() == 0);

        if (IsBitstreaming())
            return;

        // Try to keep inevitable clock jerking to a minimum after re-slaving.

        REFERENCE_TIME jitter = EstimateSlavingJitter();

        if (jitter > 0)
        {
            jitter = std::min(jitter, llMulDiv(m_device->GetBufferDuration(), OneSecond, 1000, 0));

            DspChunk chunk(m_device->GetDspFormat(), m_device->GetChannelCount(),
                           TimeToFrames(jitter, m_device->GetRate()), m_device->GetRate());

            if (!chunk.IsEmpty())
            {
                m_startClockOffset -= FramesToTime(chunk.GetFrameCount(), m_device->GetRate());

                DebugOut(ClassName(this), "push", chunk.GetFrameCount() * 1000. / m_device->GetRate(),
                         "ms of silence to minimize re-slaving jitter");

                ZeroMemory(chunk.GetData(), chunk.GetSize());
                PushToDevice(chunk, nullptr);
            }
        }
    }

    void AudioRenderer::ApplyClockCorrection()
    {
        CAutoLock objectLock(this);
        assert(m_device);
        assert(m_state == State_Running);

        // Apply corrections to internal clock.
        {
            REFERENCE_TIME offset = m_sampleCorrection.GetTimeDivergence() - m_clockCorrection;
            if (std::abs(offset) > 100)
            {
                m_myClock.OffsetAudioClock(offset);
                m_clockCorrection += offset;
                DebugOut(ClassName(this), "offset internal clock by", offset / 10000.,
                         "ms to match", ClassName(&m_sampleCorrection));
            }
        }
    }

    void AudioRenderer::ApplyRateCorrection(DspChunk& chunk)
    {
        CAutoLock objectLock(this);
        assert(m_device);
        assert(!IsBitstreaming());
        assert(m_live || m_externalClock);
        assert(m_state == State_Running);

        if (chunk.IsEmpty())
            return;

        const REFERENCE_TIME latency = m_device->GetStreamLatency() * 2; // x2.0

        const REFERENCE_TIME remaining = m_device->GetEnd() - m_device->GetPosition();

        REFERENCE_TIME deltaTime = 0;

        if (m_live)
        {
            // Rate matching.
            if (remaining > latency) // x2.0
            {
                size_t dropFrames = TimeToFrames(remaining - latency * 3 / 4, m_device->GetRate()); // x1.5

                dropFrames = std::min(dropFrames, chunk.GetFrameCount());

                chunk.ShrinkHead(chunk.GetFrameCount() - dropFrames);

                DebugOut(ClassName(this), "drop", dropFrames, "frames for rate matching");
            }
            else if (remaining < latency / 2) // x1.0
            {
                size_t padFrames = TimeToFrames(latency * 3 / 4 - remaining, m_device->GetRate()); // x1.5

                chunk.PadHead(padFrames);

                DebugOut(ClassName(this), "pad", padFrames, "frames for rate matching");
            }
        }
        else
        {
            // Clock matching.
            assert(m_externalClock);

            REFERENCE_TIME graphTime, myTime, myStartTime;
            if (SUCCEEDED(m_myClock.GetAudioClockStartTime(&myStartTime)) &&
                SUCCEEDED(m_myClock.GetAudioClockTime(&myTime, nullptr)) &&
                SUCCEEDED(m_graphClock->GetTime(&graphTime)) &&
                myTime > myStartTime)
            {
                myTime -= m_device->GetSilence();

                if (myTime > graphTime)
                {
                    // Pad and adjust backwards.
                    REFERENCE_TIME padTime = myTime - graphTime;
                    assert(padTime >= 0);

                    size_t padFrames = TimeToFrames(padTime, m_device->GetRate());

                    if (padFrames > m_device->GetRate() / 33) // ~30ms threshold
                    {
                        chunk.PadHead(padFrames);

                        REFERENCE_TIME paddedTime = FramesToTime(padFrames, m_device->GetRate());

                        m_myClock.OffsetAudioClock(-paddedTime);
                        padTime -= paddedTime;
                        assert(padTime >= 0);

                        DebugOut(ClassName(this), "pad", paddedTime / 10000., "ms for clock matching at",
                                 m_sampleCorrection.GetLastFrameEnd() / 10000., "frame position");
                    }

                    // Correct the rest with variable rate.
                    m_dspRate.Adjust(padTime);
                    m_myClock.OffsetAudioClock(-padTime);
                }
                else if (remaining > latency)
                {
                    // Crop and adjust forwards.
                    assert(myTime <= graphTime);
                    REFERENCE_TIME dropTime = std::min(graphTime - myTime, remaining - latency);
                    assert(dropTime >= 0);

                    size_t dropFrames = TimeToFrames(dropTime, m_device->GetRate());

                    dropFrames = std::min(dropFrames, chunk.GetFrameCount());

                    if (dropFrames > m_device->GetRate() / 33) // ~30ms threshold
                    {
                        chunk.ShrinkHead(chunk.GetFrameCount() - dropFrames);

                        REFERENCE_TIME droppedTime = FramesToTime(dropFrames, m_device->GetRate());

                        m_myClock.OffsetAudioClock(droppedTime);
                        dropTime -= droppedTime;
                        assert(dropTime >= 0);

                        DebugOut(ClassName(this), "drop", droppedTime / 10000., "ms for clock matching at",
                                 m_sampleCorrection.GetLastFrameEnd() / 10000., "frame position");
                    }

                    // Correct the rest with variable rate.
                    m_dspRate.Adjust(-dropTime);
                    m_myClock.OffsetAudioClock(dropTime);
                }
            }
        }
    }

    void AudioRenderer::InitializeProcessors()
    {
        CAutoLock objectLock(this);
        assert(m_inputFormat);
        assert(m_device);

        if (IsBitstreaming())
            return;

        const auto inRate = m_inputFormat->nSamplesPerSec;
        const auto inChannels = m_inputFormat->nChannels;
        const auto inMask = DspMatrix::GetChannelMask(*m_inputFormat);
        const auto outRate = m_device->GetRate();
        const auto outChannels = m_device->GetChannelCount();
        const auto outMask = DspMatrix::GetChannelMask(*m_device->GetWaveFormat());

    #ifdef SANEAR_GPL_PHASE_VOCODER
        UINT32 timestretchMethod;
        m_settings->GetTimestretchSettings(&timestretchMethod);
        const bool usePhaseVocoder = (timestretchMethod == ISettings::TIMESTRETCH_METHOD_PHASE_VOCODER);
    #endif

        m_dspMatrix.Initialize(inChannels, inMask, outChannels, outMask);
        m_dspRate.Initialize(m_live || m_externalClock, inRate, outRate, outChannels);
    #ifdef SANEAR_GPL_PHASE_VOCODER
        m_dspTempo1.Initialize(usePhaseVocoder ? 1.0 : m_rate, outRate, outChannels);
        m_dspTempo2.Initialize(usePhaseVocoder ? m_rate : 1.0, outRate, outChannels);
    #else
        m_dspTempo.Initialize(m_rate, outRate, outChannels);
    #endif
        m_dspCrossfeed.Initialize(m_settings, outRate, outChannels, outMask);
        m_dspLimiter.Initialize(outRate, outChannels, m_device->IsExclusive());
        m_dspDither.Initialize(m_device->GetDspFormat());
    }

    bool AudioRenderer::PushToDevice(DspChunk& chunk, CAMEvent* pFilledEvent)
    {
        bool firstIteration = true;
        uint32_t sleepDuration = 0;
        while (!chunk.IsEmpty())
        {
            // The device buffer is full or almost full at the beginning of the second and subsequent iterations.
            // Sleep until the buffer may have significant amount of free space. Unless interrupted.
            if (!firstIteration && m_flush.Wait(sleepDuration))
                return false;

            firstIteration = false;

            CAutoLock objectLock(this);

            assert(m_state != State_Stopped);

            if (m_device)
            {
                try
                {
                    m_device->Push(chunk, pFilledEvent);
                    sleepDuration = m_device->GetBufferDuration() / 4;
                }
                catch (HRESULT)
                {
                    ClearDevice();
                    sleepDuration = 0;
                }
            }
            else
            {
                // The code below emulates null audio device.

                if (pFilledEvent)
                    pFilledEvent->Set();

                sleepDuration = 1;

                // Loop until the graph time passes the current sample end minus 50ms.
                REFERENCE_TIME graphTime;
                if (m_state == State_Running &&
                    SUCCEEDED(m_graphClock->GetTime(&graphTime)) &&
                    graphTime + 50 * OneMillisecond > m_startTime + m_sampleCorrection.GetLastFrameEnd() +
                                                      m_sampleCorrection.GetTimeDivergence())
                {
                    break;
                }
            }
        }

        return true;
    }
}
