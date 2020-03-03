#pragma once

#ifndef SANEAR_GPL_PHASE_VOCODER
namespace SaneAudioRenderer { struct DspTempo2 final { void ShutNoPublicSymbolsWarning(); }; }
#else

#include "DspBase.h"

#include <RubberBandStretcher.h>

namespace SaneAudioRenderer
{
    class DspTempo2 final
        : public DspBase
    {
    public:

        DspTempo2() = default;
        DspTempo2(const DspTempo2&) = delete;
        DspTempo2& operator=(const DspTempo2&) = delete;

        void Initialize(double tempo, uint32_t rate, uint32_t channels);

        std::wstring Name() override { return L"Tempo"; }

        bool Active() override;

        void Process(DspChunk& chunk) override;
        void Finish(DspChunk& chunk) override;

    private:

        using DeinterleavedData = std::array<float*, 18>;

        DeinterleavedData MarkData(DspChunk& chunk);
        DeinterleavedData Deinterleave(DspChunk& chunk);
        void Interleave(DspChunk& chunk);

        std::unique_ptr<RubberBand::RubberBandStretcher> m_stretcher;

        bool m_active = false;
        bool m_finish = false;

        uint32_t m_rate = 0;
        uint32_t m_channels = 0;
    };
}

#endif
