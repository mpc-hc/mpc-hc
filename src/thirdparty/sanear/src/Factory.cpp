#include "pch.h"
#include "Factory.h"

#include "MyFilter.h"
#include "Settings.h"

namespace SaneAudioRenderer
{
    HRESULT Factory::CreateSettings(ISettings** ppOut)
    {
        IUnknownPtr unknown;
        ReturnIfFailed(CreateSettingsAggregated(nullptr, &unknown));
        return unknown->QueryInterface(IID_PPV_ARGS(ppOut));
    }

    HRESULT Factory::CreateSettingsAggregated(IUnknown* pOwner, IUnknown** ppOut)
    {
        CheckPointer(ppOut, E_POINTER);

        *ppOut = nullptr;

        auto pSettings = new(std::nothrow) Settings(pOwner);

        if (!pSettings)
            return E_OUTOFMEMORY;

        pSettings->NonDelegatingAddRef();

        HRESULT result = pSettings->NonDelegatingQueryInterface(IID_PPV_ARGS(ppOut));

        pSettings->NonDelegatingRelease();

        return result;
    }

    HRESULT Factory::CreateFilter(ISettings* pSettings, IBaseFilter** ppOut)
    {
        IUnknownPtr unknown;
        ReturnIfFailed(CreateFilterAggregated(nullptr, GetFilterGuid(), pSettings, &unknown));
        return unknown->QueryInterface(IID_PPV_ARGS(ppOut));
    }

    HRESULT Factory::CreateFilterAggregated(IUnknown* pOwner, const GUID& guid,
                                            ISettings* pSettings, IUnknown** ppOut)
    {
        CheckPointer(ppOut, E_POINTER);
        CheckPointer(pSettings, E_POINTER);

        *ppOut = nullptr;

        auto pFilter = new(std::nothrow) MyFilter(pOwner, guid);

        if (!pFilter)
            return E_OUTOFMEMORY;

        pFilter->NonDelegatingAddRef();

        HRESULT result = pFilter->Init(pSettings);

        if (SUCCEEDED(result))
            result = pFilter->NonDelegatingQueryInterface(IID_PPV_ARGS(ppOut));

        pFilter->NonDelegatingRelease();

        return result;
    }

    const GUID& Factory::GetFilterGuid()
    {
        static const GUID guid = {0x2AE00773, 0x819A, 0x40FB, {0xA5, 0x54, 0x54, 0x82, 0x7E, 0x11, 0x63, 0x59}};
        return guid;
    }
}
