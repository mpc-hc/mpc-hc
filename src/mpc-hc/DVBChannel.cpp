/*
 * (C) 2009-2017 see Authors.txt
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
#include "ISOLang.h"
#include "mplayerc.h"


LCID DVBStreamInfo::GetLCID() const
{
    return ISOLang::ISO6392ToLcid(CStringA(sLanguage));
};

CDVBChannel::CDVBChannel(CString strChannel)
{
    FromString(strChannel);
}

void CDVBChannel::FromString(CString strValue)
{
    int i = 0;

    int nVersion    = _tstol(strValue.Tokenize(_T("|"), i));
    // We don't try to parse versions newer than the one we support
    if (nVersion > FORMAT_VERSION_CURRENT) {
        AfxThrowInvalidArgException();
    }

    m_strName       = strValue.Tokenize(_T("|"), i);
    m_ulFrequency   = _tstol(strValue.Tokenize(_T("|"), i));
    m_ulBandwidth   = (nVersion > FORMAT_VERSION_4) ? _tstol(strValue.Tokenize(_T("|"), i))
                      : AfxGetAppSettings().iBDABandwidth * 1000;
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
        m_Audios[j].ulPID     = _tstol(strValue.Tokenize(_T("|"), i));
        m_Audios[j].nType     = (DVB_STREAM_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
        m_Audios[j].nPesType  = (PES_STREAM_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
        m_Audios[j].sLanguage = strValue.Tokenize(_T("|"), i);
    }

    for (int j = 0; j < m_nSubtitleCount; j++) {
        m_Subtitles[j].ulPID     = _tstol(strValue.Tokenize(_T("|"), i));
        m_Subtitles[j].nType     = (DVB_STREAM_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
        m_Subtitles[j].nPesType  = (PES_STREAM_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
        m_Subtitles[j].sLanguage = strValue.Tokenize(_T("|"), i);
    }

    if (nVersion > FORMAT_VERSION_3) {
        m_nVideoFps    = (DVB_FPS_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
        m_nVideoChroma = (DVB_CHROMA_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
        m_nVideoWidth  = _tstol(strValue.Tokenize(_T("|"), i));
        m_nVideoHeight = _tstol(strValue.Tokenize(_T("|"), i));
        m_nVideoAR     = (DVB_AspectRatio_TYPE)_tstol(strValue.Tokenize(_T("|"), i));
    }
}

CString CDVBChannel::ToString() const
{
    auto substituteEmpty = [](const CString & lang) -> CString {
        if (lang.IsEmpty())
        {
            return _T(" ");
        }
        return lang;
    };

    CString strValue;
    strValue.AppendFormat(_T("%d|%s|%lu|%lu|%d|%d|%d|%d|%lu|%lu|%lu|%lu|%lu|%lu|%d|%d|%d|%d|%d"),
                          FORMAT_VERSION_CURRENT,
                          m_strName.GetString(),
                          m_ulFrequency,
                          m_ulBandwidth,
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
        strValue.AppendFormat(_T("|%lu|%d|%d|%s"), m_Audios[i].ulPID, m_Audios[i].nType, m_Audios[i].nPesType, substituteEmpty(m_Audios[i].sLanguage).GetString());
    }

    for (int i = 0; i < m_nSubtitleCount; i++) {
        strValue.AppendFormat(_T("|%lu|%d|%d|%s"), m_Subtitles[i].ulPID, m_Subtitles[i].nType, m_Subtitles[i].nPesType, substituteEmpty(m_Subtitles[i].sLanguage).GetString());
    }

    strValue.AppendFormat(_T("|%d|%d|%lu|%lu|%d"),
                          m_nVideoFps,
                          m_nVideoChroma,
                          m_nVideoWidth,
                          m_nVideoHeight,
                          m_nVideoAR);

    return strValue;
}

CStringA CDVBChannel::ToJSON() const
{
    CStringA jsonChannel;
    jsonChannel.Format("{ \"index\" : %d, \"name\" : \"%s\" }",
                       m_nPrefNumber,
                       EscapeJSONString(UTF16To8(m_strName)).GetString());
    return jsonChannel;
}

void CDVBChannel::AddStreamInfo(ULONG ulPID, DVB_STREAM_TYPE nType, PES_STREAM_TYPE nPesType, LPCTSTR strLanguage)
{
    switch (nType) {
        case DVB_MPV:
        case DVB_H264:
        case DVB_HEVC:
            m_ulVideoPID = ulPID;
            m_nVideoType = nType;
            break;
        case DVB_MPA:
        case DVB_AC3:
        case DVB_EAC3:
        case DVB_LATM:
            if (m_nAudioCount < DVB_MAX_AUDIO) {
                m_Audios[m_nAudioCount].ulPID     = ulPID;
                m_Audios[m_nAudioCount].nType     = nType;
                m_Audios[m_nAudioCount].nPesType  = nPesType;
                m_Audios[m_nAudioCount].sLanguage = strLanguage;
                m_nAudioCount++;
            }
            break;
        case DVB_SUBTITLE:
            if (m_nSubtitleCount < DVB_MAX_SUBTITLE) {
                m_Subtitles[m_nSubtitleCount].ulPID     = ulPID;
                m_Subtitles[m_nSubtitleCount].nType     = nType;
                m_Subtitles[m_nSubtitleCount].nPesType  = nPesType;
                m_Subtitles[m_nSubtitleCount].sLanguage = strLanguage;
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
