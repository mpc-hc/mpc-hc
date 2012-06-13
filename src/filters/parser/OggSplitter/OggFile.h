/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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

#include "../BaseSplitter/BaseSplitter.h"

#pragma pack(push, 1)

struct OggPageHeader {
    DWORD capture_pattern;
    BYTE stream_structure_version;
    BYTE header_type_flag;
    enum {continued = 1, first = 2, last = 4};
    __int64 granule_position;
    DWORD bitstream_serial_number;
    DWORD page_sequence_number;
    DWORD CRC_checksum;
    BYTE number_page_segments;
};

struct OggVorbisIdHeader {
    DWORD vorbis_version;
    BYTE audio_channels;
    DWORD audio_sample_rate;
    DWORD bitrate_maximum;
    DWORD bitrate_nominal;
    DWORD bitrate_minimum;
    BYTE blocksize_0: 4;
    BYTE blocksize_1: 4;
    BYTE framing_flag;
};

struct OggVideoHeader {
    DWORD w, h;
};

struct OggAudioHeader {
    WORD nChannels, nBlockAlign;
    DWORD nAvgBytesPerSec;
};

struct OggStreamHeader {
    char streamtype[8], subtype[4];
    DWORD size;
    __int64 time_unit, samples_per_unit;
    DWORD default_len;
    DWORD buffersize;
    WORD bps;
    WORD alignmentfix1;
    union {
        OggVideoHeader v;
        OggAudioHeader a;
    };
    DWORD alignmentfix2;
};
#pragma pack(pop)

class OggPage : public CAtlArray<BYTE>
{
public:
    OggPageHeader m_hdr;
    CAtlList<int> m_lens;
    OggPage() {
        memset(&m_hdr, 0, sizeof(m_hdr));
    }
};

class COggFile : public CBaseSplitterFile
{
    HRESULT Init();

public:
    COggFile(IAsyncReader* pAsyncReader, HRESULT& hr);

    bool Sync(HANDLE hBreak = NULL);
    bool Read(OggPageHeader& hdr, HANDLE hBreak = NULL);
    bool Read(OggPage& page, bool fFull = true, HANDLE hBreak = NULL);
};
