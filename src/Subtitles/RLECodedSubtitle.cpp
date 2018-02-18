/*
 * (C) 2008-2016 see Authors.txt
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
#include "DVBSub.h"
#include "PGSSub.h"
#include "RLECodedSubtitle.h"

CRLECodedSubtitle::CRLECodedSubtitle(CCritSec* pLock, const CString& name, LCID lcid)
    : CSubPicProviderImpl(pLock)
    , m_name(name)
    , m_lcid(lcid)
    , m_eHearingImpaired(Subtitle::HI_UNKNOWN)
{
}

STDMETHODIMP CRLECodedSubtitle::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);
    *ppv = nullptr;

    return
        QI(IPersist)
        QI(ISubStream)
        QI(ISubPicProvider)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// ISubPicProvider

STDMETHODIMP CRLECodedSubtitle::GetRelativeTo(POSITION pos, RelativeTo& relativeTo)
{
    relativeTo = WINDOW;
    return S_OK;
}

// IPersist

STDMETHODIMP CRLECodedSubtitle::GetClassID(CLSID* pClassID)
{
    return pClassID ? *pClassID = __uuidof(this), S_OK : E_POINTER;
}

// ISubStream

STDMETHODIMP_(int) CRLECodedSubtitle::GetStreamCount()
{
    return 1;
}

STDMETHODIMP CRLECodedSubtitle::GetStreamInfo(int iStream, WCHAR** ppName, LCID* pLCID)
{
    if (iStream != 0) {
        return E_INVALIDARG;
    }

    if (ppName) {
        *ppName = (WCHAR*)CoTaskMemAlloc((m_name.GetLength() + 1) * sizeof(WCHAR));
        if (!(*ppName)) {
            return E_OUTOFMEMORY;
        }

        wcscpy_s(*ppName, m_name.GetLength() + 1, CStringW(m_name));
    }

    if (pLCID) {
        *pLCID = m_lcid;
    }

    return S_OK;
}

STDMETHODIMP_(int) CRLECodedSubtitle::GetStream()
{
    return 0;
}

STDMETHODIMP CRLECodedSubtitle::SetStream(int iStream)
{
    return iStream == 0 ? S_OK : E_FAIL;
}

STDMETHODIMP CRLECodedSubtitle::Reload()
{
    return S_OK;
}

STDMETHODIMP CRLECodedSubtitle::SetSourceTargetInfo(CString yuvMatrix, int targetBlackLevel, int targetWhiteLevel)
{
    yuvMatrix.Replace(_T(".VSFilter"), _T(""));
    int nPos = 0;
    CString range = yuvMatrix.Tokenize(_T("."), nPos);
    CString matrix = yuvMatrix.Mid(nPos);

    ColorConvTable::YuvRangeType sourceRange = ColorConvTable::RANGE_TV;
    if (range == _T("PC")) {
        sourceRange = ColorConvTable::RANGE_PC;
    }

    if (matrix == _T("709")) {
        m_eSourceMatrix = ColorConvTable::BT709;
    } else if (matrix == _T("240M")) {
        m_eSourceMatrix = ColorConvTable::BT709;
    } else if (matrix == _T("601")) {
        m_eSourceMatrix = ColorConvTable::BT601;
    } else {
        m_eSourceMatrix = ColorConvTable::NONE;
    }

    ColorConvTable::SetDefaultConvType(ColorConvTable::BT601, sourceRange, (targetWhiteLevel < 245), false); // Matrix isn't relevant here.

    return S_OK;
}

HRESULT CRLECodedSubtitle::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    CAutoLock cAutoLock(&m_csCritSec);

    Reset();

    return S_OK;
}
