#pragma once

#include "DspBase.h"

namespace SaneAudioRenderer
{
    class DspLimiter final
        : public DspBase
    {
    public:

        DspLimiter() = default;
        DspLimiter(const DspLimiter&) = delete;
        DspLimiter& operator=(const DspLimiter&) = delete;

        void Initialize(uint32_t rate, uint32_t channels, bool exclusive);

        std::wstring Name() override { return L"Limiter"; }

        bool Active() override;

        void Process(DspChunk& chunk) override;
        void Finish(DspChunk& chunk) override;

    private:

        void NewTreshold(float peak);

        bool m_exclusive = false;
        uint32_t m_rate = 0;
        uint32_t m_channels = 0;

        bool m_active = false;
        int64_t m_holdWindow = 0;
        float m_peak = 0.0f;
        float m_threshold = 0.0f;
    };
}
