#pragma once

#include "DspChunk.h"

namespace SaneAudioRenderer
{
    class SampleCorrection final
    {
    public:

        SampleCorrection() = default;

        void NewFormat(SharedWaveFormat format);
        void NewSegment(double rate);
        void NewDeviceBuffer();

        DspChunk ProcessSample(IMediaSample* pSample, AM_SAMPLE2_PROPERTIES& sampleProps, bool realtimeDevice);

        REFERENCE_TIME GetLastFrameEnd()   const { return m_lastFrameEnd; }
        REFERENCE_TIME GetTimeDivergence() const { return m_timeDivergence; }

    private:

        void AccumulateTimings(AM_SAMPLE2_PROPERTIES& sampleProps, size_t frames);

        uint64_t TimeToFrames(REFERENCE_TIME time);
        REFERENCE_TIME FramesToTime(uint64_t frames);

        SharedWaveFormat m_format;
        bool m_bitstream = false;

        double m_rate = 1.0;

        REFERENCE_TIME m_segmentTimeInPreviousFormats = 0;
        uint64_t m_segmentFramesInCurrentFormat = 0;

        REFERENCE_TIME m_lastFrameEnd = 0;

        REFERENCE_TIME m_timeDivergence = 0;

        bool m_freshBuffer = true;
    };
}
