// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLCRYPT_H__
#define __ATLCRYPT_H__

#pragma once

#include <atlchecked.h>
#include <wincrypt.h>


#pragma pack(push,_ATL_PACKING)
namespace ATL
{

class CCryptKey;

class CCryptProv
{
protected:
	HCRYPTPROV m_hProv;

public:
	CCryptProv() throw();
	CCryptProv( const CCryptProv& prov ) throw();
	explicit CCryptProv( HCRYPTPROV hProv, BOOL bTakeOwnership = FALSE ) throw();
	~CCryptProv() throw();

	CCryptProv& operator=( const CCryptProv& prov ) throw();

	HRESULT AddRef() throw();
	void Attach( HCRYPTPROV hProv, BOOL bTakeOwnership = FALSE ) throw();
	HCRYPTPROV Detach() throw();
	HRESULT Release() throw();


	HRESULT Initialize(DWORD dwProviderType = PROV_RSA_FULL, 
		LPCTSTR szContainer = NULL, LPCTSTR szProvider = MS_DEF_PROV,
		DWORD dwFlags = 0) throw();
	HRESULT InitVerifyContext(DWORD dwProviderType = PROV_RSA_FULL, 
		LPCTSTR szProvider = MS_DEF_PROV, DWORD dwFlags = 0) throw();
	HRESULT InitCreateKeySet(DWORD dwProviderType = PROV_RSA_FULL,
		LPCTSTR szContainer = NULL, LPCTSTR szProvider = MS_DEF_PROV,
		DWORD dwFlags = 0) throw();

	HRESULT DeleteKeySet(DWORD dwProviderType = PROV_RSA_FULL, 
		LPCTSTR szContainer = NULL, LPCTSTR szProvider = MS_DEF_PROV,
		DWORD dwFlags = 0) throw();

	HRESULT Uninitialize();

	HRESULT GetParam(DWORD dwParam, BYTE * pbData, DWORD * pdwDataLen, DWORD dwFlags = 0) throw();
	HRESULT SetParam( DWORD dwParam, BYTE* pbData, DWORD dwFlags = 0) throw();
	HRESULT GetName(__out_ecount_part_z(*pdwLength, *pdwLength) LPSTR szBuf, __inout DWORD * pdwLength) throw();
	HRESULT GetContainer(__out_ecount_part_z(*pdwLength, *pdwLength) LPSTR szBuf, __inout DWORD * pdwLength) throw();
	HRESULT GetImpType(DWORD * pdwImpType) throw();
	HRESULT GetVersion(DWORD * pdwVersion) throw();
	HRESULT GetProvType(DWORD * pdwType) throw();
	HRESULT GetSecurityDesc(SECURITY_INFORMATION * pSecInfo) throw();
	HRESULT SetSecurityDesc(SECURITY_INFORMATION SecInfo) throw();

	HRESULT GenRandom(ULONG nLength, BYTE* pbBuffer ) throw();

	inline HCRYPTPROV GetHandle() throw()
	{
		return m_hProv;
	}
}; // class CCryptProv


// class CCryptHash
// Provides base functionality of hashes.
class CCryptHash
{
protected:
	HCRYPTHASH m_hHash;

public:
	CCryptHash() throw();
	CCryptHash( const CCryptHash& hash ) throw();
	explicit CCryptHash( HCRYPTHASH hHash, BOOL bTakeOwnership = FALSE ) throw();
	~CCryptHash() throw();

	void Attach( HCRYPTHASH hHash, BOOL bTakeOwnership = FALSE ) throw();
	void Destroy() throw();
	HCRYPTHASH Detach() throw();
	HCRYPTHASH Duplicate() const throw();

	HRESULT Uninitialize() throw();
	HRESULT Detach(HCRYPTHASH * phHash) throw();
	HRESULT AddData(const BYTE * pbData, DWORD dwDataLen, DWORD dwFlags = 0) throw();
	HRESULT AddString(LPCTSTR szData, DWORD dwFlags = 0) throw();
	HRESULT GetParam(DWORD dwParam, BYTE * pbData, DWORD * pdwDataLen, DWORD dwFlags = 0) throw();
	HRESULT SetParam(DWORD dwParam, BYTE * pbData, DWORD dwFlags = 0) throw();
	HRESULT GetAlgId(ALG_ID * pAlgId) throw();
	HRESULT GetSize(DWORD * pdwSize) throw();
	HRESULT GetValue(BYTE * pBuf, DWORD * pdwSize) throw();
	HRESULT SetValue(BYTE * pBuf) throw();
	HRESULT Sign(
		BYTE * pbSignature,
		DWORD * pdwSigLen,
		DWORD dwFlags = 0,
		DWORD dwKeySpec = AT_SIGNATURE) throw();
	HRESULT VerifySignature(
		const BYTE * pbSignature,
		DWORD pdwSignLen,
		CCryptKey &PubKey,
		DWORD dwFlags = 0) throw();

	inline HCRYPTHASH GetHandle()
	{
		return m_hHash;
	}
	static CCryptHash EmptyHash;

}; // class CCryptHash

// class CCryptKey
// Provides the functionality for cryptographic keys, i.e. encrypting, decrypting.
class CCryptKey
{
protected:
	HCRYPTKEY m_hKey;

public:
	CCryptKey() throw();
	CCryptKey( const CCryptKey& key ) throw();
	explicit CCryptKey( HCRYPTKEY hKey, BOOL bTakeOwnership = FALSE ) throw();
	~CCryptKey() throw();

	void Attach( HCRYPTKEY hKey, BOOL bTakeOwnership = FALSE ) throw();
	void Destroy() throw();
	HCRYPTKEY Detach() throw();
	HCRYPTKEY Duplicate() const throw();

	HRESULT Uninitialize() throw();
	HRESULT Encrypt(
		BOOL final,
		BYTE * pbData,
		DWORD * pdwDataLen,
		DWORD dwBufLen,
		CCryptHash &Hash = CCryptHash::EmptyHash) throw();

	HRESULT Encrypt(
		const BYTE * pbPlainText,
		DWORD dwPlainTextLen,
		BYTE * pbCipherText,
		DWORD * pdwCipherTextLen,
		CCryptHash &Hash = CCryptHash::EmptyHash) throw();

	HRESULT Decrypt(
		BOOL final,
		BYTE * pbData,
		DWORD * pdwDataLen,
		CCryptHash &Hash = CCryptHash::EmptyHash) throw();
	HRESULT Decrypt(
		const BYTE * pbCipherText,
		DWORD dwCipherTextLen,
		BYTE * pbPlainText,
		DWORD * pdwPlainTextLen,
		CCryptHash &Hash = CCryptHash::EmptyHash) throw();
	HRESULT EncryptString(
		LPCTSTR szPlainText,
		BYTE * pbCipherText,
		DWORD * pdwCipherTextLen,
		CCryptHash &Hash = CCryptHash::EmptyHash) throw();
	HRESULT ExportSimpleBlob(
		CCryptKey &ExpKey,
		DWORD dwFlags,
		BYTE * pbData,
		DWORD * pdwDataLen) throw();
	HRESULT ExportPublicKeyBlob(
		CCryptKey &ExpKey,
		DWORD dwFlags,
		BYTE * pbData,
		DWORD * pdwDataLen) throw();
	HRESULT ExportPrivateKeyBlob(
		CCryptKey &ExpKey,
		DWORD dwFlags,
		BYTE * pbData,
		DWORD * pdwDataLen) throw();
	HRESULT GetParam(DWORD dwParam, BYTE * pbData, DWORD * pdwDataLen, DWORD dwFlags = 0) throw();
	HRESULT SetParam(DWORD dwParam, BYTE * pbData, DWORD dwFlags = 0) throw();
	HRESULT GetAlgId(ALG_ID * pAlgId) throw();
	HRESULT SetAlgId(ALG_ID AlgId, DWORD dwFlags) throw();
	HRESULT GetBlockLength(DWORD * pdwBlockLen) throw();
	HRESULT GetKeyLength(DWORD * pdwKeyLen) throw();
	HRESULT GetSalt(BYTE * pbSalt, DWORD * pdwLength) throw();
	HRESULT SetSalt(BYTE * pbSalt) throw();
	HRESULT SetSaltEx(_CRYPTOAPI_BLOB * pBlobSalt) throw();
	HRESULT GetPermissions(DWORD * pdwPerms) throw();
	HRESULT SetPermissions(DWORD dwPerms) throw();
	HRESULT GetP(BYTE * pbP, DWORD * pdwLength) throw();
	HRESULT SetP(_CRYPTOAPI_BLOB * pBlobP) throw();
	HRESULT SetP(BYTE * pbP, DWORD dwLength) throw();
	HRESULT GetQ(BYTE * pbQ, DWORD * pdwLength) throw();
	HRESULT SetQ(_CRYPTOAPI_BLOB * pBlobQ) throw();
	HRESULT SetQ(BYTE * pbQ, DWORD dwLength) throw();
	HRESULT GetG(BYTE * pbG, DWORD * pdwLength) throw();
	HRESULT SetG(_CRYPTOAPI_BLOB * pBlobG) throw();
	HRESULT SetG(BYTE * pbG, DWORD dwLength) throw();
	HRESULT SetX() throw();
	HRESULT GetEffKeyLen(DWORD * pdwEffKeyLen) throw();
	HRESULT SetEffKeyLen(DWORD dwEffKeyLen) throw();
	HRESULT GetPadding(DWORD * pdwPadding) throw();
	HRESULT SetPadding(DWORD dwPadding) throw();
	HRESULT GetIV(BYTE * pbIV, DWORD * pdwLength) throw();
	HRESULT SetIV(BYTE * pbIV) throw();
	HRESULT GetMode(DWORD * pdwMode) throw();
	HRESULT SetMode(DWORD dwMode) throw();
	HRESULT GetModeBits(DWORD * pdwModeBits) throw();
	HRESULT SetModeBits(DWORD dwModeBits) throw();

	inline HCRYPTKEY GetHandle() throw()
	{
		return m_hKey;
	}

	static CCryptKey EmptyKey;
}; // class CCryptKey



// Specific instances of Keys and Hashes

// class CCryptDerivedKey
// A key that is derived from a hashed password.  Two keys derived 
// from the same password will be identical.
class CCryptDerivedKey : public CCryptKey
{
public:
	HRESULT Initialize(
		CCryptProv &Prov,
		CCryptHash &Hash,
		ALG_ID algid = CALG_RC4,
		DWORD dwFlags = CRYPT_EXPORTABLE) throw();
}; // class CCryptDerivedKey

// class CCryptRandomKey
// A randomly generated key.  Can be used internally by a program 
// to protect data during execution, or can be exported with Crypt.Export
//
// Currently it is possible to pass in AT_KEYEXCHANGE or AT_SIGNATURE 
// for algid, but these two will generate keys for the current key set, and 
// the resulting handle can only be used for exporting and importing keys or 
// signing hashes.
class CCryptRandomKey : public CCryptKey
{
public:
	HRESULT Initialize(
		CCryptProv &Prov,
		ALG_ID algid = CALG_RC4,
		DWORD dwFlags = CRYPT_EXPORTABLE) throw();
}; // class CCryptRandomKey

// class CCryptUserExKey
// Obtains the user's key exchange key pair.
class CCryptUserExKey : public CCryptKey
{
public:
	HRESULT Initialize(CCryptProv &Prov) throw();
	HRESULT Create(CCryptProv &Prov) throw();
}; // class CCryptUserExKey

// class CCryptUserSigKey
// Obtains the user's signature key pair
class CCryptUserSigKey : public CCryptKey
{
public:
	HRESULT Initialize(CCryptProv &Prov) throw();
	HRESULT Create(CCryptProv &Prov) throw();
}; // class CCryptUserSigKey

// class CCryptImportKey
// Forms a key from an imported key blob
class CCryptImportKey : public CCryptKey
{
public:
	HRESULT Initialize(
		CCryptProv &Prov,
		BYTE * pbData,
		DWORD dwDataLen,
		CCryptKey &PubKey,
		DWORD dwFlags) throw();
}; // class CCryptImportKey


// class CCryptHash
// A generic hash that may or may not take a key.  
class CCryptKeyedHash : public CCryptHash
{
public:

	HRESULT Initialize(CCryptProv &Prov, ALG_ID Algid, CCryptKey &Key, DWORD dwFlags) throw();
}; // class CCryptKeyedHash

// class CCryptMD5Hash
// RSA's MD5 hash (RSA's most recent hash as of 9/7/99);
class CCryptMD5Hash : public CCryptHash
{
public:

	HRESULT Initialize(CCryptProv &Prov, LPCTSTR szText = NULL) throw();
}; // class CCryptMD5Hash

// class CCryptMD4Hash
// RSA's MD4 hash
class CCryptMD4Hash : public CCryptHash
{
public:

	HRESULT Initialize(CCryptProv &Prov, LPCTSTR szText = NULL) throw();
}; // class CCryptMD4Hash


// class CCryptMD2Hash
// RSA's MD2 hash
class CCryptMD2Hash : public CCryptHash
{
public:

	HRESULT Initialize(CCryptProv &Prov, LPCTSTR szText = NULL) throw();
}; // class CCryptMD2Hash


// class CCryptSHAHash
// The Secure Hash Algorithm hash, from NIST and NSA.  Technically, SHA-1.
class CCryptSHAHash : public CCryptHash
{
public:

	HRESULT Initialize(CCryptProv &Prov, LPCTSTR szText = NULL) throw();
}; // class CCryptSHAHash

// The Secure Hash Algorithm, from NIST and NSA.  Identical to CCryptSHA
typedef CCryptSHAHash CCryptSHA1Hash;


// class CCryptHMACHash
// Hash-base Message Authentication Code keyed hash
class CCryptHMACHash : public CCryptHash
{
public:
	HRESULT Initialize(CCryptProv &Prov, CCryptKey &Key, LPCTSTR szText = NULL) throw();
}; // class CCryptHMACHash

// class CCryptMACHash
// Message Authentication Code keyed hash.  Believed to be less secure than HMAC
class CCryptMACHash : public CCryptHash
{
public:
	HRESULT Initialize(CCryptProv &Prov, CCryptKey &Key, LPCTSTR szText = NULL) throw();
}; // class CCryptMACHash

// class CCryptSSL3SHAMD5Hash
// Hash algorithm used by Secure Socket Layer
class CCryptSSL3SHAMD5Hash : public CCryptHash
{
public:
	HRESULT Initialize(CCryptProv &Prov, CCryptKey &Key, LPCTSTR szText = NULL) throw();
}; // class CCryptSSl3SHAMD5Hash

}; // namespace ATL
 

#include <atlcrypt.inl>
#pragma pack(pop)
#endif  // __ATLCRYPT_H__
