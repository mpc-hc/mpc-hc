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

#include <windows.h>
#include <MMREG.H>
#include <mmsystem.h>
#include <msacm.h>

typedef long (*AudioPointSampler)(void *, void *, long, long, long);
typedef long (*AudioDownSampler)(void *, void *, long *, int, long, long, long);

class AudioStreamResampler 
{
private:
	AudioPointSampler ptsampleRout;
	AudioDownSampler dnsampleRout;
	long samp_frac;
	long accum;
	int holdover;
	long *filter_bank;
	int filter_width;
	bool fHighQuality;

	enum { BUFFER_SIZE=512 };
	BYTE cbuffer[4*BUFFER_SIZE];
	int bps;

public:
	AudioStreamResampler(int bps, long org_rate, long new_rate, bool fHighQuality);
	~AudioStreamResampler();

	long Downsample(void* input, long samplesin, void* output, long samplesout);
};

