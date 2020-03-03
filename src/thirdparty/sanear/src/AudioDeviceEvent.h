#pragma once

#include "AudioDevice.h"
#include "DspChunk.h"
#include "DspFormat.h"

namespace SaneAudioRenderer
{
    class AudioDeviceEvent final
        : public AudioDevice
    {
    public:

        AudioDeviceEvent(std::shared_ptr<AudioDeviceBackend> backend);
        AudioDeviceEvent(const AudioDeviceEvent&) = delete;
        AudioDeviceEvent& operator=(const AudioDeviceEvent&) = delete;
        ~AudioDeviceEvent();

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

        void EventFeed();

        void PushBufferToDevice();
        void PushChunkToBuffer(DspChunk& chunk);

        std::atomic<bool> m_endOfStream = false;
        int64_t m_endOfStreamPos = 0;

        std::thread m_thread;
        CCritSec m_threadMutex;

        CAMEvent m_wake;
        std::atomic<bool> m_exit = false;
        std::atomic<bool> m_error = false;

        uint64_t m_sentFrames = 0;
        std::atomic<uint64_t> m_receivedFrames = 0;
        std::atomic<uint64_t> m_silenceFrames = 0;

        CCritSec m_bufferMutex;
        std::deque<DspChunk> m_buffer;
        size_t m_bufferFrames = 0;

        bool m_queuedStart = false;

        bool m_observeInactivity = false;
        CAMEvent m_observeInactivityWake;
        int64_t m_activityPointCounter = 0;

        CCritSec m_renewMutex;
        bool m_awaitingRenew = false;
        int64_t m_renewPosition = 0;
        size_t m_renewSilenceFrames = 0;
    };
}
