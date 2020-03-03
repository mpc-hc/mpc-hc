#pragma once

#include "AudioDevice.h"
#include "DspChunk.h"
#include "DspFormat.h"

namespace SaneAudioRenderer
{
    class AudioDevicePush final
        : public AudioDevice
    {
    public:

        AudioDevicePush(std::shared_ptr<AudioDeviceBackend> backend);
        AudioDevicePush(const AudioDevicePush&) = delete;
        AudioDevicePush& operator=(const AudioDevicePush&) = delete;
        ~AudioDevicePush();

        void Push(DspChunk& chunk, CAMEvent* pFilledEvent) override;
        REFERENCE_TIME Finish(CAMEvent* pFilledEvent) override;

        int64_t GetPosition() override;
        int64_t GetEnd() override;
        int64_t GetSilence() override;

        void Start() override;
        void Stop() override;
        void Reset() override;

        bool RenewInactive(const RenewBackendFunction& renewBackend, int64_t& position) override;

    private:

        void SilenceFeed();

        void PushChunkToDevice(DspChunk& chunk, CAMEvent* pFilledEvent);
        UINT32 PushSilenceToDevice(UINT32 frames);

        bool m_endOfStream = false;
        int64_t m_endOfStreamPos = 0;

        uint64_t m_pushedFrames = 0;
        std::atomic<uint64_t> m_silenceFrames = 0;

        std::thread m_thread;
        CAMEvent m_wake;
        std::atomic<bool> m_exit = false;
        std::atomic<bool> m_error = false;
    };
}
