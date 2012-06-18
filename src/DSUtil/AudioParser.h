/*
 * $Id$
 *
 * (C) 2011-2012 see Authors.txt
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

#include <WinDef.h>

#define RIFF_DWORD          0x46464952

#define AC3_SYNC_WORD           0x770b
#define TRUEHD_SYNC_WORD    0xba6f72f8
#define MLP_SYNC_WORD       0xbb6f72f8
#define IEC61937_SYNC_WORD  0x4e1ff872

#define DTS_SYNC_WORD       0x0180fe7f
#define DTSHD_SYNC_WORD     0x25205864

#define EAC3_FRAME_TYPE_INDEPENDENT  0
#define EAC3_FRAME_TYPE_DEPENDENT    1
#define EAC3_FRAME_TYPE_AC3_CONVERT  2
#define EAC3_FRAME_TYPE_RESERVED     3

int GetAC3FrameSize(const BYTE* buf); // for AC3
int GetEAC3FrameSize(const BYTE* buf); // for E-AC3
int GetMLPFrameSize(const BYTE* buf); // for TrueHD and MLP

int ParseAC3Header(const BYTE* buf, int* samplerate, int* channels, int* framelength, int* bitrate);
int ParseEAC3Header(const BYTE* buf, int* samplerate, int* channels, int* framelength, int* frametype);
int ParseMLPHeader(const BYTE* buf, int* samplerate, int* channels, int* framelength, WORD* bitdepth, bool* isTrueHD);      // for TrueHD and MLP
int ParseHdmvLPCMHeader(const BYTE* buf, int* samplerate, int* channels);

DWORD GetDefChannelMask(WORD nChannels);
DWORD GetVorbisChannelMask(WORD nChannels);
