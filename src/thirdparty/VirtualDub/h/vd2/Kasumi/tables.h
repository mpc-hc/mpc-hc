//	VirtualDub - Video processing and capture application
//	Graphics support library
//	Copyright (C) 1998-2008 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef f_VD2_KASUMI_TABLES_H
#define f_VD2_KASUMI_TABLES_H

///////////////////////////////////////////////////////////////////////////////
// Cubic interpolation tables
//
// These tables give coefficients for 1-D cubic interpolation with 8-bit
// subunit precision. The [0] entry is positioned exactly on top of the
// second sample, and the [255] entry is 255/256th of the way to the third
// sample. The cardinal spline constant is -0.75 and the output range is
// [-0.1875, 1.1875], where the maximum overshoot and undershoot occur at
// the midpoint.
//
// The first and fourth coefficients are always negative; the second and
// third coefficients are always positive.
//
extern "C" const sint32 kVDCubicInterpTableFX14_075[256][4];

#ifdef _M_IX86
	extern "C" const sint16 kVDCubicInterpTableFX14_075_MMX[256][8];
#endif

#endif
