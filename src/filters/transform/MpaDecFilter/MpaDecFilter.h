/*
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

#include <atlcoll.h>
// TODO: remove this when it's fixed in MSVC
// Work around warning C4005: 'XXXX' : macro redefinition
#pragma warning(push)
#pragma warning(disable: 4005)
#include <stdint.h>
#pragma warning(pop)
#include "../../../DeCSS/DeCSSInputPin.h"
#include "IMpaDecFilter.h"
#include "MpaDecSettingsWnd.h"
#include "../../../mpc-hc/InternalFiltersConfig.h"

#define MPCAudioDecName L"MPC Audio Decoder"

enum {
    SPK_MONO = 0,
    SPK_STEREO,
    SPK_4_0,
    SPK_5_1,
    SPK_7_1
};

struct ps2_state_t {
    bool sync;
    double a[2], b[2];
    ps2_state_t() {
        reset();
    }
    void reset() {
        sync = false;
        a[0] = a[1] = b[0] = b[1] = 0;
    }
};

struct DD_stats_t {
protected:
    int mode;
    unsigned int ac3_frames;
    unsigned int eac3_frames;

public:
    void Reset();
    bool Desired(int type);
};

struct audio_params_t {
    DWORD layout;
    WORD  channels;
    void Reset() {
        layout   = 0;
        channels = 0;
    }
    bool LayoutUpdate(WORD channels_new, DWORD layout_new) {
        if (layout == layout_new && channels == channels_new) {
            return false;
        }
        layout   = layout_new;
        channels = channels_new;
        return true;
    }
};

struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct AVCodecParserContext;
struct AVAudioResampleContext;

class __declspec(uuid("3D446B6F-71DE-4437-BE15-8CE47174340F"))
    CMpaDecFilter
    : public CTransformFilter
    , public IMpaDecFilter
    , public ISpecifyPropertyPages2
{
protected:
    CCritSec m_csReceive;

    audio_params_t          m_InputParams;
    // Mixer
    AVAudioResampleContext* m_pAVRCxt;

    ps2_state_t             m_ps2_state;

#if defined(STANDALONE_FILTER) || HAS_FFMPEG_AUDIO_DECODERS
    // === FFmpeg variables
    AVCodec*                m_pAVCodec;
    AVCodecContext*         m_pAVCtx;
    AVCodecParserContext*   m_pParser;
    AVFrame*                m_pFrame;
#endif

    CAtlArray<BYTE> m_buff;
    REFERENCE_TIME m_rtStart;
    bool m_fDiscontinuity;

#if defined(STANDALONE_FILTER) || INTERNAL_DECODER_LPCM
    HRESULT ProcessLPCM();
    HRESULT ProcessHdmvLPCM(bool bAlignOldBuffer);
#endif
#if defined(STANDALONE_FILTER) || INTERNAL_DECODER_AC3
    HRESULT ProcessAC3();
    HRESULT ProcessAC3_SPDIF();
#endif
#if defined(STANDALONE_FILTER) || INTERNAL_DECODER_DTS
    HRESULT ProcessDTS_SPDIF();
#endif
#if defined(STANDALONE_FILTER) || INTERNAL_DECODER_PS2AUDIO
    HRESULT ProcessPS2PCM();
    HRESULT ProcessPS2ADPCM();
#endif
#if defined(STANDALONE_FILTER) || INTERNAL_DECODER_PCM
    HRESULT ProcessPCMraw();
    HRESULT ProcessPCMintBE();
    HRESULT ProcessPCMintLE();
    HRESULT ProcessPCMfloatBE();
    HRESULT ProcessPCMfloatLE();
#endif

    HRESULT GetDeliveryBuffer(IMediaSample** pSample, BYTE** pData);
    HRESULT Deliver(CAtlArray<float>& pBuff, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask = 0);
    HRESULT DeliverBitstream(BYTE* pBuff, int size, int sample_rate, int frame_length, BYTE type);
    HRESULT ReconnectOutput(int nSamples, CMediaType& mt);
    CMediaType CreateMediaType(MPCSampleFormat sf, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask = 0);
    CMediaType CreateMediaTypeSPDIF(DWORD nSamplesPerSec = 48000);
    HRESULT Mixing(float* pOutput, WORD out_ch, DWORD out_layout, float* pInput, int samples, WORD in_ch, DWORD in_layout);

#if defined(STANDALONE_FILTER) || HAS_FFMPEG_AUDIO_DECODERS
    bool    InitFFmpeg(enum AVCodecID nCodecId);
    void    ffmpeg_stream_finish();
    HRESULT DeliverFFmpeg(enum AVCodecID nCodecId, BYTE* p, int samples, int& size);
    HRESULT ProcessFFmpeg(enum AVCodecID nCodecId);
    static void LogLibavcodec(void* par, int level, const char* fmt, va_list valist);

    BYTE*   m_pFFBuffer;
    int     m_nFFBufferSize;

    enum AVCodecID FindCodec(const GUID subtype);

    struct {
        int flavor;
        int coded_frame_size;
        int sub_packet_h;
        int sub_packet_size;
        unsigned int deint_id;
    } m_raData;

    HRESULT ParseRealAudioHeader(const BYTE* extra, const int extralen);
#endif

protected:
    CCritSec m_csProps;
    MPCSampleFormat m_iSampleFormat;
    bool m_fMixer;
    int  m_iMixerLayout;
    bool m_fDRC;
    bool m_fSPDIF[etcount];

    bool m_bResync;
    DD_stats_t m_DDstats;

public:
    CMpaDecFilter(LPUNKNOWN lpunk, HRESULT* phr);
    virtual ~CMpaDecFilter();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    HRESULT EndOfStream();
    HRESULT BeginFlush();
    HRESULT EndFlush();
    HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
    HRESULT Receive(IMediaSample* pIn);

    HRESULT CheckInputType(const CMediaType* mtIn);
    HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
    HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
    HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);

    HRESULT StartStreaming();
    HRESULT StopStreaming();

    HRESULT SetMediaType(PIN_DIRECTION dir, const CMediaType* pmt);

    // ISpecifyPropertyPages2

    STDMETHODIMP GetPages(CAUUID* pPages);
    STDMETHODIMP CreatePage(const GUID& guid, IPropertyPage** ppPage);

    // IMpaDecFilter

    STDMETHODIMP SetSampleFormat(MPCSampleFormat sf);
    STDMETHODIMP_(MPCSampleFormat) GetSampleFormat();
    STDMETHODIMP SetMixer(bool fMixer);
    STDMETHODIMP_(bool) GetMixer();
    STDMETHODIMP SetMixerLayout(int sc);
    STDMETHODIMP_(int) GetMixerLayout();
    STDMETHODIMP SetDynamicRangeControl(bool fDRC);
    STDMETHODIMP_(bool) GetDynamicRangeControl();
    STDMETHODIMP SetSPDIF(enctype et, bool fSPDIF);
    STDMETHODIMP_(bool) GetSPDIF(enctype et);

    STDMETHODIMP SaveSettings();
};

class CMpaDecInputPin : public CDeCSSInputPin
{
public:
    CMpaDecInputPin(CTransformFilter* pFilter, HRESULT* phr, LPWSTR pName);
};
