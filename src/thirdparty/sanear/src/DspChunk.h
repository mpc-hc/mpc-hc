#pragma once

#include "DspFormat.h"

namespace SaneAudioRenderer
{
    class DspChunk final
    {
    public:

        static void ToFormat(DspFormat format, DspChunk& chunk);
        static void ToFloat(DspChunk& chunk) { ToFormat(DspFormat::Float, chunk); }
        static void ToDouble(DspChunk& chunk) { ToFormat(DspFormat::Double, chunk); }

        static void MergeChunks(DspChunk& chunk, DspChunk&& appendage);

        DspChunk();
        DspChunk(DspFormat format, uint32_t channels, size_t frames, uint32_t rate);
        DspChunk(IMediaSample* pSample, const AM_SAMPLE2_PROPERTIES& sampleProps, const WAVEFORMATEX& sampleFormat);
        DspChunk(DspChunk&& other);
        DspChunk& operator=(DspChunk&& other);

        bool IsEmpty()             const { return m_dataSize == 0; }

        DspFormat GetFormat()      const { return m_format; }
        uint32_t GetFormatSize()   const { return m_formatSize; }
        uint32_t GetChannelCount() const { return m_channels; }
        uint32_t GetFrameSize()    const { return m_formatSize * m_channels; }
        uint32_t GetRate()         const { return m_rate; }

        size_t GetSize()           const { return m_dataSize; }
        size_t GetSampleCount()    const { assert(m_formatSize); return m_dataSize / m_formatSize; }
        size_t GetFrameCount()     const { assert(m_channels != 0); return GetSampleCount() / m_channels; }

        char* GetData() { return (m_mediaSample ? m_mediaData : m_data.get()) + m_dataOffset; }

        void PadTail(size_t padFrames);
        void PadHead(size_t padFrames);

        void ShrinkTail(size_t toFrames);
        void ShrinkHead(size_t toFrames);

        void FreeMediaSample();

    private:

        void Allocate();

        IMediaSamplePtr m_mediaSample;

        DspFormat m_format;
        uint32_t m_formatSize;
        uint32_t m_channels;
        uint32_t m_rate;

        size_t m_dataSize;
        char* m_mediaData;
        std::unique_ptr<char[], AlignedFreeDeleter> m_data;
        size_t m_dataOffset;
    };
}
