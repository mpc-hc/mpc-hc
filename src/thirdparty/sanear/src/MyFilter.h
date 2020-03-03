#pragma once

#include "Interfaces.h"
#include "MyPropertyPage.h"

namespace SaneAudioRenderer
{
    class MyClock;
    class AudioRenderer;
    class MyBasicAudio;
    class MyPin;

    class MyFilter final
        : public CCritSec
        , public CBaseFilter
        , public ISpecifyPropertyPages2
        , public IStatusPageData
    {
    public:

        MyFilter(IUnknown* pUnknown, REFIID guid);
        MyFilter(const MyFilter&) = delete;
        MyFilter& operator=(const MyFilter&) = delete;

        HRESULT Init(ISettings* pSettings);

        DECLARE_IUNKNOWN

        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;

        int GetPinCount() override;
        CBasePin* GetPin(int n) override;

        STDMETHODIMP Stop() override;
        STDMETHODIMP Pause() override;
        STDMETHODIMP Run(REFERENCE_TIME startTime) override;

        STDMETHODIMP GetState(DWORD timeoutMilliseconds, FILTER_STATE* pState) override;

        STDMETHODIMP SetSyncSource(IReferenceClock* pClock) override;

        STDMETHODIMP GetPages(CAUUID* pPages) override;
        STDMETHODIMP CreatePage(const GUID& guid, IPropertyPage** ppPage) override;

        STDMETHODIMP GetPageData(bool resize, std::vector<char>& data) override;

    private:

        template <FILTER_STATE NewState, typename PinFunction>
        STDMETHODIMP ChangeState(PinFunction pinFunction);

        std::unique_ptr<MyClock> m_clock;
        //IReferenceClockPtr m_testClock;
        CAMEvent m_bufferFilled;
        std::unique_ptr<AudioRenderer> m_renderer;
        std::unique_ptr<MyBasicAudio> m_basicAudio;
        std::unique_ptr<MyPin> m_pin;
        IUnknownPtr m_seeking;
    };
}
