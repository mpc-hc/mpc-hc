/*
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

#include "stdafx.h"
#include "AudioParser.h"
#include "GolombBuffer.h"
#include <MMReg.h>

#define AC3_CHANNEL         0
#define AC3_MONO            1
#define AC3_STEREO          2
#define AC3_3F              3
#define AC3_2F1R            4
#define AC3_3F1R            5
#define AC3_2F2R            6
#define AC3_3F2R            7
#define AC3_CHANNEL1        8
#define AC3_CHANNEL2        9
#define AC3_DOLBY           10
#define AC3_CHANNEL_MASK    15
#define AC3_LFE             16

// Dolby Digital

int GetAC3FrameSize(const BYTE* buf)
{
    if (*(WORD*)buf != AC3_SYNC_WORD) { // syncword
        return 0;
    }
    if ((buf[5] >> 3) > 10) { // bsid
        return 0;
    }
    int frmsizecod = buf[4] & 0x3F;
    if (frmsizecod >= 38) {
        return 0;
    }

    int frame_size;
    static const int rates[] = { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 576, 640 };

    int rate  = rates[frmsizecod >> 1];
    switch (buf[4] & 0xc0) {
        case 0:
            frame_size = 4 * rate;
            break;
        case 0x40:
            frame_size = 2 * (320 * rate / 147 + (frmsizecod & 1));
            break;
        case 0x80:
            frame_size = 6 * rate;
            break;
        default:
            return 0;
    }

    return frame_size;
}

int GetEAC3FrameSize(const BYTE* buf)
{
    if (*(WORD*)buf != AC3_SYNC_WORD) { // syncword
        return 0;
    }
    BYTE bsid = buf[5] >> 3; // bsid
    if (bsid < 11 || bsid > 16) {
        return 0;
    }
    if ((buf[4] >> 4) == 0xf) {
        return 0;
    }

    int frame_size = (((buf[2] & 0x03) << 8) + buf[3] + 1) * 2;

    return frame_size;
}

int GetMLPFrameSize(const BYTE* buf)
{
    DWORD sync = *(DWORD*)(buf + 4);
    if (sync == TRUEHD_SYNC_WORD || sync == MLP_SYNC_WORD) {
        return (((buf[0] << 8) | buf[1]) & 0xfff) * 2;
    }
    return 0;
}


int ParseAC3Header(const BYTE* buf, int* samplerate, int* channels, int* framelength, int* bitrate)
{
    if (*(WORD*)buf != AC3_SYNC_WORD) { // syncword
        return 0;
    }

    BYTE bsid = buf[5] >> 3; // bsid
    if (bsid > 10) {
        return 0;
    }

    static const int rates[] = { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 576, 640 };
    static const unsigned char lfeon[8] = { 0x10, 0x10, 0x04, 0x04, 0x04, 0x01, 0x04, 0x01 };
    static const unsigned char halfrate[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3 };

    int frmsizecod = buf[4] & 0x3F;
    if (frmsizecod >= 38) {
        return 0;
    }

    int half = halfrate[bsid];
    int rate = rates[frmsizecod >> 1];
    *bitrate = (rate * 1000) >> half;
    int frame_size;
    switch (buf[4] & 0xc0) {
        case 0:
            *samplerate = 48000 >> half;
            frame_size  = 4 * rate;
            break;
        case 0x40:
            *samplerate = 44100 >> half;
            frame_size  = 2 * (320 * rate / 147 + (frmsizecod & 1));
            break;
        case 0x80:
            *samplerate = 32000 >> half;
            frame_size  = 6 * rate;
            break;
        default:
            return 0;
    }

    unsigned char acmod = buf[6] >> 5;
    unsigned char flags = ((((buf[6] & 0xf8) == 0x50) ? AC3_DOLBY : acmod) | ((buf[6] & lfeon[acmod]) ? AC3_LFE : 0));
    switch (flags & AC3_CHANNEL_MASK) {
        case AC3_MONO:
            *channels = 1;
            break;
        case AC3_CHANNEL:
        case AC3_STEREO:
        case AC3_CHANNEL1:
        case AC3_CHANNEL2:
        case AC3_DOLBY:
            *channels = 2;
            break;
        case AC3_2F1R:
        case AC3_3F:
            *channels = 3;
            break;
        case AC3_3F1R:
        case AC3_2F2R:
            *channels = 4;
            break;
        case AC3_3F2R:
            *channels = 5;
            break;
    }
    if (flags & AC3_LFE) {
        (*channels)++;
    }

    *framelength = 1536;
    return frame_size;
}

int ParseEAC3Header(const BYTE* buf, int* samplerate, int* channels, int* framelength, int* frametype)
{
    if (*(WORD*)buf != AC3_SYNC_WORD) { // syncword
        return 0;
    }

    BYTE bsid = buf[5] >> 3; // bsid
    if (bsid < 11 || bsid > 16) {
        return 0;
    }

    static const int sample_rates[] = { 48000, 44100, 32000, 24000, 22050, 16000 };
    static const int channels_tbl[] = { 2, 1, 2, 3, 3, 4, 4, 5 };
    static const int samples_tbl[]  = { 256, 512, 768, 1536 };

    int frame_size = (((buf[2] & 0x03) << 8) + buf[3] + 1) * 2;

    int fscod  =  buf[4] >> 6;
    int fscod2 = (buf[4] >> 4) & 0x03;

    if (fscod == 0x03 && fscod2 == 0x03) {
        return 0;
    }

    int acmod = (buf[4] >> 1) & 0x07;
    int lfeon =  buf[4] & 0x01;

    *frametype = (buf[2] >> 6) & 0x03;
    if (*frametype == EAC3_FRAME_TYPE_RESERVED) {
        return 0;
    }
    //int sub_stream_id = (buf[2] >> 3) & 0x07;
    *samplerate  = sample_rates[fscod == 0x03 ? 3 + fscod2 : fscod];
    *channels    = channels_tbl[acmod] + lfeon;
    *framelength = (fscod == 0x03) ? 1536 : samples_tbl[fscod2];

    return frame_size;
}

int ParseMLPHeader(const BYTE* buf, int* samplerate, int* channels, int* framelength, WORD* bitdepth, bool* isTrueHD)
{
    static const int sampling_rates[]           = { 48000, 96000, 192000, 0, 0, 0, 0, 0, 44100, 88200, 176400, 0, 0, 0, 0, 0 };
    static const unsigned char mlp_quants[16]   = { 16, 20, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static const unsigned char mlp_channels[32] = {     1,     2,      3, 4, 3, 4, 5, 3,     4,     5,      4, 5, 6, 4, 5, 4,
                                                        5,     6,      5, 5, 6, 0, 0, 0,     0,     0,      0, 0, 0, 0, 0, 0 };
    static const int channel_count[13] = {//   LR    C   LFE  LRs LRvh  LRc LRrs  Cs   Ts  LRsd  LRw  Cvh  LFE2
                                                2,   1,   1,   2,   2,   2,   2,   1,   1,   2,   2,   1,   1 };

    DWORD sync = *(DWORD*)(buf + 4);
    if (sync == TRUEHD_SYNC_WORD) {
        *isTrueHD = true;
    } else if (sync == MLP_SYNC_WORD) {
        *isTrueHD = false;
    } else {
        return 0;
    }

    int frame_size = (((buf[0] << 8) | buf[1]) & 0xfff) * 2;

    if (*isTrueHD) {
        *bitdepth = 24;
        *samplerate  = sampling_rates[buf[8] >> 4];
        *framelength = 40 << ((buf[8] >> 4) & 0x07);
        int chanmap_substream_1 = ((buf[ 9] & 0x0f) << 1) | (buf[10] >> 7);
        int chanmap_substream_2 = ((buf[10] & 0x1f) << 8) |  buf[11];
        int channel_map = chanmap_substream_2 ? chanmap_substream_2 : chanmap_substream_1;
        *channels = 0;
        for (int i = 0; i < 13; ++i) {
            *channels += channel_count[i] * ((channel_map >> i) & 1);
        }
    } else {
        *bitdepth    = mlp_quants[buf[8] >> 4];
        *samplerate  = sampling_rates[buf[9] >> 4];
        *framelength = 40 << ((buf[9] >> 4) & 0x07);
        *channels    = mlp_channels[buf[11] & 0x1f];
    }

    return frame_size;
}

// DTS

void dts14be_to_dts16be(const BYTE* source, BYTE* destination, int size)
{
    unsigned short* src = (unsigned short*)source;
    unsigned short* dst = (unsigned short*)destination;

    for (int i = 0, n = size / 16; i < n; i++) {
        unsigned short src_0 = (src[0] >> 8) | (src[0] << 8);
        unsigned short src_1 = (src[1] >> 8) | (src[1] << 8);
        unsigned short src_2 = (src[2] >> 8) | (src[2] << 8);
        unsigned short src_3 = (src[3] >> 8) | (src[3] << 8);
        unsigned short src_4 = (src[4] >> 8) | (src[4] << 8);
        unsigned short src_5 = (src[5] >> 8) | (src[5] << 8);
        unsigned short src_6 = (src[6] >> 8) | (src[6] << 8);
        unsigned short src_7 = (src[7] >> 8) | (src[7] << 8);

        dst[0] = (src_0 << 2)  | ((src_1 & 0x3fff) >> 12); // 14 + 2
        dst[1] = (src_1 << 4)  | ((src_2 & 0x3fff) >> 10); // 12 + 4
        dst[2] = (src_2 << 6)  | ((src_3 & 0x3fff) >> 8);  // 10 + 6
        dst[3] = (src_3 << 8)  | ((src_4 & 0x3fff) >> 6);  // 8  + 8
        dst[4] = (src_4 << 10) | ((src_5 & 0x3fff) >> 4);  // 6  + 10
        dst[5] = (src_5 << 12) | ((src_6 & 0x3fff) >> 2);  // 4  + 12
        dst[6] = (src_6 << 14) | (src_7 & 0x3fff);         // 2  + 14

        dst[0] = (dst[0] >> 8) | (dst[0] << 8);
        dst[1] = (dst[1] >> 8) | (dst[1] << 8);
        dst[2] = (dst[2] >> 8) | (dst[2] << 8);
        dst[3] = (dst[3] >> 8) | (dst[3] << 8);
        dst[4] = (dst[4] >> 8) | (dst[4] << 8);
        dst[5] = (dst[5] >> 8) | (dst[5] << 8);
        dst[6] = (dst[6] >> 8) | (dst[6] << 8);

        src += 8;
        dst += 7;
    }
}

void dts14le_to_dts16be(const BYTE* source, BYTE* destination, int size)
{
    unsigned short* src = (unsigned short*)source;
    unsigned short* dst = (unsigned short*)destination;

    for (int i = 0, n = size / 16; i < n; i++) {
        dst[0] = (src[0] << 2)  | ((src[1] & 0x3fff) >> 12); // 14 + 2
        dst[1] = (src[1] << 4)  | ((src[2] & 0x3fff) >> 10); // 12 + 4
        dst[2] = (src[2] << 6)  | ((src[3] & 0x3fff) >> 8);  // 10 + 6
        dst[3] = (src[3] << 8)  | ((src[4] & 0x3fff) >> 6);  // 8  + 8
        dst[4] = (src[4] << 10) | ((src[5] & 0x3fff) >> 4);  // 6  + 10
        dst[5] = (src[5] << 12) | ((src[6] & 0x3fff) >> 2);  // 4  + 12
        dst[6] = (src[6] << 14) | (src[7] & 0x3fff);         // 2  + 14

        dst[0] = (dst[0] >> 8) | (dst[0] << 8);
        dst[1] = (dst[1] >> 8) | (dst[1] << 8);
        dst[2] = (dst[2] >> 8) | (dst[2] << 8);
        dst[3] = (dst[3] >> 8) | (dst[3] << 8);
        dst[4] = (dst[4] >> 8) | (dst[4] << 8);
        dst[5] = (dst[5] >> 8) | (dst[5] << 8);
        dst[6] = (dst[6] >> 8) | (dst[6] << 8);

        src += 8;
        dst += 7;
    }
}

int ParseDTSHeader(const BYTE* buf, int* samplerate, int* channels, int* framelength, int* tr_bitrate)
{
    static const int dts_channels[16] = { 1, 2, 2, 2, 2, 3, 3, 4, 4, 5, 6, 6, 6, 7, 8, 8} ;
    static const int core_sample_rates[] = { 0, 8000, 16000, 32000, 0, 0, 11025, 22050, 44100, 0, 0, 12000, 24000, 48000, 96000, 192000 };
    static const int transmission_bitrates[32] = {
        32000,     56000,   64000,   96000,
        112000,   128000,  192000,  224000,
        256000,   320000,  384000,  448000,
        512000,   576000,  640000,  768000,
        960000,  1024000, 1152000, 1280000,
        1344000, 1408000, 1411200, 1472000,
        1536000, 1920000, 2048000, 3072000,
        3840000, 0, 0, 0 //open, variable, lossless
        // [15]  768000 is actually 754500 for DVD
        // [24] 1536000 is actually 1509000 for ???
        // [24] 1536000 is actually 1509750 for DVD
        // [22] 1411200 is actually 1234800 for 14-bit DTS-CD audio
    };

    unsigned int tmp;
    // minimum buffer size is 16
    DWORD sync = *(DWORD*)buf;
    BYTE hdr[14]; //minimum header size is 14
    bool isDTS14 = false;
    switch (sync) {
        case 0x0180fe7f:    // '7FFE8001' 16 bits and big endian bitstream
            memcpy(hdr, buf, 14);
            break;
        case 0x80017ffe:    // 'FE7F0180' 16 bits and little endian bitstream
            _swab((char*)buf, (char*)hdr, 14);
            break;
        case 0x00e8ff1f:    // '1FFFE800' 14 bits and big endian bitstream
            // not tested, need samples.
            dts14be_to_dts16be(buf, (BYTE*)hdr, 16);
            isDTS14 = true;
            break;
        case 0xe8001fff:    // 'FF1F00E8' 14 bits and little endian bitstream
            dts14le_to_dts16be(buf, (BYTE*)hdr, 16);
            isDTS14 = true;
            break;
        default:
            return 0;
    }
    ASSERT(*(DWORD*)hdr == 0x0180fe7f);

    *framelength = (((hdr[4] & 1) << 6 | (hdr[5] & 0xfc) >> 2) + 1) * 32;
    if (*framelength < 6 * 32) {
        return 0;
    }

    int frame_size = ((hdr[5] & 3) << 12 | hdr[6] << 4 | (hdr[7] & 0xf0) >> 4) + 1;
    if (frame_size < 96) {
        return 0;
    }
    if (isDTS14) {
        frame_size = frame_size * 8 / 14 * 2;
    }

    tmp = (hdr[7] & 0x0f) << 2 | (hdr[8] & 0xc0) >> 6;
    if (tmp >= 16) {
        return 0;
    }
    *channels = dts_channels[tmp];

    *samplerate = core_sample_rates[(hdr[8] & 0x3c) >> 2];
    if (*samplerate == 0) {
        return 0;
    }

    // transmission bitrate
    * tr_bitrate = transmission_bitrates[(hdr[8] & 3) << 3 | (hdr[9] & 0xe0) >> 5];

    // LFE
    if ((hdr[10] & 6) >> 1) {
        (*channels)++;
    }

    return frame_size;
}

// HDMV LPCM

int ParseHdmvLPCMHeader(const BYTE* buf, int* samplerate, int* channels)
{
    *samplerate = 0;
    *channels   = 0;

    int frame_size = buf[0] << 8 | buf[1];
    frame_size += 4; // add header size;

    static int channels_layout[] = { 0, 1, 0, 2, 3, 3, 4, 4, 5, 6, 7, 8, 0, 0, 0, 0 };
    BYTE channel_layout = buf[2] >> 4;
    *channels = channels_layout[channel_layout];
    if (!*channels) {
        return 0;
    }

    static int bitspersample[] = {0, 16, 20, 24};
    int bits_per_sample = bitspersample[buf[3] >> 6];
    if (!(bits_per_sample == 16 || bits_per_sample == 24)) {
        return 0;
    }

    static int freq[] = {0, 48000, 0, 0, 96000, 192000};
    *samplerate = freq[buf[2] & 0x0f];
    if (!(*samplerate == 48000 || *samplerate == 96000 || *samplerate == 192000)) {
        return 0;
    }

    return frame_size;
}

// LATM

static UINT32 LATMGetValue(CGolombBuffer& gb)
{
    BYTE bBytesForValue = (BYTE)gb.BitRead(2);
    UINT32 value = 0;
    for (int i = 0; i <= bBytesForValue; i++) {
        value *= 256; // 2^8
        value += gb.ReadByte();
    }
    return value;
}

static bool ParseLATMHeaderAudioSpecificConfig(CGolombBuffer& gb, int* samplerate, int* channels)
{
    static int sampling_frequency[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350 };
    static int channel_configuration[] = { 0, 1, 2, 3, 4, 5, 6, 8 };

    short sAudioObjectType = (short)gb.BitRead(5);
    if (sAudioObjectType == 31) {
        sAudioObjectType = 32 + (short)gb.BitRead(6);
    }

    BYTE bSamplingFrequencyIndex = (BYTE)gb.BitRead(4);
    if (bSamplingFrequencyIndex == 0xf) {
        *samplerate = (int)gb.BitRead(24);
    } else if (bSamplingFrequencyIndex < _countof(sampling_frequency)) {
        *samplerate = sampling_frequency[bSamplingFrequencyIndex];
    } else {
        return false;
    }

    BYTE bChannelConfiguration = (BYTE)gb.BitRead(4);
    if (bChannelConfiguration < _countof(channel_configuration)) {
        *channels = channel_configuration[bChannelConfiguration];
    } else {
        return false;
    }

    int sbrPresentFlag = -1;
    short sExtensionAudioObjectType;
    if (sAudioObjectType == 5) {
        sExtensionAudioObjectType = sAudioObjectType;
        sbrPresentFlag = 1;

        BYTE bExtensionSamplingFrequencyIndex = (BYTE)gb.BitRead(4);
        if (bExtensionSamplingFrequencyIndex == 0xf) {
            *samplerate = (int)gb.BitRead(24);
        } else if (bExtensionSamplingFrequencyIndex < _countof(sampling_frequency)) {
            *samplerate = sampling_frequency[bExtensionSamplingFrequencyIndex];
        } else {
            return false;
        }

        sAudioObjectType = (short)gb.BitRead(5);
        if (sAudioObjectType == 31) {
            sAudioObjectType = 32 + (short)gb.BitRead(6);
        }
    } else {
        sExtensionAudioObjectType = 0;
    }

    // The specs are unclear about that but it seems to work
    if (sbrPresentFlag == -1) {
        *samplerate *= 2;
    }

    return true;
}

static bool ParseLATMHeaderStreamMuxConfig(CGolombBuffer& gb, int* samplerate, int* channels)
{
    BYTE bAudioMuxVersion = (BYTE)gb.BitRead(1);
    BYTE bAudioMuxVersionA = 0; // default value is 0
    if (bAudioMuxVersion) {
        bAudioMuxVersionA = (BYTE)gb.BitRead(1);
    }

    bool success = false;

    if (bAudioMuxVersionA == 0) {
        if (bAudioMuxVersion == 1) {
            LATMGetValue(gb); // taraBufferFullness
        }
        gb.BitRead(1); // allStreamsSameTimeFraming
        gb.BitRead(6); // numSubFrames
        BYTE bNumProgram = (BYTE)gb.BitRead(4);

        BYTE bNumLayer = (BYTE)gb.BitRead(3);

        if (bNumProgram || bNumLayer) {
            ASSERT(0); // We support only bNumProgram = bNumLayer = 0
            return false;
        }

        BYTE bUseSameConfig = 0; // because bNumProgram = bNumLayer = 0
        if (!bUseSameConfig) {
            if (bAudioMuxVersion == 0) {
                success = ParseLATMHeaderAudioSpecificConfig(gb, samplerate, channels);
            }  else {
                UINT32 uAscLen = LATMGetValue(gb);
                success = ParseLATMHeaderAudioSpecificConfig(gb, samplerate, channels); // TODO uAscLen -= AudioSpecificConfig();
                // gb.BitRead((int)uAscLen); // fillBits
            }
        }
    }

    return success;
}

bool ParseAACLATMHeader(BYTE* buf, int len, int* samplerate, int* channels)
{
    *samplerate = 0;
    *channels   = 0;

    CGolombBuffer gb(buf, len);

    // Check LATM syncword
    if (gb.BitRead(11) != AAC_LATM_SYNC_WORD) {
        return false;
    }
    gb.BitRead(13); // audioMuxLengthBytes;

    bool success = false;
    // Parsing AudioMuxElement
    if (!gb.BitRead(1)) { // useSameStreamMux
        success = ParseLATMHeaderStreamMuxConfig(gb, samplerate, channels);
    }

    return success;
}

// Channel masks

DWORD GetDefChannelMask(WORD nChannels)
{
    switch (nChannels) {
        case 1: // 1.0 Mono (KSAUDIO_SPEAKER_MONO)
            return SPEAKER_FRONT_CENTER;
        case 2: // 2.0 Stereo (KSAUDIO_SPEAKER_STEREO)
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
        case 3: // 2.1 Stereo
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY;
        case 4: // 4.0 Quad (KSAUDIO_SPEAKER_QUAD)
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT
                   | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
        case 5: // 5.0
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER
                   | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
        case 6: // 5.1 Side (KSAUDIO_SPEAKER_5POINT1_SURROUND)
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER
                   | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
        case 7: // 6.1 Side
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER
                   | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_CENTER
                   | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
        case 8: // 7.1 Surround (KSAUDIO_SPEAKER_7POINT1_SURROUND)
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER
                   | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT
                   | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
        case 10: // 9.1 Surround
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER
                   | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT
                   | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT
                   | SPEAKER_TOP_FRONT_LEFT | SPEAKER_TOP_FRONT_RIGHT;
        default:
            return 0;
    }
}

DWORD GetVorbisChannelMask(WORD nChannels)
{
    // for Vorbis and FLAC
    // http://xiph.org/vorbis/doc/Vorbis_I_spec.html#x1-800004.3.9
    // http://flac.sourceforge.net/format.html#frame_header
    switch (nChannels) {
        case 1: // 1.0 Mono (KSAUDIO_SPEAKER_MONO)
            return SPEAKER_FRONT_CENTER;
        case 2: // 2.0 Stereo (KSAUDIO_SPEAKER_STEREO)
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
        case 3: // 3.0
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_RIGHT;
        case 4: // 4.0 Quad (KSAUDIO_SPEAKER_QUAD)
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT
                   | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
        case 5: // 5.0
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_RIGHT
                   | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
        case 6: // 5.1 (KSAUDIO_SPEAKER_5POINT1)
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_RIGHT
                   | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT
                   | SPEAKER_LOW_FREQUENCY;
        case 7: // 6.1 Side
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_RIGHT
                   | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT
                   | SPEAKER_BACK_CENTER
                   | SPEAKER_LOW_FREQUENCY;
        case 8: // 7.1 Surround (KSAUDIO_SPEAKER_7POINT1_SURROUND)
            return SPEAKER_FRONT_LEFT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_RIGHT
                   | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT
                   | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT
                   | SPEAKER_LOW_FREQUENCY;
        default:
            return 0;
    }
}
