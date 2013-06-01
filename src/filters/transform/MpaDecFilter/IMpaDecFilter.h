/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

enum MPCSampleFormat {
    SF_PCM16 = 0,
    SF_PCM32,
    SF_FLOAT,
    SF_PCM24,
    sfcount
};

interface __declspec(uuid("2067C60F-752F-4EBD-B0B1-4CBC5E00741C"))
IMpaDecFilter :
public IUnknown {
    enum enctype {
        ac3 = 0,
        eac3,
        truehd,
        dts,
        dtshd,
        etcount
    };

    STDMETHOD(SetSampleFormat(MPCSampleFormat sf, bool enable)) PURE;
    STDMETHOD_(bool, GetSampleFormat(MPCSampleFormat sf)) PURE;
    STDMETHOD_(MPCSampleFormat, SelectSampleFormat(MPCSampleFormat sf)) PURE;
    STDMETHOD(SetMixer(bool fMixer)) PURE;
    STDMETHOD_(bool, GetMixer()) PURE;
    STDMETHOD(SetMixerLayout(int sc)) PURE;
    STDMETHOD_(int, GetMixerLayout()) PURE;
    STDMETHOD(SetDynamicRangeControl(bool fDRC)) PURE;
    STDMETHOD_(bool, GetDynamicRangeControl()) PURE;
    STDMETHOD(SetSPDIF(enctype et, bool fSPDIF)) PURE;
    STDMETHOD_(bool, GetSPDIF(enctype et)) PURE;

    STDMETHOD(SaveSettings()) PURE;
};
