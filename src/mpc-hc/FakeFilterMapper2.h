/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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

#pragma once

#include <atlbase.h>


class FilterOverride
{
public:
    bool fDisabled, fTemporary;
    enum { REGISTERED, EXTERNAL } type;
    // REGISTERED
    CStringW dispname;
    // EXTERNAL
    CString path, name;
    CLSID clsid;
    // props
    CAtlList<GUID> guids, backup;
    enum { PREFERRED, BLOCK, MERIT };
    int iLoadType;
    DWORD dwMerit;

    FilterOverride()
        : fDisabled(false)
        , fTemporary(false)
        , type(EXTERNAL)
        , clsid(GUID_NULL)
        , iLoadType(0)
        , dwMerit(0) {
    }
    FilterOverride(FilterOverride* f) {
        fDisabled = f->fDisabled;
        fTemporary = f->fTemporary;
        type = f->type;
        dispname = f->dispname;
        path = f->path;
        name = f->name;
        clsid = f->clsid;
        guids.AddTailList(&f->guids);
        backup.AddTailList(&f->backup);
        iLoadType = f->iLoadType;
        dwMerit = f->dwMerit;
    }
};

class CFilterMapper2 : protected CUnknown, public IFilterMapper2
{
    static bool s_bInitialized;

    CComPtr<IUnknown> m_pFM2;
    CString m_path;

    bool m_bRefCounted, m_bAllowUnreg;

protected:
    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // IFilterMapper2

    STDMETHODIMP CreateCategory(REFCLSID clsidCategory, DWORD dwCategoryMerit, LPCWSTR Description);
    STDMETHODIMP UnregisterFilter(const CLSID* pclsidCategory, const OLECHAR* szInstance, REFCLSID Filter);
    STDMETHODIMP RegisterFilter(REFCLSID clsidFilter, LPCWSTR Name, IMoniker** ppMoniker, const CLSID* pclsidCategory, const OLECHAR* szInstance, const REGFILTER2* prf2);
    STDMETHODIMP EnumMatchingFilters(IEnumMoniker** ppEnum, DWORD dwFlags, BOOL bExactMatch, DWORD dwMerit,
                                     BOOL bInputNeeded, DWORD cInputTypes, const GUID* pInputTypes, const REGPINMEDIUM* pMedIn, const CLSID* pPinCategoryIn, BOOL bRender,
                                     BOOL bOutputNeeded, DWORD cOutputTypes, const GUID* pOutputTypes, const REGPINMEDIUM* pMedOut, const CLSID* pPinCategoryOut);

public:
    CFilterMapper2(bool bRefCounted, bool bAllowUnreg = false, LPUNKNOWN pUnkOuter = nullptr);
    virtual ~CFilterMapper2();

    void SetInner(IUnknown* pUnk) { m_pFM2 = pUnk; }

    static void Init();

    static IFilterMapper2* s_pFilterMapper2;
    CList<FilterOverride*> m_filters;
    void Register(CString path);
};
