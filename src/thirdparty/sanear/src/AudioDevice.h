#pragma once

#include "DspChunk.h"
#include "DspFormat.h"

namespace SaneAudioRenderer
{
    struct AudioDeviceBackend final
    {
        SharedString          id;
        SharedString          adapterName;
        SharedString          endpointName;
        UINT32                endpointFormFactor;
        bool                  supportsSharedEventMode;
        bool                  supportsExclusiveEventMode;

        IAudioClientPtr       audioClient;
        IAudioRenderClientPtr audioRenderClient;
        IAudioClockPtr        audioClock;

        SharedWaveFormat      mixFormat;

        SharedWaveFormat      waveFormat;
        DspFormat             dspFormat;

        uint32_t              bufferDuration;

        REFERENCE_TIME        deviceLatency;
        UINT32                deviceBufferSize;

        bool                  exclusive;
        bool                  bitstream;
        bool                  eventMode;
        bool                  realtime;

        bool                  ignoredSystemChannelMixer;
    };

    class AudioDevice
    {
    public:

        virtual ~AudioDevice() = default;

        virtual void Push(DspChunk& chunk, CAMEvent* pFilledEvent) = 0;
        virtual REFERENCE_TIME Finish(CAMEvent* pFilledEvent) = 0;

        virtual int64_t GetPosition() = 0;
        virtual int64_t GetEnd() = 0;
        virtual int64_t GetSilence() = 0;

        virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual void Reset() = 0;

        SharedString GetId()           const { return m_backend->id; }
        SharedString GetAdapterName()  const { return m_backend->adapterName; }
        SharedString GetEndpointName() const { return m_backend->endpointName; }

        IAudioClockPtr GetClock() { return m_backend->audioClock; }

        SharedWaveFormat GetMixFormat()      const { return m_backend->mixFormat; }

        SharedWaveFormat GetWaveFormat()     const { return m_backend->waveFormat; }
        uint32_t         GetRate()           const { return m_backend->waveFormat->nSamplesPerSec; }
        uint32_t         GetChannelCount()   const { return m_backend->waveFormat->nChannels; }
        DspFormat        GetDspFormat()      const { return m_backend->dspFormat; }
        uint32_t         GetBufferDuration() const { return m_backend->bufferDuration; }
        REFERENCE_TIME   GetStreamLatency()  const { return m_backend->deviceLatency; }

        bool IsExclusive() const { return m_backend->exclusive; }
        bool IsRealtime()  const { return m_backend->realtime; }

        bool IgnoredSystemChannelMixer() const { return m_backend->ignoredSystemChannelMixer; }

        using RenewBackendFunction = std::function<bool(std::shared_ptr<AudioDeviceBackend>&)>;
        virtual bool RenewInactive(const RenewBackendFunction& renewBackend, int64_t& position) = 0;

    protected:

        std::shared_ptr<AudioDeviceBackend> m_backend;

        template <class T>
        bool IsLastInstance(T& smartPointer)
        {
            bool ret = (smartPointer.GetInterfacePtr()->AddRef() == 2);
            smartPointer.GetInterfacePtr()->Release();
            return ret;
        }

        bool CheckLastInstances()
        {
            if (!m_backend.unique())
                return false;

            if (m_backend->audioClock && !IsLastInstance(m_backend->audioClock))
                return false;

            m_backend->audioClock = nullptr;

            if (m_backend->audioRenderClient && !IsLastInstance(m_backend->audioRenderClient))
                return false;

            m_backend->audioRenderClient = nullptr;

            if (m_backend->audioClient && !IsLastInstance(m_backend->audioClient))
                return false;

            return true;
        }
    };
}
