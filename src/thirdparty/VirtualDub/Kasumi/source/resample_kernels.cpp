//	VirtualDub - Video processing and capture application
//	Graphics support library
//	Copyright (C) 1998-2009 Avery Lee
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

#include <stdafx.h>
#include <math.h>
#include <vd2/Kasumi/resample_kernels.h>

///////////////////////////////////////////////////////////////////////////
//
// utility functions
//
///////////////////////////////////////////////////////////////////////////

namespace {
	inline sint32 scale32x32_fp16(sint32 x, sint32 y) {
		return (sint32)(((sint64)x * y + 0x8000) >> 16);
	}

	inline double sinc(double x) {
		return fabs(x) < 1e-9 ? 1.0 : sin(x) / x;
	}
}

///////////////////////////////////////////////////////////////////////////
//
// VDResamplerAxis
//
///////////////////////////////////////////////////////////////////////////

void VDResamplerAxis::Init(sint32 dudx) {
	this->dudx = dudx;
}

void VDResamplerAxis::Compute(sint32 count, sint32 u0, sint32 w, sint32 kernel_width) {
	u = u0;
	dx = count;

	sint32 du_kern	= (kernel_width-1) << 16;
	sint32 u2		= u + dudx*(dx-1);
	sint32 u_limit	= w << 16;

	dx_precopy	= 0;
	dx_preclip	= 0;
	dx_active	= 0;
	dx_postclip	= 0;
	dx_postcopy = 0;
	dx_dualclip	= 0;

	if (dudx == 0) {
		if (u < -du_kern)
			dx_precopy = w;
		else if (u >= u_limit)
			dx_postcopy = w;
		else if (u < 0) {
			if (u + du_kern < u_limit)
				dx_preclip = w;
			else
				dx_dualclip = w;
		} else if (u + du_kern >= u_limit)
			dx_postclip = w;
		else
			dx_active = w;

		return;
	}

	sint32 dx_temp = dx;
	sint32 u_start = u;

	// (desired - u0 + (dudx-1)) / dudx : first pixel >= desired

	sint32 dudx_m1_mu0	= dudx - 1 - u;
	sint32 first_preclip	= (dudx_m1_mu0 + 0x10000 - du_kern) / dudx;
	sint32 first_active		= (dudx_m1_mu0                    ) / dudx;
	sint32 first_postclip	= (dudx_m1_mu0 + u_limit - du_kern) / dudx;
	sint32 first_postcopy	= (dudx_m1_mu0 + u_limit - 0x10000) / dudx;

	// clamp
	if (first_preclip < 0)
		first_preclip = 0;
	if (first_active < first_preclip)
		first_active = first_preclip;
	if (first_postclip < first_active)
		first_postclip = first_active;
	if (first_postcopy < first_postclip)
		first_postcopy = first_postclip;
	if (first_preclip > dx)
		first_preclip = dx;
	if (first_active > dx)
		first_active = dx;
	if (first_postclip > dx)
		first_postclip = dx;
	if (first_postcopy > dx)
		first_postcopy = dx;

	// determine widths

	dx_precopy	= first_preclip;
	dx_preclip	= first_active - first_preclip;
	dx_active	= first_postclip - first_active;
	dx_postclip	= first_postcopy - first_postclip;
	dx_postcopy	= dx - first_postcopy;

	// sanity checks
	sint32 pos0 = dx_precopy;
	sint32 pos1 = pos0 + dx_preclip;
	sint32 pos2 = pos1 + dx_active;
	sint32 pos3 = pos2 + dx_postclip;

	VDASSERT(!((dx_precopy|dx_preclip|dx_active|dx_postcopy|dx_postclip) & 0x80000000));
	VDASSERT(dx_precopy + dx_preclip + dx_active + dx_postcopy + dx_postclip == dx);

	VDASSERT(!pos0			|| u_start + dudx*(pos0 - 1) <  0x10000 - du_kern);	// precopy -> preclip
	VDASSERT( pos0 >= pos1	|| u_start + dudx*(pos0    ) >= 0x10000 - du_kern);
	VDASSERT( pos1 <= pos0	|| u_start + dudx*(pos1 - 1) <  0);					// preclip -> active
	VDASSERT( pos1 >= pos2	|| u_start + dudx*(pos1    ) >= 0 || !dx_active);
	VDASSERT( pos2 <= pos1	|| u_start + dudx*(pos2 - 1) <  u_limit - du_kern || !dx_active);	// active -> postclip
	VDASSERT( pos2 >= pos3	|| u_start + dudx*(pos2    ) >= u_limit - du_kern);
	VDASSERT( pos3 <= pos2	|| u_start + dudx*(pos3 - 1) <  u_limit - 0x10000);	// postclip -> postcopy
	VDASSERT( pos3 >= dx	|| u_start + dudx*(pos3    ) >= u_limit - 0x10000);

	u += dx_precopy * dudx;

	// test for overlapping clipping regions
	if (!dx_active && kernel_width > w) {
		dx_dualclip = dx_preclip + dx_postclip;
		dx_preclip = dx_postclip = 0;
	}
}

///////////////////////////////////////////////////////////////////////////
//
// VDResamplerLinearFilter
//
///////////////////////////////////////////////////////////////////////////

VDResamplerLinearFilter::VDResamplerLinearFilter(double twofc)
	: mScale(twofc)
	, mTaps((int)ceil(1.0 / twofc) * 2)
{
}

int VDResamplerLinearFilter::GetFilterWidth() const {
	return mTaps;
}

double VDResamplerLinearFilter::EvaluateFilter(double t) const {
	t = 1.0f - fabs(t)*mScale;

	return t + fabs(t);
}

void VDResamplerLinearFilter::GenerateFilter(float *dst, double offset) const {
	double pos = -((double)((mTaps>>1)-1) + offset) * mScale;

	for(unsigned i=0; i<mTaps; ++i) {
		double t = 1.0 - fabs(pos);

		*dst++ = (float)(t+fabs(t));
		pos += mScale;
	}
}

void VDResamplerLinearFilter::GenerateFilterBank(float *dst) const {
	for(int offset=0; offset<256; ++offset) {
		GenerateFilter(dst, offset * (1.0f / 256.0f));
		dst += mTaps;
	}
}

///////////////////////////////////////////////////////////////////////////
//
// VDResamplerCubicFilter
//
///////////////////////////////////////////////////////////////////////////

VDResamplerCubicFilter::VDResamplerCubicFilter(double twofc, double A)
	: mScale(twofc)
	, mA0( 1.0  )
	, mA2(-3.0-A)
	, mA3( 2.0+A)
	, mB0(-4.0*A)
	, mB1( 8.0*A)
	, mB2(-5.0*A)
	, mB3(     A)
	, mTaps((int)ceil(2.0 / twofc)*2)
{
}

int VDResamplerCubicFilter::GetFilterWidth() const { return mTaps; }

double VDResamplerCubicFilter::EvaluateFilter(double t) const {
	t = fabs(t)*mScale;

	if (t < 1.0)
		return mA0 + (t*t)*(mA2 + t*mA3);
	else if (t < 2.0)
		return mB0 + t*(mB1 + t*(mB2 + t*mB3));
	else
		return 0;
}

void VDResamplerCubicFilter::GenerateFilter(float *dst, double offset) const {
	double pos = -((double)((mTaps>>1)-1) + offset) * mScale;

	for(unsigned i=0; i<mTaps; ++i) {
		double t = fabs(pos);
		double v = 0;

		if (t < 1.0)
			v = mA0 + (t*t)*(mA2 + t*mA3);
		else if (t < 2.0)
			v = mB0 + t*(mB1 + t*(mB2 + t*mB3));

		*dst++ = (float)v;
		pos += mScale;
	}
}

void VDResamplerCubicFilter::GenerateFilterBank(float *dst) const {
	for(int offset=0; offset<256; ++offset) {
		GenerateFilter(dst, offset * (1.0f / 256.0f));
		dst += mTaps;
	}
}

///////////////////////////////////////////////////////////////////////////
//
// VDResamplerLanczos3Filter
//
///////////////////////////////////////////////////////////////////////////

VDResamplerLanczos3Filter::VDResamplerLanczos3Filter(double twofc)
	: mScale(twofc)
	, mTaps((int)ceil(3.0 / twofc)*2)
{
}

int VDResamplerLanczos3Filter::GetFilterWidth() const {
	return mTaps;
}

double VDResamplerLanczos3Filter::EvaluateFilter(double t) const {
	static const double pi  = 3.1415926535897932384626433832795;	// pi
	static const double pi3 = 1.0471975511965977461542144610932;	// pi/3

	t *= mScale;

	if (fabs(t) < 3.0)
		return sinc(pi*t) * sinc(pi3*t);
	else
		return 0.0;
}

void VDResamplerLanczos3Filter::GenerateFilter(float *dst, double offset) const {
	static const double pi  = 3.1415926535897932384626433832795;	// pi
	static const double pi3 = 1.0471975511965977461542144610932;	// pi/3

	double t = -(((double)((mTaps>>1)-1) + offset) * mScale);

	for(unsigned i=0; i<mTaps; ++i) {
		double v = 0;

		if (fabs(t) < 3.0)
			v = sinc(pi*t) * sinc(pi3*t);

		*dst++ = (float)v;
		t += mScale;
	}
}

void VDResamplerLanczos3Filter::GenerateFilterBank(float *dst) const {
	for(int offset=0; offset<256; ++offset) {
		GenerateFilter(dst, offset * (1.0f / 256.0f));
		dst += mTaps;
	}
}
