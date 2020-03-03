#pragma once

#include "AudioDevice.h"
#include "AudioDeviceManager.h"
#include "DspBalance.h"
#include "DspCrossfeed.h"
#include "DspDither.h"
#include "DspLimiter.h"
#include "DspMatrix.h"
#include "DspRate.h"
#include "DspTempo.h"
#include "DspTempo2.h"
#include "DspVolume.h"
#include "Interfaces.h"
#include "SampleCorrection.h"

namespace SaneAudioRenderer
{
    class MyClock;

    class AudioRenderer final
        : public CCritSec
    {
    public:

        AudioRenderer(ISettings* pSettings, MyClock& clock, HRESULT& result);
        AudioRenderer(const AudioRenderer&) = delete;
        AudioRenderer& operator=(const AudioRenderer&) = delete;
        ~AudioRenderer();

        void SetClock(IReferenceClock* pClock);

        bool Push(IMediaSample* pSample, AM_SAMPLE2_PROPERTIES& sampleProps, CAMEvent* pFilledEvent);
        bool Finish(bool blockUntilEnd, CAMEvent* pFilledEvent);

        void BeginFlush();
        void EndFlush();

        bool CheckFormat(SharedWaveFormat inputFormat, bool live);
        void SetFormat(SharedWaveFormat inputFormat, bool live);

        void NewSegment(double rate);

        void Play(REFERENCE_TIME startTime);
        void Pause();
        void Stop();

        float GetVolume() const { return m_volume; }
        void SetVolume(float volume) { m_volume = volume; }
        float GetBalance() const { return m_balance; }
        void SetBalance(float balance) { m_balance = balance; }

        SharedWaveFormat GetInputFormat();
        const AudioDevice* GetAudioDevice();
        std::vector<std::wstring> GetActiveProcessors();

        void TakeGuidedReclock(REFERENCE_TIME offset) { m_guidedReclockOffset += offset; }

        bool OnExternalClock() const { return m_externalClock; }
        bool IsLive()          const { return m_live; }
        bool IsBitstreaming()  const { return m_bitstreaming; }

        bool OnGuidedReclock();

    private:

        void CheckDeviceSettings();
        void StartDevice();
        void CreateDevice();
        void ClearDevice();

        REFERENCE_TIME EstimateSlavingJitter();

        void PushReslavingJitter();

        void ApplyClockCorrection();

        void ApplyRateCorrection(DspChunk& chunk);

        void InitializeProcessors();

        template <typename F>
        void EnumerateProcessors(F f)
        {
            f(&m_dspMatrix);
            f(&m_dspRate);
        #ifdef SANEAR_GPL_PHASE_VOCODER
            f(&m_dspTempo1);
            f(&m_dspTempo2);
        #else
            f(&m_dspTempo);
        #endif
            f(&m_dspCrossfeed);
            f(&m_dspVolume);
            f(&m_dspBalance);
            f(&m_dspLimiter);
            f(&m_dspDither);
        }

        bool PushToDevice(DspChunk& chunk, CAMEvent* pFilledEvent);

        AudioDeviceManager m_deviceManager;
        std::unique_ptr<AudioDevice> m_device;

        FILTER_STATE m_state = State_Stopped;

        SampleCorrection m_sampleCorrection;
        REFERENCE_TIME m_clockCorrection = 0;

        MyClock& m_myClock;
        IReferenceClockPtr m_graphClock;

        std::atomic<bool> m_externalClock = false;
        std::atomic<bool> m_live = false;
        std::atomic<bool> m_bitstreaming = false;

        SharedWaveFormat m_inputFormat;

        REFERENCE_TIME m_startClockOffset = 0;
        REFERENCE_TIME m_startTime = 0;

        CAMEvent m_flush;

        DspMatrix m_dspMatrix;
        DspRate m_dspRate;
    #ifdef SANEAR_GPL_PHASE_VOCODER
        DspTempo m_dspTempo1;
        DspTempo2 m_dspTempo2;
    #else
        DspTempo m_dspTempo;
    #endif
        DspCrossfeed m_dspCrossfeed;
        DspVolume m_dspVolume;
        DspBalance m_dspBalance;
        DspLimiter m_dspLimiter;
        DspDither m_dspDither;

        ISettingsPtr m_settings;
        UINT32 m_deviceSettingsSerial = 0;

        uint32_t m_defaultDeviceSerial = 0;

        std::atomic<float> m_volume = 1.0f;
        std::atomic<float> m_balance = 0.0f;
        double m_rate = 1.0;

        std::atomic<REFERENCE_TIME> m_guidedReclockOffset = 0;
        bool m_guidedReclockActive = false;

        size_t m_dropNextFrames = 0;
    };
}
