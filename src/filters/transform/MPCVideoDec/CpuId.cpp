/*
 * (C) 2007-2012 see Authors.txt
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

#include "stdafx.h"
#include <intrin.h>
#include "CpuId.h"

#define CPUID_MMX    (1 << 23)
#define CPUID_SSE    (1 << 25)
#define CPUID_SSE2   (1 << 26)
#define CPUID_SSE3   (1 <<  0)

// Intel specific
#define CPUID_SSSE3  (1 <<  9)

// AMD specific
#define CPUID_3DNOW  (1 << 31)
#define CPUID_MMXEXT (1 << 22)


CCpuId::CCpuId()
{
    unsigned nHighestFeature;
    unsigned nHighestFeatureEx;
    int  nBuff[4];
    char szMan[13];

    // Get CPU manufacturer and highest CPUID
    __cpuid(nBuff, 0);
    nHighestFeature  = (unsigned)nBuff[0];
    *(int*)&szMan[0] = nBuff[1];
    *(int*)&szMan[4] = nBuff[3];
    *(int*)&szMan[8] = nBuff[2];
    szMan[12] = 0;

    if (strcmp(szMan, "AuthenticAMD") == 0) {
        m_nType = PROCESSOR_AMD;
    } else if (strcmp(szMan, "GenuineIntel") == 0) {
        m_nType = PROCESSOR_INTEL;
    } else {
        m_nType = PROCESSOR_UNKNOWN;
    }

    // Get highest extended feature
    __cpuid(nBuff, 0x80000000);
    nHighestFeatureEx = (unsigned)nBuff[0];

    // Get CPU features
    m_nCPUFeatures = 0;
    if (nHighestFeature >= 1) {
        __cpuid(nBuff, 1);
        if (nBuff[3] & CPUID_MMX) {
            m_nCPUFeatures |= MPC_MM_MMX;
        }
        if (nBuff[3] & CPUID_SSE) {
            m_nCPUFeatures |= MPC_MM_SSE;
        }
        if (nBuff[3] & CPUID_SSE2) {
            m_nCPUFeatures |= MPC_MM_SSE2;
        }
        if (nBuff[2] & CPUID_SSE3) {
            m_nCPUFeatures |= MPC_MM_SSE3;
        }

        // Intel specific
        if (m_nType == PROCESSOR_INTEL) {
            if (nBuff[2] & CPUID_SSSE3) {
                m_nCPUFeatures |= MPC_MM_SSSE3;
            }
        }
    }

    // AMD specific
    if (m_nType == PROCESSOR_AMD) {
        // Get extended features
        __cpuid(nBuff, 0x80000000);
        if (nHighestFeatureEx >= 0x80000001) {
            __cpuid(nBuff, 0x80000001);
            if (nBuff[3] & CPUID_3DNOW) {
                m_nCPUFeatures |= MPC_MM_3DNOW;
            }
            if (nBuff[3] & CPUID_MMXEXT) {
                m_nCPUFeatures |= MPC_MM_MMXEXT;
            }
        }
    }

}

int CCpuId::GetProcessorNumber()
{
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

    return SystemInfo.dwNumberOfProcessors;
}
