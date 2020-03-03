#include "pch.h"
#include "SampleCorrection.h"

namespace SaneAudioRenderer
{
    void SampleCorrection::NewFormat(SharedWaveFormat format)
    {
        assert(format);
        assert(format->nSamplesPerSec > 0);

        if (m_format)
        {
            m_segmentTimeInPreviousFormats += FramesToTime(m_segmentFramesInCurrentFormat);
            m_segmentFramesInCurrentFormat = 0;
        }

        m_format = format;
        m_bitstream = (DspFormatFromWaveFormat(*m_format) == DspFormat::Unknown);
    }

    void SampleCorrection::NewSegment(double rate)
    {
        assert(rate > 0.0);

        m_rate = rate;

        m_segmentTimeInPreviousFormats = 0;
        m_segmentFramesInCurrentFormat = 0;

        m_lastFrameEnd = 0;

        m_timeDivergence = 0;
    }

    void SampleCorrection::NewDeviceBuffer()
    {
        m_freshBuffer = true;
    }

    DspChunk SampleCorrection::ProcessSample(IMediaSample* pSample, AM_SAMPLE2_PROPERTIES& sampleProps, bool realtimeDevice)
    {
        assert(m_format);

        DspChunk chunk(pSample, sampleProps, *m_format);

        if (m_bitstream)
        {
            if (m_freshBuffer && !(sampleProps.dwSampleFlags & AM_SAMPLE_SPLICEPOINT))
            {
                // Drop the sample.
                DebugOut(ClassName(this), "drop [", sampleProps.tStart, sampleProps.tStop, "]");
                chunk = DspChunk();
                assert(chunk.IsEmpty());
            }
        }
        else if (!realtimeDevice && (m_lastFrameEnd == 0 || (sampleProps.dwSampleFlags & AM_SAMPLE_TIMEDISCONTINUITY)))
        {
            if ((sampleProps.dwSampleFlags & AM_SAMPLE_STOPVALID) && sampleProps.tStop <= m_lastFrameEnd)
            {
                // Drop the sample.
                DebugOut(ClassName(this), "drop [", sampleProps.tStart, sampleProps.tStop, "]");
                chunk = DspChunk();
                assert(chunk.IsEmpty());
            }
            else if ((sampleProps.dwSampleFlags & AM_SAMPLE_TIMEVALID) && sampleProps.tStart < m_lastFrameEnd)
            {
                // Crop the sample.
                const size_t cropFrames = (size_t)TimeToFrames(m_lastFrameEnd - sampleProps.tStart);

                if (cropFrames > 0)
                {
                    DebugOut(ClassName(this), "crop", cropFrames, "frames from [",
                             sampleProps.tStart, sampleProps.tStop, "]");

                    chunk.ShrinkHead(chunk.GetFrameCount() > cropFrames ? chunk.GetFrameCount() - cropFrames : 0);

                    sampleProps.tStart += FramesToTime(cropFrames);
                }
            }
            else if ((sampleProps.dwSampleFlags & AM_SAMPLE_TIMEVALID) && sampleProps.tStart > m_lastFrameEnd)
            {
                // Zero-pad the sample.
                const size_t padFrames = (size_t)TimeToFrames(sampleProps.tStart - m_lastFrameEnd);

                if (padFrames > 0 &&
                    FramesToTime(padFrames) < 10 * OneSecond)
                {
                    DebugOut(ClassName(this), "pad", padFrames, "frames before [",
                             sampleProps.tStart, sampleProps.tStop, "]");

                    chunk.PadHead(padFrames);

                    sampleProps.tStart -= FramesToTime(padFrames);
                }
            }
        }

        AccumulateTimings(sampleProps, chunk.GetFrameCount());

        return chunk;
    }

    uint64_t SampleCorrection::TimeToFrames(REFERENCE_TIME time)
    {
        assert(m_format);
        assert(m_rate > 0.0);
        return (size_t)(llMulDiv(time, m_format->nSamplesPerSec, OneSecond, 0) * m_rate);
    }

    REFERENCE_TIME SampleCorrection::FramesToTime(uint64_t frames)
    {
        assert(m_format);
        assert(m_rate > 0.0);
        return (REFERENCE_TIME)(llMulDiv(frames, OneSecond, m_format->nSamplesPerSec, 0) / m_rate);
    }

    void SampleCorrection::AccumulateTimings(AM_SAMPLE2_PROPERTIES& sampleProps, size_t frames)
    {
        assert(m_format);
        assert(m_rate > 0.0);

        if (frames == 0)
            return;

        if (sampleProps.dwSampleFlags & AM_SAMPLE_TIMEVALID)
            m_timeDivergence = sampleProps.tStart - m_lastFrameEnd;

        m_segmentFramesInCurrentFormat += frames;

        m_lastFrameEnd = m_segmentTimeInPreviousFormats + FramesToTime(m_segmentFramesInCurrentFormat);

        m_freshBuffer = false;
    }
}
