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

#pragma once

class CGolombBuffer
{
public:
    CGolombBuffer(BYTE* pBuffer, size_t nSize);
    ~CGolombBuffer();

    UINT64 BitRead(size_t nBits, bool fPeek = false);
    UINT64 UExpGolombRead();
    INT64 SExpGolombRead();
    void BitByteAlign();

    inline BYTE ReadByte() { return (BYTE)BitRead(8); };
    inline short ReadShort() { return (short)BitRead(16); };
    inline DWORD ReadDword() { return (DWORD)BitRead(32); };
    void ReadBuffer(BYTE* pDest, size_t nSize);

    void Reset();
    void Reset(BYTE* pNewBuffer, size_t nNewSize);

    void SetSize(size_t nValue) { m_nSize = nValue; };
    size_t GetSize() const { return m_nSize; };
    size_t RemainingSize() const { return m_nSize - m_nBitPos; };
    bool IsEOF() const { return m_nBitPos >= m_nSize; };
    size_t GetPos();
    BYTE* GetBufferPos() { return m_pBuffer + m_nBitPos; };

    void SkipBytes(size_t nCount);

private:
    BYTE*  m_pBuffer;
    size_t m_nSize;
    size_t m_nBitPos;
    size_t m_bitlen;
    INT64  m_bitbuff;
};
