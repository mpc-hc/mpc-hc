#pragma once

#include "DspBase.h"

namespace SaneAudioRenderer
{
    class AudioRenderer;

    class DspVolume final
        : public DspBase
    {
    public:

        DspVolume(AudioRenderer& renderer) : m_renderer(renderer) {}
        DspVolume(const DspVolume&) = delete;
        DspVolume& operator=(const DspVolume&) = delete;

        std::wstring Name() override { return L"Volume"; }

        bool Active() override;

        void Process(DspChunk& chunk) override;
        void Finish(DspChunk& chunk) override;

    private:

        const AudioRenderer& m_renderer;
    };
}
