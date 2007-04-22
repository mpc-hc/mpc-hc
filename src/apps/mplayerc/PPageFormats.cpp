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

// PPageFormats.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "PPageFormats.h"

// CPPageFormats dialog

IMPLEMENT_DYNAMIC(CPPageFormats, CPPageBase)
CPPageFormats::CPPageFormats()
	: CPPageBase(CPPageFormats::IDD, CPPageFormats::IDD)
	, m_list(0)
	, m_exts(_T(""))
	, m_iRtspHandler(0)
	, m_fRtspFileExtFirst(FALSE)
{
}

CPPageFormats::~CPPageFormats()
{
}

void CPPageFormats::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Text(pDX, IDC_EDIT1, m_exts);
	DDX_Control(pDX, IDC_STATIC1, m_autoplay);
	DDX_Control(pDX, IDC_CHECK1, m_apvideo);
	DDX_Control(pDX, IDC_CHECK2, m_apmusic);
	DDX_Control(pDX, IDC_CHECK3, m_apaudiocd);
	DDX_Control(pDX, IDC_CHECK4, m_apdvd);
	DDX_Radio(pDX, IDC_RADIO1, m_iRtspHandler);
	DDX_Check(pDX, IDC_CHECK5, m_fRtspFileExtFirst);
}

int CPPageFormats::GetChecked(int iItem)
{
	LVITEM lvi;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE;
	m_list.GetItem(&lvi);
	return(lvi.iImage);
}

void CPPageFormats::SetChecked(int iItem, int iChecked)
{
	LVITEM lvi;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE;
	lvi.iImage = iChecked;
	m_list.SetItem(&lvi);
}

static bool MakeRegParams(CString ext, CString& path, CString& fn, CString& extfile, CString& cmd)
{
	if(ext.GetLength() == 0)
		return(false);

	TCHAR buff[MAX_PATH];
	if(::GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH) == 0)
		return(false);

	path = buff;

	fn = path.Mid(path.ReverseFind('\\')+1).MakeLower();
	if(fn.IsEmpty())
		return(false);

	extfile = ext.TrimLeft('.')+_T("file");

	cmd = _T("\"") + path + _T("\" \"%1\"");

	return(true);
}

bool CPPageFormats::IsRegistered(CString ext)
{
	CString path, fn, extfile, cmd;
	if(!MakeRegParams(ext, path, fn, extfile, cmd))
		return(false);

	TCHAR buff[256];
	ULONG len = sizeof(buff);
	memset(buff, 0, len);

	CRegKey key;

	CString ExplExt = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\") + ext;
	if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, ExplExt, KEY_READ))
	{
		len = sizeof(buff);
		memset(buff, 0, len);
		if(ERROR_SUCCESS == key.QueryStringValue(_T("Application"), buff, &len))
			return(CString(buff).Trim() == cmd);
	}

	if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext, KEY_READ))
		return(false);

	len = sizeof(buff);
	memset(buff, 0, len);
	if(ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len) || (extfile = buff).Trim().IsEmpty())
		return(false);

	if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, extfile + _T("\\shell\\open\\command"), KEY_READ))
		return(false);

	len = sizeof(buff);
	memset(buff, 0, len);

	CRegKey key2;
	if(ERROR_SUCCESS == key2.Open(HKEY_CLASSES_ROOT, extfile + _T("\\shell\\open"), KEY_READ)
	&& ERROR_SUCCESS == key2.QueryStringValue(_T("LegacyDisable"), buff, &len))
		return(false);

	len = sizeof(buff);
	memset(buff, 0, len);

	if(ERROR_SUCCESS == key2.Open(HKEY_CLASSES_ROOT, extfile + _T("\\shell\\open\\DropTarget"), KEY_READ))
		return(false);

	len = sizeof(buff);
	memset(buff, 0, len);

	return(ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) 
		&& !CString(buff).CompareNoCase(cmd));
}

bool CPPageFormats::RegisterExt(CString ext, bool fRegister)
{
	if(fRegister == IsRegistered(ext))
		return(true);

	CString path, fn, extfile, cmd;
	if(!MakeRegParams(ext, path, fn, extfile, cmd))
		return(false);

	TCHAR buff[256];
	ULONG len = sizeof(buff);
	memset(buff, 0, len);

	CRegKey key;

	if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, ext))
		return(false);

	len = sizeof(buff);
	memset(buff, 0, len);

	if(ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty())
	{
		extfile = buff;
	}
	else
	{
		if(!fRegister) return(true);
		else if(ERROR_SUCCESS != key.SetStringValue(NULL, extfile)) return(false);
	}

	if(fRegister)
	{
		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, extfile + _T("\\shell")))
			return(false);

		len = sizeof(buff);
		memset(buff, 0, len);

		key.QueryStringValue(NULL, buff, &len);

		if(ERROR_SUCCESS != key.SetStringValue(fn + _T(".bak"), buff))
			return(false);

		if(ERROR_SUCCESS != key.SetStringValue(NULL, _T("open")))
		{
			key.SetStringValue(NULL, buff);
			key.DeleteValue(fn + _T(".bak"));
			return(false);
		}

		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, extfile + _T("\\shell\\open")))
			return(false);

		len = sizeof(buff);
		memset(buff, 0, len);

		if(ERROR_SUCCESS != key.SetStringValue(NULL, _T("&Open")))
			return(false);

		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, extfile + _T("\\shell\\open\\command")))
			return(false);

		len = sizeof(buff);
		memset(buff, 0, len);

		key.QueryStringValue(NULL, buff, &len);

		if(CString(buff).MakeLower() == cmd)
			return(true);

		if(ERROR_SUCCESS != key.SetStringValue(fn + _T(".bak"), buff))
			return(false);

		if(ERROR_SUCCESS != key.SetStringValue(NULL, cmd))
		{
			key.SetStringValue(NULL, buff);
			key.DeleteValue(fn + _T(".bak"));
			return(false);
		}

		len = sizeof(buff);
		memset(buff, 0, len);

		if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, extfile + _T("\\shell\\open")))
		{
			if(ERROR_SUCCESS == key.QueryStringValue(_T("LegacyDisable"), buff, &len))
			{
				key.DeleteValue(_T("LegacyDisable"));
				key.SetStringValue(_T("LegacyDisable.bak"), _T(""));
			}

			key.RecurseDeleteKey(_T("ddeexec"));
			key.RecurseDeleteKey(_T("DropTarget"));
		}

		if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\") + ext))
		{
			key.DeleteValue(_T("Application"));
		}
	}
	else
	{
		if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, extfile + _T("\\shell\\open\\command")))
			return(true);

		len = sizeof(buff);
		memset(buff, 0, len);

		if(ERROR_SUCCESS != key.QueryStringValue(fn + _T(".bak"), buff, &len))
			buff[0] = 0; //return(true);

		if(CString(buff).Trim().IsEmpty())
		{
			if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, extfile + _T("\\shell\\open"))
			|| ERROR_SUCCESS != key.RecurseDeleteKey(_T("command")))
				return(false);
		}
		else
		{
			if(ERROR_SUCCESS != key.SetStringValue(NULL, buff)
			|| ERROR_SUCCESS != key.DeleteValue(fn + _T(".bak")))
				return(false);

			len = sizeof(buff);
			memset(buff, 0, len);

			if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, extfile + _T("\\shell\\open"))
			&& ERROR_SUCCESS == key.QueryStringValue(_T("LegacyDisable.bak"), buff, &len))
			{
				key.DeleteValue(_T("LegacyDisable.bak"));
				key.SetStringValue(_T("LegacyDisable"), _T(""));
			}
		}

		if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, extfile + _T("\\shell")))
			return(true);

		len = sizeof(buff);
		memset(buff, 0, len);

		if(ERROR_SUCCESS != key.QueryStringValue(fn + _T(".bak"), buff, &len))
			return(true);

		if(CString(buff).Trim().IsEmpty())
		{
			if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, extfile)
			|| ERROR_SUCCESS != key.RecurseDeleteKey(_T("shell")))
				return(false);
		}
		else
		{
			if(ERROR_SUCCESS != key.SetStringValue(NULL, buff)
			|| ERROR_SUCCESS != key.DeleteValue(fn + _T(".bak")))
				return(false);
		}
	}

	return(true);
}

static struct {LPCSTR verb, cmd; UINT action;} handlers[] =
{
	{"VideoFiles", " %1", IDS_AUTOPLAY_PLAYVIDEO},
	{"MusicFiles", " %1", IDS_AUTOPLAY_PLAYMUSIC},
	{"CDAudio", " %1 /cd", IDS_AUTOPLAY_PLAYAUDIOCD},
	{"DVDMovie", " %1 /dvd", IDS_AUTOPLAY_PLAYDVDMOVIE},
};

void CPPageFormats::AddAutoPlayToRegistry(autoplay_t ap, bool fRegister)
{
	if(!AfxGetAppSettings().fXpOrBetter) return;

	TCHAR buff[MAX_PATH];
	if(::GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH) == 0) return;
	CString exe = buff;

	int i = (int)ap;
	if(i < 0 || i >= countof(handlers)) return;

	CRegKey key;

	if(fRegister)
	{
		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, _T("MediaPlayerClassic.Autorun"))) return;
		key.Close();

		if(ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, 
			CString(CStringA("MediaPlayerClassic.Autorun\\Shell\\Play") + handlers[i].verb + "\\Command"))) return;
		key.SetStringValue(NULL, exe + handlers[i].cmd);
		key.Close();

		if(ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE,
			CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\Handlers\\MPCPlay") + handlers[i].verb + "OnArrival"))) return;
		key.SetStringValue(_T("Action"), ResStr(handlers[i].action));
		key.SetStringValue(_T("Provider"), _T("Media Player Classic"));
		key.SetStringValue(_T("InvokeProgID"), _T("MediaPlayerClassic.Autorun"));
		key.SetStringValue(_T("InvokeVerb"), CString(CStringA("Play") + handlers[i].verb));
		key.SetStringValue(_T("DefaultIcon"), exe + _T(",0"));
		key.Close();

		if(ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, 
			CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\Play") + handlers[i].verb + "OnArrival"))) return;
		key.SetStringValue(CString(CStringA("MPCPlay") + handlers[i].verb + "OnArrival"), _T(""));
		key.Close();
	}
	else
	{
		if(ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, 
			CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\Play") + handlers[i].verb + "OnArrival"))) return;
		key.DeleteValue(CString(CStringA("MPCPlay") + handlers[i].verb + "OnArrival"));
		key.Close();
	}
}

bool CPPageFormats::IsAutoPlayRegistered(autoplay_t ap)
{
	ULONG len;
	TCHAR buff[MAX_PATH];
	if(::GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH) == 0) return(false);
	CString exe = buff;

	int i = (int)ap;
	if(i < 0 || i >= countof(handlers)) return(false);

	CRegKey key;

	if(ERROR_SUCCESS != key.Open(HKEY_LOCAL_MACHINE, 
		CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\Play") + handlers[i].verb + "OnArrival"),
		KEY_READ)) return(false);
	len = countof(buff);
	if(ERROR_SUCCESS != key.QueryStringValue(
		CString(_T("MPCPlay")) + handlers[i].verb + _T("OnArrival"), 
		buff, &len)) return(false);
	key.Close();

	if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, 
		CString(CStringA("MediaPlayerClassic.Autorun\\Shell\\Play") + handlers[i].verb + "\\Command"),
		KEY_READ)) return(false);
	len = countof(buff);
	if(ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len))
		return(false);
	if(_tcsnicmp(exe, buff, exe.GetLength()))
		return(false);
	key.Close();

	return(true);
}

void CPPageFormats::SetListItemState(int nItem)
{
	if(nItem < 0) return;

	CString str = AfxGetAppSettings().Formats[(int)m_list.GetItemData(nItem)].GetExtsWithPeriod();

	CAtlList<CString> exts;
	ExplodeMin(str, exts, ' ');

	int cnt = 0;

	POSITION pos = exts.GetHeadPosition();
	while(pos) if(IsRegistered(exts.GetNext(pos))) cnt++;

	SetChecked(nItem, cnt == 0 ? 0 : cnt == exts.GetCount() ? 1 : 2);
}

BEGIN_MESSAGE_MAP(CPPageFormats, CPPageBase)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnNMClickList1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnLvnItemchangedList1)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST1, OnBeginlabeleditList)
	ON_NOTIFY(LVN_DOLABELEDIT, IDC_LIST1, OnDolabeleditList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST1, OnEndlabeleditList)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON_EXT_SET, OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton14)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton13)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON2, OnUpdateButtonDefault)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_EXT_SET, OnUpdateButtonSet)
END_MESSAGE_MAP()

// CPPageFormats message handlers

BOOL CPPageFormats::OnInitDialog()
{
	__super::OnInitDialog();

	m_list.SetExtendedStyle(m_list.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	m_list.InsertColumn(COL_CATEGORY, _T("Category"), LVCFMT_LEFT, 300);
	m_list.InsertColumn(COL_ENGINE, _T("Engine"), LVCFMT_RIGHT, 60);

	m_onoff.Create(IDB_ONOFF, 12, 3, 0xffffff);
	m_list.SetImageList(&m_onoff, LVSIL_SMALL);

	CMediaFormats& mf = AfxGetAppSettings().Formats;
	for(int i = 0; i < mf.GetCount(); i++)
	{
		CString label = mf[i].GetLabel();
		// HACK: sorry, mpc is just not an image viewer :)
		if(!label.CompareNoCase(_T("Image file"))) continue;
		int iItem = m_list.InsertItem(i, label);
		m_list.SetItemData(iItem, i);
		engine_t e = mf[i].GetEngineType();
		m_list.SetItemText(iItem, COL_ENGINE, 
			e == DirectShow ? _T("DirectShow") : 
			e == RealMedia ? _T("RealMedia") : 
			e == QuickTime ? _T("QuickTime") : 
			e == ShockWave ? _T("ShockWave") : _T("-"));
	}

//	m_list.SetColumnWidth(COL_CATEGORY, LVSCW_AUTOSIZE);
	m_list.SetColumnWidth(COL_ENGINE, LVSCW_AUTOSIZE_USEHEADER);

	m_list.SetSelectionMark(0);
	m_list.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	m_exts = mf[(int)m_list.GetItemData(0)].GetExtsWithPeriod();

	AppSettings& s = AfxGetAppSettings();
	bool fRtspFileExtFirst;
	engine_t e = s.Formats.GetRtspHandler(fRtspFileExtFirst);
	m_iRtspHandler = (e==RealMedia?0:e==QuickTime?1:2);
	m_fRtspFileExtFirst = fRtspFileExtFirst;

	UpdateData(FALSE);

	for(int i = 0; i < m_list.GetItemCount(); i++)
	{
		SetListItemState(i);
	}

	if(AfxGetAppSettings().fXpOrBetter)
	{
		m_apvideo.SetCheck(IsAutoPlayRegistered(AP_VIDEO));
		m_apmusic.SetCheck(IsAutoPlayRegistered(AP_MUSIC));
		m_apaudiocd.SetCheck(IsAutoPlayRegistered(AP_AUDIOCD));
		m_apdvd.SetCheck(IsAutoPlayRegistered(AP_DVDMOVIE));
	}
	else
	{
		m_autoplay.ShowWindow(SW_HIDE);
		m_apvideo.ShowWindow(SW_HIDE);
		m_apmusic.ShowWindow(SW_HIDE);
		m_apaudiocd.ShowWindow(SW_HIDE);
		m_apdvd.ShowWindow(SW_HIDE);
	}

	CreateToolTip();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageFormats::OnApply()
{
	UpdateData();

	{
		int i = m_list.GetSelectionMark();
		if(i >= 0) i = (int)m_list.GetItemData(i);
		if(i >= 0)
		{
			CMediaFormats& mf = AfxGetAppSettings().Formats;
			mf[i].SetExts(m_exts);
			m_exts = mf[i].GetExtsWithPeriod();
			UpdateData(FALSE);
		}
	}

	CMediaFormats& mf = AfxGetAppSettings().Formats;

	for(int i = 0; i < m_list.GetItemCount(); i++)
	{
		int iChecked = GetChecked(i);
		if(iChecked == 2) continue;

		CAtlList<CString> exts;
		Explode(mf[(int)m_list.GetItemData(i)].GetExtsWithPeriod(), exts, ' ');

		POSITION pos = exts.GetHeadPosition();
		while(pos) RegisterExt(exts.GetNext(pos), !!iChecked);
	}

	{
		SetListItemState(m_list.GetSelectionMark());
	}

	AddAutoPlayToRegistry(AP_VIDEO, !!m_apvideo.GetCheck());
	AddAutoPlayToRegistry(AP_MUSIC, !!m_apmusic.GetCheck());
	AddAutoPlayToRegistry(AP_AUDIOCD, !!m_apaudiocd.GetCheck());
	AddAutoPlayToRegistry(AP_DVDMOVIE, !!m_apdvd.GetCheck());

//	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

	AppSettings& s = AfxGetAppSettings();
	s.Formats.SetRtspHandler(m_iRtspHandler==0?RealMedia:m_iRtspHandler==1?QuickTime:DirectShow, !!m_fRtspFileExtFirst);

	return __super::OnApply();
}

void CPPageFormats::OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

	if(lpnmlv->iItem >= 0 && lpnmlv->iSubItem == COL_CATEGORY)
	{
		CRect r;
		m_list.GetItemRect(lpnmlv->iItem, r, LVIR_ICON);
		if(r.PtInRect(lpnmlv->ptAction))
		{
			SetChecked(lpnmlv->iItem, (GetChecked(lpnmlv->iItem)&1) == 0 ? 1 : 0);
			SetModified();
		}
	}

	*pResult = 0;
}

void CPPageFormats::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if(pNMLV->iItem >= 0 && pNMLV->iSubItem == COL_CATEGORY
	&& (pNMLV->uChanged&LVIF_STATE) && (pNMLV->uNewState&LVIS_SELECTED))
	{
		m_exts = AfxGetAppSettings().Formats[(int)m_list.GetItemData(pNMLV->iItem)].GetExtsWithPeriod();
		UpdateData(FALSE);
	}

	*pResult = 0;
}

void CPPageFormats::OnBeginlabeleditList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	*pResult = FALSE;

	if(pItem->iItem < 0) 
		return;

	if(pItem->iSubItem == COL_ENGINE)
	{
		*pResult = TRUE;
	}
}

void CPPageFormats::OnDolabeleditList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	*pResult = FALSE;

	if(pItem->iItem < 0) 
		return;

	CMediaFormatCategory& mfc = AfxGetAppSettings().Formats[m_list.GetItemData(pItem->iItem)];

	CAtlList<CString> sl;
	int nSel = -1;

	if(pItem->iSubItem == COL_ENGINE)
	{
		sl.AddTail(_T("DirectShow"));
		sl.AddTail(_T("RealMedia"));
		sl.AddTail(_T("QuickTime"));
		sl.AddTail(_T("ShockWave"));

		nSel = (int)mfc.GetEngineType();

		m_list.ShowInPlaceComboBox(pItem->iItem, pItem->iSubItem, sl, nSel);

		*pResult = TRUE;
	}
}

void CPPageFormats::OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	*pResult = FALSE;

	if(!m_list.m_fInPlaceDirty)
		return;

	if(pItem->iItem < 0) 
		return;

	CMediaFormatCategory& mfc = AfxGetAppSettings().Formats[m_list.GetItemData(pItem->iItem)];

	if(pItem->iSubItem == COL_ENGINE && pItem->lParam >= 0)
	{
		mfc.SetEngineType((engine_t)pItem->lParam);
		m_list.SetItemText(pItem->iItem, pItem->iSubItem, pItem->pszText);
		*pResult = TRUE;
	}

	if(*pResult)
		SetModified();
}

void CPPageFormats::OnBnClickedButton1()
{
	for(int i = 0, j = m_list.GetItemCount(); i < j; i++)
	{
		SetChecked(i, 1);
	}

	m_apvideo.SetCheck(1);
	m_apmusic.SetCheck(1);
	m_apaudiocd.SetCheck(1);
	m_apdvd.SetCheck(1);
    
	SetModified();
}

void CPPageFormats::OnBnClickedButton14()
{
	CMediaFormats& mf = AfxGetAppSettings().Formats;

	for(int i = 0, j = m_list.GetItemCount(); i < j; i++)
	{
		SetChecked(i, mf[(int)m_list.GetItemData(i)].IsAudioOnly()?0:1);
	}

	m_apvideo.SetCheck(1);
	m_apmusic.SetCheck(0);
	m_apaudiocd.SetCheck(0);
	m_apdvd.SetCheck(1);
    
	SetModified();
}

void CPPageFormats::OnBnClickedButton13()
{
	CMediaFormats& mf = AfxGetAppSettings().Formats;

	for(int i = 0, j = m_list.GetItemCount(); i < j; i++)
	{
		SetChecked(i, mf[(int)m_list.GetItemData(i)].IsAudioOnly()?1:0);
	}

	m_apvideo.SetCheck(0);
	m_apmusic.SetCheck(1);
	m_apaudiocd.SetCheck(1);
	m_apdvd.SetCheck(0);
    
	SetModified();
}

void CPPageFormats::OnBnClickedButton12()
{
	int i = m_list.GetSelectionMark();
	if(i < 0) return;
	i = (int)m_list.GetItemData(i);
	CMediaFormats& mf = AfxGetAppSettings().Formats;
	mf[i].RestoreDefaultExts();
	m_exts = mf[i].GetExtsWithPeriod();
	SetListItemState(m_list.GetSelectionMark());
	UpdateData(FALSE);
    
	SetModified();
}

void CPPageFormats::OnBnClickedButton11()
{
	UpdateData();
	int i = m_list.GetSelectionMark();
	if(i < 0) return;
	i = (int)m_list.GetItemData(i);
	CMediaFormats& mf = AfxGetAppSettings().Formats;
	mf[i].SetExts(m_exts);
	m_exts = mf[i].GetExtsWithPeriod();
	SetListItemState(m_list.GetSelectionMark());
	UpdateData(FALSE);
    
	SetModified();
}

void CPPageFormats::OnUpdateButtonDefault(CCmdUI* pCmdUI)
{
	int i = m_list.GetSelectionMark();
	if(i < 0) {pCmdUI->Enable(FALSE); return;}
	i = (int)m_list.GetItemData(i);

	CString orgexts, newexts;
	GetDlgItem(IDC_EDIT1)->GetWindowText(newexts);
	newexts.Trim();
	orgexts = AfxGetAppSettings().Formats[i].GetBackupExtsWithPeriod();

	pCmdUI->Enable(!!newexts.CompareNoCase(orgexts));
}

void CPPageFormats::OnUpdateButtonSet(CCmdUI* pCmdUI)
{
	int i = m_list.GetSelectionMark();
	if(i < 0) {pCmdUI->Enable(FALSE); return;}
	i = (int)m_list.GetItemData(i);

	CString orgexts, newexts;
	GetDlgItem(IDC_EDIT1)->GetWindowText(newexts);
	newexts.Trim();
	orgexts = AfxGetAppSettings().Formats[i].GetExtsWithPeriod();

	pCmdUI->Enable(!!newexts.CompareNoCase(orgexts));
}
