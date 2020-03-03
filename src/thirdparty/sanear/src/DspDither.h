#pragma once

#include "DspBase.h"

namespace SaneAudioRenderer
{
    class DspDither final
        : public DspBase
    {
    public:

        DspDither() = default;
        DspDither(const DspDither&) = delete;
        DspDither& operator=(const DspDither&) = delete;

        void Initialize(DspFormat outputFormat);

        std::wstring Name() override { return L"Dither"; }

        bool Active() override;

        void Process(DspChunk& chunk) override;
        void Finish(DspChunk& chunk) override;

    private:

        bool m_enabled = false;
        bool m_active = false;
        std::array<float, 18> m_previous;
        std::array<std::minstd_rand, 18> m_generator;
        std::array<std::uniform_real_distribution<float>, 18> m_distributor;
    };
}
