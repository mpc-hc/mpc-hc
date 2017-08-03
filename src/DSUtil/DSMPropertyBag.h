/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2017 see Authors.txt
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

#include <atlcoll.h>
#include <atlsimpcoll.h>

// IDSMPropertyBag

interface __declspec(uuid("232FD5D2-4954-41E7-BF9B-09E1257B1A95"))
    IDSMPropertyBag :
    public IPropertyBag2
{
    STDMETHOD(SetProperty)(LPCWSTR key, LPCWSTR value) PURE;
    STDMETHOD(SetProperty)(LPCWSTR key, VARIANT* var) PURE;
    STDMETHOD(GetProperty)(LPCWSTR key, BSTR* value) PURE;
    STDMETHOD(DelAllProperties)() PURE;
    STDMETHOD(DelProperty)(LPCWSTR key) PURE;
};

class IDSMPropertyBagImpl : public ATL::CSimpleMap<CStringW, CStringW>, public IDSMPropertyBag, public IPropertyBag
{
    BOOL Add(const CStringW& key, const CStringW& val) {
        return __super::Add(key, val);
    }
    BOOL SetAt(const CStringW& key, const CStringW& val) {
        return __super::SetAt(key, val);
    }

public:
    IDSMPropertyBagImpl();
    virtual ~IDSMPropertyBagImpl();

    // IPropertyBag

    STDMETHODIMP Read(LPCOLESTR pszPropName, VARIANT* pVar, IErrorLog* pErrorLog);
    STDMETHODIMP Write(LPCOLESTR pszPropName, VARIANT* pVar);

    // IPropertyBag2

    STDMETHODIMP Read(ULONG cProperties, PROPBAG2* pPropBag, IErrorLog* pErrLog, VARIANT* pvarValue, HRESULT* phrError);
    STDMETHODIMP Write(ULONG cProperties, PROPBAG2* pPropBag, VARIANT* pvarValue);
    STDMETHODIMP CountProperties(ULONG* pcProperties);
    STDMETHODIMP GetPropertyInfo(ULONG iProperty, ULONG cProperties, PROPBAG2* pPropBag, ULONG* pcProperties);
    STDMETHODIMP LoadObject(LPCOLESTR pstrName, DWORD dwHint, IUnknown* pUnkObject, IErrorLog* pErrLog);

    // IDSMPropertyBag

    STDMETHODIMP SetProperty(LPCWSTR key, LPCWSTR value);
    STDMETHODIMP SetProperty(LPCWSTR key, VARIANT* var);
    STDMETHODIMP GetProperty(LPCWSTR key, BSTR* value);
    STDMETHODIMP DelAllProperties();
    STDMETHODIMP DelProperty(LPCWSTR key);
};

// IDSMResourceBag

interface __declspec(uuid("EBAFBCBE-BDE0-489A-9789-05D5692E3A93"))
    IDSMResourceBag :
    public IUnknown
{
    STDMETHOD_(DWORD, ResGetCount)() PURE;
    STDMETHOD(ResGet)(DWORD iIndex, BSTR* ppName, BSTR* ppDesc, BSTR* ppMime, BYTE** ppData, DWORD* pDataLen, DWORD_PTR* pTag) PURE;
    STDMETHOD(ResSet)(DWORD iIndex, LPCWSTR pName, LPCWSTR pDesc, LPCWSTR pMime, const BYTE* pData, DWORD len, DWORD_PTR tag) PURE;
    STDMETHOD(ResAppend)(LPCWSTR pName, LPCWSTR pDesc, LPCWSTR pMime, BYTE* pData, DWORD len, DWORD_PTR tag) PURE;
    STDMETHOD(ResRemoveAt)(DWORD iIndex) PURE;
    STDMETHOD(ResRemoveAll)(DWORD_PTR tag) PURE;
};

class CDSMResource
{
public:
    DWORD_PTR tag;
    CStringW name, desc, mime;
    CAtlArray<BYTE> data;
    CDSMResource();
    CDSMResource(const CDSMResource& r);
    CDSMResource(LPCWSTR name, LPCWSTR desc, LPCWSTR mime, BYTE* pData, int len, DWORD_PTR tag = 0);
    virtual ~CDSMResource();
    CDSMResource& operator = (const CDSMResource& r);

    // global access to all resources
    static CCritSec m_csResources;
    static CAtlMap<uintptr_t, CDSMResource*> m_resources;
};

class IDSMResourceBagImpl : public IDSMResourceBag
{
protected:
    CAtlArray<CDSMResource> m_resources;

public:
    IDSMResourceBagImpl();

    void operator += (const CDSMResource& r) { m_resources.Add(r); }

    // IDSMResourceBag

    STDMETHODIMP_(DWORD) ResGetCount();
    STDMETHODIMP ResGet(DWORD iIndex, BSTR* ppName, BSTR* ppDesc, BSTR* ppMime,
                        BYTE** ppData, DWORD* pDataLen, DWORD_PTR* pTag = nullptr);
    STDMETHODIMP ResSet(DWORD iIndex, LPCWSTR pName, LPCWSTR pDesc, LPCWSTR pMime,
                        const BYTE* pData, DWORD len, DWORD_PTR tag = 0);
    STDMETHODIMP ResAppend(LPCWSTR pName, LPCWSTR pDesc, LPCWSTR pMime,
                           BYTE* pData, DWORD len, DWORD_PTR tag = 0);
    STDMETHODIMP ResRemoveAt(DWORD iIndex);
    STDMETHODIMP ResRemoveAll(DWORD_PTR tag = 0);
};

// IDSMChapterBag

interface __declspec(uuid("2D0EBE73-BA82-4E90-859B-C7C48ED3650F"))
    IDSMChapterBag :
    public IUnknown
{
    STDMETHOD_(DWORD, ChapGetCount)() PURE;
    STDMETHOD(ChapGet)(DWORD iIndex, REFERENCE_TIME* prt, BSTR* ppName) PURE;
    STDMETHOD(ChapSet)(DWORD iIndex, REFERENCE_TIME rt, LPCWSTR pName) PURE;
    STDMETHOD(ChapAppend)(REFERENCE_TIME rt, LPCWSTR pName) PURE;
    STDMETHOD(ChapRemoveAt)(DWORD iIndex) PURE;
    STDMETHOD(ChapRemoveAll)() PURE;
    STDMETHOD_(long, ChapLookup)(REFERENCE_TIME* prt, BSTR* ppName) PURE;
    STDMETHOD(ChapSort)() PURE;
};

class CDSMChapter
{
    static int counter;
    int order;

public:
    REFERENCE_TIME rt;
    CStringW name;
    CDSMChapter();
    CDSMChapter(REFERENCE_TIME rt, LPCWSTR name);
    CDSMChapter& operator = (const CDSMChapter& c);

    bool operator <(const CDSMChapter& rhs) const {
        return (rt != rhs.rt) ? rt < rhs.rt : order < rhs.order;
    }
};

class IDSMChapterBagImpl : public IDSMChapterBag
{
protected:
    CAtlArray<CDSMChapter> m_chapters;
    bool m_fSorted;

public:
    IDSMChapterBagImpl();

    void operator += (const CDSMChapter& c) { m_chapters.Add(c); m_fSorted = false; }

    // IDSMChapterBag

    STDMETHODIMP_(DWORD) ChapGetCount();
    STDMETHODIMP ChapGet(DWORD iIndex, REFERENCE_TIME* prt, BSTR* ppName = nullptr);
    STDMETHODIMP ChapSet(DWORD iIndex, REFERENCE_TIME rt, LPCWSTR pName);
    STDMETHODIMP ChapAppend(REFERENCE_TIME rt, LPCWSTR pName);
    STDMETHODIMP ChapRemoveAt(DWORD iIndex);
    STDMETHODIMP ChapRemoveAll();
    STDMETHODIMP_(long) ChapLookup(REFERENCE_TIME* prt, BSTR* ppName = nullptr);
    STDMETHODIMP ChapSort();
};

class CDSMChapterBag : public CUnknown, public IDSMChapterBagImpl
{
public:
    CDSMChapterBag(LPUNKNOWN pUnk, HRESULT* phr);

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
};

template<class T>
__declspec(nothrow noalias) __forceinline size_t range_bsearch(CAtlArray<T> const& tArray, REFERENCE_TIME rt)
{
    // MAXSIZE_T is returned by this function for status invalid
    ptrdiff_t k = tArray.GetCount() - 1;
    if ((k < 0) || (rt >= tArray[k].rt)) {
        return k;
    }
    size_t ret = MAXSIZE_T;
    if (!k) {
        return ret;
    }

    size_t i = 0, j = k;
    do {
        size_t mid = (i + j) >> 1;
        REFERENCE_TIME midrt = tArray[mid].rt;
        if (rt == midrt) {
            ret = mid;
            break;
        } else if (rt < midrt) {
            ret = MAXSIZE_T;
            if (j == mid) {
                --mid;
            }
            j = mid;
        } else if (rt > midrt) {
            ret = mid;
            if (i == mid) {
                ++mid;
            }
            i = mid;
        }
    } while (i < j);
    return ret;
}
