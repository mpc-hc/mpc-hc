#include "pch.h"
#include "DspRate.h"

namespace SaneAudioRenderer
{
    namespace
    {
        void DestroyBackend(soxr_t& soxr)
        {
            if (soxr)
            {
                soxr_delete(soxr);
                soxr = nullptr;
            }
        }

        void Crossfade(DspChunk& toChunk, DspChunk& fromChunk, size_t transitionFrames)
        {
            assert(!toChunk.IsEmpty());
            assert(!fromChunk.IsEmpty());
            assert(toChunk.GetRate() == fromChunk.GetRate());
            assert(toChunk.GetChannelCount() == fromChunk.GetChannelCount());
            assert(toChunk.GetFrameCount() >= transitionFrames);
            assert(fromChunk.GetFrameCount() >= transitionFrames);

            DspChunk::ToFloat(toChunk);
            DspChunk::ToFloat(fromChunk);

            const uint32_t channels = toChunk.GetChannelCount();

            auto toData = reinterpret_cast<float*>(toChunk.GetData());
            auto fromData = reinterpret_cast<float*>(fromChunk.GetData());

            for (size_t frame = 0; frame < transitionFrames; frame++)
            {
                // Using linear curve for highly-correlated signals.
                const float m = (float)frame / (transitionFrames + 1);

                for (uint32_t channel = 0; channel < channels; channel++)
                {
                    size_t sample = frame * channels + channel;
                    toData[sample] = toData[sample] * m + fromData[sample] * (1.0f - m);
                }
            }
        }
    }

    DspRate::~DspRate()
    {
        DestroyBackends();
    }

    void DspRate::Initialize(bool variable, uint32_t inputRate, uint32_t outputRate, uint32_t channels)
    {
        DestroyBackends();

        m_state = State::Passthrough;

        m_inStateTransition = false;
        m_transitionCorrelation = {};
        m_transitionChunks = {};

        m_inputRate = inputRate;
        m_outputRate = outputRate;
        m_channels = channels;

        m_variableInputFrames = 0;
        m_variableOutputFrames = 0;
        m_variableDelay = 0;

        m_adjustTime = 0;

        if (variable)
        {
            m_state = State::Variable;
            CreateBackend();
            assert(m_soxrv);
        }
        else if (inputRate != outputRate)
        {
            m_state = State::Constant;
            CreateBackend();
            assert(m_soxrc);
        }
    }

    bool DspRate::Active()
    {
        return m_state != State::Passthrough;
    }

    void DspRate::Process(DspChunk& chunk)
    {
        soxr_t soxr = GetBackend();

        if (!soxr || chunk.IsEmpty())
            return;

        if (m_state == State::Variable && !m_inStateTransition && m_variableDelay > 0)
        {
            uint64_t inputPosition = llMulDiv(m_variableOutputFrames, m_inputRate, m_outputRate, 0);
            int64_t adjustedFrames = inputPosition + m_variableDelay - m_variableInputFrames;

            REFERENCE_TIME adjustTime = m_adjustTime - FramesToTimeLong(adjustedFrames, m_inputRate);

            double ratio = (double)m_inputRate * 4 / (m_outputRate * (4 + (double)adjustTime / OneSecond));

            // TODO: decrease jitter

            soxr_set_io_ratio(m_soxrv, ratio, m_outputRate / 1000);
        }

        DspChunk output = ProcessChunk(soxr, chunk);

        if (m_state == State::Variable)
        {
            m_variableInputFrames += chunk.GetFrameCount();
            m_variableOutputFrames += output.GetFrameCount();

            // soxr_delay() method is not implemented for variable rate conversion yet,
            // but the delay stays more or less constant and we can calculate it in a roundabout way.
            if (m_variableDelay == 0 && m_variableOutputFrames > 0)
            {
                uint64_t inputPosition = llMulDiv(m_variableOutputFrames, m_inputRate, m_outputRate, 0);
                m_variableDelay = m_variableInputFrames - inputPosition;
            }
        }

        FinishStateTransition(output, chunk, false);

        chunk = std::move(output);
    }

    void DspRate::Finish(DspChunk& chunk)
    {
        soxr_t soxr = GetBackend();

        if (!soxr)
            return;

        DspChunk output = ProcessEosChunk(soxr, chunk);

        FinishStateTransition(output, chunk, true);

        chunk = std::move(output);
    }

    void DspRate::Adjust(REFERENCE_TIME time)
    {
        if (m_state != State::Variable)
        {
            m_state = State::Variable;
            CreateBackend();
            assert(m_soxrv);

            m_inStateTransition = true;
        }

        m_adjustTime += time;
    }

    DspChunk DspRate::ProcessChunk(soxr_t soxr, DspChunk& chunk)
    {
        assert(soxr);
        assert(!chunk.IsEmpty());
        assert(chunk.GetRate() == m_inputRate);
        assert(chunk.GetChannelCount() == m_channels);

        DspChunk::ToFloat(chunk);

        size_t outputFrames = (size_t)(2 * (uint64_t)chunk.GetFrameCount() * m_outputRate / m_inputRate);
        DspChunk output(DspFormat::Float, chunk.GetChannelCount(), outputFrames, m_outputRate);

        size_t inputDone = 0;
        size_t outputDone = 0;
        soxr_process(soxr, chunk.GetData(), chunk.GetFrameCount(), &inputDone,
                           output.GetData(), output.GetFrameCount(), &outputDone);
        assert(inputDone == chunk.GetFrameCount());
        output.ShrinkTail(outputDone);

        return output;
    }

    DspChunk DspRate::ProcessEosChunk(soxr_t soxr, DspChunk& chunk)
    {
        assert(soxr);

        DspChunk output;

        if (!chunk.IsEmpty())
            output = ProcessChunk(soxr, chunk);

        for (;;)
        {
            DspChunk tailChunk(DspFormat::Float, m_channels, m_outputRate, m_outputRate);

            size_t inputDone = 0;
            size_t outputDo = tailChunk.GetFrameCount();
            size_t outputDone = 0;
            soxr_process(soxr, nullptr, 0, &inputDone,
                               tailChunk.GetData(), outputDo, &outputDone);
            tailChunk.ShrinkTail(outputDone);

            DspChunk::MergeChunks(output, std::move(tailChunk));

            if (outputDone < outputDo)
                break;
        }

        return output;
    }

    void DspRate::FinishStateTransition(DspChunk& processedChunk, DspChunk& unprocessedChunk, bool eos)
    {
        if (m_inStateTransition)
        {
            assert(m_state == State::Variable);

            DspChunk::ToFloat(processedChunk);
            DspChunk::ToFloat(unprocessedChunk);

            auto& first = m_transitionChunks.first;
            auto& second = m_transitionChunks.second;

            DspChunk::MergeChunks(first, std::move(processedChunk));
            assert(processedChunk.IsEmpty());

            if (m_soxrc)
            {
                // Transitioning from constant rate conversion to variable.
                if (!m_transitionCorrelation.first)
                    m_transitionCorrelation = {true, (size_t)std::round(soxr_delay(m_soxrc))};

                if (m_transitionCorrelation.second > 0)
                {
                    DspChunk::MergeChunks(second, eos ? ProcessEosChunk(m_soxrc, unprocessedChunk) :
                                                        ProcessChunk(m_soxrc, unprocessedChunk));
                }
                else
                {
                    // Nothing to flush from constant rate conversion buffer.
                    m_inStateTransition = false;
                }
            }
            else
            {
                // Transitioning from pass-through to variable rate conversion.
                m_transitionCorrelation = {};
                DspChunk::MergeChunks(second, std::move(unprocessedChunk));
            }

            // Cross-fade.
            if (m_inStateTransition)
            {
                const size_t transitionFrames = m_outputRate / 1000; // 1ms

                if (first.GetFrameCount() >= transitionFrames &&
                    second.GetFrameCount() >= m_transitionCorrelation.second + transitionFrames)
                {
                    second.ShrinkHead(second.GetFrameCount() - m_transitionCorrelation.second);
                    Crossfade(first, second, transitionFrames);
                    processedChunk = std::move(first);
                    m_inStateTransition = false;
                }
                else if (eos)
                {
                    processedChunk = std::move(second);
                    m_inStateTransition = false;
                }
            }

            if (!m_inStateTransition)
            {
                m_transitionCorrelation = {};
                m_transitionChunks = {};
                DestroyBackend(m_soxrc);
            }
        }

        unprocessedChunk = {};
    }

    void DspRate::CreateBackend()
    {
        assert(m_state != State::Passthrough);
        assert(m_inputRate > 0);
        assert(m_outputRate > 0);
        assert(m_channels > 0);

        if (m_state == State::Variable)
        {
            assert(!m_soxrv);

            auto ioSpec = soxr_io_spec(SOXR_FLOAT32_I, SOXR_FLOAT32_I);
            auto qualitySpec = soxr_quality_spec(SOXR_HQ, SOXR_VR);
            m_soxrv = soxr_create(m_inputRate * 2, m_outputRate, m_channels, nullptr, &ioSpec, &qualitySpec, nullptr);

            soxr_set_io_ratio(m_soxrv, (double)m_inputRate / m_outputRate, 0);

            m_variableInputFrames = 0;
            m_variableOutputFrames = 0;
            m_variableDelay = 0;
        }
        else if (m_state == State::Constant)
        {
            assert(m_inputRate != m_outputRate);
            assert(!m_soxrc);

            auto ioSpec = soxr_io_spec(SOXR_FLOAT32_I, SOXR_FLOAT32_I);
            auto qualitySpec = soxr_quality_spec(SOXR_HQ, 0);
            m_soxrc = soxr_create(m_inputRate, m_outputRate, m_channels, nullptr, &ioSpec, &qualitySpec, nullptr);
        }
    }

    soxr_t DspRate::GetBackend()
    {
        return (m_state == State::Constant) ? m_soxrc :
               (m_state == State::Variable) ? m_soxrv : nullptr;
    }

    void DspRate::DestroyBackends()
    {
        DestroyBackend(m_soxrc);
        DestroyBackend(m_soxrv);
    }
}
