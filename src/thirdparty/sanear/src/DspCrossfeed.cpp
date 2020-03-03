#include "pch.h"
#include "DspCrossfeed.h"

namespace SaneAudioRenderer
{
    void DspCrossfeed::Initialize(ISettings* pSettings, uint32_t rate, uint32_t channels, DWORD mask)
    {
        assert(pSettings);
        m_settings = pSettings;

        m_possible = (channels == 2 &&
                      mask == KSAUDIO_SPEAKER_STEREO &&
                      rate >= BS2B_MINSRATE &&
                      rate <= BS2B_MAXSRATE);

        if (m_possible)
        {
            m_bs2b.clear();
            m_bs2b.set_srate(rate);
            UpdateSettings();
        }
    }

    bool DspCrossfeed::Active()
    {
        return m_active;
    }

    void DspCrossfeed::Process(DspChunk& chunk)
    {
        if (m_settingsSerial != m_settings->GetSerial())
            UpdateSettings();

        if (!m_active || chunk.IsEmpty())
            return;

        assert(chunk.GetChannelCount() == 2);

        DspChunk::ToFloat(chunk);

        m_bs2b.cross_feed((float*)chunk.GetData(), (int)chunk.GetFrameCount());
    }

    void DspCrossfeed::Finish(DspChunk& chunk)
    {
        Process(chunk);
    }

    void DspCrossfeed::UpdateSettings()
    {
        m_settingsSerial = m_settings->GetSerial();

        UINT32 cutoffFrequency;
        UINT32 crossfeedLevel;
        m_settings->GetCrossfeedSettings(&cutoffFrequency, &crossfeedLevel);

        bool wasActive = m_active;

        BOOL enabled = m_settings->GetCrossfeedEnabled();

        m_active = m_possible && enabled;

        if (m_active)
        {
            m_bs2b.set_level_fcut(cutoffFrequency);
            m_bs2b.set_level_feed(crossfeedLevel);
        }
        else if (wasActive)
        {
            m_bs2b.clear();
        }
    }
}
