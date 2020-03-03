#include "pch.h"
#include "DspMatrix.h"

namespace SaneAudioRenderer
{
    namespace
    {
        const std::array<DWORD, 18> Channels = {
            SPEAKER_FRONT_LEFT,
            SPEAKER_FRONT_RIGHT,
            SPEAKER_FRONT_CENTER,
            SPEAKER_LOW_FREQUENCY,
            SPEAKER_BACK_LEFT,
            SPEAKER_BACK_RIGHT,
            SPEAKER_FRONT_LEFT_OF_CENTER,
            SPEAKER_FRONT_RIGHT_OF_CENTER,
            SPEAKER_BACK_CENTER,
            SPEAKER_SIDE_LEFT,
            SPEAKER_SIDE_RIGHT,
            SPEAKER_TOP_CENTER,
            SPEAKER_TOP_FRONT_LEFT,
            SPEAKER_TOP_FRONT_CENTER,
            SPEAKER_TOP_FRONT_RIGHT,
            SPEAKER_TOP_BACK_LEFT,
            SPEAKER_TOP_BACK_CENTER,
            SPEAKER_TOP_BACK_RIGHT
        };

        size_t IndexForChannel(DWORD channel)
        {
            switch (channel)
            {
                case SPEAKER_FRONT_LEFT:            return 0;
                case SPEAKER_FRONT_RIGHT:           return 1;
                case SPEAKER_FRONT_CENTER:          return 2;
                case SPEAKER_LOW_FREQUENCY:         return 3;
                case SPEAKER_BACK_LEFT:             return 4;
                case SPEAKER_BACK_RIGHT:            return 5;
                case SPEAKER_FRONT_LEFT_OF_CENTER:  return 6;
                case SPEAKER_FRONT_RIGHT_OF_CENTER: return 7;
                case SPEAKER_BACK_CENTER:           return 8;
                case SPEAKER_SIDE_LEFT:             return 9;
                case SPEAKER_SIDE_RIGHT:            return 10;
                case SPEAKER_TOP_CENTER:            return 11;
                case SPEAKER_TOP_FRONT_LEFT:        return 12;
                case SPEAKER_TOP_FRONT_CENTER:      return 13;
                case SPEAKER_TOP_FRONT_RIGHT:       return 14;
                case SPEAKER_TOP_BACK_LEFT:         return 15;
                case SPEAKER_TOP_BACK_CENTER:       return 16;
                case SPEAKER_TOP_BACK_RIGHT:        return 17;
            }

            throw std::logic_error("");
        }

        std::array<float, 18 * 18> BuildFullMatrix(DWORD inputMask, DWORD outputMask)
        {
            const float fullPower = 1.0f;
            const float halfPower = 0.707113564f;

            std::array<float, 18 * 18> matrix{};

            for (auto& c : Channels)
            {
                if (inputMask & c)
                    matrix[18 * IndexForChannel(c) + IndexForChannel(c)] = 1.0f;
            }

            auto feed = [&](DWORD sourceChannel, DWORD targetChannel, float multiplier)
            {
                float* source = matrix.data() + 18 * IndexForChannel(sourceChannel);
                float* target = matrix.data() + 18 * IndexForChannel(targetChannel);
                for (int i = 0; i < 18; i++)
                    target[i] += source[i] * multiplier;
            };

            auto clear = [&](DWORD targetChannel)
            {
                float* target = matrix.data() + 18 * IndexForChannel(targetChannel);
                for (int i = 0; i < 18; i++)
                    target[i] = 0.0f;
            };

            // Mix side
            {
                if (!(outputMask & SPEAKER_SIDE_LEFT))
                {
                    feed(SPEAKER_SIDE_LEFT, SPEAKER_BACK_LEFT, fullPower);
                    clear(SPEAKER_SIDE_LEFT);
                }

                if (!(outputMask & SPEAKER_SIDE_RIGHT))
                {
                    feed(SPEAKER_SIDE_RIGHT, SPEAKER_BACK_RIGHT, fullPower);
                    clear(SPEAKER_SIDE_RIGHT);
                }
            }

            // Mix back
            {
                if (!(outputMask & SPEAKER_BACK_CENTER))
                {
                    feed(SPEAKER_BACK_CENTER, SPEAKER_BACK_LEFT, halfPower);
                    feed(SPEAKER_BACK_CENTER, SPEAKER_BACK_RIGHT, halfPower);
                    clear(SPEAKER_BACK_CENTER);
                }

                if (!(outputMask & SPEAKER_BACK_LEFT))
                {
                    if (outputMask & SPEAKER_BACK_CENTER)
                    {
                        feed(SPEAKER_BACK_LEFT, SPEAKER_BACK_CENTER, fullPower);
                    }
                    else if (outputMask & SPEAKER_SIDE_LEFT)
                    {
                        feed(SPEAKER_BACK_LEFT, SPEAKER_SIDE_LEFT, fullPower);
                    }
                    else
                    {
                        feed(SPEAKER_BACK_LEFT, SPEAKER_FRONT_LEFT, halfPower);
                    }

                    clear(SPEAKER_BACK_LEFT);
                }

                if (!(outputMask & SPEAKER_BACK_RIGHT))
                {
                    if (outputMask & SPEAKER_BACK_CENTER)
                    {
                        feed(SPEAKER_BACK_RIGHT, SPEAKER_BACK_CENTER, fullPower);
                    }
                    else if (outputMask & SPEAKER_SIDE_RIGHT)
                    {
                        feed(SPEAKER_BACK_RIGHT, SPEAKER_SIDE_RIGHT, fullPower);
                    }
                    else
                    {
                        feed(SPEAKER_BACK_RIGHT, SPEAKER_FRONT_RIGHT, halfPower);
                    }

                    clear(SPEAKER_BACK_RIGHT);
                }
            }

            // Mix front
            {
                if (!(outputMask & SPEAKER_FRONT_CENTER))
                {
                    feed(SPEAKER_FRONT_CENTER, SPEAKER_FRONT_LEFT, halfPower);
                    feed(SPEAKER_FRONT_CENTER, SPEAKER_FRONT_RIGHT, halfPower);
                    clear(SPEAKER_FRONT_CENTER);
                }

                if (!(outputMask & SPEAKER_FRONT_LEFT) && (outputMask & SPEAKER_FRONT_CENTER))
                {
                    feed(SPEAKER_FRONT_LEFT, SPEAKER_FRONT_CENTER, halfPower);
                    clear(SPEAKER_FRONT_LEFT);
                }

                if (!(outputMask & SPEAKER_FRONT_RIGHT) && (outputMask & SPEAKER_FRONT_CENTER))
                {
                    feed(SPEAKER_FRONT_RIGHT, SPEAKER_FRONT_CENTER, halfPower);
                    clear(SPEAKER_FRONT_RIGHT);
                }
            }

            return matrix;
        }

        std::array<float, 18 * 18> BuildMatrix(size_t inputChannels, DWORD inputMask,
                                               size_t outputChannels, DWORD outputMask)
        {
            std::array<float, 18 * 18> fullMatrix = BuildFullMatrix(inputMask, outputMask);
            std::array<float, 18 * 18> matrix{};

            size_t y = 0;
            for (auto& yc : Channels)
            {
                if (outputMask & yc)
                {
                    size_t x = 0;
                    for (auto& xc : Channels)
                    {
                        if (inputMask & xc)
                        {
                            matrix[y * inputChannels + x] = fullMatrix[IndexForChannel(yc) * 18 +
                                                                       IndexForChannel(xc)];

                            if (++x == inputChannels)
                                break;
                        }
                    }

                    if (++y == outputChannels)
                        break;
                }
            }

            return matrix;
        }

        template <size_t InputChannels, size_t OutputChannels>
        void Mix(const float* inputData, float* outputData, const float* matrix, size_t frames)
        {
            for (size_t frame = 0; frame < frames; frame++)
            {
                for (size_t y = 0; y < OutputChannels; y++)
                {
                    float d = 0.0f;

                    for (size_t x = 0; x < InputChannels; x++)
                    {
                        d += inputData[frame * InputChannels + x] * matrix[y * InputChannels + x];
                    }

                    outputData[frame * OutputChannels + y] = d;
                }
            }
        }

        void Mix(size_t inputChannels, const float* inputData, size_t outputChannels, float* outputData,
                 const float* matrix, size_t frames)
        {
            for (size_t frame = 0; frame < frames; frame++)
            {
                for (size_t y = 0; y < outputChannels; y++)
                {
                    float d = 0.0f;

                    for (size_t x = 0; x < inputChannels; x++)
                    {
                        d += inputData[frame * inputChannels + x] * matrix[y * inputChannels + x];
                    }

                    outputData[frame * outputChannels + y] = d;
                }
            }
        }
    }

    void DspMatrix::Initialize(uint32_t inputChannels, DWORD inputMask,
                               uint32_t outputChannels, DWORD outputMask)
    {
        m_active = false;

        if (inputChannels != outputChannels || inputMask != outputMask)
        {
            m_matrix = BuildMatrix(inputChannels, inputMask, outputChannels, outputMask);

            if (inputChannels != outputChannels)
            {
                m_active = true;
            }
            else
            {
                // Redundancy check.
                for (size_t y = 0; y < outputChannels; y++)
                {
                    for (size_t x = 0; x < inputChannels; x++)
                    {
                        float d = m_matrix[y * inputChannels + x];

                        if ((x == y && d != 1.0f) ||
                            (x != y && d != 0.0f))
                        {
                            m_active = true;
                        }
                    }
                }
            }
        }

        m_inputChannels = inputChannels;
        m_outputChannels = outputChannels;
    }

    bool DspMatrix::Active()
    {
        return m_active;
    }

    void DspMatrix::Process(DspChunk& chunk)
    {
        if (!m_active || chunk.IsEmpty())
            return;

        assert(chunk.GetChannelCount() == m_inputChannels);

        DspChunk::ToFloat(chunk);

        DspChunk output(DspFormat::Float, m_outputChannels, chunk.GetFrameCount(), chunk.GetRate());

        auto inputData = reinterpret_cast<const float*>(chunk.GetData());
        auto outputData = reinterpret_cast<float*>(output.GetData());

        if (m_inputChannels == 6 && m_outputChannels == 2)
        {
            Mix<6, 2>(inputData, outputData, m_matrix.data(), chunk.GetFrameCount());
        }
        else if (m_inputChannels == 7 && m_outputChannels == 2)
        {
            Mix<7, 2>(inputData, outputData, m_matrix.data(), chunk.GetFrameCount());
        }
        else if (m_inputChannels == 8 && m_outputChannels == 2)
        {
            Mix<8, 2>(inputData, outputData, m_matrix.data(), chunk.GetFrameCount());
        }
        else
        {
            Mix(m_inputChannels, inputData, m_outputChannels, outputData, m_matrix.data(), chunk.GetFrameCount());
        }

        chunk = std::move(output);
    }

    void DspMatrix::Finish(DspChunk& chunk)
    {
        Process(chunk);
    }

    DWORD DspMatrix::GetChannelMask(const WAVEFORMATEX& format)
    {
        if (format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
            return reinterpret_cast<const WAVEFORMATEXTENSIBLE&>(format).dwChannelMask;

        switch (format.nChannels)
        {
            case 1:
                return KSAUDIO_SPEAKER_MONO;

            case 2:
                return KSAUDIO_SPEAKER_STEREO;

            case 3:
                return KSAUDIO_SPEAKER_STEREO | SPEAKER_FRONT_CENTER;

            case 4:
                return KSAUDIO_SPEAKER_SURROUND;

            case 5:
                return KSAUDIO_SPEAKER_QUAD | SPEAKER_FRONT_CENTER;

            case 6:
                return KSAUDIO_SPEAKER_5POINT1;

            case 7:
                return KSAUDIO_SPEAKER_5POINT1 | SPEAKER_BACK_CENTER;

            case 8:
                return KSAUDIO_SPEAKER_7POINT1_SURROUND;

            default:
                return 0;
        }
    }

    bool DspMatrix::IsStereoFormat(const WAVEFORMATEX& format)
    {
        return format.nChannels == 2 && GetChannelMask(format) == KSAUDIO_SPEAKER_STEREO;
    }
}
