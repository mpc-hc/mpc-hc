/*
 * (C) 2009-2015 see Authors.txt
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

#include "Rasterizer.h"
#include "ColorConvTable.h"


struct HDMV_PALETTE {
    BYTE    entry_id;
    BYTE    Y;
    BYTE    Cr;
    BYTE    Cb;
    BYTE    T;      // HDMV rule : 0 transparent, 255 opaque (compatible DirectX)
};

class CGolombBuffer;

class CompositionObject : Rasterizer
{
public:
    short m_object_id_ref;
    BYTE  m_window_id_ref;
    bool  m_object_cropped_flag;
    bool  m_forced_on_flag;
    BYTE  m_version_number;

    LONG m_horizontal_position;
    LONG m_vertical_position;
    LONG m_width;
    LONG m_height;

    LONG m_cropping_horizontal_position;
    LONG m_cropping_vertical_position;
    LONG m_cropping_width;
    LONG m_cropping_height;

    CompositionObject();
    CompositionObject(const CompositionObject& obj);
    ~CompositionObject();

    void  Init();
    void  Reset();

    void SetRLEData(const BYTE* pBuffer, size_t nSize, size_t nTotalSize);
    void AppendRLEData(const BYTE* pBuffer, size_t nSize);
    const BYTE* GetRLEData() const { return m_pRLEData; };
    size_t GetRLEDataSize() const { return m_nRLEDataSize; };
    size_t GetRLEPos() const { return m_nRLEPos; };
    bool IsRLEComplete() const { return m_nRLEPos >= m_nRLEDataSize; };
    void RenderHdmv(SubPicDesc& spd);
    void RenderDvb(SubPicDesc& spd, short nX, short nY);
    void WriteSeg(SubPicDesc& spd, short nX, short nY, short nCount, short nPaletteIndex);
    void SetPalette(int nNbEntry, const HDMV_PALETTE* pPalette, ColorConvTable::YuvMatrixType currentMatrix);
    bool HavePalette() const { return m_nColorNumber > 0; };

    // Forbid the use of direct affectation for now, it would be dangerous because
    // of possible leaks and double frees. We could do a deep copy to be safe but
    // it could possibly hurt the performance if we forgot about this and start
    // using affectation a lot.
    CompositionObject& operator=(const CompositionObject&) = delete;

private:
    BYTE* m_pRLEData;
    size_t m_nRLEDataSize;
    size_t m_nRLEPos;
    int m_nColorNumber;
    std::array<DWORD, 256> m_colors;

    void  DvbRenderField(SubPicDesc& spd, CGolombBuffer& gb, short nXStart, short nYStart, short nLength);
    void  Dvb2PixelsCodeString(SubPicDesc& spd, CGolombBuffer& gb, short& nX, short& nY);
    void  Dvb4PixelsCodeString(SubPicDesc& spd, CGolombBuffer& gb, short& nX, short& nY);
    void  Dvb8PixelsCodeString(SubPicDesc& spd, CGolombBuffer& gb, short& nX, short& nY);
};
