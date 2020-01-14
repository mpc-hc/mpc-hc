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
#include <cmath>
#include <afxinet.h>
#include "mplayerc.h"
#include "MainFrm.h"
#include "DSUtil.h"
#include "SaveTextFileDialog.h"
#include "PlayerPlaylistBar.h"
#include "SettingsDefines.h"
#include "InternalFiltersConfig.h"
#include "PathUtils.h"
#include "WinAPIUtils.h"
#include "CMPCTheme.h"
#undef SubclassWindow



IMPLEMENT_DYNAMIC(CPlayerPlaylistBar, CMPCThemePlayerBar)
CPlayerPlaylistBar::CPlayerPlaylistBar(CMainFrame* pMainFrame)
    : m_pMainFrame(pMainFrame)
    , m_list(0)
    , m_nTimeColWidth(0)
    , m_pDragImage(nullptr)
    , m_bDragging(FALSE)
    , m_nDragIndex(0)
    , m_nDropIndex(0)
    , m_bHiddenDueToFullscreen(false)
    , m_pl(AfxGetAppSettings().bShufflePlaylistItems)
{
    GetEventd().Connect(m_eventc, {
        MpcEvent::DPI_CHANGED,
    }, std::bind(&CPlayerPlaylistBar::EventCallback, this, std::placeholders::_1));
}

CPlayerPlaylistBar::~CPlayerPlaylistBar()
{
}

BOOL CPlayerPlaylistBar::Create(CWnd* pParentWnd, UINT defDockBarID)
{
    if (!__super::Create(ResStr(IDS_PLAYLIST_CAPTION), pParentWnd, ID_VIEW_PLAYLIST, defDockBarID, _T("Playlist"))) {
        return FALSE;
    }

    m_list.CreateEx(
        WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP
        | LVS_OWNERDRAWFIXED
        | LVS_NOCOLUMNHEADER
        | LVS_EDITLABELS
        | LVS_REPORT | LVS_SINGLESEL | LVS_AUTOARRANGE | LVS_NOSORTHEADER, // TODO: remove LVS_SINGLESEL and implement multiple item repositioning (dragging is ready)
        CRect(0, 0, 100, 100), this, IDC_PLAYLIST);

    m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    // The column titles don't have to be translated since they aren't displayed anyway
    m_list.InsertColumn(COL_NAME, _T("Name"), LVCFMT_LEFT);

    m_list.InsertColumn(COL_TIME, _T("Time"), LVCFMT_RIGHT);

    ScaleFont();

    m_fakeImageList.Create(1, 16, ILC_COLOR4, 10, 10);
    m_list.SetImageList(&m_fakeImageList, LVSIL_SMALL);

    m_dropTarget.Register(this);

    return TRUE;
}

BOOL CPlayerPlaylistBar::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!__super::PreCreateWindow(cs)) {
        return FALSE;
    }

    cs.dwExStyle |= WS_EX_ACCEPTFILES;

    return TRUE;
}

BOOL CPlayerPlaylistBar::PreTranslateMessage(MSG* pMsg)
{
    if (IsWindow(pMsg->hwnd) && IsVisible() && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
        if (IsDialogMessage(pMsg)) {
            return TRUE;
        }
    }

    return __super::PreTranslateMessage(pMsg);
}

void CPlayerPlaylistBar::ReloadTranslatableResources()
{
    SetWindowText(ResStr(IDS_PLAYLIST_CAPTION));
}

void CPlayerPlaylistBar::LoadState(CFrameWnd* pParent)
{
    CString section = _T("ToolBars\\") + m_strSettingName;

    if (AfxGetApp()->GetProfileInt(section, _T("Visible"), FALSE)) {
        SetAutohidden(true);
    }

    __super::LoadState(pParent);
}

void CPlayerPlaylistBar::SaveState()
{
    __super::SaveState();

    CString section = _T("ToolBars\\") + m_strSettingName;

    AfxGetApp()->WriteProfileInt(section, _T("Visible"),
                                 IsWindowVisible() || (AfxGetAppSettings().bHidePlaylistFullScreen && m_bHiddenDueToFullscreen) || IsAutohidden());
}

bool CPlayerPlaylistBar::IsHiddenDueToFullscreen() const
{
    return m_bHiddenDueToFullscreen;
}

void CPlayerPlaylistBar::SetHiddenDueToFullscreen(bool bHiddenDueToFullscreen)
{
    if (bHiddenDueToFullscreen) {
        SetAutohidden(false);
    }
    m_bHiddenDueToFullscreen = bHiddenDueToFullscreen;
}

void CPlayerPlaylistBar::AddItem(CString fn, CAtlList<CString>* subs)
{
    CAtlList<CString> sl;
    sl.AddTail(fn);
    AddItem(sl, subs);
}

void CPlayerPlaylistBar::AddItem(CAtlList<CString>& fns, CAtlList<CString>* subs, CString label, CString ydl_src)
{
    CPlaylistItem pli;

    POSITION pos = fns.GetHeadPosition();
    while (pos) {
        CString fn = fns.GetNext(pos);
        if (!fn.Trim().IsEmpty()) {
            pli.m_fns.AddTail(fn);
        }
    }

    if (pli.m_fns.IsEmpty()) {
        return;
    }

    if (subs) {
        POSITION posSub = subs->GetHeadPosition();
        while (posSub) {
            CString fn = subs->GetNext(posSub);
            if (!fn.Trim().IsEmpty()) {
                pli.m_subs.AddTail(fn);
            }
        }
    }

    pli.AutoLoadFiles();
    if (!ydl_src.IsEmpty()) {
        pli.m_label = label;
        pli.m_ydlSourceURL = ydl_src;
        pli.m_bYoutubeDL = true;
    }

    m_pl.AddTail(pli);
}

void CPlayerPlaylistBar::ReplaceCurrentItem(CAtlList<CString>& fns, CAtlList<CString>* subs, CString label, CString ydl_src)
{
    CPlaylistItem* pli = GetCur();
    if (pli == nullptr) {
        AddItem(fns, subs, label, ydl_src);
    } else {
        pli->m_fns.RemoveAll();
        pli->m_fns.AddTailList(&fns);
        pli->m_subs.RemoveAll();
        if (subs) {
            pli->m_subs.AddTailList(subs);
        }
        pli->m_label = label;
        pli->m_ydlSourceURL = ydl_src;
        pli->m_bYoutubeDL = !ydl_src.IsEmpty();

        Refresh();
        SavePlaylist();
    }
}

static bool SearchFiles(CString mask, CAtlList<CString>& sl)
{
    if (mask.Find(_T("://")) >= 0) {
        return false;
    }

    mask.Trim();
    sl.RemoveAll();

    CMediaFormats& mf = AfxGetAppSettings().m_Formats;

    WIN32_FILE_ATTRIBUTE_DATA fad;
    bool fFilterKnownExts = (GetFileAttributesEx(mask, GetFileExInfoStandard, &fad)
                             && (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
    if (fFilterKnownExts) {
        mask = CString(mask).TrimRight(_T("\\/")) + _T("\\*.*");
    }

    {
        CString dir = mask.Left(std::max(mask.ReverseFind('\\'), mask.ReverseFind('/')) + 1);

        WIN32_FIND_DATA fd;
        HANDLE h = FindFirstFile(mask, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    continue;
                }

                CString fn = fd.cFileName;
                //CString ext = fn.Mid(fn.ReverseFind('.')+1).MakeLower();
                CString ext = fn.Mid(fn.ReverseFind('.')).MakeLower();
                CString path = dir + fd.cFileName;

                if (!fFilterKnownExts || mf.FindExt(ext)) {
                    for (size_t i = 0; i < mf.GetCount(); i++) {
                        CMediaFormatCategory& mfc = mf.GetAt(i);
                        /* playlist files are skipped when playing the contents of an entire directory */
                        if ((mfc.FindExt(ext)) && (mf[i].GetLabel().CompareNoCase(_T("pls")) != 0)) {
                            sl.AddTail(path);
                            break;
                        }
                    }
                }

            } while (FindNextFile(h, &fd));

            FindClose(h);

            if (sl.IsEmpty() && mask.Find(_T(":\\")) == 1) {
                if (OpticalDisk_VideoCD == GetOpticalDiskType(mask[0], sl)) {
                    sl.RemoveAll(); // need to open VideoCD as disk
                }
            }
        }
    }

    return (sl.GetCount() > 1
            || sl.GetCount() == 1 && sl.GetHead().CompareNoCase(mask)
            || sl.IsEmpty() && mask.FindOneOf(_T("?*")) >= 0);
}

void CPlayerPlaylistBar::ParsePlayList(CString fn, CAtlList<CString>* subs)
{
    CAtlList<CString> sl;
    sl.AddTail(fn);
    ParsePlayList(sl, subs);
}

void CPlayerPlaylistBar::ResolveLinkFiles(CAtlList<CString>& fns)
{
    // resolve .lnk files

    POSITION pos = fns.GetHeadPosition();
    while (pos) {
        CString& fn = fns.GetNext(pos);

        if (PathUtils::IsLinkFile(fn)) {
            CString fnResolved = PathUtils::ResolveLinkFile(fn);
            if (!fnResolved.IsEmpty()) {
                fn = fnResolved;
            }
        }
    }
}

void CPlayerPlaylistBar::ParsePlayList(CAtlList<CString>& fns, CAtlList<CString>* subs, CString label, CString ydl_src)
{
    if (fns.IsEmpty()) {
        return;
    }

    ResolveLinkFiles(fns);

    CAtlList<CString> sl;
    if (SearchFiles(fns.GetHead(), sl)) {
        if (sl.GetCount() > 1) {
            subs = nullptr;
        }
        POSITION pos = sl.GetHeadPosition();
        while (pos) {
            ParsePlayList(sl.GetNext(pos), subs);
        }
        return;
    }

    CAtlList<CString> redir;
    CStringA ct = GetContentType(fns.GetHead(), &redir);
    if (!redir.IsEmpty()) {
        POSITION pos = redir.GetHeadPosition();
        while (pos) {
            ParsePlayList(sl.GetNext(pos), subs);
        }
        return;
    }

    if (ct == "application/x-mpc-playlist") {
        ParseMPCPlayList(fns.GetHead());
        return;
    } else {
#if INTERNAL_SOURCEFILTER_MPEG
        const CAppSettings& s = AfxGetAppSettings();
        if (ct == "application/x-bdmv-playlist" && (s.SrcFilters[SRC_MPEG] || s.SrcFilters[SRC_MPEGTS])) {
            ParseBDMVPlayList(fns.GetHead());
            return;
        }
#endif
    }

    AddItem(fns, subs, label, ydl_src);
}

static CString CombinePath(CPath p, CString fn)
{
    if (fn.Find(':') >= 0 || fn.Find(_T("\\")) == 0) {
        return fn;
    }
    p.Append(CPath(fn));
    return (LPCTSTR)p;
}

bool CPlayerPlaylistBar::ParseBDMVPlayList(CString fn)
{
    CHdmvClipInfo ClipInfo;
    CString strPlaylistFile;
    CAtlList<CHdmvClipInfo::PlaylistItem>   MainPlaylist;

    CPath Path(fn);
    Path.RemoveFileSpec();
    Path.RemoveFileSpec();

    if (SUCCEEDED(ClipInfo.FindMainMovie(Path + L"\\", strPlaylistFile, MainPlaylist, ((CMainFrame*)GetParentFrame())->m_MPLSPlaylist))) {
        CAtlList<CString> strFiles;
        strFiles.AddHead(strPlaylistFile);
        Append(strFiles, MainPlaylist.GetCount() > 1, nullptr);
    }

    return !m_pl.IsEmpty();
}

bool CPlayerPlaylistBar::ParseMPCPlayList(CString fn)
{
    CString str;
    CAtlMap<int, CPlaylistItem> pli;
    std::vector<int> idx;

    CWebTextFile f(CTextFile::UTF8);
    if (!f.Open(fn) || !f.ReadString(str) || str != _T("MPCPLAYLIST")) {
        return false;
    }

    if (f.GetEncoding() == CTextFile::DEFAULT_ENCODING) {
        f.SetEncoding(CTextFile::ANSI);
    }

    CPath base(fn);
    base.RemoveFileSpec();

    while (f.ReadString(str)) {
        CAtlList<CString> sl;
        Explode(str, sl, ',', 3);
        if (sl.GetCount() != 3) {
            continue;
        }

        if (int i = _ttoi(sl.RemoveHead())) {
            CString key = sl.RemoveHead();
            CString value = sl.RemoveHead();

            if (key == _T("type")) {
                pli[i].m_type = (CPlaylistItem::type_t)_ttol(value);
                idx.push_back(i);
            } else if (key == _T("label")) {
                pli[i].m_label = value;
            } else if (key == _T("filename")) {
                value = CombinePath(base, value);
                pli[i].m_fns.AddTail(value);
            } else if (key == _T("subtitle")) {
                value = CombinePath(base, value);
                pli[i].m_subs.AddTail(value);
            } else if (key == _T("ydlSourceURL")) {
                pli[i].m_ydlSourceURL = value;
                pli[i].m_bYoutubeDL = true;
            } else if (key == _T("video")) {
                while (pli[i].m_fns.GetCount() < 2) {
                    pli[i].m_fns.AddTail(_T(""));
                }
                pli[i].m_fns.GetHead() = value;
            } else if (key == _T("audio")) {
                while (pli[i].m_fns.GetCount() < 2) {
                    pli[i].m_fns.AddTail(_T(""));
                }
                pli[i].m_fns.GetTail() = value;
            } else if (key == _T("vinput")) {
                pli[i].m_vinput = _ttol(value);
            } else if (key == _T("vchannel")) {
                pli[i].m_vchannel = _ttol(value);
            } else if (key == _T("ainput")) {
                pli[i].m_ainput = _ttol(value);
            } else if (key == _T("country")) {
                pli[i].m_country = _ttol(value);
            }
        }
    }

    std::sort(idx.begin(), idx.end());
    for (int i : idx) {
        m_pl.AddTail(pli[i]);
    }

    return !pli.IsEmpty();
}

bool CPlayerPlaylistBar::SaveMPCPlayList(CString fn, CTextFile::enc e, bool fRemovePath)
{
    CTextFile f;
    if (!f.Save(fn, e)) {
        return false;
    }

    f.WriteString(_T("MPCPLAYLIST\n"));

    POSITION pos = m_pl.GetHeadPosition(), pos2;
    for (int i = 1; pos; i++) {
        CPlaylistItem& pli = m_pl.GetNext(pos);

        CString idx;
        idx.Format(_T("%d"), i);

        CString str;
        str.Format(_T("%d,type,%d"), i, pli.m_type);
        f.WriteString(str + _T("\n"));

        if (!pli.m_label.IsEmpty()) {
            f.WriteString(idx + _T(",label,") + pli.m_label + _T("\n"));
        }

        if (pli.m_type == CPlaylistItem::file) {
            pos2 = pli.m_fns.GetHeadPosition();
            while (pos2) {
                CString fn2 = pli.m_fns.GetNext(pos2);
                if (fRemovePath) {
                    CPath p(fn2);
                    p.StripPath();
                    fn2 = (LPCTSTR)p;
                }
                f.WriteString(idx + _T(",filename,") + fn2 + _T("\n"));
            }

            pos2 = pli.m_subs.GetHeadPosition();
            while (pos2) {
                CString fn2 = pli.m_subs.GetNext(pos2);
                if (fRemovePath) {
                    CPath p(fn2);
                    p.StripPath();
                    fn2 = (LPCTSTR)p;
                }
                f.WriteString(idx + _T(",subtitle,") + fn2 + _T("\n"));
            }
            if (pli.m_bYoutubeDL && !pli.m_ydlSourceURL.IsEmpty())
            {
                f.WriteString(idx + _T(",ydlSourceURL,") + pli.m_ydlSourceURL + _T("\n"));
            }
        } else if (pli.m_type == CPlaylistItem::device && pli.m_fns.GetCount() == 2) {
            f.WriteString(idx + _T(",video,") + pli.m_fns.GetHead() + _T("\n"));
            f.WriteString(idx + _T(",audio,") + pli.m_fns.GetTail() + _T("\n"));
            str.Format(_T("%d,vinput,%d"), i, pli.m_vinput);
            f.WriteString(str + _T("\n"));
            str.Format(_T("%d,vchannel,%d"), i, pli.m_vchannel);
            f.WriteString(str + _T("\n"));
            str.Format(_T("%d,ainput,%d"), i, pli.m_ainput);
            f.WriteString(str + _T("\n"));
            str.Format(_T("%d,country,%ld"), i, pli.m_country);
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

bool CPlayerPlaylistBar::Empty()
{
    bool bWasPlaying = m_pl.RemoveAll();
    m_list.DeleteAllItems();
    SavePlaylist();

    return bWasPlaying;
}

void CPlayerPlaylistBar::Open(CAtlList<CString>& fns, bool fMulti, CAtlList<CString>* subs)
{
    ResolveLinkFiles(fns);
    Empty();
    Append(fns, fMulti, subs);
}

void CPlayerPlaylistBar::Append(CAtlList<CString>& fns, bool fMulti, CAtlList<CString>* subs, CString label, CString ydl_src)
{
    POSITION posFirstAdded = m_pl.GetTailPosition();
    int iFirstAdded = (int)m_pl.GetCount();

    if (fMulti) {
        ASSERT(subs == nullptr || subs->IsEmpty());
        POSITION pos = fns.GetHeadPosition();
        while (pos) {
            ParsePlayList(fns.GetNext(pos), nullptr);
        }
    } else {
        ParsePlayList(fns, subs, label, ydl_src);
    }

    Refresh();
    SavePlaylist();

    // Get the POSITION of the first item we just added
    if (posFirstAdded) {
        m_pl.GetNext(posFirstAdded);
    } else { // if the playlist was originally empty
        posFirstAdded = m_pl.GetHeadPosition();
    }
    if (posFirstAdded) {
        EnsureVisible(m_pl.GetTailPosition()); // This ensures that we maximize the number of newly added items shown
        EnsureVisible(posFirstAdded);
        if (iFirstAdded) { // Select the first added item only if some were already present
            m_list.SetItemState(iFirstAdded, LVIS_SELECTED, LVIS_SELECTED);
        }
        m_list.updateSB();
    }
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
    if (!vfn.IsEmpty()) {
        sl.AddTail(vfn);
    }
    if (!afn.IsEmpty()) {
        sl.AddTail(afn);
    }
    CStringW label = Implode(sl, '|');
    label.Replace(L"|", L" - ");
    pli.m_label = CString(label);
    m_pl.AddTail(pli);

    Refresh();
    EnsureVisible(m_pl.GetTailPosition());
    m_list.SetItemState((int)m_pl.GetCount() - 1, LVIS_SELECTED, LVIS_SELECTED);
    SavePlaylist();
}

void CPlayerPlaylistBar::SetupList()
{
    m_list.DeleteAllItems();

    POSITION pos = m_pl.GetHeadPosition();
    for (int i = 0; pos; i++) {
        CPlaylistItem& pli = m_pl.GetAt(pos);
        m_list.SetItemData(m_list.InsertItem(i, pli.GetLabel()), (DWORD_PTR)pos);
        m_list.SetItemText(i, COL_TIME, pli.GetLabel(1));
        m_pl.GetNext(pos);
    }
}

void CPlayerPlaylistBar::UpdateList()
{
    POSITION pos = m_pl.GetHeadPosition();
    for (int i = 0, j = m_list.GetItemCount(); pos && i < j; i++) {
        CPlaylistItem& pli = m_pl.GetAt(pos);
        m_list.SetItemData(i, (DWORD_PTR)pos);
        m_list.SetItemText(i, COL_NAME, pli.GetLabel(0));
        m_list.SetItemText(i, COL_TIME, pli.GetLabel(1));
        m_pl.GetNext(pos);
    }
}

void CPlayerPlaylistBar::EnsureVisible(POSITION pos)
{
    int i = FindItem(pos);
    if (i < 0) {
        return;
    }
    m_list.EnsureVisible(i, TRUE);
    m_list.Invalidate();
}

int CPlayerPlaylistBar::FindItem(const POSITION pos) const
{
    for (int i = 0; i < m_list.GetItemCount(); i++) {
        if ((POSITION)m_list.GetItemData(i) == pos) {
            return (i);
        }
    }
    return -1;
}

POSITION CPlayerPlaylistBar::FindPos(int i)
{
    if (i < 0) {
        return nullptr;
    }
    return (POSITION)m_list.GetItemData(i);
}

INT_PTR CPlayerPlaylistBar::GetCount() const
{
    return m_pl.GetCount(); // TODO: n - .fInvalid
}

int CPlayerPlaylistBar::GetSelIdx() const
{
    return FindItem(m_pl.GetPos());
}

void CPlayerPlaylistBar::SetSelIdx(int i)
{
    m_pl.SetPos(FindPos(i));
}

bool CPlayerPlaylistBar::IsAtEnd()
{
    POSITION pos = m_pl.GetPos(), tail = m_pl.GetShuffleAwareTailPosition();
    bool isAtEnd = (pos && pos == tail);

    if (!isAtEnd && pos) {
        isAtEnd = m_pl.GetNextWrap(pos).m_fInvalid;
        while (isAtEnd && pos && pos != tail) {
            isAtEnd = m_pl.GetNextWrap(pos).m_fInvalid;
        }
    }

    return isAtEnd;
}

bool CPlayerPlaylistBar::GetCur(CPlaylistItem& pli) const
{
    if (!m_pl.GetPos()) {
        return false;
    }
    pli = m_pl.GetAt(m_pl.GetPos());
    return true;
}

CPlaylistItem* CPlayerPlaylistBar::GetCur()
{
    if (!m_pl.GetPos()) {
        return nullptr;
    }
    return &m_pl.GetAt(m_pl.GetPos());
}

CString CPlayerPlaylistBar::GetCurFileName()
{
    CString fn;
    CPlaylistItem* pli = GetCur();

    if (pli && pli->m_bYoutubeDL) {
        fn = pli->m_label;
    } else if (pli && !pli->m_fns.IsEmpty()) {
        fn = pli->m_fns.GetHead();
    }
    return fn;
}

bool CPlayerPlaylistBar::SetNext()
{
    POSITION pos = m_pl.GetPos(), org = pos;
    while (m_pl.GetNextWrap(pos).m_fInvalid && pos != org) {
        ;
    }
    UpdateList();
    m_pl.SetPos(pos);
    EnsureVisible(pos);

    return (pos != org);
}

bool CPlayerPlaylistBar::SetPrev()
{
    POSITION pos = m_pl.GetPos(), org = pos;
    while (m_pl.GetPrevWrap(pos).m_fInvalid && pos != org) {
        ;
    }
    m_pl.SetPos(pos);
    EnsureVisible(pos);

    return (pos != org);
}

void CPlayerPlaylistBar::SetFirstSelected()
{
    POSITION pos = m_list.GetFirstSelectedItemPosition();
    if (pos) {
        pos = FindPos(m_list.GetNextSelectedItem(pos));
    } else {
        pos = m_pl.GetShuffleAwareTailPosition();
        POSITION org = pos;
        while (m_pl.GetNextWrap(pos).m_fInvalid && pos != org) {
            ;
        }
        // Select the first item to be played when no item was previously selected
        m_list.SetItemState(FindItem(pos), LVIS_SELECTED, LVIS_SELECTED);
    }
    UpdateList();
    m_pl.SetPos(pos);
    EnsureVisible(pos);
}

void CPlayerPlaylistBar::SetFirst()
{
    POSITION pos = m_pl.GetTailPosition(), org = pos;
    while (m_pl.GetNextWrap(pos).m_fInvalid && pos != org) {
        ;
    }
    UpdateList();
    m_pl.SetPos(pos);
    EnsureVisible(pos);
}

void CPlayerPlaylistBar::SetLast()
{
    POSITION pos = m_pl.GetHeadPosition(), org = pos;
    while (m_pl.GetPrevWrap(pos).m_fInvalid && pos != org) {
        ;
    }
    m_pl.SetPos(pos);
    EnsureVisible(pos);
}

void CPlayerPlaylistBar::SetCurValid(bool fValid)
{
    POSITION pos = m_pl.GetPos();
    if (pos) {
        m_pl.GetAt(pos).m_fInvalid = !fValid;
        if (!fValid) {
            int i = FindItem(pos);
            m_list.RedrawItems(i, i);
        }
    }
}

void CPlayerPlaylistBar::SetCurTime(REFERENCE_TIME rt)
{
    POSITION pos = m_pl.GetPos();
    if (pos) {
        CPlaylistItem& pli = m_pl.GetAt(pos);
        pli.m_duration = rt;
        m_list.SetItemText(FindItem(pos), COL_TIME, pli.GetLabel(1));
    }
}

void CPlayerPlaylistBar::Randomize()
{
    m_pl.Randomize();
    SetupList();
    SavePlaylist();
}

OpenMediaData* CPlayerPlaylistBar::GetCurOMD(REFERENCE_TIME rtStart)
{
    CPlaylistItem* pli = GetCur();
    if (pli == nullptr) {
        return nullptr;
    }

    pli->AutoLoadFiles();

    CString fn = CString(pli->m_fns.GetHead()).MakeLower();

    if (fn.Find(_T("video_ts.ifo")) >= 0) {
        if (OpenDVDData* p = DEBUG_NEW OpenDVDData()) {
            p->path = pli->m_fns.GetHead();
            p->subs.AddTailList(&pli->m_subs);
            return p;
        }
    }

    if (pli->m_type == CPlaylistItem::device) {
        if (OpenDeviceData* p = DEBUG_NEW OpenDeviceData()) {
            POSITION pos = pli->m_fns.GetHeadPosition();
            for (int i = 0; i < _countof(p->DisplayName) && pos; i++) {
                p->DisplayName[i] = pli->m_fns.GetNext(pos);
            }
            p->vinput = pli->m_vinput;
            p->vchannel = pli->m_vchannel;
            p->ainput = pli->m_ainput;
            return p;
        }
    } else {
        if (OpenFileData* p = DEBUG_NEW OpenFileData()) {
            p->fns.AddTailList(&pli->m_fns);
            p->subs.AddTailList(&pli->m_subs);
            p->rtStart = rtStart;
            p->bAddToRecent = !pli->m_bYoutubeDL;
            return p;
        }
    }

    return nullptr;
}

bool CPlayerPlaylistBar::SelectFileInPlaylist(LPCTSTR filename)
{
    if (!filename) {
        return false;
    }
    POSITION pos = m_pl.GetHeadPosition();
    while (pos) {
        CPlaylistItem& pli = m_pl.GetAt(pos);
        if (pli.FindFile(filename)) {
            m_pl.SetPos(pos);
            EnsureVisible(pos);
            return true;
        }
        m_pl.GetNext(pos);
    }
    return false;
}

bool CPlayerPlaylistBar::DeleteFileInPlaylist(POSITION pos, bool recycle)
{
    // release the file handle by changing to the next item or stopping playback
    if (pos == m_pl.GetPos()) {
        if (m_pl.GetCount() > 1) {
            m_pMainFrame->OnNavigateSkipFile(ID_NAVIGATE_SKIPFORWARDFILE);
        } else {
            m_pMainFrame->SendMessage(WM_COMMAND, ID_FILE_CLOSEMEDIA);
        }
    }

    if (SUCCEEDED(FileDelete(m_pl.GetAt(pos).m_fns.GetHead(), m_pMainFrame->m_hWnd, recycle))) {
        m_list.DeleteItem(FindItem(pos));
        m_pl.RemoveAt(pos);
        SavePlaylist();
        return true;
    }
    return false;
}

void CPlayerPlaylistBar::LoadPlaylist(LPCTSTR filename)
{
    CString base;

    m_list.SetRedraw(FALSE);

    if (AfxGetMyApp()->GetAppSavePath(base)) {
        CPath p;
        p.Combine(base, _T("default.mpcpl"));

        if (p.FileExists()) {
            if (AfxGetAppSettings().bRememberPlaylistItems) {
                ParseMPCPlayList(p);
                Refresh();
                SelectFileInPlaylist(filename);
            } else {
                ::DeleteFile(p);
            }
        }
    }
    m_list.SetRedraw(TRUE);
    m_list.RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
}

void CPlayerPlaylistBar::SavePlaylist()
{
    CString base;

    if (AfxGetMyApp()->GetAppSavePath(base)) {
        CPath p;
        p.Combine(base, _T("default.mpcpl"));

        if (AfxGetAppSettings().bRememberPlaylistItems) {
            // Only create this folder when needed
            if (!PathUtils::Exists(base)) {
                ::CreateDirectory(base, nullptr);
            }

            SaveMPCPlayList(p, CTextFile::UTF8, false);
        } else if (p.FileExists()) {
            ::DeleteFile(p);
        }
    }
}

BEGIN_MESSAGE_MAP(CPlayerPlaylistBar, CMPCThemePlayerBar)
    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_NOTIFY(LVN_KEYDOWN, IDC_PLAYLIST, OnLvnKeyDown)
    ON_NOTIFY(NM_DBLCLK, IDC_PLAYLIST, OnNMDblclkList)
    //ON_NOTIFY(NM_CUSTOMDRAW, IDC_PLAYLIST, OnCustomdrawList)
    ON_WM_MEASUREITEM()
    ON_WM_DRAWITEM()
    ON_COMMAND_EX(ID_PLAY_PLAY, OnPlayPlay)
    ON_NOTIFY(LVN_BEGINDRAG, IDC_PLAYLIST, OnBeginDrag)
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
    ON_WM_TIMER()
    ON_WM_CONTEXTMENU()
    ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_PLAYLIST, OnLvnBeginlabeleditList)
    ON_NOTIFY(LVN_ENDLABELEDIT, IDC_PLAYLIST, OnLvnEndlabeleditList)
    ON_WM_XBUTTONDOWN()
    ON_WM_XBUTTONUP()
    ON_WM_XBUTTONDBLCLK()
    ON_WM_ERASEBKGND()
    ON_WM_VSCROLL()
END_MESSAGE_MAP()


void CPlayerPlaylistBar::ScaleFont()
{
    LOGFONT lf;
    GetMessageFont(&lf);
    lf.lfHeight = m_pMainFrame->m_dpi.ScaleSystemToOverrideY(lf.lfHeight);

    m_font.DeleteObject();
    if (m_font.CreateFontIndirect(&lf)) {
        m_list.SetFont(&m_font);
    }

    CDC* pDC = m_list.GetDC();
    CFont* old = pDC->SelectObject(m_list.GetFont());
    m_nTimeColWidth = pDC->GetTextExtent(_T("000:00:00")).cx + m_pMainFrame->m_dpi.ScaleX(5);
    pDC->SelectObject(old);
    m_list.ReleaseDC(pDC);
    m_list.SetColumnWidth(COL_TIME, m_nTimeColWidth);
}

void CPlayerPlaylistBar::EventCallback(MpcEvent ev)
{
    switch (ev) {
        case MpcEvent::DPI_CHANGED:
            ScaleFont();
            ResizeListColumn();
            break;

        default:
            ASSERT(FALSE);
    }
}

// CPlayerPlaylistBar message handlers

void CPlayerPlaylistBar::ResizeListColumn()
{
    if (::IsWindow(m_list.m_hWnd)) {
        CRect r;
        GetClientRect(r);
        r.DeflateRect(2, 2);

        m_list.SetRedraw(FALSE);
        m_list.SetColumnWidth(COL_NAME, 0);
        m_list.SetRedraw(TRUE);

        m_list.MoveWindow(r, FALSE);
        m_list.GetClientRect(r);

        m_list.SetRedraw(FALSE);
        m_list.SetColumnWidth(COL_NAME, r.Width() - m_nTimeColWidth);
        m_list.SetRedraw(TRUE);

        Invalidate();
        m_list.updateSB();
        m_list.RedrawWindow(nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE);
    }
}

void CPlayerPlaylistBar::OnDestroy()
{
    m_dropTarget.Revoke();
    __super::OnDestroy();
}

void CPlayerPlaylistBar::OnSize(UINT nType, int cx, int cy)
{
    __super::OnSize(nType, cx, cy);

    ResizeListColumn();
}

void CPlayerPlaylistBar::OnLvnKeyDown(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

    *pResult = FALSE;

    CList<int> items;
    POSITION pos = m_list.GetFirstSelectedItemPosition();
    while (pos) {
        items.AddHead(m_list.GetNextSelectedItem(pos));
    }

    if (pLVKeyDown->wVKey == VK_DELETE && !items.IsEmpty()) {
        pos = items.GetHeadPosition();
        while (pos) {
            int i = items.GetNext(pos);
            if (m_pl.RemoveAt(FindPos(i))) {
                m_pMainFrame->SendMessage(WM_COMMAND, ID_FILE_CLOSEMEDIA);
            }
            m_list.DeleteItem(i);
        }

        m_list.SetItemState(-1, 0, LVIS_SELECTED);
        m_list.SetItemState(
            std::max(std::min(items.GetTail(), m_list.GetItemCount() - 1), 0),
            LVIS_SELECTED, LVIS_SELECTED);

        ResizeListColumn();

        *pResult = TRUE;
    } else if (pLVKeyDown->wVKey == VK_SPACE && items.GetCount() == 1) {
        m_pl.SetPos(FindPos(items.GetHead()));
        m_list.Invalidate();
        m_pMainFrame->OpenCurPlaylistItem();
        m_pMainFrame->SetFocus();

        *pResult = TRUE;
    }
}

void CPlayerPlaylistBar::OnNMDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

    if (lpnmlv->iItem >= 0 && lpnmlv->iSubItem >= 0) {
        POSITION pos = FindPos(lpnmlv->iItem);
        // If the file is already playing, don't try to restore a previously saved position
        if (m_pMainFrame->GetPlaybackMode() == PM_FILE && pos == m_pl.GetPos()) {
            const CPlaylistItem& pli = m_pl.GetAt(pos);

            CAppSettings& s = AfxGetAppSettings();
            s.filePositions.RemoveEntry(pli.m_fns.GetHead());
        } else {
            m_pl.SetPos(pos);
        }
        m_list.Invalidate();
        m_pMainFrame->OpenCurPlaylistItem();
    }

    m_pMainFrame->SetFocus();

    *pResult = 0;
}

void CPlayerPlaylistBar::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    __super::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
    lpMeasureItemStruct->itemHeight = m_pMainFrame->m_dpi.ScaleSystemToOverrideY(lpMeasureItemStruct->itemHeight);
}

/*
void CPlayerPlaylistBar::OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

    *pResult = CDRF_DODEFAULT;

    if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
    {
        *pResult = CDRF_NOTIFYPOSTPAINT|CDRF_NOTIFYITEMDRAW;
    }
    else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
    {
        pLVCD->nmcd.uItemState &= ~CDIS_SELECTED;
        pLVCD->nmcd.uItemState &= ~CDIS_FOCUS;

        pLVCD->clrText = (pLVCD->nmcd.dwItemSpec == m_playList.m_idx) ? 0x0000ff : CLR_DEFAULT;
        pLVCD->clrTextBk = m_list.GetItemState(pLVCD->nmcd.dwItemSpec, LVIS_SELECTED) ? 0xf1dacc : CLR_DEFAULT;

        *pResult = CDRF_NOTIFYPOSTPAINT;
    }
    else if (CDDS_ITEMPOSTPAINT == pLVCD->nmcd.dwDrawStage)
    {
        int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);

        if (m_list.GetItemState(pLVCD->nmcd.dwItemSpec, LVIS_SELECTED))
        {
            CRect r, r2;
            m_list.GetItemRect(nItem, &r, LVIR_BOUNDS);
            m_list.GetItemRect(nItem, &r2, LVIR_LABEL);
            r.left = r2.left;
            FrameRect(pLVCD->nmcd.hdc, &r, CBrush(0xc56a31));
        }

        *pResult = CDRF_SKIPDEFAULT;
    }
    else if (CDDS_POSTPAINT == pLVCD->nmcd.dwDrawStage)
    {
    }
}
*/

void CPlayerPlaylistBar::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    if (nIDCtl != IDC_PLAYLIST) {
        return;
    }


    int nItem = lpDrawItemStruct->itemID;
    CRect rcItem = lpDrawItemStruct->rcItem;
    POSITION pos = FindPos(nItem);
    bool itemPlaying = pos == m_pl.GetPos();
    bool itemSelected = !!m_list.GetItemState(nItem, LVIS_SELECTED);
    CPlaylistItem& pli = m_pl.GetAt(pos);

    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
    int oldDC = pDC->SaveDC();
    const CAppSettings& s = AfxGetAppSettings();

    COLORREF bgColor;
    
    if (itemSelected) {
        if (s.bMPCThemeLoaded) {
            bgColor = CMPCTheme::ContentSelectedColor;
        } else {
            bgColor = 0xf1dacc;
            FrameRect(pDC->m_hDC, rcItem, CBrush(0xc56a31));
        }
    } else {
        if (s.bMPCThemeLoaded) {
            bgColor = CMPCTheme::ContentBGColor;
        } else {
            bgColor = GetSysColor(COLOR_WINDOW);
        }
    }
    FillRect(pDC->m_hDC, rcItem, CBrush(bgColor));
    pDC->SetBkColor(bgColor);

    COLORREF textcolor;
    CString bullet = _T("\x25CF ");
    CSize bulletWidth = pDC->GetTextExtent(bullet);

    if (s.bMPCThemeLoaded) {
        if (pli.m_fInvalid) {
            textcolor = CMPCTheme::ContentTextDisabledFGColorFade2;
        } else if (itemPlaying) {
            textcolor = itemSelected ? CMPCTheme::ActivePlayListItemHLColor : CMPCTheme::ActivePlayListItemColor;
        } else {
            textcolor = CMPCTheme::TextFGColor;
        }
    } else {
        textcolor = itemPlaying ? 0xff : 0;
        if (pli.m_fInvalid) {
            textcolor |= 0xA0A0A0;
        }
    }

    CString time = !pli.m_fInvalid ? m_list.GetItemText(nItem, COL_TIME) : CString(_T("Invalid"));
    CPoint timept(rcItem.right, 0);
    if (!time.IsEmpty()) {
        CSize timesize = pDC->GetTextExtent(time);
        if ((3 + timesize.cx + 3) < rcItem.Width() / 2) {
            timept = CPoint(rcItem.right - (3 + timesize.cx + 3), (rcItem.top + rcItem.bottom - timesize.cy) / 2);

            pDC->SetTextColor(textcolor);
            pDC->TextOut(timept.x, timept.y, time);
        }
    }

    CString fmt, file;
    fmt.Format(_T("%%0%dd. %%s"), (int)log10(0.1 + m_pl.GetCount()) + 1);
    file.Format(fmt, nItem + 1, m_list.GetItemText(nItem, COL_NAME).GetString());
    CSize filesize = pDC->GetTextExtent(file);
    while (3 + bulletWidth.cx + filesize.cx + 6 > timept.x && file.GetLength() > 3) {
        file = file.Left(file.GetLength() - 4) + _T("...");
        filesize = pDC->GetTextExtent(file);
    }

    if (file.GetLength() > 3) {
        pDC->SetTextColor(textcolor);
        pDC->TextOut(rcItem.left + 3 + bulletWidth.cx, (rcItem.top + rcItem.bottom - filesize.cy) / 2, file);
        if (itemPlaying) {
            pDC->TextOut(rcItem.left + 3, (rcItem.top + rcItem.bottom - filesize.cy) / 2, bullet);
        }
    }
    pDC->RestoreDC(oldDC);
}

BOOL CPlayerPlaylistBar::OnPlayPlay(UINT nID)
{
    m_list.Invalidate();
    return FALSE;
}

DROPEFFECT CPlayerPlaylistBar::OnDropAccept(COleDataObject*, DWORD, CPoint)
{
    return DROPEFFECT_COPY;
}

void CPlayerPlaylistBar::OnDropFiles(CAtlList<CString>& slFiles, DROPEFFECT)
{
    SetForegroundWindow();
    m_list.SetFocus();

    PathUtils::ParseDirs(slFiles);

    Append(slFiles, true);
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
    if (m_bDragging) {
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

            if (iOverItem == iTopItem && iTopItem != 0) { // top of list
                SetTimer(1, 100, nullptr);
            } else {
                KillTimer(1);
            }

            if (iOverItem >= iBottomItem && iBottomItem != (m_list.GetItemCount() - 1)) { // bottom of list
                SetTimer(2, 100, nullptr);
            } else {
                KillTimer(2);
            }
        }
    }

    __super::OnMouseMove(nFlags, point);
}

void CPlayerPlaylistBar::OnTimer(UINT_PTR nIDEvent)
{
    int iTopItem = m_list.GetTopIndex();
    int iBottomItem = iTopItem + m_list.GetCountPerPage() - 1;

    if (m_bDragging) {
        m_pDragImage->DragShowNolock(FALSE);

        if (nIDEvent == 1) {
            m_list.EnsureVisible(iTopItem - 1, false);
            m_list.UpdateWindow();
            if (m_list.GetTopIndex() == 0) {
                KillTimer(1);
            }
        } else if (nIDEvent == 2) {
            m_list.EnsureVisible(iBottomItem + 1, false);
            m_list.UpdateWindow();
            if (m_list.GetBottomIndex() == (m_list.GetItemCount() - 1)) {
                KillTimer(2);
            }
        }

        m_pDragImage->DragShowNolock(TRUE);
    }

    __super::OnTimer(nIDEvent);
}

void CPlayerPlaylistBar::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_bDragging) {
        ::ReleaseCapture();

        m_bDragging = FALSE;
        m_pDragImage->DragLeave(GetDesktopWindow());
        m_pDragImage->EndDrag();

        delete m_pDragImage;
        m_pDragImage = nullptr;

        KillTimer(1);
        KillTimer(2);

        CPoint pt(point);
        ClientToScreen(&pt);

        if (WindowFromPoint(pt) == &m_list) {
            DropItemOnList();
        }
    }

    ModifyStyle(0, WS_EX_ACCEPTFILES);

    __super::OnLButtonUp(nFlags, point);
}

void CPlayerPlaylistBar::DropItemOnList()
{
    m_ptDropPoint.y += 10;
    m_nDropIndex = m_list.HitTest(CPoint(10, m_ptDropPoint.y));

    TCHAR szLabel[MAX_PATH];
    LV_ITEM lvi;
    ZeroMemory(&lvi, sizeof(LV_ITEM));
    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
    lvi.stateMask = LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;
    lvi.pszText = szLabel;
    lvi.iItem = m_nDragIndex;
    lvi.cchTextMax = MAX_PATH;
    m_list.GetItem(&lvi);

    if (m_nDropIndex < 0) {
        m_nDropIndex = m_list.GetItemCount();
    }
    lvi.iItem = m_nDropIndex;
    m_list.InsertItem(&lvi);

    CHeaderCtrl* pHeader = (CHeaderCtrl*)m_list.GetDlgItem(0);
    int nColumnCount = pHeader->GetItemCount();
    lvi.mask = LVIF_TEXT;
    lvi.iItem = m_nDropIndex;
    //INDEX OF DRAGGED ITEM WILL CHANGE IF ITEM IS DROPPED ABOVE ITSELF
    if (m_nDropIndex < m_nDragIndex) {
        m_nDragIndex++;
    }
    for (int col = 1; col < nColumnCount; col++) {
        _tcscpy_s(lvi.pszText, MAX_PATH, (LPCTSTR)(m_list.GetItemText(m_nDragIndex, col)));
        lvi.iSubItem = col;
        m_list.SetItem(&lvi);
    }

    m_list.DeleteItem(m_nDragIndex);

    for (int i = 0; i < m_list.GetItemCount(); i++) {
        POSITION pos = (POSITION)m_list.GetItemData(i);
        m_pl.MoveToTail(pos);
    }

    ResizeListColumn();
}

BOOL CPlayerPlaylistBar::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;

    if ((HWND)pTTT->lParam != m_list.m_hWnd) {
        return FALSE;
    }

    int row = ((pNMHDR->idFrom - 1) >> 10) & 0x3fffff;
    int col = (pNMHDR->idFrom - 1) & 0x3ff;

    if (row < 0 || size_t(row) >= m_pl.GetCount()) {
        return FALSE;
    }

    CPlaylistItem& pli = m_pl.GetAt(FindPos(row));

    static CString strTipText; // static string
    strTipText.Empty();

    if (col == COL_NAME) {
        POSITION pos = pli.m_fns.GetHeadPosition();
        while (pos) {
            strTipText += _T("\n") + pli.m_fns.GetNext(pos);
        }
        strTipText.Trim();

        if (pli.m_type == CPlaylistItem::device) {
            if (pli.m_vinput >= 0) {
                strTipText.AppendFormat(_T("\nVideo Input %d"), pli.m_vinput);
            }
            if (pli.m_vchannel >= 0) {
                strTipText.AppendFormat(_T("\nVideo Channel %d"), pli.m_vchannel);
            }
            if (pli.m_ainput >= 0) {
                strTipText.AppendFormat(_T("\nAudio Input %d"), pli.m_ainput);
            }
        }

        ::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 1000);
    } else if (col == COL_TIME) {
        return FALSE;
    }

    pTTT->lpszText = (LPWSTR)(LPCWSTR)strTipText;

    *pResult = 0;

    return TRUE;    // message was handled
}

void CPlayerPlaylistBar::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    LVHITTESTINFO lvhti;

    bool bOnItem;
    if (point.x == -1 && point.y == -1) {
        lvhti.iItem = m_list.GetSelectionMark();

        if (lvhti.iItem == -1 && m_pl.GetCount() == 1) {
            lvhti.iItem = 0;
        }

        CRect r;
        if (!!m_list.GetItemRect(lvhti.iItem, r, LVIR_BOUNDS)) {
            point.SetPoint(r.left, r.bottom);
        } else {
            point.SetPoint(0, 0);
        }
        m_list.ClientToScreen(&point);
        bOnItem = lvhti.iItem != -1;
    } else {
        lvhti.pt = point;
        m_list.ScreenToClient(&lvhti.pt);
        m_list.SubItemHitTest(&lvhti);
        bOnItem = lvhti.iItem >= 0 && !!(lvhti.flags & LVHT_ONITEM);
        if (!bOnItem && m_pl.GetCount() == 1) {
            bOnItem = true;
            lvhti.iItem = 0;
        }
    }

    POSITION pos = FindPos(lvhti.iItem);
    bool bIsLocalFile = bOnItem ? PathUtils::Exists(m_pl.GetAt(pos).m_fns.GetHead()) : false;

    CMPCThemeMenu m;
    m.CreatePopupMenu();

    enum {
        M_OPEN = 1,
        M_ADD,
        M_REMOVE,
        M_CLEAR,
        M_CLIPBOARD,
        M_SHOWFOLDER,
        M_ADDFOLDER,
        M_RECYCLE,
        M_SAVEAS,
        M_SORTBYNAME,
        M_SORTBYPATH,
        M_RANDOMIZE,
        M_SORTBYID,
        M_SHUFFLE,
        M_HIDEFULLSCREEN
    };

    CAppSettings& s = AfxGetAppSettings();

    m.AppendMenu(MF_STRING | (!bOnItem ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_OPEN, ResStr(IDS_PLAYLIST_OPEN));
    if (m_pMainFrame->GetPlaybackMode() == PM_ANALOG_CAPTURE) {
        m.AppendMenu(MF_STRING | MF_ENABLED, M_ADD, ResStr(IDS_PLAYLIST_ADD));
    }
    m.AppendMenu(MF_STRING | (!bOnItem ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_REMOVE, ResStr(IDS_PLAYLIST_REMOVE));
    m.AppendMenu(MF_SEPARATOR);
    m.AppendMenu(MF_STRING | (!m_pl.GetCount() ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_CLEAR, ResStr(IDS_PLAYLIST_CLEAR));
    m.AppendMenu(MF_SEPARATOR);
    m.AppendMenu(MF_STRING | (!bOnItem ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_CLIPBOARD, ResStr(IDS_PLAYLIST_COPYTOCLIPBOARD));
    m.AppendMenu(MF_STRING | ((!bOnItem || !bIsLocalFile) ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_SHOWFOLDER, ResStr(IDS_PLAYLIST_SHOWFOLDER));
    m.AppendMenu(MF_STRING | ((!bOnItem || !bIsLocalFile) ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_RECYCLE, ResStr(IDS_FILE_RECYCLE));
    m.AppendMenu(MF_STRING | ((!bOnItem || !bIsLocalFile) ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_ADDFOLDER, ResStr(IDS_PLAYLIST_ADDFOLDER));
    m.AppendMenu(MF_SEPARATOR);
    m.AppendMenu(MF_STRING | (!m_pl.GetCount() ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_SAVEAS, ResStr(IDS_PLAYLIST_SAVEAS));
    m.AppendMenu(MF_SEPARATOR);
    m.AppendMenu(MF_STRING | (!m_pl.GetCount() ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_SORTBYNAME, ResStr(IDS_PLAYLIST_SORTBYLABEL));
    m.AppendMenu(MF_STRING | (!m_pl.GetCount() ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_SORTBYPATH, ResStr(IDS_PLAYLIST_SORTBYPATH));
    m.AppendMenu(MF_STRING | (!m_pl.GetCount() ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_RANDOMIZE, ResStr(IDS_PLAYLIST_RANDOMIZE));
    m.AppendMenu(MF_STRING | (!m_pl.GetCount() ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_SORTBYID, ResStr(IDS_PLAYLIST_RESTORE));
    m.AppendMenu(MF_SEPARATOR);
    m.AppendMenu(MF_STRING | MF_ENABLED | (s.bShufflePlaylistItems ? MF_CHECKED : MF_UNCHECKED), M_SHUFFLE, ResStr(IDS_PLAYLIST_SHUFFLE));
    m.AppendMenu(MF_SEPARATOR);
    m.AppendMenu(MF_STRING | MF_ENABLED | (s.bHidePlaylistFullScreen ? MF_CHECKED : MF_UNCHECKED), M_HIDEFULLSCREEN, ResStr(IDS_PLAYLIST_HIDEFS));
    if (s.bMPCThemeLoaded) {
        m.fulfillThemeReqs();
    }

    //use mainframe as parent to take advantage of measure redirect (was 'this' but text was not printed)
    int nID = (int)m.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RETURNCMD, point.x, point.y, m_pMainFrame); 
    switch (nID) {
        case M_OPEN:
            m_pl.SetPos(pos);
            m_list.Invalidate();
            m_pMainFrame->OpenCurPlaylistItem();
            break;
        case M_ADD:
            m_pMainFrame->AddCurDevToPlaylist();
            m_pl.SetPos(m_pl.GetTailPosition());
            break;
        case M_REMOVE:
            if (m_pl.RemoveAt(pos)) {
                m_pMainFrame->SendMessage(WM_COMMAND, ID_FILE_CLOSEMEDIA);
            }
            m_list.DeleteItem(lvhti.iItem);
            SavePlaylist();
            break;
        case M_RECYCLE:
            DeleteFileInPlaylist(pos);
            break;
        case M_CLEAR:
            if (Empty()) {
                m_pMainFrame->SendMessage(WM_COMMAND, ID_FILE_CLOSEMEDIA);
            }
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
            Randomize();
            break;
        case M_CLIPBOARD:
            if (OpenClipboard() && EmptyClipboard()) {
                CString str;

                CPlaylistItem& pli = m_pl.GetAt(pos);
                POSITION pos2 = pli.m_fns.GetHeadPosition();
                while (pos2) {
                    str += _T("\r\n") + pli.m_fns.GetNext(pos2);
                }
                str.Trim();

                if (HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, (str.GetLength() + 1) * sizeof(TCHAR))) {
                    if (TCHAR* cp = (TCHAR*)GlobalLock(h)) {
                        _tcscpy_s(cp, str.GetLength() + 1, str);
                        GlobalUnlock(h);
                        SetClipboardData(CF_UNICODETEXT, h);
                    }
                }
                CloseClipboard();
            }
            break;
        case M_SHOWFOLDER:
            ExploreToFile(m_pl.GetAt(pos).m_fns.GetHead());
            break;
        case M_ADDFOLDER: {
            // add all media files in current playlist item folder that are not yet in the playlist
            const CString currentFileName = m_pl.GetAt(pos).m_fns.GetHead();
            const CString dirName = PathUtils::DirName(currentFileName);
            if (PathUtils::IsDir(dirName)) {
                CAtlList<CString> fileListAtl;
                if (SearchFiles(dirName, fileListAtl)) {
                    std::set<CString, CStringUtils::LogicalLess> fileList;
                    {
                        // convert to stl
                        POSITION pos2 = fileListAtl.GetHeadPosition();
                        while (pos2) {
                            fileList.emplace_hint(fileList.end(), fileListAtl.GetNext(pos2));
                        }

                        // deduplicate
                        pos2 = m_pl.GetHeadPosition();
                        while (pos2) {
                            const CPlaylistItem& pli = m_pl.GetNext(pos2);
                            POSITION subpos = pli.m_fns.GetHeadPosition();
                            while (subpos) {
                                fileList.erase(pli.m_fns.GetNext(subpos));
                            }
                        }
                    }

                    CStringUtils::LogicalLess less;
                    for (auto rit = fileList.crbegin(); rit != fileList.crend(); ++rit) {
                        // determine insert position
                        bool bLower = false;
                        while (pos) {
                            const CString& fileName = m_pl.GetAt(pos).m_fns.GetHead();
                            if (!less(*rit, fileName) || PathUtils::DirName(fileName).CompareNoCase(PathUtils::DirName(*rit))) {
                                break;
                            }
                            bLower = true;
                            m_pl.GetPrev(pos);
                        }
                        if (!bLower) {
                            m_pl.GetNext(pos);
                            while (pos) {
                                const CString& fileName = m_pl.GetAt(pos).m_fns.GetHead();
                                if (!less(fileName, *rit) || PathUtils::DirName(fileName).CompareNoCase(PathUtils::DirName(*rit))) {
                                    break;
                                }
                                m_pl.GetNext(pos);
                            }
                        }

                        // insert new item
                        CPlaylistItem pli;
                        pli.m_fns.AddTail(*rit);
                        if (bLower) {
                            pos = pos ? m_pl.InsertAfter(pos, pli) : m_pl.AddHead(pli);
                        } else {
                            pos = pos ? m_pl.InsertBefore(pos, pli) : m_pl.AddTail(pli);
                        }
                    }

                    // rebuild list and restore selection
                    if (!fileList.empty()) {
                        int insertedBefore = 0;
                        for (const auto& fileName : fileList) {
                            if (less(fileName, currentFileName)) {
                                insertedBefore++;
                            } else {
                                break;
                            }
                        }
                        Refresh();
                        m_list.SetItemState(lvhti.iItem + insertedBefore, LVIS_SELECTED, LVIS_SELECTED);
                        m_list.SetSelectionMark(lvhti.iItem + insertedBefore);
                        m_list.EnsureVisible(lvhti.iItem + insertedBefore, TRUE);
                        SavePlaylist();
                    }
                }
            }
            break;
        }
        case M_SAVEAS: {
            CSaveTextFileDialog fd(
                CTextFile::DEFAULT_ENCODING, nullptr, nullptr,
                _T("MPC-HC playlist (*.mpcpl)|*.mpcpl|Playlist (*.pls)|*.pls|Winamp playlist (*.m3u)|*.m3u|Windows Media playlist (*.asx)|*.asx||"),
                this);

            if (fd.DoModal() != IDOK) {
                break;
            }

            CTextFile::enc encoding = (CTextFile::enc)fd.GetEncoding();

            int idx = fd.m_pOFN->nFilterIndex;

            if (encoding == CTextFile::DEFAULT_ENCODING) {
                if (idx == 1 || idx == 3) {
                    encoding = CTextFile::UTF8;
                } else {
                    encoding = CTextFile::ANSI;
                }
            }

            CPath path(fd.GetPathName());

            switch (idx) {
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
            while (pos && fRemovePath) {
                CPlaylistItem& pli = m_pl.GetNext(pos);

                if (pli.m_type != CPlaylistItem::file) {
                    fRemovePath = false;
                } else {
                    POSITION pos2;

                    pos2 = pli.m_fns.GetHeadPosition();
                    while (pos2 && fRemovePath) {
                        CString fn = pli.m_fns.GetNext(pos2);

                        CPath fnPath(fn);
                        fnPath.RemoveFileSpec();
                        if (base != (LPCTSTR)fnPath) {
                            fRemovePath = false;
                        }
                    }

                    pos2 = pli.m_subs.GetHeadPosition();
                    while (pos2 && fRemovePath) {
                        CString fn = pli.m_subs.GetNext(pos2);

                        CPath fnPath(fn);
                        fnPath.RemoveFileSpec();
                        if (base != (LPCTSTR)fnPath) {
                            fRemovePath = false;
                        }
                    }
                }
            }

            if (idx == 1) {
                SaveMPCPlayList(path, encoding, fRemovePath);
                break;
            }

            CTextFile f;
            if (!f.Save(path, encoding)) {
                break;
            }

            if (idx == 2) {
                f.WriteString(_T("[playlist]\n"));
            } else if (idx == 4) {
                f.WriteString(_T("<ASX version = \"3.0\">\n"));
            }

            pos = m_pl.GetHeadPosition();
            CString str;
            int i;
            for (i = 0; pos; i++) {
                CPlaylistItem& pli = m_pl.GetNext(pos);

                if (pli.m_type != CPlaylistItem::file) {
                    continue;
                }

                CString fn = pli.m_fns.GetHead();

                /*
                            if (fRemovePath)
                            {
                                CPath p(path);
                                p.StripPath();
                                fn = (LPCTSTR)p;
                            }
                */

                switch (idx) {
                    case 2:
                        str.Format(_T("File%d=%s\n"), i + 1, fn.GetString());
                        break;
                    case 3:
                        str.Format(_T("%s\n"), fn.GetString());
                        break;
                    case 4:
                        str.Format(_T("<Entry><Ref href = \"%s\"/></Entry>\n"), fn.GetString());
                        break;
                    default:
                        break;
                }
                f.WriteString(str);
            }

            if (idx == 2) {
                str.Format(_T("NumberOfEntries=%d\n"), i);
                f.WriteString(str);
                f.WriteString(_T("Version=2\n"));
            } else if (idx == 4) {
                f.WriteString(_T("</ASX>\n"));
            }
        }
        break;
        case M_SHUFFLE:
            s.bShufflePlaylistItems = !s.bShufflePlaylistItems;
            m_pl.SetShuffle(s.bShufflePlaylistItems);
            break;
        case M_HIDEFULLSCREEN:
            s.bHidePlaylistFullScreen = !s.bHidePlaylistFullScreen;
            break;
        default:
            break;
    }
}

void CPlayerPlaylistBar::OnLvnBeginlabeleditList(NMHDR* pNMHDR, LRESULT* pResult) {
    const CAppSettings& s = AfxGetAppSettings();
    if (s.bMPCThemeLoaded) {
        HWND e_hwnd = (HWND)m_list.SendMessage(LVM_GETEDITCONTROL);
        if (::IsWindow(m_edit.m_hWnd)) m_edit.UnsubclassWindow();
        m_edit.SubclassWindow(e_hwnd);
    }
}

void CPlayerPlaylistBar::OnLvnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

    if (pDispInfo->item.iItem >= 0 && pDispInfo->item.pszText) {
        CPlaylistItem& pli = m_pl.GetAt((POSITION)m_list.GetItemData(pDispInfo->item.iItem));
        pli.m_label = pDispInfo->item.pszText;
        m_list.SetItemText(pDispInfo->item.iItem, 0, pDispInfo->item.pszText);
    }
    const CAppSettings& s = AfxGetAppSettings();
    if (s.bMPCThemeLoaded) {
        if (::IsWindow(m_edit.m_hWnd)) m_edit.UnsubclassWindow();
    }
    *pResult = 0;
}

void CPlayerPlaylistBar::OnXButtonDown(UINT nFlags, UINT nButton, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(point);
    if (m_pMainFrame->GetPlaybackMode() == PM_FILE && GetCount() > 1) {
        switch (nButton) {
            case XBUTTON1:
                if (SetPrev()) {
                    m_pMainFrame->OpenCurPlaylistItem();
                }
                break;
            case XBUTTON2:
                if (SetNext()) {
                    m_pMainFrame->OpenCurPlaylistItem();
                }
                break;
        }
    }
}

void CPlayerPlaylistBar::OnXButtonUp(UINT nFlags, UINT nButton, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(nButton);
    UNREFERENCED_PARAMETER(point);
    // do nothing
}

void CPlayerPlaylistBar::OnXButtonDblClk(UINT nFlags, UINT nButton, CPoint point)
{
    OnXButtonDown(nFlags, nButton, point);
}
