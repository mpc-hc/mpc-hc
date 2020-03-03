#include "pch.h"
#include "DspTempo2.h"

#ifndef SANEAR_GPL_PHASE_VOCODER
namespace SaneAudioRenderer { void DspTempo2::ShutNoPublicSymbolsWarning() {} }
#else

namespace SaneAudioRenderer
{
    void DspTempo2::Initialize(double tempo, uint32_t rate, uint32_t channels)
    {
        m_stretcher = nullptr;

        m_active = false;
        m_finish = false;

        m_rate = rate;
        m_channels = channels;

        if (tempo != 1.0)
        {
            try
            {
                auto options = RubberBand::RubberBandStretcher::OptionTransientsMixed |
                               RubberBand::RubberBandStretcher::OptionProcessRealTime;

                m_stretcher = std::make_unique<RubberBand::RubberBandStretcher>(rate, channels, options, 1.0 / tempo);

                m_stretcher->setMaxProcessSize(rate);

                m_active = true;
            }
            catch (std::bad_alloc&)
            {
            }
        }
    }

    bool DspTempo2::Active()
    {
        return m_active;
    }

    void DspTempo2::Process(DspChunk& chunk)
    {
        if (!m_active || chunk.IsEmpty())
            return;

        assert(chunk.GetRate() == m_rate);
        assert(chunk.GetChannelCount() == m_channels);

        DspChunk::ToFloat(chunk);
        m_stretcher->process(Deinterleave(chunk).data(), chunk.GetFrameCount(), m_finish);

        size_t outputFrames = m_stretcher->available();

        if (outputFrames > 0)
        {

            DspChunk output(DspFormat::Float, m_channels, outputFrames, m_rate);

            size_t outputDone = m_stretcher->retrieve(MarkData(output).data(), outputFrames);
            assert(outputDone == outputFrames);

            Interleave(output);

            chunk = std::move(output);
        }
        else
        {
            chunk = DspChunk();
        }
    }

    void DspTempo2::Finish(DspChunk& chunk)
    {
        if (!m_active)
            return;

        assert(!m_finish);
        m_finish = true;

        Process(chunk);
    }

    DspTempo2::DeinterleavedData DspTempo2::MarkData(DspChunk& chunk)
    {
        assert(!chunk.IsEmpty());
        assert(chunk.GetFormat() == DspFormat::Float);

        DeinterleavedData data = {};

        for (size_t i = 0; i < m_channels; i++)
            data[i] = (float*)(chunk.GetData() + chunk.GetFormatSize() * chunk.GetFrameCount() * i);

        return data;
    }

    DspTempo2::DeinterleavedData DspTempo2::Deinterleave(DspChunk& chunk)
    {
        assert(!chunk.IsEmpty());
        assert(chunk.GetFormat() == DspFormat::Float);

        DspChunk output(DspFormat::Float, m_channels, chunk.GetFrameCount(), m_rate);
        DeinterleavedData outputData = MarkData(output);

        float* inputData = (float*)chunk.GetData();

        for (size_t channel = 0; channel < m_channels; channel++)
            for (size_t i = 0, n = chunk.GetFrameCount(); i < n; i++)
                outputData[channel][i] = inputData[channel + i * m_channels];

        chunk = std::move(output);

        return outputData;
    }

    void DspTempo2::Interleave(DspChunk& chunk)
    {
        assert(!chunk.IsEmpty());
        assert(chunk.GetFormat() == DspFormat::Float);

        DspChunk output(DspFormat::Float, m_channels, chunk.GetFrameCount(), m_rate);
        float* outputData = (float*)output.GetData();

        DeinterleavedData inputData = MarkData(chunk);

        for (size_t channel = 0; channel < m_channels; channel++)
            for (size_t i = 0, n = chunk.GetFrameCount(); i < n; i++)
                outputData[channel + i * m_channels] = inputData[channel][i];

        chunk = std::move(output);
    }
}

#endif
