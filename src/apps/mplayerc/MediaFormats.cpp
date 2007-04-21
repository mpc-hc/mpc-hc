/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include <atlbase.h>
#include "MediaFormats.h"

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
		if(ext[0] == '\\') {m_engine = (engine_t)_tcstol(ext.TrimLeft('\\'), NULL, 10); m_exts.RemoveAt(cur);}
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
		ADDFMT((_T("Windows Media file"), _T("wmv wmp wm asf")));
		ADDFMT((_T("Windows Media Audio file"), _T("wma"), true));
		ADDFMT((_T("Video file"), _T("avi")));
		ADDFMT((_T("Audio file"), _T("wav"), true));
		ADDFMT((_T("MPEG Media file "), _T("mpg mpeg mpe m1v m2v mpv2 mp2v dat ts tp tpr pva pss")));
		ADDFMT((_T("MPEG Audio file"), _T("mpa mp2 m1a m2a"), true));
		ADDFMT((_T("DVD file"), _T("vob ifo")));
		ADDFMT((_T("DVD Audio file"), _T("ac3 dts"), true));
		ADDFMT((_T("MP3 Format Sound"), _T("mp3"), true));
		ADDFMT((_T("MIDI file"), _T("mid midi rmi"), true));
		ADDFMT((_T("Indeo Video file"), _T("ivf")));
		ADDFMT((_T("AIFF Format Sound"), _T("aif aifc aiff"), true));
		ADDFMT((_T("AU Format Sound"), _T("au snd"), true));
		ADDFMT((_T("Ogg Media file"), _T("ogm")));
		ADDFMT((_T("Ogg Vorbis Audio file"), _T("ogg"), true));
		ADDFMT((_T("CD Audio Track"), _T("cda"), true, _T("Windows 2000/XP or better")));
		ADDFMT((_T("FLIC file"), _T("fli flc flic")));
		ADDFMT((_T("DVD2AVI Project file"), _T("d2v")));
		ADDFMT((_T("MPEG4 file "), _T("mp4 m4v m4p m4b 3gp 3gpp 3g2 3gp2")));
		ADDFMT((_T("MPEG4 Audio file "), _T("m4a aac"), true));
		ADDFMT((_T("Matroska Media file"), _T("mkv")));
		ADDFMT((_T("Matroska Audio file"), _T("mka"), true));
		ADDFMT((_T("Smacker/Bink Media file"), _T("smk bik"), false, _T("smackw32/binkw32.dll in dll path")));
		ADDFMT((_T("ratdvd file"), _T("ratdvd"), false, _T("ratdvd media file")));
		ADDFMT((_T("RoQ Media file"), _T("roq"), false));
		ADDFMT((_T("Real Media file "), _T("rm ram rmvb rpm"), false, _T("RealOne or codec pack")));
		ADDFMT((_T("Real Audio file "), _T("ra"), true, _T("RealOne or codec pack")));
		ADDFMT((_T("Real Script file"), _T("rt rp smi smil"), false, _T("RealOne or codec pack"), RealMedia));
		ADDFMT((_T("Dirac Video file"), _T("drc"), false));
		ADDFMT((_T("DirectShow Media file"), _T("dsm dsv dsa dss")));
		ADDFMT((_T("Musepack file"), _T("mpc"), true));
		ADDFMT((_T("Flash Video file "), _T("flv")));
		ADDFMT((_T("Shockwave Flash file"), _T("swf"), false, _T("ShockWave ActiveX control"), ShockWave));
		ADDFMT((_T("Quicktime file "), _T("mov qt amr"), false, _T("QuickTime Player or codec pack"), QuickTime));
		ADDFMT((_T("Image file"), _T("jpeg jpg bmp gif pic png dib tiff tif")));
		ADDFMT((_T("Playlist file"), _T("asx m3u pls wvx wax wmx mpcpl")));
		ADDFMT((_T("Other "), _T("divx vp6")));
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
	filter += _T("Media files (all types)|__dummy|");
	mask.Add(_T(""));

	for(int i = 0; i < GetCount(); i++) 
		mask[0] += GetAt(i).GetFilter() + _T(";");
	mask[0].TrimRight(_T(";"));

	for(int i = 0; i < GetCount(); i++)
	{
		CMediaFormatCategory& mfc = GetAt(i);
		filter += mfc.GetLabel() + _T("|__dummy|");
		mask.Add(mfc.GetFilter());
	}

	filter += _T("All files (*.*)|__dummy|");
	mask.Add(_T("*.*"));

	filter += _T("|");
}

void CMediaFormats::GetAudioFilter(CString& filter, CAtlArray<CString>& mask)
{
	filter += _T("Audio files (all types)|__dummy|");
	mask.Add(_T(""));

	for(int i = 0; i < GetCount(); i++)
	{
		CMediaFormatCategory& mfc = GetAt(i);
		if(!mfc.IsAudioOnly() || mfc.GetEngineType() != DirectShow) continue;
		mask[0] += GetAt(i).GetFilter() + _T(";");
	}

	mask[0].TrimRight(_T(";"));

	for(int i = 0; i < GetCount(); i++)
	{
		CMediaFormatCategory& mfc = GetAt(i);
		if(!mfc.IsAudioOnly() || mfc.GetEngineType() != DirectShow) continue;
		filter += mfc.GetLabel() + _T("|__dummy|");
		mask.Add(mfc.GetFilter());
	}

	filter += _T("All files (*.*)|__dummy|");
	mask.Add(_T("*.*"));

	filter += _T("|");
}