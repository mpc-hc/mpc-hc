/*
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

typedef enum {DIAuto, DIWeave, DIBlend, DIBob, DIFieldShift} ditype;

[uuid("0ABEAA65-0317-47B9-AE1D-D9EA905AFD25")]
interface IMpeg2DecFilter :
public IUnknown
{
    STDMETHOD(SetDeinterlaceMethod(ditype di)) = 0;
    STDMETHOD_(ditype, GetDeinterlaceMethod()) = 0;

    // Brightness: -255.0 to 255.0, default 0.0
    // Contrast: 0.0 to 10.0, default 1.0
    // Hue: -180.0 to +180.0, default 0.0
    // Saturation: 0.0 to 10.0, default 1.0

    STDMETHOD(SetBrightness(float brightness)) = 0;
    STDMETHOD(SetContrast(float contrast)) = 0;
    STDMETHOD(SetHue(float hue)) = 0;
    STDMETHOD(SetSaturation(float saturation)) = 0;
    STDMETHOD_(float, GetBrightness()) = 0;
    STDMETHOD_(float, GetContrast()) = 0;
    STDMETHOD_(float, GetHue()) = 0;
    STDMETHOD_(float, GetSaturation()) = 0;

    STDMETHOD(EnableForcedSubtitles(bool fEnable)) = 0;
    STDMETHOD_(bool, IsForcedSubtitlesEnabled()) = 0;

    STDMETHOD(EnablePlanarYUV(bool fEnable)) = 0;
    STDMETHOD_(bool, IsPlanarYUVEnabled()) = 0;

    STDMETHOD(EnableInterlaced(bool fEnable)) = 0;
    STDMETHOD_(bool, IsInterlacedEnabled()) = 0;
};

