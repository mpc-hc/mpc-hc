#include "pch.h"
#include "MyBasicAudio.h"

#include "AudioRenderer.h"

namespace SaneAudioRenderer
{
    MyBasicAudio::MyBasicAudio(IUnknown* pUnknown, AudioRenderer& renderer)
        : CBasicAudio(L"SaneAudioRenderer::MyBasicAudio", pUnknown)
        , m_renderer(renderer)
    {
    }

    STDMETHODIMP MyBasicAudio::put_Volume(long volume)
    {
        if (volume < -10000 || volume > 0)
            return E_FAIL;

        float f = (volume == 0) ?
                      1.0f : pow(10.0f, (float)volume / 2000.0f);

        m_renderer.SetVolume(f);

        return S_OK;
    }

    STDMETHODIMP MyBasicAudio::get_Volume(long* pVolume)
    {
        CheckPointer(pVolume, E_POINTER);

        float f = m_renderer.GetVolume();

        *pVolume = (f == 1.0f) ?
                       0 : (long)(log10(f) * 2000.0f);

        assert(*pVolume <= 0 && *pVolume >= -10000);

        return S_OK;
    }

    STDMETHODIMP MyBasicAudio::put_Balance(long balance)
    {
        if (balance < -10000 || balance > 10000)
            return E_FAIL;

        float f = (balance == 0) ?
                      0.0f : pow(10.0f, (float)abs(balance) / -2000.0f);

        m_renderer.SetBalance(copysign(f, (float)balance));

        return S_OK;
    }

    STDMETHODIMP MyBasicAudio::get_Balance(long* pBalance)
    {
        CheckPointer(pBalance, E_POINTER);

        float f = m_renderer.GetBalance();

        *pBalance = (f == 0.0f) ?
                        0 : (long)(copysign(log10(abs(f)), f) * 2000.0f);

        assert(*pBalance >= -10000 && *pBalance <= 10000);

        return S_OK;
    }
}
