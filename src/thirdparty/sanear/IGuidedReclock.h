// This file is released under CC0 1.0 license
// License text can be found at http://creativecommons.org/publicdomain/zero/1.0/

// Originally designed as part of sanear project

#pragma once

struct __declspec(uuid("243F1282-94C3-46A1-B3F6-72B400786FEC"))
IGuidedReclock : IUnknown
{
    STDMETHOD(SlaveClock)(DOUBLE multiplier) = 0;
    STDMETHOD(UnslaveClock)() = 0;

    STDMETHOD(OffsetClock)(LONGLONG offset) = 0;

    STDMETHOD(GetImmediateTime)(LONGLONG* pTime) = 0;
};
