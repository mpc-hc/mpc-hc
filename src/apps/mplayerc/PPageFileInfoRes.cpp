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
#include "mplayerc.h"
#include "PPageFileInfoRes.h"


// CPPageFileInfoRes dialog

IMPLEMENT_DYNAMIC(CPPageFileInfoRes, CPPageBase)
CPPageFileInfoRes::CPPageFileInfoRes(CString fn, IFilterGraph* pFG)
    : CPPageBase(CPPageFileInfoRes::IDD, CPPageFileInfoRes::IDD)
    , m_fn(fn)
    , m_hIcon(NULL)
    , m_pFG(pFG)
{
}

CPPageFileInfoRes::~CPPageFileInfoRes()
{
    if(m_hIcon) DestroyIcon(m_hIcon);
}

void CPPageFileInfoRes::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEFAULTICON, m_icon);
    DDX_Text(pDX, IDC_EDIT1, m_fn);
    DDX_Control(pDX, IDC_LIST1, m_list);
}

BEGIN_MESSAGE_MAP(CPPageFileInfoRes, CPPageBase)
    ON_BN_CLICKED(IDC_BUTTON1, OnSaveAs)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON1, OnUpdateSaveAs)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnNMDblclkList1)
END_MESSAGE_MAP()

// CPPageFileInfoRes message handlers

BOOL CPPageFileInfoRes::OnInitDialog()
{
    __super::OnInitDialog();

    if(m_hIcon = LoadIcon(m_fn, false))
        m_icon.SetIcon(m_hIcon);

    m_fn.TrimRight('/');
    int i = max(m_fn.ReverseFind('\\'), m_fn.ReverseFind('/'));
    if(i >= 0 && i < m_fn.GetLength()-1)
        m_fn = m_fn.Mid(i+1);

    m_list.SetExtendedStyle(m_list.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

    m_list.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 187);
    m_list.InsertColumn(1, _T("Mime Type"), LVCFMT_LEFT, 127);

    BeginEnumFilters(m_pFG, pEF, pBF)
    {
        if(CComQIPtr<IDSMResourceBag> pRB = pBF)
            if(pRB && pRB->ResGetCount() > 0)
            {
                for(DWORD i = 0; i < pRB->ResGetCount(); i++)
                {
                    CComBSTR name, desc, mime;
                    BYTE* pData = NULL;
                    DWORD len = 0;
                    if(SUCCEEDED(pRB->ResGet(i, &name, &desc, &mime, &pData, &len, NULL)))
                    {
                        CDSMResource r;
                        r.name = name;
                        r.desc = desc;
                        r.mime = mime;
                        r.data.SetCount(len);
                        memcpy(r.data.GetData(), pData, r.data.GetCount());
                        CoTaskMemFree(pData);
                        POSITION pos = m_res.AddTail(r);
                        int iItem = m_list.InsertItem(m_list.GetItemCount(), CString(name));
                        m_list.SetItemText(iItem, 1, CString(mime));
                        m_list.SetItemData(iItem, (DWORD_PTR)pos);
                    }
                }
            }
    }
    EndEnumFilters

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPageFileInfoRes::OnSaveAs()
{
    int i = m_list.GetSelectionMark();
    if(i < 0) return;

    CDSMResource& r = m_res.GetAt((POSITION)m_list.GetItemData(i));

    CFileDialog fd(FALSE, NULL, CString(r.name),
                   OFN_EXPLORER|OFN_ENABLESIZING|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
                   _T("All files|*.*||"), this, 0);
    if(fd.DoModal() == IDOK)
    {
        if(FILE* f = _tfopen(fd.GetPathName(), _T("wb")))
        {
            fwrite(r.data.GetData(), 1, r.data.GetCount(), f);
            fclose(f);
        }
    }
}

void CPPageFileInfoRes::OnUpdateSaveAs(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_list.GetSelectedCount());
}

void CPPageFileInfoRes::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
    int i = m_list.GetSelectionMark();
    if(i < 0) return;

    CDSMResource& r = m_res.GetAt((POSITION)m_list.GetItemData(i));

    CString url;
    url.Format(_T("http://localhost:%d/convres.html?id=%x"), AfxGetAppSettings().nWebServerPort, (DWORD)&r);
    ShellExecute(NULL, _T("open"), url, NULL, NULL, SW_SHOWDEFAULT);

    *pResult = 0;
}
