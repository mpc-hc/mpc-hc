/*
 * (C) 2012-2013 see Authors.txt
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

#include <atlcoll.h>
#include "PaddedArray.h"

struct AVCodec;
struct AVCodecContext;
struct AVCodecParserContext;
struct AVFrame;

enum AVCodecID FindCodec(const GUID subtype);

class CFFAudioDecoder
{
protected:
    AVCodec*              m_pAVCodec;
    AVCodecContext*       m_pAVCtx;
    AVCodecParserContext* m_pParser;
    AVFrame*              m_pFrame;

    struct {
        int flavor;
        int coded_frame_size;
        int sub_packet_h;
        int sub_packet_size;
        unsigned int deint_id;
    } m_raData;

    static void LogLibavcodec(void* par, int level, const char* fmt, va_list valist);

    HRESULT ParseRealAudioHeader(const BYTE* extra, const int extralen);

public:
    CFFAudioDecoder();

    bool    Init(enum AVCodecID nCodecId, CTransformInputPin* m_pInput);
    void    SetDRC(bool fDRC);

    bool    RealPrepare(BYTE* p, int buffsize, CPaddedArray& BuffOut);
    HRESULT Decode(enum AVCodecID nCodecId, BYTE* p, int buffsize, int& size, CAtlArray<BYTE>& BuffOut, enum AVSampleFormat& samplefmt);
    void    FlushBuffers();
    void    StreamFinish();

    // info
    enum AVCodecID      GetCodecId();   // safe
    enum AVSampleFormat GetSampleFmt(); // unsafe
    DWORD GetSampleRate();  // unsafe
    WORD  GetChannels();    // unsafe
    DWORD GetChannelMask(); // unsafe
};
