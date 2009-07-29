// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLSESSION_H__
#define __ATLSESSION_H__

#pragma once
#pragma warning(push)
#pragma warning(disable: 4702) // unreachable code

#include <atldbcli.h>
#include <atlcom.h>
#include <atlstr.h>
#include <stdio.h>
#include <atlcoll.h>
#include <atltime.h>
#include <atlcrypt.h>
#include <atlenc.h>
#include <atlutil.h>
#include <atlcache.h>
#include <atlspriv.h>
#include <atlsiface.h>

#pragma warning(disable: 4625) // copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning(disable: 4626) // assignment operator could not be generated because a base class assignment operator is inaccessible

#ifndef MAX_SESSION_KEY_LEN 
	#define MAX_SESSION_KEY_LEN 128
#endif

#ifndef MAX_VARIABLE_NAME_LENGTH 
	#define MAX_VARIABLE_NAME_LENGTH 50
#endif

#ifndef MAX_VARIABLE_VALUE_LENGTH 
	#define MAX_VARIABLE_VALUE_LENGTH 1024
#endif

#ifndef MAX_CONNECTION_STRING_LEN
	#define MAX_CONNECTION_STRING_LEN 2048
#endif

#ifndef SESSION_COOKIE_NAME
	#define SESSION_COOKIE_NAME "SESSIONID"
#endif

#ifndef ATL_SESSION_TIMEOUT
	#define ATL_SESSION_TIMEOUT 600000 //10 min
#endif

#ifndef ATL_SESSION_SWEEPER_TIMEOUT
	#define ATL_SESSION_SWEEPER_TIMEOUT 1000 // 1sec
#endif

#define INVALID_DB_SESSION_POS 0x0
#define ATL_DBSESSION_ID _T("__ATL_SESSION_DB_CONNECTION")

#pragma pack(push,_ATL_PACKING)
namespace ATL {

// CSessionNameGenerator
// This is a helper class that generates random data for session key
// names. This class tries to use the CryptoApi to generate random
// bytes for the session key name. If the CryptoApi isn't available
// then the CRT rand() is used to generate the random bytes. This
// class's GetNewSessionName member function is used to actually
// generate the session name.
class CSessionNameGenerator :
	public CCryptProv
{
public:
	bool m_bCryptNotAvailable;
	enum {MIN_SESSION_KEY_LEN=5};

	CSessionNameGenerator() throw() :
		m_bCryptNotAvailable(false)
	{
		// Note that the crypto api is being
		// initialized with no private key
		// information
		HRESULT hr = InitVerifyContext();
		m_bCryptNotAvailable = FAILED(hr) ? true : false;
	}

	// This function creates a new session name and base64 encodes it.
	// The base64 encoding algorithm used needs at least MIN_SESSION_KEY_LEN
	// bytes to work correctly. Since we stack allocate the temporary
	// buffer that holds the key name, the buffer must be less than or equal to
	// the MAX_SESSION_KEY_LEN in size.
	HRESULT GetNewSessionName(__out_ecount_part_z(*pdwSize, *pdwSize) LPSTR szNewID, __inout DWORD *pdwSize) throw()
	{
		HRESULT hr = E_FAIL;

		if (!pdwSize)
			return E_POINTER;

		if (*pdwSize < MIN_SESSION_KEY_LEN ||
			*pdwSize > MAX_SESSION_KEY_LEN)
			return E_INVALIDARG;

		if (!szNewID)
			return E_POINTER;

		BYTE key[MAX_SESSION_KEY_LEN] = {0x0};


		// calculate the number of bytes that will fit in the
		// buffer we've been passed
		DWORD dwDataSize = CalcMaxInputSize(*pdwSize);

		if (dwDataSize && *pdwSize >= (DWORD)(Base64EncodeGetRequiredLength(dwDataSize,
			ATL_BASE64_FLAG_NOCRLF)))
		{
			int dwKeySize = *pdwSize;
			hr = GenerateRandomName(key, dwDataSize);
			if (SUCCEEDED(hr))
			{
				if( Base64Encode(key,
								dwDataSize,
								szNewID,
								&dwKeySize,
								ATL_BASE64_FLAG_NOCRLF) )
				{
					//null terminate
					szNewID[dwKeySize]=0;
					*pdwSize = dwKeySize+1;
				}
				else
					hr = E_FAIL;
			}
			else
			{
				*pdwSize = (DWORD)(Base64EncodeGetRequiredLength(dwDataSize,
					ATL_BASE64_FLAG_NOCRLF));
				return E_OUTOFMEMORY;
			}
		}
		return hr;
	}

	DWORD CalcMaxInputSize(DWORD nOutputSize) throw()
	{
		if (nOutputSize < (DWORD)MIN_SESSION_KEY_LEN)
			return 0;
		// subtract one from the output size to make room
		// for the NULL terminator in the output then
		// calculate the biggest number of input bytes that
		// when base64 encoded will fit in a buffer of size
		// nOutputSize (including base64 padding)
		int nInputSize = ((nOutputSize-1)*3)/4;
		int factor = ((nInputSize*4)/3)%4;
		if (factor)
			nInputSize -= factor;
		return nInputSize;
	}


	HRESULT GenerateRandomName(BYTE *pBuff, DWORD dwBuffSize) throw()
	{
		if (!pBuff)
			return E_POINTER;

		if (!dwBuffSize)
			return E_UNEXPECTED;

		if (!m_bCryptNotAvailable && GetHandle())
		{
			// Use the crypto api to generate random data.
			return GenRandom(dwBuffSize, pBuff);
		}

		// CryptoApi isn't available so we generate
		// random data using rand. We seed the random
		// number generator with a seed that is a combination
		// of bytes from an arbitrary number and the system
		// time which changes every millisecond so it will
		// be different for every call to this function.
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);
		static DWORD dwVal = 0x21;
		DWORD dwSeed = (dwVal++ << 0x18) | (ft.dwLowDateTime & 0x00ffff00) | dwVal++ & 0x000000ff;
		srand(dwSeed);
		BYTE *pCurr = pBuff;
		// fill buffer with random bytes
		for (int i=0; i < (int)dwBuffSize; i++)
		{
			*pCurr = (BYTE) (rand() & 0x000000ff);
			pCurr++;
		}
		return S_OK;
	}
};


//
// CDefaultQueryClass
// returns Query strings for use in SQL queries used 
// by the database persisted session service.
class CDefaultQueryClass
{
public:
	LPCTSTR GetSessionRefDelete() throw()
	{
		return 	_T("DELETE FROM SessionReferences ")
				_T("WHERE SessionID=? AND RefCount <= 0 ")
				_T("AND DATEDIFF(millisecond,  LastAccess, getdate()) > TimeoutMs");
	}

	LPCTSTR GetSessionRefIsExpired() throw()
	{
		return _T("SELECT SessionID FROM SessionReferences ")
			   _T("WHERE (SessionID=?) AND (DATEDIFF(millisecond,  LastAccess, getdate()) > TimeoutMs)");
	}

	LPCTSTR GetSessionRefDeleteFinal() throw()
	{
		return _T("DELETE FROM SessionReferences ")
			   _T("WHERE SessionID=?");
	}

	LPCTSTR GetSessionRefCreate() throw()
	{
		return _T("INSERT INTO SessionReferences ")
			_T("(SessionID, LastAccess, RefCount, TimeoutMs) ")
			_T("VALUES (?, getdate(), 1, ?)");
	}

	LPCTSTR GetSessionRefUpdateTimeout() throw()
	{
		return _T("UPDATE SessionReferences ")
			   _T("SET TimeoutMs=? WHERE SessionID=?");
	}

	LPCTSTR GetSessionRefAddRef() throw()
	{
		return _T("UPDATE SessionReferences ")
			_T("SET RefCount=RefCount+1, ")
			_T("LastAccess=getdate() ")
			_T("WHERE SessionID=?");
	}

	LPCTSTR GetSessionRefRemoveRef() throw()
	{
		return _T("UPDATE SessionReferences ")
					_T("SET RefCount=RefCount-1, ")
					_T("LastAccess=getdate() ")
					_T("WHERE SessionID=?");
	}

	LPCTSTR GetSessionRefAccess() throw()
	{
		return 	_T("UPDATE SessionReferences ")
				_T("SET LastAccess=getdate() ")
				_T("WHERE SessionID=?");
	}

	LPCTSTR GetSessionRefSelect() throw()
	{
		return _T("SELECT * FROM SessionReferences ")
			   _T("WHERE SessionID=?");
	}

	LPCTSTR GetSessionRefGetCount() throw()
	{
		return 	_T("SELECT COUNT(*) FROM SessionReferences");
	}


	LPCTSTR GetSessionVarCount() throw()
	{
		return _T("SELECT COUNT(*) FROM SessionVariables WHERE SessionID=?");
	}

	LPCTSTR GetSessionVarInsert() throw()
	{
		return  _T("INSERT INTO SessionVariables ")
				_T("(VariableValue, SessionID, VariableName) ")
				_T("VALUES (?, ?, ?)");
	}

	LPCTSTR GetSessionVarUpdate() throw()
	{
		return 	_T("UPDATE SessionVariables ")
				_T("SET VariableValue=? ")
				_T("WHERE SessionID=? AND VariableName=?");
	}

	LPCTSTR GetSessionVarDeleteVar() throw()
	{
		return _T("DELETE FROM SessionVariables ")
				_T("WHERE SessionID=? AND VariableName=?");
	}

	LPCTSTR GetSessionVarDeleteAllVars() throw()
	{
		return _T("DELETE FROM SessionVariables WHERE (SessionID=?)");
	}

	LPCTSTR GetSessionVarSelectVar()throw()
	{
		return _T("SELECT SessionID, VariableName, VariableValue ")
			   _T("FROM SessionVariables ")
			   _T("WHERE SessionID=? AND VariableName=?");
	}

	LPCTSTR GetSessionVarSelectAllVars() throw()
	{
		return _T("SELECT SessionID, VariableName, VariableValue ")
				_T("FROM SessionVariables ")
				_T("WHERE SessionID=?");
	}

	LPCTSTR GetSessionReferencesSet() throw()
	{
		return _T("UPDATE SessionReferences SET TimeoutMs=?");
	}
};


// Contains the data for the session variable accessors
class CSessionDataBase
{
public:
	TCHAR m_szSessionID[MAX_SESSION_KEY_LEN];
	TCHAR m_VariableName[MAX_VARIABLE_NAME_LENGTH];
	BYTE m_VariableValue[MAX_VARIABLE_VALUE_LENGTH];
	DBLENGTH m_VariableLen;
	CSessionDataBase() throw()
	{
		m_szSessionID[0] = '\0';
		m_VariableName[0] = '\0';
		m_VariableValue[0] = '\0';
		m_VariableLen = 0;
	}
	HRESULT Assign(LPCTSTR szSessionID, LPCTSTR szVarName, VARIANT *pVal) throw()
	{
		HRESULT hr = S_OK;
		CVariantStream stream;
		if ( szSessionID )
		{
			if (Checked::tcsnlen(szSessionID, MAX_SESSION_KEY_LEN)< MAX_SESSION_KEY_LEN)
				Checked::tcscpy_s(m_szSessionID, _countof(m_szSessionID), szSessionID);
			else
				hr = E_OUTOFMEMORY;
		}
		else
			return E_INVALIDARG;

		if (hr == S_OK && szVarName)
			if (Checked::tcsnlen(szVarName, MAX_VARIABLE_NAME_LENGTH) < MAX_VARIABLE_NAME_LENGTH)
				Checked::tcscpy_s(m_VariableName, _countof(m_VariableName), szVarName);
			else
				hr = E_OUTOFMEMORY;

		if (hr == S_OK && pVal)
		{
			hr = stream.InsertVariant(pVal);
			if (hr == S_OK)
			{
				BYTE *pBytes = stream.m_stream;
				size_t size = stream.GetVariantSize();
				if (pBytes && size && size < MAX_VARIABLE_VALUE_LENGTH)
				{
					Checked::memcpy_s(m_VariableValue, MAX_VARIABLE_VALUE_LENGTH, pBytes, size);
					m_VariableLen = static_cast<DBLENGTH>(size);
				}
				else
					hr = E_INVALIDARG;
			}
		}

		return hr;
	}
};

// Use to select a session variable given the name
// of a session and the name of a variable.
class CSessionDataSelector : public CSessionDataBase
{
public:
	BEGIN_COLUMN_MAP(CSessionDataSelector) 
		COLUMN_ENTRY(1, m_szSessionID)
		COLUMN_ENTRY(2, m_VariableName)
		COLUMN_ENTRY_LENGTH(3, m_VariableValue, m_VariableLen)
	END_COLUMN_MAP()
	BEGIN_PARAM_MAP(CSessionDataSelector) 
		SET_PARAM_TYPE(DBPARAMIO_INPUT)
		COLUMN_ENTRY(1, m_szSessionID)
		COLUMN_ENTRY(2, m_VariableName)
	END_PARAM_MAP()
};

// Use to select all session variables given the name of
// of a session.
class CAllSessionDataSelector : public CSessionDataBase
{
public:
	BEGIN_COLUMN_MAP(CAllSessionDataSelector) 
		COLUMN_ENTRY(1, m_szSessionID)
		COLUMN_ENTRY(2, m_VariableName)
		COLUMN_ENTRY_LENGTH(3, m_VariableValue, m_VariableLen)
	END_COLUMN_MAP()
	BEGIN_PARAM_MAP(CAllSessionDataSelector) 
		SET_PARAM_TYPE(DBPARAMIO_INPUT)
		COLUMN_ENTRY(1, m_szSessionID)
	END_PARAM_MAP()
};

// Use to update the value of a session variable
class CSessionDataUpdator : public CSessionDataBase
{
public:
	BEGIN_PARAM_MAP(CSessionDataUpdator) 
		SET_PARAM_TYPE(DBPARAMIO_INPUT)
		COLUMN_ENTRY_LENGTH(1, m_VariableValue, m_VariableLen)
		COLUMN_ENTRY(2, m_szSessionID)
		COLUMN_ENTRY(3, m_VariableName)
	END_PARAM_MAP()
};

// Use to delete a session variable given the
// session name and the name of the variable
class CSessionDataDeletor
{
public:
	CSessionDataDeletor()
	{
		m_szSessionID[0] = '\0';
		m_VariableName[0] = '\0';
	}

	TCHAR m_szSessionID[MAX_SESSION_KEY_LEN];
	TCHAR m_VariableName[MAX_VARIABLE_NAME_LENGTH];
	HRESULT Assign(LPCTSTR szSessionID, LPCTSTR szVarName) throw()
	{
		if (szSessionID)
		{
			if (Checked::tcsnlen(szSessionID, MAX_SESSION_KEY_LEN) < MAX_SESSION_KEY_LEN)
				Checked::tcscpy_s(m_szSessionID, _countof(m_szSessionID), szSessionID);
			else
				return E_OUTOFMEMORY;
		}

		if (szVarName)
		{
			if(Checked::tcsnlen(szVarName, MAX_VARIABLE_NAME_LENGTH) < MAX_VARIABLE_NAME_LENGTH)
				Checked::tcscpy_s(m_VariableName, _countof(m_VariableName), szVarName);
			else
				return E_OUTOFMEMORY;
		}
		return S_OK;
	}

	BEGIN_PARAM_MAP(CSessionDataDeletor) 
		SET_PARAM_TYPE(DBPARAMIO_INPUT)
		COLUMN_ENTRY(1, m_szSessionID)
		COLUMN_ENTRY(2, m_VariableName)
	END_PARAM_MAP()
};

class CSessionDataDeleteAll
{
public:
	TCHAR m_szSessionID[MAX_SESSION_KEY_LEN];
	HRESULT Assign(LPCTSTR szSessionID) throw()
	{
		if (!szSessionID)
			return E_INVALIDARG;

		if (Checked::tcsnlen(szSessionID, MAX_SESSION_KEY_LEN) < MAX_SESSION_KEY_LEN)
			Checked::tcscpy_s(m_szSessionID, _countof(m_szSessionID), szSessionID);
		else
			return E_OUTOFMEMORY;

		return S_OK;
	}

	BEGIN_PARAM_MAP(CSessionDataDeleteAll) 
		SET_PARAM_TYPE(DBPARAMIO_INPUT)
		COLUMN_ENTRY(1, m_szSessionID)
	END_PARAM_MAP()
};

// Used for retrieving the count of session variables for
// a given session ID.
class CCountAccessor
{
public:
	LONG m_nCount;
	TCHAR m_szSessionID[MAX_SESSION_KEY_LEN];
	CCountAccessor() throw()
	{
		m_szSessionID[0] = '\0';
		m_nCount = 0;
	}

	HRESULT Assign(LPCTSTR szSessionID) throw()
	{
		if (!szSessionID)
			return E_INVALIDARG;

		if (Checked::tcsnlen(szSessionID, MAX_SESSION_KEY_LEN) < MAX_SESSION_KEY_LEN)
			Checked::tcscpy_s(m_szSessionID, _countof(m_szSessionID), szSessionID);
		else
			return E_OUTOFMEMORY;

		return S_OK;
	}

	BEGIN_COLUMN_MAP(CCountAccessor)
		COLUMN_ENTRY(1, m_nCount)
	END_COLUMN_MAP()
	BEGIN_PARAM_MAP(CCountAccessor)
		SET_PARAM_TYPE(DBPARAMIO_INPUT)
		COLUMN_ENTRY(1, m_szSessionID)
	END_PARAM_MAP()
};


// Used for updating entries in the session
// references table, given a session ID
class CSessionRefUpdator
{
public:
	TCHAR m_SessionID[MAX_SESSION_KEY_LEN];
	HRESULT Assign(LPCTSTR szSessionID) throw()
	{
		if (!szSessionID)
			return E_INVALIDARG;
		if (Checked::tcsnlen(szSessionID, MAX_SESSION_KEY_LEN) < MAX_SESSION_KEY_LEN)
			Checked::tcscpy_s(m_SessionID, _countof(m_SessionID), szSessionID);
		else
			return E_OUTOFMEMORY;
		return S_OK;
	}
	BEGIN_PARAM_MAP(CSessionRefUpdator)
		SET_PARAM_TYPE(DBPARAMIO_INPUT)
		COLUMN_ENTRY(1, m_SessionID)
	END_PARAM_MAP()
};

class CSessionRefIsExpired
{
public:
	TCHAR m_SessionID[MAX_SESSION_KEY_LEN];
	TCHAR m_SessionIDOut[MAX_SESSION_KEY_LEN];
	HRESULT Assign(LPCTSTR szSessionID) throw()
	{
		m_SessionIDOut[0]=0;
		if (!szSessionID)
			return E_INVALIDARG;
		if (Checked::tcsnlen(szSessionID, MAX_SESSION_KEY_LEN) < MAX_SESSION_KEY_LEN)
			Checked::tcscpy_s(m_SessionID, _countof(m_SessionID), szSessionID);
		else
			return E_OUTOFMEMORY;
		return S_OK;
	}
	BEGIN_COLUMN_MAP(CSessionRefIsExpired)
		COLUMN_ENTRY(1, m_SessionIDOut)
	END_COLUMN_MAP()
	BEGIN_PARAM_MAP(CSessionRefIsExpired)
		SET_PARAM_TYPE(DBPARAMIO_INPUT)
		COLUMN_ENTRY(1, m_SessionID)
	END_PARAM_MAP()
};

class CSetAllTimeouts
{
public:
	unsigned __int64 m_dwNewTimeout;
	HRESULT Assign(unsigned __int64 dwNewValue)
	{
		m_dwNewTimeout = dwNewValue;
		return S_OK;
	}
	BEGIN_PARAM_MAP(CSetAllTimeouts)
		SET_PARAM_TYPE(DBPARAMIO_INPUT)
		COLUMN_ENTRY(1, m_dwNewTimeout)
	END_PARAM_MAP()
};

class CSessionRefUpdateTimeout
{
public:
	TCHAR m_SessionID[MAX_SESSION_KEY_LEN];
	unsigned __int64 m_nNewTimeout;
	HRESULT Assign(LPCTSTR szSessionID, unsigned __int64 nNewTimeout) throw()
	{
		if (!szSessionID)
			return E_INVALIDARG;

		if (Checked::tcsnlen(szSessionID, MAX_SESSION_KEY_LEN) < MAX_SESSION_KEY_LEN)
			Checked::tcscpy_s(m_SessionID, _countof(m_SessionID), szSessionID);
		else
			return E_OUTOFMEMORY;

		m_nNewTimeout = nNewTimeout;

		return S_OK;
	}

	BEGIN_PARAM_MAP(CSessionRefUpdateTimeout)
		SET_PARAM_TYPE(DBPARAMIO_INPUT)
		COLUMN_ENTRY(1, m_nNewTimeout)
		COLUMN_ENTRY(2, m_SessionID)
	END_PARAM_MAP()
};

class CSessionRefSelector
{
public:
	TCHAR m_SessionID[MAX_SESSION_KEY_LEN];
	int m_RefCount;
	HRESULT Assign(LPCTSTR szSessionID) throw()
	{
		if (!szSessionID)
			return E_INVALIDARG;
		if (Checked::tcsnlen(szSessionID, MAX_SESSION_KEY_LEN) < MAX_SESSION_KEY_LEN)
			Checked::tcscpy_s(m_SessionID, _countof(m_SessionID), szSessionID);
		else
			return E_OUTOFMEMORY;
		return S_OK;
	}
	BEGIN_COLUMN_MAP(CSessionRefSelector)
		COLUMN_ENTRY(1, m_SessionID)
		COLUMN_ENTRY(3, m_RefCount)
	END_COLUMN_MAP()
	BEGIN_PARAM_MAP(CSessionRefSelector)
		SET_PARAM_TYPE(DBPARAMIO_INPUT)
		COLUMN_ENTRY(1, m_SessionID)
	END_PARAM_MAP()
};

class CSessionRefCount
{
public:
	LONG m_nCount;
	BEGIN_COLUMN_MAP(CSessionRefCount)
		COLUMN_ENTRY(1, m_nCount)
	END_COLUMN_MAP()
};

// Used for creating new entries in the session
// references table.
class CSessionRefCreator
{
public:
	TCHAR m_SessionID[MAX_SESSION_KEY_LEN];
	unsigned __int64 m_TimeoutMs;
	HRESULT Assign(LPCTSTR szSessionID, unsigned __int64 timeout) throw()
	{
		if (!szSessionID)
			return E_INVALIDARG;
		if (Checked::tcsnlen(szSessionID, MAX_SESSION_KEY_LEN) < MAX_SESSION_KEY_LEN)
		{
			Checked::tcscpy_s(m_SessionID, _countof(m_SessionID), szSessionID);
			m_TimeoutMs = timeout;
		}
		else
			return E_OUTOFMEMORY;
		return S_OK;
	}
	BEGIN_PARAM_MAP(CSessionRefCreator)
		SET_PARAM_TYPE(DBPARAMIO_INPUT)
		COLUMN_ENTRY(1, m_SessionID)
		COLUMN_ENTRY(2, m_TimeoutMs)
	END_PARAM_MAP()
};


// CDBSession
// This session persistance class persists session variables to
// an OLEDB datasource. The following table gives a general description
// of the table schema for the tables this class uses.
//
// TableName: SessionVariables
// Column		Name			Type							Description
// 1			SessionID		char[MAX_SESSION_KEY_LEN]		Session Key name
// 2			VariableName	char[MAX_VARIABLE_NAME_LENGTH]	Variable Name
// 3			VariableValue	varbinary[MAX_VARIABLE_VALUE_LENGTH]	Variable Value

//
// TableName: SessionReferences
// Column		Name			Type							Description
// 1			SessionID		char[MAX_SESSION_KEY_LEN]		Session Key Name.
// 2			LastAccess		datetime						Date and time of last access to this session.
// 3			RefCount		int								Current references on this session.
// 4			TimeoutMS		int								Timeout value for the session in milli seconds

typedef bool (*PFN_GETPROVIDERINFO)(DWORD_PTR, wchar_t **);

template <class QueryClass=CDefaultQueryClass>
class CDBSession:
	public ISession,
	public CComObjectRootEx<CComGlobalsThreadModel>

{
	typedef CCommand<CAccessor<CAllSessionDataSelector> >  iterator_accessor;
public:
	typedef QueryClass DBQUERYCLASS_TYPE;
	BEGIN_COM_MAP(CDBSession)
		COM_INTERFACE_ENTRY(ISession)
	END_COM_MAP()

	CDBSession() throw():
		m_dwTimeout(ATL_SESSION_TIMEOUT)
	{
		m_szSessionName[0] = '\0';
	}

	~CDBSession() throw()
	{
	}

	void FinalRelease()throw()
	{
		SessionUnlock();
	}

	STDMETHOD(SetVariable)(LPCSTR szName, VARIANT Val) throw()
	{
		HRESULT hr = E_FAIL;
		if (!szName)
			return E_INVALIDARG;

		// Get the data connection for this thread.
		CDataConnection dataconn;
		hr = GetSessionConnection(&dataconn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		// Update the last access time for this session
		hr = Access();
		if (hr != S_OK)
			return hr;

		// Allocate an updator command and fill out it's input parameters.
		CCommand<CAccessor<CSessionDataUpdator> > command;
		_ATLTRY
		{
			CA2CT name(szName);
			hr = command.Assign(m_szSessionName, name, &Val);
		}
		_ATLCATCHALL()
		{
			hr = E_OUTOFMEMORY;
		}
		if (hr != S_OK)
			return hr;

		// Try an update. Update will fail if the variable is not already there.
		DBROWCOUNT nRows = 0;

		hr = command.Open(dataconn, 
						m_QueryObj.GetSessionVarUpdate(),
						NULL, &nRows, DBGUID_DEFAULT, false);
		if (hr == S_OK && nRows <= 0)
			hr = E_UNEXPECTED;
		if (hr != S_OK)
		{
			// Try an insert
			hr = command.Open(dataconn, m_QueryObj.GetSessionVarInsert(), NULL, &nRows, DBGUID_DEFAULT, false);
			if (hr == S_OK && nRows <=0)
				hr = E_UNEXPECTED;
		}

		return hr;
	}

	// Warning: For string data types, depending on the configuration of
	// your database, strings might be returned with trailing white space.
	STDMETHOD(GetVariable)(LPCSTR szName, VARIANT *pVal) throw()
	{
		HRESULT hr = E_FAIL;
		if (!szName)
			return E_INVALIDARG;
		if (pVal)
			VariantInit(pVal);
		else
			return E_POINTER;

		// Get the data connection for this thread
		CDataConnection dataconn;
		hr = GetSessionConnection(&dataconn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		// Update the last access time for this session
		hr = Access();
		if (hr != S_OK)
			return hr;

		// Allocate a command a fill out it's input parameters.
		CCommand<CAccessor<CSessionDataSelector> > command;
		_ATLTRY
		{
			CA2CT name(szName);
			hr = command.Assign(m_szSessionName, name, NULL);
		}
		_ATLCATCHALL()
		{
			hr = E_OUTOFMEMORY;
		}

		if (hr == S_OK)
		{
			hr = command.Open(dataconn, m_QueryObj.GetSessionVarSelectVar());
			if (SUCCEEDED(hr))
			{
				if ( S_OK == (hr = command.MoveFirst()))
				{
					CStreamOnByteArray stream(command.m_VariableValue);
					CComVariant vOut;
					hr = vOut.ReadFromStream(static_cast<IStream*>(&stream));
					if (hr == S_OK)
						hr = vOut.Detach(pVal);
				}
			}
		}
		return hr;
	}

	STDMETHOD(RemoveVariable)(LPCSTR szName) throw()
	{
		HRESULT hr = E_FAIL;
		if (!szName)
			return E_INVALIDARG;

		// Get the data connection for this thread.
		CDataConnection dataconn;
		hr = GetSessionConnection(&dataconn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		// update the last access time for this session
		hr = Access();
		if (hr != S_OK)
			return hr;

		// allocate a command and set it's input parameters
		CCommand<CAccessor<CSessionDataDeletor> > command;
		_ATLTRY
		{
			CA2CT name(szName);
			hr = command.Assign(m_szSessionName, name);
		}
		_ATLCATCHALL()
		{
			return E_OUTOFMEMORY;
		}

		// execute the command
		DBROWCOUNT nRows = 0;
		if (hr == S_OK)
			hr = command.Open(dataconn, m_QueryObj.GetSessionVarDeleteVar(),
			NULL, &nRows, DBGUID_DEFAULT, false);
		if (hr == S_OK && nRows <= 0)
			hr = E_FAIL;
		return hr;
	}

	// Gives the count of rows in the table for this session ID.
	STDMETHOD(GetCount)(long *pnCount) throw()
	{
		HRESULT hr = S_OK;
		if (pnCount)
			*pnCount = 0;
		else
			return E_POINTER;

		// Get the database connection for this thread.
		CDataConnection dataconn;
		hr = GetSessionConnection(&dataconn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;
		hr = Access();
		if (hr != S_OK)
			return hr;
		CCommand<CAccessor<CCountAccessor> > command;

		hr = command.Assign(m_szSessionName);
		if (hr == S_OK)
		{
			hr = command.Open(dataconn, m_QueryObj.GetSessionVarCount());
			if (hr == S_OK)
			{
				if (S_OK == (hr = command.MoveFirst()))
				{
					*pnCount = command.m_nCount;
					hr = S_OK;
				}
			}
		}
		return hr;
	}

	STDMETHOD(RemoveAllVariables)() throw()
	{
		HRESULT hr = E_UNEXPECTED;

		// Get the data connection for this thread.
		CDataConnection dataconn;
		hr = GetSessionConnection(&dataconn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		CCommand<CAccessor<CSessionDataDeleteAll> > command;
		hr = command.Assign(m_szSessionName);
		if (hr != S_OK)
			return hr;

		// delete all session variables
		hr = command.Open(dataconn, m_QueryObj.GetSessionVarDeleteAllVars(), NULL, NULL, DBGUID_DEFAULT, false);
		return hr;
	}

	// Iteration of variables works by taking a snapshot
	// of the sessions at the point in time BeginVariableEnum
	// is called, and then keeping an index variable that you use to
	// move through the snapshot rowset. It is important to know
	// that the handle returned in phEnum is not thread safe. It
	// should only be used by the calling thread.
	STDMETHOD(BeginVariableEnum)(POSITION *pPOS, HSESSIONENUM *phEnum) throw()
	{
		HRESULT hr = E_FAIL;
		if (!pPOS)
			return E_POINTER;

		if (phEnum)
			*phEnum = NULL;
		else
			return E_POINTER;

		// Get the data connection for this thread.
		CDataConnection dataconn;
		hr = GetSessionConnection(&dataconn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		// Update the last access time for this session.
		hr = Access();
		if (hr != S_OK)
			return hr;

		// Allocate a new iterator accessor and initialize it's input parameters.
		iterator_accessor *pIteratorAccessor = NULL;
		ATLTRYALLOC(pIteratorAccessor = new iterator_accessor);
		if (!pIteratorAccessor)
			return E_OUTOFMEMORY;

		hr = pIteratorAccessor->Assign(m_szSessionName, NULL, NULL);
		if (hr == S_OK)
		{
			// execute the command and move to the first row of the recordset.
			hr = pIteratorAccessor->Open(dataconn, 
								m_QueryObj.GetSessionVarSelectAllVars());
			if (hr == S_OK)
			{
				hr = pIteratorAccessor->MoveFirst();
				if (hr == S_OK)
				{
					*pPOS = (POSITION) INVALID_DB_SESSION_POS + 1;
					*phEnum = reinterpret_cast<HSESSIONENUM>(pIteratorAccessor);
				}
			}

			if (hr != S_OK)
			{
				*pPOS = INVALID_DB_SESSION_POS;
				*phEnum = NULL;
				delete pIteratorAccessor;
			}
		}
		return hr;
	}

	// The values for hEnum and pPos must have been initialized in a previous
	// call to BeginVariableEnum. On success, the out variant will hold the next
	// variable
	STDMETHOD(GetNextVariable)(POSITION *pPOS, VARIANT *pVal, HSESSIONENUM hEnum, LPSTR szName=NULL, DWORD dwLen=0) throw()
	{
		if (!pPOS)
			return E_INVALIDARG;

		if (pVal)
			VariantInit(pVal);
		else
			return E_POINTER;

		if (!hEnum)
			return E_UNEXPECTED;

		if (*pPOS <= INVALID_DB_SESSION_POS)
			return E_UNEXPECTED;

		iterator_accessor *pIteratorAccessor = reinterpret_cast<iterator_accessor*>(hEnum);

		// update the last access time.
		HRESULT hr = Access();

		POSITION posCurrent = *pPOS;

		if (szName)
		{
			// caller wants entry name
			_ATLTRY
			{
				CT2CA szVarName(pIteratorAccessor->m_VariableName);
				if (szVarName != NULL && dwLen > Checked::strnlen(szVarName, dwLen))
				{
					Checked::strcpy_s(szName, dwLen, szVarName);
				}
				else
					hr = E_OUTOFMEMORY; // buffer not big enough
			}				
			_ATLCATCHALL()
			{
				hr = E_OUTOFMEMORY;
			}
				
		}

		if (hr == S_OK)
		{
			CStreamOnByteArray stream(pIteratorAccessor->m_VariableValue);
			CComVariant vOut;
			hr = vOut.ReadFromStream(static_cast<IStream*>(&stream));
			if (hr == S_OK)
				vOut.Detach(pVal);
			else
				return hr;
		}
		else
			return hr;

		hr = pIteratorAccessor->MoveNext();
		*pPOS = ++posCurrent;

		if (hr == DB_S_ENDOFROWSET)
		{
			// We're done iterating, reset everything
			*pPOS = INVALID_DB_SESSION_POS;
			hr = S_OK;
		}

		if (hr != S_OK)
		{
			VariantClear(pVal);
		}
		return hr;
	}

	// CloseEnum frees up any resources allocated by the iterator
	STDMETHOD(CloseEnum)(HSESSIONENUM hEnum) throw()
	{
		iterator_accessor *pIteratorAccessor = reinterpret_cast<iterator_accessor*>(hEnum);
		if (!pIteratorAccessor)
			return E_INVALIDARG;
		pIteratorAccessor->Close();
		delete pIteratorAccessor;
		return S_OK;
	}

	//
	// Returns S_FALSE if it's not expired
	// S_OK if it is expired and an error HRESULT
	// if an error occurred.
	STDMETHOD(IsExpired)() throw()
	{
		HRESULT hrRet = S_FALSE;
		HRESULT hr = E_UNEXPECTED;

		// Get the data connection for this thread.
		CDataConnection dataconn;
		hr = GetSessionConnection(&dataconn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		CCommand<CAccessor<CSessionRefIsExpired> > command;
		hr = command.Assign(m_szSessionName);
		if (hr != S_OK)
			return hr;

		hr = command.Open(dataconn, m_QueryObj.GetSessionRefIsExpired(), 
							NULL, NULL, DBGUID_DEFAULT, true);
		if (hr == S_OK)
		{
			if (S_OK == command.MoveFirst())
			{
				if (!_tcscmp(command.m_SessionIDOut, m_szSessionName))
					hrRet = S_OK;
			}
		}

		if (hr == S_OK)
			return hrRet;
		return hr;
	}

	STDMETHOD(SetTimeout)(unsigned __int64 dwNewTimeout) throw()
	{
		HRESULT hr = E_UNEXPECTED;

		// Get the data connection for this thread.
		CDataConnection dataconn;
		hr = GetSessionConnection(&dataconn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		// allocate a command and set it's input parameters
		CCommand<CAccessor<CSessionRefUpdateTimeout> > command;
		hr = command.Assign(m_szSessionName, dwNewTimeout);
		if (hr != S_OK)
			return hr;

		hr = command.Open(dataconn, m_QueryObj.GetSessionRefUpdateTimeout(),
						NULL, NULL, DBGUID_DEFAULT, false);

		return hr;
	}

	// SessionLock increments the session reference count for this session.
	// If there is not a session by this name in the session references table,
	// a new session entry is created in the the table.
	HRESULT SessionLock() throw()
	{
		HRESULT hr = E_UNEXPECTED;
		if (!m_szSessionName || m_szSessionName[0]==0)
			return hr; // no session to lock.

		// retrieve the data connection for this thread
		CDataConnection dataconn;
		hr = GetSessionConnection(&dataconn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		// first try to update a session with this name
		DBROWCOUNT nRows = 0;
		CCommand<CAccessor<CSessionRefUpdator> > updator;
		if (S_OK == updator.Assign(m_szSessionName))
		{
			if (S_OK != (hr = updator.Open(dataconn, m_QueryObj.GetSessionRefAddRef(),
				NULL, &nRows, DBGUID_DEFAULT, false)) ||
				nRows == 0)
			{
				// No session to update. Use the creator accessor
				// to create a new session reference.
				CCommand<CAccessor<CSessionRefCreator> > creator;
				hr = creator.Assign(m_szSessionName, m_dwTimeout);
				if (hr == S_OK)
					hr = creator.Open(dataconn, m_QueryObj.GetSessionRefCreate(),
					NULL, &nRows, DBGUID_DEFAULT, false);
			}
		}

		// We should have been able to create or update a session.
		ATLASSERT(nRows > 0);
		if (hr == S_OK && nRows <= 0)
			hr = E_UNEXPECTED;

		return hr;
	}

	// SessionUnlock decrements the session RefCount for this session.
	// Sessions cannot be removed from the database unless the session
	// refcount is 0
	HRESULT SessionUnlock() throw()
	{
		HRESULT hr = E_UNEXPECTED;
		if (!m_szSessionName ||
			m_szSessionName[0]==0)
			return hr; 

		// get the data connection for this thread
		CDataConnection dataconn;
		hr = GetSessionConnection(&dataconn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		// The session must exist at this point in order to unlock it
		// so we can just use the session updator here.
		DBROWCOUNT nRows = 0;
		CCommand<CAccessor<CSessionRefUpdator> > updator;
		hr = updator.Assign(m_szSessionName);
		if (hr == S_OK)
		{
			hr = updator.Open(	dataconn,
								m_QueryObj.GetSessionRefRemoveRef(),
								NULL,
								&nRows,
								DBGUID_DEFAULT,
								false);
		}
		if (hr != S_OK)
			return hr;

		// delete the session from the database if 
		// nobody else is using it and it's expired.
		hr = FreeSession();
		return hr;
	}

	// Access updates the last access time for the session. The access
	// time for sessions is updated using the SQL GETDATE function on the
	// database server so that all clients will be using the same clock
	// to compare access times against.
	HRESULT Access() throw()
	{
		HRESULT hr = E_UNEXPECTED;

		if (!m_szSessionName || 
			m_szSessionName[0]==0)
			return hr; // no session to access

		// get the data connection for this thread
		CDataConnection dataconn;
		hr = GetSessionConnection(&dataconn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		// The session reference entry in the references table must
		// be created prior to calling this function so we can just
		// use an updator to update the current entry.
		CCommand<CAccessor<CSessionRefUpdator> > updator;

		DBROWCOUNT nRows = 0;
		hr = updator.Assign(m_szSessionName);
		if (hr == S_OK)
		{
			hr = updator.Open(	dataconn,
								m_QueryObj.GetSessionRefAccess(),
								NULL,
								&nRows,
								DBGUID_DEFAULT,
								false);
		}

		ATLASSERT(nRows > 0);
		if (hr == S_OK && nRows <= 0)
			hr = E_UNEXPECTED;
		return hr;
	}

	// If the session is expired and it's reference is 0,
	// it can be deleted. SessionUnlock calls this function to
	// unlock the session and delete it after we release a session
	// lock. Note that our SQL command will only delete the session
	// if it is expired and it's refcount is <= 0
	HRESULT FreeSession() throw()
	{
		HRESULT hr = E_UNEXPECTED;
		if (!m_szSessionName ||
			m_szSessionName[0]==0)
			return hr;

		// Get the data connection for this thread.
		CDataConnection dataconn;
		hr = GetSessionConnection(&dataconn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		CCommand<CAccessor<CSessionRefUpdator> > updator;

		// The SQL for this command only deletes the
		// session reference from the references table if it's access
		// count is 0 and it has expired.
		return updator.Open(dataconn,
							m_QueryObj.GetSessionRefDelete(),
							NULL,
							NULL,
							DBGUID_DEFAULT,
							false);
	}

	// Initialize is called each time a new session is created.
	HRESULT Initialize( LPCSTR szSessionName, 
						IServiceProvider *pServiceProvider,
						DWORD_PTR dwCookie,
						PFN_GETPROVIDERINFO pfnInfo) throw()
	{
		if (!szSessionName)
			return E_INVALIDARG;

		if (!pServiceProvider)
			return E_INVALIDARG;

		if (!pfnInfo)
			return E_INVALIDARG;

		m_pfnInfo = pfnInfo;
		m_dwProvCookie = dwCookie;
		m_spServiceProvider = pServiceProvider;

		_ATLTRY
		{
			CA2CT tcsSessionName(szSessionName);
			if (Checked::tcsnlen(tcsSessionName, MAX_SESSION_KEY_LEN) < MAX_SESSION_KEY_LEN)
				Checked::tcscpy_s(m_szSessionName, _countof(m_szSessionName), tcsSessionName);
			else
				return E_OUTOFMEMORY;
		}
		_ATLCATCHALL()
		{
			return E_OUTOFMEMORY;
		}
		return SessionLock();
	}

	HRESULT GetSessionConnection(CDataConnection *pConn,
								 IServiceProvider *pProv) throw()
	{
		if (!pProv)
			return E_INVALIDARG;

		if (!m_pfnInfo || 
			!m_dwProvCookie)
			return E_UNEXPECTED;

		wchar_t *wszProv = NULL;
		if (m_pfnInfo(m_dwProvCookie, &wszProv) && wszProv!=NULL)
		{
			return GetDataSource(pProv,
						ATL_DBSESSION_ID,
						wszProv,
						pConn);
		}
		return E_FAIL;
	}


protected:
	TCHAR m_szSessionName[MAX_SESSION_KEY_LEN];
	unsigned __int64 m_dwTimeout;
	CComPtr<IServiceProvider> m_spServiceProvider;
	DWORD_PTR m_dwProvCookie;
	PFN_GETPROVIDERINFO m_pfnInfo;
	DBQUERYCLASS_TYPE m_QueryObj;
}; // CDBSession


template <class TDBSession=CDBSession<> >
class CDBSessionServiceImplT
{
	wchar_t m_szConnectionString[MAX_CONNECTION_STRING_LEN];
	CComPtr<IServiceProvider> m_spServiceProvider;
	typename TDBSession::DBQUERYCLASS_TYPE m_QueryObj;
public:
	typedef const wchar_t* SERVICEIMPL_INITPARAM_TYPE;
	CDBSessionServiceImplT() throw()
	{
		m_dwTimeout = ATL_SESSION_TIMEOUT;
		m_szConnectionString[0] = '\0';
	}

	static bool GetProviderInfo(DWORD_PTR dwProvCookie, wchar_t **ppszProvInfo) throw()
	{
		if (dwProvCookie &&
			ppszProvInfo)
		{
			CDBSessionServiceImplT<TDBSession> *pSvc = 
				reinterpret_cast<CDBSessionServiceImplT<TDBSession>*>(dwProvCookie);
			*ppszProvInfo = pSvc->m_szConnectionString;
			return true;
		}
		return false;
	}

	HRESULT GetSessionConnection(CDataConnection *pConn,
								 IServiceProvider *pProv) throw()
	{
		if (!pProv)
			return E_INVALIDARG;

		if(!m_szConnectionString[0])
			return E_UNEXPECTED;

		return GetDataSource(pProv,
					ATL_DBSESSION_ID,
					m_szConnectionString,
					pConn);
	}

	HRESULT Initialize(SERVICEIMPL_INITPARAM_TYPE pData,
						IServiceProvider *pProvider,
						unsigned __int64 dwInitialTimeout) throw()
	{
		if (!pData || !pProvider)
			return E_INVALIDARG;

		if (Checked::wcsnlen(pData, MAX_CONNECTION_STRING_LEN) < MAX_CONNECTION_STRING_LEN)
		{
			Checked::wcscpy_s(m_szConnectionString, _countof(m_szConnectionString), pData);
		}
		else
			return E_OUTOFMEMORY;

		m_dwTimeout = dwInitialTimeout;
		m_spServiceProvider = pProvider;
		return S_OK;
	}

	HRESULT CreateNewSession(__out_ecount_part_z(*pdwSize, *pdwSize) LPSTR szNewID, __inout DWORD *pdwSize, __deref_out ISession** ppSession) throw()
	{
		HRESULT hr = E_FAIL;
		CComObject<TDBSession> *pNewSession = NULL;

		if (!pdwSize)
			return E_POINTER;

		if (ppSession)
			*ppSession = NULL;
		else
			return E_POINTER;

		if (szNewID)
			*szNewID = NULL;
		else
			return E_INVALIDARG;


		// Create new session
		CComObject<TDBSession>::CreateInstance(&pNewSession);
		if (pNewSession == NULL)
			return E_OUTOFMEMORY;

		// Create a session name and initialize the object
		hr = m_SessionNameGenerator.GetNewSessionName(szNewID, pdwSize);
		if (hr == S_OK)
		{
			hr = pNewSession->Initialize(szNewID, 
										m_spServiceProvider,
										reinterpret_cast<DWORD_PTR>(this),
										GetProviderInfo);
			if (hr == S_OK)
			{
				// we don't hold a reference to the object
				hr = pNewSession->QueryInterface(ppSession);
			}
		}

		if (hr != S_OK)
			delete pNewSession;
		return hr;
	}

	HRESULT CreateNewSessionByName(__in_z LPSTR szNewID, __deref_out ISession** ppSession) throw()
	{
		HRESULT hr = E_FAIL;
		CComObject<TDBSession> *pNewSession = NULL;

		if (!szNewID || *szNewID == 0)
			return E_INVALIDARG;

		if (ppSession)
			*ppSession = NULL;
		else
			return E_POINTER;

		// Create new session
		CComObject<TDBSession>::CreateInstance(&pNewSession);
		if (pNewSession == NULL)
			return E_OUTOFMEMORY;

		hr = pNewSession->Initialize(szNewID, 
									m_spServiceProvider,
									reinterpret_cast<DWORD_PTR>(this),
									GetProviderInfo);
		if (hr == S_OK)
		{
			// we don't hold a reference to the object
			hr = pNewSession->QueryInterface(ppSession);
		}


		if (hr != S_OK)
			delete pNewSession;
		return hr;
	}


	HRESULT GetSession(LPCSTR szID, ISession **ppSession) throw()
	{
		HRESULT hr = E_FAIL;
		if (!szID)
			return E_INVALIDARG;

		if (ppSession)
			*ppSession = NULL;
		else
			return E_POINTER;

		CComObject<TDBSession> *pNewSession = NULL;

		// Check the DB to see if the session ID is a valid session
		_ATLTRY
		{
			CA2CT session(szID);
			hr = IsValidSession(session);
		}
		_ATLCATCHALL()
		{
			hr = E_OUTOFMEMORY;
		}
		if (hr == S_OK)
		{
			// Create new session object to represent this session
			CComObject<TDBSession>::CreateInstance(&pNewSession);
			if (pNewSession == NULL)
				return E_OUTOFMEMORY;

			hr = pNewSession->Initialize(szID,
										m_spServiceProvider,
										reinterpret_cast<DWORD_PTR>(this),
										GetProviderInfo);
			if (hr == S_OK)
			{
				// we don't hold a reference to the object
				hr = pNewSession->QueryInterface(ppSession);
			}
		}

		if (hr != S_OK && pNewSession)
			delete pNewSession;
		return hr;
	}

	HRESULT CloseSession(LPCSTR szID) throw()
	{
		if (!szID)
			return E_INVALIDARG;

		CDataConnection conn;
		HRESULT hr = GetSessionConnection(&conn,
										  m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		// set up accessors
		CCommand<CAccessor<CSessionRefUpdator> > updator;
		CCommand<CAccessor<CSessionDataDeleteAll> > command;
		_ATLTRY
		{
			CA2CT session(szID);
			hr = updator.Assign(session);
			if (hr == S_OK)
				hr = command.Assign(session);
		}
		_ATLCATCHALL()
		{
			hr = E_OUTOFMEMORY;
		}

		if (hr == S_OK)
		{
			// delete all session variables (may not be any!)
			hr = command.Open(conn,
								m_QueryObj.GetSessionVarDeleteAllVars(),
								NULL,
								NULL,
								DBGUID_DEFAULT,
								false);
			if (hr == S_OK)
			{
				DBROWCOUNT nRows = 0;
				nRows = 0;
				// delete references in the session references table
				hr = updator.Open(conn,
							m_QueryObj.GetSessionRefDeleteFinal(),
							NULL,
							&nRows,
							DBGUID_DEFAULT,
							false);
				if (nRows == 0)
					hr = E_UNEXPECTED;
			}
		}
		return hr;
	}

	HRESULT SetSessionTimeout(unsigned __int64 nTimeout) throw()
	{
		// Get the data connection for this thread
		CDataConnection conn;

		HRESULT hr = GetSessionConnection(&conn, m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		// all sessions get the same timeout
		CCommand<CAccessor<CSetAllTimeouts> > command;
		hr = command.Assign(nTimeout);
		if (hr == S_OK)
		{
			hr = command.Open(conn, m_QueryObj.GetSessionReferencesSet(),
						NULL,
						NULL,
						DBGUID_DEFAULT,
						false);
			if (hr == S_OK)
			{
				m_dwTimeout = nTimeout;
			}
		}
		return hr;
	}


	HRESULT GetSessionTimeout(unsigned __int64* pnTimeout) throw()
	{
		if (pnTimeout)
			*pnTimeout = m_dwTimeout;
		else
			return E_INVALIDARG;

		return S_OK;
	}

	HRESULT GetSessionCount(DWORD *pnCount) throw()
	{
		if (pnCount)
			*pnCount = 0;
		else
			return E_POINTER;

		CCommand<CAccessor<CSessionRefCount> > command;
		CDataConnection conn;
		HRESULT hr = GetSessionConnection(&conn,
											m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		hr = command.Open(conn,
							m_QueryObj.GetSessionRefGetCount());
		if (hr == S_OK)
		{
			hr = command.MoveFirst();
			if (hr == S_OK)
			{
				*pnCount = (DWORD)command.m_nCount;
			}
		}

		return hr;
	}

	void ReleaseAllSessions() throw()
	{
		// nothing to do
	}

	void SweepSessions() throw()
	{
		// nothing to do
	}


	// Helpers
	HRESULT IsValidSession(LPCTSTR szID) throw()
	{
		if (!szID)
			return E_INVALIDARG;
		// Look in the sessionreferences table to see if there is an entry
		// for this session.
		if (m_szConnectionString[0] == 0)
			return E_UNEXPECTED;

		CDataConnection conn;
		HRESULT hr = GetSessionConnection(&conn,
											m_spServiceProvider);
		if (hr != S_OK)
			return hr;

		// Check the session references table to see if
		// this is a valid session
		CCommand<CAccessor<CSessionRefSelector> > selector;
		hr = selector.Assign(szID);
		if (hr != S_OK)
			return hr;

		hr = selector.Open(conn,
							m_QueryObj.GetSessionRefSelect(),
							NULL,
							NULL,
							DBGUID_DEFAULT,
							true);
		if (hr == S_OK)
			return selector.MoveFirst();
		return hr;
	}

	CSessionNameGenerator m_SessionNameGenerator; // Object for generating session names
	unsigned __int64 m_dwTimeout;
}; // CDBSessionServiceImplT

typedef CDBSessionServiceImplT<> CDBSessionServiceImpl;





//////////////////////////////////////////////////////////////////
//
// In-memory persisted session
//
//////////////////////////////////////////////////////////////////

// In-memory persisted session service keeps a pointer
// to the session obejct around in memory. The pointer is
// contained in a CComPtr, which is stored in a CAtlMap, so
// we have to have a CElementTraits class for that.
typedef CComPtr<ISession> SESSIONPTRTYPE;

template<>
class CElementTraits<SESSIONPTRTYPE> :
	public CElementTraitsBase<SESSIONPTRTYPE>
{
public:
	static ULONG Hash( INARGTYPE obj ) throw()
	{
		return( (ULONG)(ULONG_PTR)obj.p);
	}

	static BOOL CompareElements( OUTARGTYPE element1, OUTARGTYPE element2 ) throw()
	{
		return element1.IsEqualObject(element2.p) ? TRUE : FALSE;
	}

	static int CompareElementsOrdered( INARGTYPE , INARGTYPE ) throw()
	{
		ATLASSERT(0); // NOT IMPLEMENTED
		return 0;
	}
};


// CMemSession
// This session persistance class persists session variables in memory.
// Note that this type of persistance should only be used on single server
// web sites.
class CMemSession :
	public ISession,
	public CComObjectRootEx<CComGlobalsThreadModel>
{
public:
	BEGIN_COM_MAP(CMemSession)
		COM_INTERFACE_ENTRY(ISession)
	END_COM_MAP()

	CMemSession() throw(...)
	{
	}
	virtual ~CMemSession()
	{
	}	

	STDMETHOD(GetVariable)(LPCSTR szName, VARIANT *pVal) throw()
	{
		if (!szName)
			return E_INVALIDARG;

		if (pVal)
			VariantInit(pVal);
		else
			return E_POINTER;

		HRESULT hr = Access();
		if (hr == S_OK)
		{
			CSLockType lock(m_cs, false);
			hr = lock.Lock();
			if (FAILED(hr))
				return hr;

			hr = E_FAIL;
			_ATLTRY
			{
				CComVariant val;
				if (m_Variables.Lookup(szName, val))
				{
					hr = VariantCopy(pVal, &val);
				}
			}
			_ATLCATCHALL()
			{
				hr = E_UNEXPECTED;
			}
		}
		return hr;
	}

	STDMETHOD(SetVariable)(LPCSTR szName, VARIANT vNewVal) throw()
	{
		if (!szName)
			return E_INVALIDARG;

		HRESULT hr = Access();
		if (hr == S_OK)
		{
			CSLockType lock(m_cs, false);
			hr = lock.Lock();
			if (FAILED(hr))
				return hr;
			_ATLTRY
			{
				hr = m_Variables.SetAt(szName, vNewVal) ? S_OK : E_FAIL;
			}
			_ATLCATCHALL()
			{
				hr = E_UNEXPECTED;
			}
		}
		return hr;
	}

	STDMETHOD(RemoveVariable)(LPCSTR szName) throw()
	{
		if (!szName)
			return E_INVALIDARG;

		HRESULT hr = Access();
		if (hr == S_OK)
		{
			CSLockType lock(m_cs, false);
			hr = lock.Lock();
			if (FAILED(hr))
				return hr;
			_ATLTRY
			{
				hr = m_Variables.RemoveKey(szName) ? S_OK : E_FAIL;
			}
			_ATLCATCHALL()
			{
				hr = E_UNEXPECTED;
			}
		}
		return hr;
	}

	STDMETHOD(GetCount)(long *pnCount) throw()
	{
		if (pnCount)
			*pnCount = 0;
		else
			return E_POINTER;

		HRESULT hr = Access();
		if (hr == S_OK)
		{
			CSLockType lock(m_cs, false);
			hr = lock.Lock();
			if (FAILED(hr))
				return hr;
			*pnCount = (long) m_Variables.GetCount();
		}
		return hr;
	}

	STDMETHOD(RemoveAllVariables)() throw()
	{
		HRESULT hr = Access();
		if (hr == S_OK)
		{
			CSLockType lock(m_cs, false);
			hr = lock.Lock();
			if (FAILED(hr))
				return hr;
			m_Variables.RemoveAll();
		}

		return hr;
	}

	STDMETHOD(BeginVariableEnum)(POSITION *pPOS, HSESSIONENUM *phEnumHandle=NULL) throw()
	{
		if (phEnumHandle)
			*phEnumHandle = NULL;

		if (pPOS)
			*pPOS = NULL;
		else
			return E_POINTER;

		HRESULT hr = Access();
		if (hr == S_OK)
		{
			CSLockType lock(m_cs, false);
			hr = lock.Lock();
			if (FAILED(hr))
				return hr;
			*pPOS = m_Variables.GetStartPosition();
		}
		return hr;
	}

	STDMETHOD(GetNextVariable)(POSITION *pPOS, VARIANT *pVal,
							   HSESSIONENUM hEnum=NULL,
							   LPSTR szName=NULL,
							   DWORD dwLen=0 ) throw()
	{
		(hEnum); // Unused!
		if (pVal)
			VariantInit(pVal);
		else
			return E_POINTER;

		if (!pPOS)
			return E_POINTER;

		CComVariant val;
		POSITION pos = *pPOS;
		HRESULT hr = Access();
		if (hr == S_OK)
		{
			CSLockType lock(m_cs, false);
			hr = lock.Lock();
			if (FAILED(hr))
				return hr;

			hr = E_FAIL;
			_ATLTRY
			{
				if (szName)
				{
					CStringA strName = m_Variables.GetKeyAt(pos);
					if (strName.GetLength())
					{
						if (dwLen > (DWORD)strName.GetLength())
						{
							Checked::strcpy_s(szName, dwLen, strName);
							hr = S_OK;
						}
						else
							hr = E_OUTOFMEMORY;
					}
				}
				else
					hr = S_OK;

				if (hr == S_OK)
				{
					val = m_Variables.GetNextValue(pos);
					hr = VariantCopy(pVal, &val);
					if (hr == S_OK)
						*pPOS = pos;
				}
			}
			_ATLCATCHALL()
			{
				hr = E_UNEXPECTED;
			}
		}
		return hr;
	}

	STDMETHOD(CloseEnum)(HSESSIONENUM /*hEnumHandle*/) throw()
	{
		return S_OK;
	}

	STDMETHOD(IsExpired)() throw()
	{
		CTime tmNow = CTime::GetCurrentTime();
		CTimeSpan span = tmNow-m_tLastAccess;
		if ((unsigned __int64)((span.GetTotalSeconds()*1000)) > m_dwTimeout)
			return S_OK;
		return S_FALSE;
	}

	HRESULT Access() throw()
	{
		// We lock here to protect against multiple threads
		// updating the same member concurrently.
		CSLockType lock(m_cs, false);
		HRESULT hr = lock.Lock();
		if (FAILED(hr))
			return hr;
		m_tLastAccess = CTime::GetCurrentTime();
		return S_OK;
	}

	STDMETHOD(SetTimeout)(unsigned __int64 dwNewTimeout) throw()
	{
		// We lock here to protect against multiple threads
		// updating the same member concurrently
		CSLockType lock(m_cs, false);
		HRESULT hr = lock.Lock();
		if (FAILED(hr))
			return hr;
		m_dwTimeout = dwNewTimeout;
		return S_OK;
	}

	HRESULT SessionLock() throw()
	{
		Access();
		return S_OK;
	}

	HRESULT SessionUnlock() throw()
	{
		return S_OK;
	}

protected:
	typedef CAtlMap<CStringA,
					CComVariant,
					CStringElementTraits<CStringA> > VarMapType;
	unsigned __int64 m_dwTimeout;
	CTime m_tLastAccess;
	VarMapType m_Variables;
	CComAutoCriticalSection m_cs;
	typedef CComCritSecLock<CComAutoCriticalSection> CSLockType;
}; // CMemSession


//
// CMemSessionServiceImpl
// Implements the service part of in-memory persisted session services.
//
class CMemSessionServiceImpl
{
public:
	typedef void* SERVICEIMPL_INITPARAM_TYPE;
	CMemSessionServiceImpl() throw()
	{
		m_dwTimeout = ATL_SESSION_TIMEOUT;
	}

	~CMemSessionServiceImpl() throw()
	{
		m_CritSec.Term();
	}

	HRESULT CreateNewSession(__out_ecount_part_z(*pdwSize, *pdwSize) LPSTR szNewID, __inout DWORD *pdwSize, __deref_out_opt ISession** ppSession) throw()
	{
		HRESULT hr = E_FAIL;
		CComObject<CMemSession> *pNewSession = NULL;

		if (!szNewID)
			return E_INVALIDARG;

		if (!pdwSize)
			return E_POINTER;

		if (ppSession)
			*ppSession = NULL;
		else
			return E_POINTER;

		_ATLTRY
		{
			// Create new session
			CComObject<CMemSession>::CreateInstance(&pNewSession);
			if (pNewSession == NULL)
				return E_OUTOFMEMORY;

			// Initialize and add to list of CSessionData
			hr = m_SessionNameGenerator.GetNewSessionName(szNewID, pdwSize);

			if (SUCCEEDED(hr))
			{
				CComPtr<ISession> spSession;
				hr = pNewSession->QueryInterface(&spSession);
				if (SUCCEEDED(hr))
				{
					pNewSession->SetTimeout(m_dwTimeout);
					pNewSession->Access();
					CSLockType lock(m_CritSec, false);
					hr = lock.Lock();
					if (FAILED(hr))
						return hr;
					hr = m_Sessions.SetAt(szNewID, spSession) != NULL ? S_OK : E_FAIL;
					if (hr == S_OK)
						*ppSession = spSession.Detach();
				}
			}
		}
		_ATLCATCHALL()
		{
			hr = E_UNEXPECTED;
		}

		return hr;

	}

	HRESULT CreateNewSessionByName(__in_z LPSTR szNewID, __deref_out_opt ISession** ppSession) throw()
	{
		HRESULT hr = E_FAIL;
		CComObject<CMemSession> *pNewSession = NULL;

		if (!szNewID || *szNewID == 0)
			return E_INVALIDARG;

		if (ppSession)
			*ppSession = NULL;
		else
			return E_POINTER;

		CComPtr<ISession> spSession;
		// If the session already exists, you get a pointer to the
		// existing session. You can't have multiple entries with the
		// same name in CAtlMap
		hr = GetSession(szNewID, &spSession);
		if (hr == S_OK)
		{
			*ppSession = spSession.Detach();
			return hr;
		}

		_ATLTRY
		{
			// Create new session
			CComObject<CMemSession>::CreateInstance(&pNewSession);
			if (pNewSession == NULL)
				return E_OUTOFMEMORY;


			hr = pNewSession->QueryInterface(&spSession);
			if (SUCCEEDED(hr))
			{
				pNewSession->SetTimeout(m_dwTimeout);
				pNewSession->Access();
				CSLockType lock(m_CritSec, false);
				hr = lock.Lock();
				if (FAILED(hr))
					return hr;

				hr = m_Sessions.SetAt(szNewID, spSession) != NULL ? S_OK : E_FAIL;

				if (hr == S_OK)
					*ppSession = spSession.Detach();
			}
		}
		_ATLCATCHALL()
		{
			hr = E_UNEXPECTED;
		}

		return hr;

	}

	HRESULT GetSession(LPCSTR szID, ISession **ppSession) throw()
	{
		HRESULT hr = E_FAIL;
		SessMapType::CPair *pPair = NULL;

		if (ppSession)
			*ppSession = NULL;
		else
			return E_POINTER;

		if (!szID)
			return E_INVALIDARG;

		CSLockType lock(m_CritSec, false);
		hr = lock.Lock();
		if (FAILED(hr))
			return hr;

		hr = E_FAIL;
		_ATLTRY
		{
			pPair = m_Sessions.Lookup(szID); 
			if (pPair) // the session exists and is in our local map of sessions
			{
				hr = pPair->m_value.QueryInterface(ppSession);
			}
		}
		_ATLCATCHALL()
		{
			return E_UNEXPECTED;
		}

		return hr;	
	}

	HRESULT CloseSession(LPCSTR szID) throw()
	{
		if (!szID)
			return E_INVALIDARG;

		HRESULT hr = E_FAIL;
		CSLockType lock(m_CritSec, false);
		hr = lock.Lock();
		if (FAILED(hr))
			return hr;
		_ATLTRY
		{
			hr = m_Sessions.RemoveKey(szID) ? S_OK : E_UNEXPECTED;
		}
		_ATLCATCHALL()
		{
			hr = E_UNEXPECTED;
		}
		return hr;
	}

	void SweepSessions() throw()
	{
		POSITION posRemove = NULL;
		const SessMapType::CPair *pPair = NULL;
		POSITION pos = NULL;

		CSLockType lock(m_CritSec, false);
		if (FAILED(lock.Lock()))
			return;
		pos = m_Sessions.GetStartPosition();
		while (pos)
		{
			posRemove = pos;
			pPair = m_Sessions.GetNext(pos);
			if (pPair)
			{
				if (pPair->m_value.p &&
					S_OK == pPair->m_value->IsExpired())
				{
					// remove our reference on the session
					m_Sessions.RemoveAtPos(posRemove);
				}
			}
		}
	}

	HRESULT SetSessionTimeout(unsigned __int64 nTimeout) throw()
	{
		HRESULT hr = S_OK;
		CComPtr<ISession> spSession;
		m_dwTimeout = nTimeout;

		CSLockType lock(m_CritSec, false);
		hr = lock.Lock();
		if (FAILED(hr))
			return hr;

		POSITION pos = m_Sessions.GetStartPosition();
		if (!pos)
			return S_OK; // no sessions to set the timeout on


		while (pos)
		{
			SessMapType::CPair *pPair = const_cast<SessMapType::CPair*>(m_Sessions.GetNext(pos));
			if (pPair)
			{
				spSession = pPair->m_value;
				if (spSession)
				{
					// if we fail on any of the sets we will return the
					// error code immediately
					hr = spSession->SetTimeout(nTimeout);
					spSession.Release();
					if (hr != S_OK)
						break;
				}
				else
				{
					hr = E_UNEXPECTED;
					break;
				}
			}
			else
			{
				hr = E_UNEXPECTED;
				break;
			}
		}

		return hr;
	}

	HRESULT GetSessionTimeout(unsigned __int64* pnTimeout) throw()
	{
		if (pnTimeout)
			*pnTimeout = m_dwTimeout;
		else
			return E_POINTER;

		return S_OK;
	}

	HRESULT GetSessionCount(DWORD *pnCount) throw()
	{
		if (pnCount)
			*pnCount = 0;
		else
			return E_POINTER;

		CSLockType lock(m_CritSec, false);
		HRESULT hr = lock.Lock();
		if (FAILED(hr))
			return hr;
		*pnCount = (DWORD)m_Sessions.GetCount();

		return S_OK;
	}

	void ReleaseAllSessions() throw()
	{
		CSLockType lock(m_CritSec, false);
		if (FAILED(lock.Lock()))
			return;
		m_Sessions.RemoveAll();
	}

	HRESULT Initialize(SERVICEIMPL_INITPARAM_TYPE,
					   IServiceProvider*,
					   unsigned __int64 dwNewTimeout) throw()
	{
		m_dwTimeout = dwNewTimeout;
		return m_CritSec.Init();
	}

	typedef CAtlMap<CStringA,
					SESSIONPTRTYPE,
					CStringElementTraits<CStringA>,
					CElementTraitsBase<SESSIONPTRTYPE> > SessMapType;

	SessMapType m_Sessions; // map for holding sessions in memory
	CComCriticalSection m_CritSec; // for synchronizing access to map
	typedef CComCritSecLock<CComCriticalSection> CSLockType;
	CSessionNameGenerator m_SessionNameGenerator; // Object for generating session names
	unsigned __int64 m_dwTimeout;
}; // CMemSessionServiceImpl



//
// CSessionStateService
// This class implements the session state service which can be
// exposed to request handlers.
//
// Template Parameters:
// MonitorClass: Provides periodic sweeping services for the session service class.
// TServiceImplClass: The class that actually implements the methods of the
//                    ISessionStateService and ISessionStateControl interfaces.
template <class MonitorClass, class TServiceImplClass >
class CSessionStateService : 
	public ISessionStateService,
	public ISessionStateControl,
	public IWorkerThreadClient,
	public CComObjectRootEx<CComGlobalsThreadModel>
{
protected:
	MonitorClass m_Monitor;
	HANDLE m_hTimer;
	CComPtr<IServiceProvider> m_spServiceProvider;
	TServiceImplClass m_SessionServiceImpl;
public:
	// Construction/Initialization
	CSessionStateService() throw() :
	  m_hTimer(NULL)
	  {

	  }
	~CSessionStateService() throw()
	{
		ATLASSUME(m_hTimer == NULL);
	}
	BEGIN_COM_MAP(CSessionStateService)
		COM_INTERFACE_ENTRY(ISessionStateService)
		COM_INTERFACE_ENTRY(ISessionStateControl)
	END_COM_MAP()

// ISessionStateServie methods
	STDMETHOD(CreateNewSession)(LPSTR szNewID, DWORD *pdwSize, ISession** ppSession) throw()
	{
		return m_SessionServiceImpl.CreateNewSession(szNewID, pdwSize, ppSession);
	}

	STDMETHOD(CreateNewSessionByName)(LPSTR szNewID, ISession** ppSession) throw()
	{
		return m_SessionServiceImpl.CreateNewSessionByName(szNewID, ppSession);
	}

	STDMETHOD(GetSession)(LPCSTR szID, ISession **ppSession) throw()
	{
		return m_SessionServiceImpl.GetSession(szID, ppSession);
	}

	STDMETHOD(CloseSession)(LPCSTR szSessionID) throw()
	{
		return m_SessionServiceImpl.CloseSession(szSessionID);
	}

	STDMETHOD(SetSessionTimeout)(unsigned __int64 nTimeout) throw()
	{
		return m_SessionServiceImpl.SetSessionTimeout(nTimeout);
	}

	STDMETHOD(GetSessionTimeout)(unsigned __int64 *pnTimeout) throw()
	{
		return m_SessionServiceImpl.GetSessionTimeout(pnTimeout);
	}

	STDMETHOD(GetSessionCount)(DWORD *pnSessionCount) throw()
	{
		return m_SessionServiceImpl.GetSessionCount(pnSessionCount);
	}

	void SweepSessions() throw()
	{
		m_SessionServiceImpl.SweepSessions();
	}

	void ReleaseAllSessions() throw()
	{
		m_SessionServiceImpl.ReleaseAllSessions();
	}

	HRESULT Initialize(
		IServiceProvider *pServiceProvider = NULL,
		typename TServiceImplClass::SERVICEIMPL_INITPARAM_TYPE pInitData = NULL,
		unsigned __int64 dwTimeout = ATL_SESSION_TIMEOUT) throw()
	{
		HRESULT hr = S_OK;
		if (pServiceProvider)
			m_spServiceProvider = pServiceProvider;

		hr = m_SessionServiceImpl.Initialize(pInitData, pServiceProvider, dwTimeout);

		return hr;
	}


	template <class ThreadTraits>
	HRESULT Initialize(
		CWorkerThread<ThreadTraits> *pWorker,
		IServiceProvider *pServiceProvider = NULL,
		typename TServiceImplClass::SERVICEIMPL_INITPARAM_TYPE pInitData = NULL,
		unsigned __int64 dwTimeout = ATL_SESSION_TIMEOUT) throw()
	{
		if (!pWorker)
			return E_INVALIDARG;

		HRESULT hr = Initialize(pServiceProvider, pInitData, dwTimeout);
		if (hr == S_OK)
		{
			hr = m_Monitor.Initialize(pWorker);
			if (hr == S_OK)
			{
				//sweep every 500ms
				hr = m_Monitor.AddTimer(ATL_SESSION_SWEEPER_TIMEOUT, this, 0, &m_hTimer);
			}
		}
		return hr;
	}

	void Shutdown() throw()
	{
		if (m_hTimer)
		{
			if(FAILED(m_Monitor.RemoveHandle(m_hTimer)))
			{
				/* can't report from here */
				ATLASSERT(FALSE);
			}
			m_hTimer = NULL;
		}
		ReleaseAllSessions();
	}
// Implementation
	HRESULT Execute(DWORD_PTR /*dwParam*/, HANDLE /*hObject*/) throw()
	{
		SweepSessions();
		return S_OK;
	}

	HRESULT CloseHandle(HANDLE hHandle) throw()
	{
		::CloseHandle(hHandle);
		m_hTimer = NULL;
		return S_OK;
	}

}; // CSessionStateService

} // namespace ATL
#pragma pack(pop)

#pragma warning(pop)
#endif // __ATLSESSION_H__
