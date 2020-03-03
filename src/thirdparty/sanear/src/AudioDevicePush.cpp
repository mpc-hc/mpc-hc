#include "pch.h"
#include "AudioDevicePush.h"

namespace SaneAudioRenderer
{
    AudioDevicePush::AudioDevicePush(std::shared_ptr<AudioDeviceBackend> backend)
    {
        DebugOut(ClassName(this), "create");

        assert(backend);
        assert(!backend->eventMode);
        m_backend = backend;

        if (static_cast<HANDLE>(m_wake) == NULL)
            throw E_OUTOFMEMORY;
    }

    AudioDevicePush::~AudioDevicePush()
    {
        DebugOut(ClassName(this), "destroy");

        m_exit = true;
        m_wake.Set();

        if (m_thread.joinable())
            m_thread.join();

        assert(CheckLastInstances());
        m_backend = nullptr;
    }

    void AudioDevicePush::Push(DspChunk& chunk, CAMEvent* pFilledEvent)
    {
        assert(!m_endOfStream);

        PushChunkToDevice(chunk, pFilledEvent);
    }

    REFERENCE_TIME AudioDevicePush::Finish(CAMEvent* pFilledEvent)
    {
        if (m_error)
            throw E_FAIL;

        if (!m_endOfStream)
        {
            DebugOut(ClassName(this), "finish");

            m_endOfStream = true;
            m_endOfStreamPos = GetEnd();

            try
            {
                if (!m_thread.joinable())
                {
                    assert(!m_exit);
                    m_thread = std::thread(std::bind(&AudioDevicePush::SilenceFeed, this));
                }
            }
            catch (std::system_error&)
            {
                throw E_OUTOFMEMORY;
            }
        }

        if (pFilledEvent)
            pFilledEvent->Set();

        return m_endOfStreamPos - GetPosition();
    }

    int64_t AudioDevicePush::GetPosition()
    {
        UINT64 deviceClockFrequency, deviceClockPosition;
        ThrowIfFailed(m_backend->audioClock->GetFrequency(&deviceClockFrequency));
        ThrowIfFailed(m_backend->audioClock->GetPosition(&deviceClockPosition, nullptr));

        return llMulDiv(deviceClockPosition, OneSecond, deviceClockFrequency, 0);
    }

    int64_t AudioDevicePush::GetEnd()
    {
        return FramesToTimeLong(m_pushedFrames, GetRate());
    }

    int64_t AudioDevicePush::GetSilence()
    {
        return FramesToTimeLong(m_silenceFrames, GetRate());
    }

    void AudioDevicePush::Start()
    {
        DebugOut(ClassName(this), "start");

        m_backend->audioClient->Start();
    }

    void AudioDevicePush::Stop()
    {
        DebugOut(ClassName(this), "stop");

        m_backend->audioClient->Stop();
    }

    void AudioDevicePush::Reset()
    {
        DebugOut(ClassName(this), "reset");

        if (m_thread.joinable())
        {
            m_exit = true;
            m_wake.Set();
            m_thread.join();
            m_exit = false;
        }

        m_backend->audioClient->Reset();
        m_pushedFrames = 0;
        m_silenceFrames = 0;

        m_endOfStream = false;
        m_endOfStreamPos = 0;
    }

    bool AudioDevicePush::RenewInactive(const RenewBackendFunction& renewBackend, int64_t& position)
    {
        position = 0;
        return true;
    }

    void AudioDevicePush::SilenceFeed()
    {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

        while (!m_exit && !m_error)
        {
            try
            {
                m_silenceFrames += PushSilenceToDevice(m_backend->deviceBufferSize);
                m_wake.Wait(m_backend->bufferDuration / 4);
            }
            catch (HRESULT)
            {
                m_error = true;
            }
        }
    }

    void AudioDevicePush::PushChunkToDevice(DspChunk& chunk, CAMEvent* pFilledEvent)
    {
        // Get up-to-date information on the device buffer.
        UINT32 bufferPadding;
        ThrowIfFailed(m_backend->audioClient->GetCurrentPadding(&bufferPadding));

        // Find out how many frames we can write this time.
        const UINT32 doFrames = std::min(m_backend->deviceBufferSize - bufferPadding, (UINT32)chunk.GetFrameCount());

        if (doFrames == 0)
            return;

        // Write frames to the device buffer.
        BYTE* deviceBuffer;
        ThrowIfFailed(m_backend->audioRenderClient->GetBuffer(doFrames, &deviceBuffer));
        assert(chunk.GetFrameSize() == (m_backend->waveFormat->wBitsPerSample / 8 * m_backend->waveFormat->nChannels));
        memcpy(deviceBuffer, chunk.GetData(), doFrames * chunk.GetFrameSize());
        ThrowIfFailed(m_backend->audioRenderClient->ReleaseBuffer(doFrames, 0));

        // If the buffer is fully filled, set the corresponding event (if requested).
        if (pFilledEvent &&
            bufferPadding + doFrames == m_backend->deviceBufferSize)
        {
            pFilledEvent->Set();
        }

        assert(doFrames <= chunk.GetFrameCount());
        chunk.ShrinkHead(chunk.GetFrameCount() - doFrames);

        m_pushedFrames += doFrames;
    }

    UINT32 AudioDevicePush::PushSilenceToDevice(UINT32 frames)
    {
        // Get up-to-date information on the device buffer.
        UINT32 bufferPadding;
        ThrowIfFailed(m_backend->audioClient->GetCurrentPadding(&bufferPadding));

        // Find out how many frames we can write this time.
        const UINT32 doFrames = std::min(m_backend->deviceBufferSize - bufferPadding, frames);

        if (doFrames == 0)
            return 0;

        // Write frames to the device buffer.
        BYTE* deviceBuffer;
        ThrowIfFailed(m_backend->audioRenderClient->GetBuffer(doFrames, &deviceBuffer));
        ThrowIfFailed(m_backend->audioRenderClient->ReleaseBuffer(doFrames, AUDCLNT_BUFFERFLAGS_SILENT));

        DebugOut(ClassName(this), "push", 1000. * doFrames / m_backend->waveFormat->nSamplesPerSec, "ms of silence");

        m_pushedFrames += doFrames;

        return doFrames;
    }
}
