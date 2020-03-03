#include "pch.h"
#include "DspLimiter.h"

namespace SaneAudioRenderer
{
    namespace
    {
        const float slope = 1.0f - 1.0f / 20.0f; // 20:1 ratio

        template <typename T>
        T GetPeak(const T* data, size_t n)
        {
            T peak = 0;

            for (size_t i = 0; i < n; i++)
                peak = std::max(peak, std::abs(data[i]));

            return peak;
        }

        template <typename T>
        void ApplyLimiter(T* data, size_t n, T threshold)
        {
            for (size_t i = 0; i < n; i++)
            {
                T& sample = data[i];
                const T absSample = std::abs(sample);

                if (absSample > threshold)
                    sample *= std::pow(threshold / absSample, slope);

                assert(std::abs(sample) <= 1);
            }
        }
    }

    void DspLimiter::Initialize(uint32_t rate, uint32_t channels, bool exclusive)
    {
        m_exclusive = exclusive;
        m_rate = rate;
        m_channels = channels;

        m_active = false;
        m_holdWindow = 0;
        m_peak = 0.0f;
        m_threshold = 0.0f;
    }

    bool DspLimiter::Active()
    {
        return m_active;
    }

    void DspLimiter::Process(DspChunk& chunk)
    {
        if (chunk.IsEmpty())
            return;

        if (!m_exclusive || (chunk.GetFormat() != DspFormat::Float &&
                             chunk.GetFormat() != DspFormat::Double))
        {
            m_active = false;
            return;
        }

        m_active = true;

        // Analyze samples
        float peak;
        if (chunk.GetFormat() == DspFormat::Double)
        {
            double largePeak = GetPeak((double*)chunk.GetData(), chunk.GetSampleCount());
            peak = std::nexttoward((float)largePeak, largePeak);
        }
        else
        {
            assert(chunk.GetFormat() == DspFormat::Float);
            peak = GetPeak((float*)chunk.GetData(), chunk.GetSampleCount());
        }

        // Configure limiter
        if (peak > 1.0f)
        {
            if (m_holdWindow <= 0)
            {
                NewTreshold(std::max(peak, 1.4f));
            }
            else if (peak > m_peak)
            {
                NewTreshold(peak);
            }

            m_holdWindow = (int64_t)m_rate * m_channels * 10; // 10 seconds
        }

        // Apply limiter
        if (m_holdWindow > 0)
        {
            if (chunk.GetFormat() == DspFormat::Double)
            {
                ApplyLimiter<double>((double*)chunk.GetData(), chunk.GetSampleCount(), m_threshold);
            }
            else
            {
                assert(chunk.GetFormat() == DspFormat::Float);
                ApplyLimiter((float*)chunk.GetData(), chunk.GetSampleCount(), m_threshold);
            }

            m_holdWindow -= chunk.GetSampleCount();
        }
    }

    void DspLimiter::Finish(DspChunk& chunk)
    {
        Process(chunk);
    }

    void DspLimiter::NewTreshold(float peak)
    {
        m_peak = peak;
        m_threshold = std::pow(1.0f / peak, 1.0f / slope - 1.0f) - 0.0001f;
        DebugOut(ClassName(this), "active with", m_peak, "peak and", m_threshold, "threshold");
    }
}
