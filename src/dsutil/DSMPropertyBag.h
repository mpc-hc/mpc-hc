#pragma once
#include <atlcoll.h>
#include <atlsimpcoll.h>

// IDSMPropertyBag

[uuid("232FD5D2-4954-41E7-BF9B-09E1257B1A95")]
interface IDSMPropertyBag : public IPropertyBag2
{
	STDMETHOD(SetProperty) (LPCWSTR key, LPCWSTR value) = 0;
	STDMETHOD(SetProperty) (LPCWSTR key, VARIANT* var) = 0;
	STDMETHOD(GetProperty) (LPCWSTR key, BSTR* value) = 0;
	STDMETHOD(DelAllProperties) () = 0;
	STDMETHOD(DelProperty) (LPCWSTR key) = 0;
};

class IDSMPropertyBagImpl : public ATL::CSimpleMap<CStringW, CStringW>, public IDSMPropertyBag, public IPropertyBag
{
	BOOL Add(const CStringW& key, const CStringW& val) {return __super::Add(key, val);}
	BOOL SetAt(const CStringW& key, const CStringW& val) {return __super::SetAt(key, val);}

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

[uuid("EBAFBCBE-BDE0-489A-9789-05D5692E3A93")]
interface IDSMResourceBag : public IUnknown
{
	STDMETHOD_(DWORD, ResGetCount) () = 0;
	STDMETHOD(ResGet) (DWORD iIndex, BSTR* ppName, BSTR* ppDesc, BSTR* ppMime, BYTE** ppData, DWORD* pDataLen, DWORD_PTR* pTag) = 0;
	STDMETHOD(ResSet) (DWORD iIndex, LPCWSTR pName, LPCWSTR pDesc, LPCWSTR pMime, BYTE* pData, DWORD len, DWORD_PTR tag) = 0;
	STDMETHOD(ResAppend) (LPCWSTR pName, LPCWSTR pDesc, LPCWSTR pMime, BYTE* pData, DWORD len, DWORD_PTR tag) = 0;
	STDMETHOD(ResRemoveAt) (DWORD iIndex) = 0;
	STDMETHOD(ResRemoveAll) (DWORD_PTR tag) = 0;
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
	void operator = (const CDSMResource& r);

	// global access to all resources
	static CCritSec m_csResources;
	static CAtlMap<DWORD, CDSMResource*> m_resources;
};

class IDSMResourceBagImpl : public IDSMResourceBag
{
protected:
	CAtlArray<CDSMResource> m_resources;

public:
	IDSMResourceBagImpl();

	void operator += (const CDSMResource& r) {m_resources.Add(r);}

	// IDSMResourceBag

	STDMETHODIMP_(DWORD) ResGetCount();
	STDMETHODIMP ResGet(DWORD iIndex, BSTR* ppName, BSTR* ppDesc, BSTR* ppMime, BYTE** ppData, DWORD* pDataLen, DWORD_PTR* pTag = NULL);
	STDMETHODIMP ResSet(DWORD iIndex, LPCWSTR pName, LPCWSTR pDesc, LPCWSTR pMime, BYTE* pData, DWORD len, DWORD_PTR tag = 0);
	STDMETHODIMP ResAppend(LPCWSTR pName, LPCWSTR pDesc, LPCWSTR pMime, BYTE* pData, DWORD len, DWORD_PTR tag = 0);
	STDMETHODIMP ResRemoveAt(DWORD iIndex);
	STDMETHODIMP ResRemoveAll(DWORD_PTR tag = 0);
};

// IDSMChapterBag

[uuid("2D0EBE73-BA82-4E90-859B-C7C48ED3650F")]
interface IDSMChapterBag : public IUnknown
{
	STDMETHOD_(DWORD, ChapGetCount) () = 0;
	STDMETHOD(ChapGet) (DWORD iIndex, REFERENCE_TIME* prt, BSTR* ppName) = 0;
	STDMETHOD(ChapSet) (DWORD iIndex, REFERENCE_TIME rt, LPCWSTR pName) = 0;
	STDMETHOD(ChapAppend) (REFERENCE_TIME rt, LPCWSTR pName) = 0;
	STDMETHOD(ChapRemoveAt) (DWORD iIndex) = 0;
	STDMETHOD(ChapRemoveAll) () = 0;
	STDMETHOD_(long, ChapLookup) (REFERENCE_TIME* prt, BSTR* ppName) = 0;
	STDMETHOD(ChapSort) () = 0;
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
	void operator = (const CDSMChapter& c);
	static int Compare(const void* a, const void* b);
};

class IDSMChapterBagImpl : public IDSMChapterBag
{
protected:
	CAtlArray<CDSMChapter> m_chapters;
	bool m_fSorted;

public:
	IDSMChapterBagImpl();

	void operator += (const CDSMChapter& c) {m_chapters.Add(c); m_fSorted = false;}

	// IDSMChapterBag

	STDMETHODIMP_(DWORD) ChapGetCount();
	STDMETHODIMP ChapGet(DWORD iIndex, REFERENCE_TIME* prt, BSTR* ppName = NULL);
	STDMETHODIMP ChapSet(DWORD iIndex, REFERENCE_TIME rt, LPCWSTR pName);
	STDMETHODIMP ChapAppend(REFERENCE_TIME rt, LPCWSTR pName);
	STDMETHODIMP ChapRemoveAt(DWORD iIndex);
	STDMETHODIMP ChapRemoveAll();
	STDMETHODIMP_(long) ChapLookup(REFERENCE_TIME* prt, BSTR* ppName = NULL);
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
int range_bsearch(const CAtlArray<T>& array, REFERENCE_TIME rt)
{
	int i = 0, j = array.GetCount() - 1, ret = -1;
	if(j >= 0 && rt >= array[j].rt) return j;
	while(i < j)
	{
		int mid = (i + j) >> 1;
		REFERENCE_TIME midrt = array[mid].rt;
		if(rt == midrt) {ret = mid; break;}
		else if(rt < midrt) {ret = -1; if(j == mid) mid--; j = mid;}
		else if(rt > midrt) {ret = mid; if(i == mid) mid++; i = mid;}
	}
	return ret;
}
