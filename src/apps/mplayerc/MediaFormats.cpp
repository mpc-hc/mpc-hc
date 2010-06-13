/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <atlbase.h>
#include "MediaFormats.h"
#include "resource.h"

//
// CMediaFormatCategory
//

CMediaFormatCategory::CMediaFormatCategory()
    : m_fAudioOnly(false)
{
}

CMediaFormatCategory::CMediaFormatCategory(
    CString label, CAtlList<CString>& exts, bool fAudioOnly,
    CString specreqnote, engine_t engine)
{
    m_label = label;
    m_exts.AddTailList(&exts);
    m_backupexts.AddTailList(&m_exts);
    m_specreqnote = specreqnote;
    m_fAudioOnly = fAudioOnly;
    m_engine = engine;
}

CMediaFormatCategory::CMediaFormatCategory(
    CString label, CString exts, bool fAudioOnly,
    CString specreqnote, engine_t engine)
{
    m_label = label;
    ExplodeMin(exts, m_exts, ' ');
    POSITION pos = m_exts.GetHeadPosition();
    while(pos) m_exts.GetNext(pos).TrimLeft('.');

    m_backupexts.AddTailList(&m_exts);
    m_specreqnote = specreqnote;
    m_fAudioOnly = fAudioOnly;
    m_engine = engine;
}

CMediaFormatCategory::~CMediaFormatCategory()
{
}

void CMediaFormatCategory::UpdateData(bool fSave)
{
    if(fSave)
    {
        AfxGetApp()->WriteProfileString(_T("FileFormats"), m_label, GetExts(true));
    }
    else
    {
        SetExts(AfxGetApp()->GetProfileString(_T("FileFormats"), m_label, GetExts(true)));
    }
}

CMediaFormatCategory::CMediaFormatCategory(const CMediaFormatCategory& mfc)
{
    *this = mfc;
}

CMediaFormatCategory& CMediaFormatCategory::operator = (const CMediaFormatCategory& mfc)
{
    m_label = mfc.m_label;
    m_specreqnote = mfc.m_specreqnote;
    m_exts.RemoveAll();
    m_exts.AddTailList(&mfc.m_exts);
    m_backupexts.RemoveAll();
    m_backupexts.AddTailList(&mfc.m_backupexts);
    m_fAudioOnly = mfc.m_fAudioOnly;
    m_engine = mfc.m_engine;
    return *this;
}

void CMediaFormatCategory::RestoreDefaultExts()
{
    m_exts.RemoveAll();
    m_exts.AddTailList(&m_backupexts);
}

void CMediaFormatCategory::SetExts(CAtlList<CString>& exts)
{
    m_exts.RemoveAll();
    m_exts.AddTailList(&exts);
}

void CMediaFormatCategory::SetExts(CString exts)
{
    m_exts.RemoveAll();
    ExplodeMin(exts, m_exts, ' ');
    POSITION pos = m_exts.GetHeadPosition();
    while(pos)
    {
        POSITION cur = pos;
        CString& ext = m_exts.GetNext(pos);
        if(ext[0] == '\\')
        {
            m_engine = (engine_t)_tcstol(ext.TrimLeft('\\'), NULL, 10);
            m_exts.RemoveAt(cur);
        }
        else ext.TrimLeft('.');
    }
}

CString CMediaFormatCategory::GetFilter()
{
    CString filter;
    POSITION pos = m_exts.GetHeadPosition();
    while(pos) filter += _T("*.") + m_exts.GetNext(pos) + _T(";");
    filter.TrimRight(_T(";")); // cheap...
    return(filter);
}

CString CMediaFormatCategory::GetExts(bool fAppendEngine)
{
    CString exts = Implode(m_exts, ' ');
    if(fAppendEngine) exts += CString(_T(" \\")) + (TCHAR)(0x30 + (int)m_engine);
    return(exts);
}

CString CMediaFormatCategory::GetExtsWithPeriod(bool fAppendEngine)
{
    CString exts;
    POSITION pos = m_exts.GetHeadPosition();
    while(pos) exts += _T(".") + m_exts.GetNext(pos) + _T(" ");
    exts.TrimRight(_T(" ")); // cheap...
    if(fAppendEngine) exts += CString(_T(" \\")) + (TCHAR)(0x30 + (int)m_engine);
    return(exts);
}

CString CMediaFormatCategory::GetBackupExtsWithPeriod(bool fAppendEngine)
{
    CString exts;
    POSITION pos = m_backupexts.GetHeadPosition();
    while(pos) exts += _T(".") + m_backupexts.GetNext(pos) + _T(" ");
    exts.TrimRight(_T(" ")); // cheap...
    if(fAppendEngine) exts += CString(_T(" \\")) + (TCHAR)(0x30 + (int)m_engine);
    return(exts);
}

//
// 	CMediaFormats
//

CMediaFormats::CMediaFormats()
{
}

CMediaFormats::~CMediaFormats()
{
}

void CMediaFormats::UpdateData(bool fSave)
{
    if(fSave)
    {
        AfxGetApp()->WriteProfileString(_T("FileFormats"), NULL, NULL);

        AfxGetApp()->WriteProfileInt(_T("FileFormats"), _T("RtspHandler"), m_iRtspHandler);
        AfxGetApp()->WriteProfileInt(_T("FileFormats"), _T("RtspFileExtFirst"), m_fRtspFileExtFirst);
    }
    else
    {
        RemoveAll();

#define ADDFMT(f) Add(CMediaFormatCategory##f)
        ADDFMT((ResStr(IDS_MEDIAFORMATS_0),	  _T("wmv wmp wm asf")));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_1),   _T("wma"), true));
        ADDFMT((ResStr(IDS_AG_VIDEO_FILE),    _T("avi")));
        ADDFMT((ResStr(IDS_AG_AUDIO_FILE),    _T("wav"), true));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_4),   _T("mpg mpeg mpe m1v m2v mpv2 mp2v ts tp tpr pva m2ts m2t mts evo m2p")));
        ADDFMT((_T("VCD file"),               _T("dat")));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_5),   _T("mpa mp2 m1a m2a"), true));
        ADDFMT((ResStr(IDS_AG_DVD_FILE),      _T("vob ifo")));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_7),   _T("ac3 dts"), true));
        ADDFMT((_T("MP3 Format Sound"),       _T("mp3"), true));
        ADDFMT((ResStr(IDS_AG_MIDI_FILE),     _T("mid midi rmi"), true));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_9),   _T("ivf")));
        ADDFMT((_T("AIFF Format Sound"),      _T("aif aifc aiff"), true));
        ADDFMT((_T("AU Format Sound"),        _T("au snd"), true));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_10),  _T("ogm ogv")));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_11),  _T("ogg oga"), true));
        ADDFMT((_T("CD Audio Track"),         _T("cda"), true, ResStr(IDS_MEDIAFORMATS_12)));
        ADDFMT((ResStr(IDS_AG_FLIC_FILE),     _T("fli flc flic")));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_14),  _T("d2v")));
        ADDFMT((ResStr(IDS_AG_MPEG4_FILE),    _T("mp4 m4v mp4v mpv4 hdmov 3gp 3gpp")));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_16),  _T("m4a m4b aac"), true, _T(""), QuickTime));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_17),  _T("mkv")));
        ADDFMT((_T("WebM video file"),        _T("webm")));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_18),  _T("mka"), true));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_19),  _T("smk bik"), false, _T("smackw32/binkw32.dll in dll path")));
        ADDFMT((ResStr(IDS_AG_RATDVD_FILE),   _T("ratdvd"), false, _T("ratdvd media file")));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_21),  _T("roq")));
#ifdef _WIN64
        ADDFMT((ResStr(IDS_MEDIAFORMATS_22),  _T("rm ram rpm rmm")));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_23),  _T("ra"), true));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_24),  _T("rt rp smi smil")));
#else
        ADDFMT((ResStr(IDS_MEDIAFORMATS_22),  _T("rm ram rpm rmm"), false, _T("RealPlayer or Real Alternative"), RealMedia));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_23),  _T("ra"), true, _T("RealPlayer or Real Alternative"), RealMedia));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_24),  _T("rt rp smi smil"), false, _T("RealPlayer or Real Alternative"), RealMedia));
#endif
        ADDFMT((ResStr(IDS_MEDIAFORMATS_25),  _T("drc")));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_26),  _T("dsm dsv dsa dss")));
        ADDFMT((ResStr(IDS_AG_MUSEPACK_FILE), _T("mpc"), true));
        ADDFMT((_T("FLAC Audio file"),        _T("flac"), true));
        ADDFMT((_T("WavPack Audio file"),     _T("wv"), true));
        ADDFMT((_T("ALAC Audio file"),        _T("alac"), true));
        ADDFMT((_T("OptimFrog Audio file"),   _T("ofr ofs"), true));
        ADDFMT((_T("Monkey's Audio file"),    _T("ape apl"), true));
        ADDFMT((_T("True Audio file"),        _T("tta"), true));
        ADDFMT((_T("AMR Audio file"),         _T("amr"), true));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_28),  _T("flv iflv f4v")));
        ADDFMT((ResStr(IDS_MEDIAFORMATS_29),  _T("swf"), false, _T("ShockWave ActiveX control"), ShockWave));
#ifdef _WIN64
        ADDFMT((ResStr(IDS_MEDIAFORMATS_30),  _T("mov 3g2 3gp2")));
#else
        ADDFMT((ResStr(IDS_MEDIAFORMATS_30),  _T("mov 3g2 3gp2"), false, _T("QuickTime (Alternative)"), QuickTime));
#endif
        ADDFMT((ResStr(IDS_AG_PLAYLIST_FILE), _T("asx m3u pls wvx wax wmx mpcpl")));
        ADDFMT((_T("Blu-ray playlist file"),  _T("mpls bdmv")));
        ADDFMT((ResStr(IDS_AG_OTHER),		      _T("divx rmvb amv")));
#undef ADDFMT

        m_iRtspHandler = (engine_t)AfxGetApp()->GetProfileInt(_T("FileFormats"), _T("RtspHandler"), (int)RealMedia);
        m_fRtspFileExtFirst = !!AfxGetApp()->GetProfileInt(_T("FileFormats"), _T("RtspFileExtFirst"), 1);
    }

    for(int i = 0; i < GetCount(); i++)
        GetAt(i).UpdateData(fSave);
}

engine_t CMediaFormats::GetRtspHandler(bool& fRtspFileExtFirst)
{
    fRtspFileExtFirst = m_fRtspFileExtFirst;
    return m_iRtspHandler;
}

void CMediaFormats::SetRtspHandler(engine_t e, bool fRtspFileExtFirst)
{
    m_iRtspHandler = e;
    m_fRtspFileExtFirst = fRtspFileExtFirst;
}

bool CMediaFormats::IsUsingEngine(CString path, engine_t e)
{
    return(GetEngine(path) == e);
}

engine_t CMediaFormats::GetEngine(CString path)
{
    path.Trim().MakeLower();

    if(!m_fRtspFileExtFirst && path.Find(_T("rtsp://")) == 0)
        return m_iRtspHandler;

    CString ext = CPath(path).GetExtension();
    ext.MakeLower();
    if(!ext.IsEmpty())
    {
        if(path.Find(_T("rtsp://")) == 0)
        {
            if(ext == _T(".ram") || ext == _T(".rm") || ext == _T(".ra"))
                return RealMedia;
            if(ext == _T(".qt") || ext == _T(".mov"))
                return QuickTime;
        }

        for(int i = 0; i < GetCount(); i++)
        {
            CMediaFormatCategory& mfc = GetAt(i);
            if(mfc.FindExt(ext))
                return mfc.GetEngineType();
        }
    }

    if(m_fRtspFileExtFirst && path.Find(_T("rtsp://")) == 0)
        return m_iRtspHandler;

    return DirectShow;
}

bool CMediaFormats::FindExt(CString ext, bool fAudioOnly)
{
    ext.TrimLeft(_T("."));

    if(!ext.IsEmpty())
    {
        for(int i = 0; i < GetCount(); i++)
        {
            CMediaFormatCategory& mfc = GetAt(i);
            if((!fAudioOnly || mfc.IsAudioOnly()) && mfc.FindExt(ext))
                return(true);
        }
    }

    return(false);
}

void CMediaFormats::GetFilter(CString& filter, CAtlArray<CString>& mask)
{
    CString		strTemp;

    filter += ResStr(IDS_MEDIAFORMATS_34);
    mask.Add(_T(""));

    for(int i = 0; i < GetCount(); i++)
    {
        strTemp  = GetAt(i).GetFilter() + _T(";");
        mask[0] += strTemp;
        filter  += strTemp;
    }
    mask[0].TrimRight(_T(";"));
    filter.TrimRight(_T(";"));
    filter += _T("|");

    for(int i = 0; i < GetCount(); i++)
    {
        CMediaFormatCategory& mfc = GetAt(i);
        filter += mfc.GetLabel() + _T("|" + GetAt(i).GetFilter() + _T("|"));
        mask.Add(mfc.GetFilter());
    }

    filter += ResStr(IDS_MEDIAFORMATS_35);
    mask.Add(_T("*.*"));

    filter += _T("|");
}

void CMediaFormats::GetAudioFilter(CString& filter, CAtlArray<CString>& mask)
{
    CString		strTemp;
    filter += ResStr(IDS_MEDIAFORMATS_36);
    mask.Add(_T(""));

    for(int i = 0; i < GetCount(); i++)
    {
        CMediaFormatCategory& mfc = GetAt(i);
        if(!mfc.IsAudioOnly() || mfc.GetEngineType() != DirectShow) continue;
        strTemp  = GetAt(i).GetFilter() + _T(";");
        mask[0] += strTemp;
        filter  += strTemp;
    }

    mask[0].TrimRight(_T(";"));
    filter.TrimRight(_T(";"));
    filter += _T("|");

    for(int i = 0; i < GetCount(); i++)
    {
        CMediaFormatCategory& mfc = GetAt(i);
        if(!mfc.IsAudioOnly() || mfc.GetEngineType() != DirectShow) continue;
        filter += mfc.GetLabel() + _T("|") + GetAt(i).GetFilter() + _T("|");
        mask.Add(mfc.GetFilter());
    }

    filter += ResStr(IDS_MEDIAFORMATS_35);
    mask.Add(_T("*.*"));

    filter += _T("|");
}
