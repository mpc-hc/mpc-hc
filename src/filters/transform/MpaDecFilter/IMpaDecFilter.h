/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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

enum MPCSampleFormat {SF_PCM16, SF_PCM24, SF_PCM32, SF_FLOAT32};

enum DolbyDigitalMode {
	DD_Unknown,
	DD_AC3,			// Standard AC3
	DD_EAC3,		// Dolby Digital +
	DD_TRUEHD,		// Dolby True HD
	DD_MLP			// Meridian Lossless Packing
};


interface __declspec(uuid("2067C60F-752F-4EBD-B0B1-4CBC5E00741C"))
IMpaDecFilter :
public IUnknown {
	enum enctype {ac3, dts, etlast};

	STDMETHOD(SetSampleFormat(MPCSampleFormat sf)) = 0;
	STDMETHOD_(MPCSampleFormat, GetSampleFormat()) = 0;
	STDMETHOD(SetNormalize(bool fNormalize)) = 0;
	STDMETHOD_(bool, GetNormalize()) = 0;
	STDMETHOD(SetSpeakerConfig(enctype et, int sc)) = 0; // sign of sc tells if spdif is active
	STDMETHOD_(int, GetSpeakerConfig(enctype et)) = 0;
	STDMETHOD(SetDynamicRangeControl(enctype et, bool fDRC)) = 0;
	STDMETHOD_(bool, GetDynamicRangeControl(enctype et)) = 0;
	STDMETHOD(SetBoost(float boost)) = 0;
	STDMETHOD_(float, GetBoost()) = 0;
	STDMETHOD_(DolbyDigitalMode, GetDolbyDigitalMode()) = 0;

	STDMETHOD(SaveSettings()) = 0;
};
