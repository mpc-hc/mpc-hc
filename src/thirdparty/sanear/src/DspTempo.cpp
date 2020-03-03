#include "pch.h"
#include "DspTempo.h"

namespace SaneAudioRenderer
{
    void DspTempo::Initialize(double tempo, uint32_t rate, uint32_t channels)
    {
        m_stouch.clear();

        m_active = false;

        m_rate = rate;
        m_channels = channels;

        m_tempo = tempo;
        m_ftempo1 = (float)tempo;
        m_ftempo2 = std::nexttoward(m_ftempo1, tempo);
        m_ftempo = m_ftempo1;
        m_outSamples1 = 0;
        m_outSamples2 = 0;

        if (tempo != 1.0)
        {
            m_stouch.setSampleRate(rate);
            m_stouch.setChannels(channels);

            m_stouch.setTempo(m_ftempo);

            //m_stouch.setSetting(SETTING_SEQUENCE_MS, 40);
            //m_stouch.setSetting(SETTING_SEEKWINDOW_MS, 15);
            //m_stouch.setSetting(SETTING_OVERLAP_MS, 8);

            m_active = true;
        }
    }

    bool DspTempo::Active()
    {
        return m_active;
    }

    void DspTempo::Process(DspChunk& chunk)
    {
        if (!m_active || chunk.IsEmpty())
            return;

        assert(chunk.GetRate() == m_rate);
        assert(chunk.GetChannelCount() == m_channels);

        // DirectShow speed is in double precision, SoundTouch operates in single.
        // We have to adjust it dynamically.
        AdjustTempo();

        DspChunk::ToFloat(chunk);

        m_stouch.putSamples((const float*)chunk.GetData(), (uint32_t)chunk.GetFrameCount());

        DspChunk output(DspFormat::Float, m_channels, m_stouch.numSamples(), m_rate);

        uint32_t done = m_stouch.receiveSamples((float*)output.GetData(), (uint32_t)output.GetFrameCount());
        assert(done == output.GetFrameCount());
        output.ShrinkTail(done);

        auto& outSamples = (m_ftempo == m_ftempo1) ? m_outSamples1 : m_outSamples2;
        outSamples += done;

        chunk = std::move(output);
    }

    void DspTempo::Finish(DspChunk& chunk)
    {
        if (!m_active)
            return;

        Process(chunk);

        m_stouch.flush();
        uint32_t undone = m_stouch.numSamples();

        if (undone > 0)
        {
            DspChunk output(DspFormat::Float, m_channels, chunk.GetFrameCount() + undone, m_rate);

            if (!chunk.IsEmpty())
                memcpy(output.GetData(), chunk.GetData(), chunk.GetSize());

            m_stouch.flush();

            uint32_t done = m_stouch.receiveSamples((float*)output.GetData() + chunk.GetSampleCount(), undone);
            assert(done == undone);
            output.ShrinkTail(chunk.GetFrameCount() + done);

            chunk = std::move(output);
        }
    }

    void DspTempo::AdjustTempo()
    {
        if (m_tempo != m_ftempo)
        {
            assert(m_tempo != m_ftempo1);
            assert(m_tempo != m_ftempo2);

            double ratio21 = std::abs((m_tempo - m_ftempo1) / (m_tempo - m_ftempo2));

            if (m_ftempo != m_ftempo2 &&
                m_outSamples1 * ratio21 - m_outSamples2 > 60 * m_rate)
            {
                DebugOut(ClassName(this), "adjusting for float/double imprecision (2), ratio", ratio21);
                m_ftempo = m_ftempo2;
                m_stouch.setTempo(m_ftempo);
            }
            else if (m_ftempo != m_ftempo1 &&
                     m_outSamples2 - m_outSamples1 * ratio21 > 60 * m_rate)
            {
                DebugOut(ClassName(this), "adjusting for float/double imprecision (1), ratio", ratio21);
                m_ftempo = m_ftempo1;
                m_stouch.setTempo(m_ftempo);
            }
        }
    }
}
