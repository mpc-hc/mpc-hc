/*
* (C) 2015 see Authors.txt
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

struct ColorConvTable {
    enum YuvMatrixType {
        NONE,
        BT601,
        BT709
    };

    enum YuvRangeType {
        RANGE_NONE,
        RANGE_TV,
        RANGE_PC
    };

    static void SetDefaultConvType(YuvMatrixType yuv_type, YuvRangeType range, bool bOutputTVRange, bool bVSFilterCorrection);

    static YuvMatrixType GetDefaultYUVType();
    static YuvRangeType GetDefaultRangeType();

    static DWORD Argb2Ayuv(DWORD argb);
    static DWORD Argb2Ayuv_TV_BT601(DWORD argb);
    static DWORD Argb2Auyv(DWORD argb);
    static DWORD Ayuv2Auyv(DWORD ayuv);
    static DWORD Rgb2Y(int r8, int g8, int b8);
    static DWORD PreMulArgb2Ayuv(int a8, int r8, int g8, int b8);

    static DWORD Ayuv2Argb(DWORD ayuv);
    static DWORD Ayuv2Argb_TV_BT601(DWORD ayuv);
    static DWORD A8Y8U8V8_To_ARGB_TV_BT601(int a8, int y8, int u8, int v8);
    static DWORD A8Y8U8V8_To_ARGB_PC_BT601(int a8, int y8, int u8, int v8);
    static DWORD Ayuv2Argb_TV_BT709(DWORD ayuv);
    static DWORD A8Y8U8V8_To_ARGB_TV_BT709(int a8, int y8, int u8, int v8);
    static DWORD A8Y8U8V8_To_ARGB_PC_BT709(int a8, int y8, int u8, int v8);

    static DWORD A8Y8U8V8_PC_To_TV(int a8, int y8, int u8, int v8);
    static DWORD A8Y8U8V8_TV_To_PC(int a8, int y8, int u8, int v8);

    //should not past NONE into it
    static DWORD A8Y8U8V8_TO_AYUV(int a8, int y8, int u8, int v8, YuvRangeType in_range, YuvMatrixType in_type, YuvRangeType out_range, YuvMatrixType out_type);
    static DWORD A8Y8U8V8_TO_CUR_AYUV(int a8, int y8, int u8, int v8, YuvRangeType in_range, YuvMatrixType in_type);
    static DWORD A8Y8U8V8_TO_ARGB(int a8, int y8, int u8, int v8, YuvMatrixType in_type);

    static DWORD RGB_PC_TO_TV(DWORD argb);

    static DWORD ColorCorrection(DWORD argb);

    ColorConvTable() = delete;
};
