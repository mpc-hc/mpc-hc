#pragma once

namespace SaneAudioRenderer
{
    class AudioDevice;

    // NOTE: This is internal interface and shouldn't be used outside of Sanear.
    struct __declspec(uuid("361657BC-CC1E-420A-BE7B-21C34E3D9F76"))
    IStatusPageData : IUnknown
    {
        STDMETHOD(GetPageData)(bool resize, std::vector<char>& data) = 0;
    };
    _COM_SMARTPTR_TYPEDEF(IStatusPageData, __uuidof(IStatusPageData));

    class _declspec(uuid("7EEEDEC8-8B8E-4220-AF12-08BC0CE844F0"))
    MyPropertyPage final
        : public CUnknown
        , public IPropertyPage
    {
    public:

        static std::vector<char> CreateDialogData(bool resize, SharedWaveFormat inputFormat, const AudioDevice* device,
                                                  std::vector<std::wstring> processors, bool externalClock, bool live,
                                                  bool guidedReclock);

        MyPropertyPage();
        MyPropertyPage(HRESULT& result, IStatusPageData* pData);
        MyPropertyPage(const MyPropertyPage&) = delete;
        MyPropertyPage& operator=(const MyPropertyPage&) = delete;

        DECLARE_IUNKNOWN

        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;

        STDMETHODIMP SetPageSite(IPropertyPageSite* pPageSite) override;
        STDMETHODIMP Activate(HWND hParent, LPCRECT pRect, BOOL bModal) override;
        STDMETHODIMP Deactivate() override;
        STDMETHODIMP GetPageInfo(PROPPAGEINFO* pPageInfo) override;
        STDMETHODIMP SetObjects(ULONG nObjects, IUnknown** ppUnk) override;
        STDMETHODIMP Show(UINT cmdShow) override;
        STDMETHODIMP Move(LPCRECT pRect) override;
        STDMETHODIMP IsPageDirty() override { return S_FALSE; }
        STDMETHODIMP Apply() override { return S_OK; }
        STDMETHODIMP Help(LPCOLESTR) override { return E_NOTIMPL; }
        STDMETHODIMP TranslateAccelerator(MSG*) override { return E_NOTIMPL; }

    private:

        const bool m_delayedData;
        std::vector<char> m_dialogData;
        IPropertyPageSitePtr m_pageSite;
        HWND m_hWindow = NULL;
    };
}
