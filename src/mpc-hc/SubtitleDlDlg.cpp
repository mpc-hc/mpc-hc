/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include "DSUtil.h"

// User Defined Window Messages
#define UWM_PARSE  (WM_USER + 100)
#define UWM_FAILED (WM_USER + 101)

CSubtitleDlDlg::CSubtitleDlDlg(CWnd* pParent, const CStringA& url, const CString& filename)
    : CResizableDialog(CSubtitleDlDlg::IDD, pParent)
    , m_url(url)
    , ps(m_list.GetSafeHwnd(), 0, TRUE)
    , defps(m_list.GetSafeHwnd(), filename)
    , m_status()
    , m_pTA(nullptr)
    , m_fReplaceSubs(false)
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

size_t CSubtitleDlDlg::StrMatch(LPCTSTR a, LPCTSTR b)
{
    size_t count = 0, alen = _tcslen(a), blen = _tcslen(b);

    for (size_t i = 0; i < alen && i < blen; i++) {
        if (_totlower(a[i]) != _totlower(b[i])) {
            break;
        } else {
            count++;
        }
    }
    return count;
}

CString CSubtitleDlDlg::LangCodeToName(LPCSTR code)
{
    // accept only three-letter language codes
    size_t codeLen = strlen(code);
    if (codeLen != 3) {
        return _T("");
    }

    CString name = ISO6392ToLanguage(code);
    if (!name.IsEmpty()) {
        // workaround for ISO6392ToLanguage function behaivior
        // for unknown language code it returns the code parameter back
        if (code != name) {
            return name;
        }
    }

    // support abbreviations loosely based on first letters of language name

    // this list is limited to upload-enabled languages
    // retrieved with:
    // wget -q -O- http://www.opensubtitles.org/addons/export_languages.php | \
    // awk 'NR > 1 { if ($(NF-1) == "1") print ("\"" $(NF-2)  "\",")}'
    static LPCSTR ltable[] = {
        "Albanian",  "Arabic",    "Armenian",  "Basque",     "Bengali",       "Bosnian",    "Breton",    "Bulgarian",
        "Burmese",   "Catalan",   "Chinese",   "Czech",      "Danish",        "Dutch",      "English",   "Esperanto",
        "Estonian",  "Finnish",   "French",    "Georgian",   "German",        "Galician",   "Greek",     "Hebrew",
        "Hindi",     "Croatian",  "Hungarian", "Icelandic",  "Indonesian",    "Italian",    "Japanese",  "Kazakh",
        "Khmer",     "Korean",    "Latvian",   "Lithuanian", "Luxembourgish", "Macedonian", "Malayalam", "Malay",
        "Mongolian", "Norwegian", "Occitan",   "Persian",    "Polish",        "Portuguese", "Russian",   "Serbian",
        "Sinhalese", "Slovak",    "Slovenian", "Spanish",    "Swahili",       "Swedish",    "Syriac",    "Telugu",
        "Tagalog",   "Thai",      "Turkish",   "Ukrainian",  "Urdu",          "Vietnamese", "Romanian",  "Brazilian",
    };

    for (size_t i = 0; i < _countof(ltable); ++i) {
        CString name = ltable[i];
        if (StrMatch(name, CString(code)) == codeLen) {
            return name;
        }
    }
    return _T("");
}

int CALLBACK CSubtitleDlDlg::DefSortCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    PDEFPARAMSORT defps = reinterpret_cast<PDEFPARAMSORT>(lParamSort);
    TCHAR left[MAX_PATH] = _T(""), right[MAX_PATH] = _T("");

    // sort by language first
    ListView_GetItemText(defps->m_hWnd, lParam1, COL_LANGUAGE, left, sizeof(left));
    ListView_GetItemText(defps->m_hWnd, lParam2, COL_LANGUAGE, right, sizeof(right));
    // user-provided sort order
    int lpos, rpos;
    if (!defps->m_langPos.Lookup(left, lpos)) {
        lpos = INT_MAX;
    }
    if (!defps->m_langPos.Lookup(right, rpos)) {
        rpos = INT_MAX;
    }
    if (lpos < rpos) {
        return -1;
    } else if (lpos > rpos) {
        return 1;
    } else if (lpos == INT_MAX && rpos == INT_MAX) {
        // lexicographical order
        int res = _tcscmp(left, right);
        if (res != 0) {
            return res;
        }
    }

    // sort by filename
    ListView_GetItemText(defps->m_hWnd, lParam1, COL_FILENAME, left, sizeof(left));
    ListView_GetItemText(defps->m_hWnd, lParam2, COL_FILENAME, right, sizeof(right));
    size_t lmatch = StrMatch(defps->m_filename, left);
    size_t rmatch = StrMatch(defps->m_filename, right);
    // sort by matching character number
    if (lmatch > rmatch) {
        return -1;
    } else if (lmatch < rmatch) {
        return 1;
    }
    // prefer shorter names
    size_t llen = _tcslen(left);
    size_t rlen = _tcslen(right);
    if (llen < rlen) {
        return -1;
    } else if (llen > rlen) {
        return 1;
    }
    return 0;
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

    // sort by language and filename
    defps.m_hWnd = m_list.GetSafeHwnd();
    ListView_SortItemsEx(m_list.GetSafeHwnd(), DefSortCompare, &defps);

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
        CStringA value = str.Mid(str.Find('=') + 1);

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
        } else if (param.IsEmpty() && value == "endsubtitle") {
            m.subs.AddTail(sub);
        } else if (param.IsEmpty() && value == "endmovie") {
            m_pTA->raw_movies.AddTail(m);
        } else if (param.IsEmpty() && value == "end") {
            break;
        }
    }

    // Parse movies
    pos = m_pTA->raw_movies.GetHeadPosition();
    while (pos) {
        isdb_movie& raw_movie = m_pTA->raw_movies.GetNext(pos);
        isdb_movie_parsed p;

        CStringA titlesA = Implode(raw_movie.titles, '|');
        titlesA.Replace("|", ", ");
        p.titles = UTF8To16(titlesA);
        p.checked = false;

        POSITION pos2 = raw_movie.subs.GetHeadPosition();
        while (pos2) {
            const isdb_subtitle& s = raw_movie.subs.GetNext(pos2);
            p.name = UTF8To16(s.name);
            p.language = s.language;
            p.format = s.format;
            p.disc.Format(_T("%d/%d"), s.disc_no, s.discs);
            p.ptr = reinterpret_cast<DWORD_PTR>(&s);

            m_parsed_movies.Add(p);
        }
    }

    bool ret = true;
    if (m_parsed_movies.IsEmpty()) {
        ret = false;
    }

    return ret;
}

void CSubtitleDlDlg::SetStatus(const CString& status)
{
    m_status.SetText(status, 0, 0);
}

UINT CSubtitleDlDlg::RunThread(LPVOID pParam)
{
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
    TCHAR left[MAX_PATH] = _T(""), right[MAX_PATH] = _T("");

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

    m_status.Create(WS_CHILD | WS_VISIBLE | CCS_BOTTOM, CRect(0, 0, 0, 0), this, IDC_STATUSBAR);

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

    // set language sorting order
    const CAppSettings& settings = AfxGetAppSettings();
    CString order = settings.strSubtitlesLanguageOrder;
    // fill language->position map
    int listPos = 0;
    int tPos = 0;
    CString langCode = order.Tokenize(_T(",; "), tPos);
    while (tPos != -1) {
        CString langName = LangCodeToName(CStringA(langCode));
        if (!langName.IsEmpty()) {
            int pos;
            if (!defps.m_langPos.Lookup(langName, pos)) {
                defps.m_langPos[langName] = listPos++;
            }
        }
        langCode = order.Tokenize(_T(",; "), tPos);
    }

    // start new worker thread to download the list of subtitles
    m_pTA = DEBUG_NEW THREADSTRUCT;
    m_pTA->url = m_url;
    m_pTA->hWND = GetSafeHwnd();

    SetStatus(ResStr(IDS_SUBDL_DLG_DOWNLOADING));
    AfxBeginThread(RunThread, static_cast<LPVOID>(m_pTA));

    return TRUE;
}

BOOL CSubtitleDlDlg::PreTranslateMessage(MSG* pMsg)
{
    // Inhibit default handling for the Enter key when the list has the focus and an item is selected.
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN
            && pMsg->hwnd == m_list.GetSafeHwnd() && m_list.GetSelectedCount() > 0) {
        return FALSE;
    }

    return __super::PreTranslateMessage(pMsg);
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

    CAppSettings& s = AfxGetAppSettings();
    CComPtr<ISubStream> pSubStreamToSet;

    POSITION pos = m_selsubs.GetHeadPosition();
    while (pos) {
        const isdb_subtitle& sub = m_selsubs.GetNext(pos);
        CInternetSession is;
        CStringA url = "http://" + s.strISDb + "/dl.php?";
        CStringA ticket = UrlEncode(m_pTA->ticket);
        CStringA args, str;
        args.Format("id=%d&ticket=%s", sub.id, ticket);
        url.Append(args);

        if (OpenUrl(is, CString(url), str)) {
            CAutoPtr<CRenderedTextSubtitle> pRTS(DEBUG_NEW CRenderedTextSubtitle(&pMF->m_csSubLock, &s.subdefstyle, s.fUseDefaultSubtitlesStyle));
            if (pRTS && pRTS->Open((BYTE*)(LPCSTR)str, str.GetLength(), DEFAULT_CHARSET, CString(sub.name)) && pRTS->GetStreamCount() > 0) {
                SubtitleInput subElement(pRTS.Detach());
                pMF->m_pSubStreams.AddTail(subElement);
                if (!pSubStreamToSet) {
                    pSubStreamToSet = subElement.subStream;
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
        msg.Format(IDS_SUBDL_DLG_SUBS_AVAIL, m_list.GetItemCount());
        SetStatus(msg);
    } else {
        SetStatus(ResStr(IDS_SUBDL_DLG_NOT_FOUND));
    }
}

void CSubtitleDlDlg::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
    *pResult = 0;

    if (phdr->iItem == ps.m_colIndex) {
        ps.m_ascending = !ps.m_ascending;
    } else {
        ps.m_ascending = true;
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

void CSubtitleDlDlg::DownloadSelectedSubtitles()
{
    POSITION pos = m_list.GetFirstSelectedItemPosition();
    while (pos) {
        int nItem = m_list.GetNextSelectedItem(pos);
        if (nItem >= 0 && nItem < m_list.GetItemCount()) {
            ListView_SetCheckState(m_list.GetSafeHwnd(), nItem, TRUE);
        }
    }
    OnOK();
}

BEGIN_MESSAGE_MAP(CSubtitleDlDlg, CResizableDialog)
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_MESSAGE_VOID(UWM_PARSE, OnParse)
    ON_MESSAGE_VOID(UWM_FAILED, OnFailedConnection)
    ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOk)
    ON_NOTIFY(HDN_ITEMCLICK, 0, OnColumnClick)
    ON_WM_DESTROY()
    ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDoubleClickSubtitle)
    ON_NOTIFY(LVN_KEYDOWN, IDC_LIST1, OnKeyPressedSubtitle)
END_MESSAGE_MAP()

void CSubtitleDlDlg::OnDoubleClickSubtitle(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pItemActivate = (LPNMITEMACTIVATE)(pNMHDR);

    if (pItemActivate->iItem >= 0) {
        DownloadSelectedSubtitles();
    }
}

void CSubtitleDlDlg::OnKeyPressedSubtitle(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

    if (pLVKeyDow->wVKey == VK_RETURN) {
        DownloadSelectedSubtitles();
        *pResult = TRUE;
    }
}
