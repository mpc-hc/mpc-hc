// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLPERF_INL__
#define __ATLPERF_INL__

#pragma once
 
#ifndef __ATLPERF_H__
	#error atlperf.inl requires atlperf.h to be included first
#endif

#pragma warning(push)

#ifndef _CPPUNWIND
#pragma warning(disable: 4702) // unreachable code
#endif

namespace ATL
{

extern __declspec(selectany) const TCHAR * const c_szAtlPerfCounter = _T("Counter");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfFirstCounter = _T("First Counter");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfLastCounter = _T("Last Counter");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfHelp = _T("Help");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfFirstHelp = _T("First Help");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfLastHelp = _T("Last Help");

extern __declspec(selectany) const WCHAR * const c_szAtlPerfGlobal = L"Global";
extern __declspec(selectany) const TCHAR * const c_szAtlPerfLibrary = _T("Library");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfOpen = _T("Open");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfCollect = _T("Collect");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfClose = _T("Close");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfLanguages = _T("Languages");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfMap = _T("Map");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfPerformance = _T("Performance");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfServicesKey = _T("SYSTEM\\CurrentControlSet\\Services\\%s");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfPerformanceKey = _T("SYSTEM\\CurrentControlSet\\Services\\%s\\Performance");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfPerfLibKey = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");
extern __declspec(selectany) const TCHAR * const c_szAtlPerfPerfLibLangKey = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\%3.3x");

inline CPerfMon::CounterInfo* CPerfMon::CategoryInfo::_GetCounterInfo(UINT nIndex) throw()
{
	ATLASSERT(nIndex < _GetNumCounters());
	return &m_counters[nIndex];
}

inline UINT CPerfMon::CategoryInfo::_GetNumCounters() throw()
{
	return (UINT) m_counters.GetCount();
}

inline CPerfMon::~CPerfMon() throw()
{
	UnInitialize();
}

inline HRESULT CPerfMon::CreateMap(LANGID language, HINSTANCE hResInstance, UINT* pSampleRes) throw()
{
	(language); // unused
	(hResInstance); // unused
	(pSampleRes); // unused
	return S_OK;
}

inline UINT CPerfMon::_GetNumCategoriesAndCounters() throw()
{
	UINT nResult = _GetNumCategories();
	for (UINT i=0; i<_GetNumCategories(); i++)
	{
		nResult += _GetCategoryInfo(i)->_GetNumCounters();
	}

	return nResult;
}

inline CPerfMon::CategoryInfo* CPerfMon::_GetCategoryInfo(UINT nIndex) throw()
{
	ATLASSERT(nIndex < _GetNumCategories());
	return &m_categories[nIndex];
}

inline UINT CPerfMon::_GetNumCategories() throw()
{
	return (UINT) m_categories.GetCount();
}

inline CPerfObject* CPerfMon::_GetFirstInstance(CAtlFileMappingBase* pBlock)
{
	ATLENSURE(pBlock != NULL);

	// should never happen if Initialize succeeded
	// are you checking return codes?
	ATLASSERT(pBlock->GetData() != NULL);

	return reinterpret_cast<CPerfObject*>(LPBYTE(pBlock->GetData()) + m_nHeaderSize);
}

inline CPerfObject* CPerfMon::_GetNextInstance(CPerfObject* pInstance)
{
	ATLENSURE_RETURN_VAL(pInstance != NULL, NULL);
	ATLENSURE_RETURN_VAL(pInstance->m_nAllocSize != (ULONG)-1, NULL);
	ATLASSERT(pInstance->m_nAllocSize != (ULONG)0);

	return reinterpret_cast<CPerfObject*>(LPBYTE(pInstance) + pInstance->m_nAllocSize);
}

inline CAtlFileMappingBase* CPerfMon::_GetNextBlock(CAtlFileMappingBase* pBlock) throw()
{
	// calling _GetNextBlock(NULL) will return the first block
	DWORD dwNextBlockIndex = 0;
	DWORD* pDw= _GetBlockId_NoThrow(pBlock);
	if (pDw)
	{
		dwNextBlockIndex = *pDw +1;
	}
	if (m_aMem.GetCount() == dwNextBlockIndex)
		return NULL;
	return m_aMem[dwNextBlockIndex];
}

inline CAtlFileMappingBase* CPerfMon::_OpenNextBlock(CAtlFileMappingBase* pPrev) throw()
{
	CAutoPtr<CAtlFileMappingBase> spMem;
	CAtlFileMappingBase* pMem = NULL;
	ATLTRY(spMem.Attach(new CAtlFileMappingBase));
	if (spMem == NULL)
		return NULL;

	// create a unique name for the shared mem segment based on the index
	DWORD dwNextBlockIndex;
	DWORD* pDw= _GetBlockId_NoThrow(pPrev);
	if (pDw)
	{
		dwNextBlockIndex = *pDw +1;
	}
	else
	{
		// use the system allocation granularity (65536 currently. may be different in the future)
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		m_nAllocSize = si.dwAllocationGranularity;

		dwNextBlockIndex = 0;
	}

	_ATLTRY
	{
		CString strName;
		strName.Format(_T("Global\\ATLPERF_%s_%3.3d"), GetAppName(), dwNextBlockIndex);

		HRESULT hr = spMem->OpenMapping(strName, m_nAllocSize, 0, FILE_MAP_READ);
		if (FAILED(hr))
			return NULL;

		pMem = spMem;
		m_aMem.Add(spMem);
	}
	_ATLCATCHALL()
	{
		return NULL;
	}

	return pMem;
}

inline CAtlFileMappingBase* CPerfMon::_AllocNewBlock(CAtlFileMappingBase* pPrev, BOOL* pbExisted /* == NULL */) throw()
{
	CAtlFileMappingBase* pMem = NULL;
	_ATLTRY
	{
		CSecurityAttributes sa;
		sa.Set(m_sd);

		CAutoPtr<CAtlFileMappingBase> spMem;
		spMem.Attach(new CAtlFileMappingBase);
		if (spMem == NULL)
		{
			return NULL;
		}

		// create a unique name for the shared mem segment based on the index
		DWORD dwNextBlockIndex;
		if (pPrev != NULL)
		{
			dwNextBlockIndex = _GetBlockId(pPrev) +1;
		}
		else
		{
			// use the system allocation granularity (65536 currently. may be different in the future)
			SYSTEM_INFO si;
			GetSystemInfo(&si);
			m_nAllocSize = si.dwAllocationGranularity;

			dwNextBlockIndex = 0;
		}

		BOOL bExisted = FALSE;
		CString strName;
		strName.Format(_T("Global\\ATLPERF_%s_%3.3d"), GetAppName(), dwNextBlockIndex);

		HRESULT hr = spMem->MapSharedMem(m_nAllocSize, strName, &bExisted, &sa);
		if (FAILED(hr))
		{
			return NULL;
		}

		if(!bExisted)
		{
			memset(spMem->GetData(), 0, m_nAllocSize);
			// save the index of this block
			// don't for first block since we don't know m_nSchemaSize yet
			if (dwNextBlockIndex)
			{
				_GetBlockId(spMem) = dwNextBlockIndex;
			}
		}
		else
		{
			CSid owner;
			CDacl dacl;

			m_sd.GetOwner(&owner);
			m_sd.GetDacl(&dacl);

			// prevent us from using an object someone else has opened
			if (::SetSecurityInfo(spMem->GetHandle(), SE_KERNEL_OBJECT,
					DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
					const_cast<SID*>(owner.GetPSID()),
					NULL,
					const_cast<ACL*>(dacl.GetPACL()),
					NULL) != ERROR_SUCCESS)
			{
				return NULL;
			}
		}

		if (pbExisted)
		{
			*pbExisted = bExisted;
		}

		pMem = spMem;
		m_aMem.Add(spMem);

		OnBlockAlloc(pMem);
	}
	_ATLCATCHALL()
	{
		return NULL;
	}

	return pMem;
}

inline HRESULT CPerfMon::_OpenAllBlocks() throw()
{
	HRESULT hr;

	// if we haven't opened any yet, initialize
	if (m_aMem.GetCount() == 0)
	{
		CAtlFileMappingBase* pMem = _OpenNextBlock(NULL);
		if (pMem == NULL)
			return S_OK;

		hr = _LoadMap(LPDWORD(pMem->GetData()));
		if (FAILED(hr))
		{
			m_aMem.RemoveAll();
			return hr;
		}

		m_nSchemaSize = *LPDWORD(pMem->GetData());
		m_nHeaderSize = m_nSchemaSize + sizeof(DWORD);
		m_nHeaderSize  = AtlAlignUp(m_nHeaderSize,16);
	}

	// open any new blocks
	CAtlFileMappingBase* pMem = m_aMem[m_aMem.GetCount()-1];
	while (pMem)
		pMem = _OpenNextBlock(pMem);

	return S_OK;
}

inline HRESULT CPerfMon::_LoadMap(DWORD* pData) throw()
{
	_ATLTRY
	{
		HRESULT hr;

		ClearMap();

		DWORD dwDataSize = *pData++; // blob size
		DWORD dwNumItems = *pData++; // number of items

		// see if we have name data
		DWORD* pNameData = NULL;
		if (dwDataSize > (2+dwNumItems*9) * sizeof(DWORD))
			pNameData = pData + dwNumItems*9; // blob size and item count already skipped. skip item data

		for (DWORD i=0; i<dwNumItems; i++)
		{
			DWORD dwIsObject = *pData++;
			DWORD dwPerfId = *pData++;
			DWORD dwDetailLevel = *pData++;

			CString strName;
			if (pNameData)
			{
				strName = CString(LPWSTR(pNameData+1), *pNameData);
				pNameData += AtlAlignUp(sizeof(WCHAR) * *pNameData, sizeof(DWORD))/sizeof(DWORD) + 1;
			}

			if (dwIsObject)
			{
				DWORD dwDefaultCounter = *pData++;
				DWORD dwInstanceLess = *pData++;
				DWORD dwStructSize = *pData++;
				DWORD dwMaxInstanceNameLen = *pData++;

				hr = AddCategoryDefinition(
					dwPerfId,
					strName,
					NULL,
					dwDetailLevel,
					dwDefaultCounter,
					dwInstanceLess,
					dwStructSize,
					dwMaxInstanceNameLen);
				if (FAILED(hr))
				{
					ClearMap();
					return hr;
				}

				DWORD dwNameId = *pData++;
				DWORD dwHelpId = *pData++;
				CategoryInfo* pCategoryInfo = _GetCategoryInfo(_GetNumCategories()-1);
				pCategoryInfo->m_nNameId = dwNameId;
				pCategoryInfo->m_nHelpId = dwHelpId;
			}
			else
			{
				DWORD dwCounterType = *pData++;
				DWORD dwMaxCounterSize = *pData++;
				DWORD dwDataOffset = *pData++;
				DWORD dwDefaultScale = *pData++;

				hr = AddCounterDefinition(
					dwPerfId,
					strName,
					NULL,
					dwDetailLevel,
					dwCounterType,
					dwMaxCounterSize,
					dwDataOffset,
					dwDefaultScale);
				if (FAILED(hr))
				{
					ClearMap();
					return hr;
				}

				DWORD dwNameId = *pData++;
				DWORD dwHelpId = *pData++;
				CategoryInfo* pCategoryInfo = _GetCategoryInfo(_GetNumCategories()-1);
				CounterInfo* pCounterInfo = pCategoryInfo->_GetCounterInfo(pCategoryInfo->_GetNumCounters()-1);
				pCounterInfo->m_nNameId = dwNameId;
				pCounterInfo->m_nHelpId = dwHelpId;
			}
		}

		// fill in cache data
		ULONG* pnCounterBlockSize = NULL; // pointer to the object's counter block size
		for (DWORD i=0; i<_GetNumCategories(); i++)
		{
			CategoryInfo* pCategoryInfo = _GetCategoryInfo(i);
			// align at 8 bytes per Q262335
			pCategoryInfo->m_nCounterBlockSize = (ULONG) AtlAlignUp(sizeof(PERF_COUNTER_BLOCK), 8);
			pnCounterBlockSize = &pCategoryInfo->m_nCounterBlockSize;
			_FillCategoryType(pCategoryInfo);
			for (DWORD j=0; j<pCategoryInfo->_GetNumCounters(); j++)
			{
				CounterInfo* pCounterInfo = pCategoryInfo->_GetCounterInfo(j);
				_FillCounterDef(pCounterInfo, pnCounterBlockSize);
			}
			// align at 8 bytes per Q262335
			pCategoryInfo->m_nCounterBlockSize = (ULONG) AtlAlignUp(pCategoryInfo->m_nCounterBlockSize, 8);
		}

		return S_OK;
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}
}

inline HRESULT CPerfMon::_SaveMap() throw()
{
	_ATLTRY
	{
		// figure out how much memory we need
		size_t nSize = (2 + 9*_GetNumCategoriesAndCounters()) * sizeof(DWORD);
		for (UINT i=0; i<_GetNumCategories(); i++)
		{
			// if any of the entries have names, they'd better all have names
			CategoryInfo* pCategoryInfo = _GetCategoryInfo(i);
			if (!pCategoryInfo->m_strName.IsEmpty())
			{
				nSize += sizeof(DWORD) + AtlAlignUp(sizeof(WCHAR) * pCategoryInfo->m_strName.GetLength(), sizeof(DWORD));
				for (UINT j=0; j<pCategoryInfo->_GetNumCounters(); j++)
				{
					CounterInfo* pCounterInfo = pCategoryInfo->_GetCounterInfo(j);
					nSize += sizeof(DWORD) + AtlAlignUp(sizeof(WCHAR) * pCounterInfo->m_strName.GetLength(), sizeof(DWORD));
				}
			}
		}

		CHeapPtr<BYTE> blob;
		if (!blob.Allocate(nSize))
			return E_OUTOFMEMORY;

		// start with blob size and number of items in the blob
		DWORD* pCurrent = reinterpret_cast<DWORD*>(blob.m_pData);
		memset(pCurrent, 0, nSize);
		*pCurrent++ = (DWORD) nSize; // blob size
		*pCurrent++ = _GetNumCategoriesAndCounters(); // number of items
		size_t nSizeLast = nSize;
		nSize -= 2 * sizeof(DWORD);
		if(nSize > nSizeLast) return E_FAIL;

		for (UINT i=0; i<_GetNumCategories(); i++)
		{
			// add all the relevant runtime info to the blob for each item
			CategoryInfo* pCategoryInfo = _GetCategoryInfo(i);

			*pCurrent++ = TRUE; // is object
			*pCurrent++ = pCategoryInfo->m_dwCategoryId;
			*pCurrent++ = pCategoryInfo->m_dwDetailLevel;
			*pCurrent++ = pCategoryInfo->m_nDefaultCounter;
			*pCurrent++ = pCategoryInfo->m_nInstanceLess;
			*pCurrent++ = pCategoryInfo->m_nStructSize;
			*pCurrent++ = pCategoryInfo->m_nMaxInstanceNameLen;
			*pCurrent++ = pCategoryInfo->m_nNameId;
			*pCurrent++ = pCategoryInfo->m_nHelpId;
			nSizeLast = nSize;
			nSize -= 9 * sizeof(DWORD);
			if(nSize > nSizeLast) return E_FAIL;

			for (UINT j=0; j<pCategoryInfo->_GetNumCounters(); j++)
			{
				CounterInfo* pCounterInfo = pCategoryInfo->_GetCounterInfo(j);

				*pCurrent++ = FALSE; // is object
				*pCurrent++ = pCounterInfo->m_dwCounterId;
				*pCurrent++ = pCounterInfo->m_dwDetailLevel;
				*pCurrent++ = pCounterInfo->m_dwCounterType;
				*pCurrent++ = pCounterInfo->m_nMaxCounterSize;
				*pCurrent++ = pCounterInfo->m_nDataOffset;
				*pCurrent++ = pCounterInfo->m_nDefaultScale;
				*pCurrent++ = pCounterInfo->m_nNameId;
				*pCurrent++ = pCounterInfo->m_nHelpId;
				nSizeLast = nSize;
				nSize -= 9 * sizeof(DWORD);
				if(nSize > nSizeLast) return E_FAIL;
			}
		}

		// add names to the blob
		for (UINT i=0; i<_GetNumCategories(); i++)
		{
			CategoryInfo* pCategoryInfo = _GetCategoryInfo(i);
			// copy the len of the string (in characters) then the wide-char version of the string
			// pad the string to a dword boundary
			int nLen = pCategoryInfo->m_strName.GetLength();
			*pCurrent++ = nLen;
			nSizeLast = nSize;
			nSize -= sizeof(DWORD);
			if(nSize > nSizeLast) return E_FAIL;

			Checked::memcpy_s(pCurrent, nSize, CT2CW(pCategoryInfo->m_strName), sizeof(WCHAR)*nLen);
			pCurrent += AtlAlignUp(sizeof(WCHAR) * nLen, sizeof(DWORD))/sizeof(DWORD);
			nSizeLast = nSize;
			nSize -= sizeof(WCHAR)*nLen;
			if(nSize > nSizeLast) return E_FAIL;

			for (UINT j=0; j<pCategoryInfo->_GetNumCounters(); j++)
			{
				CounterInfo* pCounterInfo = pCategoryInfo->_GetCounterInfo(j);
				// copy the len of the string (in characters) then the wide-char version of the string
				// pad the string to a dword boundary
				int nCounterLen = pCounterInfo->m_strName.GetLength();
				*pCurrent++ = nCounterLen;
				nSizeLast = nSize;
				nSize -= sizeof(DWORD);
				if(nSize > nSizeLast) return E_FAIL;

				Checked::memcpy_s(pCurrent, nSize, CT2CW(pCounterInfo->m_strName), sizeof(WCHAR)*nCounterLen);
				pCurrent += AtlAlignUp(sizeof(WCHAR) * nCounterLen, sizeof(DWORD))/sizeof(DWORD);
				nSizeLast = nSize;
				nSize -= sizeof(WCHAR)*nCounterLen;
				if(nSize > nSizeLast) return E_FAIL;
			}
		}

		CRegKey rkApp;
		CString str;
		DWORD dwErr;

		str.Format(c_szAtlPerfPerformanceKey, GetAppName());
		dwErr = rkApp.Open(HKEY_LOCAL_MACHINE, str);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		rkApp.SetBinaryValue(c_szAtlPerfMap, blob, *LPDWORD(blob.m_pData));

		return S_OK;
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}
}

inline CPerfMon::CategoryInfo* CPerfMon::_FindCategoryInfo(DWORD dwCategoryId) throw()
{
	for (UINT i=0; i<_GetNumCategories(); i++)
	{
		CategoryInfo* pCategoryInfo = _GetCategoryInfo(i);
		if (pCategoryInfo->m_dwCategoryId == dwCategoryId)
			return pCategoryInfo;
	}

	return NULL;
}

inline CPerfMon::CounterInfo* CPerfMon::_FindCounterInfo(CategoryInfo* pCategoryInfo, DWORD dwCounterId)
{
	ATLENSURE_RETURN_VAL(pCategoryInfo != NULL, NULL);

	for (DWORD i=0; i<pCategoryInfo->_GetNumCounters(); i++)
	{
		CounterInfo* pCounter = pCategoryInfo->_GetCounterInfo(i);
		if (pCounter->m_dwCounterId == dwCounterId)
			return pCounter;
	}

	return NULL;
}

inline CPerfMon::CounterInfo* CPerfMon::_FindCounterInfo(DWORD dwCategoryId, DWORD dwCounterId) throw()
{
	CategoryInfo* pCategoryInfo = _FindCategoryInfo(dwCategoryId);
	if (pCategoryInfo != NULL)
		return _FindCounterInfo(pCategoryInfo, dwCounterId);

	return NULL;
}

inline BOOL CPerfMon::_WantCategoryType(__in_z LPWSTR szValue, __in DWORD dwCategoryId) throw(...)
{
	ATLASSERT(szValue != NULL);

	if (lstrcmpiW(c_szAtlPerfGlobal, szValue) == 0)
		return TRUE;

	CString strList(szValue);
	int nStart = 0;

	CString strNum = strList.Tokenize(_T(" "), nStart);
	while (!strNum.IsEmpty())
	{
		if (_ttoi(strNum) == int(dwCategoryId))
			return TRUE;

		strNum = strList.Tokenize(_T(" "), nStart);
	}

	return FALSE;
}

inline LPBYTE CPerfMon::_AllocData(LPBYTE& pData, ULONG nBytesAvail, ULONG* pnBytesUsed, size_t nBytesNeeded)
{
	ATLENSURE_RETURN_VAL(pnBytesUsed != NULL, NULL);
	ULONG newSize = *pnBytesUsed+static_cast<ULONG>(nBytesNeeded);

	if ((newSize < *pnBytesUsed) || (newSize < (ULONG) nBytesNeeded) || (nBytesAvail < newSize))
		return NULL;

	LPBYTE p = pData;
	pData += nBytesNeeded;
	*pnBytesUsed += (ULONG) nBytesNeeded;

	return p;
}

inline DWORD& CPerfMon::_GetBlockId(CAtlFileMappingBase* pBlock) 
{
	DWORD* pDw = _GetBlockId_NoThrow(pBlock);
	ATLENSURE(pDw);
	return *pDw;
}

inline DWORD* CPerfMon::_GetBlockId_NoThrow(CAtlFileMappingBase* pBlock) 
{
	if (pBlock == NULL)
		return NULL;

	return LPDWORD(LPBYTE(pBlock->GetData()) + m_nSchemaSize);
}

inline void CPerfMon::_FillCategoryType(CategoryInfo* pCategoryInfo) throw()
{
	PERF_OBJECT_TYPE& type = pCategoryInfo->m_cache;
	type.DefinitionLength = sizeof(PERF_OBJECT_TYPE) + sizeof(PERF_COUNTER_DEFINITION) * pCategoryInfo->_GetNumCounters();
	type.TotalByteLength = type.DefinitionLength; // we will add the instance definitions/counter blocks as we go
	type.HeaderLength = sizeof(PERF_OBJECT_TYPE);
	type.ObjectNameTitleIndex = pCategoryInfo->m_nNameId;
	type.ObjectNameTitle = NULL;
	type.ObjectHelpTitleIndex = pCategoryInfo->m_nHelpId;
	type.ObjectHelpTitle = NULL;
	type.DetailLevel = pCategoryInfo->m_dwDetailLevel;
	type.NumCounters = pCategoryInfo->_GetNumCounters();
	type.DefaultCounter = pCategoryInfo->m_nDefaultCounter;
	if (pCategoryInfo->m_nInstanceLess == PERF_NO_INSTANCES)
		type.NumInstances = PERF_NO_INSTANCES;
	else
		type.NumInstances = 0; // this will be calculated as objects are processed
	type.CodePage = 0;
	type.PerfTime.QuadPart = 0;
	QueryPerformanceFrequency (&(type.PerfFreq));
}

inline void CPerfMon::_FillCounterDef(CounterInfo* pCounterInfo, ULONG* pnCounterBlockSize) throw()
{
	PERF_COUNTER_DEFINITION& def = pCounterInfo->m_cache;

	def.ByteLength = sizeof(PERF_COUNTER_DEFINITION);
	def.CounterNameTitleIndex = pCounterInfo->m_nNameId;
	def.CounterNameTitle = NULL;
	def.CounterHelpTitleIndex = pCounterInfo->m_nHelpId;
	def.CounterHelpTitle = NULL;
	def.DefaultScale = pCounterInfo->m_nDefaultScale;
	def.DetailLevel = pCounterInfo->m_dwDetailLevel;
	def.CounterType = pCounterInfo->m_dwCounterType;
	DWORD dwAlignOfCounter=0;
	switch (pCounterInfo->m_dwCounterType & ATLPERF_SIZE_MASK)
	{
	case PERF_SIZE_DWORD:
		def.CounterSize = sizeof(DWORD);
		dwAlignOfCounter = sizeof(DWORD);
		break;
	case PERF_SIZE_LARGE:
		def.CounterSize = sizeof(__int64);
		dwAlignOfCounter = sizeof(__int64);
		break;
	case PERF_SIZE_ZERO:
		def.CounterSize = 0;
		dwAlignOfCounter = 0;
		break;
	case PERF_SIZE_VARIABLE_LEN:
		ATLASSERT((pCounterInfo->m_dwCounterType & ATLPERF_TYPE_MASK) == PERF_TYPE_TEXT);
		if ((pCounterInfo->m_dwCounterType & ATLPERF_TEXT_MASK) == PERF_TEXT_UNICODE)
		{
			def.CounterSize = (DWORD) AtlAlignUp(pCounterInfo->m_nMaxCounterSize * sizeof(WCHAR), sizeof(DWORD));
		}
		else
		{
			def.CounterSize = (DWORD) AtlAlignUp(pCounterInfo->m_nMaxCounterSize * sizeof(char), sizeof(DWORD));
		}
		break;
	}
	*pnCounterBlockSize = AtlAlignUp(*pnCounterBlockSize, dwAlignOfCounter);
	def.CounterOffset = *pnCounterBlockSize;
	*pnCounterBlockSize += def.CounterSize;
}

inline HRESULT CPerfMon::_CollectInstance(
	CategoryInfo* pCategoryInfo,
	LPBYTE& pData,
	ULONG nBytesAvail,
	ULONG* pnBytesUsed,
	CPerfObject* _pInstance,
	PERF_OBJECT_TYPE* pObjectType,
	PERF_COUNTER_DEFINITION* pCounterDefs
	) throw()
{
	DWORD dwInstance = _pInstance->m_dwInstance;

	// grab a snapshot of the object
	USES_ATL_SAFE_ALLOCA;
	CPerfObject* pInstance = (CPerfObject*) _ATL_SAFE_ALLOCA(_pInstance->m_nAllocSize, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	if (pInstance == NULL)
	{
		return E_OUTOFMEMORY;
	}
	Checked::memcpy_s(pInstance, _pInstance->m_nAllocSize, _pInstance, _pInstance->m_nAllocSize);

	// if it was changed or deleted between when we first saw it and when we copied
	// it, then forget about whatever happens to be there for this collection period
	if (pInstance->m_dwCategoryId != pCategoryInfo->m_dwCategoryId ||
			dwInstance != pInstance->m_dwInstance ||
			pInstance->m_nRefCount == 0)
		return S_OK;

	// we have a copy of something that claims to be the object type we're expecting
	// put it into the data blob
	PERF_INSTANCE_DEFINITION* pInstanceDef = NULL;

	if (pCategoryInfo->m_nInstanceLess == PERF_NO_INSTANCES)
		pObjectType->NumInstances = PERF_NO_INSTANCES;
	else
	{
		pObjectType->NumInstances++;

		// create an instance definition
		pInstanceDef = _AllocStruct(pData, nBytesAvail, pnBytesUsed, (PERF_INSTANCE_DEFINITION*) NULL);
		if (pInstanceDef == NULL)
			return E_OUTOFMEMORY;

		pInstanceDef->ParentObjectTitleIndex = 0;
		pInstanceDef->ParentObjectInstance = 0;
		pInstanceDef->UniqueID = PERF_NO_UNIQUE_ID;

		// handle the instance name
		LPCWSTR szInstNameSrc = LPCWSTR(LPBYTE(pInstance)+pInstance->m_nInstanceNameOffset);
		pInstanceDef->NameLength = (ULONG)(wcslen(szInstNameSrc)+1)*sizeof(WCHAR);
		// align at 8 bytes per Q262335
		ULONG nNameAlloc = (ULONG) AtlAlignUp(pInstanceDef->NameLength, 8);
		LPWSTR szInstNameDest = (LPWSTR) _AllocData(pData, nBytesAvail, pnBytesUsed, nNameAlloc);
		if (szInstNameDest == NULL)
			return E_OUTOFMEMORY;

		Checked::memcpy_s(szInstNameDest, nNameAlloc, szInstNameSrc, pInstanceDef->NameLength);
		pInstanceDef->NameOffset = ULONG(LPBYTE(szInstNameDest) - LPBYTE(pInstanceDef));

		pInstanceDef->ByteLength = DWORD(sizeof(PERF_INSTANCE_DEFINITION) + nNameAlloc);
	}

	// create the counter block + data
	LPBYTE pCounterData = _AllocData(pData, nBytesAvail, pnBytesUsed, pCategoryInfo->m_nCounterBlockSize);
	if (pCounterData == NULL)
		return E_OUTOFMEMORY;

	// fill in the counter block header for the data
	PERF_COUNTER_BLOCK* pCounterBlock = (PERF_COUNTER_BLOCK*) pCounterData;
	pCounterBlock->ByteLength = pCategoryInfo->m_nCounterBlockSize;

	// fill in the data
	for (ULONG i=0; i<pObjectType->NumCounters; i++)
	{
		CounterInfo* pCounterInfo = pCategoryInfo->_GetCounterInfo(i);
		PERF_COUNTER_DEFINITION& def = pCounterDefs[i];
		LPBYTE pSrc = LPBYTE(pInstance)+pCounterInfo->m_nDataOffset;
		LPBYTE pDest = pCounterData+def.CounterOffset;
		switch (pCounterInfo->m_dwCounterType & ATLPERF_SIZE_MASK)
		{
		case PERF_SIZE_DWORD:
			*LPDWORD(pDest) = *LPDWORD(pSrc);
			break;
		case PERF_SIZE_LARGE:
			 *(ULONGLONG*)(pDest) =  *(ULONGLONG*)(pSrc);
			break;
		case PERF_SIZE_VARIABLE_LEN:
			if ((pCounterInfo->m_dwCounterType & ATLPERF_TEXT_MASK) == PERF_TEXT_UNICODE)
			{
				LPCWSTR szSrc = reinterpret_cast<LPCWSTR>(pSrc);
				LPWSTR szDest = reinterpret_cast<LPWSTR>(pDest);
				size_t nLen = __min(wcslen(szSrc), pCounterInfo->m_nMaxCounterSize-1);
				Checked::wcsncpy_s(szDest, pCounterInfo->m_nMaxCounterSize-1, szSrc, nLen);
				szDest[nLen] = 0;
			}
			else
			{
				LPCSTR szSrc = reinterpret_cast<LPCSTR>(pSrc);
				LPSTR szDest = reinterpret_cast<LPSTR>(pDest);
				size_t nLen = __min(strlen(szSrc), pCounterInfo->m_nMaxCounterSize-1);
				Checked::strncpy_s(szDest, pCounterInfo->m_nMaxCounterSize-1, szSrc, nLen);
				szDest[nLen] = 0;
			}
			break;
		}
	}

	if (pInstanceDef != NULL)
		pObjectType->TotalByteLength += pInstanceDef->ByteLength;
	pObjectType->TotalByteLength += pCounterBlock->ByteLength;

	return S_OK;
}

inline HRESULT CPerfMon::_CollectInstance(
	CategoryInfo* pCategoryInfo,
	LPBYTE& pData,
	ULONG nBytesAvail,
	ULONG* pnBytesUsed,
	PERF_OBJECT_TYPE* pObjectType,
	PERF_COUNTER_DEFINITION* pCounterDefs
	) throw()
{
	// specialization to collect an instanceless object with no instance data
	ATLASSERT(pCategoryInfo->m_nInstanceLess == PERF_NO_INSTANCES);
	pObjectType->NumInstances = PERF_NO_INSTANCES;

	// create the counter block + data
	LPBYTE pCounterData = _AllocData(pData, nBytesAvail, pnBytesUsed, pCategoryInfo->m_nCounterBlockSize);
	if (pCounterData == NULL)
		return E_OUTOFMEMORY;

	// fill in the counter block header for the data
	PERF_COUNTER_BLOCK* pCounterBlock = (PERF_COUNTER_BLOCK*) pCounterData;
	pCounterBlock->ByteLength = pCategoryInfo->m_nCounterBlockSize;

	// fill in the data
	for (ULONG i=0; i<pObjectType->NumCounters; i++)
	{
		CounterInfo* pCounterInfo = pCategoryInfo->_GetCounterInfo(i);
		PERF_COUNTER_DEFINITION& def = pCounterDefs[i];
		LPBYTE pDest = pCounterData+def.CounterOffset;
		switch (pCounterInfo->m_dwCounterType & ATLPERF_SIZE_MASK)
		{
		case PERF_SIZE_DWORD:
			*LPDWORD(pDest) = 0;
			break;
		case PERF_SIZE_LARGE:
			*PULONGLONG(pDest) = 0;
			break;
		case PERF_SIZE_VARIABLE_LEN:
			if ((pCounterInfo->m_dwCounterType & ATLPERF_TEXT_MASK) == PERF_TEXT_UNICODE)
				memset(pDest, 0, pCounterInfo->m_nMaxCounterSize*sizeof(WCHAR));
			else
				memset(pDest, 0, pCounterInfo->m_nMaxCounterSize*sizeof(CHAR));
			break;
		}
	}

	pObjectType->TotalByteLength += pCounterBlock->ByteLength;

	return S_OK;
}

inline HRESULT CPerfMon::_CollectCategoryType(
	CategoryInfo* pCategoryInfo,
	LPBYTE pData,
	ULONG nBytesAvail,
	ULONG* pnBytesUsed
	) throw()
{
	ATLENSURE_RETURN(pCategoryInfo != NULL);
	ATLASSERT(pnBytesUsed != NULL);

	// write the object definition out
	PERF_OBJECT_TYPE* pObjectType = _AllocStruct(pData, nBytesAvail, pnBytesUsed, (PERF_OBJECT_TYPE*) NULL);
	if (pObjectType == NULL)
		return E_OUTOFMEMORY;

	Checked::memcpy_s(pObjectType, sizeof(PERF_OBJECT_TYPE), &pCategoryInfo->m_cache, sizeof(PERF_OBJECT_TYPE));

	// save a pointer to the first counter entry and counter definition.
	// we'll need them when we create the PERF_COUNTER_BLOCK data
	PERF_COUNTER_DEFINITION* pCounterDefs = reinterpret_cast<PERF_COUNTER_DEFINITION*>(pData);

	// write the counter definitions out
	for (DWORD i=0; i<pCategoryInfo->_GetNumCounters(); i++)
	{
		CounterInfo* pCounterInfo = pCategoryInfo->_GetCounterInfo(i);

		PERF_COUNTER_DEFINITION* pCounterDef = _AllocStruct(pData, nBytesAvail, pnBytesUsed, (PERF_COUNTER_DEFINITION*) NULL);
		if (pCounterDef == NULL)
			return E_OUTOFMEMORY;

		Checked::memcpy_s(pCounterDef, sizeof(PERF_COUNTER_DEFINITION), &pCounterInfo->m_cache, sizeof(PERF_COUNTER_DEFINITION));
		
		// set PerfTime and PerfFreq for PERF_ELAPSED_TIME counter.
		if(pCounterDef->CounterType == PERF_ELAPSED_TIME)
		{
			LARGE_INTEGER currTime;
			if (FALSE != QueryPerformanceCounter(&currTime))
				pObjectType->PerfTime = currTime;
			else
				pObjectType->PerfTime.QuadPart = 0;
			QueryPerformanceFrequency (&(pObjectType->PerfFreq));
		}
	}

	// search for objects of the appropriate type and write out their instance/counter data
	bool bGotInstance = false;

	CAtlFileMappingBase* pCurrentBlock = _GetNextBlock(NULL);
	if (pCurrentBlock != NULL)
	{
		CPerfObject* pInstance = _GetFirstInstance(pCurrentBlock);
		while (pInstance && pInstance->m_nAllocSize != 0)
		{
			if (pInstance->m_dwCategoryId == pCategoryInfo->m_dwCategoryId)
			{
				bGotInstance = true;
				HRESULT hr = _CollectInstance(pCategoryInfo, pData, nBytesAvail,
						pnBytesUsed, pInstance, pObjectType, pCounterDefs);
				if (FAILED(hr))
					return hr;
			}

			pInstance = _GetNextInstance(pInstance);
			ATLENSURE_RETURN(pInstance!= NULL);

			if (pInstance->m_nAllocSize == (ULONG) -1)
			{
				pCurrentBlock = _GetNextBlock(pCurrentBlock);
				if (pCurrentBlock == NULL)
					pInstance = NULL;
				else
					pInstance = _GetFirstInstance(pCurrentBlock);
			}
		}
	}

	if (pCategoryInfo->m_nInstanceLess == PERF_NO_INSTANCES && !bGotInstance)
	{
		// we have an instanceless (singleton) object with no data. send zeroed data
		HRESULT hr = _CollectInstance(pCategoryInfo, pData, nBytesAvail,
				pnBytesUsed, pObjectType, pCounterDefs);
		if (FAILED(hr))
			return hr;
	}

	return S_OK;
}

inline DWORD CPerfMon::Open(LPWSTR szDeviceNames) throw()
{
	(szDeviceNames); // unused

	return 0;
}

inline DWORD CPerfMon::Collect(
	__in_z LPWSTR szValue,
	__deref_inout_bcount(*pcbBytes) LPVOID* ppData,
	__inout LPDWORD pcbBytes,
	__inout LPDWORD pcObjectTypes
	) throw()
{

 
 


	_ATLTRY
	{
		if (FAILED(_OpenAllBlocks()))
		{
			*pcbBytes = 0;
			*pcObjectTypes = 0;
			return ERROR_SUCCESS;
		}

		LPBYTE pData = LPBYTE(*ppData);
		ULONG nBytesLeft = *pcbBytes;
		*pcbBytes = 0;

		if (_GetNumCategories() == 0)
		{
			// nothing is providing data. we need to load the map directly
			// from the registry in order to provide category/counter data
			CRegKey rkApp;
			DWORD dwErr;
			CString strAppKey;

			strAppKey.Format(c_szAtlPerfPerformanceKey, GetAppName());

			dwErr = rkApp.Open(HKEY_LOCAL_MACHINE, strAppKey, KEY_READ);
			if (dwErr != ERROR_SUCCESS)
			{
				*pcbBytes = 0;
				*pcObjectTypes = 0;
				return ERROR_SUCCESS;
			}

			ULONG nBytes = 0;
			dwErr = rkApp.QueryBinaryValue(c_szAtlPerfMap, NULL, &nBytes);
			if (dwErr != ERROR_SUCCESS)
			{
				*pcbBytes = 0;
				*pcObjectTypes = 0;
				return ERROR_SUCCESS;
			}

			CHeapPtr<DWORD> buf;
			if (!buf.Allocate((nBytes+3)/4))
			{
				*pcbBytes = 0;
				*pcObjectTypes = 0;
				return ERROR_SUCCESS;
			}

			dwErr = rkApp.QueryBinaryValue(c_szAtlPerfMap, buf, &nBytes);
			if (dwErr != ERROR_SUCCESS)
			{
				*pcbBytes = 0;
				*pcObjectTypes = 0;
				return ERROR_SUCCESS;
			}

			if (FAILED(_LoadMap(buf)))
			{
				*pcbBytes = 0;
				*pcObjectTypes = 0;
				return ERROR_SUCCESS;
			}
		}

		for (UINT i=0; i<_GetNumCategories(); i++)
		{
			CategoryInfo* pCategoryInfo = _GetCategoryInfo(i);
			if (_WantCategoryType(szValue, pCategoryInfo->m_nNameId))
			{
				ULONG nBytesUsed = 0;
				HRESULT hr = _CollectCategoryType(pCategoryInfo, pData, nBytesLeft, &nBytesUsed);
				if (hr == E_OUTOFMEMORY)
				{
					*pcbBytes = 0;
					*pcObjectTypes = 0;
					return ERROR_MORE_DATA;
				}
				else if (FAILED(hr))
				{
					*pcbBytes = 0;
					*pcObjectTypes = 0;
					return ERROR_SUCCESS;
				}

				(*pcObjectTypes)++;
				(*pcbBytes) += nBytesUsed;
				nBytesLeft -= nBytesUsed;
				pData += nBytesUsed;
			}
		}

		*ppData = pData;
		return ERROR_SUCCESS;
	}
	_ATLCATCHALL()
	{
		*pcbBytes = 0;
		*pcObjectTypes = 0;
		return ERROR_SUCCESS;
	}
}

inline DWORD CPerfMon::Close() throw()
{
	UnInitialize();
	return ERROR_SUCCESS;
}

#ifdef _ATL_PERF_REGISTER
#pragma warning (push)
#pragma warning(disable : 4996)

inline void CPerfMon::_AppendStrings(
	LPTSTR& pszNew,
	CAtlArray<CString>& astrStrings,
	ULONG iFirstIndex
	) throw()
{
	for (UINT iString = 0; iString < astrStrings.GetCount(); iString++)
	{
		INT nFormatChars = _stprintf(pszNew, _T("%d"), iFirstIndex+2*iString);
		pszNew += nFormatChars + 1;
		_tcscpy(pszNew, astrStrings[iString]);
		pszNew += astrStrings[iString].GetLength() + 1;
	}
}

#pragma warning (pop)

inline HRESULT CPerfMon::_AppendRegStrings(
	CRegKey& rkLang,
	LPCTSTR szValue,
	CAtlArray<CString>& astrStrings,
	ULONG nNewStringSize,
	ULONG iFirstIndex,
	ULONG iLastIndex
	) throw()
{
	_ATLTRY
	{
		// load the existing strings, add the new data, and resave the strings
		ULONG nCharsOrig = 0;
		ULONG nCharsNew;
		DWORD dwErr;

		dwErr = rkLang.QueryMultiStringValue(szValue, NULL, &nCharsOrig);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		nCharsNew = nCharsOrig + nNewStringSize;

		CString strOrig;
		dwErr = rkLang.QueryMultiStringValue(szValue, CStrBuf(strOrig, nCharsOrig, CStrBuf::SET_LENGTH), &nCharsOrig);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);
		LPCTSTR pszOrig = strOrig;

		CString strNew;
		CStrBuf szNew(strNew, nCharsNew, CStrBuf::SET_LENGTH);
		LPTSTR pszNew = szNew;

		bool bNewStringsAdded = false;

		while (*pszOrig != '\0')
		{
			ULONG iIndex = _ttoi(pszOrig);
			int nLen = (int) _tcslen(pszOrig) + 1; // get the length of the index and null
			nLen += (int) _tcslen(pszOrig+nLen) + 1; // add the length of the description and null

			if (!bNewStringsAdded && iIndex >= iFirstIndex)
			{
				LPTSTR pszOld =pszNew;
				_AppendStrings(pszNew, astrStrings, iFirstIndex);
				bNewStringsAdded = true;
				ULONG nCharsNewLast = nCharsNew;
				nCharsNew -= ULONG(pszNew-pszOld);
				if(nCharsNew > nCharsNewLast) 
				{
					return E_FAIL;
				}
			}

			if (iIndex < iFirstIndex || iIndex > iLastIndex)
			{
				Checked::memmove_s(pszNew, nCharsNew, pszOrig, nLen*sizeof(TCHAR));
				pszNew += nLen;
			}
			pszOrig += nLen;
		}
		if (!bNewStringsAdded)
			_AppendStrings(pszNew, astrStrings, iFirstIndex);

		*pszNew++ = '\0'; // must have 2 null terminators at end of multi_sz

		dwErr = rkLang.SetMultiStringValue(szValue, strNew);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		return S_OK;
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}
}

inline HRESULT CPerfMon::_RemoveRegStrings(
	CRegKey& rkLang,
	LPCTSTR szValue,
	ULONG iFirstIndex,
	ULONG iLastIndex
	) throw()
{
	_ATLTRY
	{
		// load the existing strings, remove the data, and resave the strings
		DWORD nChars = 0;
		DWORD dwErr;
		
		dwErr = rkLang.QueryMultiStringValue(szValue, NULL, &nChars);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		CString str;
		CStrBuf szBuf(str, nChars, CStrBuf::SET_LENGTH);
		DWORD nMaxLen = nChars*sizeof(TCHAR);

		dwErr = rkLang.QueryMultiStringValue(szValue, szBuf, &nChars);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		LPCTSTR pszRead = szBuf;
		LPTSTR pszWrite = szBuf;
		while (*pszRead != '\0')
		{
			ULONG iIndex = _ttoi(pszRead);
			int nLen = (int) _tcslen(pszRead) + 1; // get the length of the index and null
			nLen += (int) _tcslen(pszRead+nLen) + 1; // add the length of the description and null
			if (iIndex < iFirstIndex || iIndex > iLastIndex)
			{
				Checked::memmove_s(pszWrite, nMaxLen , pszRead, nLen*sizeof(TCHAR));
				UINT nMaxLenLast = nMaxLen;
				nMaxLen -= nLen*sizeof(TCHAR);
				if(nMaxLen > nMaxLenLast) return E_FAIL;
				pszWrite += nLen;
			}
			pszRead += nLen;
		}
		*pszWrite++ = '\0'; // must have 2 null terminators at end of multi_sz

		dwErr = rkLang.SetMultiStringValue(szValue, szBuf);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		return S_OK;
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}
}

inline HRESULT CPerfMon::_ReserveStringRange(DWORD& dwFirstCounter, DWORD& dwFirstHelp) throw()
{
	CRegKey rkApp;
	CString strAppKey;
	DWORD dwErr;

	_ATLTRY
	{
		strAppKey.Format(c_szAtlPerfPerformanceKey, GetAppName());
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}

	DWORD nNumStrings = _GetNumCategoriesAndCounters();

	dwErr = rkApp.Open(HKEY_LOCAL_MACHINE, strAppKey);
	if (dwErr == ERROR_SUCCESS)
	{
		// see if we already have a sufficient range reserved
		DWORD dwFirstAppCounter;
		DWORD dwFirstAppHelp;
		DWORD dwLastAppCounter;
		DWORD dwLastAppHelp;

		if (rkApp.QueryDWORDValue(c_szAtlPerfFirstCounter, dwFirstAppCounter) == ERROR_SUCCESS &&
				rkApp.QueryDWORDValue(c_szAtlPerfFirstHelp, dwFirstAppHelp) == ERROR_SUCCESS &&
				rkApp.QueryDWORDValue(c_szAtlPerfLastCounter, dwLastAppCounter) == ERROR_SUCCESS &&
				rkApp.QueryDWORDValue(c_szAtlPerfLastHelp, dwLastAppHelp) == ERROR_SUCCESS &&
				dwLastAppCounter-dwFirstAppCounter+2 >= 2*nNumStrings &&
				dwLastAppHelp-dwFirstAppHelp+2 >= 2*nNumStrings)
		{
			dwFirstCounter = dwFirstAppCounter;
			dwFirstHelp = dwFirstAppHelp;
			return S_OK;
		}
	}

	CRegKey rkPerfLib;

	dwErr = rkPerfLib.Open(HKEY_LOCAL_MACHINE, c_szAtlPerfPerfLibKey);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	if (!rkApp)
	{
		dwErr = rkApp.Create(HKEY_LOCAL_MACHINE, strAppKey);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);
	}

	// figure out the counter range
	DWORD dwLastCounter;
	DWORD dwLastHelp;

	dwErr = rkPerfLib.QueryDWORDValue(c_szAtlPerfLastCounter, dwLastCounter);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkPerfLib.QueryDWORDValue(c_szAtlPerfLastHelp, dwLastHelp);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwFirstCounter = dwLastCounter + 2;
	dwFirstHelp = dwLastHelp + 2;
	dwLastCounter += 2*nNumStrings;
	dwLastHelp += 2*nNumStrings;

	dwErr = rkPerfLib.SetDWORDValue(c_szAtlPerfLastCounter, dwLastCounter);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkPerfLib.SetDWORDValue(c_szAtlPerfLastHelp, dwLastHelp);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	// register the used counter range
	dwErr = rkApp.SetDWORDValue(c_szAtlPerfFirstCounter, dwFirstCounter);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkApp.SetDWORDValue(c_szAtlPerfLastCounter, dwLastCounter);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkApp.SetDWORDValue(c_szAtlPerfFirstHelp, dwFirstHelp);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkApp.SetDWORDValue(c_szAtlPerfLastHelp, dwLastHelp);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	return S_OK;
}

inline HRESULT CPerfMon::Register(
	LPCTSTR szOpenFunc,
	LPCTSTR szCollectFunc,
	LPCTSTR szCloseFunc,
	HINSTANCE hDllInstance /* == _AtlBaseModule.GetModuleInstance() */
	) throw()
{
	ATLASSERT(szOpenFunc != NULL);
	ATLASSERT(szCollectFunc != NULL);
	ATLASSERT(szCloseFunc != NULL);

	CString str;
	DWORD dwErr;
	HRESULT hr;
   	hr = CreateMap(LANGIDFROMLCID(GetThreadLocale()), hDllInstance);
	if (FAILED(hr)){
		hr = CreateMap(LANGIDFROMLCID(1033), hDllInstance);
		if (FAILED(hr))
		return hr;
	}

	CString strAppKey;
	_ATLTRY
	{
		strAppKey.Format(c_szAtlPerfPerformanceKey, GetAppName());
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}

	// if we're already registered, unregister so we can redo registration
	_UnregisterStrings();
	
	// reserve a range for our counter and help strings
	DWORD dwFirstCounter = 0;
	DWORD dwFirstHelp = 0;
	hr = _ReserveStringRange(dwFirstCounter, dwFirstHelp);
	if (FAILED(hr))
		return hr;

	DWORD dwCurrentName = dwFirstCounter;
	DWORD dwCurrentHelp = dwFirstHelp;
	for (UINT i=0; i<_GetNumCategories(); i++)
	{
		CategoryInfo* pCategoryInfo = _GetCategoryInfo(i);

		pCategoryInfo->m_nNameId = dwCurrentName;
		dwCurrentName += 2;
		pCategoryInfo->m_nHelpId = dwCurrentHelp;
		dwCurrentHelp += 2;

		for (UINT j=0; j<pCategoryInfo->_GetNumCounters(); j++)
		{
			CounterInfo* pCounterInfo = pCategoryInfo->_GetCounterInfo(j);

			pCounterInfo->m_nNameId = dwCurrentName;
			dwCurrentName += 2;
			pCounterInfo->m_nHelpId = dwCurrentHelp;
			dwCurrentHelp += 2;
		}
	}

	// register the app entry points
	CRegKey rkApp;

	dwErr = rkApp.Create(HKEY_LOCAL_MACHINE, strAppKey);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	_ATLTRY
	{
		DWORD dwFLen = GetModuleFileName(hDllInstance, CStrBuf(str, MAX_PATH), MAX_PATH);
		if( dwFLen == 0 )
			return AtlHresultFromLastError();
		else if( dwFLen == MAX_PATH )
			return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}

	dwErr = rkApp.SetStringValue(c_szAtlPerfLibrary, str);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkApp.SetStringValue(c_szAtlPerfOpen, szOpenFunc);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkApp.SetStringValue(c_szAtlPerfCollect, szCollectFunc);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkApp.SetStringValue(c_szAtlPerfClose, szCloseFunc);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkApp.SetStringValue(c_szAtlPerfLanguages, _T(""));
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	hr = _SaveMap();
	if (FAILED(hr))
		return hr;

	// if the dll is disabled, reenable it since we just reregistered it
	rkApp.DeleteValue(_T("Disable Performance Counters"));

	return S_OK;
}

inline HRESULT CPerfMon::RegisterStrings(
	LANGID language /* = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL) */,
	HINSTANCE hResInstance /* = _AtlBaseModule.GetResourceInstance() */
	) throw()
{
	_ATLTRY
	{
		CString str;
		DWORD dwErr;
		HRESULT hr;
		CRegKey rkLang;
		CRegKey rkApp;

		LANGID wPrimaryLanguage = (LANGID) PRIMARYLANGID(language);

		if (language == MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
		{
			//First try current thread locale
			language = LANGIDFROMLCID(GetThreadLocale());
			wPrimaryLanguage = (LANGID) PRIMARYLANGID(language);
		}
		str.Format(c_szAtlPerfPerfLibLangKey, wPrimaryLanguage);
		dwErr = rkLang.Open(HKEY_LOCAL_MACHINE, str);
		if (dwErr == ERROR_FILE_NOT_FOUND)
		{
			// failed using current thread, so try default system lcid
			language = GetSystemDefaultLangID();
			wPrimaryLanguage = (LANGID) PRIMARYLANGID(language);
			str.Format(c_szAtlPerfPerfLibLangKey, wPrimaryLanguage);
			dwErr = rkLang.Open(HKEY_LOCAL_MACHINE, str);
		}
		if (dwErr == ERROR_FILE_NOT_FOUND)
			return S_FALSE; // the language isn't installed on the system
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		hr = CreateMap(language, hResInstance);
		if (FAILED(hr))
			return hr;

		// load list of language strings already registered
		str.Format(c_szAtlPerfPerformanceKey, GetAppName());
		dwErr = rkApp.Open(HKEY_LOCAL_MACHINE, str);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		DWORD dwLangsLen = 0;
		CString strLangs;

		dwErr = rkApp.QueryStringValue(c_szAtlPerfLanguages, NULL, &dwLangsLen);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		ULONG nLangsBuffSize = dwLangsLen+4;
		CStrBuf szLangs(strLangs, nLangsBuffSize, CStrBuf::SET_LENGTH); // reserve room for adding new language
		dwErr = rkApp.QueryStringValue(c_szAtlPerfLanguages, szLangs, &dwLangsLen);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);
		dwLangsLen--; // don't count '\0'

		// see if this language has already been registered and if so, return
		TCHAR szNewLang[5];
		_sntprintf_s(szNewLang, _countof(szNewLang), _countof(szNewLang)-1, _T("%3.3x "), wPrimaryLanguage);
		if (strLangs.Find(szNewLang) != -1)
			return S_OK;

		// load the strings we want to append and figure out how much extra space is needed for them
		// (including up to 5-digit index values and 2 null separators)
		CAtlArray<CString> astrCounters;
		CAtlArray<CString> astrHelp;
		ULONG nNewCounterSize = 0;
		ULONG nNewHelpSize = 0;

		for (UINT i=0; i<_GetNumCategories(); i++)
		{
			CategoryInfo* pCategoryInfo = _GetCategoryInfo(i);

			astrCounters.Add(pCategoryInfo->m_strName);
			astrHelp.Add(pCategoryInfo->m_strHelp);

			for (UINT j=0; j<pCategoryInfo->_GetNumCounters(); j++)
			{
				CounterInfo* pCounterInfo = pCategoryInfo->_GetCounterInfo(j);

				astrCounters.Add(pCounterInfo->m_strName);
				astrHelp.Add(pCounterInfo->m_strHelp);
			}
		}

		for (size_t i=0; i<astrCounters.GetCount(); i++)
		{
			nNewCounterSize += astrCounters[i].GetLength() + 7;
			nNewHelpSize += astrHelp[i].GetLength() + 7;
		}

		DWORD dwFirstCounter;
		DWORD dwFirstHelp;
		DWORD dwLastCounter;
		DWORD dwLastHelp;

		dwErr = rkApp.QueryDWORDValue(c_szAtlPerfFirstCounter, dwFirstCounter);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		dwErr = rkApp.QueryDWORDValue(c_szAtlPerfFirstHelp, dwFirstHelp);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		dwErr = rkApp.QueryDWORDValue(c_szAtlPerfLastCounter, dwLastCounter);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		dwErr = rkApp.QueryDWORDValue(c_szAtlPerfLastHelp, dwLastHelp);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		hr = _AppendRegStrings(rkLang, c_szAtlPerfCounter, astrCounters, nNewCounterSize, dwFirstCounter, dwLastCounter);
		if (FAILED(hr))
			return hr;

		hr = _AppendRegStrings(rkLang, c_szAtlPerfHelp, astrHelp, nNewHelpSize, dwFirstHelp, dwLastHelp);
		if (FAILED(hr))
			return hr;

		// add the language to the list of installed languages
		Checked::tcscpy_s(szLangs+dwLangsLen, nLangsBuffSize-dwLangsLen, szNewLang);

		dwErr = rkApp.SetStringValue(c_szAtlPerfLanguages, szLangs);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		return S_OK;
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}
}

inline BOOL CPerfMon::EnumResLangProc(
	HINSTANCE hModule,
	LPCTSTR szType,
	LPCTSTR szName,
	LANGID wIDLanguage,
	LPARAM lParam
	) throw()
{
	hModule; // unused
	szType; // unused
	szName; // unused

	CAtlArray<LANGID>* pLangs = reinterpret_cast<CAtlArray<LANGID>*>(lParam);
	_ATLTRY
	{
		pLangs->Add(wIDLanguage);
	}
	_ATLCATCHALL()
	{
		return FALSE;
	}

	return TRUE;
}

inline HRESULT CPerfMon::RegisterAllStrings(
	HINSTANCE hResInstance /* = NULL */
	) throw()
{
	HRESULT hrReturn = S_FALSE;
	HRESULT hr;

	UINT nRes;
	hr = CreateMap(0, hResInstance, &nRes);
	if (FAILED(hr))
		return hr;

	if (nRes == 0)
		return RegisterStrings(0, hResInstance);

	if (hResInstance != NULL)
		return _RegisterAllStrings(nRes, hResInstance);

	for (int i = 0; hResInstance = _AtlBaseModule.GetHInstanceAt(i), hResInstance != NULL; i++)
	{
		hr = _RegisterAllStrings(nRes, hResInstance);
		if (FAILED(hr))
			return hr;
		if (hr == S_OK)
			hrReturn = S_OK;
	}

	return hrReturn;
}

inline HRESULT CPerfMon::_RegisterAllStrings(
	UINT nRes,
	HINSTANCE hResInstance
	) throw()
{
	HRESULT hrReturn = S_FALSE;
	HRESULT hr;

	CAtlArray<LANGID> langs;
	if (!EnumResourceLanguages(hResInstance, RT_STRING, MAKEINTRESOURCE((nRes>>4)+1), EnumResLangProc, reinterpret_cast<LPARAM>(&langs)))
		return AtlHresultFromLastError();

	for (UINT i=0; i<langs.GetCount(); i++)
	{
		hr = RegisterStrings(langs[i], hResInstance);
		if (FAILED(hr))
			return hr;
		if (hr == S_OK)
			hrReturn = S_OK;
	}

	return hrReturn;
}

inline HRESULT CPerfMon::_UnregisterStrings() throw()
{
	_ATLTRY
	{
		CString str;
		HRESULT hr;
		DWORD dwErr;

		// unregister the PerfMon counter and help strings
		CRegKey rkApp;

		str.Format(c_szAtlPerfPerformanceKey, GetAppName());
		dwErr = rkApp.Open(HKEY_LOCAL_MACHINE, str);
		//The register strings was unregistered.
		if (dwErr == ERROR_FILE_NOT_FOUND)
			return S_OK;
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		DWORD dwFirstAppCounter;
		DWORD dwFirstAppHelp;
		DWORD dwLastAppCounter;
		DWORD dwLastAppHelp;

		dwErr = rkApp.QueryDWORDValue(c_szAtlPerfFirstCounter, dwFirstAppCounter);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		dwErr = rkApp.QueryDWORDValue(c_szAtlPerfFirstHelp, dwFirstAppHelp);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		dwErr = rkApp.QueryDWORDValue(c_szAtlPerfLastCounter, dwLastAppCounter);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		dwErr = rkApp.QueryDWORDValue(c_szAtlPerfLastHelp, dwLastAppHelp);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		// iterate through the installed languages and delete them all
		DWORD nChars = 0;
		dwErr = rkApp.QueryStringValue(c_szAtlPerfLanguages, NULL, &nChars);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		CString strLangs;
		dwErr = rkApp.QueryStringValue(c_szAtlPerfLanguages, CStrBuf(strLangs, nChars, CStrBuf::SET_LENGTH), &nChars);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);

		int nIndex = 0;
		CString strLang = strLangs.Tokenize(_T(" "), nIndex);
		while (!strLang.IsEmpty())
		{
			CRegKey rkLang;

			dwErr = rkLang.Open(HKEY_LOCAL_MACHINE, CString(c_szAtlPerfPerfLibKey) + _T("\\") + strLang);
			if (dwErr != ERROR_SUCCESS)
				return AtlHresultFromWin32(dwErr);

			hr = _RemoveRegStrings(rkLang, c_szAtlPerfCounter, dwFirstAppCounter, dwLastAppCounter);
			if (FAILED(hr))
				return hr;

			hr = _RemoveRegStrings(rkLang, c_szAtlPerfHelp, dwFirstAppHelp, dwLastAppHelp);
			if (FAILED(hr))
				return hr;

			strLang = strLangs.Tokenize(_T(" "), nIndex);
		}

		dwErr = rkApp.SetStringValue(c_szAtlPerfLanguages, _T(""));
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);
			
		return S_OK;
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}
}

inline HRESULT CPerfMon::Unregister() throw()
{
	CString str;
	HRESULT hr;
	DWORD dwErr;

	CRegKey rkPerfLib;
	CRegKey rkApp;

	hr = _UnregisterStrings();
	if (FAILED(hr))
		return hr;

	dwErr = rkPerfLib.Open(HKEY_LOCAL_MACHINE, c_szAtlPerfPerfLibKey);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	_ATLTRY
	{
		str.Format(c_szAtlPerfPerformanceKey, GetAppName());
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}
	dwErr = rkApp.Open(HKEY_LOCAL_MACHINE, str);
	// The performance counter was unregistered
	if (dwErr == ERROR_FILE_NOT_FOUND)
		return S_OK;
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	DWORD dwLastCounter;
	DWORD dwLastHelp;
	DWORD dwFirstAppCounter;
	DWORD dwFirstAppHelp;
	DWORD dwLastAppCounter;
	DWORD dwLastAppHelp;

	dwErr = rkPerfLib.QueryDWORDValue(c_szAtlPerfLastCounter, dwLastCounter);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkPerfLib.QueryDWORDValue(c_szAtlPerfLastHelp, dwLastHelp);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkApp.QueryDWORDValue(c_szAtlPerfFirstCounter, dwFirstAppCounter);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkApp.QueryDWORDValue(c_szAtlPerfFirstHelp, dwFirstAppHelp);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkApp.QueryDWORDValue(c_szAtlPerfLastCounter, dwLastAppCounter);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkApp.QueryDWORDValue(c_szAtlPerfLastHelp, dwLastAppHelp);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	// rewind the Last Help/Last Counter values if possible
	if (dwLastCounter == dwLastAppCounter)
	{
		dwErr = rkPerfLib.SetDWORDValue(c_szAtlPerfLastCounter, dwFirstAppCounter-2);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);
	}

	if (dwLastHelp == dwLastAppHelp)
	{
		dwErr = rkPerfLib.SetDWORDValue(c_szAtlPerfLastHelp, dwFirstAppHelp-2);
		if (dwErr != ERROR_SUCCESS)
			return AtlHresultFromWin32(dwErr);
	}
	rkApp.Close();

	// delete the app key
	CRegKey rkServices;

	_ATLTRY
	{
		str.Format(c_szAtlPerfServicesKey, GetAppName());
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}
	dwErr = rkServices.Open(HKEY_LOCAL_MACHINE, str);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	dwErr = rkServices.RecurseDeleteKey(c_szAtlPerfPerformance);
	if (dwErr != ERROR_SUCCESS)
		return AtlHresultFromWin32(dwErr);

	return S_OK;
}
#endif

inline HRESULT CPerfMon::Initialize() throw()
{
	CMutex tempLock;
	CString strAppName;
	HRESULT hr;

	_ATLTRY
	{
		strAppName = GetAppName();

		ATLASSUME(m_aMem.GetCount() == 0);

		CAccessToken at;
		if (!at.GetEffectiveToken(TOKEN_QUERY))
			return E_FAIL;

	 
		CSid self;
		if (!at.GetUser(&self))
			return E_FAIL;

		// set up security information for creating the mutex
		CDacl dacl;
		 
 
		dacl.AddAllowedAce(Sids::NetworkService(),GENERIC_READ);  
		dacl.AddAllowedAce(Sids::Admins(), GENERIC_ALL);
		dacl.AddAllowedAce(Sids::System(), GENERIC_ALL);
		dacl.AddAllowedAce(self, GENERIC_ALL);

		m_sd.SetDacl(dacl);
		m_sd.SetOwner(self);

		CSecurityAttributes sa;
		sa.Set(m_sd);

		// create a mutex to handle syncronizing access to the shared memory area
		CString strMutexName;
		strMutexName.Format(_T("Global\\ATLPERF_%s_LOCK"), strAppName);
		tempLock.Create(&sa, FALSE, strMutexName);
		if (tempLock.m_h == NULL)
			return AtlHresultFromLastError();

		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			// prevent us from using an object someone else has opened
			if (::SetSecurityInfo(tempLock, SE_KERNEL_OBJECT,
					DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
					const_cast<SID*>(self.GetPSID()),
					NULL,
					const_cast<ACL*>(dacl.GetPACL()),
					NULL) != ERROR_SUCCESS)
				return E_FAIL;
		}

		// now set up the dacl for creating shared memory segments and store it
		dacl.AddAllowedAce(Sids::Interactive(), GENERIC_READ);
		m_sd.SetDacl(dacl);

		// create a shared memory area to share data between the app being measured and the client doing the measuring
		{
			CMutexLock lock(tempLock);

			BOOL bExisted = FALSE;

			CAtlFileMappingBase* pMem;
			pMem = _AllocNewBlock(NULL, &bExisted);
			if (pMem == NULL)
				return E_OUTOFMEMORY;

			if (!bExisted)
			{
				// copy the map from the registry to the shared memory
				CRegKey rkApp;
				DWORD dwErr;
				CString strAppKey;

				strAppKey.Format(c_szAtlPerfPerformanceKey, GetAppName());

				dwErr = rkApp.Open(HKEY_LOCAL_MACHINE, strAppKey, KEY_READ);
				if (dwErr != ERROR_SUCCESS)
				{
					m_aMem.RemoveAll();
					return AtlHresultFromWin32(dwErr);
				}

				ULONG nBytes = m_nAllocSize;
				dwErr = rkApp.QueryBinaryValue(c_szAtlPerfMap, pMem->GetData(), &nBytes);
				if (dwErr != ERROR_SUCCESS)
				{
					m_aMem.RemoveAll();
					return AtlHresultFromWin32(dwErr);
				}
			}

			hr = _LoadMap(LPDWORD(pMem->GetData()));
			if (FAILED(hr))
			{
				m_aMem.RemoveAll();
				return hr;
			}

			m_nSchemaSize = *LPDWORD(pMem->GetData());
			m_nHeaderSize = m_nSchemaSize + sizeof(DWORD);
			m_nHeaderSize  = AtlAlignUp(m_nHeaderSize,16);
		}

		m_lock.Attach(tempLock.Detach());
	}
	_ATLCATCHALL()
	{
		m_aMem.RemoveAll();
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

inline void CPerfMon::UnInitialize() throw()
{
	if (m_lock.m_h != NULL)
		m_lock.Close();
	m_aMem.RemoveAll();
	ClearMap();
}

inline HRESULT CPerfMon::_CreateInstance(
	DWORD dwCategoryId,
	DWORD dwInstance,
	LPCWSTR szInstanceName,
	CPerfObject** ppInstance,
	bool bByName
	) throw()
{
	CPerfObject* pEmptyBlock = NULL;

	if (ppInstance == NULL)
		return E_POINTER;

	CAtlFileMappingBase* pCurrentBlock = _GetNextBlock(NULL);
	if (pCurrentBlock == NULL || pCurrentBlock->GetData() == NULL || m_lock.m_h == NULL)
		return E_UNEXPECTED; // Initialize must succeed before calling CreateInstance

	*ppInstance = NULL;

	CategoryInfo* pCategoryInfo = _FindCategoryInfo(dwCategoryId);
	if (pCategoryInfo == NULL)
		return E_INVALIDARG;
	if (szInstanceName == NULL && bByName)
		return E_INVALIDARG;
	if (pCategoryInfo->m_nInstanceLess == PERF_NO_INSTANCES &&
			(dwInstance != 0 || szInstanceName != NULL))
		return E_INVALIDARG;

	CPerfLock lock(this);
	if (FAILED(lock.GetStatus()))
		return lock.GetStatus();

	CPerfObject* pInstance = _GetFirstInstance(pCurrentBlock);
	ULONG nMaxInstance = 0;
	ULONG nUsedSpace = 0;

	// walk all of the existing objects trying to find one that matches the request
	while (pInstance->m_nAllocSize != 0)
	{
		nUsedSpace += pInstance->m_nAllocSize;

		if (pInstance->m_dwCategoryId == dwCategoryId)
		{
			nMaxInstance = __max(nMaxInstance, pInstance->m_dwInstance);

			// check to see if we've found the one the caller wants
			if (!bByName && pInstance->m_dwInstance == dwInstance &&
				(pCategoryInfo->m_nInstanceLess == PERF_NO_INSTANCES || dwInstance != 0))
			{
				*ppInstance = pInstance;
				pInstance->m_nRefCount++;
				return S_OK;
			}
			if (bByName)
			{
				LPWSTR szInstName = (LPWSTR(LPBYTE(pInstance)+pInstance->m_nInstanceNameOffset));
				if (wcsncmp(szInstName, szInstanceName, pCategoryInfo->m_nMaxInstanceNameLen-1) == 0)
				{
					*ppInstance = pInstance;
					pInstance->m_nRefCount++;
					return S_OK;
				}
			}
		}

		if (pInstance->m_nAllocSize == pCategoryInfo->m_nAllocSize && pInstance->m_nRefCount == 0)
			pEmptyBlock = pInstance;

		pInstance = _GetNextInstance(pInstance);
		ATLENSURE_RETURN(pInstance!= NULL);

		if (pInstance->m_nAllocSize == 0 &&
			m_nHeaderSize + nUsedSpace + pCategoryInfo->m_nAllocSize + sizeof(CPerfObject) > m_nAllocSize)
		{
			// we've reached the end of the block and have no room to allocate an object of this
			// type. cap the block with a sentinel
			pInstance->m_nAllocSize = (ULONG) -1;
		}

		// check for an end-of-shared-mem sentinel
		if (pInstance->m_nAllocSize == (ULONG) -1)
		{
			nUsedSpace = 0;
			CAtlFileMappingBase* pNextBlock = _GetNextBlock(pCurrentBlock);
			if (pNextBlock == NULL)
			{
				// we've reached the last block of shared mem.
				// the instance hasn't been found, so either use a
				// previously freed instance block (pEmptyBlock) or allocate a new
				// shared mem block to hold the new instance
				if (pEmptyBlock == NULL)
				{
					pNextBlock = _AllocNewBlock(pCurrentBlock);
					if (pNextBlock == NULL)
						return E_OUTOFMEMORY;
				}
				else
					break;
			}
			pCurrentBlock = pNextBlock;
			pInstance = _GetFirstInstance(pCurrentBlock);
		}
	}

	// allocate a new object
	if (pEmptyBlock != NULL)
		pInstance = pEmptyBlock;
	else
		pInstance->m_nAllocSize = pCategoryInfo->m_nAllocSize;

	if (dwInstance == 0 && pCategoryInfo->m_nInstanceLess != PERF_NO_INSTANCES)
		pInstance->m_dwInstance = nMaxInstance + 1;
	else
		pInstance->m_dwInstance = dwInstance;

	pInstance->m_nRefCount = 1;

	// copy the instance name, truncate if necessary
	if (pCategoryInfo->m_nInstanceLess != PERF_NO_INSTANCES)
	{
		ULONG nNameLen = (ULONG)__min(wcslen(szInstanceName), pCategoryInfo->m_nMaxInstanceNameLen-1);
		ULONG nNameBytes = (nNameLen+1) * sizeof(WCHAR);
		pInstance->m_nInstanceNameOffset = pInstance->m_nAllocSize-nNameBytes;
		Checked::memcpy_s(LPBYTE(pInstance)+pInstance->m_nInstanceNameOffset, pInstance->m_nAllocSize-pInstance->m_nInstanceNameOffset, szInstanceName, nNameBytes);
		LPWSTR(LPBYTE(pInstance)+pInstance->m_nInstanceNameOffset)[nNameLen] = 0;
	}

	// copy the CategoryId last: it won't be collected until this is set
	pInstance->m_dwCategoryId = pCategoryInfo->m_dwCategoryId;

	*ppInstance = pInstance;

	return S_OK;
}

inline HRESULT CPerfMon::CreateInstance(
	DWORD dwCategoryId,
	DWORD dwInstance,
	LPCWSTR szInstanceName,
	CPerfObject** ppInstance
	) throw()
{
	return _CreateInstance(dwCategoryId, dwInstance, szInstanceName, ppInstance, false);
}

inline HRESULT CPerfMon::CreateInstanceByName(
	DWORD dwCategoryId,
	LPCWSTR szInstanceName,
	CPerfObject** ppInstance
	) throw()
{
	return _CreateInstance(dwCategoryId, 0, szInstanceName, ppInstance, true);
}

inline HRESULT CPerfMon::ReleaseInstance(CPerfObject* pInstance) throw()
{
	ATLASSERT(pInstance != NULL);
	if (pInstance == NULL)
		return E_INVALIDARG;

	CPerfLock lock(this);
	if (FAILED(lock.GetStatus()))
		return lock.GetStatus();

	if (--pInstance->m_nRefCount == 0)
	{
		pInstance->m_dwInstance = 0;
		pInstance->m_dwCategoryId = 0;
	}

	return S_OK;
}

inline HRESULT CPerfMon::LockPerf(DWORD dwTimeout /* == INFINITE */) throw()
{
	if (m_lock.m_h == NULL)
		return E_UNEXPECTED;

	DWORD dwRes = WaitForSingleObject(m_lock.m_h, dwTimeout);
	if (dwRes == WAIT_ABANDONED || dwRes == WAIT_OBJECT_0)
		return S_OK;
	if (dwRes == WAIT_TIMEOUT)
		return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
	return AtlHresultFromLastError();
}

inline void CPerfMon::UnlockPerf() throw()
{
	m_lock.Release();
}

// map building routines
inline HRESULT CPerfMon::AddCategoryDefinition(
	DWORD dwCategoryId,
	LPCTSTR szCategoryName,
	LPCTSTR szHelpString,
	DWORD dwDetailLevel,
	INT nDefaultCounter,
	BOOL bInstanceLess,
	UINT nStructSize,
	UINT nMaxInstanceNameLen) throw()
{
	// must have one and only one of these
	ATLASSERT(!bInstanceLess ^ !nMaxInstanceNameLen);

	// get the things that can fail out of the way first
	CString strName;
	CString strHelp;
	_ATLTRY
	{
		strName = szCategoryName;
		strHelp = szHelpString;
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}

	if (!m_categories.SetCount(m_categories.GetCount()+1))
	{
		return E_OUTOFMEMORY;
	}

	// category has been added, set the data
	CategoryInfo* pCategoryInfo = _GetCategoryInfo(_GetNumCategories()-1);

	pCategoryInfo->m_dwCategoryId = dwCategoryId;
	pCategoryInfo->m_dwDetailLevel = dwDetailLevel;
	pCategoryInfo->m_nDefaultCounter = nDefaultCounter;
	pCategoryInfo->m_nInstanceLess = bInstanceLess ? PERF_NO_INSTANCES : 0;
	pCategoryInfo->m_nStructSize = nStructSize;
	pCategoryInfo->m_nMaxInstanceNameLen = nMaxInstanceNameLen;
	pCategoryInfo->m_nAllocSize = nStructSize + nMaxInstanceNameLen*sizeof(WCHAR);
	pCategoryInfo->m_strName = strName;
	pCategoryInfo->m_strHelp = strHelp;
	pCategoryInfo->m_nNameId = 0;
	pCategoryInfo->m_nHelpId = 0;

	return S_OK;
}

inline HRESULT CPerfMon::AddCounterDefinition(
	DWORD dwCounterId,
	LPCTSTR szCounterName,
	LPCTSTR szHelpString,
	DWORD dwDetailLevel,
	DWORD dwCounterType,
	ULONG nMaxCounterSize,
	UINT nOffset,
	INT nDefaultScale) throw()
{
	// must add category BEFORE adding counter!
	ATLASSERT(_GetNumCategories() > 0);

	CounterInfo counter;

	counter.m_dwCounterId = dwCounterId;
	_ATLTRY
	{
		counter.m_strName = szCounterName;
		counter.m_strHelp = szHelpString;
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}
	counter.m_dwDetailLevel = dwDetailLevel;
	counter.m_dwCounterType = dwCounterType;
	counter.m_nDefaultScale = nDefaultScale;
	counter.m_nMaxCounterSize = nMaxCounterSize;
	counter.m_nDataOffset = nOffset;

	counter.m_nNameId = 0;
	counter.m_nHelpId = 0;

	// add the counter to the category
	CategoryInfo* pCategoryInfo = _GetCategoryInfo(_GetNumCategories()-1);
	_ATLTRY
	{
		pCategoryInfo->m_counters.Add(counter);
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}

	if (counter.m_nMaxCounterSize > 0)
	{
		ATLASSERT(counter.m_dwCounterType & PERF_TYPE_TEXT);
		pCategoryInfo->m_nAllocSize += counter.m_nMaxCounterSize * sizeof(WCHAR);
	}

	return S_OK;
}

inline HRESULT CPerfMon::RegisterCategory(
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
	INT nDefaultCounter) throw()
{
	if (pSampleRes)
		*pSampleRes = nNameString;
   
	CString strName;
	CString strHelp;
   
	_ATLTRY
	{
		 
		if (!strName.LoadString(hResInstance, nNameString, wLanguage) ||
			!strHelp.LoadString(hResInstance, nHelpString, wLanguage))
		{
			return E_FAIL;
		}
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}

	return RegisterCategory(
		wLanguage,
		hResInstance,
		pSampleRes,
		dwCategoryId,
		strName,
		strHelp,
		dwDetail,
		bInstanceless,
		nStructSize,
		nMaxInstanceNameLen,
		nDefaultCounter);
}

inline HRESULT CPerfMon::RegisterCategory(
	WORD /* wLanguage */,
	HINSTANCE /* hResInstance */,
	UINT* /* pSampleRes */,
	DWORD dwCategoryId,
	LPCTSTR szNameString,
	LPCTSTR szHelpString,
	DWORD dwDetail,
	BOOL bInstanceless,
	UINT nStructSize,
	UINT nMaxInstanceNameLen,
	INT nDefaultCounter) throw()
{
	return AddCategoryDefinition(
		dwCategoryId,
		szNameString,
		szHelpString,
		dwDetail,
		nDefaultCounter,
		bInstanceless,
		nStructSize,
		nMaxInstanceNameLen);
}

inline HRESULT CPerfMon::RegisterCounter(
	WORD wLanguage,
	HINSTANCE hResInstance,
	DWORD dwCounterId,
	UINT nNameString,
	UINT nHelpString,
	DWORD dwDetail,
	DWORD dwCounterType,
	ULONG nMaxCounterSize,
	UINT nOffset,
	INT nDefaultScale) throw()
{
	CString strName;
	CString strHelp;
  
	_ATLTRY
	{
		 
		if (!strName.LoadString(hResInstance, nNameString, wLanguage) ||
			!strHelp.LoadString(hResInstance, nHelpString, wLanguage))
		{
			return E_FAIL;
		}
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}

	return RegisterCounter(
		wLanguage,
		hResInstance,
		dwCounterId,
		strName,
		strHelp,
		dwDetail,
		dwCounterType,
		nMaxCounterSize,
		nOffset,
		nDefaultScale);
}

inline HRESULT CPerfMon::RegisterCounter(
	WORD /* wLanguage */,
	HINSTANCE /* hResInstance */,
	DWORD dwCounterId,
	LPCTSTR szNameString,
	LPCTSTR szHelpString,
	DWORD dwDetail,
	DWORD dwCounterType,
	ULONG nMaxCounterSize,
	UINT nOffset,
	INT nDefaultScale) throw()
{
	return AddCounterDefinition(
		dwCounterId,
		szNameString,
		szHelpString,
		dwDetail,
		dwCounterType,
		nMaxCounterSize,
		nOffset,
		nDefaultScale);
}

inline void CPerfMon::ClearMap() throw()
{
	m_categories.RemoveAll();
}

#ifndef _ATL_PERF_NOXML

ATL_NOINLINE inline HRESULT CPerfMon::PersistToXML(IStream *pStream, BOOL bFirst/*=TRUE*/, BOOL bLast/*=TRUE*/) throw(...)
{
	ATLASSERT(pStream != NULL);
	if (pStream == NULL)
		return E_INVALIDARG;

	CPerfLock lock(this);
	if (FAILED(lock.GetStatus()))
		return ERROR_SUCCESS;

	CStringA strXML;
	HRESULT hr = S_OK;
	ULONG nLen = 0;
	
	if (bFirst)
	{
		strXML = "<?xml version=\"1.0\" ?>\r\n<perfPersist>\r\n";
		hr = pStream->Write(strXML, strXML.GetLength(), &nLen);
		if (hr != S_OK)
			return hr;
	}

	strXML.Format("\t<perfmon name=\"%s\">\r\n", CT2CA(GetAppName()));
	hr = pStream->Write(strXML, strXML.GetLength(), &nLen);

	for (UINT i=0; i<_GetNumCategories(); i++)
	{
		CategoryInfo* pCategoryInfo = _GetCategoryInfo(i);

		CAtlFileMappingBase *pCurrentBlock = _GetNextBlock(NULL);
		CPerfObject *pInstance = _GetFirstInstance(pCurrentBlock);

		strXML.Format("\t\t<perfObject perfid=\"%d\">\r\n", 
			pCategoryInfo->m_dwCategoryId, pCategoryInfo->m_nNameId, pCategoryInfo->m_nHelpId);

		hr = pStream->Write(strXML, strXML.GetLength(), &nLen);
		if (hr != S_OK)
			return E_FAIL;

		while (pInstance && pInstance->m_nAllocSize)
		{
			if (pInstance->m_dwCategoryId == pCategoryInfo->m_dwCategoryId)
			{
				if (pCategoryInfo->m_nInstanceLess != PERF_NO_INSTANCES)
				{
					// handle the instance name
					LPCWSTR wszInstNameSrc = LPCWSTR(LPBYTE(pInstance)+pInstance->m_nInstanceNameOffset);
					int nInstLen = (int) wcslen(wszInstNameSrc);

					// convert to UTF8
					int nLength = AtlUnicodeToUTF8(wszInstNameSrc, nInstLen, NULL, 0);
					CHeapPtr<CHAR> szUTF8;
					if ((nLength < 0) || (nLength+1<nLength) || !szUTF8.Allocate(nLength+1))
						return E_OUTOFMEMORY;
					nLength = AtlUnicodeToUTF8(wszInstNameSrc, nInstLen, szUTF8, nLength);
					szUTF8[nLength] = '\0';

					strXML.Format("\t\t\t<instance name=\"%s\" id=\"%d\">\r\n", szUTF8, pInstance->m_dwInstance);
					hr = pStream->Write(strXML, strXML.GetLength(), &nLen);
					if (hr != S_OK)
						return hr;
				}

				for (UINT j=0; j<pCategoryInfo->_GetNumCounters(); j++)
				{
					CounterInfo *pCounterInfo = pCategoryInfo->_GetCounterInfo(j);
					switch (pCounterInfo->m_dwCounterType & ATLPERF_SIZE_MASK)
					{
						case PERF_SIZE_DWORD:
						{
							strXML.Format("\t\t\t\t<counter type=\"perf_size_dword\" value=\"%d\" offset=\"%d\"/>\r\n",
								*LPDWORD(LPBYTE(pInstance)+pCounterInfo->m_nDataOffset), 
								pCounterInfo->m_nDataOffset);
							break;
						}
						case PERF_SIZE_LARGE:
						{
							strXML.Format("\t\t\t\t<counter type=\"perf_size_large\" value=\"%d\" offset=\"%d\"/>\r\n",
								*PULONGLONG(LPBYTE(pInstance)+pCounterInfo->m_nDataOffset),
								pCounterInfo->m_nDataOffset);
							break;
						}
						case PERF_SIZE_VARIABLE_LEN:
						{
							CHeapPtr<CHAR> szUTF8;
							LPBYTE pSrc = LPBYTE(pInstance)+pCounterInfo->m_nDataOffset;
							if ((pCounterInfo->m_dwCounterType & ATLPERF_TEXT_MASK) == PERF_TEXT_UNICODE)
							{
								ULONG nTextLen = (ULONG)wcslen(LPCWSTR(pSrc));
								// convert to UTF8
								nLen = AtlUnicodeToUTF8(LPCWSTR(pSrc), nTextLen, NULL, 0);
								if (!szUTF8.Allocate(nLen+1))
									return E_OUTOFMEMORY;

								nLen = AtlUnicodeToUTF8(LPCWSTR(pSrc), nTextLen, szUTF8, nLen);	
								szUTF8[nLen] = '\0';
								strXML.Format("\t\t\t\t<counter type=\"perf_size_variable_len_unicode\" value=\"%s\" offset=\"%d\"/>\r\n",
										szUTF8,
										pCounterInfo->m_nDataOffset);
							}
							else
							{
								ULONG nTextLen = (ULONG)strlen(LPCSTR(pSrc));
								if (!szUTF8.Allocate(nTextLen+1))
									return E_OUTOFMEMORY;
								Checked::strcpy_s(szUTF8, nTextLen+1, LPCSTR(pSrc));
								strXML.Format("\t\t\t\t<counter type=\"perf_size_variable_len_ansi\" value=\"%s\" offset=\"%d\"/>\r\n",
										szUTF8,
										pCounterInfo->m_nDataOffset);
							}
							break;
						}
						default:
							// error:
							return E_FAIL;
					}
					hr = pStream->Write(strXML, strXML.GetLength(), &nLen);
					if (hr != S_OK)
						return hr;
				}

				if (pCategoryInfo->m_nInstanceLess != PERF_NO_INSTANCES)
				{
					hr = pStream->Write("\t\t\t</instance>\r\n", sizeof("\t\t\t</instance>\r\n")-1, &nLen);
					if (hr != S_OK)
						return hr;
				}
			}

			pInstance = _GetNextInstance(pInstance);
			ATLENSURE_RETURN(pInstance!= NULL);

			if (pInstance->m_nAllocSize == (ULONG)-1)
			{
				pCurrentBlock = _GetNextBlock(pCurrentBlock);
				if (pCurrentBlock == NULL)
					pInstance = NULL;
				else
					pInstance = _GetFirstInstance(pCurrentBlock);
			}
		}

		hr = pStream->Write("\t\t</perfObject>\r\n", sizeof("\t\t</perfObject>\r\n")-1, &nLen);
		if (hr != S_OK)
			return hr;
	}

	hr = pStream->Write("\t</perfmon>\r\n", sizeof("\t</perfmon>\r\n")-1, &nLen);
	if (hr != S_OK)
		return hr;

	if (hr == S_OK && bLast)
		hr = pStream->Write("</perfPersist>", sizeof("</perfPersist>")-1, &nLen);

	return hr;
}

// This function is very lenient with inappropriate XML
ATL_NOINLINE inline HRESULT CPerfMon::LoadFromXML(IStream *pStream) throw(...)
{	
	ATLASSERT(pStream != NULL);
	if (pStream == NULL)
		return E_INVALIDARG;

	// Get a lock
	CPerfLock lock(this);
	if (FAILED(lock.GetStatus()))
		return ERROR_SUCCESS;

	CComPtr<IXMLDOMDocument> spdoc;

	// load the xml
	HRESULT hr = CoCreateInstance(__uuidof(DOMDocument), NULL, CLSCTX_INPROC, __uuidof(IXMLDOMDocument), (void **) &spdoc);
	if (FAILED(hr))
	{
		return hr;
	}

	spdoc->put_async(VARIANT_FALSE);

	CComPtr<IPersistStreamInit> spSI;
	hr = spdoc->QueryInterface(&spSI);
	if (hr != S_OK)
		return hr;
	hr = spSI->Load(pStream);
	if (hr != S_OK)
		return hr;

	// validate that it is a perfPersist stream
	CComPtr<IXMLDOMElement> spRoot;

	hr = spdoc->get_documentElement(&spRoot);
	if (hr != S_OK)
		return hr;

	CComBSTR bstrName;
	hr = spRoot->get_baseName(&bstrName);
	if (wcscmp(bstrName, L"perfPersist"))
		return S_FALSE;

	// find the appropriate perfmon node
	CComPtr<IXMLDOMNode> spChild;
	hr = spRoot->get_firstChild(&spChild);
	while (hr == S_OK)
	{
		bstrName.Empty();
		hr = spChild->get_baseName(&bstrName);
		if (hr == S_OK)
		{
			if (!wcscmp(bstrName, L"perfmon"))
			{
				bstrName.Empty();
				hr = _GetAttribute(spChild, L"name", &bstrName);
				if (hr == S_OK)
				{
					if (!_tcscmp(CW2CT(bstrName), GetAppName()))
						break;
				}
			}
		}

		CComPtr<IXMLDOMNode> spNext;
		hr = spChild->get_nextSibling(&spNext);
		spChild.Attach(spNext.Detach());
	}

	// there is no perfmon node in the XML for the current CPerfMon class
	if (hr != S_OK)
		return S_FALSE;

	CComPtr<IXMLDOMNode> spPerfRoot;
	spPerfRoot.Attach(spChild.Detach());

	// iterate over the objects in the perfmon subtree
	// this is the loop that does the real work
	hr = spPerfRoot->get_firstChild(&spChild);
	while (hr == S_OK)
	{
		// see if it's a perfObject
		bstrName.Empty();
		hr = spChild->get_baseName(&bstrName);
		if (hr != S_OK || wcscmp(bstrName, L"perfObject"))
			return S_FALSE;

		// get the perfid
		bstrName.Empty();
		hr = _GetAttribute(spChild, L"perfid", &bstrName);
		DWORD dwPerfId = _wtoi(bstrName);

		// iterate over children
		CComPtr<IXMLDOMNode> spInstChild;
		hr = spChild->get_firstChild(&spInstChild);
		while (hr == S_OK)
		{
			// see if it's a instance
			bstrName.Empty();
			hr = spInstChild->get_baseName(&bstrName);
			if (hr != S_OK || wcscmp(bstrName, L"instance"))
				return S_FALSE;

			// get the instance name
			bstrName.Empty();
			hr = _GetAttribute(spInstChild, L"name", &bstrName);
			if (hr != S_OK)
				return S_FALSE;

			// get the instance id
			bstrName.Empty();
			hr = _GetAttribute(spChild, L"id", &bstrName);
			if (hr != S_OK)
				return S_FALSE;
			DWORD dwInstance = _wtoi(bstrName);

			// create the instance
			CPerfObject *pInstance = NULL;
			hr = CreateInstance(dwPerfId, dwInstance++, bstrName, &pInstance);
			if (hr != S_OK)
				return S_FALSE;

			// iterate over the counters and set the data
			CComPtr<IXMLDOMNode> spCntrChild;
			hr = spInstChild->get_firstChild(&spCntrChild);
			while (hr == S_OK)
			{
				// get the base name
				bstrName.Empty();
				hr = spCntrChild->get_baseName(&bstrName);
				if (hr != S_OK || wcscmp(bstrName, L"counter"))
					return S_FALSE;

				// get the type
				bstrName.Empty();
				hr = _GetAttribute(spCntrChild, L"type", &bstrName);
				if (hr != S_OK)
					return S_FALSE;

				DWORD dwType;
				if (!wcscmp(bstrName, L"perf_size_dword"))
					dwType = PERF_SIZE_DWORD;
				else if (!wcscmp(bstrName, L"perf_size_large"))
					dwType = PERF_SIZE_LARGE;
				else if (!wcscmp(bstrName, L"perf_size_variable_len_ansi"))
					dwType = PERF_SIZE_VARIABLE_LEN;
				else if (!wcscmp(bstrName, L"perf_size_variable_len_unicode"))
					dwType = PERF_SIZE_VARIABLE_LEN | PERF_TEXT_UNICODE;
				else
					return S_FALSE;

				// get the value
				bstrName.Empty();
				hr = _GetAttribute(spCntrChild, L"value", &bstrName);
				if (hr != S_OK)
					return S_FALSE;

				CComBSTR bstrOffset;
				hr = _GetAttribute(spCntrChild, L"offset", &bstrOffset);
				if (hr != S_OK)
					return S_FALSE;

				WCHAR *pStop = NULL;
				DWORD dwOffset = wcstoul(bstrOffset, &pStop, 10);

				if (dwType == PERF_SIZE_DWORD) // add it as a DWORD
				{
					DWORD dwVal = wcstoul(bstrName, &pStop, 10);
					*LPDWORD(LPBYTE(pInstance)+dwOffset) = dwVal;
				}
				else if (dwType == PERF_SIZE_LARGE) // add it is a ULONGLONG
				{
					ULONGLONG qwVal = _wcstoui64(bstrName, &pStop, 10);
					*PULONGLONG(LPBYTE(pInstance)+dwOffset) = qwVal;
				}
				else if (dwType == PERF_SIZE_VARIABLE_LEN) // add it as an ansi string
				{
					AtlW2AHelper(LPSTR(LPBYTE(pInstance)+dwOffset), bstrName, bstrName.Length(), ATL::_AtlGetConversionACP());
				}
				else // add it as a unicode string
				{
					Checked::memcpy_s(LPBYTE(pInstance)+dwOffset, pInstance->m_nAllocSize-dwOffset, bstrName, bstrName.Length()*sizeof(WCHAR));
				}

				CComPtr<IXMLDOMNode> spCntrNext;
				hr = spCntrChild->get_nextSibling(&spCntrNext);
				spCntrChild.Attach(spCntrNext.Detach());
			}

			CComPtr<IXMLDOMNode> spInstNext;
			hr = spInstChild->get_nextSibling(&spInstNext);
			spInstChild.Attach(spInstNext.Detach());
		}

		CComPtr<IXMLDOMNode> spNext;
		hr = spChild->get_nextSibling(&spNext);
		spChild.Attach(spNext.Detach());
	}

	return S_OK;
}

// a little utility function to retrieve a named attribute from a node
ATL_NOINLINE inline HRESULT CPerfMon::_GetAttribute(IXMLDOMNode *pNode, LPCWSTR szAttrName, BSTR *pbstrVal) throw()
{
	ATLENSURE_RETURN(pNode != NULL);
	ATLASSERT(szAttrName != NULL);
	ATLENSURE_RETURN(pbstrVal != NULL);

	*pbstrVal = NULL;
	CComPtr<IXMLDOMNamedNodeMap> spAttrs;

	HRESULT hr = pNode->get_attributes(&spAttrs);
	if (hr != S_OK)
		return hr;
	
	CComPtr<IXMLDOMNode> spAttr;
	
	hr = spAttrs->getNamedItem((BSTR) szAttrName, &spAttr);
	if (hr != S_OK)
		return hr;
	
	CComVariant varVal;
	hr = spAttr->get_nodeValue(&varVal);
	if (hr != S_OK)
		return hr;
	
	hr = varVal.ChangeType(VT_BSTR);
	if (hr != S_OK)
		return hr;

	*pbstrVal = varVal.bstrVal;
	varVal.vt = VT_EMPTY;

	return S_OK;
}

#endif // _ATL_PERF_NOXML

#if defined(_ATL_PERF_REGISTER) & !defined(_ATL_PERF_NOEXPORT)

ATL_NOINLINE inline HRESULT RegisterPerfMon(HINSTANCE hDllInstance /* = _AtlBaseModule.GetModuleInstance() */) throw() 
{
	CPerfMon **ppPerf = &__pperfA; 
	HRESULT hr = S_OK; 
	while (ppPerf != &__pperfZ) 
	{ 
		if (*ppPerf != NULL) 
		{ 
			hr = (*ppPerf)->Register(_T( ATLPERF_FUNCID_OPEN ), _T( ATLPERF_FUNCID_COLLECT ), _T( ATLPERF_FUNCID_CLOSE ), hDllInstance);
			if (FAILED(hr)) 
				return hr; 
			hr = (*ppPerf)->RegisterAllStrings(hDllInstance);
			if (FAILED(hr)) 
				return hr; 
		} 
		ppPerf++; 
	} 
	return S_OK; 
} 

ATL_NOINLINE inline HRESULT UnregisterPerfMon() throw() 
{ 
	CPerfMon **ppPerf = &__pperfA; 
	HRESULT hr = S_OK; 
	while (ppPerf != &__pperfZ) 
	{ 
		if (*ppPerf != NULL) 
		{ 
			hr = (*ppPerf)->Unregister(); 
			if (FAILED(hr)) 
				return hr; 
		} 
		ppPerf++; 
	} 
	return S_OK; 
} 

extern "C" ATL_NOINLINE inline DWORD __declspec(dllexport) WINAPI OpenPerfMon(LPWSTR lpDeviceNames) throw()
{
	CPerfMon **ppPerf = &__pperfA;
	DWORD dwErr = 0;
	while (ppPerf != &__pperfZ)
	{
		if (*ppPerf != NULL)
		{
			dwErr = (*ppPerf)->Open(lpDeviceNames);
			if (dwErr != 0)
				return dwErr;
		}
		ppPerf++;
	}
	return 0;
}

extern "C" ATL_NOINLINE inline DWORD __declspec(dllexport) WINAPI CollectPerfMon(LPWSTR lpwszValue, LPVOID* lppData,
	LPDWORD lpcbBytes, LPDWORD lpcObjectTypes) throw()
{
	DWORD dwOrigBytes = *lpcbBytes;
	DWORD dwBytesRemaining = *lpcbBytes;
	CPerfMon **ppPerf = &__pperfA;
	DWORD dwErr = 0;
	while (ppPerf != &__pperfZ)
	{
		if (*ppPerf != NULL)
		{
			dwErr = (*ppPerf)->Collect(lpwszValue, lppData, lpcbBytes, lpcObjectTypes);
			if (dwErr != 0)
				return dwErr;
			dwBytesRemaining -= *lpcbBytes;
			*lpcbBytes = dwBytesRemaining;
		}
		ppPerf++;
	}
	*lpcbBytes = dwOrigBytes - dwBytesRemaining;
	return 0;
}

extern "C" ATL_NOINLINE inline DWORD __declspec(dllexport) WINAPI ClosePerfMon() throw()
{
	CPerfMon **ppPerf = &__pperfA;
	while (ppPerf != &__pperfZ)
	{
		if (*ppPerf != NULL)
		{
			(*ppPerf)->Close();
		}
		ppPerf++;
	}
	return 0;
}

#endif // defined(_ATL_PERF_REGISTER) & !defined(_ATL_PERF_NOEXPORT)

} // namespace ATL

#pragma warning(pop)

#endif // __ATLPERF_INL__
