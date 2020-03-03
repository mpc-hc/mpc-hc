#include "pch.h"
#include "MyPin.h"

#include "AudioRenderer.h"

namespace SaneAudioRenderer
{
    MyPin::MyPin(AudioRenderer& renderer, CBaseFilter* pFilter, HRESULT& result)
        : CBaseInputPin(L"SaneAudioRenderer::MyPin", pFilter, this, &result, L"Input0")
        , m_bufferFilled(TRUE/*manual reset*/)
        , m_renderer(renderer)
    {
        if (FAILED(result))
            return;

        if (static_cast<HANDLE>(m_bufferFilled) == NULL)
            result = E_OUTOFMEMORY;
    }

    HRESULT MyPin::CheckMediaType(const CMediaType* pmt)
    {
        CheckPointer(pmt, E_POINTER);

        if (pmt->majortype == MEDIATYPE_Audio &&
            pmt->formattype == FORMAT_WaveFormatEx)
        {
            auto pFormat = reinterpret_cast<const WAVEFORMATEX*>(pmt->pbFormat);

            if (!pFormat ||
                pmt->cbFormat < sizeof(WAVEFORMATEX) ||
                pmt->cbFormat != sizeof(WAVEFORMATEX) + pFormat->cbSize)
            {
                return E_INVALIDARG;
            }

            try
            {
                if (m_renderer.CheckFormat(CopyWaveFormat(*pFormat), m_live))
                    return S_OK;
            }
            catch (std::bad_alloc&)
            {
                return E_OUTOFMEMORY;
            }
        }

        return S_FALSE;
    }

    HRESULT MyPin::SetMediaType(const CMediaType* pmt)
    {
        assert(CritCheckIn(this));

        ReturnIfFailed(CBaseInputPin::SetMediaType(pmt));

        auto pFormat = reinterpret_cast<const WAVEFORMATEX*>(pmt->pbFormat);

        // No point in doing integrity checks, that was done in CheckMediaType().
        assert(pFormat);
        assert(pmt->cbFormat == sizeof(WAVEFORMATEX) + pFormat->cbSize);

        m_live = CheckLive(m_Connected);

        try
        {
            m_renderer.SetFormat(CopyWaveFormat(*pFormat), m_live);
        }
        catch (std::bad_alloc&)
        {
            return E_OUTOFMEMORY;
        }

        return S_OK;
    }

    HRESULT MyPin::CheckConnect(IPin* pPin)
    {
        assert(CritCheckIn(this));

        ReturnIfFailed(CBaseInputPin::CheckConnect(pPin));

        m_live = CheckLive(pPin);

        return S_OK;
    }

    STDMETHODIMP MyPin::NewSegment(REFERENCE_TIME startTime, REFERENCE_TIME stopTime, double rate)
    {
        CAutoLock receiveLock(&m_receiveMutex);
        CAutoLock objectLock(this);

        CBaseInputPin::NewSegment(startTime, stopTime, rate);
        m_renderer.NewSegment(rate);

        return S_OK;
    }

    STDMETHODIMP MyPin::Receive(IMediaSample* pSample)
    {
        CAutoLock receiveLock(&m_receiveMutex);

        {
            CAutoLock objectLock(this);

            if (m_state == State_Stopped)
                return VFW_E_WRONG_STATE;

            ReturnIfNotEquals(S_OK, CBaseInputPin::Receive(pSample));

            if (m_SampleProps.dwSampleFlags & AM_SAMPLE_TYPECHANGED)
            {
                // TODO: don't recreate the device when possible
                m_renderer.Finish(false, &m_bufferFilled);
                ReturnIfFailed(SetMediaType(static_cast<CMediaType*>(m_SampleProps.pMediaType)));
            }

            if (m_eosUp)
                return S_FALSE;
        }

        // Raise Receive() thread priority, once.
        if (m_hReceiveThread != GetCurrentThread())
        {
            m_hReceiveThread = GetCurrentThread();
            if (GetThreadPriority(m_hReceiveThread) < THREAD_PRIORITY_ABOVE_NORMAL)
                SetThreadPriority(m_hReceiveThread, THREAD_PRIORITY_ABOVE_NORMAL);
        }

        // Push() returns 'false' in case of interruption.
        return m_renderer.Push(pSample, m_SampleProps, &m_bufferFilled) ? S_OK : S_FALSE;
    }

    STDMETHODIMP MyPin::EndOfStream()
    {
        CAutoLock receiveLock(&m_receiveMutex);

        {
            CAutoLock objectLock(this);

            if (m_state == State_Stopped)
                return VFW_E_WRONG_STATE;

            if (m_bFlushing)
                return S_FALSE;

            m_eosUp = true;
        }

        // We ask audio renderer to block until all samples are played.
        // Finish() returns 'false' in case of interruption.
        bool eosDown = m_renderer.Finish(true, &m_bufferFilled);

        {
            CAutoLock objectLock(this);

            m_eosDown = eosDown;

            if (m_eosDown)
                m_pFilter->NotifyEvent(EC_COMPLETE, S_OK, (LONG_PTR)m_pFilter);
        }

        return S_OK;
    }

    STDMETHODIMP MyPin::BeginFlush()
    {
        // Parent method locks the object before modifying it, all is good.
        CBaseInputPin::BeginFlush();

        m_renderer.BeginFlush();

        // Barrier for any present Receive() and EndOfStream() calls.
        // Subsequent ones will be rejected because m_bFlushing == TRUE.
        CAutoLock receiveLock(&m_receiveMutex);

        m_bufferFilled.Reset();

        {
            CAutoLock objectLock(this);

            m_eosUp = false;
            m_eosDown = false;
        }

        m_hReceiveThread = NULL;

        m_renderer.EndFlush();

        return S_OK;
    }

    STDMETHODIMP MyPin::EndFlush()
    {
        // Parent method locks the object before modifying it, all is good.
        CBaseInputPin::EndFlush();

        return S_OK;
    }

    HRESULT MyPin::Active()
    {
        CAutoLock objectLock(this);

        assert(m_state != State_Paused);
        m_state = State_Paused;

        if (IsConnected())
        {
            m_renderer.Pause();
        }
        else
        {
            m_eosUp = true;
            m_eosDown = true;
        }

        return S_OK;
    }

    HRESULT MyPin::Run(REFERENCE_TIME startTime)
    {
        CAutoLock objectLock(this);

        assert(m_state == State_Paused);
        m_state = State_Running;

        if (m_eosDown)
        {
            m_pFilter->NotifyEvent(EC_COMPLETE, S_OK, (LONG_PTR)m_pFilter);
        }
        else
        {
            assert(IsConnected());
            m_renderer.Play(startTime);
        }

        return S_OK;
    }

    HRESULT MyPin::Inactive()
    {
        {
            CAutoLock objectLock(this);

            assert(m_state != State_Stopped);
            m_state = State_Stopped;

            CBaseInputPin::Inactive();
        }

        m_renderer.BeginFlush();

        // Barrier for any present Receive() and EndOfStream() calls.
        // Subsequent ones will be rejected because m_state == State_Stopped.
        CAutoLock receiveLock(&m_receiveMutex);

        m_bufferFilled.Reset();

        {
            CAutoLock objectLock(this);

            m_eosUp = false;
            m_eosDown = false;
        }

        m_hReceiveThread = NULL;

        m_renderer.Stop();
        m_renderer.EndFlush();

        return S_OK;
    }

    bool MyPin::StateTransitionFinished(uint32_t timeoutMilliseconds)
    {
        {
            CAutoLock objectLock(this);

            if (!IsConnected() || m_state != State_Paused)
                return true;
        }

        // Don't lock the object, we don't want to block Receive() method.

        // There won't be any state transitions during the wait,
        // because MyFilter always locks itself before calling this method.

        return !!m_bufferFilled.Wait(timeoutMilliseconds);
    }

    bool MyPin::CheckLive(IPin* pPin)
    {
        assert(pPin);

        bool live = false;

        IAMGraphStreamsPtr graphStreams;
        IAMPushSourcePtr pushSource;
        if (SUCCEEDED(m_pFilter->GetFilterGraph()->QueryInterface(IID_PPV_ARGS(&graphStreams))) &&
            SUCCEEDED(graphStreams->FindUpstreamInterface(pPin, IID_PPV_ARGS(&pushSource), AM_INTF_SEARCH_OUTPUT_PIN)))
        {
            live = true;

            ULONG flags;
            if (SUCCEEDED(pushSource->GetPushSourceFlags(&flags)))
            {
                if (flags & AM_PUSHSOURCECAPS_INTERNAL_RM)
                    DebugOut(ClassName(this), "upstream live pin has AM_PUSHSOURCECAPS_INTERNAL_RM flag");

                if (flags & AM_PUSHSOURCECAPS_NOT_LIVE)
                {
                    DebugOut(ClassName(this), "upstream live pin has AM_PUSHSOURCECAPS_NOT_LIVE flag");
                    live = false;
                }

                if (flags & AM_PUSHSOURCECAPS_PRIVATE_CLOCK)
                    DebugOut(ClassName(this), "upstream live pin has AM_PUSHSOURCECAPS_PRIVATE_CLOCK flag");

                if (flags & AM_PUSHSOURCEREQS_USE_STREAM_CLOCK)
                    DebugOut(ClassName(this), "upstream live pin has AM_PUSHSOURCEREQS_USE_STREAM_CLOCK flag");

                if (!flags)
                    DebugOut(ClassName(this), "upstream live pin has no flags");
            }
        }

        return live;
    }
}
