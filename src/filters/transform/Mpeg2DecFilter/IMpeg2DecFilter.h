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

typedef enum {
    DIAuto,
    DIWeave,
    DIBlend,
    DIBob,
    DIFieldShift,
    DIELA
} ditype;

interface __declspec(uuid("0ABEAA65-0317-47B9-AE1D-D9EA905AFD25"))
IMpeg2DecFilter :
public IUnknown {
    STDMETHOD(SetDeinterlaceMethod(ditype di)) PURE;
    STDMETHOD_(ditype, GetDeinterlaceMethod()) PURE;
    // Brightness: -255.0 to 255.0, default 0.0
    // Contrast: 0.0 to 10.0, default 1.0
    // Hue: -180.0 to +180.0, default 0.0
    // Saturation: 0.0 to 10.0, default 1.0

    STDMETHOD(SetBrightness(float brightness)) PURE;
    STDMETHOD(SetContrast(float contrast)) PURE;
    STDMETHOD(SetHue(float hue)) PURE;
    STDMETHOD(SetSaturation(float saturation)) PURE;
    STDMETHOD_(float, GetBrightness()) PURE;
    STDMETHOD_(float, GetContrast()) PURE;
    STDMETHOD_(float, GetHue()) PURE;
    STDMETHOD_(float, GetSaturation()) PURE;

    STDMETHOD(EnableForcedSubtitles(bool fEnable)) PURE;
    STDMETHOD_(bool, IsForcedSubtitlesEnabled()) PURE;

    STDMETHOD(EnablePlanarYUV(bool fEnable)) PURE;
    STDMETHOD_(bool, IsPlanarYUVEnabled()) PURE;

    STDMETHOD(EnableInterlaced(bool fEnable)) PURE;
    STDMETHOD_(bool, IsInterlacedEnabled()) PURE;

    STDMETHOD(EnableReadARFromStream(bool fEnable)) PURE;
    STDMETHOD_(bool, IsReadARFromStreamEnabled()) PURE;

    STDMETHOD(Apply()) PURE;
};
