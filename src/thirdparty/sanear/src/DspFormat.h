#pragma once

namespace SaneAudioRenderer
{
    enum class DspFormat
    {
        Unknown,
        Pcm8,
        Pcm16,
        Pcm24,
        Pcm24in32,
        Pcm32,
        Float,
        Double,
    };

    template <DspFormat OutputFormat>
    struct DspFormatTraits;

    template <>
    struct DspFormatTraits<DspFormat::Pcm8>
    {
        typedef int8_t SampleType;
    };

    template <>
    struct DspFormatTraits<DspFormat::Pcm16>
    {
        typedef int16_t SampleType;
    };

    #pragma pack(push, 1)
    typedef struct { int8_t d[3]; } int24_t;
    #pragma pack(pop)

    static_assert(sizeof(int24_t) == 3, "Failed to pack the struct properly");

    template <>
    struct DspFormatTraits<DspFormat::Pcm24>
    {
        typedef int24_t SampleType;
    };

    template <>
    struct DspFormatTraits<DspFormat::Pcm24in32>
    {
        typedef int32_t SampleType;
    };

    template <>
    struct DspFormatTraits<DspFormat::Pcm32>
    {
        typedef int32_t SampleType;
    };

    template <>
    struct DspFormatTraits<DspFormat::Float>
    {
        typedef float SampleType;
    };

    template <>
    struct DspFormatTraits<DspFormat::Double>
    {
        typedef double SampleType;
    };

    static_assert(sizeof(float) == 4, "Floats are not IEEE compliant");
    static_assert(sizeof(double) == 8, "Floats are not IEEE compliant");

    inline uint32_t DspFormatSize(DspFormat format)
    {
        return (format == DspFormat::Unknown) ? 0 :
               (format == DspFormat::Pcm8) ? 1 :
               (format == DspFormat::Pcm16) ? 2 :
               (format == DspFormat::Pcm24) ? 3 :
               (format == DspFormat::Double) ? 8 : 4;
    }

    inline DspFormat DspFormatFromWaveFormat(const WAVEFORMATEX& format)
    {
        if (format.nSamplesPerSec == 0)
            return DspFormat::Unknown;

        if (format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
        {
            switch (format.wBitsPerSample)
            {
                case 32: return DspFormat::Float;
                case 64: return DspFormat::Double;
            }
        }
        else if (format.wFormatTag == WAVE_FORMAT_PCM)
        {
            switch (format.wBitsPerSample)
            {
                case 8:  return DspFormat::Pcm8;
                case 16: return DspFormat::Pcm16;
                case 24: return DspFormat::Pcm24;
                case 32: return DspFormat::Pcm32;
            }
        }
        else if (format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
        {
            const WAVEFORMATEXTENSIBLE& formatExtensible = (const WAVEFORMATEXTENSIBLE&)format;

            if (formatExtensible.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
            {
                switch (format.wBitsPerSample)
                {
                    case 32: return DspFormat::Float;
                    case 64: return DspFormat::Double;
                }
            }
            else if (formatExtensible.SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
            {
                switch (format.wBitsPerSample)
                {
                    case 8:  return DspFormat::Pcm8;
                    case 16: return DspFormat::Pcm16;
                    case 24: return DspFormat::Pcm24;
                    case 32: return formatExtensible.Samples.wValidBitsPerSample == 24 ? DspFormat::Pcm24in32 :
                                                                                         DspFormat::Pcm32;
                }
            }
        }

        return DspFormat::Unknown;
    }
}
