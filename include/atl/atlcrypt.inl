// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.


#ifndef __ATLCRYPT_INL__
#define __ATLCRYPT_INL__

#pragma once

#ifndef __ATLCRYPT_H__
	#error atlcrypt.inl requires atlcrypt.h to be included first
#endif


namespace ATL
{

inline CCryptProv::CCryptProv( const CCryptProv& prov ) throw()
{
	m_hProv = prov.m_hProv;
	if (m_hProv)
		AddRef();
}

inline CCryptProv::CCryptProv( HCRYPTPROV hProv, BOOL bTakeOwnership ) throw()
{
	m_hProv = hProv;
	if (m_hProv && !bTakeOwnership)
		AddRef();
}

inline CCryptProv::~CCryptProv() throw()
{
	Release();
}

inline CCryptProv& CCryptProv::operator=( const CCryptProv& prov ) throw()
{
    if(this!=&prov)
    {
	    Release();

	    m_hProv = prov.m_hProv;
	    if( m_hProv != NULL )
	    {
		    AddRef();
	    }
    }
	return( *this );
}

inline HRESULT CCryptProv::AddRef() throw()
{
	ATLASSUME( m_hProv != NULL );

	if (!CryptContextAddRef( m_hProv, NULL, 0))
	{
		return AtlHresultFromLastError();
	}
	return S_OK;
}

inline void CCryptProv::Attach( HCRYPTPROV hProv, BOOL bTakeOwnership ) throw()
{
	ATLASSUME( m_hProv == NULL );

	m_hProv = hProv;
	if (m_hProv && !bTakeOwnership)
		AddRef();
}

inline HCRYPTPROV CCryptProv::Detach() throw()
{
	HCRYPTPROV hProv;

	hProv = m_hProv;
	m_hProv = NULL;

	return( hProv );
}


inline CCryptProv::CCryptProv() throw() :
	m_hProv( NULL )
{
}

inline HRESULT CCryptProv::Release() throw()
{
	if( m_hProv != NULL )
	{
		if (!CryptReleaseContext( m_hProv, 0 ))
		{
			return AtlHresultFromLastError();
		}
		m_hProv = NULL;
	}
	return S_OK;
}

inline HRESULT CCryptProv::Initialize(
	DWORD dwProviderType,
	LPCTSTR szContainer,
	LPCTSTR szProvider,
	DWORD dwFlags) throw()
{
	ATLASSUME(m_hProv == NULL);

	if (!CryptAcquireContext(&m_hProv, szContainer, szProvider, dwProviderType, dwFlags))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptProv::InitVerifyContext(
	DWORD dwProviderType,
	LPCTSTR szProvider,
	DWORD dwFlags) throw()
{
	ATLASSUME(m_hProv == NULL);

	if (!CryptAcquireContext(&m_hProv, NULL, szProvider, dwProviderType, CRYPT_VERIFYCONTEXT | dwFlags))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptProv::InitCreateKeySet(
	DWORD dwProviderType,
	LPCTSTR szContainer,
	LPCTSTR szProvider, 
	DWORD dwFlags) throw()
{
	ATLASSUME(m_hProv == NULL);

	if (!CryptAcquireContext(&m_hProv, szContainer, szProvider, dwProviderType, CRYPT_NEWKEYSET | dwFlags))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptProv::DeleteKeySet(
	DWORD dwProviderType,
	LPCTSTR szContainer,
	LPCTSTR szProvider,
	DWORD dwFlags) throw()
{
	HCRYPTPROV hProv = NULL;
	if (!CryptAcquireContext(&hProv, szContainer, szProvider, dwProviderType, CRYPT_DELETEKEYSET | dwFlags))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}


inline HRESULT CCryptProv::Uninitialize() throw()
{
	ATLASSUME(m_hProv != NULL);

	if (!CryptReleaseContext(m_hProv, 0))
	{
		return AtlHresultFromLastError();
	}
	else 
	{
		m_hProv = NULL;
		return S_OK;
	}
}

inline HRESULT CCryptProv::GetParam(DWORD dwParam, BYTE * pbData, DWORD * pdwDataLen, DWORD dwFlags) throw()
{
	ATLASSUME(m_hProv != NULL);

	if (!CryptGetProvParam(m_hProv, dwParam, pbData, pdwDataLen, dwFlags))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptProv::SetParam( DWORD dwParam, BYTE* pbData, DWORD dwFlags) throw()
{
	ATLASSUME(m_hProv != NULL);

	if (!CryptSetProvParam(m_hProv, dwParam, pbData, dwFlags ))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptProv::GetName(__out_bcount_part(*pdwLength, *pdwLength) LPSTR szBuf, __inout DWORD * pdwLength) throw()
{
	return GetParam(PP_NAME, (BYTE *)szBuf, pdwLength);
}

inline HRESULT CCryptProv::GetContainer(__out_bcount_part(*pdwLength, *pdwLength) LPSTR szBuf, __inout DWORD * pdwLength) throw()
{
	return GetParam(PP_CONTAINER, (BYTE *)szBuf, pdwLength);
}

inline HRESULT CCryptProv::GetImpType(DWORD * pdwImpType) throw()
{
	DWORD dwLength = sizeof(DWORD);
	return GetParam(PP_IMPTYPE, (BYTE *)pdwImpType, &dwLength);
}

inline HRESULT CCryptProv::GetVersion(DWORD * pdwVersion) throw()
{
	DWORD dwLength = sizeof(DWORD);
	return GetParam(PP_VERSION, (BYTE *)pdwVersion, &dwLength);
}

inline HRESULT CCryptProv::GetProvType(DWORD * pdwType) throw()
{
	DWORD dwLength = sizeof(DWORD);
	return GetParam(PP_PROVTYPE, (BYTE * )pdwType, &dwLength);
}

inline HRESULT CCryptProv::GetSecurityDesc(SECURITY_INFORMATION * pSecInfo) throw()
{
	DWORD dwSize = sizeof(SECURITY_INFORMATION);
	return GetParam(PP_KEYSET_SEC_DESCR, (BYTE *)pSecInfo, &dwSize);
}

inline HRESULT CCryptProv::SetSecurityDesc(SECURITY_INFORMATION SecInfo) throw()
{
	return SetParam(PP_KEYSET_SEC_DESCR, (BYTE *)&SecInfo);
}

inline HRESULT CCryptProv::GenRandom(ULONG nLength, BYTE* pbBuffer ) throw()
{
	ATLASSUME(m_hProv != NULL);

	if (!CryptGenRandom( m_hProv, nLength, pbBuffer ))
	{
		return AtlHresultFromLastError();
	}

	return S_OK;
}

inline CCryptHash::CCryptHash() throw() :
	m_hHash( NULL )
{
}

inline CCryptHash::CCryptHash( const CCryptHash& hash ) throw()
{
	m_hHash = hash.Duplicate();
}

inline CCryptHash::CCryptHash( HCRYPTHASH hHash, BOOL bTakeOwnership ) throw()
{
	if (bTakeOwnership)
		m_hHash = hHash;
	else
	{
		m_hHash = NULL;
		BOOL bRet = ::CryptDuplicateHash( hHash, NULL, 0, &m_hHash );
		if (!bRet)
			m_hHash = NULL;
	}
}

inline CCryptHash::~CCryptHash() throw()
{
	Destroy();
}

inline void CCryptHash::Attach( HCRYPTHASH hHash, BOOL bTakeOwnership ) throw()
{
	ATLASSUME( m_hHash == NULL );

	if (bTakeOwnership)
		m_hHash = hHash;
	else
	{
		m_hHash = NULL;
		BOOL bRet = ::CryptDuplicateHash( hHash, NULL, 0, &m_hHash );
		if (!bRet)
			m_hHash = NULL;
	}
}

inline void CCryptHash::Destroy() throw()
{
	if( m_hHash != NULL )
	{
		BOOL bSuccess;

		bSuccess = ::CryptDestroyHash( m_hHash );

		// can fail if the cryptographic service provider
		// (managed by CCryptProv) has already been destroyed
		ATLASSERT( bSuccess );
		m_hHash = NULL;
	}
}

inline HCRYPTHASH CCryptHash::Detach() throw()
{
	HCRYPTHASH hHash;

	hHash = m_hHash;
	m_hHash = NULL;

	return hHash;
}

inline HCRYPTHASH CCryptHash::Duplicate() const throw()
{
	BOOL bSuccess;
	HCRYPTHASH hHash;

	ATLASSUME( m_hHash != NULL );

	hHash = NULL;
	bSuccess = ::CryptDuplicateHash( m_hHash, NULL, 0, &hHash );
	if( !bSuccess )
	{
		return NULL;
	}

	return hHash;
}

inline HRESULT CCryptHash::Uninitialize() throw()
{
	ATLASSUME(m_hHash != NULL);

	if (!CryptDestroyHash(m_hHash))
	{
		return AtlHresultFromLastError();
	}
	else 
	{
		m_hHash = NULL;
		return S_OK;
	}
}

inline HRESULT CCryptHash::Detach(HCRYPTHASH * phHash) throw()
{
	ATLASSERT(phHash);
	if (!phHash)
		return E_INVALIDARG;

	*phHash = m_hHash;
	m_hHash = NULL;

	return S_OK;
}

inline HRESULT CCryptHash::AddData(const BYTE * pbData, DWORD dwDataLen, DWORD dwFlags) throw()
{
	ATLASSUME(m_hHash != NULL);

	if (!CryptHashData(m_hHash, pbData, dwDataLen, dwFlags))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;

}

inline HRESULT CCryptHash::AddString(LPCTSTR szData, DWORD dwFlags) throw()
{
	return AddData((BYTE *)szData, (DWORD)_tcslen(szData) * sizeof(TCHAR), dwFlags);
}

inline HRESULT CCryptHash::GetParam(DWORD dwParam, BYTE * pbData, DWORD * pdwDataLen, DWORD dwFlags) throw()
{
	ATLASSUME(m_hHash != NULL);

	if (!CryptGetHashParam(m_hHash, dwParam, pbData, pdwDataLen, dwFlags))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptHash::SetParam(DWORD dwParam, BYTE * pbData, DWORD dwFlags) throw()
{
	ATLASSUME(m_hHash != NULL);

	if (!CryptSetHashParam(m_hHash, dwParam, pbData, dwFlags))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptHash::GetAlgId(ALG_ID * pAlgId) throw()
{
	DWORD dwSize = sizeof(ALG_ID);
	return GetParam(HP_ALGID, (BYTE *)pAlgId, &dwSize);
}

inline HRESULT CCryptHash::GetSize(DWORD * pdwSize) throw()
{
	DWORD dwLength = sizeof(DWORD);
	return GetParam(HP_HASHSIZE, (BYTE *)pdwSize, &dwLength);
}

inline HRESULT CCryptHash::GetValue(BYTE * pBuf, DWORD * pdwSize) throw()
{
	return GetParam(HP_HASHVAL, pBuf, pdwSize);
}

inline HRESULT CCryptHash::SetValue(BYTE * pBuf) throw()
{
	return SetParam(HP_HASHVAL, pBuf);
}

inline HRESULT CCryptHash::Sign(
	BYTE * pbSignature,
	DWORD * pdwSigLen,
	DWORD dwFlags,
	DWORD dwKeySpec) throw()
{
	ATLASSUME(m_hHash != NULL);

	if (!CryptSignHash(m_hHash, dwKeySpec, NULL, dwFlags, pbSignature, pdwSigLen))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptHash::VerifySignature(
	const BYTE * pbSignature,
	DWORD dwSigLen,
	CCryptKey &PubKey,
	DWORD dwFlags) throw()
{
	ATLASSUME(m_hHash != NULL);

	if (!CryptVerifySignature(m_hHash, pbSignature, dwSigLen, PubKey.GetHandle(), NULL, dwFlags))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

__declspec(selectany) CCryptHash CCryptHash::EmptyHash = CCryptHash();
__declspec(selectany) CCryptKey CCryptKey::EmptyKey = CCryptKey();
inline CCryptKey::CCryptKey() throw() :
	m_hKey( NULL )
{
}

inline CCryptKey::CCryptKey( const CCryptKey& key ) throw()
{
	m_hKey = key.Duplicate();
}

inline CCryptKey::CCryptKey( HCRYPTKEY hKey, BOOL bTakeOwnership ) throw()
{
	if (bTakeOwnership)
		m_hKey = hKey;
	else
	{
		BOOL bSuccess = ::CryptDuplicateKey( hKey, NULL, 0, &m_hKey );
		if( !bSuccess )
			m_hKey = NULL;
	}
}

inline CCryptKey::~CCryptKey() throw()
{
	Destroy();
}

inline void CCryptKey::Attach( HCRYPTKEY hKey, BOOL bTakeOwnership ) throw()
{
	ATLASSUME( m_hKey == NULL );
	if (bTakeOwnership)
		m_hKey = hKey;
	else
	{
		BOOL bSuccess = ::CryptDuplicateKey( hKey, NULL, 0, &m_hKey );
		if( !bSuccess )
			m_hKey = NULL;
	}
}

inline void CCryptKey::Destroy() throw()
{
	if( m_hKey != NULL )
	{
		BOOL bSuccess;

		bSuccess = ::CryptDestroyKey( m_hKey );

		// can fail if the cryptographic service provider
		// (managed by CCryptProv) has already been destroyed
		ATLASSERT( bSuccess );
		m_hKey = NULL;
	}
}

inline HCRYPTKEY CCryptKey::Detach() throw()
{
	HCRYPTKEY hKey;

	hKey = m_hKey;
	m_hKey = NULL;

	return( hKey );
}

inline HCRYPTKEY CCryptKey::Duplicate() const throw()
{
	BOOL bSuccess;

	ATLASSUME( m_hKey != NULL );

	HCRYPTKEY hKey = NULL;
	bSuccess = ::CryptDuplicateKey( m_hKey, NULL, 0, &hKey );
	if( !bSuccess )
		return NULL;

	return hKey;
}

inline HRESULT CCryptKey::Uninitialize() throw()
{
	ATLASSUME(m_hKey != NULL);

	if (!CryptDestroyKey(m_hKey))
	{
		return AtlHresultFromLastError();
	}
	else 
	{
		m_hKey = NULL;
		return S_OK;
	}
}	

inline HRESULT CCryptKey::Encrypt(
	BOOL final,
	BYTE * pbData,
	DWORD * pdwDataLen,
	DWORD dwBufLen,
	CCryptHash &Hash) throw()
{
	ATLASSUME(m_hKey != NULL);

	if (!::CryptEncrypt(m_hKey, Hash.GetHandle(), final, 0, pbData, pdwDataLen, dwBufLen))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;

}

inline HRESULT CCryptKey::Decrypt(BOOL final, BYTE * pbData, DWORD * pdwDataLen, CCryptHash &Hash) throw()
{
	ATLASSUME(m_hKey != NULL);

	if (!::CryptDecrypt(m_hKey, Hash.GetHandle(), final, 0, pbData, pdwDataLen))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}


inline HRESULT CCryptKey::Encrypt(
	const BYTE * pbPlainText,
	DWORD dwPlainTextLen,
	BYTE * pbCipherText,
	DWORD * pdwCipherTextLen,
	CCryptHash &Hash) throw()
{
	ATLASSUME(m_hKey != NULL);

	if (*pdwCipherTextLen < dwPlainTextLen)
		return ERROR_MORE_DATA;

	Checked::memcpy_s(pbCipherText, dwPlainTextLen, pbPlainText, dwPlainTextLen);
	DWORD dwSize = dwPlainTextLen;
	if (!::CryptEncrypt(m_hKey, Hash.GetHandle(), TRUE, 0, pbCipherText, &dwSize, *pdwCipherTextLen))
	{
		return AtlHresultFromLastError();
	}

	*pdwCipherTextLen = dwSize;
	return S_OK;

}

inline HRESULT CCryptKey::Decrypt(
	const BYTE * pbCipherText,
	DWORD dwCipherTextLen,
	BYTE * pbPlainText,
	DWORD * pdwPlainTextLen,
	CCryptHash &Hash) throw()
{
	ATLASSUME(m_hKey != NULL);

	if (*pdwPlainTextLen < dwCipherTextLen)
		return ERROR_MORE_DATA;

	Checked::memcpy_s(pbPlainText, dwCipherTextLen, pbCipherText, dwCipherTextLen);
	DWORD dwSize = dwCipherTextLen;
	if (!::CryptDecrypt(m_hKey, Hash.GetHandle(), TRUE, 0, pbPlainText, &dwSize))
	{
		return AtlHresultFromLastError();
	}

	*pdwPlainTextLen = dwSize;
	return S_OK;
}

inline HRESULT CCryptKey::EncryptString(
	LPCTSTR szPlainText,
	BYTE * pbCipherText,
	DWORD * pdwCipherTextLen,
	CCryptHash &Hash) throw()
{
	DWORD dwSize = ((DWORD)_tcslen(szPlainText) + 1) * sizeof(TCHAR);
	return Encrypt((BYTE *)szPlainText, dwSize, pbCipherText, pdwCipherTextLen, Hash);
}

inline HRESULT CCryptKey::ExportSimpleBlob(
	CCryptKey &ExpKey,
	DWORD dwFlags,
	BYTE * pbData,
	DWORD * pdwDataLen) throw()
{
	ATLASSUME(m_hKey != NULL);

	if (!CryptExportKey(m_hKey, ExpKey.GetHandle(), SIMPLEBLOB, dwFlags, pbData, pdwDataLen))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptKey::ExportPublicKeyBlob(
	CCryptKey &ExpKey,
	DWORD dwFlags,
	BYTE * pbData,
	DWORD * pdwDataLen) throw()
{
	ATLASSUME(m_hKey != NULL);

	if (!CryptExportKey(m_hKey, ExpKey.GetHandle(), PUBLICKEYBLOB, dwFlags, pbData, pdwDataLen))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptKey::ExportPrivateKeyBlob(
	CCryptKey &ExpKey,
	DWORD dwFlags,
	BYTE * pbData,
	DWORD * pdwDataLen) throw()
{
	ATLASSUME(m_hKey != NULL);

	if (!CryptExportKey(m_hKey, ExpKey.GetHandle(), PRIVATEKEYBLOB, dwFlags, pbData, pdwDataLen))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptKey::GetParam(DWORD dwParam, BYTE * pbData, DWORD * pdwDataLen, DWORD dwFlags) throw()
{
	ATLASSUME(m_hKey != NULL);

	if (!CryptGetKeyParam(m_hKey, dwParam, pbData, pdwDataLen, dwFlags))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptKey::SetParam(DWORD dwParam, BYTE * pbData, DWORD dwFlags) throw()
{
	ATLASSUME(m_hKey != NULL);

	if (!CryptSetKeyParam(m_hKey, dwParam, pbData, dwFlags))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptKey::GetAlgId(ALG_ID * pAlgId) throw()
{
	DWORD dwSize = sizeof(DWORD);
	return GetParam(KP_ALGID, (BYTE *)pAlgId, &dwSize);
}

inline HRESULT CCryptKey::SetAlgId(ALG_ID AlgId, DWORD dwFlags) throw()
{
	return SetParam(KP_ALGID, (BYTE *)&AlgId, dwFlags);
}

inline HRESULT CCryptKey::GetBlockLength(DWORD * pdwBlockLen) throw()
{
	DWORD dwSize = sizeof(DWORD);
	return GetParam(KP_BLOCKLEN, (BYTE *)pdwBlockLen, &dwSize);
}

inline HRESULT CCryptKey::GetKeyLength(DWORD * pdwKeyLen) throw()
{
	DWORD dwSize = sizeof(DWORD);
	return GetParam(KP_KEYLEN, (BYTE *)pdwKeyLen, &dwSize);
}

inline HRESULT CCryptKey::GetSalt(BYTE * pbSalt, DWORD * pdwLength) throw()
{
	return GetParam(KP_SALT, pbSalt, pdwLength);
}

inline HRESULT CCryptKey::SetSalt(BYTE * pbSalt) throw()
{
	return SetParam(KP_SALT, pbSalt);
}

inline HRESULT CCryptKey::SetSaltEx(_CRYPTOAPI_BLOB * pBlobSalt) throw()
{
	return SetParam(KP_SALT_EX, (BYTE *)pBlobSalt);
}

inline HRESULT CCryptKey::GetPermissions(DWORD * pdwPerms) throw()
{
	DWORD dwSize = sizeof(DWORD);
	return GetParam(KP_PERMISSIONS, (BYTE *)pdwPerms, &dwSize);
}

inline HRESULT CCryptKey::SetPermissions(DWORD dwPerms) throw()
{
	return SetParam(KP_PERMISSIONS, (BYTE *)&dwPerms);
}

inline HRESULT CCryptKey::GetP(BYTE * pbP, DWORD * pdwLength) throw()
{
	return GetParam(KP_P, (BYTE *)pbP, pdwLength);
}

inline HRESULT CCryptKey::SetP(_CRYPTOAPI_BLOB * pBlobP) throw()
{
	return SetParam(KP_P, (BYTE *)pBlobP);
}

inline HRESULT CCryptKey::SetP(BYTE * pbP, DWORD dwLength) throw()
{
	_CRYPTOAPI_BLOB blob = { dwLength, pbP };
	return SetParam(KP_P, (BYTE *)&blob);
}

inline HRESULT CCryptKey::GetQ(BYTE * pbQ, DWORD * pdwLength) throw()
{
	return GetParam(KP_Q, (BYTE *)pbQ, pdwLength);
}

inline HRESULT CCryptKey::SetQ(_CRYPTOAPI_BLOB * pBlobQ) throw()
{
	return SetParam(KP_Q, (BYTE *)pBlobQ);
}

inline HRESULT CCryptKey::SetQ(BYTE * pbQ, DWORD dwLength) throw()
{
	_CRYPTOAPI_BLOB blob = { dwLength, pbQ };
	return SetParam(KP_Q, (BYTE *)&blob);
}

inline HRESULT CCryptKey::GetG(BYTE * pbG, DWORD * pdwLength) throw()
{
	return GetParam(KP_G, (BYTE *)pbG, pdwLength);
}

inline HRESULT CCryptKey::SetG(_CRYPTOAPI_BLOB * pBlobG) throw()
{
	return SetParam(KP_G, (BYTE *)pBlobG);
}

inline HRESULT CCryptKey::SetG(BYTE * pbG, DWORD dwLength) throw()
{
	_CRYPTOAPI_BLOB blob = { dwLength, pbG };
	return SetParam(KP_G, (BYTE *)&blob);
}

inline HRESULT CCryptKey::SetX() throw()
{
	return SetParam(KP_X, NULL);
}

inline HRESULT CCryptKey::GetEffKeyLen(DWORD * pdwEffKeyLen) throw()
{
	DWORD dwSize = sizeof(DWORD);
	return GetParam(KP_EFFECTIVE_KEYLEN, (BYTE *)pdwEffKeyLen, &dwSize);
}

inline HRESULT CCryptKey::SetEffKeyLen(DWORD dwEffKeyLen) throw()
{
	return SetParam(KP_EFFECTIVE_KEYLEN, (BYTE *)&dwEffKeyLen);
}

inline HRESULT CCryptKey::GetPadding(DWORD * pdwPadding) throw()
{
	DWORD dwSize = sizeof(DWORD);
	return GetParam(KP_PADDING, (BYTE *)pdwPadding, &dwSize);
}

inline HRESULT CCryptKey::SetPadding(DWORD dwPadding) throw()
{
	return SetParam(KP_PADDING, (BYTE *)&dwPadding);
}

inline HRESULT CCryptKey::GetIV(BYTE * pbIV, DWORD * pdwLength) throw()
{
	return GetParam(KP_IV, pbIV, pdwLength);
}

inline HRESULT CCryptKey::SetIV(BYTE * pbIV) throw()
{
	return SetParam(KP_IV, pbIV);
}

inline HRESULT CCryptKey::GetMode(DWORD * pdwMode) throw()
{
	DWORD dwSize = sizeof(DWORD);
	return GetParam(KP_MODE, (BYTE *)pdwMode, &dwSize);
}

inline HRESULT CCryptKey::SetMode(DWORD dwMode) throw()
{
	return SetParam(KP_MODE, (BYTE *)&dwMode);
}

inline HRESULT CCryptKey::GetModeBits(DWORD * pdwModeBits) throw()
{
	DWORD dwSize = sizeof(DWORD);
	return GetParam(KP_MODE_BITS, (BYTE *)pdwModeBits, &dwSize);
}

inline HRESULT CCryptKey::SetModeBits(DWORD dwModeBits) throw()
{
	return SetParam(KP_MODE_BITS, (BYTE *)&dwModeBits);
}

inline HRESULT CCryptDerivedKey::Initialize(
	CCryptProv &Prov,
	CCryptHash &Hash,
	ALG_ID algid,
	DWORD dwFlags) throw()
{
	ATLASSUME(m_hKey == NULL);

	if (!CryptDeriveKey(Prov.GetHandle(), algid, Hash.GetHandle(), dwFlags, &m_hKey))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptRandomKey::Initialize(CCryptProv &Prov, ALG_ID algid, DWORD dwFlags) throw()
{
	ATLASSUME(m_hKey == NULL);

	if (!CryptGenKey(Prov.GetHandle(), algid, dwFlags, &m_hKey))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;

}

inline HRESULT CCryptUserExKey::Initialize(CCryptProv &Prov) throw()
{
	ATLASSUME(m_hKey == NULL);

	if (!CryptGetUserKey(Prov.GetHandle(), AT_KEYEXCHANGE, &m_hKey))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptUserExKey::Create(CCryptProv &Prov) throw()
{
	ATLASSUME(m_hKey == NULL);

	if (!CryptGenKey(Prov.GetHandle(), AT_KEYEXCHANGE, 0, &m_hKey))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptUserSigKey::Initialize(CCryptProv &Prov) throw()
{
	ATLASSUME(m_hKey == NULL);

	if (!CryptGetUserKey(Prov.GetHandle(), AT_SIGNATURE, &m_hKey))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptUserSigKey::Create(CCryptProv &Prov) throw()
{
	ATLASSUME(m_hKey == NULL);

	if (!CryptGenKey(Prov.GetHandle(), AT_SIGNATURE, 0, &m_hKey))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptImportKey::Initialize(
	CCryptProv &Prov,
	BYTE * pbData,
	DWORD dwDataLen,
	CCryptKey &PubKey,
	DWORD dwFlags) throw()
{
	ATLASSUME(m_hKey == NULL);

	if (!CryptImportKey(Prov.GetHandle(), pbData, dwDataLen, PubKey.GetHandle(), dwFlags, &m_hKey))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptKeyedHash::Initialize(
	CCryptProv &Prov,
	ALG_ID Algid,
	CCryptKey &Key,
	DWORD dwFlags) throw()
{
	ATLASSUME(m_hHash == NULL);

	if (!CryptCreateHash(Prov.GetHandle(), Algid, Key.GetHandle(), dwFlags, &m_hHash))
	{
		return AtlHresultFromLastError();
	}
	else return S_OK;
}

inline HRESULT CCryptMD5Hash::Initialize(CCryptProv &Prov, LPCTSTR szText) throw()
{
	ATLASSUME(m_hHash == NULL);

	if (!CryptCreateHash(Prov.GetHandle(), CALG_MD5, 0, 0, &m_hHash))
	{
		return AtlHresultFromLastError();
	}

	if (szText!=NULL)
		return AddString(szText);
	else return S_OK;
}

inline HRESULT CCryptMD4Hash::Initialize(CCryptProv &Prov, LPCTSTR szText) throw()
{
	ATLASSUME(m_hHash == NULL);

	if (!CryptCreateHash(Prov.GetHandle(), CALG_MD4, 0, 0, &m_hHash))
	{
		return AtlHresultFromLastError();
	}
	if (szText!=NULL)
		return AddString(szText);
	else return S_OK;
}

inline HRESULT CCryptMD2Hash::Initialize(CCryptProv &Prov, LPCTSTR szText) throw()
{
	ATLASSUME(m_hHash == NULL);

	if (!CryptCreateHash(Prov.GetHandle(), CALG_MD2, 0, 0, &m_hHash))
	{
		return AtlHresultFromLastError();
	}
	if (szText!=NULL)
		return AddString(szText);
	else return S_OK;
}

inline HRESULT CCryptSHAHash::Initialize(CCryptProv &Prov, LPCTSTR szText) throw()
{
	ATLASSUME(m_hHash == NULL);

	if (!CryptCreateHash(Prov.GetHandle(), CALG_SHA, 0, 0, &m_hHash))
	{
		return AtlHresultFromLastError();
	}
	if (szText!=NULL)
		return AddString(szText);
	else return S_OK;
}

inline HRESULT CCryptHMACHash::Initialize(CCryptProv &Prov, CCryptKey &Key, LPCTSTR szText) throw()
{
	ATLASSUME(m_hHash == NULL);

	if (!CryptCreateHash(Prov.GetHandle(), CALG_HMAC, Key.GetHandle(), 0, &m_hHash))
	{
		return AtlHresultFromLastError();
	}
	if (szText!=NULL)
		return AddString(szText);
	else return S_OK;

}

inline HRESULT CCryptMACHash::Initialize(CCryptProv &Prov, CCryptKey &Key, LPCTSTR szText) throw()
{
	ATLASSUME(m_hHash == NULL);

	if (!CryptCreateHash(Prov.GetHandle(), CALG_MAC, Key.GetHandle(), 0, &m_hHash))
	{
		return AtlHresultFromLastError();
	}
	if (szText!=NULL)
		return AddString(szText);
	else return S_OK;

}

inline HRESULT CCryptSSL3SHAMD5Hash::Initialize(CCryptProv &Prov, CCryptKey &Key, LPCTSTR szText) throw()
{
	ATLASSUME(m_hHash == NULL);

	if (!CryptCreateHash(Prov.GetHandle(), CALG_SSL3_SHAMD5, Key.GetHandle(), 0, &m_hHash))
	{
		return AtlHresultFromLastError();
	}
	if (szText!=NULL)
		return AddString(szText);
	else return S_OK;

}

}; // namespace ATL

#endif //__ATLCRYPT_INL__
