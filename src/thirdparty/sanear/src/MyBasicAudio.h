#pragma once

namespace SaneAudioRenderer
{
    class AudioRenderer;

    class MyBasicAudio final
        : public CBasicAudio
    {
    public:

        MyBasicAudio(IUnknown* pUnknown, AudioRenderer& renderer);
        MyBasicAudio(const MyBasicAudio&) = delete;
        MyBasicAudio& operator=(const MyBasicAudio&) = delete;

        STDMETHODIMP put_Volume(long volume) override;
        STDMETHODIMP get_Volume(long* pVolume) override;
        STDMETHODIMP put_Balance(long balance) override;
        STDMETHODIMP get_Balance(long* pBalance) override;

    private:

        AudioRenderer& m_renderer;
    };
}
