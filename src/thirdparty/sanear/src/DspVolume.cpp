#include "pch.h"
#include "DspVolume.h"

#include "AudioRenderer.h"

namespace SaneAudioRenderer
{
    bool DspVolume::Active()
    {
        return m_renderer.GetVolume() != 1.0f;
    }

    void DspVolume::Process(DspChunk& chunk)
    {
        const float volume = m_renderer.GetVolume();
        assert(volume >= 0.0f && volume <= 1.0f);

        if (volume == 1.0f || chunk.IsEmpty())
            return;

        DspChunk::ToFloat(chunk);

        auto data = reinterpret_cast<float*>(chunk.GetData());
        for (size_t i = 0, n = chunk.GetSampleCount(); i < n; i++)
            data[i] *= volume;
    }

    void DspVolume::Finish(DspChunk& chunk)
    {
        Process(chunk);
    }
}
