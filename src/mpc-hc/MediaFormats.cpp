/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2017 see Authors.txt
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
#include <atlbase.h>
#include <atlpath.h>
#include "MediaFormats.h"
#include "resource.h"

//
// CMediaFormatCategory
//

CMediaFormatCategory::CMediaFormatCategory()
    : m_fAudioOnly(false)
    , m_engine(DirectShow)
    , m_fAssociable(true)
{
}

CMediaFormatCategory::CMediaFormatCategory(
    CString label, CString description, CAtlList<CString>& exts, bool fAudioOnly,
    CString specreqnote, engine_t engine, bool fAssociable)
{
    m_label = label;
    m_description = description;
    m_exts.AddTailList(&exts);
    m_backupexts.AddTailList(&m_exts);
    m_specreqnote = specreqnote;
    m_fAudioOnly = fAudioOnly;
    m_engine = engine;
    m_fAssociable = fAssociable;
}

CMediaFormatCategory::CMediaFormatCategory(
    CString label, CString description, CString exts, bool fAudioOnly,
    CString specreqnote, engine_t engine, bool fAssociable)
{
    m_label = label;
    m_description = description;
    ExplodeMin(exts, m_exts, ' ');
    POSITION pos = m_exts.GetHeadPosition();
    while (pos) {
        m_exts.GetNext(pos).TrimLeft('.');
    }

    m_backupexts.AddTailList(&m_exts);
    m_specreqnote = specreqnote;
    m_fAudioOnly = fAudioOnly;
    m_engine = engine;
    m_fAssociable = fAssociable;
}

CMediaFormatCategory::~CMediaFormatCategory()
{
}

void CMediaFormatCategory::UpdateData(bool fSave)
{
    if (fSave) {
        AfxGetApp()->WriteProfileString(_T("FileFormats"), m_label, GetExts(true));
    } else {
        SetExts(AfxGetApp()->GetProfileString(_T("FileFormats"), m_label, GetExts(true)));
    }
}

CMediaFormatCategory::CMediaFormatCategory(const CMediaFormatCategory& mfc)
{
    *this = mfc;
}

CMediaFormatCategory& CMediaFormatCategory::operator = (const CMediaFormatCategory& mfc)
{
    if (this != &mfc) {
        m_label = mfc.m_label;
        m_description = mfc.m_description;
        m_specreqnote = mfc.m_specreqnote;
        m_exts.RemoveAll();
        m_exts.AddTailList(&mfc.m_exts);
        m_backupexts.RemoveAll();
        m_backupexts.AddTailList(&mfc.m_backupexts);
        m_fAudioOnly = mfc.m_fAudioOnly;
        m_engine = mfc.m_engine;
        m_fAssociable = mfc.m_fAssociable;
    }
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
    while (pos) {
        POSITION cur = pos;
        CString& ext = m_exts.GetNext(pos);
        if (ext[0] == '\\') {
            m_engine = (engine_t)_tcstol(ext.TrimLeft('\\'), nullptr, 10);
            m_exts.RemoveAt(cur);
        } else {
            ext.TrimLeft('.');
        }
    }
}

CString CMediaFormatCategory::GetFilter() const
{
    CString filter;
    POSITION pos = m_exts.GetHeadPosition();
    while (pos) {
        filter += _T("*.") + m_exts.GetNext(pos) + _T(";");
    }
    filter.TrimRight(_T(';')); // cheap...
    return filter;
}

CString CMediaFormatCategory::GetExts(bool fAppendEngine) const
{
    CString exts = Implode(m_exts, ' ');
    if (fAppendEngine) {
        exts += CString(_T(" \\")) + (TCHAR)(0x30 + (int)m_engine);
    }
    return exts;
}

CString CMediaFormatCategory::GetExtsWithPeriod(bool fAppendEngine) const
{
    CString exts;
    POSITION pos = m_exts.GetHeadPosition();
    while (pos) {
        exts += _T(".") + m_exts.GetNext(pos) + _T(" ");
    }
    exts.TrimRight(_T(' ')); // cheap...
    if (fAppendEngine) {
        exts += CString(_T(" \\")) + (TCHAR)(0x30 + (int)m_engine);
    }
    return exts;
}

CString CMediaFormatCategory::GetBackupExtsWithPeriod(bool fAppendEngine) const
{
    CString exts;
    POSITION pos = m_backupexts.GetHeadPosition();
    while (pos) {
        exts += _T(".") + m_backupexts.GetNext(pos) + _T(" ");
    }
    exts.TrimRight(_T(' ')); // cheap...
    if (fAppendEngine) {
        exts += CString(_T(" \\")) + (TCHAR)(0x30 + (int)m_engine);
    }
    return exts;
}

//
//  CMediaFormats
//

CMediaFormats::CMediaFormats()
    : m_iRtspHandler((engine_t)(int)RealMedia)
    , m_fRtspFileExtFirst(1)
{
}

CMediaFormats::~CMediaFormats()
{
}

void CMediaFormats::UpdateData(bool fSave)
{
    if (fSave) {
        AfxGetApp()->WriteProfileString(_T("FileFormats"), nullptr, nullptr);

        AfxGetApp()->WriteProfileInt(_T("FileFormats"), _T("RtspHandler"), m_iRtspHandler);
        AfxGetApp()->WriteProfileInt(_T("FileFormats"), _T("RtspFileExtFirst"), m_fRtspFileExtFirst);
    } else {
        RemoveAll();

#define ADDFMT(f) Add(CMediaFormatCategory##f)

        ADDFMT((_T("avi"),         StrRes(IDS_MFMT_AVI),         _T("avi")));
        ADDFMT((_T("mpeg"),        StrRes(IDS_MFMT_MPEG),        _T("mpg mpeg mpe m1v m2v mpv2 mp2v pva evo m2p")));
        ADDFMT((_T("mpegts"),      StrRes(IDS_MFMT_MPEGTS),      _T("ts tp trp m2t m2ts mts rec ssif")));
        ADDFMT((_T("dvdvideo"),    StrRes(IDS_MFMT_DVDVIDEO),    _T("vob ifo")));
        ADDFMT((_T("mkv"),         StrRes(IDS_MFMT_MKV),         _T("mkv mk3d")));
        ADDFMT((_T("webm"),        StrRes(IDS_MFMT_WEBM),        _T("webm")));
        ADDFMT((_T("mp4"),         StrRes(IDS_MFMT_MP4),         _T("mp4 m4v mp4v mpv4 hdmov")));
        ADDFMT((_T("mov"),         StrRes(IDS_MFMT_MOV),         _T("mov")));
        ADDFMT((_T("3gp"),         StrRes(IDS_MFMT_3GP),         _T("3gp 3gpp 3ga")));
        ADDFMT((_T("3g2"),         StrRes(IDS_MFMT_3G2),         _T("3g2 3gp2")));
        ADDFMT((_T("flv"),         StrRes(IDS_MFMT_FLV),         _T("flv f4v")));
        ADDFMT((_T("ogm"),         StrRes(IDS_MFMT_OGM),         _T("ogm ogv")));
        ADDFMT((_T("rm"),          StrRes(IDS_MFMT_RM),          _T("rm rmvb")));
#ifdef _WIN64
        ADDFMT((_T("rt"),          StrRes(IDS_MFMT_RT),          _T("rt ram rpm rmm rp smi smil")));
#else
        ADDFMT((_T("rt"),          StrRes(IDS_MFMT_RT),          _T("rt ram rpm rmm rp smi smil"), false, _T("RealPlayer or Real Alternative"), RealMedia));
#endif
        ADDFMT((_T("wmv"),         StrRes(IDS_MFMT_WMV),         _T("wmv wmp wm asf")));
        ADDFMT((_T("bink"),        StrRes(IDS_MFMT_BINK),        _T("smk bik")));
        ADDFMT((_T("flic"),        StrRes(IDS_MFMT_FLIC),        _T("fli flc flic")));
        ADDFMT((_T("dsm"),         StrRes(IDS_MFMT_DSM),         _T("dsm dsv dsa dss")));
        ADDFMT((_T("ivf"),         StrRes(IDS_MFMT_IVF),         _T("ivf")));
        ADDFMT((_T("swf"),         StrRes(IDS_MFMT_SWF),         _T("swf"), false, _T("ShockWave ActiveX control"), ShockWave));
        ADDFMT((_T("other"),       StrRes(IDS_MFMT_OTHER),       _T("divx amv mxf")));
        ADDFMT((_T("ac3"),         StrRes(IDS_MFMT_AC3),         _T("ac3"), true));
        ADDFMT((_T("dts"),         StrRes(IDS_MFMT_DTS),         _T("dts dtshd dtsma"), true));
        ADDFMT((_T("aiff"),        StrRes(IDS_MFMT_AIFF),        _T("aif aifc aiff"), true));
        ADDFMT((_T("alac"),        StrRes(IDS_MFMT_ALAC),        _T("alac"), true));
        ADDFMT((_T("amr"),         StrRes(IDS_MFMT_AMR),         _T("amr"), true));
        ADDFMT((_T("ape"),         StrRes(IDS_MFMT_APE),         _T("ape apl"), true));
        ADDFMT((_T("au"),          StrRes(IDS_MFMT_AU),          _T("au snd"), true));
        ADDFMT((_T("audiocd"),     StrRes(IDS_MFMT_CDA),         _T("cda"), true));
        ADDFMT((_T("flac"),        StrRes(IDS_MFMT_FLAC),        _T("flac"), true));
        ADDFMT((_T("m4a"),         StrRes(IDS_MFMT_M4A),         _T("m4a m4b m4r aac"), true));
        ADDFMT((_T("midi"),        StrRes(IDS_MFMT_MIDI),        _T("mid midi rmi"), true));
        ADDFMT((_T("mka"),         StrRes(IDS_MFMT_MKA),         _T("mka"), true));
        ADDFMT((_T("mp3"),         StrRes(IDS_MFMT_MP3),         _T("mp3"), true));
        ADDFMT((_T("mpa"),         StrRes(IDS_MFMT_MPA),         _T("mpa mp2 m1a m2a"), true));
        ADDFMT((_T("mpc"),         StrRes(IDS_MFMT_MPC),         _T("mpc"), true));
        ADDFMT((_T("ofr"),         StrRes(IDS_MFMT_OFR),         _T("ofr ofs"), true));
        ADDFMT((_T("ogg"),         StrRes(IDS_MFMT_OGG),         _T("ogg oga"), true));
        ADDFMT((_T("opus"),        StrRes(IDS_MFMT_OPUS),        _T("opus"), true));
        ADDFMT((_T("ra"),          StrRes(IDS_MFMT_RA),          _T("ra"), true));
        ADDFMT((_T("tak"),         StrRes(IDS_MFMT_TAK),         _T("tak"), true));
        ADDFMT((_T("tta"),         StrRes(IDS_MFMT_TTA),         _T("tta"), true));
        ADDFMT((_T("wav"),         StrRes(IDS_MFMT_WAV),         _T("wav"), true));
        ADDFMT((_T("wma"),         StrRes(IDS_MFMT_WMA),         _T("wma"), true));
        ADDFMT((_T("wavpack"),     StrRes(IDS_MFMT_WV),          _T("wv"), true));
        ADDFMT((_T("other_audio"), StrRes(IDS_MFMT_OTHER_AUDIO), _T("aob mlp thd mpl spx caf"), true));
        ADDFMT((_T("pls"),         StrRes(IDS_MFMT_PLS),         _T("asx m3u m3u8 pls wvx wax wmx mpcpl")));
        ADDFMT((_T("bdpls"),       StrRes(IDS_MFMT_BDPLS),       _T("mpls bdmv")));
        ADDFMT((_T("rar"),         StrRes(IDS_MFMT_RAR),         _T("rar"), false, _T("RARFileSource"), DirectShow, false));
#undef ADDFMT

        m_iRtspHandler = (engine_t)AfxGetApp()->GetProfileInt(_T("FileFormats"), _T("RtspHandler"), (int)RealMedia);
        m_fRtspFileExtFirst = !!AfxGetApp()->GetProfileInt(_T("FileFormats"), _T("RtspFileExtFirst"), TRUE);
    }

    for (size_t i = 0; i < GetCount(); i++) {
        GetAt(i).UpdateData(fSave);
    }
}

engine_t CMediaFormats::GetRtspHandler(bool& fRtspFileExtFirst) const
{
    fRtspFileExtFirst = m_fRtspFileExtFirst;
    return m_iRtspHandler;
}

void CMediaFormats::SetRtspHandler(engine_t e, bool fRtspFileExtFirst)
{
    m_iRtspHandler = e;
    m_fRtspFileExtFirst = fRtspFileExtFirst;
}

bool CMediaFormats::IsUsingEngine(CString path, engine_t e) const
{
    return (GetEngine(path) == e);
}

engine_t CMediaFormats::GetEngine(CString path) const
{
    path.Trim().MakeLower();

    if (!m_fRtspFileExtFirst && path.Find(_T("rtsp://")) == 0) {
        return m_iRtspHandler;
    }

    CString ext = CPath(path).GetExtension();
    ext.MakeLower();
    if (!ext.IsEmpty()) {
        if (path.Find(_T("rtsp://")) == 0) {
            if (ext == _T(".ram") || ext == _T(".rm") || ext == _T(".ra")) {
                return RealMedia;
            }
            if (ext == _T(".qt") || ext == _T(".mov")) {
                return QuickTime;
            }
        }

        for (size_t i = 0; i < GetCount(); i++) {
            const CMediaFormatCategory& mfc = GetAt(i);
            if (mfc.FindExt(ext)) {
                return mfc.GetEngineType();
            }
        }
    }

    if (m_fRtspFileExtFirst && path.Find(_T("rtsp://")) == 0) {
        return m_iRtspHandler;
    }

    return DirectShow;
}

bool CMediaFormats::FindExt(CString ext, bool fAudioOnly, bool fAssociableOnly) const
{
    return (FindMediaByExt(ext, fAudioOnly, fAssociableOnly) != nullptr);
}

const CMediaFormatCategory* CMediaFormats::FindMediaByExt(CString ext, bool fAudioOnly, bool fAssociableOnly) const
{
    ext.TrimLeft(_T('.'));

    if (!ext.IsEmpty()) {
        for (size_t i = 0; i < GetCount(); i++) {
            const CMediaFormatCategory& mfc = GetAt(i);
            if ((!fAudioOnly || mfc.IsAudioOnly()) && (!fAssociableOnly || mfc.IsAssociable()) && mfc.FindExt(ext)) {
                return &mfc;
            }
        }
    }

    return nullptr;
}

void CMediaFormats::GetFilter(CString& filter, CAtlArray<CString>& mask) const
{
    CString strTemp;

    filter.AppendFormat(IsExtHidden() ? _T("%s|") : _T("%s (*.avi;*.mp4;*.mkv;...)|"), ResStr(IDS_AG_MEDIAFILES).GetString());
    mask.Add(_T(""));

    for (size_t i = 0; i < GetCount(); i++) {
        strTemp = GetAt(i).GetFilter() + _T(";");
        mask[0] += strTemp;
        filter  += strTemp;
    }
    mask[0].TrimRight(_T(';'));
    filter.TrimRight(_T(';'));
    filter += _T("|");

    for (size_t i = 0; i < GetCount(); i++) {
        const CMediaFormatCategory& mfc = GetAt(i);
        filter += mfc.GetDescription() + _T("|" + GetAt(i).GetFilter() + _T("|"));
        mask.Add(mfc.GetFilter());
    }

    filter.AppendFormat(IDS_AG_ALLFILES);
    mask.Add(_T("*.*"));

    filter += _T("|");
}

void CMediaFormats::GetAudioFilter(CString& filter, CAtlArray<CString>& mask) const
{
    CString strTemp;

    filter.AppendFormat(IsExtHidden() ? _T("%s|") : _T("%s (*.mp3;*.aac;*.wav;...)|"), ResStr(IDS_AG_AUDIOFILES).GetString());
    mask.Add(_T(""));

    for (size_t i = 0; i < GetCount(); i++) {
        const CMediaFormatCategory& mfc = GetAt(i);
        if (!mfc.IsAudioOnly() || mfc.GetEngineType() != DirectShow) {
            continue;
        }
        strTemp  = GetAt(i).GetFilter() + _T(";");
        mask[0] += strTemp;
        filter  += strTemp;
    }

    mask[0].TrimRight(_T(';'));
    filter.TrimRight(_T(';'));
    filter += _T("|");

    for (size_t i = 0; i < GetCount(); i++) {
        const CMediaFormatCategory& mfc = GetAt(i);
        if (!mfc.IsAudioOnly() || mfc.GetEngineType() != DirectShow) {
            continue;
        }
        filter += mfc.GetDescription() + _T("|") + GetAt(i).GetFilter() + _T("|");
        mask.Add(mfc.GetFilter());
    }

    filter.AppendFormat(IDS_AG_ALLFILES);
    mask.Add(_T("*.*"));

    filter += _T("|");
}

bool CMediaFormats::IsExtHidden()
{
    CRegKey key;
    if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"), KEY_READ)) {
        DWORD value;
        if (ERROR_SUCCESS == key.QueryDWORDValue(_T("HideFileExt"), value)) {
            return !!value;
        }
    }
    return false;
}
