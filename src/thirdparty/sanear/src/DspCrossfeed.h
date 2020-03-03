#pragma once

#include "DspBase.h"
#include "Interfaces.h"

#include <bs2bclass.h>

namespace SaneAudioRenderer
{
    class DspCrossfeed final
        : public DspBase
    {
    public:

        DspCrossfeed() = default;
        DspCrossfeed(const DspCrossfeed&) = delete;
        DspCrossfeed& operator=(const DspCrossfeed&) = delete;

        void Initialize(ISettings* pSettings, uint32_t rate, uint32_t channels, DWORD mask);

        std::wstring Name() override { return L"Crossfeed"; }

        bool Active() override;

        void Process(DspChunk& chunk) override;
        void Finish(DspChunk& chunk) override;

    private:

        void UpdateSettings();

        bs2b_base m_bs2b;

        ISettingsPtr m_settings;
        UINT32 m_settingsSerial = 0;

        bool m_possible = false;
        bool m_active = false;
    };
}
