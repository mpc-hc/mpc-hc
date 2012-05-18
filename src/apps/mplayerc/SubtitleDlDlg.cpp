/*
 * $Id$
 *
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

#include "stdafx.h"
#include <afxwin.h>
#include "SubtitleDlDlg.h"
#include "MainFrm.h"

// User Defined Window Messages
#define UWM_PARSE (WM_USER + 100)
#define UWM_FAILED (WM_USER + 101)

CSubtitleDlDlg::CSubtitleDlDlg(CWnd* pParent, const CStringA& url)
	: CResizableDialog(CSubtitleDlDlg::IDD, pParent),
	  m_url(url), ps(m_list.GetSafeHwnd(), 0, TRUE), m_status()
{
}

CSubtitleDlDlg::~CSubtitleDlDlg()
{
	delete m_pTA;
}

void CSubtitleDlDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}

void CSubtitleDlDlg::LoadList()
{
	m_list.SetRedraw(FALSE);
	for (int i = 0; i < m_parsed_movies.GetCount(); ++i) {
		isdb_movie_parsed& m = m_parsed_movies[i];

		int iItem = m_list.InsertItem(i, _T(""));
		m_list.SetItemData(iItem, m.ptr);
		m_list.SetItemText(iItem, COL_FILENAME, m.name);
		m_list.SetItemText(iItem, COL_LANGUAGE, m.language);
		m_list.SetItemText(iItem, COL_FORMAT, m.format);
		m_list.SetItemText(iItem, COL_DISC, m.disc);
		m_list.SetItemText(iItem, COL_TITLES, m.titles);
		m_list.SetCheck(iItem, m.checked);
	}

	m_list.SetRedraw(TRUE);
	m_list.Invalidate();
	m_list.UpdateWindow();
}

bool CSubtitleDlDlg::Parse()
{
	// Parse raw list
	isdb_movie m;
	isdb_subtitle sub;

	CAtlList<CStringA> sl;
	Explode(m_pTA->raw_list, sl, '\n');
	CString str;

	POSITION pos = sl.GetHeadPosition();
	while (pos) {
		str = sl.GetNext(pos);

		CStringA param = str.Left(max(0, str.Find('=')));
		CStringA value = str.Mid(str.Find('=')+1);

		if (param == "ticket") {
			m_pTA->ticket = value;
		} else if (param == "movie") {
			m.reset();
			Explode(value, m.titles, '|');
		} else if (param == "subtitle") {
			sub.reset();
			sub.id = atoi(value);
		} else if (param == "name") {
			sub.name = value;
		} else if (param == "discs") {
			sub.discs = atoi(value);
		} else if (param == "disc_no") {
			sub.disc_no = atoi(value);
		} else if (param == "format") {
			sub.format = value;
		} else if (param == "iso639_2") {
			sub.iso639_2 = value;
		} else if (param == "language") {
			sub.language = value;
		} else if (param == "nick") {
			sub.nick = value;
		} else if (param == "email") {
			sub.email = value;
		} else if (param == "" && value == "endsubtitle") {
			m.subs.AddTail(sub);
		} else if (param == "" && value == "endmovie") {
			m_pTA->raw_movies.AddTail(m);
		} else if (param == "" && value == "end") {
			break;
		}
	}

	// Parse movies
	pos = m_pTA->raw_movies.GetHeadPosition();
	while (pos) {
		isdb_movie& m = m_pTA->raw_movies.GetNext(pos);
		isdb_movie_parsed p;

		CStringA titlesA = Implode(m.titles, '|');
		titlesA.Replace("|", ", ");
		p.titles = UTF8To16(titlesA);
		p.checked = false;

		POSITION pos2 = m.subs.GetHeadPosition();
		while (pos2) {
			const isdb_subtitle& s = m.subs.GetNext(pos2);
			p.name = UTF8To16(s.name);
			p.language = s.language;
			p.format = s.format;
			p.disc.Format(_T("%d/%d"), s.disc_no, s.discs);
			p.ptr = reinterpret_cast<DWORD_PTR>(&s);

			m_parsed_movies.Add(p);
		}
	}

	bool ret = true;
	if (m_parsed_movies.GetCount() == 0) {
		ret = false;
	}

	return ret;
}

void CSubtitleDlDlg::SetStatus(const CString& status)
{
	m_status.SetText(status, 0, 0);
}

UINT CSubtitleDlDlg::RunThread(LPVOID pParam) {
	PTHREADSTRUCT pTA = reinterpret_cast<PTHREADSTRUCT>(pParam);

	if (!OpenUrl(pTA->is, CString(pTA->url), pTA->raw_list)) {
		::PostMessage(pTA->hWND, UWM_FAILED, (WPARAM)0, (LPARAM)0);
		AfxEndThread(1, TRUE);
	}

	::PostMessage(pTA->hWND, UWM_PARSE, (WPARAM)0, (LPARAM)0);

	return 0;
};

int CALLBACK CSubtitleDlDlg::SortCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	PPARAMSORT ps = reinterpret_cast<PPARAMSORT>(lParamSort);
	TCHAR left[256] = _T(""), right[256] = _T("");

	ListView_GetItemText(ps->m_hWnd, lParam1, ps->m_colIndex, left, sizeof(left));
	ListView_GetItemText(ps->m_hWnd, lParam2, ps->m_colIndex, right, sizeof(right));

	if (ps->m_ascending) {
		return _tcscmp(left, right);
	} else {
		return _tcscmp(right, left);
	}
}

BOOL CSubtitleDlDlg::OnInitDialog()
{
	__super::OnInitDialog();

	m_status.Create(WS_CHILD|WS_VISIBLE|CCS_BOTTOM, CRect(0,0,0,0), this, IDC_STATUSBAR);

	int n, curPos = 0;
	CArray<int> columnWidth;

	CString strColumnWidth = AfxGetApp()->GetProfileString(IDS_R_DLG_SUBTITLEDL, IDS_RS_DLG_SUBTITLEDL_COLWIDTH, _T(""));
	CString token = strColumnWidth.Tokenize(_T(","), curPos);
	while (!token.IsEmpty()) {
		if (_stscanf_s(token, L"%i", &n) == 1) {
			columnWidth.Add(n);
			token = strColumnWidth.Tokenize(_T(","), curPos);
		} else {
			throw 1;
		}
	}

	m_list.SetExtendedStyle(m_list.GetExtendedStyle()
							| LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT
							| LVS_EX_CHECKBOXES   | LVS_EX_LABELTIP);

	if (columnWidth.GetCount() != 5) {
		// default sizes
		columnWidth.RemoveAll();
		columnWidth.Add(290);
		columnWidth.Add(70);
		columnWidth.Add(50);
		columnWidth.Add(50);
		columnWidth.Add(270);
	}

	m_list.InsertColumn(COL_FILENAME, ResStr(IDS_SUBDL_DLG_FILENAME_COL), LVCFMT_LEFT, columnWidth[0]);
	m_list.InsertColumn(COL_LANGUAGE, ResStr(IDS_SUBDL_DLG_LANGUAGE_COL), LVCFMT_CENTER, columnWidth[1]);
	m_list.InsertColumn(COL_FORMAT, ResStr(IDS_SUBDL_DLG_FORMAT_COL), LVCFMT_CENTER, columnWidth[2]);
	m_list.InsertColumn(COL_DISC, ResStr(IDS_SUBDL_DLG_DISC_COL), LVCFMT_CENTER, columnWidth[3]);
	m_list.InsertColumn(COL_TITLES, ResStr(IDS_SUBDL_DLG_TITLES_COL), LVCFMT_LEFT, columnWidth[4]);

	AddAnchor(IDC_LIST1, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_CHECK1, BOTTOM_LEFT);
	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDC_STATUSBAR, BOTTOM_LEFT, BOTTOM_RIGHT);

	const CSize s(420, 200);
	SetMinTrackSize(s);
	EnableSaveRestore(IDS_R_DLG_SUBTITLEDL);

	// start new worker thread to download the list of subtitles
	m_pTA = DNew THREADSTRUCT;
	m_pTA->url = m_url;
	m_pTA->hWND = GetSafeHwnd();

	SetStatus(ResStr(IDS_SUBDL_DLG_DOWNLOADING));
	AfxBeginThread(RunThread, static_cast<LPVOID>(m_pTA));

	return TRUE;
}

void CSubtitleDlDlg::OnOK()
{
	SetStatus(ResStr(IDS_SUBDL_DLG_DOWNLOADING));

	for (int i = 0; i < m_list.GetItemCount(); ++i) {
		if (m_list.GetCheck(i)) {
			m_selsubs.AddTail(*reinterpret_cast<isdb_subtitle*>(m_list.GetItemData(i)));
		}
	}

	m_fReplaceSubs = IsDlgButtonChecked(IDC_CHECK1) == BST_CHECKED;

	CMainFrame* pMF = static_cast<CMainFrame*>(GetParentFrame());

	if (m_fReplaceSubs) {
		pMF->m_pSubStreams.RemoveAll();
	}

	CComPtr<ISubStream> pSubStreamToSet;

	POSITION pos = m_selsubs.GetHeadPosition();
	while (pos) {
		const isdb_subtitle& sub = m_selsubs.GetNext(pos);
		AppSettings& s = AfxGetAppSettings();
		CInternetSession is;
		CStringA url = "http://" + s.strISDb + "/dl.php?";
		CStringA args, ticket, str;
		args.Format("id=%d&ticket=%s", sub.id, UrlEncode(ticket));
		url.Append(args);

		if (OpenUrl(is, CString(url), str)) {
			CAutoPtr<CRenderedTextSubtitle> pRTS(DNew CRenderedTextSubtitle(&pMF->m_csSubLock, &s.subdefstyle, s.fUseDefaultSubtitlesStyle));
			if (pRTS && pRTS->Open((BYTE*)(LPCSTR)str, str.GetLength(), DEFAULT_CHARSET, CString(sub.name)) && pRTS->GetStreamCount() > 0) {
				CComPtr<ISubStream> pSubStream = pRTS.Detach();
				pMF->m_pSubStreams.AddTail(pSubStream);
				if (!pSubStreamToSet) {
					pSubStreamToSet = pSubStream;
				}
			}
		}
	}

	if (pSubStreamToSet) {
		pMF->SetSubtitle(pSubStreamToSet);
	}

	__super::OnOK();
}

void CSubtitleDlDlg::OnUpdateOk(CCmdUI* pCmdUI)
{
	bool fEnable = false;
	for (int i = 0; !fEnable && i < m_list.GetItemCount(); ++i) {
		fEnable = !!m_list.GetCheck(i);
	}

	pCmdUI->Enable(fEnable);
}

void CSubtitleDlDlg::OnFailedConnection()
{
	SetStatus(ResStr(IDS_SUBDL_DLG_CONNECT_ERROR));
}

void CSubtitleDlDlg::OnParse()
{
	SetStatus(ResStr(IDS_SUBDL_DLG_PARSING));
	if (Parse()) {
		LoadList();
		CString msg;
		msg.Format(ResStr(IDS_SUBDL_DLG_SUBS_AVAIL), m_list.GetItemCount());
		SetStatus(msg);
	}
	else {
		SetStatus(ResStr(IDS_SUBDL_DLG_NOT_FOUND));
	}
}

void CSubtitleDlDlg::OnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	*pResult = 0;

	if (phdr->iItem == ps.m_colIndex) {
		ps.m_ascending = !ps.m_ascending;
	} else {
		ps.m_ascending = TRUE;
	}
	ps.m_colIndex = phdr->iItem;
	ps.m_hWnd = m_list.GetSafeHwnd();

	SetRedraw(FALSE);
	ListView_SortItemsEx(m_list.GetSafeHwnd(), SortCompare, &ps);
	SetRedraw(TRUE);
	m_list.Invalidate();
	m_list.UpdateWindow();
}

void CSubtitleDlDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	ArrangeLayout();
}

void CSubtitleDlDlg::OnDestroy()
{
	RemoveAllAnchors();

	const CHeaderCtrl& pHC = *m_list.GetHeaderCtrl();
	CString strColumnWidth;
	int w;

	for (int i = 0; i < pHC.GetItemCount(); ++i) {
		w = m_list.GetColumnWidth(i);
		strColumnWidth.AppendFormat(L"%d,", w);
	}
	AfxGetApp()->WriteProfileString(IDS_R_DLG_SUBTITLEDL, IDS_RS_DLG_SUBTITLEDL_COLWIDTH, strColumnWidth);

	__super::OnDestroy();
}

BOOL CSubtitleDlDlg::OnEraseBkgnd(CDC* pDC)
{
	EraseBackground(pDC);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CSubtitleDlDlg, CResizableDialog)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE_VOID(UWM_PARSE, OnParse)
	ON_MESSAGE_VOID(UWM_FAILED, OnFailedConnection)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnColumnClick)
	ON_WM_DESTROY()
END_MESSAGE_MAP()
