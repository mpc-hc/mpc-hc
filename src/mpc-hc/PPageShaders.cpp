/*
 * (C) 2013 see Authors.txt
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
#include "PPageShaders.h"
#include "MainFrm.h"

CShaderListBox::CShaderListBox()
    : CListBox()
{
}

int CShaderListBox::AddShader(const CAppSettings::Shader& shader)
{
    int ret = CListBox::AddString(GetTitle(shader));
    if (ret >= 0) {
        m_List.push_back(shader);
    }
    return ret;
}

int CShaderListBox::InsertShader(int nIndex, const CAppSettings::Shader& shader)
{
    int ret = CListBox::InsertString(nIndex, GetTitle(shader));
    if (ret >= 0) {
        m_List.insert(m_List.begin() + ret, shader);
    }
    return ret;
}

int CShaderListBox::UpShader(UINT nIndex)
{
    int ret = LB_ERR;
    if ((nIndex != 0) && ((int)nIndex < GetCount())) {
        ASSERT(nIndex < m_List.size());
        CAppSettings::Shader shader = m_List.at(nIndex);
        VERIFY(DeleteShader(nIndex) != LB_ERR);
        ret = InsertShader(nIndex - 1, shader);
    }
    return ret;
}

int CShaderListBox::DownShader(UINT nIndex)
{
    int ret = LB_ERR;
    if ((int)nIndex + 1 < GetCount()) {
        ASSERT(nIndex < m_List.size());
        CAppSettings::Shader shader = m_List.at(nIndex);
        VERIFY(DeleteShader(nIndex) != LB_ERR);
        ret = InsertShader(nIndex + 1, shader);
    }
    return ret;
}

int CShaderListBox::DeleteShader(UINT nIndex)
{
    int ret = CListBox::DeleteString(nIndex);
    if (ret >= 0) {
        ASSERT(ret < (int)m_List.size());
        m_List.erase(m_List.begin() + nIndex);
    }
    return ret;
}

void CShaderListBox::ResetContent()
{
    CListBox::ResetContent();
    m_List.clear();
}

void CShaderListBox::SetList(const CAppSettings::ShaderList& list)
{
    ResetContent();
    for (auto it = list.cbegin(); it != list.cend(); ++it) {
        VERIFY(AddShader(*it) >= 0);
    }
}

const CAppSettings::ShaderList& CShaderListBox::GetList()
{
    return m_List;
}

bool CShaderListBox::UpCurrentShader()
{
    bool ret = false;
    int sel = GetCurSel();
    if ((sel != LB_ERR) && (sel > 0)) {
        sel = UpShader(sel);
        if (sel >= 0) {
            if (SetCurSel(sel) != LB_ERR) {
                ret = true;
            } else {
                ASSERT(FALSE);
            }
        } else {
            ASSERT(FALSE);
        }
    }
    return ret;
}

bool CShaderListBox::DownCurrentShader()
{
    bool ret = false;
    int sel = GetCurSel();
    if ((sel != LB_ERR) && (sel + 1 < GetCount())) {
        sel = DownShader(sel);
        if (sel >= 0) {
            if (SetCurSel(sel) != LB_ERR) {
                ret = true;
            } else {
                ASSERT(FALSE);
            }
        } else {
            ASSERT(FALSE);
        }
    }
    return ret;
}

bool CShaderListBox::DeleteCurrentShader()
{
    bool ret = false;
    int sel = GetCurSel();
    if (sel != LB_ERR) {
        if (DeleteShader(sel) != LB_ERR) {
            sel = min(sel, GetCount() - 1);
            if ((sel < 0) || (SetCurSel(sel) != LB_ERR)) {
                ret = true;
            } else {
                ASSERT(FALSE);
            }
        } else {
            ASSERT(FALSE);
        }
    }
    return ret;
}

CString CShaderListBox::GetTitle(const CAppSettings::Shader& shader)
{
    int r;
    CString ret = shader.filePath;
    r = ret.ReverseFind(_T('\\'));
    if (r != -1) { // remove dirname
        ret = ret.Right(ret.GetLength() - r - 1);
    }
    r = ret.ReverseFind(_T('.'));
    if (r != -1) { // remove ext
        ret = ret.Left(r);
    }
    if (!PathFileExists(shader.filePath)) {
        ret += _T(" <not found!>"); // TODO: externalize this string and merge it with the one in PPageExternalFilters
    }
    return ret;
}

void CShaderListBox::PreSubclassWindow()
{
    CListBox::PreSubclassWindow();
    EnableToolTips(TRUE);
}

INT_PTR CShaderListBox::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
    BOOL out = FALSE;
    pTI->uId = ItemFromPoint(point, out);
    if (out) {
        return -1;
    }
    CRect rect;
    VERIFY(GetItemRect(pTI->uId, rect) != LB_ERR);
    pTI->rect = rect;
    pTI->hwnd = m_hWnd;
    pTI->lpszText = LPSTR_TEXTCALLBACK;
    pTI->uFlags |= TTF_ALWAYSTIP;
    return pTI->uId;
}

BOOL CShaderListBox::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;
    ASSERT((int)pNMHDR->idFrom < GetCount());
    ASSERT(pNMHDR->idFrom < m_List.size());
    pTTT->lpszText = (LPTSTR)(LPCTSTR)m_List.at(pNMHDR->idFrom).filePath;
    return TRUE;
}

BEGIN_MESSAGE_MAP(CShaderListBox, CListBox)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()

CPPageShaders::CPPageShaders()
    : CPPageBase(IDD_PPAGESHADERS, IDD_PPAGESHADERS)
{
}

void CPPageShaders::DoDataExchange(CDataExchange* pDX)
{
    CPPageBase::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_Shaders);
    DDX_Control(pDX, IDC_LIST2, m_PreResize);
    DDX_Control(pDX, IDC_LIST3, m_PostResize);
}

BOOL CPPageShaders::OnInitDialog()
{
    CPPageBase::OnInitDialog();
    const CAppSettings& s = AfxGetAppSettings();

    m_Shaders.ResetContent();
    CFileFind finder;
    ASSERT(CAppSettings::ShaderList::GetShadersDir().Right(1) == _T('\\'));
    ASSERT(CString(SHADERS_EXT).Left(1) == _T('.'));
    if (finder.FindFile(CAppSettings::ShaderList::GetShadersDir() + _T('*') + SHADERS_EXT)) {
        while (finder.FindNextFile()) {
            CAppSettings::Shader shader;
            shader.filePath = finder.GetFilePath();
            VERIFY(m_Shaders.AddShader(shader) >= 0);
        }
    }

    for (auto it = s.m_ShadersExtra.cbegin(); it != s.m_ShadersExtra.cend(); ++it) {
        if (!it->IsDefault()) {
            VERIFY(m_Shaders.AddShader(*it) >= 0);
        }
    }

    m_PreResize.SetList(s.m_ShadersSelected.preResize);
    m_PostResize.SetList(s.m_ShadersSelected.postResize);

    UpdateState();

    return TRUE;
}

BOOL CPPageShaders::OnApply()
{
    CAppSettings& s = AfxGetAppSettings();
    bool updatePre = false, updatePost = false;

    if (m_PreResize.GetList() != s.m_ShadersSelected.preResize) {
        s.m_ShadersSelected.preResize = m_PreResize.GetList();
        updatePre = true;
    }

    if (m_PostResize.GetList() != s.m_ShadersSelected.postResize) {
        s.m_ShadersSelected.postResize = m_PostResize.GetList();
        updatePost = true;
    }

    ((CMainFrame*)AfxGetMainWnd())->SetShaders(updatePre, updatePost);

    s.m_ShadersExtra.clear();
    auto list = m_Shaders.GetList();
    for (auto it = list.cbegin(); it != list.cend(); ++it) {
        if (!it->IsDefault()) {
            s.m_ShadersExtra.push_back(*it);
        }
    }

    return CPPageBase::OnApply();
}

void CPPageShaders::UpdateState()
{
    BOOL state;
    int sel;

    sel = m_Shaders.GetCurSel();
    state = (sel != LB_ERR) ? TRUE : FALSE;
    GetDlgItem(IDC_BUTTON1)->EnableWindow(state); // "add to pre-resize" button
    GetDlgItem(IDC_BUTTON2)->EnableWindow(state); // "add to post-resize" button
    if (state) {
        auto list = m_Shaders.GetList();
        ASSERT(sel < (int)list.size());
        if (list.at(sel).IsDefault()) { // default shaders are not allowed to be removed
            state = FALSE;
        }
    }
    GetDlgItem(IDC_BUTTON13)->EnableWindow(state); // "remove shader" button
    // "add shader file" button is always enabled

    // TODO: "load preset" button
    // TODO: "save preset" button
    // TODO: "delete preset" button

    sel = m_PreResize.GetCurSel();
    state = (sel != LB_ERR) && (sel > 0) ? TRUE : FALSE;
    GetDlgItem(IDC_BUTTON6)->EnableWindow(state); // "up pre-resize shader" button
    state = (sel != LB_ERR) && (sel + 1 < m_PreResize.GetCount()) ? TRUE : FALSE;
    GetDlgItem(IDC_BUTTON7)->EnableWindow(state); // "down pre-resize shader" button
    state = sel != LB_ERR ? TRUE : FALSE;
    GetDlgItem(IDC_BUTTON8)->EnableWindow(state); // "remove pre-resize shader" button

    sel = m_PostResize.GetCurSel();
    state = (sel != LB_ERR) && (sel > 0) ? TRUE : FALSE;
    GetDlgItem(IDC_BUTTON9)->EnableWindow(state);  // "up post-resize shader" button
    state = (sel != LB_ERR) && (sel + 1 < m_PostResize.GetCount()) ? TRUE : FALSE;
    GetDlgItem(IDC_BUTTON10)->EnableWindow(state); // "down post-resize shader" button
    state = sel != LB_ERR ? TRUE : FALSE;
    GetDlgItem(IDC_BUTTON11)->EnableWindow(state); // "remove post-resize shader" button
}

void CPPageShaders::OnLoadShaderPreset()
{
    // TODO: write me
}

void CPPageShaders::OnSaveShaderPreset()
{
    // TODO: write me
}

void CPPageShaders::OnDeleteShaderPreset()
{
    // TODO: write me
}

void CPPageShaders::OnAddToPreResize()
{
    int sel;
    sel = m_Shaders.GetCurSel();
    ASSERT(sel != LB_ERR);
    VERIFY(m_PreResize.AddShader(m_Shaders.GetList().at(sel)) >= 0);
    UpdateState();
    SetModified();
}

void CPPageShaders::OnAddToPostResize()
{
    int sel;
    sel = m_Shaders.GetCurSel();
    ASSERT(sel != LB_ERR);
    VERIFY(m_PostResize.AddShader(m_Shaders.GetList().at(sel)) >= 0);
    UpdateState();
    SetModified();
}

void CPPageShaders::OnUpPreResize()
{
    VERIFY(m_PreResize.UpCurrentShader());
    UpdateState();
    SetModified();
}

void CPPageShaders::OnDownPreResize()
{
    VERIFY(m_PreResize.DownCurrentShader());
    UpdateState();
    SetModified();
}

void CPPageShaders::OnRemovePreResize()
{
    VERIFY(m_PreResize.DeleteCurrentShader());
    UpdateState();
    SetModified();
}

void CPPageShaders::OnUpPostResize()
{
    VERIFY(m_PostResize.UpCurrentShader());
    UpdateState();
    SetModified();
}

void CPPageShaders::OnDownPostResize()
{
    VERIFY(m_PostResize.DownCurrentShader());
    UpdateState();
    SetModified();
}

void CPPageShaders::OnRemovePostResize()
{
    VERIFY(m_PostResize.DeleteCurrentShader());
    UpdateState();
    SetModified();
}

void CPPageShaders::OnAddShaderFile()
{
    ASSERT(CString(SHADERS_EXT).Left(1) == _T('.'));
    CString dlgFilter = CString(_T("Pixel Shader Files (*")) + SHADERS_EXT + _T(")|*") + SHADERS_EXT + _T("|");
    DWORD dlgFlags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    CFileDialog dlg(TRUE, NULL, NULL, dlgFlags, dlgFilter, this);

    if (dlg.DoModal() == IDOK) {
        auto list = m_Shaders.GetList();
        POSITION dlgPos = dlg.GetStartPosition();
        while (dlgPos) {
            CAppSettings::Shader shader;
            shader.filePath = dlg.GetNextPathName(dlgPos);

            bool alreadyInList = false;
            for (auto it = list.cbegin(); it != list.cend(); ++it) {
                if (*it == shader) {
                    alreadyInList = true;
                    break;
                }
            }
            if (!alreadyInList) {
                m_Shaders.AddShader(shader);
            }
        }

        UpdateState();
        SetModified();
    }
}

void CPPageShaders::OnRemoveShader()
{
    VERIFY(m_Shaders.DeleteCurrentShader());
    UpdateState();
    SetModified();
}

BEGIN_MESSAGE_MAP(CPPageShaders, CPPageBase)
    ON_LBN_SELCHANGE(IDC_LIST1, UpdateState)
    ON_LBN_SELCHANGE(IDC_LIST2, UpdateState)
    ON_LBN_SELCHANGE(IDC_LIST3, UpdateState)
    ON_BN_CLICKED(IDC_BUTTON1,  OnAddToPreResize)
    ON_BN_CLICKED(IDC_BUTTON2,  OnAddToPostResize)
    ON_BN_CLICKED(IDC_BUTTON3,  OnLoadShaderPreset)
    ON_BN_CLICKED(IDC_BUTTON4,  OnSaveShaderPreset)
    ON_BN_CLICKED(IDC_BUTTON5,  OnDeleteShaderPreset)
    ON_BN_CLICKED(IDC_BUTTON6,  OnUpPreResize)
    ON_BN_CLICKED(IDC_BUTTON7,  OnDownPreResize)
    ON_BN_CLICKED(IDC_BUTTON8,  OnRemovePreResize)
    ON_BN_CLICKED(IDC_BUTTON9,  OnUpPostResize)
    ON_BN_CLICKED(IDC_BUTTON10, OnDownPostResize)
    ON_BN_CLICKED(IDC_BUTTON11, OnRemovePostResize)
    ON_BN_CLICKED(IDC_BUTTON12, OnAddShaderFile)
    ON_BN_CLICKED(IDC_BUTTON13, OnRemoveShader)
END_MESSAGE_MAP()
