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

#include "stdafx.h"
#include "CompositionObject.h"
#include "ColorConvTable.h"
#include "../DSUtil/GolombBuffer.h"


CompositionObject::CompositionObject()
{
    Init();
}

CompositionObject::CompositionObject(const CompositionObject& obj)
    : m_object_id_ref(obj.m_object_id_ref)
    , m_window_id_ref(obj.m_window_id_ref)
    , m_object_cropped_flag(obj.m_object_cropped_flag)
    , m_forced_on_flag(obj.m_forced_on_flag)
    , m_version_number(obj.m_version_number)
    , m_horizontal_position(obj.m_horizontal_position)
    , m_vertical_position(obj.m_vertical_position)
    , m_width(obj.m_width)
    , m_height(obj.m_height)
    , m_cropping_horizontal_position(obj.m_cropping_horizontal_position)
    , m_cropping_vertical_position(obj.m_cropping_vertical_position)
    , m_cropping_width(obj.m_cropping_width)
    , m_cropping_height(obj.m_cropping_height)
    , m_pRLEData(nullptr)
    , m_nRLEDataSize(0)
    , m_nRLEPos(0)
    , m_nColorNumber(obj.m_nColorNumber)
    , m_colors(obj.m_colors)
{
    if (obj.m_pRLEData) {
        SetRLEData(obj.m_pRLEData, obj.m_nRLEPos, obj.m_nRLEDataSize);
    }
}

CompositionObject::~CompositionObject()
{
    delete [] m_pRLEData;
}

void CompositionObject::Init()
{
    m_pRLEData = nullptr;
    m_nRLEDataSize = m_nRLEPos = 0;
    m_nColorNumber = 0;
    m_object_id_ref = 0;
    m_window_id_ref = 0;
    m_object_cropped_flag = false;
    m_forced_on_flag = false;
    m_version_number = 0;
    m_horizontal_position = m_vertical_position = 0;
    m_width = m_height = 0;
    m_cropping_horizontal_position = m_cropping_vertical_position = 0;
    m_cropping_width = m_cropping_height = 0;

    m_colors.fill(0);
}

void CompositionObject::Reset()
{
    delete[] m_pRLEData;
    Init();
}

void CompositionObject::SetPalette(int nNbEntry, const HDMV_PALETTE* pPalette, ColorConvTable::YuvMatrixType currentMatrix)
{
    m_nColorNumber = nNbEntry;
    for (int i = 0; i < nNbEntry; i++) {
        m_colors[pPalette[i].entry_id] = ColorConvTable::A8Y8U8V8_TO_ARGB(pPalette[i].T, pPalette[i].Y, pPalette[i].Cb, pPalette[i].Cr, currentMatrix);
    }
}

void CompositionObject::SetRLEData(const BYTE* pBuffer, size_t nSize, size_t nTotalSize)
{
    delete [] m_pRLEData;

    if (nTotalSize > 0 && nSize <= nTotalSize) {
        m_pRLEData     = DEBUG_NEW BYTE[nTotalSize];
        m_nRLEDataSize = nTotalSize;
        m_nRLEPos      = nSize;

        memcpy(m_pRLEData, pBuffer, nSize);
    } else {
        m_pRLEData     = nullptr;
        m_nRLEDataSize = m_nRLEPos = 0;
        ASSERT(FALSE); // This shouldn't happen in normal operation
    }
}

void CompositionObject::AppendRLEData(const BYTE* pBuffer, size_t nSize)
{
    if (m_nRLEPos + nSize <= m_nRLEDataSize) {
        memcpy(m_pRLEData + m_nRLEPos, pBuffer, nSize);
        m_nRLEPos += nSize;
    } else {
        ASSERT(FALSE); // This shouldn't happen in normal operation
    }
}

void CompositionObject::RenderHdmv(SubPicDesc& spd)
{
    if (!m_pRLEData || !m_nColorNumber) {
        return;
    }

    CGolombBuffer GBuffer(m_pRLEData, m_nRLEDataSize);
    BYTE  bSwitch;
    BYTE  nPaletteIndex = 0;
    LONG nCount;
    LONG nX = m_horizontal_position;
    LONG nY = m_vertical_position;

    while ((nY < (m_vertical_position + m_height)) && !GBuffer.IsEOF()) {
        BYTE bTemp = GBuffer.ReadByte();
        if (bTemp != 0) {
            nPaletteIndex = bTemp;
            nCount = 1;
        } else {
            bSwitch = GBuffer.ReadByte();
            if (!(bSwitch & 0x80)) {
                if (!(bSwitch & 0x40)) {
                    nCount = bSwitch & 0x3F;
                    if (nCount > 0) {
                        nPaletteIndex = 0;
                    }
                } else {
                    nCount = (bSwitch & 0x3F) << 8 | (short)GBuffer.ReadByte();
                    nPaletteIndex = 0;
                }
            } else {
                if (!(bSwitch & 0x40)) {
                    nCount = bSwitch & 0x3F;
                    nPaletteIndex = GBuffer.ReadByte();
                } else {
                    nCount = (bSwitch & 0x3F) << 8 | (short)GBuffer.ReadByte();
                    nPaletteIndex = GBuffer.ReadByte();
                }
            }
        }

        if (nCount > 0) {
            if (nPaletteIndex != 0xFF) {    // Fully transparent (section 9.14.4.2.2.1.1)
                FillSolidRect(spd, nX, nY, nCount, 1, m_colors[nPaletteIndex]);
            }
            nX += nCount;
        } else {
            nY++;
            nX = m_horizontal_position;
        }
    }
}

void CompositionObject::RenderDvb(SubPicDesc& spd, short nX, short nY)
{
    if (!m_pRLEData) {
        return;
    }

    CGolombBuffer gb(m_pRLEData, m_nRLEDataSize);
    short sTopFieldLength;
    short sBottomFieldLength;

    sTopFieldLength    = gb.ReadShort();
    sBottomFieldLength = gb.ReadShort();

    DvbRenderField(spd, gb, nX, nY, sTopFieldLength);
    DvbRenderField(spd, gb, nX, nY + 1, sBottomFieldLength);
}

void CompositionObject::DvbRenderField(SubPicDesc& spd, CGolombBuffer& gb, short nXStart, short nYStart, short nLength)
{
    //FillSolidRect(spd, nXStart, nYStart, m_width, m_height, 0xFFFF0000);  // Red opaque
    //FillSolidRect(spd, nXStart, nYStart, m_width, m_height, 0xCC00FF00);  // Green 80%
    //FillSolidRect(spd, nXStart, nYStart, m_width, m_height, 0x100000FF);  // Blue 60%
    //return;
    short nX = nXStart;
    short nY = nYStart;
    size_t nEnd = gb.GetPos() + nLength;
    if (nEnd > gb.GetSize()) {
        // Unexpected end of data, the file is probably corrupted
        // but try to render the subtitles anyway
        ASSERT(FALSE);
        nEnd = gb.GetSize();
    }

    while (gb.GetPos() < nEnd) {
        BYTE bType = gb.ReadByte();
        switch (bType) {
            case 0x10:
                Dvb2PixelsCodeString(spd, gb, nX, nY);
                break;
            case 0x11:
                Dvb4PixelsCodeString(spd, gb, nX, nY);
                break;
            case 0x12:
                Dvb8PixelsCodeString(spd, gb, nX, nY);
                break;
            case 0x20:
                gb.SkipBytes(2);
                break;
            case 0x21:
                gb.SkipBytes(4);
                break;
            case 0x22:
                gb.SkipBytes(16);
                break;
            case 0xF0:
                nX  = nXStart;
                nY += 2;
                break;
            default:
                ASSERT(FALSE);
                break;
        }
    }
}

void CompositionObject::Dvb2PixelsCodeString(SubPicDesc& spd, CGolombBuffer& gb, short& nX, short& nY)
{
    bool bQuit = false;

    while (!bQuit && !gb.IsEOF()) {
        short nCount = 0;
        BYTE nPaletteIndex = 0;
        BYTE bTemp = (BYTE)gb.BitRead(2);
        if (bTemp != 0) {
            nPaletteIndex = bTemp;
            nCount = 1;
        } else {
            if (gb.BitRead(1) == 1) {                               // switch_1
                nCount = 3 + (short)gb.BitRead(3);                  // run_length_3-9
                nPaletteIndex = (BYTE)gb.BitRead(2);
            } else {
                if (gb.BitRead(1) == 0) {                           // switch_2
                    switch (gb.BitRead(2)) {                        // switch_3
                        case 0:
                            bQuit = true;
                            break;
                        case 1:
                            nCount = 2;
                            break;
                        case 2:                                     // if (switch_3 == '10')
                            nCount = 12 + (short)gb.BitRead(4);     // run_length_12-27
                            nPaletteIndex = (BYTE)gb.BitRead(2);    // 4-bit_pixel-code
                            break;
                        case 3:
                            nCount = 29 + gb.ReadByte();            // run_length_29-284
                            nPaletteIndex = (BYTE)gb.BitRead(2);    // 4-bit_pixel-code
                            break;
                    }
                } else {
                    nCount = 1;
                }
            }
        }

        if (nCount > 0) {
            FillSolidRect(spd, nX, nY, nCount, 1, m_colors[nPaletteIndex]);
            nX += nCount;
        }
    }

    gb.BitByteAlign();
}

void CompositionObject::Dvb4PixelsCodeString(SubPicDesc& spd, CGolombBuffer& gb, short& nX, short& nY)
{
    bool bQuit = false;

    while (!bQuit && !gb.IsEOF()) {
        short nCount = 0;
        BYTE nPaletteIndex = 0;
        BYTE bTemp = (BYTE)gb.BitRead(4);
        if (bTemp != 0) {
            nPaletteIndex = bTemp;
            nCount = 1;
        } else {
            if (gb.BitRead(1) == 0) {                               // switch_1
                nCount = (short)gb.BitRead(3);                      // run_length_3-9
                if (nCount != 0) {
                    nCount += 2;
                } else {
                    bQuit = true;
                }
            } else {
                if (gb.BitRead(1) == 0) {                           // switch_2
                    nCount = 4 + (short)gb.BitRead(2);              // run_length_4-7
                    nPaletteIndex = (BYTE)gb.BitRead(4);            // 4-bit_pixel-code
                } else {
                    switch (gb.BitRead(2)) {                        // switch_3
                        case 0:
                            nCount = 1;
                            break;
                        case 1:
                            nCount = 2;
                            break;
                        case 2:                                     // if (switch_3 == '10')
                            nCount = 9 + (short)gb.BitRead(4);      // run_length_9-24
                            nPaletteIndex = (BYTE)gb.BitRead(4);    // 4-bit_pixel-code
                            break;
                        case 3:
                            nCount = 25 + gb.ReadByte();            // run_length_25-280
                            nPaletteIndex = (BYTE)gb.BitRead(4);    // 4-bit_pixel-code
                            break;
                    }
                }
            }
        }

        if (nCount > 0) {
            FillSolidRect(spd, nX, nY, nCount, 1, m_colors[nPaletteIndex]);
            nX += nCount;
        }
    }

    gb.BitByteAlign();
}

void CompositionObject::Dvb8PixelsCodeString(SubPicDesc& spd, CGolombBuffer& gb, short& nX, short& nY)
{
    bool bQuit = false;

    while (!bQuit && !gb.IsEOF()) {
        short nCount = 0;
        BYTE nPaletteIndex = 0;
        BYTE bTemp = gb.ReadByte();
        if (bTemp != 0) {
            nPaletteIndex = bTemp;
            nCount = 1;
        } else {
            if (gb.BitRead(1) == 0) {                   // switch_1
                nCount = (short)gb.BitRead(7);          // run_length_1-127
                if (nCount == 0) {
                    bQuit = true;
                }
            } else {
                nCount = (short)gb.BitRead(7);          // run_length_3-127
                nPaletteIndex = gb.ReadByte();
            }
        }

        if (nCount > 0) {
            FillSolidRect(spd, nX, nY, nCount, 1, m_colors[nPaletteIndex]);
            nX += nCount;
        }
    }

    gb.BitByteAlign();
}
