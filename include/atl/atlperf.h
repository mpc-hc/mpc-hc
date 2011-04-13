// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLPERF_H__
#define __ATLPERF_H__

#pragma once

#ifdef _WIN32_WCE
	#error atlpref.h is not supported on Windows CE (_WIN32_WCE is defined)
#endif //_WIN32_WCE

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <atlfile.h>
#include <atlsync.h>
#include <winperf.h>
#include <atlcoll.h>
#include <atlsecurity.h>

#ifndef _ATL_PERF_NOXML
#include <atlenc.h>
#include <oaidl.h>
#include <xmldomdid.h>

/* xmldsodid and mshtmdid both have the same identifiers defined, with differing values. So we are renaming the XML ones since there are less identifiers dependent on those.  */
#ifdef DISPID_XOBJ_MIN
/* in case the HTM one was included first, we'll undef these first */
#define _ATL_MSHTMDID_INCLUDED_ALREADY
#undef DISPID_XOBJ_MIN
#undef DISPID_XOBJ_MAX
#undef DISPID_XOBJ_BASE
#endif

#include <xmldsodid.h>
#include <msxmldid.h>

/* re-undef the clashing names, and their direct dependents */
#undef DISPID_XOBJ_MIN
#undef DISPID_XOBJ_MAX
#undef DISPID_XOBJ_BASE
#undef DISPID_XMLDSO
#undef DISPID_XMLELEMENTCOLLECTION

/* re-def the clashing names, and their direct dependents, with un-clashed values */
#define DISPID_XMLDSO_XOBJ_MIN                 0x00010000
#define DISPID_XMLDSO_XOBJ_MAX                 0x0001FFFF
#define DISPID_XMLDSO_XOBJ_BASE                DISPID_XMLDSO_XOBJ_MIN
#define DISPID_XMLDSO			               DISPID_XMLDSO_XOBJ_BASE
#define DISPID_XMLELEMENTCOLLECTION            DISPID_XMLDSO_XOBJ_BASE
#ifdef _ATL_MSHTMDID_INCLUDED_ALREADY

/* redef the clashing names to the HTM values */
#define DISPID_XOBJ_MIN                 0x80010000
#define DISPID_XOBJ_MAX                 0x8001FFFF
#define DISPID_XOBJ_BASE                DISPID_XOBJ_MIN
#endif
#include <msxml.h>
#endif

#pragma warning(push)
#pragma warning(disable: 4625) // copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning(disable: 4626) // assignment operator could not be generated because a base class assignment operator is inaccessible


#pragma pack(push,_ATL_PACKING)
namespace ATL
{

const DWORD ATLPERF_SIZE_MASK = 0x00000300;
const DWORD ATLPERF_TYPE_MASK = 0x00000C00;
const DWORD ATLPERF_TEXT_MASK = 0x00010000;

#ifndef ATLPERF_DEFAULT_MAXINSTNAMELENGTH
#define ATLPERF_DEFAULT_MAXINSTNAMELENGTH 64
#endif

// base class for user-defined perf objects
struct CPerfObject
{
	// implementation

	ULONG m_nAllocSize;
	DWORD m_dwCategoryId;
	DWORD m_dwInstance;
	ULONG m_nRefCount;
	ULONG m_nInstanceNameOffset; // byte offset from beginning of PerfObject to LPWSTR szInstanceName
};

class CPerfMon
{
public:
	virtual ~CPerfMon() throw();

#ifdef _ATL_PERF_REGISTER
	// registration
	HRESULT Register(
		LPCTSTR szOpenFunc,
		LPCTSTR szCollectFunc,
		LPCTSTR szCloseFunc,
		HINSTANCE hDllInstance = _AtlBaseModule.GetModuleInstance()) throw();
	HRESULT RegisterStrings(
		LANGID wLanguage = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		HINSTANCE hResInstance = _AtlBaseModule.GetResourceInstance()) throw();
	HRESULT RegisterAllStrings(HINSTANCE hResInstance = NULL) throw();
	HRESULT Unregister() throw();
#endif

	HRESULT Initialize() throw();
	void UnInitialize() throw();
	HRESULT CreateInstance(
		DWORD dwCategoryId,
		DWORD dwInstance,
		LPCWSTR szInstanceName,
		CPerfObject** ppInstance) throw();
	HRESULT CreateInstanceByName(
		DWORD dwCategoryId,
		LPCWSTR szInstanceName,
		CPerfObject** ppInstance) throw();

	template <class T>
	HRESULT CreateInstance(
		DWORD dwInstance,
		LPCWSTR szInstanceName,
		T** ppInstance) throw()
	{
		// Ensure T derives from CPerfObject
		static_cast<CPerfObject*>(*ppInstance);
		
		return CreateInstance(
			T::kCategoryId,
			dwInstance,
			szInstanceName,
			reinterpret_cast<CPerfObject**>(ppInstance)
			);
	}

	template <class T>
	HRESULT CreateInstanceByName(
		LPCWSTR szInstanceName,
		T** ppInstance) throw()
	{
		// Ensure T derives from CPerfObject
		static_cast<CPerfObject*>(*ppInstance);
		
		return CreateInstanceByName(
			T::kCategoryId,
			szInstanceName,
			reinterpret_cast<CPerfObject**>(ppInstance)
			);
	}

	HRESULT ReleaseInstance(CPerfObject* pInstance) throw();
	HRESULT LockPerf(DWORD dwTimeout = INFINITE) throw();
	void UnlockPerf() throw();

#ifndef _ATL_PERF_NOXML
	HRESULT PersistToXML(IStream *pStream, BOOL bFirst=TRUE, BOOL bLast=TRUE) throw(...);
	HRESULT LoadFromXML(IStream *pStream) throw(...);
#endif

	// implementation

public:
	// PerfMon entry point helpers
	DWORD Open(LPWSTR lpDeviceNames) throw();
	DWORD Collect(__in_z LPWSTR lpwszValue, __deref_inout_bcount(*pcbBytes) LPVOID* lppData, __inout LPDWORD lpcbBytes, __inout LPDWORD lpcObjectTypes) throw();
	DWORD Close() throw();

	// map building routines
	HRESULT AddCategoryDefinition(
		DWORD dwCategoryId,
		LPCTSTR szCategoryName,
		LPCTSTR szHelpString,
		DWORD dwDetailLevel,
		INT nDefaultCounter,
		BOOL bInstanceLess,
		UINT nStructSize,
		UINT nMaxInstanceNameLen = ATLPERF_DEFAULT_MAXINSTNAMELENGTH) throw();
	HRESULT AddCounterDefinition(	
		DWORD dwCounterId,
		LPCTSTR szCounterName,
		LPCTSTR szHelpString,
		DWORD dwDetailLevel,
		DWORD dwCounterType,
		ULONG nMaxCounterSize,
		UINT nOffset,
		INT nDefaultScale) throw();

	// macro helpers
	HRESULT RegisterCategory(
		WORD wLanguage,
		HINSTANCE hResInstance,
		UINT* pSampleRes,
		DWORD dwCategoryId,
		UINT nNameString,
		UINT nHelpString,
		DWORD dwDetail,
		BOOL bInstanceless,
		UINT nStructSize,
		UINT nMaxInstanceNameLen,
		INT nDefaultCounter) throw();
	HRESULT RegisterCategory(
		WORD wLanguage,
		HINSTANCE hResInstance,
		UINT* pSampleRes,
		DWORD dwCategoryId,
		LPCTSTR szNameString,
		LPCTSTR szHelpString,
		DWORD dwDetail,
		BOOL bInstanceless,
		UINT nStructSize,
		UINT nMaxInstanceNameLen,
		INT nDefaultCounter) throw();
	HRESULT RegisterCounter(
		WORD wLanguage,
		HINSTANCE hResInstance,
		DWORD dwCounterId,
		UINT nNameString,
		UINT nHelpString,
		DWORD dwDetail,
		DWORD dwCounterType,
		ULONG nMaxCounterSize,
		UINT nOffset,
		INT nDefaultScale) throw();
	HRESULT RegisterCounter(
		WORD wLanguage,
		HINSTANCE hResInstance,
		DWORD dwCounterId,
		LPCTSTR szNameString,
		LPCTSTR szHelpString,
		DWORD dwDetail,
		DWORD dwCounterType,
		ULONG nMaxCounterSize,
		UINT nOffset,
		INT nDefaultScale) throw();

protected:
	void ClearMap() throw();

	virtual LPCTSTR GetAppName() const throw() = 0;
	virtual HRESULT CreateMap(WORD wLanguage, HINSTANCE hResInstance, UINT* pSampleRes = NULL) throw();
	virtual void OnBlockAlloc(CAtlFileMappingBase* /*pNewBlock*/) { }
#ifdef _ATL_PERF_REGISTER
	static BOOL CALLBACK EnumResLangProc(HINSTANCE hModule, LPCTSTR szType, LPCTSTR szName, LANGID wIDLanguage, LPARAM lParam);
#endif

	// implementation helpers
	struct CounterInfo
	{
		CounterInfo() : m_dwCounterId(0), m_dwDetailLevel(0), m_nNameId(0),
				m_nHelpId(0), m_dwCounterType(0), m_nDefaultScale(0),
				m_nMaxCounterSize(0), m_nDataOffset(0)
		{
			ZeroMemory (&m_cache, sizeof(m_cache));
		}
		// implementation

		DWORD m_dwCounterId;
		CString m_strName;
		CString m_strHelp;
		DWORD m_dwDetailLevel;

		// the ids that correspond to the name and help strings stored in the registry
		UINT m_nNameId;
		UINT m_nHelpId;

		// counter data

		DWORD m_dwCounterType;
		LONG m_nDefaultScale;

		// the maximum size of the string counter data in characters, including the null terminator
		// ignored if not a string counter
		ULONG m_nMaxCounterSize;

		ULONG m_nDataOffset;

		// cached data to be copied into request
		PERF_COUNTER_DEFINITION m_cache;
	};

	struct CategoryInfo
	{
		// implementation

		DWORD m_dwCategoryId;
		CString m_strName;
		CString m_strHelp;
		DWORD m_dwDetailLevel;

		// the ids that correspond to the name and help strings stored in the registry
		UINT m_nNameId;
		UINT m_nHelpId;

		// category data

		LONG m_nDefaultCounter;
		LONG m_nInstanceLess; // PERF_NO_INSTANCES if instanceless

		// the size of the struct not counting the name and string counters
		ULONG m_nStructSize;

		// in characters including the null terminator
		ULONG m_nMaxInstanceNameLen;

		ULONG m_nAllocSize;

		// cached data to be copied into request
		PERF_OBJECT_TYPE m_cache;
		ULONG m_nCounterBlockSize;

		// counters
		UINT _GetNumCounters() throw();
		CounterInfo* _GetCounterInfo(UINT nIndex) throw();

		CAtlArray<CounterInfo> m_counters;
	};

	LPBYTE _AllocData(LPBYTE& pData, ULONG nBytesAvail, ULONG* pnBytesUsed, size_t nBytesNeeded) throw();
	template<typename T> T* _AllocStruct(LPBYTE& pData, ULONG nBytesAvail, ULONG* pnBytesUsed, T*) throw()
	{
		return reinterpret_cast<T*>(_AllocData(pData, nBytesAvail, pnBytesUsed, sizeof(T)));
	}

	UINT _GetNumCategoriesAndCounters() throw();
	CategoryInfo* _GetCategoryInfo(UINT nIndex) throw();
	UINT _GetNumCategories() throw();
	CPerfObject* _GetFirstInstance(CAtlFileMappingBase* pBlock) throw();
	CPerfObject* _GetNextInstance(CPerfObject* pInstance) throw();
	CAtlFileMappingBase* _GetNextBlock(CAtlFileMappingBase* pBlock) throw();
	CAtlFileMappingBase* _OpenNextBlock(CAtlFileMappingBase* pPrev) throw();
	CAtlFileMappingBase* _AllocNewBlock(CAtlFileMappingBase* pPrev, BOOL* pbExisted = NULL) throw();
	HRESULT _OpenAllBlocks() throw();
	DWORD& _GetBlockId(CAtlFileMappingBase* pBlock) throw(...);
	DWORD* _GetBlockId_NoThrow(CAtlFileMappingBase* pBlock) throw();
	CategoryInfo* _FindCategoryInfo(DWORD dwCategoryId) throw();
	CounterInfo* _FindCounterInfo(CategoryInfo* pCategoryInfo, DWORD dwCounterId) throw();
	CounterInfo* _FindCounterInfo(DWORD dwCategoryId, DWORD dwCounterId) throw();
	BOOL _WantCategoryType(__in_z LPWSTR lpwszValue, __in DWORD dwPerfId) throw(...);
	void _FillCategoryType(CategoryInfo* pCategoryInfo) throw();
	void _FillCounterDef(CounterInfo* pCounterInfo, ULONG* pnCounterBlockSize) throw();
	HRESULT CPerfMon::_CollectInstance(
		CategoryInfo* pCategoryInfo,
		LPBYTE& pData,
		ULONG nBytesAvail,
		ULONG* pnBytesUsed,
		CPerfObject* pInstance,
		PERF_OBJECT_TYPE* pObjectType,
		PERF_COUNTER_DEFINITION* pCounterDefs
		) throw();
	HRESULT _CollectInstance(
		CategoryInfo* pCategoryInfo,
		LPBYTE& pData,
		ULONG nBytesAvail,
		ULONG* pnBytesUsed,
		PERF_OBJECT_TYPE* pObjectType,
		PERF_COUNTER_DEFINITION* pCounterDefs
		) throw();
	HRESULT _CollectCategoryType(
		CategoryInfo* pCategoryInfo,
		LPBYTE pData,
		ULONG nBytesAvail,
		ULONG* pnBytesUsed) throw();
	HRESULT _LoadMap(DWORD* pData) throw();
	HRESULT _SaveMap() throw();
	HRESULT _GetAttribute(
		IXMLDOMNode *pNode, 
		LPCWSTR szAttrName, 
		BSTR *pbstrVal) throw();
	HRESULT CPerfMon::_CreateInstance(
		DWORD dwCategoryId,
		DWORD dwInstance,
		LPCWSTR szInstanceName,
		CPerfObject** ppInstance,
		bool bByName) throw();

#ifdef _ATL_PERF_REGISTER
	void _AppendStrings(
		LPTSTR& pszNew,
		CAtlArray<CString>& astrStrings,
		ULONG iFirstIndex
		) throw();
	HRESULT _AppendRegStrings(
		CRegKey& rkLang,
		LPCTSTR szValue,
		CAtlArray<CString>& astrStrings,
		ULONG nNewStringSize,
		ULONG iFirstIndex,
		ULONG iLastIndex) throw();
	HRESULT _RemoveRegStrings(
		CRegKey& rkLang,
		LPCTSTR szValue,
		ULONG iFirstIndex,
		ULONG iLastIndex) throw();
	HRESULT _ReserveStringRange(DWORD& dwFirstCounter, DWORD& dwFirstHelp) throw();
	HRESULT _UnregisterStrings() throw();
	HRESULT _RegisterAllStrings(UINT nRes, HINSTANCE hResInstance) throw();
#endif
private:
	CAtlArray<CategoryInfo> m_categories;
	CAutoPtrArray<CAtlFileMappingBase> m_aMem;
	CMutex m_lock;
	ULONG m_nAllocSize;
	ULONG m_nHeaderSize;
	ULONG m_nSchemaSize;
	CSecurityDesc m_sd;
};

class CPerfLock
{
public:
	CPerfLock(CPerfMon* pPerfMon, DWORD dwTimeout = INFINITE)
	{
		ATLENSURE(pPerfMon != NULL);
		m_pPerfMon = pPerfMon;
		m_hrStatus = m_pPerfMon->LockPerf(dwTimeout);
	}

	~CPerfLock() throw()
	{
		if (SUCCEEDED(m_hrStatus))
			m_pPerfMon->UnlockPerf();
	}

	HRESULT GetStatus() const throw()
	{
		return m_hrStatus;
	}

private:
	CPerfMon* m_pPerfMon;
	HRESULT m_hrStatus;
};

////////////////////////////////////////////////////////////////////////
// map macros

// empty definition just for ease of use with code wizards, etc.
#define BEGIN_PERFREG_MAP()

// empty definition just for ease of use with code wizards, etc.
#define END_PERFREG_MAP()

#if !defined(_ATL_PERF_REGISTER) | defined(_ATL_PERF_NOEXPORT)
#define PERFREG_ENTRY(className)
#endif

#ifdef _ATL_PERF_REGISTER
#define BEGIN_PERF_MAP(AppName) \
	private: \
		LPCTSTR GetAppName() const throw() { return AppName; } \
		HRESULT CreateMap(WORD wLanguage, HINSTANCE hResInstance, UINT* pSampleRes = NULL) throw() \
		{ \
			if (pSampleRes) \
				*pSampleRes = 0; \
			ClearMap();

#define BEGIN_COUNTER_MAP(categoryclass) \
	public: \
		typedef categoryclass _PerfCounterClass; \
		static HRESULT CreateMap(CPerfMon* pPerf, WORD wLanguage, HINSTANCE hResInstance, UINT* pSampleRes) throw() \
		{ \
			HRESULT hr = RegisterCategory(pPerf, wLanguage, hResInstance, pSampleRes); \
			if (FAILED(hr)) \
				return hr;

#define DECLARE_PERF_CATEGORY_EX(dwCategoryId, namestring, helpstring, detail, instanceless, structsize, maxinstnamelen, defcounter) \
		static HRESULT RegisterCategory(CPerfMon* pPerf, WORD wLanguage, HINSTANCE hResInstance, UINT* pSampleRes) throw() \
		{ \
			return pPerf->RegisterCategory(wLanguage, hResInstance, pSampleRes, dwCategoryId, namestring, helpstring, detail, instanceless, structsize, maxinstnamelen, defcounter); \
		} \
		/* NOTE: put a semicolon after your call to DECLARE_PERF_CATEGORY*(...) */ \
		/* this is needed for the code wizards to parse things properly */ \
		static const DWORD kCategoryId = dwCategoryId

#define CHAIN_PERF_CATEGORY(categoryclass) \
			if (FAILED(categoryclass::CreateMap(this, wLanguage, hResInstance, pSampleRes))) \
				return E_FAIL;

// CAssertValidField ensures that the member variable that's being passed to
// DEFINE_COUNTER[_EX] is the proper type. only 32-bit integral types can be used with
// PERF_SIZE_DWORD and only 64-bit integral types can be used with PERF_SIZE_LARGE
template< DWORD t_dwSize >
class CAssertValidField
{
};

template<>
class CAssertValidField< PERF_SIZE_DWORD >
{
public:
	template< class C > static void AssertValidFieldType( ULONG C::* ) throw() { }
	template< class C > static void AssertValidFieldType( LONG C::* ) throw() { }
};

template<>
class CAssertValidField< PERF_SIZE_LARGE >
{
public:
	template< class C > static void AssertValidFieldType( ULONGLONG C::* ) throw() { }
	template< class C > static void AssertValidFieldType( LONGLONG C::* ) throw() { }
};

#define DEFINE_COUNTER_EX(member, dwCounterId, namestring, helpstring, detail, countertype, maxcountersize, defscale) \
			CAssertValidField< (countertype) & ATLPERF_SIZE_MASK >::AssertValidFieldType( &_PerfCounterClass::member ); \
			hr = pPerf->RegisterCounter(wLanguage, hResInstance, dwCounterId, namestring, helpstring, detail, countertype, maxcountersize, (ULONG) offsetof(_PerfCounterClass, member), defscale); \
			if (FAILED(hr)) \
				return hr;

#define END_PERF_MAP() \
			return S_OK; \
		}

#define END_COUNTER_MAP() \
			return S_OK; \
		}

#else // _ATL_PERF_REGISTER

#define BEGIN_PERF_MAP(AppName) \
	private: \
		LPCTSTR GetAppName() const throw() { return AppName; }

#define BEGIN_COUNTER_MAP(objectclass)

#define DECLARE_PERF_CATEGORY_EX(dwCategoryId, namestring, helpstring, detail, instanceless, structsize, maxinstnamelen, defcounter) \
		/* NOTE: put a semicolon after your call to DECLARE_PERF_CATEGORY*(...) */ \
		/* this is needed for the code wizards to parse things properly */ \
		static const DWORD kCategoryId = dwCategoryId

#define CHAIN_PERF_CATEGORY(objectclass)
#define DEFINE_COUNTER_EX(member, dwCounterId, namestring, helpstring, detail, countertype, maxcountersize, defscale)

#define END_PERF_MAP()
#define END_COUNTER_MAP()

#endif // _ATL_PERF_REGISTER

#define DECLARE_PERF_CATEGORY(objectclass, dwCategoryId, namestring, helpstring, defcounter) \
	DECLARE_PERF_CATEGORY_EX(dwCategoryId, namestring, helpstring, PERF_DETAIL_NOVICE, 0, sizeof(objectclass), ATLPERF_DEFAULT_MAXINSTNAMELENGTH, defcounter)
#define DECLARE_PERF_CATEGORY_NO_INSTANCES(objectclass, dwCategoryId, namestring, helpstring, defcounter) \
	DECLARE_PERF_CATEGORY_EX(dwCategoryId, namestring, helpstring, PERF_DETAIL_NOVICE, PERF_NO_INSTANCES, sizeof(objectclass), 0, defcounter)

#define DEFINE_COUNTER(member, namestring, helpstring, countertype, defscale) \
	DEFINE_COUNTER_EX(member, 0, namestring, helpstring, PERF_DETAIL_NOVICE, countertype, 0, defscale)

#pragma deprecated( DECLARE_PERF_OBJECT_EX )
#pragma deprecated( DECLARE_PERF_OBJECT )
#pragma deprecated( DECLARE_PERF_OBJECT_NO_INSTANCES )
#pragma deprecated( CHAIN_PERF_OBJECT )
#define DECLARE_PERF_OBJECT_EX DECLARE_PERF_CATEGORY_EX
#define DECLARE_PERF_OBJECT DECLARE_PERF_CATEGORY
#define DECLARE_PERF_OBJECT_NO_INSTANCES DECLARE_PERF_CATEGORY_NO_INSTANCES
#define CHAIN_PERF_OBJECT CHAIN_PERF_CATEGORY

////////////////////////////////////////////////////////////////////////
// automagic registration stuff

#if defined(_ATL_PERF_REGISTER) & !defined(_ATL_PERF_NOEXPORT)

// define _ATL_PERF_NOEXPORT if you don't want to use the PERFREG map and don't want these
// functions exported from your DLL

// Perf register map stuff
// this is for ease of integration with the module attribute and for the 
// perfmon wizard

#pragma section("ATLP$A", read, shared)
#pragma section("ATLP$Z", read, shared)
#pragma section("ATLP$C", read, shared)
extern "C"
{
__declspec(selectany) __declspec(allocate("ATLP$A")) CPerfMon * __pperfA = NULL;
__declspec(selectany) __declspec(allocate("ATLP$Z")) CPerfMon * __pperfZ = NULL;
}

#if !defined(_M_IA64)
#pragma comment(linker, "/merge:ATLP=.rdata")
#endif

#if defined(_M_IA64) || defined(_M_AMD64)
	#define ATLPERF_FUNCID_OPEN    "OpenPerfMon"
	#define ATLPERF_FUNCID_COLLECT "CollectPerfMon"
	#define ATLPERF_FUNCID_CLOSE   "ClosePerfMon"
#elif defined(_M_IX86)
	#define ATLPERF_FUNCID_OPEN    "_OpenPerfMon@4"
	#define ATLPERF_FUNCID_COLLECT "_CollectPerfMon@16"
	#define ATLPERF_FUNCID_CLOSE   "_ClosePerfMon@0"
#else
#if !defined(ATLPERF_FUNCID_OPEN) || !defined(ATLPERF_FUNCID_COLLECT) || !defined (ATLPERF_FUNCID_CLOSE)
#error "Unknown platform. Define ATLPERF_FUNCID_OPEN, ATLPERF_FUNCID_COLLECT, ATLPERF_FUNCID_CLOSE"
#endif
#endif

HRESULT RegisterPerfMon(HINSTANCE hDllInstance = _AtlBaseModule.GetModuleInstance()) throw();
HRESULT UnregisterPerfMon() throw();

extern "C" DWORD __declspec(dllexport) WINAPI OpenPerfMon(LPWSTR lpDeviceNames) throw();
extern "C" DWORD __declspec(dllexport) WINAPI CollectPerfMon(LPWSTR lpwszValue, LPVOID* lppData,
	LPDWORD lpcbBytes, LPDWORD lpcObjectTypes) throw();
extern "C" DWORD __declspec(dllexport) WINAPI ClosePerfMon() throw();

// this class handles integrating the registration with CComModule
class _CAtlPerfSetFuncPtr
{
public:
	_CAtlPerfSetFuncPtr()
	{
		_pPerfRegFunc = RegisterPerfMon;
		_pPerfUnRegFunc = UnregisterPerfMon;
	}
};

extern "C" { __declspec(selectany) _CAtlPerfSetFuncPtr g_atlperfinit; }

#if defined(_M_IX86)
#pragma comment(linker, "/INCLUDE:_g_atlperfinit")
#elif defined(_M_IA64) || defined(_M_AMD64)
#pragma comment(linker, "/INCLUDE:g_atlperfinit")
#else
#pragma message("Unknown platform.  Make sure the linker includes g_atlperfinit")
#endif

#ifndef PERF_ENTRY_PRAGMA

#if defined(_M_IX86)
#define PERF_ENTRY_PRAGMA(class) __pragma(comment(linker, "/include:___pperf_" #class));
#elif defined(_M_IA64)
#define PERF_ENTRY_PRAGMA(class) __pragma(comment(linker, "/include:__pperf_" #class));
#elif defined(_M_AMD64)
#define PERF_ENTRY_PRAGMA(class) __pragma(comment(linker, "/include:__pperf_" #class));
#else
#error Unknown Platform. define PERF_ENTRY_PRAGMA
#endif

#endif // PERF_ENTRY_PRAGMA

#define PERFREG_ENTRY(className) \
	__declspec(selectany) className __perf_##className; \
	extern "C" __declspec(allocate("ATLP$C")) __declspec(selectany) CPerfMon * const __pperf_##className = \
		static_cast<CPerfMon*>(&__perf_##className); \
	PERF_ENTRY_PRAGMA(className)

#endif // _ATL_PERF_NOEXPORT

} // namespace ATL
 

#include <atlperf.inl>

#pragma pack(pop)
#pragma warning(pop)

#endif // __ATLPERF_H__
