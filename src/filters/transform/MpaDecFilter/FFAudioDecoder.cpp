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

#include "stdafx.h"
#include "FFAudioDecoder.h"

#pragma warning(disable: 4005 4244)
extern "C" {
#include "ffmpeg/libavcodec/avcodec.h"
#include "ffmpeg/libavutil/intreadwrite.h"
#include "ffmpeg/libavutil/opt.h"
}
#pragma warning(default: 4005 4244)


#include "moreuuids.h"
#include "../../../DSUtil/DSUtil.h"
#include "../../../DSUtil/AudioParser.h"
#include "../../../mpc-hc/InternalFiltersConfig.h"

#define INT16_PEAK  32768
#define INT32_PEAK  2147483648

typedef struct {
    const CLSID* clsMinorType;
    const enum AVCodecID nFFCodec;
} FFMPEG_AUDIO_CODECS;

static const FFMPEG_AUDIO_CODECS ffAudioCodecs[] = {
#if INTERNAL_DECODER_AMR
    // AMR
    { &MEDIASUBTYPE_AMR,        AV_CODEC_ID_AMR_NB },
    { &MEDIASUBTYPE_SAMR,       AV_CODEC_ID_AMR_NB },
    { &MEDIASUBTYPE_SAWB,       AV_CODEC_ID_AMR_WB },
#endif
#if INTERNAL_DECODER_AAC
    // AAC
    { &MEDIASUBTYPE_AAC,        AV_CODEC_ID_AAC },
    { &MEDIASUBTYPE_MP4A,       AV_CODEC_ID_AAC },
    { &MEDIASUBTYPE_mp4a,       AV_CODEC_ID_AAC },
    { &MEDIASUBTYPE_AAC_ADTS,   AV_CODEC_ID_AAC },
    { &MEDIASUBTYPE_LATM_AAC,   AV_CODEC_ID_AAC_LATM },
#endif
#if INTERNAL_DECODER_ALAC
    // ALAC
    { &MEDIASUBTYPE_ALAC,       AV_CODEC_ID_ALAC },
#endif
#if INTERNAL_DECODER_ALS
    // MPEG-4 ALS
    { &MEDIASUBTYPE_ALS,        AV_CODEC_ID_MP4ALS },
#endif
#if INTERNAL_DECODER_VORBIS
    // Ogg Vorbis
    { &MEDIASUBTYPE_Vorbis2,    AV_CODEC_ID_VORBIS },
#endif
#if INTERNAL_DECODER_NELLYMOSER
    // NellyMoser
    { &MEDIASUBTYPE_NELLYMOSER, AV_CODEC_ID_NELLYMOSER },
#endif
#if INTERNAL_DECODER_ADPCM
    // Qt ADPCM
    { &MEDIASUBTYPE_IMA4,       AV_CODEC_ID_ADPCM_IMA_QT },
    // FLV ADPCM
    { &MEDIASUBTYPE_ADPCM_SWF,  AV_CODEC_ID_ADPCM_SWF },
    // AMV IMA ADPCM
    { &MEDIASUBTYPE_ADPCM_AMV,  AV_CODEC_ID_ADPCM_IMA_AMV },
#endif
#if INTERNAL_DECODER_MPEGAUDIO
    // MPEG Audio
    { &MEDIASUBTYPE_MPEG1Packet,       AV_CODEC_ID_MP2 },
    { &MEDIASUBTYPE_MPEG1Payload,      AV_CODEC_ID_MP2 },
    { &MEDIASUBTYPE_MPEG1AudioPayload, AV_CODEC_ID_MP2 },
    { &MEDIASUBTYPE_MPEG2_AUDIO,       AV_CODEC_ID_MP2 },
    { &MEDIASUBTYPE_MP3,               AV_CODEC_ID_MP3 },
#endif
#if INTERNAL_DECODER_REALAUDIO
    // RealMedia Audio
    { &MEDIASUBTYPE_14_4,           AV_CODEC_ID_RA_144 },
    { &MEDIASUBTYPE_28_8,           AV_CODEC_ID_RA_288 },
    { &MEDIASUBTYPE_ATRC,           AV_CODEC_ID_ATRAC3 },
    { &MEDIASUBTYPE_COOK,           AV_CODEC_ID_COOK   },
    { &MEDIASUBTYPE_SIPR,           AV_CODEC_ID_SIPR   },
    { &MEDIASUBTYPE_RAAC,           AV_CODEC_ID_AAC    },
    { &MEDIASUBTYPE_RACP,           AV_CODEC_ID_AAC    },
    //{ &MEDIASUBTYPE_DNET,           AV_CODEC_ID_AC3    },
#endif
#if INTERNAL_DECODER_AC3
    // AC3, E-AC3, TrueHD, MLP
    //{ &MEDIASUBTYPE_DOLBY_AC3,      AV_CODEC_ID_AC3    },
    //{ &MEDIASUBTYPE_WAVE_DOLBY_AC3, AV_CODEC_ID_AC3    },
    { &MEDIASUBTYPE_DOLBY_DDPLUS,   AV_CODEC_ID_EAC3   },
    { &MEDIASUBTYPE_DOLBY_TRUEHD,   AV_CODEC_ID_TRUEHD },
    { &MEDIASUBTYPE_MLP,            AV_CODEC_ID_MLP    },
#endif
#if INTERNAL_DECODER_DTS
    // DTS
    { &MEDIASUBTYPE_DTS,            AV_CODEC_ID_DTS },
    { &MEDIASUBTYPE_WAVE_DTS,       AV_CODEC_ID_DTS },
#endif
#if INTERNAL_DECODER_FLAC
    // FLAC
    { &MEDIASUBTYPE_FLAC_FRAMED,    AV_CODEC_ID_FLAC },
#endif

    { &MEDIASUBTYPE_None,           AV_CODEC_ID_NONE },
};

enum AVCodecID FindCodec(const GUID subtype)
{
    for (int i = 0; i < _countof(ffAudioCodecs); i++) {
        if (subtype == *ffAudioCodecs[i].clsMinorType) {
            return ffAudioCodecs[i].nFFCodec;
        }
    }

    return AV_CODEC_ID_NONE;
}

static DWORD get_lav_channel_layout(uint64_t layout)
{
    if (layout > UINT32_MAX) {
        if (layout & AV_CH_WIDE_LEFT) {
            layout = (layout & ~AV_CH_WIDE_LEFT) | AV_CH_FRONT_LEFT_OF_CENTER;
        }
        if (layout & AV_CH_WIDE_RIGHT) {
            layout = (layout & ~AV_CH_WIDE_RIGHT) | AV_CH_FRONT_RIGHT_OF_CENTER;
        }

        if (layout & AV_CH_SURROUND_DIRECT_LEFT) {
            layout = (layout & ~AV_CH_SURROUND_DIRECT_LEFT) | AV_CH_SIDE_LEFT;
        }
        if (layout & AV_CH_SURROUND_DIRECT_RIGHT) {
            layout = (layout & ~AV_CH_SURROUND_DIRECT_RIGHT) | AV_CH_SIDE_RIGHT;
        }
    }

    return (DWORD)layout;
}

// CFFAudioDecoder

void CFFAudioDecoder::LogLibavcodec(void* par, int level, const char* fmt, va_list valist)
{
#if defined(_DEBUG) && 0
    if (level <= AV_LOG_VERBOSE) {
        char Msg [500];
        vsnprintf_s(Msg, sizeof(Msg), _TRUNCATE, fmt, valist);
        TRACE(_T("AVLIB : %s\n"), Msg);
    }
#endif
}

CFFAudioDecoder::CFFAudioDecoder()
    : m_pAVCodec(nullptr)
    , m_pAVCtx(nullptr)
    , m_pParser(nullptr)
    , m_pFrame(nullptr)
{
    memset(&m_raData, 0, sizeof(m_raData));
}

bool CFFAudioDecoder::Init(enum AVCodecID nCodecId, CTransformInputPin* pInput)
{
    if (nCodecId == AV_CODEC_ID_NONE) {
        return false;
    }

    bool bRet = false;

    avcodec_register_all();
    av_log_set_callback(LogLibavcodec);

    if (m_pAVCodec) {
        StreamFinish();
    }
    switch (nCodecId) {
        case AV_CODEC_ID_MP1:
            m_pAVCodec = avcodec_find_decoder_by_name("mp1float");
        case AV_CODEC_ID_MP2:
            m_pAVCodec = avcodec_find_decoder_by_name("mp2float");
        case AV_CODEC_ID_MP3:
            m_pAVCodec = avcodec_find_decoder_by_name("mp3float");
        default:
            m_pAVCodec = avcodec_find_decoder(nCodecId);
    }

    if (m_pAVCodec) {
        DWORD nSamples, nBytesPerSec;
        WORD nChannels, nBitsPerSample, nBlockAlign;
        audioFormatTypeHandler((BYTE*)pInput->CurrentMediaType().Format(), pInput->CurrentMediaType().FormatType(), &nSamples, &nChannels, &nBitsPerSample, &nBlockAlign, &nBytesPerSec);

        if (nCodecId == AV_CODEC_ID_AMR_NB || nCodecId == AV_CODEC_ID_AMR_WB) {
            nChannels = 1;
            nSamples  = 8000;
        }

        m_pAVCtx = avcodec_alloc_context3(m_pAVCodec);
        CheckPointer(m_pAVCtx, false);

        m_pAVCtx->sample_rate           = nSamples;
        m_pAVCtx->channels              = nChannels;
        m_pAVCtx->bit_rate              = nBytesPerSec << 3;
        m_pAVCtx->bits_per_coded_sample = nBitsPerSample;
        m_pAVCtx->block_align           = nBlockAlign;

        m_pAVCtx->err_recognition       = AV_EF_CAREFUL;
        m_pAVCtx->codec_id              = nCodecId;
        if (m_pAVCodec->capabilities & CODEC_CAP_TRUNCATED) {
            m_pAVCtx->flags            |= CODEC_FLAG_TRUNCATED;
        }

        if (nCodecId != AV_CODEC_ID_AAC) {
            m_pParser = av_parser_init(nCodecId);
        }

        const void* format = pInput->CurrentMediaType().Format();
        GUID format_type   = pInput->CurrentMediaType().formattype;
        DWORD formatlen    = pInput->CurrentMediaType().cbFormat;
        unsigned extralen  = 0;
        getExtraData((BYTE*)format, &format_type, formatlen, nullptr, &extralen);

        memset(&m_raData, 0, sizeof(m_raData));

        if (extralen) {
            if (nCodecId == AV_CODEC_ID_COOK || nCodecId == AV_CODEC_ID_ATRAC3 || nCodecId == AV_CODEC_ID_SIPR) {
                uint8_t* extra = (uint8_t*)av_mallocz(extralen + FF_INPUT_BUFFER_PADDING_SIZE);
                getExtraData((BYTE*)format, &format_type, formatlen, extra, nullptr);

                if (extra[0] == '.' && extra[1] == 'r' && extra[2] == 'a' && extra[3] == 0xfd) {
                    HRESULT hr = ParseRealAudioHeader(extra, extralen);
                    av_freep(&extra);
                    if (FAILED(hr)) {
                        return false;
                    }
                    if (nCodecId == AV_CODEC_ID_SIPR) {
                        if (m_raData.flavor > 3) {
                            TRACE(_T("FFAudioDecoder: Invalid SIPR flavor (%d)\n"), m_raData.flavor);
                            return false;
                        }
                        static BYTE sipr_subpk_size[4] = { 29, 19, 37, 20 };
                        m_pAVCtx->block_align = sipr_subpk_size[m_raData.flavor];
                    }
                } else {
                    // Try without any processing?
                    m_pAVCtx->extradata_size = extralen;
                    m_pAVCtx->extradata      = extra;
                }
            } else {
                m_pAVCtx->extradata_size = extralen;
                m_pAVCtx->extradata      = (uint8_t*)av_mallocz(m_pAVCtx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
                getExtraData((BYTE*)format, &format_type, formatlen, (BYTE*)m_pAVCtx->extradata, nullptr);
            }
        }

        if (avcodec_open2(m_pAVCtx, m_pAVCodec, nullptr) >= 0) {
            m_pFrame = avcodec_alloc_frame();
            bRet     = true;
        }
    }

    if (!bRet) {
        StreamFinish();
    }

    return bRet;
}

void CFFAudioDecoder::SetDRC(bool fDRC)
{
    if (m_pAVCtx) {
        AVCodecID codec_id = m_pAVCtx->codec_id;
        if (codec_id == AV_CODEC_ID_AC3 || codec_id == AV_CODEC_ID_EAC3) {
            av_opt_set_double(m_pAVCtx, "drc_scale", fDRC ? 1.0f : 0.0f, AV_OPT_SEARCH_CHILDREN);
        }
    }
}

HRESULT CFFAudioDecoder::Decode(enum AVCodecID nCodecId, BYTE* p, int buffsize, int& size, CAtlArray<BYTE>& BuffOut, enum AVSampleFormat& samplefmt)
{

    if (GetCodecId() == AV_CODEC_ID_NONE) {
        return E_FAIL;
    }

    size = 0;

    int got_frame = 0;
    bool b_use_parse = m_pParser && ((nCodecId == AV_CODEC_ID_TRUEHD) ? ((buffsize > 2000) ? true : false) : true); // Dirty hack for use with MPC MPEGSplitter

    AVPacket avpkt;
    av_init_packet(&avpkt);

    if (b_use_parse) {
        BYTE* pOut = nullptr;
        int pOut_size = 0;

        int used_bytes = av_parser_parse2(m_pParser, m_pAVCtx, &pOut, &pOut_size, p, buffsize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (used_bytes < 0) {
            TRACE(_T("FFAudioDecoder: audio parsing failed (ret: %d)\n"), -used_bytes);
            return E_FAIL;
        } else if (used_bytes == 0 && pOut_size == 0) {
            TRACE(_T("FFAudioDecoder: could not process buffer while parsing\n"));
        }

        size = used_bytes;

        if (pOut_size > 0) {
            avpkt.data = pOut;
            avpkt.size = pOut_size;

            int ret2 = avcodec_decode_audio4(m_pAVCtx, m_pFrame, &got_frame, &avpkt);
            if (ret2 < 0) {
                TRACE(_T("FFAudioDecoder: decoding failed despite successfull parsing\n"));
                return S_FALSE;
            }
        }
    } else {
        avpkt.data = p;
        avpkt.size = buffsize;

        int used_bytes = avcodec_decode_audio4(m_pAVCtx, m_pFrame, &got_frame, &avpkt);

        if (used_bytes < 0) {
            TRACE(_T("FFAudioDecoder: decoding failed\n"));
            return E_FAIL;
        } else if (used_bytes == 0 && !got_frame) {
            TRACE(_T("FFAudioDecoder: could not process buffer while decoding\n"));
        } else if (m_pAVCtx->channels > 8) {
            // sometimes avcodec_decode_audio4 cannot identify the garbage and produces incorrect data.
            // this code does not solve the problem, it only reduces the likelihood of crash.
            // do it better!
            return E_FAIL;
        }
        ASSERT(buffsize >= used_bytes);

        size = used_bytes;
    }

    if (got_frame) {
        size_t nSamples = m_pFrame->nb_samples;

        if (nSamples) {
            WORD nChannels = m_pAVCtx->channels;
            /*DWORD dwChannelMask;
            if (m_pAVCtx->channel_layout) {
                dwChannelMask = get_lav_channel_layout(m_pAVCtx->channel_layout);
            } else {
                dwChannelMask = GetDefChannelMask(nChannels);
            }*/
            samplefmt = m_pAVCtx->sample_fmt;
            size_t monosize = nSamples * av_get_bytes_per_sample(samplefmt);
            BuffOut.SetCount(monosize * nChannels);

            if (av_sample_fmt_is_planar(samplefmt)) {
                BYTE* pOut = BuffOut.GetData();
                for (int ch = 0; ch < nChannels; ++ch) {
                    memcpy(pOut, m_pFrame->extended_data[ch], monosize);
                    pOut += monosize;
                }
            } else {
                memcpy(BuffOut.GetData(), m_pFrame->data[0], BuffOut.GetCount());
            }
        }
    }

    return S_OK;
}

void CFFAudioDecoder::FlushBuffers()
{
    if (m_pParser) { // reset the parser
        av_parser_close(m_pParser);
        m_pParser = av_parser_init(GetCodecId());
    }
    if (m_pAVCtx) {
        avcodec_flush_buffers(m_pAVCtx);
    }
}

void CFFAudioDecoder::StreamFinish()
{
    m_pAVCodec = nullptr;
    if (m_pAVCtx) {
        if (m_pAVCtx->extradata) {
            av_freep(&m_pAVCtx->extradata);
        }
        if (m_pAVCtx->codec) {
            avcodec_close(m_pAVCtx);
        }
        av_freep(&m_pAVCtx);
    }

    if (m_pParser) {
        av_parser_close(m_pParser);
        m_pParser = nullptr;
    }

    if (m_pFrame) {
        av_freep(&m_pFrame);
    }
}

// RealAudio

HRESULT CFFAudioDecoder::ParseRealAudioHeader(const BYTE* extra, const int extralen)
{
    const uint8_t* fmt = extra + 4;
    uint16_t version = AV_RB16(fmt);
    fmt += 2;
    if (version == 3) {
        TRACE(_T("FFAudioDecoder: RealAudio Header version 3 unsupported\n"));
        return VFW_E_UNSUPPORTED_AUDIO;
    } else if (version == 4 || version == 5 && extralen > 50) {
        // main format block
        fmt += 2;   // word - unused (always 0)
        fmt += 4;   // byte[4] - .ra4/.ra5 signature
        fmt += 4;   // dword - unknown
        fmt += 2;   // word - Version2
        fmt += 4;   // dword - header size
        m_raData.flavor = AV_RB16(fmt);
        fmt += 2;   // word - codec flavor
        m_raData.coded_frame_size = AV_RB32(fmt);
        fmt += 4;   // dword - coded frame size
        fmt += 12;  // byte[12] - unknown
        m_raData.sub_packet_h = AV_RB16(fmt);
        fmt += 2;   // word - sub packet h
        fmt += 2;   // word - frame size
        m_raData.sub_packet_size = m_pAVCtx->block_align = AV_RB16(fmt);
        fmt += 2;   // word - subpacket size
        fmt += 2;   // word - unknown
        // 6 Unknown bytes in ver 5
        if (version == 5) {
            fmt += 6;
        }
        // Audio format block
        fmt += 8;
        // Tag info in v4
        if (version == 4) {
            int len = *fmt++;
            m_raData.deint_id = AV_RB32(fmt);
            fmt += len;
            len = *fmt++;
            fmt += len;
        } else if (version == 5) {
            m_raData.deint_id = AV_RB32(fmt);
            fmt += 4;
            fmt += 4;
        }
        fmt += 3;
        if (version == 5) {
            fmt++;
        }

        int ra_extralen = AV_RB32(fmt);
        if (ra_extralen > 0)  {
            m_pAVCtx->extradata_size = ra_extralen;
            m_pAVCtx->extradata      = (uint8_t*)av_mallocz(ra_extralen + FF_INPUT_BUFFER_PADDING_SIZE);
            memcpy((void*)m_pAVCtx->extradata, fmt + 4, ra_extralen);
        }
    } else {
        TRACE(_T("FFAudioDecoder: unknown RealAudio Header version: %d\n"), version);
        return VFW_E_UNSUPPORTED_AUDIO;
    }

    return S_OK;
}

bool CFFAudioDecoder::RealPrepare(BYTE* p, int buffsize, CPaddedArray& BuffOut)
{
    if (m_raData.deint_id == MAKEFOURCC('r', 'n', 'e', 'g') || m_raData.deint_id == MAKEFOURCC('r', 'p', 'i', 's')) {

        int w   = m_raData.coded_frame_size;
        int h   = m_raData.sub_packet_h;
        int len = w * h;

        if (buffsize >= len) {
            BuffOut.SetCount(len);
            BYTE* dest = BuffOut.GetData();

            int sps = m_raData.sub_packet_size;
            if (sps > 0 && m_raData.deint_id == MAKEFOURCC('r', 'n', 'e', 'g')) { // COOK and ATRAC codec
                for (int y = 0; y < h; y++) {
                    for (int x = 0, w2 = w / sps; x < w2; x++) {
                        memcpy(dest + sps * (h * x + ((h + 1) / 2) * (y & 1) + (y >> 1)), p, sps);
                        p += sps;
                    }
                }
                return true;
            }

            if (m_raData.deint_id == MAKEFOURCC('r', 'p', 'i', 's')) { // SIPR codec
                memcpy(dest, p, len);

                // http://mplayerhq.hu/pipermail/mplayer-dev-eng/2002-August/010569.html
                static BYTE sipr_swaps[38][2] = {
                    {0,  63}, {1,  22}, {2,  44}, {3,  90}, {5,  81}, {7,  31}, {8,  86}, {9,  58}, {10, 36}, {12, 68},
                    {13, 39}, {14, 73}, {15, 53}, {16, 69}, {17, 57}, {19, 88}, {20, 34}, {21, 71}, {24, 46},
                    {25, 94}, {26, 54}, {28, 75}, {29, 50}, {32, 70}, {33, 92}, {35, 74}, {38, 85}, {40, 56},
                    {42, 87}, {43, 65}, {45, 59}, {48, 79}, {49, 93}, {51, 89}, {55, 95}, {61, 76}, {67, 83},
                    {77, 80}
                };

                int bs = h * w * 2 / 96; // nibbles per subpacket
                for (int n = 0; n < 38; n++) {
                    int i = bs * sipr_swaps[n][0];
                    int o = bs * sipr_swaps[n][1];
                    // swap nibbles of block 'i' with 'o'
                    for (int j = 0; j < bs; j++) {
                        int x = (i & 1) ? (dest[(i >> 1)] >> 4) : (dest[(i >> 1)] & 15);
                        int y = (o & 1) ? (dest[(o >> 1)] >> 4) : (dest[(o >> 1)] & 15);
                        if (o & 1) {
                            dest[(o >> 1)] = (dest[(o >> 1)] & 0x0F) | (x << 4);
                        } else {
                            dest[(o >> 1)] = (dest[(o >> 1)] & 0xF0) | x;
                        }
                        if (i & 1) {
                            dest[(i >> 1)] = (dest[(i >> 1)] & 0x0F) | (y << 4);
                        } else {
                            dest[(i >> 1)] = (dest[(i >> 1)] & 0xF0) | y;
                        }
                        ++i;
                        ++o;
                    }
                }
                return true;
            }
        }
    }
    return false;
}

// Info

enum AVCodecID CFFAudioDecoder::GetCodecId()
{
    if (m_pAVCtx) {
        return m_pAVCtx->codec_id;
    }
    return AV_CODEC_ID_NONE;
}

enum AVSampleFormat CFFAudioDecoder::GetSampleFmt()
{
    return m_pAVCtx->sample_fmt;
}

DWORD CFFAudioDecoder::GetSampleRate()
{
    return (DWORD)m_pAVCtx->sample_rate;
}

WORD CFFAudioDecoder::GetChannels()
{
    return (WORD)m_pAVCtx->channels;

}

DWORD CFFAudioDecoder::GetChannelMask()
{
    return get_lav_channel_layout(m_pAVCtx->channel_layout);
}
