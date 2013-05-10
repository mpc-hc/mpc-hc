/*
 * (C) 2009-2013 see Authors.txt
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
#include "DVBChannel.h"


CDVBChannel::CDVBChannel()
    : m_ulFrequency(0)
    , m_nPrefNumber(0)
    , m_nOriginNumber(0)
    , m_bEncrypted(false)
    , m_bNowNextFlag(false)
    , m_ulONID(0)
    , m_ulTSID(0)
    , m_ulSID(0)
    , m_ulPMT(0)
    , m_ulPCR(0)
    , m_ulVideoPID(0)
    , m_nVideoType(DVB_MPV)
    , m_nVideoFps(DVB_FPS_25_0)
    , m_nVideoChroma(DVB_Chroma_4_2_0)
    , m_nVideoWidth(0)
    , m_nVideoHeight(0)
    , m_nVideoAR(DVB_AR_NULL)
    , m_nAudioCount(0)
    , m_nDefaultAudio(0)
    , m_nSubtitleCount(0)
    , m_nDefaultSubtitle(-1)
{
}

CDVBChannel::~CDVBChannel()
{
}

void CDVBChannel::FromString(CString strValue)
{
    int i = 0;
    int nVersion;

    nVersion        = _tstol(strValue.Tokenize(_T("|"), i));
    m_strName       = strValue.Tokenize(_T("|"), i);
    m_ulFrequency   = _tstol(strValue.Tokenize(_T("|"), i));
    m_nPrefNumber   = _tstol(strValue.Tokenize(_T("|"), i));
    m_nOriginNumber = _tstol(strValue.Tokenize(_T("|"), i));
    if (nVersion > FORMAT_VERSION_0) {
        m_bEncrypted = !!_tstol(strValue.Tokenize(_T("|"), i));
    }
    if (nVersion > FORMAT_VERSION_1) {
        m_bNowNextFlag = !!_tstol(strValue.Tokenize(_T("|"), i));
    }
    m_ulONID      = _tstol(strValue.Tokenize(_T("|"), i));
    m_ulTSID      = _tstol(strValue.Tokenize(_T("|"), i));
    m_ulSID       = _tstol(strValue.Tokenize(_T("|"), i));
    m_ulPMT       = _tstol(strValue.Tokenize(_T("|"), i));
    m_ulPCR       = _tstol(strValue.Tokenize(_T("|"), i));
    m_ulVideoPID  = _tstol(strValue.Tokenize(_T("|"), i));
    m_nVideoType  = (DVB_STREAM_TYPE) _tstol(strValue.Tokenize(_T("|"), i));
    m_nAudioCount = _tstol(strValue.Tokenize(_T("|"), i));
    if (nVersion > FORMAT_VERSION_1) {
        m_nDefaultAudio = _tstol(strValue.Tokenize(_T("|"), i));
    }
    m_nSubtitleCount = _tstol(strValue.Tokenize(_T("|"), i));
    if (nVersion > FORMAT_VERSION_2) {
        m_nDefaultSubtitle = _tstol(strValue.Tokenize(_T("|"), i));
    }

    for (int j = 0; j < m_nAudioCount; j++) {
        m_Audios[j].PID = _tstol(strValue.Tokenize(_T("|"), i));
        m_Audios[j].Type = (DVB_STREAM_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
        m_Audios[j].PesType = (PES_STREAM_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
        m_Audios[j].Language = strValue.Tokenize(_T("|"), i);
    }

    for (int j = 0; j < m_nSubtitleCount; j++) {
        m_Subtitles[j].PID = _tstol(strValue.Tokenize(_T("|"), i));
        m_Subtitles[j].Type = (DVB_STREAM_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
        m_Subtitles[j].PesType = (PES_STREAM_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
        m_Subtitles[j].Language = strValue.Tokenize(_T("|"), i);
    }
    if (nVersion > FORMAT_VERSION_3) {
        m_nVideoFps  = (DVB_FPS_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
        m_nVideoChroma = (DVB_CHROMA_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
        m_nVideoWidth = _tstol(strValue.Tokenize(_T("|"), i));
        m_nVideoHeight = _tstol(strValue.Tokenize(_T("|"), i));
        m_nVideoAR = (DVB_AspectRatio_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
    }

}

CString CDVBChannel::ToString()
{
    CString strValue;
    strValue.AppendFormat(_T("%d|%s|%ld|%d|%d|%d|%d|%ld|%ld|%ld|%ld|%ld|%ld|%ld|%d|%ld|%d|%d"),
                          FORMAT_VERSION_CURRENT,
                          m_strName,
                          m_ulFrequency,
                          m_nPrefNumber,
                          m_nOriginNumber,
                          m_bEncrypted,
                          m_bNowNextFlag,
                          m_ulONID,
                          m_ulTSID,
                          m_ulSID,
                          m_ulPMT,
                          m_ulPCR,
                          m_ulVideoPID,
                          m_nVideoType,
                          m_nAudioCount,
                          m_nDefaultAudio,
                          m_nSubtitleCount,
                          m_nDefaultSubtitle);

    for (int i = 0; i < m_nAudioCount; i++) {
        if (m_Audios[i].Language.IsEmpty()) {
            m_Audios[i].Language = " ";
        }
        strValue.AppendFormat(_T("|%ld|%d|%d|%s"), m_Audios[i].PID, m_Audios[i].Type, m_Audios[i].PesType, m_Audios[i].Language);
    }

    for (int i = 0; i < m_nSubtitleCount; i++) {
        if (m_Subtitles[i].Language.IsEmpty()) {
            m_Subtitles[i].Language = " ";
        }
        strValue.AppendFormat(_T("|%ld|%d|%d|%s"), m_Subtitles[i].PID, m_Subtitles[i].Type, m_Subtitles[i].PesType, m_Subtitles[i].Language);
    }

    strValue.AppendFormat(_T("|%d|%d|%ld|%ld|%d"),
                          m_nVideoFps,
                          m_nVideoChroma,
                          m_nVideoWidth,
                          m_nVideoHeight,
                          m_nVideoAR);
    return strValue;
}

void CDVBChannel::AddStreamInfo(ULONG ulPID, DVB_STREAM_TYPE nType, PES_STREAM_TYPE nPesType, LPCTSTR strLanguage)
{
    switch (nType) {
        case DVB_MPV:
            m_ulVideoPID = ulPID;
            m_nVideoType = nType;
            break;
        case DVB_H264:
            m_ulVideoPID = ulPID;
            m_nVideoType = nType;
            break;
        case DVB_MPA:
        case DVB_AC3:
        case DVB_EAC3:
            if (m_nAudioCount < DVB_MAX_AUDIO) {
                m_Audios[m_nAudioCount].PID = ulPID;
                m_Audios[m_nAudioCount].Type = nType;
                m_Audios[m_nAudioCount].PesType = nPesType;
                m_Audios[m_nAudioCount].Language = strLanguage;
                m_nAudioCount++;
            }
            break;
        case DVB_LATM:
            if (m_nAudioCount < DVB_MAX_AUDIO) {
                m_Audios[m_nAudioCount].PID = ulPID;
                m_Audios[m_nAudioCount].Type = nType;
                m_Audios[m_nAudioCount].PesType = nPesType;
                m_Audios[m_nAudioCount].Language = strLanguage;
                m_nAudioCount++;
            }
            break;
        case DVB_SUBTITLE:
            if (m_nSubtitleCount < DVB_MAX_SUBTITLE) {
                m_Subtitles[m_nSubtitleCount].PID = ulPID;
                m_Subtitles[m_nSubtitleCount].Type = nType;
                m_Subtitles[m_nSubtitleCount].PesType = nPesType;
                m_Subtitles[m_nSubtitleCount].Language = strLanguage;
                m_nSubtitleCount++;
            }
            break;
    }
}

REFERENCE_TIME CDVBChannel::GetAvgTimePerFrame()
{
    REFERENCE_TIME Value;
    switch (m_nVideoFps) {
        case DVB_FPS_23_976:
            Value = 417084;
            break;
        case DVB_FPS_24_0:
            Value = 416667;
            break;
        case DVB_FPS_25_0:
            Value = 400000;
            break;
        case DVB_FPS_29_97:
            Value = 333667;
            break;
        case DVB_FPS_30_0:
            Value = 333333;
            break;
        case DVB_FPS_50_0:
            Value = 200000;
            break;
        case DVB_FPS_59_94:
            Value = 166834;
            break;
        case DVB_FPS_60_0:
            Value = 166667;
            break;
        default:
            Value = 0;
            break;
    }
    return Value;
}

CString CDVBChannel::GetVideoFpsDesc()
{
    CString strValue;
    switch (m_nVideoFps) {
        case DVB_FPS_23_976:
            strValue = _T("23.976");
            break;
        case DVB_FPS_24_0:
            strValue = _T("24.000");
            break;
        case DVB_FPS_25_0:
            strValue = _T("25.000");
            break;
        case DVB_FPS_29_97:
            strValue = _T("29.970");
            break;
        case DVB_FPS_30_0:
            strValue = _T("30.000");
            break;
        case DVB_FPS_50_0:
            strValue = _T("50.000");
            break;
        case DVB_FPS_59_94:
            strValue = _T("59.940");
            break;
        case DVB_FPS_60_0:
            strValue = _T("60.000");
            break;
        default:
            strValue = _T("     -");
            break;
    }
    return strValue;

}

DWORD CDVBChannel::GetVideoARx()
{
    DWORD Value;
    switch (GetVideoAR()) {
        case DVB_AR_1:
            Value = 1;
            break;
        case DVB_AR_3_4:
            Value = 4;
            break;
        case DVB_AR_9_16:
            Value = 16;
            break;
        case DVB_AR_1_2_21:
            Value = 221;
            break;
        default:
            Value = 0;
            break;
    }
    return Value;
}

DWORD CDVBChannel::GetVideoARy()
{
    DWORD Value;
    switch (GetVideoAR()) {
        case DVB_AR_1:
            Value = 1;
            break;
        case DVB_AR_3_4:
            Value = 3;
            break;
        case DVB_AR_9_16:
            Value = 9;
            break;
        case DVB_AR_1_2_21:
            Value = 100;
            break;
        default:
            Value = 0;
            break;
    }
    return Value;
}
