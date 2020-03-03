#pragma once

namespace SaneAudioRenderer
{
    class AudioRenderer;

    class MyPin final
        : public CCritSec
        , public CBaseInputPin
    {
    public:

        MyPin(AudioRenderer& renderer, CBaseFilter* pFilter, HRESULT& result);
        MyPin(const MyPin&) = delete;
        MyPin& operator=(const MyPin&) = delete;

        HRESULT CheckMediaType(const CMediaType* pmt) override;
        HRESULT SetMediaType(const CMediaType* pmt) override;
        HRESULT CheckConnect(IPin* pPin) override;

        STDMETHODIMP NewSegment(REFERENCE_TIME startTime, REFERENCE_TIME stopTime, double rate) override;
        STDMETHODIMP ReceiveCanBlock() override { return S_OK; }
        STDMETHODIMP Receive(IMediaSample* pSample) override;
        STDMETHODIMP EndOfStream() override;

        STDMETHODIMP BeginFlush() override;
        STDMETHODIMP EndFlush() override;

        HRESULT Active() override;
        HRESULT Run(REFERENCE_TIME startTime) override;
        HRESULT Inactive() override;

        bool StateTransitionFinished(uint32_t timeoutMilliseconds);

    private:

        bool CheckLive(IPin* pPin);

        FILTER_STATE m_state = State_Stopped;
        bool m_eosUp = false;
        bool m_eosDown = false;

        bool m_live = false;

        CCritSec m_receiveMutex;
        HANDLE m_hReceiveThread = NULL;

        CAMEvent m_bufferFilled;
        AudioRenderer& m_renderer;
    };
}
