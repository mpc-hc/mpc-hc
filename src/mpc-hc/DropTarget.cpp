/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012, 2015 see Authors.txt
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
#include "DropTarget.h"

CDropTarget::CDropTarget()
{
    m_pDropHelper.CoCreateInstance(CLSID_DragDropHelper, nullptr, CLSCTX_INPROC_SERVER);
};

DROPEFFECT CDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
    DROPEFFECT dropEffect = DROPEFFECT_NONE;

    auto pClient = dynamic_cast<CDropClient*>(pWnd);
    if (pClient && (pDataObject->IsDataAvailable(CF_HDROP) || pDataObject->IsDataAvailable(CF_URL))) {
        dropEffect = pClient->OnDropAccept(pDataObject, dwKeyState, point);
    }

    if (m_pDropHelper) {
        m_pDropHelper->DragEnter(pWnd->GetSafeHwnd(), pDataObject->GetIDataObject(FALSE), &point, dropEffect);
    }

    return dropEffect;
}

DROPEFFECT CDropTarget::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
    DROPEFFECT dropEffect = DROPEFFECT_NONE;

    auto pClient = dynamic_cast<CDropClient*>(pWnd);
    if (pClient && (pDataObject->IsDataAvailable(CF_HDROP) || pDataObject->IsDataAvailable(CF_URL))) {
        dropEffect = pClient->OnDropAccept(pDataObject, dwKeyState, point);
    }

    if (m_pDropHelper) {
        m_pDropHelper->DragOver(&point, dropEffect);
    }

    return dropEffect;
}

BOOL CDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
    BOOL bResult = FALSE;
    CAtlList<CString> slFiles;

    if (auto pClient = dynamic_cast<CDropClient*>(pWnd)) {
        if (pDataObject->IsDataAvailable(CF_HDROP)) {
            if (HGLOBAL hGlobal = pDataObject->GetGlobalData(CF_HDROP)) {
                if (HDROP hDrop = static_cast<HDROP>(GlobalLock(hGlobal))) {
                    UINT nFiles = ::DragQueryFile(hDrop, UINT_MAX, nullptr, 0);
                    for (UINT iFile = 0; iFile < nFiles; iFile++) {
                        CString fn;
                        fn.ReleaseBuffer(::DragQueryFile(hDrop, iFile, fn.GetBuffer(MAX_PATH), MAX_PATH));
                        slFiles.AddTail(fn);
                    }
                    ::DragFinish(hDrop);
                    pClient->OnDropFiles(slFiles, dropEffect);
                    bResult = TRUE;
                }
                GlobalUnlock(hGlobal);
            }
        } else if (pDataObject->IsDataAvailable(CF_URL)) {
            FORMATETC fmt = { CF_URL, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
            if (HGLOBAL hGlobal = pDataObject->GetGlobalData(CF_URL, &fmt)) {
                LPCSTR pText = static_cast<LPCSTR>(GlobalLock(hGlobal));
                if (AfxIsValidString(pText)) {
                    slFiles.AddTail(pText);
                    pClient->OnDropFiles(slFiles, dropEffect);
                    GlobalUnlock(hGlobal);
                    bResult = TRUE;
                }
            }
        }
    }

    if (m_pDropHelper) {
        m_pDropHelper->Drop(pDataObject->GetIDataObject(FALSE), &point, dropEffect);
    }

    return bResult;
}

DROPEFFECT CDropTarget::OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point)
{
    if (OnDrop(pWnd, pDataObject, dropDefault, point)) {
        return dropDefault;
    }
    return DROPEFFECT_NONE;
}

void CDropTarget::OnDragLeave(CWnd* pWnd)
{
    if (m_pDropHelper) {
        m_pDropHelper->DragLeave();
    }
}

DROPEFFECT CDropTarget::OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point)
{
    return DROPEFFECT_NONE;
}
