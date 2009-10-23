////////////////////////////////////////////////////////////////////////////////
///
/// Win32 version of the x86 CPU detect routine.
///
/// This file is to be compiled in Windows platform with Microsoft Visual C++ 
/// Compiler. Please see 'cpu_detect_x86_gcc.cpp' for the gcc compiler version 
/// for all GNU platforms.
///
/// Author        : Copyright (c) Olli Parviainen
/// Author e-mail : oparviai 'at' iki.fi
/// SoundTouch WWW: http://www.surina.net/soundtouch
///
////////////////////////////////////////////////////////////////////////////////
//
// Last changed  : $Date: 2008-02-10 18:26:55 +0200 (Sun, 10 Feb 2008) $
// File revision : $Revision: 4 $
//
// $Id: cpu_detect_x86_win.cpp 11 2008-02-10 16:26:55Z oparviai $
//
////////////////////////////////////////////////////////////////////////////////
//
// License :
//
//  SoundTouch audio processing library
//  Copyright (c) Olli Parviainen
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////

#include "cpu_detect.h"
#include "intrin.h"

typedef enum PROCESSOR_TYPE
{
	PROCESSOR_AMD,
	PROCESSOR_INTEL,
	PROCESSOR_UNKNOWN
};


//////////////////////////////////////////////////////////////////////////////
//
// processor instructions extension detection routines
//
//////////////////////////////////////////////////////////////////////////////

// Flag variable indicating whick ISA extensions are disabled (for debugging)
static uint _dwDisabledISA = 0x00;      // 0xffffffff; //<- use this to disable all extensions


// Disables given set of instruction extensions. See SUPPORT_... defines.
void disableExtensions(uint dwDisableMask)
{
    _dwDisabledISA = dwDisableMask;
}



/// Checks which instruction set extensions are supported by the CPU.
uint detectCPUextensions(void)
{
    uint res = 0;

    if (_dwDisabledISA == 0xffffffff) return 0;

unsigned		nHighestFeature;
unsigned		nHighestFeatureEx;
int				nBuff[4];
char			szMan[13];
char			szFeatures[256];
PROCESSOR_TYPE	nType;

	// Get CPU manufacturer and highest CPUID
	__cpuid(nBuff, 0);
	nHighestFeature = (unsigned)nBuff[0];
	*(int*)&szMan[0] = nBuff[1];
	*(int*)&szMan[4] = nBuff[3];
	*(int*)&szMan[8] = nBuff[2];
	szMan[12] = 0;
	if(strcmp(szMan, "AuthenticAMD") == 0)
		nType = PROCESSOR_AMD;
	else if(strcmp(szMan, "GenuineIntel") == 0)
		nType = PROCESSOR_INTEL;
	else
		nType = PROCESSOR_UNKNOWN;

	// Get highest extended feature
	__cpuid(nBuff, 0x80000000);
	nHighestFeatureEx = (unsigned)nBuff[0];

	// Get CPU features
	szFeatures[0]	= 0;
	if(nHighestFeature >= 1)
	{
		__cpuid(nBuff, 1);
		if(nBuff[3] & 1<<23)	res|=SUPPORT_MMX;
		if(nBuff[3] & 1<<25)	res|=SUPPORT_SSE;
		if(nBuff[3] & 1<<26)	res|=SUPPORT_SSE2;
	}

	// AMD specific:
	if(nType == PROCESSOR_AMD)
	{
		// Get extended features
		__cpuid(nBuff, 0x80000000);
		if(nHighestFeatureEx >= 0x80000001)
		{
			__cpuid(nBuff, 0x80000001);
			if(nBuff[3] & 1<<31)	res|=SUPPORT_3DNOW;
		}
	}
	

    return res & ~_dwDisabledISA;
}
