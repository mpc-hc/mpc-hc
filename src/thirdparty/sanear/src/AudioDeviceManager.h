#pragma once

#include "AudioDevice.h"
#include "Interfaces.h"

namespace SaneAudioRenderer
{
    class AudioDeviceNotificationClient final
        : public CUnknown
        , public IMMNotificationClient
    {
    public:

        DECLARE_IUNKNOWN

        AudioDeviceNotificationClient(std::atomic<uint32_t>& defaultDeviceSerial);
        AudioDeviceNotificationClient(const AudioDeviceNotificationClient&) = delete;
        AudioDeviceNotificationClient& operator=(const AudioDeviceNotificationClient&) = delete;

        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;

        STDMETHODIMP OnDeviceStateChanged(LPCWSTR, DWORD) override { return S_OK; }
        STDMETHODIMP OnDeviceAdded(LPCWSTR) override { return S_OK; }
        STDMETHODIMP OnDeviceRemoved(LPCWSTR) override { return S_OK; }
        STDMETHODIMP OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR);
        STDMETHODIMP OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY) override { return S_OK; }

    private:

        std::atomic<uint32_t>& m_defaultDeviceSerial;
    };

    class AudioDeviceManager final
    {
    public:

        AudioDeviceManager(HRESULT& result);
        AudioDeviceManager(const AudioDeviceManager&) = delete;
        AudioDeviceManager& operator=(const AudioDeviceManager&) = delete;
        ~AudioDeviceManager();

        bool BitstreamFormatSupported(SharedWaveFormat format, ISettings* pSettings);
        std::unique_ptr<AudioDevice> CreateDevice(SharedWaveFormat format, bool realtime, ISettings* pSettings);
        bool RenewInactiveDevice(AudioDevice& device, int64_t& position);

        uint32_t GetDefaultDeviceSerial() { return m_defaultDeviceSerial; }
        std::unique_ptr<WCHAR, CoTaskMemFreeDeleter> GetDefaultDeviceId();

    private:

        std::thread m_thread;
        std::atomic<bool> m_exit = false;
        CAMEvent m_wake;
        CAMEvent m_done;

        std::function<HRESULT(void)> m_function;
        HRESULT m_result = S_OK;

        IMMDeviceEnumeratorPtr m_enumerator;

        IMMNotificationClientPtr m_notificationClient;
        std::atomic<uint32_t> m_defaultDeviceSerial = 0;
    };
}
