/*
 * (C) 2008-2014 see Authors.txt
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
#include <algorithm>
#include "GolombBuffer.h"

CGolombBuffer::CGolombBuffer(BYTE* pBuffer, size_t nSize)
    : m_pBuffer(pBuffer)
    , m_nSize(nSize)
{
    Reset();
}

CGolombBuffer::~CGolombBuffer()
{
}

UINT64 CGolombBuffer::BitRead(size_t nBits, bool fPeek)
{
    //ASSERT(nBits >= 0 && nBits <= 64);

    while (m_bitlen < nBits) {
        m_bitbuff <<= 8;

        if (m_nBitPos >= m_nSize) {
            return 0;
        }

        *(BYTE*)&m_bitbuff = m_pBuffer[m_nBitPos++];
        m_bitlen += 8;
    }

    size_t bitlen = m_bitlen - nBits;

    UINT64 ret;
    // The shift to 64 bits can give incorrect results.
    // "The behavior is undefined if the right operand is negative, or greater than or equal to the length in bits of the promoted left operand."
    if (nBits == 64) {
        ret = m_bitbuff;
    } else {
        ret = (m_bitbuff >> bitlen) & ((1ui64 << nBits) - 1);
    }

    if (!fPeek) {
        m_bitbuff &= ((1ui64 << bitlen) - 1);
        m_bitlen = bitlen;
    }

    return ret;
}

UINT64 CGolombBuffer::UExpGolombRead()
{
    int n = -1;
    for (BYTE b = 0; !b; n++) {
        b = (BYTE)BitRead(1);
    }
    return (1ui64 << n) - 1 + BitRead(n);
}

INT64 CGolombBuffer::SExpGolombRead()
{
    UINT64 k = UExpGolombRead();
    return ((k & 1) ? 1 : -1) * ((k + 1) >> 1);
}

void CGolombBuffer::BitByteAlign()
{
    m_bitlen &= ~7;
}

size_t CGolombBuffer::GetPos()
{
    return m_nBitPos - (m_bitlen >> 3);
}

void CGolombBuffer::ReadBuffer(BYTE* pDest, size_t nSize)
{
    ASSERT(m_nBitPos + nSize <= m_nSize);
    ASSERT(m_bitlen == 0);
    nSize = std::min(nSize, m_nSize - m_nBitPos);

    memcpy(pDest, m_pBuffer + m_nBitPos, nSize);
    m_nBitPos += nSize;
}

void CGolombBuffer::Reset()
{
    m_nBitPos = 0;
    m_bitlen  = 0;
    m_bitbuff = 0;
}

void CGolombBuffer::Reset(BYTE* pNewBuffer, size_t nNewSize)
{
    m_pBuffer = pNewBuffer;
    m_nSize   = nNewSize;

    Reset();
}

void CGolombBuffer::SkipBytes(size_t nCount)
{
    m_nBitPos += nCount;
    m_bitlen   = 0;
    m_bitbuff  = 0;
}
