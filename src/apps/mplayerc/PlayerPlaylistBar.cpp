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
#include <math.h>
#include <afxinet.h>
#include <atlrx.h>
#include <atlutil.h>
#include "mplayerc.h"
#include "MainFrm.h"
#include "../../DSUtil/DSUtil.h"
#include "SaveTextFileDialog.h"
#include "PlayerPlaylistBar.h"
#include "SettingsDefines.h"

IMPLEMENT_DYNAMIC(CPlayerPlaylistBar, CSizingControlBarG)
CPlayerPlaylistBar::CPlayerPlaylistBar()
	: m_list(0)
	, m_nTimeColWidth(0)
{
	m_bDragging = FALSE;
}

CPlayerPlaylistBar::~CPlayerPlaylistBar()
{
}

BOOL CPlayerPlaylistBar::Create(CWnd* pParentWnd)
{
	if(!CSizingControlBarG::Create(_T("Playlist"), pParentWnd, ID_VIEW_PLAYLIST))
		return FALSE;

	m_list.CreateEx(
		WS_EX_DLGMODALFRAME|WS_EX_CLIENTEDGE,
		WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_TABSTOP
		|LVS_OWNERDRAWFIXED
		|LVS_NOCOLUMNHEADER
		|LVS_EDITLABELS
		|LVS_REPORT|LVS_SINGLESEL|LVS_AUTOARRANGE|LVS_NOSORTHEADER, // TODO: remove LVS_SINGLESEL and implement multiple item repositioning (dragging is ready)
		CRect(0,0,100,100), this, IDC_PLAYLIST);

	m_list.SetExtendedStyle(m_list.GetExtendedStyle()|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);

	m_list.InsertColumn(COL_NAME, _T("Name"), LVCFMT_LEFT, 380);

	CDC* pDC = m_list.GetDC();
	CFont* old = pDC->SelectObject(GetFont());
	m_nTimeColWidth = pDC->GetTextExtent(_T("000:00:00")).cx + 5;
	pDC->SelectObject(old);
	m_list.ReleaseDC(pDC);
	m_list.InsertColumn(COL_TIME, _T("Time"), LVCFMT_RIGHT, m_nTimeColWidth);

	m_fakeImageList.Create(1, 16, ILC_COLOR4, 10, 10);
	m_list.SetImageList(&m_fakeImageList, LVSIL_SMALL);

	return TRUE;
}

BOOL CPlayerPlaylistBar::PreCreateWindow(CREATESTRUCT& cs)
{
	if(!CSizingControlBarG::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_ACCEPTFILES;

	return TRUE;
}

BOOL CPlayerPlaylistBar::PreTranslateMessage(MSG* pMsg)
{
	if(IsWindow(pMsg->hwnd) && IsVisible() && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		{
			GetParentFrame()->ShowControlBar(this, FALSE, TRUE);
			return TRUE;
		}

		if(IsDialogMessage(pMsg))
			return TRUE;
	}

	return CSizingControlBarG::PreTranslateMessage(pMsg);
}

void CPlayerPlaylistBar::AddItem(CString fn, CAtlList<CString>* subs)
{
	CAtlList<CString> sl;
	sl.AddTail(fn);
	AddItem(sl, subs);
}

void CPlayerPlaylistBar::AddItem(CAtlList<CString>& fns, CAtlList<CString>* subs)
{
	CPlaylistItem pli;

	POSITION pos = fns.GetHeadPosition();
	while(pos)
	{
		CString fn = fns.GetNext(pos);
		if(!fn.Trim().IsEmpty()) pli.m_fns.AddTail(fn);
	}

	if(subs)
	{
		POSITION pos = subs->GetHeadPosition();
		while(pos)
		{
			CString fn = subs->GetNext(pos);
			if(!fn.Trim().IsEmpty()) pli.m_subs.AddTail(fn);
		}
	}

	pli.AutoLoadFiles();

	m_pl.AddTail(pli);
}

static bool SearchFiles(CString mask, CAtlList<CString>& sl)
{
	if(mask.Find(_T("://")) >= 0)
		return(false);

	mask.Trim();
	sl.RemoveAll();

	CMediaFormats& mf = AfxGetAppSettings().Formats;

	WIN32_FILE_ATTRIBUTE_DATA fad;
	bool fFilterKnownExts = (GetFileAttributesEx(mask, GetFileExInfoStandard, &fad)
							 && (fad.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY));
	if (fFilterKnownExts)
		mask = CString(mask).TrimRight(_T("\\/")) + _T("\\*.*");

	{
		CString dir = mask.Left(max(mask.ReverseFind('\\'), mask.ReverseFind('/'))+1);

		WIN32_FIND_DATA fd;
		HANDLE h = FindFirstFile(mask, &fd);
		if(h != INVALID_HANDLE_VALUE)
		{
			do
			{
				if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) continue;

				CString fn = fd.cFileName;
				//CString ext = fn.Mid(fn.ReverseFind('.')+1).MakeLower();
				CString ext = fn.Mid(fn.ReverseFind('.')).MakeLower();
				CString path = dir + fd.cFileName;

				if(!fFilterKnownExts || mf.FindExt(ext))
				{
					for(int i = 0; i < mf.GetCount(); i++)
					{
						CMediaFormatCategory& mfc = mf.GetAt(i);
						/* playlist files are skipped when playing the contents of an entire directory */
						if((mfc.FindExt(ext)) && (mf[i].GetLabel().CompareNoCase(ResStr(IDS_AG_PLAYLIST_FILE)) != 0))
						{
							sl.AddTail(path);
							break;
						}
					}
				}

			}
			while(FindNextFile(h, &fd));

			FindClose(h);

			if(sl.GetCount() == 0 && mask.Find(_T(":\\")) == 1)
			{
				GetCDROMType(mask[0], sl);
			}
		}
	}

	return(sl.GetCount() > 1
		   || sl.GetCount() == 1 && sl.GetHead().CompareNoCase(mask)
		   || sl.GetCount() == 0 && mask.FindOneOf(_T("?*")) >= 0);
}

void CPlayerPlaylistBar::ParsePlayList(CString fn, CAtlList<CString>* subs)
{
	CAtlList<CString> sl;
	sl.AddTail(fn);
	ParsePlayList(sl, subs);
}

void CPlayerPlaylistBar::ResolveLinkFiles( CAtlList<CString> &fns )
{
	// resolve .lnk files

	CComPtr<IShellLink> pSL;
	pSL.CoCreateInstance(CLSID_ShellLink);
	CComQIPtr<IPersistFile> pPF = pSL;

	POSITION pos = fns.GetHeadPosition();
	while(pSL && pPF && pos)
	{
		CString& fn = fns.GetNext(pos);
		TCHAR buff[_MAX_PATH];
		if(CPath(fn).GetExtension().MakeLower() != _T(".lnk")
				|| FAILED(pPF->Load(CStringW(fn), STGM_READ))
				|| FAILED(pSL->Resolve(NULL, SLR_ANY_MATCH|SLR_NO_UI))
				|| FAILED(pSL->GetPath(buff, countof(buff), NULL, 0)))
			continue;

		fn = buff;
	}
}

void CPlayerPlaylistBar::ParsePlayList(CAtlList<CString>& fns, CAtlList<CString>* subs)
{
	if(fns.IsEmpty()) return;

	ResolveLinkFiles(fns);

	CAtlList<CString> sl;
	if(SearchFiles(fns.GetHead(), sl))
	{
		if(sl.GetCount() > 1) subs = NULL;
		POSITION pos = sl.GetHeadPosition();
		while(pos) ParsePlayList(sl.GetNext(pos), subs);
		return;
	}

	CAtlList<CString> redir;
	CStringA ct = GetContentType(fns.GetHead(), &redir);
	if(!redir.IsEmpty())
	{
		POSITION pos = redir.GetHeadPosition();
		while(pos) ParsePlayList(sl.GetNext(pos), subs);
		return;
	}

	if(ct == "application/x-mpc-playlist")
	{
		ParseMPCPlayList(fns.GetHead());
		return;
	}
	else if(ct == "application/x-bdmv-playlist")
	{
		ParseBDMVPlayList(fns.GetHead());
		return;
	}

	AddItem(fns, subs);
}

static int s_int_comp(const void* i1, const void* i2)
{
	return (int)i1 - (int)i2;
}

static CString CombinePath(CPath p, CString fn)
{
	if(fn.Find(':') >= 0 || fn.Find(_T("\\")) == 0) return fn;
	p.Append(CPath(fn));
	return (LPCTSTR)p;
}

bool CPlayerPlaylistBar::ParseBDMVPlayList(CString fn)
{
	CHdmvClipInfo		ClipInfo;
	CString				strPlaylistFile;
	CAtlList<CHdmvClipInfo::PlaylistItem>	MainPlaylist;

	CPath Path(fn);
	Path.RemoveFileSpec();
	Path.RemoveFileSpec();

	if (SUCCEEDED (ClipInfo.FindMainMovie (Path + L"\\", strPlaylistFile, MainPlaylist)))
	{
		CAtlList<CString>		strFiles;
		strFiles.AddHead (strPlaylistFile);
		Append(strFiles, MainPlaylist.GetCount()>1, NULL);
	}

	return m_pl.GetCount() > 0;
}

bool CPlayerPlaylistBar::ParseMPCPlayList(CString fn)
{
	CString str;
	CAtlMap<int, CPlaylistItem> pli;
	CAtlArray<int> idx;

	CWebTextFile f;
	if(!f.Open(fn) || !f.ReadString(str) || str != _T("MPCPLAYLIST"))
		return false;

	if(f.GetEncoding() == CTextFile::ASCII)
		f.SetEncoding(CTextFile::ANSI);

	CPath base(fn);
	base.RemoveFileSpec();

	while(f.ReadString(str))
	{
		CAtlList<CString> sl;
		Explode(str, sl, ',', 3);
		if(sl.GetCount() != 3) continue;

		if(int i = _ttoi(sl.RemoveHead()))
		{
			CString key = sl.RemoveHead();
			CString value = sl.RemoveHead();

			if(key == _T("type"))
			{
				pli[i].m_type = (CPlaylistItem::type_t)_ttol(value);
				idx.Add(i);
			}
			else if(key == _T("label")) pli[i].m_label = value;
			else if(key == _T("filename"))
			{
				value = CombinePath(base, value);
				pli[i].m_fns.AddTail(value);
			}
			else if(key == _T("subtitle"))
			{
				value = CombinePath(base, value);
				pli[i].m_subs.AddTail(value);
			}
			else if(key == _T("video"))
			{
				while(pli[i].m_fns.GetCount() < 2) pli[i].m_fns.AddTail(_T(""));
				pli[i].m_fns.GetHead() = value;
			}
			else if(key == _T("audio"))
			{
				while(pli[i].m_fns.GetCount() < 2) pli[i].m_fns.AddTail(_T(""));
				pli[i].m_fns.GetTail() = value;
			}
			else if(key == _T("vinput")) pli[i].m_vinput = _ttol(value);
			else if(key == _T("vchannel")) pli[i].m_vchannel = _ttol(value);
			else if(key == _T("ainput")) pli[i].m_ainput = _ttol(value);
			else if(key == _T("country")) pli[i].m_country = _ttol(value);
		}
	}

	qsort(idx.GetData(), idx.GetCount(), sizeof(int), s_int_comp);
	for(int i = 0; i < idx.GetCount(); i++)
		m_pl.AddTail(pli[idx[i]]);

	return pli.GetCount() > 0;
}

bool CPlayerPlaylistBar::SaveMPCPlayList(CString fn, CTextFile::enc e, bool fRemovePath)
{
	CTextFile f;
	if(!f.Save(fn, e))
		return false;

	f.WriteString(_T("MPCPLAYLIST\n"));

	POSITION pos = m_pl.GetHeadPosition(), pos2;
	for(int i = 1; pos; i++)
	{
		CPlaylistItem& pli = m_pl.GetNext(pos);

		CString idx;
		idx.Format(_T("%d"), i);

		CString str;
		str.Format(_T("%d,type,%d"), i, pli.m_type);
		f.WriteString(str + _T("\n"));

		if(!pli.m_label.IsEmpty())
			f.WriteString(idx + _T(",label,") + pli.m_label + _T("\n"));

		if(pli.m_type == CPlaylistItem::file)
		{
			pos2 = pli.m_fns.GetHeadPosition();
			while(pos2)
			{
				CString fn = pli.m_fns.GetNext(pos2);
				if(fRemovePath)
				{
					CPath p(fn);
					p.StripPath();
					fn = (LPCTSTR)p;
				}
				f.WriteString(idx + _T(",filename,") + fn + _T("\n"));
			}

			pos2 = pli.m_subs.GetHeadPosition();
			while(pos2)
			{
				CString fn = pli.m_subs.GetNext(pos2);
				if(fRemovePath)
				{
					CPath p(fn);
					p.StripPath();
					fn = (LPCTSTR)p;
				}
				f.WriteString(idx + _T(",subtitle,") + fn + _T("\n"));
			}
		}
		else if(pli.m_type == CPlaylistItem::device && pli.m_fns.GetCount() == 2)
		{
			f.WriteString(idx + _T(",video,") + pli.m_fns.GetHead() + _T("\n"));
			f.WriteString(idx + _T(",audio,") + pli.m_fns.GetTail() + _T("\n"));
			str.Format(_T("%d,vinput,%d"), i, pli.m_vinput);
			f.WriteString(str + _T("\n"));
			str.Format(_T("%d,vchannel,%d"), i, pli.m_vchannel);
			f.WriteString(str + _T("\n"));
			str.Format(_T("%d,ainput,%d"), i, pli.m_ainput);
			f.WriteString(str + _T("\n"));
			str.Format(_T("%d,country,%d"), i, pli.m_country);
			f.WriteString(str + _T("\n"));
		}
	}

	return true;
}

void CPlayerPlaylistBar::Refresh()
{
	SetupList();
	ResizeListColumn();
}

void CPlayerPlaylistBar::Empty()
{
	m_pl.RemoveAll();
	m_list.DeleteAllItems();
	SavePlaylist();
}

void CPlayerPlaylistBar::Open(CAtlList<CString>& fns, bool fMulti, CAtlList<CString>* subs)
{
	ResolveLinkFiles(fns);
	Empty();
	Append(fns, fMulti, subs);
}

void CPlayerPlaylistBar::Append(CAtlList<CString>& fns, bool fMulti, CAtlList<CString>* subs)
{
	if(fMulti)
	{
		ASSERT(subs == NULL || subs->GetCount() == 0);
		POSITION pos = fns.GetHeadPosition();
		while(pos) ParsePlayList(fns.GetNext(pos), NULL);
	}
	else
	{
		ParsePlayList(fns, subs);
	}

	Refresh();
	SavePlaylist();
}

void CPlayerPlaylistBar::Open(CStringW vdn, CStringW adn, int vinput, int vchannel, int ainput)
{
	Empty();
	Append(vdn, adn, vinput, vchannel, ainput);
}

void CPlayerPlaylistBar::Append(CStringW vdn, CStringW adn, int vinput, int vchannel, int ainput)
{
	CPlaylistItem pli;
	pli.m_type = CPlaylistItem::device;
	pli.m_fns.AddTail(CString(vdn));
	pli.m_fns.AddTail(CString(adn));
	pli.m_vinput = vinput;
	pli.m_vchannel = vchannel;
	pli.m_ainput = ainput;
	CAtlList<CStringW> sl;
	CStringW vfn = GetFriendlyName(vdn);
	CStringW afn = GetFriendlyName(adn);
	if(!vfn.IsEmpty()) sl.AddTail(vfn);
	if(!afn.IsEmpty()) sl.AddTail(afn);
	CStringW label = Implode(sl, '|');
	label.Replace(L"|", L" - ");
	pli.m_label = CString(label);
	m_pl.AddTail(pli);

	Refresh();
	SavePlaylist();
}

void CPlayerPlaylistBar::SetupList()
{
	m_list.DeleteAllItems();

	POSITION pos = m_pl.GetHeadPosition();
	for(int i = 0; pos; i++)
	{
		CPlaylistItem& pli = m_pl.GetAt(pos);
		m_list.SetItemData(m_list.InsertItem(i, pli.GetLabel()), (DWORD_PTR)pos);
		m_list.SetItemText(i, COL_TIME, pli.GetLabel(1));
		m_pl.GetNext(pos);
	}
}

void CPlayerPlaylistBar::UpdateList()
{
	POSITION pos = m_pl.GetHeadPosition();
	for(int i = 0, j = m_list.GetItemCount(); pos && i < j; i++)
	{
		CPlaylistItem& pli = m_pl.GetAt(pos);
		m_list.SetItemData(i, (DWORD_PTR)pos);
		m_list.SetItemText(i, COL_NAME, pli.GetLabel(0));
		m_list.SetItemText(i, COL_TIME, pli.GetLabel(1));
		m_pl.GetNext(pos);
	}
}

void CPlayerPlaylistBar::EnsureVisible(POSITION pos)
{
	int i = FindItem(m_pl.GetPos());
	if(i < 0) return;
	m_list.EnsureVisible(i, TRUE);
	m_list.Invalidate();
}

int CPlayerPlaylistBar::FindItem(POSITION pos)
{
	for(int i = 0; i < m_list.GetItemCount(); i++)
		if((POSITION)m_list.GetItemData(i) == pos)
			return(i);
	return(-1);
}

POSITION CPlayerPlaylistBar::FindPos(int i)
{
	if(i < 0) return(NULL);
	return((POSITION)m_list.GetItemData(i));
}

int CPlayerPlaylistBar::GetCount()
{
	return(m_pl.GetCount()); // TODO: n - .fInvalid
}

int CPlayerPlaylistBar::GetSelIdx()
{
	return(FindItem(m_pl.GetPos()));
}

void CPlayerPlaylistBar::SetSelIdx(int i)
{
	m_pl.SetPos(FindPos(i));
}

bool CPlayerPlaylistBar::IsAtEnd()
{
	return(m_pl.GetPos() && m_pl.GetPos() == m_pl.GetTailPosition());
}

bool CPlayerPlaylistBar::GetCur(CPlaylistItem& pli)
{
	if(!m_pl.GetPos()) return(false);
	pli = m_pl.GetAt(m_pl.GetPos());
	return(true);
}

CPlaylistItem* CPlayerPlaylistBar::GetCur()
{
	if(!m_pl.GetPos()) return NULL;
	return &m_pl.GetAt(m_pl.GetPos());
}

CString CPlayerPlaylistBar::GetCurFileName()
{
	CString fn;
	CPlaylistItem* pli = GetCur();
	if(pli && !pli->m_fns.IsEmpty())
		fn = pli->m_fns.GetHead();
	return(fn);
}

void CPlayerPlaylistBar::SetNext()
{
	POSITION pos = m_pl.GetPos(), org = pos;
	while(m_pl.GetNextWrap(pos).m_fInvalid && pos != org);
	UpdateList();
	m_pl.SetPos(pos);
	EnsureVisible(pos);
}

void CPlayerPlaylistBar::SetPrev()
{
	POSITION pos = m_pl.GetPos(), org = pos;
	while(m_pl.GetPrevWrap(pos).m_fInvalid && pos != org);
	m_pl.SetPos(pos);
	EnsureVisible(pos);
}

void CPlayerPlaylistBar::SetFirstSelected()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if(pos)
		pos = FindPos(m_list.GetNextSelectedItem(pos));
	else
	{
		pos = m_pl.GetTailPosition();
		POSITION org = pos;
		while(m_pl.GetNextWrap(pos).m_fInvalid && pos != org);
	}
	UpdateList();
	m_pl.SetPos(pos);
	EnsureVisible(pos);
}

void CPlayerPlaylistBar::SetFirst()
{
	POSITION pos = m_pl.GetTailPosition(), org = pos;
	while(m_pl.GetNextWrap(pos).m_fInvalid && pos != org);
	UpdateList();
	m_pl.SetPos(pos);
	EnsureVisible(pos);
}

void CPlayerPlaylistBar::SetLast()
{
	POSITION pos = m_pl.GetHeadPosition(), org = pos;
	while(m_pl.GetPrevWrap(pos).m_fInvalid && pos != org);
	m_pl.SetPos(pos);
	EnsureVisible(pos);
}

void CPlayerPlaylistBar::SetCurValid(bool fValid)
{
	POSITION pos = m_pl.GetPos();
	if(pos)
	{
		m_pl.GetAt(pos).m_fInvalid = !fValid;
		if(!fValid)
		{
			int i = FindItem(pos);
			m_list.RedrawItems(i, i);
		}
	}
}

void CPlayerPlaylistBar::SetCurTime(REFERENCE_TIME rt)
{
	POSITION pos = m_pl.GetPos();
	if(pos)
	{
		CPlaylistItem& pli = m_pl.GetAt(pos);
		pli.m_duration = rt;
		m_list.SetItemText(FindItem(pos), COL_TIME, pli.GetLabel(1));
	}
}

OpenMediaData* CPlayerPlaylistBar::GetCurOMD(REFERENCE_TIME rtStart)
{
	CPlaylistItem* pli = GetCur();
	if(pli == NULL)
		return NULL;

	pli->AutoLoadFiles();

	CString fn = CString(pli->m_fns.GetHead()).MakeLower();

	if(fn.Find(_T("video_ts.ifo")) >= 0
			|| fn.Find(_T(".ratdvd")) >= 0)
	{
		if(OpenDVDData* p = DNew OpenDVDData())
		{
			p->path = pli->m_fns.GetHead();
			p->subs.AddTailList(&pli->m_subs);
			return p;
		}
	}

	if(pli->m_type == CPlaylistItem::device)
	{
		if(OpenDeviceData* p = DNew OpenDeviceData())
		{
			POSITION pos = pli->m_fns.GetHeadPosition();
			for(int i = 0; i < countof(p->DisplayName) && pos; i++)
				p->DisplayName[i] = pli->m_fns.GetNext(pos);
			p->vinput = pli->m_vinput;
			p->vchannel = pli->m_vchannel;
			p->ainput = pli->m_ainput;
			return p;
		}
	}
	else
	{
		if(OpenFileData* p = DNew OpenFileData())
		{
			p->fns.AddTailList(&pli->m_fns);
			p->subs.AddTailList(&pli->m_subs);
			p->rtStart = rtStart;
			return p;
		}
	}

	return NULL;
}

bool CPlayerPlaylistBar::SelectFileInPlaylist(LPCTSTR filename)
{
	if (!filename)
		return false;
	POSITION pos = m_pl.GetHeadPosition();
	while(pos)
	{
		CPlaylistItem& pli = m_pl.GetAt(pos);
		if (pli.FindFile(filename))
		{
			m_pl.SetPos(pos);
			EnsureVisible(pos);
			return true;
		}
		m_pl.GetNext(pos);
	}
	return false;
}

void CPlayerPlaylistBar::LoadPlaylist(LPCTSTR filename)
{
	CString base;

	if(AfxGetMyApp()->GetAppSavePath(base))
	{
		CPath p;
		p.Combine(base, _T("default.mpcpl"));

		if(p.FileExists()) {
			if(AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, _T("RememberPlaylistItems"), TRUE))
			{
				ParseMPCPlayList(p);
				Refresh();
				SelectFileInPlaylist(filename);
			}
			else
			{
				::DeleteFile(p);
			}
		}
	}
}

void CPlayerPlaylistBar::SavePlaylist()
{
	CString base;

	if(AfxGetMyApp()->GetAppSavePath(base)) {
		CPath p;
		p.Combine(base, _T("default.mpcpl"));

		if(AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, _T("RememberPlaylistItems"), TRUE))
		{
			// Only create this folder when needed
			if(!::PathFileExists(base))
				::CreateDirectory(base, NULL);

			SaveMPCPlayList(p, CTextFile::UTF8, false);
		}
		else if(p.FileExists()) {
			::DeleteFile(p);
		}
	}
}

BEGIN_MESSAGE_MAP(CPlayerPlaylistBar, CSizingControlBarG)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_KEYDOWN, IDC_PLAYLIST, OnLvnKeyDown)
	ON_NOTIFY(NM_DBLCLK, IDC_PLAYLIST, OnNMDblclkList)
//	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PLAYLIST, OnCustomdrawList)
	ON_WM_DRAWITEM()
	ON_COMMAND_EX(ID_FILE_CLOSEPLAYLIST, OnFileClosePlaylist)
	ON_COMMAND_EX(ID_PLAY_PLAY, OnPlayPlay)
	ON_WM_DROPFILES()
	ON_NOTIFY(LVN_BEGINDRAG, IDC_PLAYLIST, OnBeginDrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_PLAYLIST, OnLvnEndlabeleditList)
END_MESSAGE_MAP()


// CPlayerPlaylistBar message handlers

void CPlayerPlaylistBar::ResizeListColumn()
{
	if(::IsWindow(m_list.m_hWnd))
	{
		CRect r;
		GetClientRect(r);
		r.DeflateRect(2, 2);
		m_list.SetRedraw(FALSE);
		m_list.MoveWindow(r);
		m_list.GetClientRect(r);
		m_list.SetColumnWidth(COL_NAME, r.Width()-m_nTimeColWidth); //LVSCW_AUTOSIZE_USEHEADER
		m_list.SetRedraw(TRUE);
	}
}

void CPlayerPlaylistBar::OnSize(UINT nType, int cx, int cy)
{
	CSizingControlBarG::OnSize(nType, cx, cy);

	ResizeListColumn();
}

void CPlayerPlaylistBar::OnLvnKeyDown(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	*pResult = FALSE;

	CList<int> items;
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while(pos) items.AddHead(m_list.GetNextSelectedItem(pos));

	if(pLVKeyDown->wVKey == VK_DELETE && items.GetCount() > 0)
	{
		pos = items.GetHeadPosition();
		while(pos)
		{
			int i = items.GetNext(pos);
			if(m_pl.RemoveAt(FindPos(i))) ((CMainFrame*)AfxGetMainWnd())->CloseMedia();
			m_list.DeleteItem(i);
		}

		m_list.SetItemState(-1, 0, LVIS_SELECTED);
		m_list.SetItemState(
			max(min(items.GetTail(), m_list.GetItemCount()-1), 0),
			LVIS_SELECTED, LVIS_SELECTED);

		ResizeListColumn();

		*pResult = TRUE;
	}
	else if(pLVKeyDown->wVKey == VK_SPACE && items.GetCount() == 1)
	{
		m_pl.SetPos(FindPos(items.GetHead()));

		((CMainFrame*)AfxGetMainWnd())->OpenCurPlaylistItem();

		*pResult = TRUE;
	}
}

void CPlayerPlaylistBar::OnNMDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

	if(lpnmlv->iItem >= 0 && lpnmlv->iSubItem >= 0)
	{
		AppSettings& s = AfxGetAppSettings();
		FILE_POSITION*	FilePosition = s.CurrentFilePosition();
		if (FilePosition)	FilePosition->llPosition = 0;

		m_pl.SetPos(FindPos(lpnmlv->iItem));
		m_list.Invalidate();
		((CMainFrame*)AfxGetMainWnd())->OpenCurPlaylistItem();
	}

	*pResult = 0;
}
/*
void CPlayerPlaylistBar::OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

	*pResult = CDRF_DODEFAULT;

	if(CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYPOSTPAINT|CDRF_NOTIFYITEMDRAW;
	}
	else if(CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		pLVCD->nmcd.uItemState &= ~CDIS_SELECTED;
		pLVCD->nmcd.uItemState &= ~CDIS_FOCUS;

		pLVCD->clrText = (pLVCD->nmcd.dwItemSpec == m_playList.m_idx) ? 0x0000ff : CLR_DEFAULT;
		pLVCD->clrTextBk = m_list.GetItemState(pLVCD->nmcd.dwItemSpec, LVIS_SELECTED) ? 0xf1dacc : CLR_DEFAULT;

		*pResult = CDRF_NOTIFYPOSTPAINT;
	}
	else if(CDDS_ITEMPOSTPAINT == pLVCD->nmcd.dwDrawStage)
	{
		int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);

		if(m_list.GetItemState(pLVCD->nmcd.dwItemSpec, LVIS_SELECTED))
		{
			CRect r, r2;
			m_list.GetItemRect(nItem, &r, LVIR_BOUNDS);
			m_list.GetItemRect(nItem, &r2, LVIR_LABEL);
			r.left = r2.left;
			FrameRect(pLVCD->nmcd.hdc, &r, CBrush(0xc56a31));
		}

		*pResult = CDRF_SKIPDEFAULT;
	}
	else if(CDDS_POSTPAINT == pLVCD->nmcd.dwDrawStage)
	{
	}
}
*/

void CPlayerPlaylistBar::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if(nIDCtl != IDC_PLAYLIST) return;

	int nItem = lpDrawItemStruct->itemID;
	CRect rcItem = lpDrawItemStruct->rcItem;
	POSITION pos = FindPos(nItem);
	bool fSelected = pos == m_pl.GetPos();
	CPlaylistItem& pli = m_pl.GetAt(pos);

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	if(!!m_list.GetItemState(nItem, LVIS_SELECTED))
	{
		FillRect(pDC->m_hDC, rcItem, CBrush(0xf1dacc));
		FrameRect(pDC->m_hDC, rcItem, CBrush(0xc56a31));
	}
	else
	{
		FillRect(pDC->m_hDC, rcItem, CBrush(GetSysColor(COLOR_WINDOW)));
	}

	COLORREF textcolor = fSelected?0xff:0;
	if(pli.m_fInvalid) textcolor |= 0xA0A0A0;

	CString time = !pli.m_fInvalid ? m_list.GetItemText(nItem, COL_TIME) : _T("Invalid");
	CSize timesize(0, 0);
	CPoint timept(rcItem.right, 0);
	if(time.GetLength() > 0)
	{
		timesize = pDC->GetTextExtent(time);
		if((3+timesize.cx+3) < rcItem.Width()/2)
		{
			timept = CPoint(rcItem.right-(3+timesize.cx+3), (rcItem.top+rcItem.bottom-timesize.cy)/2);

			pDC->SetTextColor(textcolor);
			pDC->TextOut(timept.x, timept.y, time);
		}
	}

	CString fmt, file;
	fmt.Format(_T("%%0%dd. %%s"), (int)log10(0.1+m_pl.GetCount())+1);
	file.Format(fmt, nItem+1, m_list.GetItemText(nItem, COL_NAME));
	CSize filesize = pDC->GetTextExtent(file);
	while(3+filesize.cx+6 > timept.x && file.GetLength() > 3)
	{
		file = file.Left(file.GetLength()-4) + _T("...");
		filesize = pDC->GetTextExtent(file);
	}

	if(file.GetLength() > 3)
	{
		pDC->SetTextColor(textcolor);
		pDC->TextOut(rcItem.left+3, (rcItem.top+rcItem.bottom-filesize.cy)/2, file);
	}
}

BOOL CPlayerPlaylistBar::OnFileClosePlaylist(UINT nID)
{
	Empty();
	return FALSE;
}

BOOL CPlayerPlaylistBar::OnPlayPlay(UINT nID)
{
	m_list.Invalidate();
	return FALSE;
}

void CPlayerPlaylistBar::OnDropFiles(HDROP hDropInfo)
{
	SetActiveWindow();

	CAtlList<CString> sl;

	UINT nFiles = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	for(UINT iFile = 0; iFile < nFiles; iFile++)
	{
		TCHAR szFileName[_MAX_PATH];
		::DragQueryFile(hDropInfo, iFile, szFileName, _MAX_PATH);
		sl.AddTail(szFileName);
	}
	::DragFinish(hDropInfo);

	Append(sl, true);
}

void CPlayerPlaylistBar::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	ModifyStyle(WS_EX_ACCEPTFILES, 0);

	m_nDragIndex = ((LPNMLISTVIEW)pNMHDR)->iItem;

	CPoint p(0, 0);
	m_pDragImage = m_list.CreateDragImageEx(&p);

	CPoint p2 = ((LPNMLISTVIEW)pNMHDR)->ptAction;

	m_pDragImage->BeginDrag(0, p2 - p);
	m_pDragImage->DragEnter(GetDesktopWindow(), ((LPNMLISTVIEW)pNMHDR)->ptAction);

	m_bDragging = TRUE;
	m_nDropIndex = -1;

	SetCapture();
}

void CPlayerPlaylistBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_bDragging)
	{
		m_ptDropPoint = point;
		ClientToScreen(&m_ptDropPoint);

		m_pDragImage->DragMove(m_ptDropPoint);
		m_pDragImage->DragShowNolock(FALSE);

		WindowFromPoint(m_ptDropPoint)->ScreenToClient(&m_ptDropPoint);

		m_pDragImage->DragShowNolock(TRUE);

		{
			int iOverItem = m_list.HitTest(m_ptDropPoint);
			int iTopItem = m_list.GetTopIndex();
			int iBottomItem = m_list.GetBottomIndex();

			if(iOverItem == iTopItem && iTopItem != 0) // top of list
				SetTimer(1, 100, NULL);
			else
				KillTimer(1);

			if(iOverItem >= iBottomItem && iBottomItem != (m_list.GetItemCount() - 1)) // bottom of list
				SetTimer(2, 100, NULL);
			else
				KillTimer(2);
		}
	}

	__super::OnMouseMove(nFlags, point);
}

void CPlayerPlaylistBar::OnTimer(UINT_PTR nIDEvent)
{
	int iTopItem = m_list.GetTopIndex();
	int iBottomItem = iTopItem + m_list.GetCountPerPage() - 1;

	if(m_bDragging)
	{
		m_pDragImage->DragShowNolock(FALSE);

		if(nIDEvent == 1)
		{
			m_list.EnsureVisible(iTopItem - 1, false);
			m_list.UpdateWindow();
			if(m_list.GetTopIndex() == 0) KillTimer(1);
		}
		else if(nIDEvent == 2)
		{
			m_list.EnsureVisible(iBottomItem + 1, false);
			m_list.UpdateWindow();
			if(m_list.GetBottomIndex() == (m_list.GetItemCount() - 1)) KillTimer(2);
		}

		m_pDragImage->DragShowNolock(TRUE);
	}

	__super::OnTimer(nIDEvent);
}

void CPlayerPlaylistBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_bDragging)
	{
		::ReleaseCapture();

		m_bDragging = FALSE;
		m_pDragImage->DragLeave(GetDesktopWindow());
		m_pDragImage->EndDrag();

		delete m_pDragImage;
		m_pDragImage = NULL;

		KillTimer(1);
		KillTimer(2);

		CPoint pt(point);
		ClientToScreen(&pt);

		if(WindowFromPoint(pt) == &m_list)
			DropItemOnList();
	}

	ModifyStyle(0, WS_EX_ACCEPTFILES);

	__super::OnLButtonUp(nFlags, point);
}

void CPlayerPlaylistBar::DropItemOnList()
{
	m_ptDropPoint.y += 10; //
	m_nDropIndex = m_list.HitTest(CPoint(10, m_ptDropPoint.y));

	TCHAR szLabel[_MAX_PATH];
	LV_ITEM lvi;
	ZeroMemory(&lvi, sizeof(LV_ITEM));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
	lvi.stateMask = LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;
	lvi.pszText = szLabel;
	lvi.iItem = m_nDragIndex;
	lvi.cchTextMax = _MAX_PATH;
	m_list.GetItem(&lvi);

	if(m_nDropIndex < 0) m_nDropIndex = m_list.GetItemCount();
	lvi.iItem = m_nDropIndex;
	m_list.InsertItem(&lvi);

	CHeaderCtrl* pHeader = (CHeaderCtrl*)m_list.GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	lvi.mask = LVIF_TEXT;
	lvi.iItem = m_nDropIndex;
	//INDEX OF DRAGGED ITEM WILL CHANGE IF ITEM IS DROPPED ABOVE ITSELF
	if(m_nDropIndex < m_nDragIndex) m_nDragIndex++;
	for(int col=1; col < nColumnCount; col++)
	{
		_tcscpy(lvi.pszText, (LPCTSTR)(m_list.GetItemText(m_nDragIndex, col)));
		lvi.iSubItem = col;
		m_list.SetItem(&lvi);
	}

	m_list.DeleteItem(m_nDragIndex);

	CList<CPlaylistItem> tmp;
	UINT id = (UINT)-1;
	for(int i = 0; i < m_list.GetItemCount(); i++)
	{
		POSITION pos = (POSITION)m_list.GetItemData(i);
		CPlaylistItem& pli = m_pl.GetAt(pos);
		tmp.AddTail(pli);
		if(pos == m_pl.GetPos()) id = pli.m_id;
	}
	m_pl.RemoveAll();
	POSITION pos = tmp.GetHeadPosition();
	for(int i = 0; pos; i++)
	{
		CPlaylistItem& pli = tmp.GetNext(pos);
		m_pl.AddTail(pli);
		if(pli.m_id == id) m_pl.SetPos(m_pl.GetTailPosition());
		m_list.SetItemData(i, (DWORD_PTR)m_pl.GetTailPosition());
	}

	ResizeListColumn();
}

BOOL CPlayerPlaylistBar::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

	if((pNMHDR->code == TTN_NEEDTEXTA && (HWND)pTTTA->lParam != m_list.m_hWnd)
			|| (pNMHDR->code == TTN_NEEDTEXTW && (HWND)pTTTW->lParam != m_list.m_hWnd))
		return FALSE;

	int row = ((pNMHDR->idFrom-1) >> 10) & 0x3fffff;
	int col = (pNMHDR->idFrom-1) & 0x3ff;

	if(row < 0 || row >= m_pl.GetCount())
		return FALSE;

	CPlaylistItem& pli = m_pl.GetAt(FindPos(row));

	CString strTipText;

	if(col == COL_NAME)
	{
		POSITION pos = pli.m_fns.GetHeadPosition();
		while(pos) strTipText += _T("\n") + pli.m_fns.GetNext(pos);
		strTipText.Trim();

		if(pli.m_type == CPlaylistItem::device)
		{
			CString str;
			str.Format(_T("Video Input %d"), pli.m_vinput);
			if(pli.m_vinput >= 0) strTipText += _T("\n") + str;
			str.Format(_T("Video Channel %d"), pli.m_vchannel);
			if(pli.m_vchannel >= 0) strTipText += _T("\n") + str;
			str.Format(_T("Audio Input %d"), pli.m_ainput);
			if(pli.m_ainput >= 0) strTipText += _T("\n") + str;
		}

		::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, (LPARAM)(INT)1000);
	}
	else if(col == COL_TIME)
	{
		return FALSE;
	}

	static CStringA m_strTipTextA;
	static CStringW m_strTipTextW;

	if(pNMHDR->code == TTN_NEEDTEXTA)
	{
		m_strTipTextA = strTipText;
		pTTTA->lpszText = (LPSTR)(LPCSTR)m_strTipTextA;
	}
	else
	{
		m_strTipTextW = strTipText;
		pTTTW->lpszText = (LPWSTR)(LPCWSTR)m_strTipTextW;
	}

	*pResult = 0;

	return TRUE;    // message was handled
}

void CPlayerPlaylistBar::OnContextMenu(CWnd* /*pWnd*/, CPoint p)
{
	LVHITTESTINFO lvhti;
	lvhti.pt = p;
	m_list.ScreenToClient(&lvhti.pt);
	m_list.SubItemHitTest(&lvhti);

	POSITION pos = FindPos(lvhti.iItem);
//	bool fSelected = (pos == m_pl.GetPos());
	bool fOnItem = !!(lvhti.flags&LVHT_ONITEM);

	CMenu m;
	m.CreatePopupMenu();

	enum
	{
		M_OPEN=1, M_ADD, M_REMOVE, M_CLIPBOARD, M_SAVEAS,
		M_SORTBYNAME, M_SORTBYPATH, M_RANDOMIZE, M_SORTBYID,
		M_REMEMBERPLAYLIST, M_SHUFFLE, M_HIDEFULLSCREEN
	};

	m.AppendMenu(MF_STRING|(!fOnItem?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_OPEN, ResStr(IDS_PLAYLIST_OPEN));
	if(((CMainFrame*)AfxGetMainWnd())->GetPlaybackMode() == PM_CAPTURE) m.AppendMenu(MF_STRING|MF_ENABLED, M_ADD, ResStr(IDS_PLAYLIST_ADD));
	m.AppendMenu(MF_STRING|(/*fSelected||*/!fOnItem?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_REMOVE, ResStr(IDS_PLAYLIST_REMOVE));
	m.AppendMenu(MF_SEPARATOR);
	m.AppendMenu(MF_STRING|(!fOnItem?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_CLIPBOARD, ResStr(IDS_PLAYLIST_COPYTOCLIPBOARD));
	m.AppendMenu(MF_STRING|(!m_pl.GetCount()?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_SAVEAS, ResStr(IDS_PLAYLIST_SAVEAS));
	m.AppendMenu(MF_SEPARATOR);
	m.AppendMenu(MF_STRING|(!m_pl.GetCount()?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_SORTBYNAME, ResStr(IDS_PLAYLIST_SORTBYLABEL));
	m.AppendMenu(MF_STRING|(!m_pl.GetCount()?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_SORTBYPATH, ResStr(IDS_PLAYLIST_SORTBYPATH));
	m.AppendMenu(MF_STRING|(!m_pl.GetCount()?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_RANDOMIZE, ResStr(IDS_PLAYLIST_RANDOMIZE));
	m.AppendMenu(MF_STRING|(!m_pl.GetCount()?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_SORTBYID, ResStr(IDS_PLAYLIST_RESTORE));
	m.AppendMenu(MF_SEPARATOR);
	m.AppendMenu(MF_STRING|MF_ENABLED|(AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, _T("ShufflePlaylistItems"), FALSE)?MF_CHECKED:0), M_SHUFFLE, ResStr(IDS_PLAYLIST_SHUFFLE));
	m.AppendMenu(MF_STRING|MF_ENABLED|(AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, _T("RememberPlaylistItems"), TRUE)?MF_CHECKED:0), M_REMEMBERPLAYLIST, ResStr(IDS_PLAYLIST_REMEBERITEMS));
	m.AppendMenu(MF_SEPARATOR);
	m.AppendMenu(MF_STRING|MF_ENABLED|(AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, _T("HidePlaylistFullScreen"), FALSE)?MF_CHECKED:0), M_HIDEFULLSCREEN, ResStr(IDS_PLAYLIST_HIDEFS));

	CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

	int nID = (int)m.TrackPopupMenu(TPM_LEFTBUTTON|TPM_RETURNCMD, p.x, p.y, this);
	switch(nID)
	{
	case M_OPEN:
		m_pl.SetPos(pos);
		m_list.Invalidate();
		pMainFrm->OpenCurPlaylistItem();
		break;
	case M_ADD:
		pMainFrm->AddCurDevToPlaylist();
		m_pl.SetPos(m_pl.GetTailPosition());
		break;
	case M_REMOVE:
		if(m_pl.RemoveAt(pos)) pMainFrm->CloseMedia();
		m_list.DeleteItem(lvhti.iItem);
		SavePlaylist();
		break;
	case M_SORTBYID:
		m_pl.SortById();
		SetupList();
		SavePlaylist();
		break;
	case M_SORTBYNAME:
		m_pl.SortByName();
		SetupList();
		SavePlaylist();
		break;
	case M_SORTBYPATH:
		m_pl.SortByPath();
		SetupList();
		SavePlaylist();
		break;
	case M_RANDOMIZE:
		m_pl.Randomize();
		SetupList();
		SavePlaylist();
		break;
	case M_CLIPBOARD:
		if(OpenClipboard() && EmptyClipboard())
		{
			CString str;

			CPlaylistItem& pli = m_pl.GetAt(pos);
			POSITION pos = pli.m_fns.GetHeadPosition();
			while(pos) str += _T("\r\n") + pli.m_fns.GetNext(pos);
			str.Trim();

			if(HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, (str.GetLength()+1)*sizeof(TCHAR)))
			{
				if(TCHAR* s = (TCHAR*)GlobalLock(h))
				{
					_tcscpy(s, str);
					GlobalUnlock(h);
#ifdef UNICODE
					SetClipboardData(CF_UNICODETEXT, h);
#else
					SetClipboardData(CF_TEXT, h);
#endif
				}
			}
			CloseClipboard();
		}
		break;
	case M_SAVEAS:
	{
		CSaveTextFileDialog fd(
			CTextFile::ASCII, NULL, NULL,
			_T("Media Player Classic playlist (*.mpcpl)|*.mpcpl|Playlist (*.pls)|*.pls|Winamp playlist (*.m3u)|*.m3u|Windows Media playlist (*.asx)|*.asx||"),
			this);

		if(fd.DoModal() != IDOK)
			break;

		CTextFile::enc encoding = (CTextFile::enc)fd.GetEncoding();
		if(encoding == CTextFile::ASCII) encoding = CTextFile::ANSI;

		int idx = fd.m_pOFN->nFilterIndex;

		CPath path(fd.GetPathName());

		switch(idx)
		{
		case 1:
			path.AddExtension(_T(".mpcpl"));
			break;
		case 2:
			path.AddExtension(_T(".pls"));
			break;
		case 3:
			path.AddExtension(_T(".m3u"));
			break;
		case 4:
			path.AddExtension(_T(".asx"));
			break;
		default:
			break;
		}

		bool fRemovePath = true;

		CPath p(path);
		p.RemoveFileSpec();
		CString base = (LPCTSTR)p;

		pos = m_pl.GetHeadPosition();
		while(pos && fRemovePath)
		{
			CPlaylistItem& pli = m_pl.GetNext(pos);

			if(pli.m_type != CPlaylistItem::file) fRemovePath = false;
			else
			{
				POSITION pos;

				pos = pli.m_fns.GetHeadPosition();
				while(pos && fRemovePath)
				{
					CString fn = pli.m_fns.GetNext(pos);

					CPath p(fn);
					p.RemoveFileSpec();
					if(base != (LPCTSTR)p) fRemovePath = false;
				}

				pos = pli.m_subs.GetHeadPosition();
				while(pos && fRemovePath)
				{
					CString fn = pli.m_subs.GetNext(pos);

					CPath p(fn);
					p.RemoveFileSpec();
					if(base != (LPCTSTR)p) fRemovePath = false;
				}
			}
		}

		if(idx == 1)
		{
			SaveMPCPlayList(path, encoding, fRemovePath);
			break;
		}

		CTextFile f;
		if(!f.Save(path, encoding))
			break;

		if (idx == 2)
		{
			f.WriteString(_T("[playlist]\n"));
		}
		else if (idx == 4)
		{
			f.WriteString(_T("<ASX version = \"3.0\">\n"));
		}

		pos = m_pl.GetHeadPosition();
		CString str;
		int i;
		for(i = 0; pos; i++)
		{
			CPlaylistItem& pli = m_pl.GetNext(pos);

			if(pli.m_type != CPlaylistItem::file)
				continue;

			CString fn = pli.m_fns.GetHead();

/*
			if(fRemovePath)
			{
				CPath p(path);
				p.StripPath();
				fn = (LPCTSTR)p;
			}
*/

			switch(idx)
			{
			case 2:
				str.Format(_T("File%d=%s\n"), i+1, fn);
				break;
			case 3:
				str.Format(_T("%s\n"), fn);
				break;
			case 4:
				str.Format(_T("<Entry><Ref href = \"%s\"/></Entry>\n"), fn);
				break;
			default:
				break;
			}
			f.WriteString(str);
		}

		if (idx == 2)
		{
			str.Format(_T("NumberOfEntries=%d\n"), i);
			f.WriteString(str);
			f.WriteString(_T("Version=2\n"));
		}
		else if (idx == 4)
		{
			f.WriteString(_T("</ASX>\n"));
		}
	}
	break;
	case M_REMEMBERPLAYLIST:
		AfxGetApp()->WriteProfileInt(IDS_R_SETTINGS, _T("RememberPlaylistItems"),
									 !AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, _T("RememberPlaylistItems"), TRUE));
		break;
	case M_SHUFFLE:
		AfxGetApp()->WriteProfileInt(IDS_R_SETTINGS, _T("ShufflePlaylistItems"),
									 !AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, _T("ShufflePlaylistItems"), FALSE));
		break;
	case M_HIDEFULLSCREEN:
		AfxGetApp()->WriteProfileInt(IDS_R_SETTINGS, _T("HidePlaylistFullScreen"),
									 !AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, _T("HidePlaylistFullScreen"), FALSE));
		break;
	default:
		break;
	}
}

void CPlayerPlaylistBar::OnLvnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if(pDispInfo->item.iItem >= 0 && pDispInfo->item.pszText)
	{
		CPlaylistItem& pli = m_pl.GetAt((POSITION)m_list.GetItemData(pDispInfo->item.iItem));
		pli.m_label = pDispInfo->item.pszText;
		m_list.SetItemText(pDispInfo->item.iItem, 0, pDispInfo->item.pszText);
	}

	*pResult = 0;
}
