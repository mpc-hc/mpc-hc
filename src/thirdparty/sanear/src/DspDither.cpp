#include "pch.h"
#include "DspDither.h"

namespace SaneAudioRenderer
{
    void DspDither::Initialize(DspFormat outputFormat)
    {
        m_enabled = (outputFormat == DspFormat::Pcm16);
        m_active = m_enabled;

        for (size_t i = 0; i < 18; i++)
        {
            m_previous[i] = 0.0f;
            m_generator[i].seed((uint32_t)(GetPerformanceCounter() + i));
            m_distributor[i] = std::uniform_real_distribution<float>(0, 1.0f);
        }
    }

    bool DspDither::Active()
    {
        return m_enabled && m_active;
    }

    void DspDither::Process(DspChunk& chunk)
    {
        if (!m_enabled || chunk.IsEmpty() || chunk.GetFormatSize() <= DspFormatSize(DspFormat::Pcm16))
        {
            m_active = false;
            return;
        }

        m_active = true;

        DspChunk::ToFloat(chunk);

        DspChunk output(DspFormat::Pcm16, chunk.GetChannelCount(), chunk.GetFrameCount(), chunk.GetRate());

        auto inputData = reinterpret_cast<const float*>(chunk.GetData());
        auto outputData = reinterpret_cast<int16_t*>(output.GetData());
        const size_t channels = chunk.GetChannelCount();

        for (size_t frame = 0, frames = chunk.GetFrameCount(); frame < frames; frame++)
        {
            for (size_t channel = 0; channel < channels; channel++)
            {
                float inputSample = inputData[frame * channels + channel] * (INT16_MAX - 1);

                // High-pass TPDF, 2 LSB amplitude.
                float r = m_distributor[channel](m_generator[channel]);
                float noise = r - m_previous[channel];
                m_previous[channel] = r;

                float outputSample = std::round(inputSample + noise);
                assert(outputSample >= INT16_MIN && outputSample <= INT16_MAX);
                outputData[frame * channels + channel] = (int16_t)outputSample;
            }
        }

        chunk = std::move(output);
    }

    void DspDither::Finish(DspChunk& chunk)
    {
        Process(chunk);
    }
}
