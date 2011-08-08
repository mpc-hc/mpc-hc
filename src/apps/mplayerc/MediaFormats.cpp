/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
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
#include <atlpath.h>
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
	CString label, CString description, CAtlList<CString>& exts, bool fAudioOnly,
	CString specreqnote, engine_t engine)
{
	m_label = label;
	m_description = description;
	m_exts.AddTailList(&exts);
	m_backupexts.AddTailList(&m_exts);
	m_specreqnote = specreqnote;
	m_fAudioOnly = fAudioOnly;
	m_engine = engine;
}

CMediaFormatCategory::CMediaFormatCategory(
	CString label, CString description, CString exts, bool fAudioOnly,
	CString specreqnote, engine_t engine)
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
			m_engine = (engine_t)_tcstol(ext.TrimLeft('\\'), NULL, 10);
			m_exts.RemoveAt(cur);
		} else {
			ext.TrimLeft('.');
		}
	}
}

CString CMediaFormatCategory::GetFilter()
{
	CString filter;
	POSITION pos = m_exts.GetHeadPosition();
	while (pos) {
		filter += _T("*.") + m_exts.GetNext(pos) + _T(";");
	}
	filter.TrimRight(_T(";")); // cheap...
	return(filter);
}

CString CMediaFormatCategory::GetExts(bool fAppendEngine)
{
	CString exts = Implode(m_exts, ' ');
	if (fAppendEngine) {
		exts += CString(_T(" \\")) + (TCHAR)(0x30 + (int)m_engine);
	}
	return(exts);
}

CString CMediaFormatCategory::GetExtsWithPeriod(bool fAppendEngine)
{
	CString exts;
	POSITION pos = m_exts.GetHeadPosition();
	while (pos) {
		exts += _T(".") + m_exts.GetNext(pos) + _T(" ");
	}
	exts.TrimRight(_T(" ")); // cheap...
	if (fAppendEngine) {
		exts += CString(_T(" \\")) + (TCHAR)(0x30 + (int)m_engine);
	}
	return(exts);
}

CString CMediaFormatCategory::GetBackupExtsWithPeriod(bool fAppendEngine)
{
	CString exts;
	POSITION pos = m_backupexts.GetHeadPosition();
	while (pos) {
		exts += _T(".") + m_backupexts.GetNext(pos) + _T(" ");
	}
	exts.TrimRight(_T(" ")); // cheap...
	if (fAppendEngine) {
		exts += CString(_T(" \\")) + (TCHAR)(0x30 + (int)m_engine);
	}
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
	if (fSave) {
		AfxGetApp()->WriteProfileString(_T("FileFormats"), NULL, NULL);

		AfxGetApp()->WriteProfileInt(_T("FileFormats"), _T("RtspHandler"), m_iRtspHandler);
		AfxGetApp()->WriteProfileInt(_T("FileFormats"), _T("RtspFileExtFirst"), m_fRtspFileExtFirst);
	} else {
		RemoveAll();

#define ADDFMT(f) Add(CMediaFormatCategory##f)

		ADDFMT((_T("avi"),      ResStr(IDS_MFMT_AVI),      _T("avi")));
		ADDFMT((_T("mpeg"),     ResStr(IDS_MFMT_MPEG),     _T("mpg mpeg mpe m1v m2v mpv2 mp2v pva evo m2p")));
		ADDFMT((_T("mpegts"),   ResStr(IDS_MFMT_MPEGTS),   _T("ts tp trp m2t m2ts mts rec")));
		ADDFMT((_T("dvdvideo"), ResStr(IDS_MFMT_DVDVIDEO), _T("vob ifo")));
		ADDFMT((_T("mkv"),      ResStr(IDS_MFMT_MKV),      _T("mkv")));
		ADDFMT((_T("webm"),     ResStr(IDS_MFMT_WEBM),     _T("webm")));
		ADDFMT((_T("mp4"),      ResStr(IDS_MFMT_MP4),      _T("mp4 m4v mp4v mpv4 hdmov")));
		ADDFMT((_T("mov"),      ResStr(IDS_MFMT_MOV),      _T("mov"), false, _T("QuickTime or QT Lite")));
		ADDFMT((_T("3gp"),      ResStr(IDS_MFMT_3GP),      _T("3gp 3gpp")));
#ifdef _WIN64
		ADDFMT((_T("3g2"),      ResStr(IDS_MFMT_3G2),      _T("3g2 3gp2")));
#else
		ADDFMT((_T("3g2"),      ResStr(IDS_MFMT_3G2),      _T("3g2 3gp2"), false, _T("QuickTime or QT Lite"), QuickTime));
#endif
		ADDFMT((_T("flv"),      ResStr(IDS_MFMT_FLV),      _T("flv f4v")));
		ADDFMT((_T("ogm"),      ResStr(IDS_MFMT_OGM),      _T("ogm ogv")));
#ifdef _WIN64
		ADDFMT((_T("rm"),       ResStr(IDS_MFMT_RM),       _T("rm ram rpm rmm")));
		ADDFMT((_T("rt"),       ResStr(IDS_MFMT_RT),       _T("rt rp smi smil")));
#else
		ADDFMT((_T("rm"),       ResStr(IDS_MFMT_RM),       _T("rm ram rpm rmm"), false, _T("RealPlayer or Real Alternative"), RealMedia));
		ADDFMT((_T("rt"),       ResStr(IDS_MFMT_RT),       _T("rt rp smi smil"), false, _T("RealPlayer or Real Alternative"), RealMedia));
#endif
		ADDFMT((_T("wmv"),      ResStr(IDS_MFMT_WMV),      _T("wmv wmp wm asf")));
		ADDFMT((_T("videocd"),  ResStr(IDS_MFMT_VIDEOCD),  _T("dat")));
		ADDFMT((_T("ratdvd"),   ResStr(IDS_MFMT_RATDVD),   _T("ratdvd"), false, _T("ratdvd media file")));
		ADDFMT((_T("bink"),     ResStr(IDS_MFMT_BINK),     _T("smk bik"), false, _T("smackw32/binkw32.dll in dll path")));
		ADDFMT((_T("flic"),     ResStr(IDS_MFMT_FLIC),     _T("fli flc flic")));
		ADDFMT((_T("dsm"),      ResStr(IDS_MFMT_DSM),      _T("dsm dsv dsa dss")));
		ADDFMT((_T("ivf"),      ResStr(IDS_MFMT_IVF),      _T("ivf")));
		ADDFMT((_T("dvd2avi"),  ResStr(IDS_MFMT_D2V),      _T("d2v")));
		ADDFMT((_T("swf"),      ResStr(IDS_MFMT_SWF),      _T("swf"), false, _T("ShockWave ActiveX control"), ShockWave));
		ADDFMT((_T("other"),    ResStr(IDS_MFMT_OTHER),    _T("divx rmvb amv")));
		ADDFMT((_T("ac3dts"),   ResStr(IDS_MFMT_AC3),      _T("ac3 dts"), true));
		ADDFMT((_T("aiff"),     ResStr(IDS_MFMT_AIFF),     _T("aif aifc aiff"), true));
		ADDFMT((_T("alac"),     ResStr(IDS_MFMT_ALAC),     _T("alac"), true));
		ADDFMT((_T("amr"),      ResStr(IDS_MFMT_AMR),      _T("amr"), true));
		ADDFMT((_T("ape"),      ResStr(IDS_MFMT_APE),      _T("ape apl"), true));
		ADDFMT((_T("au"),       ResStr(IDS_MFMT_AU),       _T("au snd"), true));
		ADDFMT((_T("audiocd"),  ResStr(IDS_MFMT_CDA),      _T("cda"), true));
		ADDFMT((_T("flac"),     ResStr(IDS_MFMT_FLAC),     _T("flac"), true));
#ifdef _WIN64
		ADDFMT((_T("m4a"),      ResStr(IDS_MFMT_M4A),      _T("m4a m4b aac"), true));
#else
		ADDFMT((_T("m4a"),      ResStr(IDS_MFMT_M4A),      _T("m4a m4b aac"), true, _T(""), QuickTime));
#endif
		ADDFMT((_T("midi"),     ResStr(IDS_MFMT_MIDI),     _T("mid midi rmi"), true));
		ADDFMT((_T("mka"),      ResStr(IDS_MFMT_MKA),      _T("mka"), true));
		ADDFMT((_T("mp3"),      ResStr(IDS_MFMT_MP3),      _T("mp3"), true));
		ADDFMT((_T("mpa"),      ResStr(IDS_MFMT_MPA),      _T("mpa mp2 m1a m2a"), true));
		ADDFMT((_T("mpc"),      ResStr(IDS_MFMT_MPC),      _T("mpc"), true));
		ADDFMT((_T("ofr"),      ResStr(IDS_MFMT_OFR),      _T("ofr ofs"), true));
		ADDFMT((_T("ogg"),      ResStr(IDS_MFMT_OGG),      _T("ogg oga"), true));
#ifdef _WIN64
		ADDFMT((_T("ra"),       ResStr(IDS_MFMT_RA),       _T("ra"), true));
#else
		ADDFMT((_T("ra"),       ResStr(IDS_MFMT_RA),       _T("ra"), true, _T("RealPlayer or Real Alternative"), RealMedia));
#endif
		ADDFMT((_T("tta"),      ResStr(IDS_MFMT_TTA),      _T("tta"), true));
		ADDFMT((_T("wav"),      ResStr(IDS_MFMT_WAV),      _T("wav"), true));
		ADDFMT((_T("wma"),      ResStr(IDS_MFMT_WMA),      _T("wma"), true));
		ADDFMT((_T("wavpack"),  ResStr(IDS_MFMT_WV),       _T("wv"), true));
		ADDFMT((_T("pls"),      ResStr(IDS_MFMT_PLS),      _T("asx m3u m3u8 pls wvx wax wmx mpcpl")));
		ADDFMT((_T("bdpls"),    ResStr(IDS_MFMT_BDPLS),    _T("mpls bdmv")));
#undef ADDFMT

		m_iRtspHandler = (engine_t)AfxGetApp()->GetProfileInt(_T("FileFormats"), _T("RtspHandler"), (int)RealMedia);
		m_fRtspFileExtFirst = !!AfxGetApp()->GetProfileInt(_T("FileFormats"), _T("RtspFileExtFirst"), 1);
	}

	for (int i = 0; i < GetCount(); i++) {
		GetAt(i).UpdateData(fSave);
	}
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

		for (int i = 0; i < GetCount(); i++) {
			CMediaFormatCategory& mfc = GetAt(i);
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

bool CMediaFormats::FindExt(CString ext, bool fAudioOnly)
{
	ext.TrimLeft(_T("."));

	if (!ext.IsEmpty()) {
		for (int i = 0; i < GetCount(); i++) {
			CMediaFormatCategory& mfc = GetAt(i);
			if ((!fAudioOnly || mfc.IsAudioOnly()) && mfc.FindExt(ext)) {
				return(true);
			}
		}
	}

	return(false);
}

void CMediaFormats::GetFilter(CString& filter, CAtlArray<CString>& mask)
{
	CString		strTemp;

	filter += ResStr(IDS_AG_MEDIAFILES);
	mask.Add(_T(""));

	for (int i = 0; i < GetCount(); i++) {
		strTemp  = GetAt(i).GetFilter() + _T(";");
		mask[0] += strTemp;
		filter  += strTemp;
	}
	mask[0].TrimRight(_T(";"));
	filter.TrimRight(_T(";"));
	filter += _T("|");

	for (int i = 0; i < GetCount(); i++) {
		CMediaFormatCategory& mfc = GetAt(i);
		filter += mfc.GetDescription() + _T("|" + GetAt(i).GetFilter() + _T("|"));
		mask.Add(mfc.GetFilter());
	}

	filter += ResStr(IDS_AG_ALLFILES);
	mask.Add(_T("*.*"));

	filter += _T("|");
}

void CMediaFormats::GetAudioFilter(CString& filter, CAtlArray<CString>& mask)
{
	CString		strTemp;
	filter += ResStr(IDS_AG_AUDIOFILES);
	mask.Add(_T(""));

	for (int i = 0; i < GetCount(); i++) {
		CMediaFormatCategory& mfc = GetAt(i);
		if (!mfc.IsAudioOnly() || mfc.GetEngineType() != DirectShow) {
			continue;
		}
		strTemp  = GetAt(i).GetFilter() + _T(";");
		mask[0] += strTemp;
		filter  += strTemp;
	}

	mask[0].TrimRight(_T(";"));
	filter.TrimRight(_T(";"));
	filter += _T("|");

	for (int i = 0; i < GetCount(); i++) {
		CMediaFormatCategory& mfc = GetAt(i);
		if (!mfc.IsAudioOnly() || mfc.GetEngineType() != DirectShow) {
			continue;
		}
		filter += mfc.GetDescription() + _T("|") + GetAt(i).GetFilter() + _T("|");
		mask.Add(mfc.GetFilter());
	}

	filter += ResStr(IDS_AG_ALLFILES);
	mask.Add(_T("*.*"));

	filter += _T("|");
}
