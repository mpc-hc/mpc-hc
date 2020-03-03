#pragma once

#include "DspBase.h"

#include <SoundTouch.h>

namespace SaneAudioRenderer
{
    class DspTempo final
        : public DspBase
    {
    public:

        DspTempo() = default;
        DspTempo(const DspTempo&) = delete;
        DspTempo& operator=(const DspTempo&) = delete;

        void Initialize(double tempo, uint32_t rate, uint32_t channels);

        std::wstring Name() override { return L"Tempo"; }

        bool Active() override;

        void Process(DspChunk& chunk) override;
        void Finish(DspChunk& chunk) override;

    private:

        void AdjustTempo();

        soundtouch::SoundTouch m_stouch;

        bool m_active = false;

        uint32_t m_rate = 0;
        uint32_t m_channels = 0;

        double m_tempo = 1.0;
        float m_ftempo1 = 1.0f;
        float m_ftempo2 = 1.0f;
        float m_ftempo = 1.0f;
        uint64_t m_outSamples1 = 0;
        uint64_t m_outSamples2 = 0;
    };
}
