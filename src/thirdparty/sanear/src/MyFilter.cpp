#include "pch.h"
#include "MyFilter.h"

#include "AudioRenderer.h"
#include "Factory.h"
#include "MyBasicAudio.h"
#include "MyClock.h"
#include "MyTestClock.h"
#include "MyPin.h"

namespace SaneAudioRenderer
{
    MyFilter::MyFilter(IUnknown* pUnknown, REFIID guid)
        : CBaseFilter(L"SaneAudioRenderer::MyFilter", pUnknown, this, guid)
        , m_bufferFilled(TRUE/*manual reset*/)
    {
    }

    HRESULT MyFilter::Init(ISettings* pSettings)
    {
        HRESULT result = S_OK;

        try
        {
            if (SUCCEEDED(result))
                m_clock = std::make_unique<MyClock>(GetOwner(), m_renderer, result);

            //if (SUCCEEDED(result))
            //    m_testClock = new MyTestClock(nullptr, result);

            if (SUCCEEDED(result))
                m_renderer = std::make_unique<AudioRenderer>(pSettings, *m_clock, result);

            if (SUCCEEDED(result))
                m_basicAudio = std::make_unique<MyBasicAudio>(GetOwner(), *m_renderer);

            if (SUCCEEDED(result))
                m_pin = std::make_unique<MyPin>(*m_renderer, this, result);

            if (SUCCEEDED(result))
                result = CreatePosPassThru(GetOwner(), FALSE, m_pin.get(), &m_seeking);
        }
        catch (std::bad_alloc&)
        {
            result = E_OUTOFMEMORY;
        }

        return result;
    }

    STDMETHODIMP MyFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
    {
        if (riid == IID_IUnknown)
            return CUnknown::NonDelegatingQueryInterface(riid, ppv);

        //if (riid == IID_IReferenceClock || riid == IID_IReferenceClockTimerControl)
        //    return m_testClock->QueryInterface(riid, ppv);

        if (riid == IID_IReferenceClock || riid == IID_IReferenceClockTimerControl)
            return m_clock->NonDelegatingQueryInterface(riid, ppv);

        if (riid == IID_IBasicAudio)
            return m_basicAudio->NonDelegatingQueryInterface(riid, ppv);

        if (riid == IID_IMediaSeeking)
            return m_seeking->QueryInterface(riid, ppv);

        if (riid == __uuidof(ISpecifyPropertyPages2))
            return GetInterface(static_cast<ISpecifyPropertyPages2*>(this), ppv);

        if (riid == IID_ISpecifyPropertyPages)
            return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);

        if (riid == __uuidof(IStatusPageData))
            return GetInterface(static_cast<IStatusPageData*>(this), ppv);

        return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }

    int MyFilter::GetPinCount()
    {
        return 1;
    }

    CBasePin* MyFilter::GetPin(int n)
    {
        return (n == 0) ? m_pin.get() : nullptr;
    }

    STDMETHODIMP MyFilter::Stop()
    {
        return ChangeState<State_Stopped>(std::bind(&MyPin::Inactive, m_pin.get()));
    }

    STDMETHODIMP MyFilter::Pause()
    {
        return ChangeState<State_Paused>(std::bind(&MyPin::Active, m_pin.get()));
    }

    STDMETHODIMP MyFilter::Run(REFERENCE_TIME startTime)
    {
        return ChangeState<State_Running>(std::bind(&MyPin::Run, m_pin.get(), startTime));
    }

    STDMETHODIMP MyFilter::GetState(DWORD timeoutMilliseconds, FILTER_STATE* pState)
    {
        CheckPointer(pState, E_POINTER);

        CAutoLock objectLock(this);

        *pState = m_State;

        if (!m_pin->StateTransitionFinished(timeoutMilliseconds))
            return VFW_S_STATE_INTERMEDIATE;

        return S_OK;
    }

    STDMETHODIMP MyFilter::SetSyncSource(IReferenceClock* pClock)
    {
        CAutoLock objectLock(this);

        CBaseFilter::SetSyncSource(pClock);

        m_renderer->SetClock(pClock);

        return S_OK;
    }

    STDMETHODIMP MyFilter::GetPages(CAUUID* pPages)
    {
        CheckPointer(pPages, E_POINTER);

        if (m_pin->IsConnected())
        {
            pPages->cElems = 1;
            pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
            CheckPointer(pPages->pElems, E_OUTOFMEMORY);
            pPages->pElems[0] = __uuidof(MyPropertyPage);
        }
        else
        {
            pPages->cElems = 0;
            pPages->pElems = nullptr;
        }

        return S_OK;
    }

    STDMETHODIMP MyFilter::CreatePage(const GUID& guid, IPropertyPage** ppPage)
    {
        CheckPointer(ppPage, E_POINTER);

        if (guid != __uuidof(MyPropertyPage))
            return E_UNEXPECTED;

        MyPropertyPage* pPage;

        HRESULT result;

        pPage = new(std::nothrow) MyPropertyPage(result, this);

        CheckPointer(pPage, E_OUTOFMEMORY);

        pPage->AddRef();

        if (SUCCEEDED(result))
            result = pPage->QueryInterface(IID_PPV_ARGS(ppPage));

        pPage->Release();

        return result;
    }

    STDMETHODIMP MyFilter::GetPageData(bool resize, std::vector<char>& data)
    {
        try
        {
            CAutoLock rendererLock(m_renderer.get());

            auto inputFormat = m_renderer->GetInputFormat();
            auto audioDevice = m_renderer->GetAudioDevice();

            data = MyPropertyPage::CreateDialogData(resize, inputFormat, audioDevice,
                                                    m_renderer->GetActiveProcessors(),
                                                    m_renderer->OnExternalClock(),
                                                    m_renderer->IsLive(),
                                                    m_renderer->OnGuidedReclock());
        }
        catch (std::bad_alloc&)
        {
            return E_OUTOFMEMORY;
        }

        return S_OK;
    }

    template <FILTER_STATE NewState, typename PinFunction>
    STDMETHODIMP MyFilter::ChangeState(PinFunction pinFunction)
    {
        CAutoLock objectLock(this);

        if (m_State != NewState)
        {
            ReturnIfFailed(pinFunction());
            m_State = NewState;
        }

        if (!m_pin->StateTransitionFinished(0))
            return S_FALSE;

        return S_OK;
    }
}
