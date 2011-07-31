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
#include "mplayerc.h"
#include "PPageFormats.h"
#include "WinAPIUtils.h"
#include <psapi.h>
#include <string>

// CPPageFormats dialog


CComPtr<IApplicationAssociationRegistration>	CPPageFormats::m_pAAR;

// TODO: change this along with the root key for settings and the mutex name to
//       avoid possible risks of conflict with the old MPC (non HC version).
#ifdef _WIN64
	#define PROGID _T("mplayerc64")
#else
	#define PROGID _T("mplayerc")
#endif // _WIN64

LPCTSTR			g_strRegisteredAppName = _T("Media Player Classic");
LPCTSTR			g_strOldAssoc		   = _T("PreviousRegistration");
CString			g_strRegisteredKey	   = _T("Software\\Clients\\Media\\Media Player Classic\\Capabilities");

int	f_setContextFiles = 0;
int	f_getContextFiles = 0;

int	f_setAssociatedWithIcon = 0;

IMPLEMENT_DYNAMIC(CPPageFormats, CPPageBase)
CPPageFormats::CPPageFormats()
	: CPPageBase(CPPageFormats::IDD, CPPageFormats::IDD)
	, m_list(0)
	, m_exts(_T(""))
	, m_iRtspHandler(0)
	, m_fRtspFileExtFirst(FALSE)
	, m_bInsufficientPrivileges(false)
{
	if (m_pAAR == NULL) {
		// Default manager (requiered at least Vista)
		HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration,
									  NULL,
									  CLSCTX_INPROC,
									  __uuidof(IApplicationAssociationRegistration),
									  (void**)&m_pAAR);
		UNUSED_ALWAYS(hr);
	}
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
	DDX_Control(pDX, IDC_CHECK6, m_fContextDir);
	DDX_Control(pDX, IDC_CHECK7, m_fContextFiles);
	DDX_Control(pDX, IDC_CHECK8, m_fAssociatedWithIcons);
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

CString CPPageFormats::GetEnqueueCommand()
{
	CString		 path;

	TCHAR buff[_MAX_PATH];
	if (::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH) == 0) {
		return _T("");
	}

	path = buff;
	return _T("\"") + path + _T("\" /add \"%1\"");
}

CString CPPageFormats::GetOpenCommand()
{
	CString		 path;
	TCHAR buff[_MAX_PATH];

	if (::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH) == 0) {
		return _T("");
	}

	path = buff;
	return _T("\"") + path + _T("\" \"%1\"");
}

bool CPPageFormats::IsRegistered(CString ext)
{
	HRESULT	hr;
	BOOL	bIsDefault = FALSE;
	CString strProgID = PROGID + ext;

	if (m_pAAR == NULL) {
		// Default manager (requires at least Vista)
		hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration,
							  NULL,
							  CLSCTX_INPROC,
							  __uuidof(IApplicationAssociationRegistration),
							  (void**)&m_pAAR);
	}

	if (m_pAAR) {
		// The Vista way
		hr = m_pAAR->QueryAppIsDefault(ext, AT_FILEEXTENSION, AL_EFFECTIVE, g_strRegisteredAppName, &bIsDefault);
	} else {
		// The 2000/XP way
		CRegKey		key;
		TCHAR		buff[256];
		ULONG		len = sizeof(buff)/sizeof(buff[0]);
		memset(buff, 0, sizeof(buff));

		if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext)) {
			return false;
		}

		if (ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty()) {
			return false;
		}

		bIsDefault = (buff == strProgID);
	}
	if (!f_setContextFiles) {
		CRegKey		key;
		TCHAR		buff[_MAX_PATH];
		ULONG		len = sizeof(buff)/sizeof(buff[0]);

		if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open"), KEY_READ)) {
			CString		strCommand = ResStr(IDS_OPEN_WITH_MPC);
			if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len)) {
				f_setContextFiles = (strCommand.CompareNoCase(CString(buff)) == 0);
			}
		}
	}

	// Check if association is for this instance of MPC
	if (bIsDefault) {
		CRegKey		key;
		TCHAR		buff[_MAX_PATH];
		ULONG		len = sizeof(buff)/sizeof(buff[0]);

		bIsDefault = FALSE;
		if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open\\command"), KEY_READ)) {
			CString		strCommand = GetOpenCommand();
			if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len)) {
				bIsDefault = (strCommand.CompareNoCase(CString(buff)) == 0);
			}
		}

	}

	return !!bIsDefault;
}

CString GetProgramDir()
{
	CString RtnVal;
	TCHAR    FileName[_MAX_PATH];
	::GetModuleFileName(AfxGetInstanceHandle(), FileName, _MAX_PATH);
	RtnVal = FileName;
	RtnVal = RtnVal.Left(RtnVal.ReverseFind('\\'));
	return RtnVal;
}

int FileExists(const TCHAR *fileName)
{
	DWORD fileAttr;
	fileAttr = ::GetFileAttributes(fileName);
	if (0xFFFFFFFF == fileAttr) {
		return false;
	}
	return true;
}

typedef int (*GetIconIndexFunc)(CString);

int GetIconIndex(CString ext)
{
	int iconindex = -1;
	GetIconIndexFunc _getIconIndexFunc;
	HINSTANCE mpciconlib = LoadLibrary(_T("mpciconlib.dll"));
	if (mpciconlib) {
		_getIconIndexFunc = (GetIconIndexFunc) GetProcAddress(mpciconlib, "get_icon_index");
		if (_getIconIndexFunc) {
			iconindex = _getIconIndexFunc(ext);
		}
		FreeLibrary(mpciconlib);
	}

	return iconindex;
}

bool CPPageFormats::RegisterExt(CString ext, CString strLabel, bool fRegister)
{
	CRegKey		key;
	bool		bSetValue;
	CString strProgID = PROGID + ext;

	if (!fRegister) {
		if (fRegister != IsRegistered(ext)) {
			SetFileAssociation (ext, strProgID, fRegister);
		}
		key.Attach(HKEY_CLASSES_ROOT);
		key.RecurseDeleteKey(strProgID);
		return(true);
	}

	bSetValue = fRegister || (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open\\command"), KEY_READ));

	// Create ProgID for this file type
	if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID)) {
		return(false);
	}
	if (ERROR_SUCCESS != key.SetStringValue(NULL, strLabel)) {
		return(false);
	}

	// Add to playlist option
	if (f_setContextFiles) {
		if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\enqueue"))) {
			return(false);
		}
		if (ERROR_SUCCESS != key.SetStringValue(NULL, ResStr(IDS_ADD_TO_PLAYLIST))) {
			return(false);
		}

		if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\enqueue\\command"))) {
			return(false);
		}
		if (bSetValue && (ERROR_SUCCESS != key.SetStringValue(NULL, GetEnqueueCommand()))) {
			return(false);
		}
	} else {
		key.Close();
		key.Attach(HKEY_CLASSES_ROOT);
		key.RecurseDeleteKey(strProgID + _T("\\shell\\enqueue"));
	}

	// Play option
	if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open"))) {
		return(false);
	}
	if (f_setContextFiles) {
		if (ERROR_SUCCESS != key.SetStringValue(NULL, ResStr(IDS_OPEN_WITH_MPC))) {
			return(false);
		}
	} else {
		if (ERROR_SUCCESS != key.SetStringValue(NULL, _T(""))) {
			return(false);
		}
	}

	if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open\\command"))) {
		return(false);
	}
	if (bSetValue && (ERROR_SUCCESS != key.SetStringValue(NULL, GetOpenCommand()))) {
		return(false);
	}

	if (ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, g_strRegisteredKey + _T("\\FileAssociations"))) {
		return(false);
	}
	if (ERROR_SUCCESS != key.SetStringValue(ext, strProgID)) {
		return(false);
	}

	if (f_setAssociatedWithIcon) {
		CString AppIcon = _T("");
		TCHAR buff[_MAX_PATH];

		CString mpciconlib = GetProgramDir() + _T("\\mpciconlib.dll");

		if (FileExists(mpciconlib)) {
			int icon_index = GetIconIndex(ext);
			CString m_typeicon = mpciconlib;

			/* icon_index value -1 means no icon was found in the iconlib for the file extension */
			if ((icon_index >= 0) && ExtractIcon(AfxGetApp()->m_hInstance,(LPCWSTR)m_typeicon, icon_index)) {
				m_typeicon = "\""+mpciconlib+"\"";
				AppIcon.Format(_T("%s,%d"), m_typeicon, icon_index);
			}
		}

		/* no icon was found for the file extension, so use MPC's icon */
		if ((AppIcon.IsEmpty()) && (::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH))) {
			AppIcon = buff;
			AppIcon = "\""+AppIcon+"\"";
			AppIcon += _T(",0");
		}

		if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\DefaultIcon"))) {
			return(false);
		}
		if (bSetValue && (ERROR_SUCCESS != key.SetStringValue(NULL, AppIcon))) {
			return(false);
		}
	} else {
		key.Attach(HKEY_CLASSES_ROOT);
		key.RecurseDeleteKey(strProgID + _T("\\DefaultIcon"));
	}

	if (fRegister != IsRegistered(ext)) {
		SetFileAssociation (ext, strProgID, fRegister);
	}

	return(true);
}

static struct {
	LPCSTR verb, cmd;
	UINT action;
} handlers[] = {
	{"VideoFiles", " %1", IDS_AUTOPLAY_PLAYVIDEO},
	{"MusicFiles", " %1", IDS_AUTOPLAY_PLAYMUSIC},
	{"CDAudio", " %1 /cd", IDS_AUTOPLAY_PLAYAUDIOCD},
	{"DVDMovie", " %1 /dvd", IDS_AUTOPLAY_PLAYDVDMOVIE},
};

void CPPageFormats::AddAutoPlayToRegistry(autoplay_t ap, bool fRegister)
{
	TCHAR buff[_MAX_PATH];
	if (::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH) == 0) {
		return;
	}
	CString exe = buff;

	int i = (int)ap;
	if (i < 0 || i >= countof(handlers)) {
		return;
	}

	CRegKey key;

	if (fRegister) {
		if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, _T("MediaPlayerClassic.Autorun"))) {
			return;
		}
		key.Close();

		if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT,
										CString(CStringA("MediaPlayerClassic.Autorun\\Shell\\Play") + handlers[i].verb + "\\Command"))) {
			return;
		}
		key.SetStringValue(NULL, _T("\"") + exe + _T("\"") + handlers[i].cmd);
		key.Close();

		if (ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE,
										CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\Handlers\\MPCPlay") + handlers[i].verb + "OnArrival"))) {
			return;
		}
		key.SetStringValue(_T("Action"), ResStr(handlers[i].action));
		key.SetStringValue(_T("Provider"), _T("Media Player Classic"));
		key.SetStringValue(_T("InvokeProgID"), _T("MediaPlayerClassic.Autorun"));
		key.SetStringValue(_T("InvokeVerb"), CString(CStringA("Play") + handlers[i].verb));
		key.SetStringValue(_T("DefaultIcon"), exe + _T(",0"));
		key.Close();

		if (ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE,
										CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\Play") + handlers[i].verb + "OnArrival"))) {
			return;
		}
		key.SetStringValue(CString(CStringA("MPCPlay") + handlers[i].verb + "OnArrival"), _T(""));
		key.Close();
	} else {
		if (ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE,
										CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\Play") + handlers[i].verb + "OnArrival"))) {
			return;
		}
		key.DeleteValue(CString(CStringA("MPCPlay") + handlers[i].verb + "OnArrival"));
		key.Close();
	}
}

bool CPPageFormats::IsAutoPlayRegistered(autoplay_t ap)
{
	ULONG len;
	TCHAR buff[_MAX_PATH];
	if (::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH) == 0) {
		return(false);
	}
	CString exe = buff;

	int i = (int)ap;
	if (i < 0 || i >= countof(handlers)) {
		return(false);
	}

	CRegKey key;

	if (ERROR_SUCCESS != key.Open(HKEY_LOCAL_MACHINE,
								  CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\Play") + handlers[i].verb + "OnArrival"),
								  KEY_READ)) {
		return(false);
	}
	len = countof(buff);
	if (ERROR_SUCCESS != key.QueryStringValue(
				CString(_T("MPCPlay")) + handlers[i].verb + _T("OnArrival"),
				buff, &len)) {
		return(false);
	}
	key.Close();

	if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT,
								  CString(CStringA("MediaPlayerClassic.Autorun\\Shell\\Play") + handlers[i].verb + "\\Command"),
								  KEY_READ)) {
		return(false);
	}
	len = countof(buff);
	if (ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len)) {
		return(false);
	}
	if (_tcsnicmp(_T("\"") + exe, buff, exe.GetLength() + 1)) {
		return(false);
	}
	key.Close();

	return(true);
}

void CPPageFormats::SetListItemState(int nItem)
{
	if (nItem < 0) {
		return;
	}

	CString str = AfxGetAppSettings().m_Formats[(int)m_list.GetItemData(nItem)].GetExtsWithPeriod();

	CAtlList<CString> exts;
	ExplodeMin(str, exts, ' ');

	int cnt = 0;

	POSITION pos = exts.GetHeadPosition();
	while (pos) if (IsRegistered(exts.GetNext(pos))) {
			cnt++;
		}

	if (cnt != 0) {
		cnt = (cnt == (int)exts.GetCount() ? 1 : 2);
	}
	SetChecked(nItem, cnt);
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
	ON_BN_CLICKED(IDC_BUTTON5, OnBnVistaModify)
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

	CMediaFormats& mf = AfxGetAppSettings().m_Formats;
	mf.UpdateData(FALSE);
	for (int i = 0; i < (int)mf.GetCount(); i++) {
		CString label;
		label.Format (_T("%s (%s)"), mf[i].GetLabel(), mf[i].GetExts());

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
	engine_t e = s.m_Formats.GetRtspHandler(fRtspFileExtFirst);
	m_iRtspHandler = (e==RealMedia?0:e==QuickTime?1:2);
	m_fRtspFileExtFirst = fRtspFileExtFirst;

	UpdateData(FALSE);

	f_setContextFiles = 0;

	for (int i = 0; i < m_list.GetItemCount(); i++) {
		SetListItemState(i);
	}
	m_fContextFiles.SetCheck(f_setContextFiles);

	m_apvideo.SetCheck(IsAutoPlayRegistered(AP_VIDEO));
	m_apmusic.SetCheck(IsAutoPlayRegistered(AP_MUSIC));
	m_apaudiocd.SetCheck(IsAutoPlayRegistered(AP_AUDIOCD));
	m_apdvd.SetCheck(IsAutoPlayRegistered(AP_DVDMOVIE));

	CreateToolTip();


	if (IsWinVistaOrLater() && !IsUserAnAdmin()) {
		GetDlgItem(IDC_BUTTON1)->ShowWindow (SW_HIDE);
		GetDlgItem(IDC_BUTTON3)->ShowWindow (SW_HIDE);
		GetDlgItem(IDC_BUTTON4)->ShowWindow (SW_HIDE);
		GetDlgItem(IDC_CHECK1)->EnableWindow (FALSE);
		GetDlgItem(IDC_CHECK2)->EnableWindow (FALSE);
		GetDlgItem(IDC_CHECK3)->EnableWindow (FALSE);
		GetDlgItem(IDC_CHECK4)->EnableWindow (FALSE);
		GetDlgItem(IDC_CHECK5)->EnableWindow (FALSE);

		GetDlgItem(IDC_RADIO1)->EnableWindow (FALSE);
		GetDlgItem(IDC_RADIO2)->EnableWindow (FALSE);
		GetDlgItem(IDC_RADIO3)->EnableWindow (FALSE);

		GetDlgItem(IDC_BUTTON5)->ShowWindow (SW_SHOW);
		GetDlgItem(IDC_BUTTON5)->SendMessage (BCM_SETSHIELD, 0, 1);

		m_bInsufficientPrivileges = true;
	} else {
		GetDlgItem(IDC_BUTTON5)->ShowWindow (SW_HIDE);
	}


	CRegKey		key;
	TCHAR		buff[_MAX_PATH];
	ULONG		len = sizeof(buff)/sizeof(buff[0]);

	int fContextDir = 0;
	if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("Directory\\shell\\") PROGID _T(".play\\command"), KEY_READ)) {
		CString		strCommand = GetOpenCommand();
		if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len)) {
			fContextDir = (strCommand.CompareNoCase(CString(buff)) == 0);
		}
	}
	m_fContextDir.SetCheck(fContextDir);
	m_fAssociatedWithIcons.SetCheck(s.fAssociatedWithIcons);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageFormats::SetFileAssociation(CString strExt, CString strProgID, bool fRegister)
{
	CString		extoldreg, extOldIcon;
	CRegKey		key;
	HRESULT		hr = S_OK;
	TCHAR		buff[256];
	ULONG		len = sizeof(buff)/sizeof(buff[0]);
	memset(buff, 0, sizeof(buff));

	if (m_pAAR == NULL) {
		// Default manager (requiered at least Vista)
		HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration,
									  NULL,
									  CLSCTX_INPROC,
									  __uuidof(IApplicationAssociationRegistration),
									  (void**)&m_pAAR);
		UNUSED_ALWAYS(hr);
	}

	if (m_pAAR) {
		// The Vista way
		CString		strNewApp;
		if (fRegister) {
			// Create non existing file type
			if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strExt)) {
				return(false);
			}

			WCHAR*		pszCurrentAssociation;
			// Save current application associated
			if (SUCCEEDED (m_pAAR->QueryCurrentDefault (strExt, AT_FILEEXTENSION, AL_EFFECTIVE, &pszCurrentAssociation))) {
				if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID)) {
					return(false);
				}

				key.SetStringValue(g_strOldAssoc, pszCurrentAssociation);

				// Get current icon for file type
				/*
				if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, CString(pszCurrentAssociation) + _T("\\DefaultIcon")))
				{
					len = sizeof(buff);
					memset(buff, 0, len);
					if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty())
					{
						if (ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\DefaultIcon")))
							key.SetStringValue (NULL, buff);
					}
				}
				*/
				CoTaskMemFree (pszCurrentAssociation);
			}
			strNewApp = g_strRegisteredAppName;
		} else {
			if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, strProgID)) {
				return(false);
			}

			if (ERROR_SUCCESS == key.QueryStringValue(g_strOldAssoc, buff, &len)) {
				strNewApp = buff;
			}

			// TODO : retrieve registered app name from previous association (or find Bill function for that...)
		}

		hr = m_pAAR->SetAppAsDefault(strNewApp, strExt, AT_FILEEXTENSION);
	} else {
		// The 2000/XP way
		if (fRegister) {
			// Set new association
			if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strExt)) {
				return(false);
			}

			len = sizeof(buff)/sizeof(buff[0]);
			memset(buff, 0, sizeof(buff));
			if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty()) {
				extoldreg = buff;
			}
			if (ERROR_SUCCESS != key.SetStringValue(NULL, strProgID)) {
				return(false);
			}

			// Get current icon for file type
			/*
			if (!extoldreg.IsEmpty())
			{
				if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, extoldreg + _T("\\DefaultIcon")))
				{
					len = sizeof(buff);
					memset(buff, 0, len);
					if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty())
						extOldIcon = buff;
				}
			}
			*/

			// Save old association
			if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID)) {
				return(false);
			}
			key.SetStringValue(g_strOldAssoc, extoldreg);

			/*
			if (!extOldIcon.IsEmpty() && (ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\DefaultIcon"))))
				key.SetStringValue (NULL, extOldIcon);
			*/
		} else {
			// Get previous association
			len = sizeof(buff)/sizeof(buff[0]);
			memset(buff, 0, sizeof(buff));
			if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID)) {
				return(false);
			}
			if (ERROR_SUCCESS == key.QueryStringValue(g_strOldAssoc, buff, &len) && !CString(buff).Trim().IsEmpty()) {
				extoldreg = buff;
			}

			// Set previous association
			if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strExt)) {
				return(false);
			}
			key.SetStringValue(NULL, extoldreg);
		}

	}

	return SUCCEEDED (hr);
}

BOOL CPPageFormats::OnApply()
{
	UpdateData();

	{
		int i = m_list.GetSelectionMark();
		if (i >= 0) {
			i = (int)m_list.GetItemData(i);
		}
		if (i >= 0) {
			CMediaFormats& mf = AfxGetAppSettings().m_Formats;
			mf[i].SetExts(m_exts);
			m_exts = mf[i].GetExtsWithPeriod();
			UpdateData(FALSE);
		}
	}

	CMediaFormats& mf = AfxGetAppSettings().m_Formats;

	CString AppIcon = _T("");
	TCHAR buff[_MAX_PATH];

	if (::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH)) {
		AppIcon = buff;
		AppIcon = "\""+AppIcon+"\"";
		AppIcon += _T(",0");
	}

	if (m_pAAR) {
		// Register MPC for the windows "Default application" manager
		CRegKey		key;

		if (ERROR_SUCCESS == key.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\RegisteredApplications"))) {
			key.SetStringValue(_T("Media Player Classic"), g_strRegisteredKey);

			if (ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, g_strRegisteredKey)) {
				return(false);
			}

			// ==>>  TODO icon !!!
			key.SetStringValue(_T("ApplicationDescription"), ResStr(IDS_APP_DESCRIPTION), REG_EXPAND_SZ);
			key.SetStringValue(_T("ApplicationIcon"), AppIcon, REG_EXPAND_SZ);
			key.SetStringValue(_T("ApplicationName"), ResStr(IDR_MAINFRAME), REG_EXPAND_SZ);
		}
	}

	f_setContextFiles = m_fContextFiles.GetCheck();
	f_setAssociatedWithIcon = m_fAssociatedWithIcons.GetCheck();

	for (int i = 0; i < m_list.GetItemCount(); i++) {
		int iChecked = GetChecked(i);
		if (iChecked == 2) {
			continue;
		}

		CAtlList<CString> exts;
		Explode(mf[(int)m_list.GetItemData(i)].GetExtsWithPeriod(), exts, ' ');

		POSITION pos = exts.GetHeadPosition();
		while (pos) {
			RegisterExt(exts.GetNext(pos), mf[(int)m_list.GetItemData(i)].GetLabel(), !!iChecked);
		}
	}

	CRegKey	key;
	if (m_fContextDir.GetCheck()) {
		if (ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, _T("Directory\\shell\\") PROGID _T(".enqueue"))) {
			key.SetStringValue(NULL, ResStr(IDS_ADD_TO_PLAYLIST));
		}

		if (ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, _T("Directory\\shell\\") PROGID _T(".enqueue\\command"))) {
			key.SetStringValue(NULL, GetEnqueueCommand());
		}

		if (ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, _T("Directory\\shell\\") PROGID _T(".play"))) {
			key.SetStringValue(NULL, ResStr(IDS_OPEN_WITH_MPC));
		}

		if (ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, _T("Directory\\shell\\") PROGID _T(".play\\command"))) {
			key.SetStringValue(NULL, GetOpenCommand());
		}
	} else {
		key.Attach(HKEY_CLASSES_ROOT);
		key.RecurseDeleteKey(_T("Directory\\shell\\") PROGID _T(".enqueue"));
		key.RecurseDeleteKey(_T("Directory\\shell\\") PROGID _T(".play"));
	}

	{
		SetListItemState(m_list.GetSelectionMark());
	}

	AddAutoPlayToRegistry(AP_VIDEO, !!m_apvideo.GetCheck());
	AddAutoPlayToRegistry(AP_MUSIC, !!m_apmusic.GetCheck());
	AddAutoPlayToRegistry(AP_AUDIOCD, !!m_apaudiocd.GetCheck());
	AddAutoPlayToRegistry(AP_DVDMOVIE, !!m_apdvd.GetCheck());

	AppSettings& s = AfxGetAppSettings();
	s.m_Formats.SetRtspHandler(m_iRtspHandler==0?RealMedia:m_iRtspHandler==1?QuickTime:DirectShow, !!m_fRtspFileExtFirst);
	s.fAssociatedWithIcons = !!m_fAssociatedWithIcons.GetCheck();

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

	return __super::OnApply();
}

void CPPageFormats::OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

	if (lpnmlv->iItem >= 0 && lpnmlv->iSubItem == COL_CATEGORY) {
		CRect r;
		m_list.GetItemRect(lpnmlv->iItem, r, LVIR_ICON);
		if (r.PtInRect(lpnmlv->ptAction)) {
			if (m_bInsufficientPrivileges) {
				MessageBox (ResStr (IDS_CANNOT_CHANGE_FORMAT));
			} else {
				SetChecked(lpnmlv->iItem, (GetChecked(lpnmlv->iItem)&1) == 0 ? 1 : 0);
				SetModified();
			}
		}
	}

	*pResult = 0;
}

void CPPageFormats::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if (pNMLV->iItem >= 0 && pNMLV->iSubItem == COL_CATEGORY
			&& (pNMLV->uChanged&LVIF_STATE) && (pNMLV->uNewState&LVIS_SELECTED)) {
		m_exts = AfxGetAppSettings().m_Formats[(int)m_list.GetItemData(pNMLV->iItem)].GetExtsWithPeriod();
		UpdateData(FALSE);
	}

	*pResult = 0;
}

void CPPageFormats::OnBeginlabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	*pResult = FALSE;

	if (pItem->iItem < 0) {
		return;
	}

	if (pItem->iSubItem == COL_ENGINE) {
		*pResult = TRUE;
	}
}

void CPPageFormats::OnDolabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	*pResult = FALSE;

	if (pItem->iItem < 0) {
		return;
	}

	CMediaFormatCategory& mfc = AfxGetAppSettings().m_Formats[m_list.GetItemData(pItem->iItem)];

	CAtlList<CString> sl;
	int nSel = -1;

	if (pItem->iSubItem == COL_ENGINE) {
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

	if (!m_list.m_fInPlaceDirty) {
		return;
	}

	if (pItem->iItem < 0) {
		return;
	}

	CMediaFormatCategory& mfc = AfxGetAppSettings().m_Formats[m_list.GetItemData(pItem->iItem)];

	if (pItem->iSubItem == COL_ENGINE && pItem->lParam >= 0) {
		mfc.SetEngineType((engine_t)pItem->lParam);
		m_list.SetItemText(pItem->iItem, pItem->iSubItem, pItem->pszText);
		*pResult = TRUE;
	}

	if (*pResult) {
		SetModified();
	}
}

void CPPageFormats::OnBnClickedButton1()
{
	for (int i = 0, j = m_list.GetItemCount(); i < j; i++) {
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
	CMediaFormats& mf = AfxGetAppSettings().m_Formats;

	for (int i = 0, j = m_list.GetItemCount(); i < j; i++) {
		if (!mf[m_list.GetItemData(i)].GetLabel().CompareNoCase(ResStr(IDS_AG_PLAYLIST_FILE))) {
			SetChecked(i, 0);
			continue;
		}
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
	CMediaFormats& mf = AfxGetAppSettings().m_Formats;

	for (int i = 0, j = m_list.GetItemCount(); i < j; i++) {
		SetChecked(i, mf[(int)m_list.GetItemData(i)].IsAudioOnly()?1:0);
	}

	m_apvideo.SetCheck(0);
	m_apmusic.SetCheck(1);
	m_apaudiocd.SetCheck(1);
	m_apdvd.SetCheck(0);

	SetModified();
}

void CPPageFormats::OnBnVistaModify()
{
	CString			strCmd;
	TCHAR			strApp [_MAX_PATH];

	strCmd.Format (_T("/adminoption %d"), IDD);
	GetModuleFileNameEx (GetCurrentProcess(), AfxGetMyApp()->m_hInstance, strApp, _MAX_PATH);

	AfxGetMyApp()->RunAsAdministrator (strApp, strCmd, true);

	for (int i = 0; i < m_list.GetItemCount(); i++) {
		SetListItemState(i);
	}
}

void CPPageFormats::OnBnClickedButton12()
{
	int i = m_list.GetSelectionMark();
	if (i < 0) {
		return;
	}
	i = (int)m_list.GetItemData(i);
	CMediaFormats& mf = AfxGetAppSettings().m_Formats;
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
	if (i < 0) {
		return;
	}
	i = (int)m_list.GetItemData(i);
	CMediaFormats& mf = AfxGetAppSettings().m_Formats;
	mf[i].SetExts(m_exts);
	m_exts = mf[i].GetExtsWithPeriod();
	SetListItemState(m_list.GetSelectionMark());
	UpdateData(FALSE);

	SetModified();
}

void CPPageFormats::OnUpdateButtonDefault(CCmdUI* pCmdUI)
{
	int i = m_list.GetSelectionMark();
	if (i < 0) {
		pCmdUI->Enable(FALSE);
		return;
	}
	i = (int)m_list.GetItemData(i);

	CString orgexts, newexts;
	GetDlgItem(IDC_EDIT1)->GetWindowText(newexts);
	newexts.Trim();
	orgexts = AfxGetAppSettings().m_Formats[i].GetBackupExtsWithPeriod();

	pCmdUI->Enable(!!newexts.CompareNoCase(orgexts));
}

void CPPageFormats::OnUpdateButtonSet(CCmdUI* pCmdUI)
{
	int i = m_list.GetSelectionMark();
	if (i < 0) {
		pCmdUI->Enable(FALSE);
		return;
	}
	i = (int)m_list.GetItemData(i);

	CString orgexts, newexts;
	GetDlgItem(IDC_EDIT1)->GetWindowText(newexts);
	newexts.Trim();
	orgexts = AfxGetAppSettings().m_Formats[i].GetExtsWithPeriod();

	pCmdUI->Enable(!!newexts.CompareNoCase(orgexts));
}
