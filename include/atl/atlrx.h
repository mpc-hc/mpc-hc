// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLRX_H__
#define __ATLRX_H__

#pragma once

#ifdef _WIN32_WCE
	#error atlrx.h is not supported on Windows CE (_WIN32_WCE is defined)
#endif //_WIN32_WCE

#include <atlbase.h>
#include <atlcoll.h>
#include <mbstring.h>

#ifndef ATL_REGEXP_MIN_STACK
#define ATL_REGEXP_MIN_STACK 256
#endif

/* 
	Regular Expression Grammar

	R    - top level grammar rule
	RE   - regular expression
	AltE - Alternative expression
	E    - expression
	SE   - simple expression

	R -> RE
		 '^'RE		(matches begining of string)

	RE -> AltE RE
		  AltE


	AltE -> E
			E '|' AltE
	E -> SE (RepeatOp '?'?)?
	SE -> Arg
		Group
		CharClass
		'\'Abbrev		(see below)
		'\'EscapedChar	(any character including reserved symbols)
		'\'Digit+    (Arg back reference)
		'!'   (not)
		'.'   (any char)
		'$'   (end of input)
		Symbol			(any non-reserved character)
	Arg -> '{'RE'}'
	Group -> '('RE')'
	CharClass -> '[' '^'? CharSet ']'
	CharSet -> CharItem+
	CharItem -> Char('-'Char)?
	RepeatOp ->  '*'
				 '+'
				 '?'
	Abbrev -> Abbreviation defined in CAtlRECharTraits
		Abbrev  Expansion					Meaning
		a		([a-zA-Z0-9])				alpha numeric
		b		([ \\t])					white space (blank)
		c		([a-zA-Z])					alpha
		d		([0-9])						digit
		h		([0-9a-fA-F])				hex digit
		n		(\r|(\r?\n))				newline
		q		(\"[^\"]*\")|(\'[^\']*\')	quoted string
		w		([a-zA-Z]+)					simple word
		z		([0-9]+)					integer
*/

#pragma pack(push,_ATL_PACKING)
namespace ATL {

//Convertion utility classes used to convert char* to RECHAR.
//Used by rx debugging printing.
template <typename RECHARTYPE=char>
class CAToREChar
{
public:
	CAToREChar(const char* psz) throw()
	: m_psz(psz)
	{
	}
	operator const RECHARTYPE*() const throw() { return m_psz; }
	const char* m_psz;
};

template<>
class CAToREChar<wchar_t>
{
public:
	CAToREChar(const char* psz) throw()
	: m_a2w(psz)
	{
	}
	operator const wchar_t*() const throw() { return (wchar_t*)m_a2w; }
	
private:
	CA2W m_a2w;
};

class CAtlRECharTraitsA
{
public:
	typedef char RECHARTYPE;

	static size_t GetBitFieldForRangeArrayIndex(const RECHARTYPE *sz) throw()
	{
#ifndef ATL_NO_CHECK_BIT_FIELD
		ATLASSERT(UseBitFieldForRange());
#endif
		return static_cast<size_t>(static_cast<unsigned char>(*sz));		
	}
	static RECHARTYPE *Next(const RECHARTYPE *sz) throw()
	{
		return (RECHARTYPE *) (sz+1);
	}

	static int Strncmp(const RECHARTYPE *szLeft, const RECHARTYPE *szRight, size_t nCount) throw()
	{
		return strncmp(szLeft, szRight, nCount);
	}

	static int Strnicmp(const RECHARTYPE *szLeft, const RECHARTYPE *szRight, size_t nCount) throw()
	{
		return _strnicmp(szLeft, szRight, nCount);
	}

	_ATL_INSECURE_DEPRECATE("CAtlRECharTraitsA::Strlwr must be passed a buffer size.")
	static RECHARTYPE *Strlwr(RECHARTYPE *sz) throw()
	{
		#pragma warning (push)
		#pragma warning(disable : 4996)
		return _strlwr(sz);
		#pragma warning (pop)
	}

	static RECHARTYPE *Strlwr(RECHARTYPE *sz, int nSize) throw()
	{
		Checked::strlwr_s(sz, nSize);
		return sz;
	}

	static long Strtol(const RECHARTYPE *sz, RECHARTYPE **szEnd, int nBase) throw()
	{
		return strtol(sz, szEnd, nBase);
	}

	static int Isdigit(RECHARTYPE ch) throw()
	{
		return isdigit(static_cast<unsigned char>(ch));
	}

	static const RECHARTYPE** GetAbbrevs()
	{
		static const RECHARTYPE *s_szAbbrevs[] = 
		{
			"a([a-zA-Z0-9])",	// alpha numeric
			"b([ \\t])",		// white space (blank)
			"c([a-zA-Z])",	// alpha
			"d([0-9])",		// digit
			"h([0-9a-fA-F])",	// hex digit
			"n(\r|(\r?\n))",	// newline
			"q(\"[^\"]*\")|(\'[^\']*\')",	// quoted string
			"w([a-zA-Z]+)",	// simple word
			"z([0-9]+)",		// integer
			NULL
		};

		return s_szAbbrevs;
	}

	static BOOL UseBitFieldForRange() throw()
	{
		return TRUE;
	}

	static int ByteLen(const RECHARTYPE *sz) throw()
	{
		return int(strlen(sz));
	}
};

class CAtlRECharTraitsW
{
public:
	typedef WCHAR RECHARTYPE;
	
	static size_t GetBitFieldForRangeArrayIndex(const RECHARTYPE *sz) throw()
	{		
#ifndef ATL_NO_CHECK_BIT_FIELD
		ATLASSERT(UseBitFieldForRange());
#endif
		return static_cast<size_t>(*sz);
	}
	static RECHARTYPE *Next(const RECHARTYPE *sz) throw()
	{
		return (RECHARTYPE *) (sz+1);
	}

	static int Strncmp(const RECHARTYPE *szLeft, const RECHARTYPE *szRight, size_t nCount) throw()
	{
		return wcsncmp(szLeft, szRight, nCount);
	}

	static int Strnicmp(const RECHARTYPE *szLeft, const RECHARTYPE *szRight, size_t nCount) throw()
	{
		return _wcsnicmp(szLeft, szRight, nCount);
	}

	_ATL_INSECURE_DEPRECATE("CAtlRECharTraitsW::Strlwr must be passed a buffer size.")
	static RECHARTYPE *Strlwr(RECHARTYPE *sz) throw()
	{
		#pragma warning (push)
		#pragma warning(disable : 4996)
		return _wcslwr(sz);
		#pragma warning (pop)
	}

	static RECHARTYPE *Strlwr(RECHARTYPE *sz, int nSize) throw()
	{
		Checked::wcslwr_s(sz, nSize);
		return sz;
	}

	static long Strtol(const RECHARTYPE *sz, RECHARTYPE **szEnd, int nBase) throw()
	{
		return wcstol(sz, szEnd, nBase);
	}

	static int Isdigit(RECHARTYPE ch) throw()
	{
		return iswdigit(ch);
	}

	static const RECHARTYPE** GetAbbrevs()
	{
		static const RECHARTYPE *s_szAbbrevs[] = 
		{
			L"a([a-zA-Z0-9])",	// alpha numeric
			L"b([ \\t])",		// white space (blank)
			L"c([a-zA-Z])",	// alpha
			L"d([0-9])",		// digit
			L"h([0-9a-fA-F])",	// hex digit
			L"n(\r|(\r?\n))",	// newline
			L"q(\"[^\"]*\")|(\'[^\']*\')",	// quoted string
			L"w([a-zA-Z]+)",	// simple word
			L"z([0-9]+)",		// integer
			NULL
		};

		return s_szAbbrevs;
	}

	static BOOL UseBitFieldForRange() throw()
	{
		return FALSE;
	}

	static int ByteLen(const RECHARTYPE *sz) throw()
	{
		return int(wcslen(sz)*sizeof(WCHAR));
	}
};

class CAtlRECharTraitsMB
{
public:
	typedef unsigned char RECHARTYPE;

	static size_t GetBitFieldForRangeArrayIndex(const RECHARTYPE *sz) throw()
	{		
#ifndef ATL_NO_CHECK_BIT_FIELD
		ATLASSERT(UseBitFieldForRange());
#endif

		return static_cast<size_t>(*sz);		
	}

	static RECHARTYPE *Next(const RECHARTYPE *sz) throw()
	{
		return _mbsinc(sz);
	}

	static int Strncmp(const RECHARTYPE *szLeft, const RECHARTYPE *szRight, size_t nCount) throw()
	{
		return _mbsncmp(szLeft, szRight, nCount);
	}

	static int Strnicmp(const RECHARTYPE *szLeft, const RECHARTYPE *szRight, size_t nCount) throw()
	{
		return _mbsnicmp(szLeft, szRight, nCount);
	}

	_ATL_INSECURE_DEPRECATE("CAtlRECharTraitsMB::Strlwr must be passed a buffer size.")
	static RECHARTYPE *Strlwr(RECHARTYPE *sz) throw()
	{
		#pragma warning (push)
		#pragma warning(disable : 4996)
		return _mbslwr(sz);
		#pragma warning (pop)
	}

	static RECHARTYPE *Strlwr(RECHARTYPE *sz, int nSize) throw()
	{
		Checked::mbslwr_s(sz, nSize);
		return sz;
	}

	static long Strtol(const RECHARTYPE *sz, RECHARTYPE **szEnd, int nBase) throw()
	{
		return strtol((const char *) sz, (char **) szEnd, nBase);
	}

	static int Isdigit(RECHARTYPE ch) throw()
	{
		return _ismbcdigit((unsigned int) ch);
	}

	static const RECHARTYPE** GetAbbrevs()
	{
		return reinterpret_cast<const RECHARTYPE **>(CAtlRECharTraitsA::GetAbbrevs());
	}

	static BOOL UseBitFieldForRange() throw()
	{
		return FALSE;
	}

	static int ByteLen(const RECHARTYPE *sz) throw()
	{
		return (int)strlen((const char *) sz);
	}
};

#ifndef _UNICODE
typedef CAtlRECharTraitsA CAtlRECharTraits;
#else	// _UNICODE
typedef CAtlRECharTraitsW CAtlRECharTraits;
#endif // !_UNICODE
// Note: If you want to use CAtlRECharTraitsMB you must pass it in
// as a template argument

template <class CharTraits=CAtlRECharTraits>
class CAtlRegExp;	// forward declaration

template <class CharTraits=CAtlRECharTraits>
class CAtlREMatchContext
{
public:
	friend CAtlRegExp<CharTraits>;
	typedef typename CharTraits::RECHARTYPE RECHAR;

	struct MatchGroup
	{
		const RECHAR *szStart;
		const RECHAR *szEnd;
	};

	UINT m_uNumGroups;

	MatchGroup m_Match;

	void GetMatch(UINT nIndex, const RECHAR **szStart, const RECHAR **szEnd)
	{
		ATLENSURE(szStart != NULL);
		ATLENSURE(szEnd != NULL);
		ATLENSURE(nIndex >=0 && nIndex < m_uNumGroups);
		*szStart = m_Matches[nIndex].szStart;
		*szEnd = m_Matches[nIndex].szEnd;
	}

	void GetMatch(UINT nIndex, MatchGroup *pGroup)
	{
		 
		ATLENSURE(pGroup != NULL);
		ATLENSURE(nIndex >=0&&(static_cast<UINT>(nIndex))< m_uNumGroups);
		pGroup->szStart = m_Matches[nIndex].szStart;
		pGroup->szEnd = m_Matches[nIndex].szEnd;
	}

protected:
	CAutoVectorPtr<void *> m_Mem;
	CAutoVectorPtr<MatchGroup> m_Matches;
	CAtlArray<void *> m_stack;
	size_t m_nTos;

public:
	CAtlREMatchContext(size_t nInitStackSize=ATL_REGEXP_MIN_STACK)
	{
		m_uNumGroups = 0;
		m_nTos = 0;
		m_stack.SetCount(nInitStackSize);
		m_Match.szStart = NULL;
		m_Match.szEnd = NULL;
	}

protected:
	BOOL Initialize(UINT uRequiredMem, UINT uNumGroups) throw()
	{
		m_nTos = 0;

		m_uNumGroups = 0;
		m_Matches.Free();

		if (!m_Matches.Allocate(uNumGroups))
			return FALSE;

		m_uNumGroups = uNumGroups;

		m_Mem.Free();

		if (!m_Mem.Allocate(uRequiredMem))
			return FALSE;

		memset(m_Mem.m_p, 0x00, uRequiredMem*sizeof(void *));

		memset(m_Matches, 0x00, m_uNumGroups * sizeof(MatchGroup));
		return TRUE;
	}

	BOOL Push(void *p)
	{
		m_nTos++;
		if (m_stack.GetCount() <= (UINT) m_nTos)
		{
			if (!m_stack.SetCount((m_nTos+1)*2))
			{
				m_nTos--;
				return FALSE;
			}
		}
		m_stack[m_nTos] = p;
		return TRUE;
	}

	BOOL Push(size_t n)
	{
		return Push((void *) n);
	}

	void *Pop() throw()
	{
		if (m_nTos==0)
		{
			// stack underflow
			// this should never happen at match time.
			// (the parsing succeeded when it shouldn't have)
			ATLASSERT(FALSE);
			return NULL;
		}
		void *p = m_stack[m_nTos];
		m_nTos--;
		return p;
	}
};

enum REParseError {
	REPARSE_ERROR_OK = 0,				// No error occurred
	REPARSE_ERROR_OUTOFMEMORY,			// Out of memory
	REPARSE_ERROR_BRACE_EXPECTED,		// A closing brace was expected
	REPARSE_ERROR_PAREN_EXPECTED,		// A closing parenthesis was expected
	REPARSE_ERROR_BRACKET_EXPECTED,		// A closing bracket was expected
	REPARSE_ERROR_UNEXPECTED,			// An unspecified fatal error occurred
	REPARSE_ERROR_EMPTY_RANGE,			// A range expression was empty
	REPARSE_ERROR_INVALID_GROUP,		// A backreference was made to a group
										// that did not exist
	REPARSE_ERROR_INVALID_RANGE,		// An invalid range was specified
	REPARSE_ERROR_EMPTY_REPEATOP,		// A possibly empty * or + was detected
	REPARSE_ERROR_INVALID_INPUT,		// The input string was invalid
};

template <class CharTraits /* =CAtlRECharTraits */>
class CAtlRegExp
{
public:
	CAtlRegExp() throw()
	{
		m_uNumGroups = 0;
		m_uRequiredMem = 0;
		m_bCaseSensitive = TRUE;
		m_LastError = REPARSE_ERROR_OK;
	}

	typedef typename CharTraits::RECHARTYPE RECHAR;

	// CAtlRegExp::Parse
	// Parses the regular expression
	// returns REPARSE_ERROR_OK if successful, an REParseError otherwise
	REParseError Parse(const RECHAR *szRE, BOOL bCaseSensitive=TRUE)
	{
		ATLASSERT(szRE);
		if (!szRE)
			return REPARSE_ERROR_INVALID_INPUT;

		Reset();

		m_bCaseSensitive = bCaseSensitive;

		const RECHAR *szInput = szRE;

		if (!bCaseSensitive)
		{
			// copy the string
			int nSize = CharTraits::ByteLen(szRE)+sizeof(RECHAR);
			szInput = (const RECHAR *) malloc(nSize);
			if (!szInput)
				return REPARSE_ERROR_OUTOFMEMORY;

			Checked::memcpy_s((char *) szInput, nSize, szRE, nSize);

			CharTraits::Strlwr(const_cast<RECHAR *>(szInput), nSize/sizeof(RECHAR));
		}
		const RECHAR *sz = szInput;

		int nCall = AddInstruction(RE_CALL);
		if (nCall < 0)
			return REPARSE_ERROR_OUTOFMEMORY;

		if (*sz == '^')
		{
			if (AddInstruction(RE_FAIL) < 0)
				return REPARSE_ERROR_OUTOFMEMORY;
			sz++;
		}
		else
		{
			if (AddInstruction(RE_ADVANCE) < 0)
				return REPARSE_ERROR_OUTOFMEMORY;
		}

		bool bEmpty = true;
		ParseRE(&sz, bEmpty);
		if (!GetLastParseError())
		{
			GetInstruction(nCall).call.nTarget = 2;

			if (AddInstruction(RE_MATCH) < 0)
				return REPARSE_ERROR_OUTOFMEMORY;
		}

		if (szInput != szRE)
			free((void *) szInput);

		return GetLastParseError();
	}

	BOOL Match(const RECHAR *szIn, CAtlREMatchContext<CharTraits> *pContext, const RECHAR **ppszEnd=NULL)
	{
		ATLASSERT(szIn);
		ATLASSERT(pContext);

		if (!szIn || !pContext)
			return FALSE;

		if (ppszEnd)
			*ppszEnd = NULL;

		const RECHAR *szInput = szIn;

		if (!m_bCaseSensitive)
		{
			int nSize = CharTraits::ByteLen(szIn)+sizeof(RECHAR);
			szInput = (const RECHAR *) malloc(nSize);
			if (!szInput)
				return FALSE;

			Checked::memcpy_s((char *) szInput, nSize, szIn, nSize);
			CharTraits::Strlwr(const_cast<RECHAR *>(szInput), nSize/sizeof(RECHAR));
		}

		if (!pContext->Initialize(m_uRequiredMem, m_uNumGroups))
		{
			if (szInput != szIn)
				free((void *) szInput);
			return FALSE;
		}

		size_t ip = 0;

		const RECHAR *sz = szInput;
		const RECHAR *szCurrInput = szInput;

#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant

		while (1)
		{
#ifdef ATLRX_DEBUG
			OnDebugEvent(ip, szInput, sz, pContext);
#endif
			if (ip == 0)
				pContext->m_Match.szStart = sz;

			switch (GetInstruction(ip).type)
 			{
			case RE_NOP:
				ip++;
				break;

			case RE_SYMBOL:
				if (GetInstruction(ip).symbol.nSymbol == static_cast<size_t>(static_cast<_TUCHAR>(*sz)))
				{
					sz = CharTraits::Next(sz);
					ip++;
				}
				else
				{
					ip = (size_t) pContext->Pop();
				}
				break;

			case RE_ANY:
				if (*sz)
				{
					sz = CharTraits::Next(sz);
					ip++;
				}
				else
				{
					ip = (size_t) pContext->Pop();
				}
				break;

			case RE_GROUP_START:
				pContext->m_Matches[GetInstruction(ip).group.nGroup].szStart = sz;
				ip++;
				break;

			case RE_GROUP_END:
				pContext->m_Matches[GetInstruction(ip).group.nGroup].szEnd = sz;
				ip++;
				break;

			case RE_PUSH_CHARPOS:
				pContext->Push((void *) sz);
				ip++;
				break;

			case RE_POP_CHARPOS:
				sz = (RECHAR *) pContext->Pop();
				ip++;
				break;

			case RE_CALL:
				pContext->Push(ip+1);
				ip = GetInstruction(ip).call.nTarget;
				break;

			case RE_JMP:
				ip = GetInstruction(ip).jmp.nTarget;
				break;

			case RE_RETURN:
				ip = (size_t) pContext->Pop();
				break;

			case RE_PUSH_MEMORY:
				pContext->Push((void *) (pContext->m_Mem[GetInstruction(ip).memory.nIndex]));
				ip++;
				break;

			case RE_POP_MEMORY:
				pContext->m_Mem[GetInstruction(ip).memory.nIndex] = pContext->Pop();
				ip++;
				break;

			case RE_STORE_CHARPOS:
				pContext->m_Mem[GetInstruction(ip).memory.nIndex] = (void *) sz;
				ip++;
				break;

			case RE_GET_CHARPOS:
				sz = (RECHAR *) pContext->m_Mem[GetInstruction(ip).memory.nIndex];
				ip++;
				break;

			case RE_STORE_STACKPOS:
				pContext->m_Mem[GetInstruction(ip).memory.nIndex] = (void *) pContext->m_nTos;
				ip++;
				break;

			case RE_GET_STACKPOS:
				pContext->m_nTos = (size_t) pContext->m_Mem[GetInstruction(ip).memory.nIndex];
				ip++;
				break;

			case RE_RET_NOMATCH:
				if (sz == (RECHAR *) pContext->m_Mem[GetInstruction(ip).memory.nIndex])
				{
					// do a return
					ip = (size_t) pContext->Pop();
				}
				else
					ip++;
				break;

			case RE_ADVANCE:
				sz = CharTraits::Next(szCurrInput);
				szCurrInput = sz;
				if (*sz == '\0')
					goto Error;
				ip = 0;
				pContext->m_nTos = 0;
				break;

			case RE_FAIL:
				goto Error;

			case RE_RANGE:
				{
					if (*sz == '\0')
					{
						ip = (size_t) pContext->Pop();
						break;
					}

					RECHAR *pBits = reinterpret_cast<RECHAR *>((&m_Instructions[ip]+1));
					size_t u = CharTraits::GetBitFieldForRangeArrayIndex(sz);
					if (pBits[u >> 3] & 1 << (u & 0x7))
					{
						ip += InstructionsPerRangeBitField();
						ip++;
						sz = CharTraits::Next(sz);
					}
					else
					{
						ip = (size_t) pContext->Pop();
					}
				}
				break;

			case RE_NOTRANGE:
				{
					if (*sz == '\0')
					{
						ip = (size_t) pContext->Pop();
						break;
					}

					RECHAR *pBits = reinterpret_cast<RECHAR *>((&m_Instructions[ip]+1));
					size_t u = static_cast<size_t>(static_cast<_TUCHAR>(* ((RECHAR *) sz)));
					if (pBits[u >> 3] & 1 << (u & 0x7))
					{
						ip = (size_t) pContext->Pop();
					}
					else
					{
						ip += InstructionsPerRangeBitField();
						ip++;
						sz = CharTraits::Next(sz);
					}
				}
				break;

			case RE_RANGE_EX:
				{
					if (*sz == '\0')
					{
						ip = (size_t) pContext->Pop();
						break;
					}

					BOOL bMatch = FALSE;
					size_t inEnd = GetInstruction(ip).range.nTarget;
					ip++;

					while (ip < inEnd)
					{						
						if (static_cast<size_t>(static_cast<_TUCHAR>(*sz)) >= GetInstruction(ip).memory.nIndex && 
							static_cast<size_t>(static_cast<_TUCHAR>(*sz)) <= GetInstruction(ip+1).memory.nIndex)
						{
							// if we match, we jump to the end
							sz = CharTraits::Next(sz);
							ip = inEnd;
							bMatch = TRUE;
						}
						else
						{
							ip += 2;
						}
					}
					if (!bMatch)
					{
						ip = (size_t) pContext->Pop();
					}
				}
				break;

			case RE_NOTRANGE_EX:
				{
					if (*sz == '\0')
					{
						ip = (size_t) pContext->Pop();
						break;
					}

					BOOL bMatch = TRUE;
					size_t inEnd = GetInstruction(ip).range.nTarget;
					ip++;

					while (ip < inEnd)
					{
						if (static_cast<size_t>(static_cast<_TUCHAR>(*sz)) >= GetInstruction(ip).memory.nIndex && 
							static_cast<size_t>(static_cast<_TUCHAR>(*sz)) <= GetInstruction(ip+1).memory.nIndex)
						{
							ip = (size_t) pContext->Pop();
							bMatch = FALSE;
							break;
						}
						else
						{
							// if we match, we jump to the end
							ip += 2;
						}
					}
					if (bMatch)
						sz = CharTraits::Next(sz);
				}
				break;

			case RE_PREVIOUS:
				{
					BOOL bMatch = FALSE;
					if (m_bCaseSensitive)
					{
						bMatch = !CharTraits::Strncmp(sz, pContext->m_Matches[GetInstruction(ip).prev.nGroup].szStart,
							pContext->m_Matches[GetInstruction(ip).prev.nGroup].szEnd-pContext->m_Matches[GetInstruction(ip).prev.nGroup].szStart);
					}
					else
					{
						bMatch = !CharTraits::Strnicmp(sz, pContext->m_Matches[GetInstruction(ip).prev.nGroup].szStart,
							pContext->m_Matches[GetInstruction(ip).prev.nGroup].szEnd-pContext->m_Matches[GetInstruction(ip).prev.nGroup].szStart);
					}
					if (bMatch)
					{
						sz += pContext->m_Matches[GetInstruction(ip).prev.nGroup].szEnd-pContext->m_Matches[GetInstruction(ip).prev.nGroup].szStart;
						ip++;
						break;
					}
					ip = (size_t) pContext->Pop();
				}
				break;

			case RE_MATCH:
				pContext->m_Match.szEnd = sz;
				if (!m_bCaseSensitive)
					FixupMatchContext(pContext, szIn, szInput);
				if (ppszEnd)
					*ppszEnd = szIn + (sz - szInput);
				if (szInput != szIn)
					free((void *) szInput);
				return TRUE;
				break;

			case RE_PUSH_GROUP:
				pContext->Push((void *) pContext->m_Matches[GetInstruction(ip).group.nGroup].szStart);
				pContext->Push((void *) pContext->m_Matches[GetInstruction(ip).group.nGroup].szEnd);
				ip++;
				break;

			case RE_POP_GROUP:
				pContext->m_Matches[GetInstruction(ip).group.nGroup].szEnd = (const RECHAR *) pContext->Pop();
				pContext->m_Matches[GetInstruction(ip).group.nGroup].szStart = (const RECHAR *) pContext->Pop();
				ip++;
				break;

			default:
				ATLASSERT(FALSE);
				break;
			}
		}

#pragma warning(pop) // 4127

		ATLASSERT(FALSE);
Error:
		pContext->m_Match.szEnd = sz;
		if (!m_bCaseSensitive)
			FixupMatchContext(pContext, szIn, szInput);
		if (ppszEnd)
			*ppszEnd = szIn + (sz - szInput);
		if (szInput != szIn)
			free((void *) szInput);
		return FALSE;
	}

protected:
	REParseError m_LastError;

	REParseError GetLastParseError() throw()
	{
		return m_LastError;
	}

	void SetLastParseError(REParseError Error) throw()
	{
		m_LastError = Error;
	}
	// CAtlRegExp::Reset
	// Removes all instructions to allow reparsing into the same instance
	void Reset() throw()
	{
		m_Instructions.RemoveAll();
		m_uRequiredMem = 0;
		m_bCaseSensitive = TRUE;
		m_uNumGroups = 0;
		SetLastParseError(REPARSE_ERROR_OK);
	}


	enum REInstructionType { 
		RE_NOP,
		RE_GROUP_START,
		RE_GROUP_END, 
		RE_SYMBOL,
		RE_ANY,
		RE_RANGE,
		RE_NOTRANGE,
		RE_RANGE_EX,
		RE_NOTRANGE_EX,
		RE_PLUS,
		RE_NG_PLUS,
		RE_QUESTION,
		RE_NG_QUESTION,
		RE_JMP,
		RE_PUSH_CHARPOS,
		RE_POP_CHARPOS,
		RE_CALL,
		RE_RETURN,
		RE_STAR_BEGIN,
		RE_NG_STAR_BEGIN, 
		RE_PUSH_MEMORY,
		RE_POP_MEMORY,
		RE_STORE_CHARPOS,
		RE_STORE_STACKPOS,
		RE_GET_CHARPOS,
		RE_GET_STACKPOS,
		RE_RET_NOMATCH,
		RE_PREVIOUS,
		RE_FAIL,
		RE_ADVANCE,
		RE_MATCH,
		RE_PUSH_GROUP,
		RE_POP_GROUP,
	};

	struct INSTRUCTION_SYMBOL
	{
		size_t nSymbol;
	};

	struct INSTRUCTION_JMP
	{
		size_t nTarget;	
	};

	struct INSTRUCTION_GROUP
	{
		size_t nGroup;
	};

	struct INSTRUCTION_CALL
	{
		size_t nTarget;
	};

	struct INSTRUCTION_MEMORY
	{
		size_t nIndex;
	};

	struct INSTRUCTION_PREVIOUS
	{
		size_t nGroup;
	};

	struct INSTRUCTION_RANGE_EX
	{
		size_t nTarget;
	};

	struct INSTRUCTION
	{
		REInstructionType type;
		union
		{
			INSTRUCTION_SYMBOL symbol;
			INSTRUCTION_JMP jmp;
			INSTRUCTION_GROUP group;
			INSTRUCTION_CALL call;
			INSTRUCTION_MEMORY memory;
			INSTRUCTION_PREVIOUS prev;
			INSTRUCTION_RANGE_EX range;
		};
	};

	inline int InstructionsPerRangeBitField() throw()
	{
		return (256/8) / sizeof(INSTRUCTION) + (((256/8) % sizeof(INSTRUCTION)) ? 1 : 0);
	}

	CAtlArray<INSTRUCTION> m_Instructions;

	UINT m_uNumGroups;
	UINT m_uRequiredMem;
	BOOL m_bCaseSensitive;


	// class used internally to restore
	// parsing state when unwinding
	class CParseState
	{
	public:
		int m_nNumInstructions;
		UINT m_uNumGroups;
		UINT m_uRequiredMem;

		CParseState(CAtlRegExp *pRegExp) throw()
		{
			m_nNumInstructions = (int) pRegExp->m_Instructions.GetCount();
			m_uNumGroups = pRegExp->m_uNumGroups;
			m_uRequiredMem = pRegExp->m_uRequiredMem;
		}

		void Restore(CAtlRegExp *pRegExp)
		{
			pRegExp->m_Instructions.SetCount(m_nNumInstructions);
			pRegExp->m_uNumGroups = m_uNumGroups;
			pRegExp->m_uRequiredMem = m_uRequiredMem;
		}
	};

	int AddInstruction(REInstructionType type)
	{
		if (!m_Instructions.SetCount(m_Instructions.GetCount()+1))
		{
			SetLastParseError(REPARSE_ERROR_OUTOFMEMORY);
			return -1;
		}

		m_Instructions[m_Instructions.GetCount()-1].type = type;
		return (int) m_Instructions.GetCount()-1;
	}

	BOOL PeekToken(const RECHAR **ppszRE, int ch) throw()
	{
		if (**ppszRE != ch)
			return FALSE;
		return TRUE;
	}

	BOOL MatchToken(const RECHAR **ppszRE, int ch) throw()
	{
		if (!PeekToken(ppszRE, ch))
			return FALSE;
		*ppszRE = CharTraits::Next(*ppszRE);
		return TRUE;
	}

	INSTRUCTION &GetInstruction(size_t nIndex) throw()
	{
		return m_Instructions[nIndex];
	}

	// ParseArg: parse grammar rule Arg
	int ParseArg(const RECHAR **ppszRE, bool &bEmpty)
	{
		int nPushGroup = AddInstruction(RE_PUSH_GROUP);
		if (nPushGroup < 0)
			return -1;

		GetInstruction(nPushGroup).group.nGroup = m_uNumGroups;

		int p = AddInstruction(RE_GROUP_START);
		if (p < 0)
			return -1;
		GetInstruction(p).group.nGroup = m_uNumGroups++;

		int nCall = AddInstruction(RE_CALL);
		if (nCall < 0)
			return -1;

		int nPopGroup = AddInstruction(RE_POP_GROUP);
		if (nPopGroup < 0)
			return -1;
		GetInstruction(nPopGroup).group.nGroup = GetInstruction(nPushGroup).group.nGroup;

		if (AddInstruction(RE_RETURN) < 0)
			return -1;

		int nAlt = ParseRE(ppszRE, bEmpty);
		if (nAlt < 0)
		{
			if (GetLastParseError())
				return -1;

			if (!PeekToken(ppszRE, '}'))
			{
				SetLastParseError(REPARSE_ERROR_BRACE_EXPECTED);
				return -1;
			}

			// in the case of an empty group, we add a nop
			nAlt = AddInstruction(RE_NOP);
			if (nAlt < 0)
				return -1;
		}

		GetInstruction(nCall).call.nTarget = nAlt;

		if (!MatchToken(ppszRE, '}'))
		{
			SetLastParseError(REPARSE_ERROR_BRACE_EXPECTED);
			return -1;
		}

		int nEnd = AddInstruction(RE_GROUP_END);
		if (nEnd < 0)
			return -1;
		GetInstruction(nEnd).group.nGroup = GetInstruction(p).group.nGroup;
		return nPushGroup;
	}

	// ParseGroup: parse grammar rule Group
	int ParseGroup(const RECHAR **ppszRE, bool &bEmpty)
	{
		int nCall = AddInstruction(RE_CALL);
		if (nCall < 0)
			return -1;

		if (AddInstruction(RE_RETURN) < 0)
			return -1;

		int nAlt = ParseRE(ppszRE, bEmpty);
		if (nAlt < 0)
		{
			if (GetLastParseError())
				return -1;

			if (!PeekToken(ppszRE, ')'))
			{
				SetLastParseError(REPARSE_ERROR_PAREN_EXPECTED);
				return -1;
			}

			// in the case of an empty group, we add a nop
			nAlt = AddInstruction(RE_NOP);
			if (nAlt < 0)
				return -1;
		}

		GetInstruction(nCall).call.nTarget = nAlt;

		if (!MatchToken(ppszRE, ')'))
		{
			SetLastParseError(REPARSE_ERROR_PAREN_EXPECTED);
			return -1;
		}

		return nCall;
	}

	RECHAR GetEscapedChar(RECHAR ch) throw()
	{
		if (ch == 't')
			return '\t';
		return ch;
	}

	// ParseCharItem: parse grammar rule CharItem
	int ParseCharItem(const RECHAR **ppszRE, RECHAR *pchStartChar, RECHAR *pchEndChar) throw()
	{
		if (**ppszRE == '\\')
		{
			*ppszRE = CharTraits::Next(*ppszRE);
			*pchStartChar = GetEscapedChar(**ppszRE);
		}
		else
			*pchStartChar = **ppszRE;
		*ppszRE = CharTraits::Next(*ppszRE);

		if (!MatchToken(ppszRE, '-'))
		{
			*pchEndChar = *pchStartChar;
			return 0;
		}

		// check for unterminated range
		if (!**ppszRE || PeekToken(ppszRE, ']'))
		{
			SetLastParseError(REPARSE_ERROR_BRACKET_EXPECTED);
			return -1;
		}

		*pchEndChar = **ppszRE;
		*ppszRE = CharTraits::Next(*ppszRE);

		if (*pchEndChar < *pchStartChar)
		{
			SetLastParseError(REPARSE_ERROR_INVALID_RANGE);
			return -1;
		}
		return 0;
	}

	int AddInstructions(int nNumInstructions)
	{
		size_t nCurr = m_Instructions.GetCount();
		if (!m_Instructions.SetCount(nCurr+nNumInstructions))
		{
			SetLastParseError(REPARSE_ERROR_OUTOFMEMORY);
			return -1;
		}
		return (int) nCurr;
	}

	// ParseCharSet: parse grammar rule CharSet
	int ParseCharSet(const RECHAR **ppszRE, BOOL bNot)
	{
		int p = -1;

		unsigned char *pBits = NULL;

		if (CharTraits::UseBitFieldForRange())
		{
			// we use a bit field to represent the characters
			// a 1 bit means match against the character
			// the last 5 bits are used as an index into 
			// the byte array, and the first 3 bits
			// are used to index into the selected byte

			p = AddInstruction(bNot ? RE_NOTRANGE : RE_RANGE);
			if (p < 0)
				return -1;

			// add the required space to hold the character
			// set.  We use one bit per character for ansi
			if (AddInstructions(InstructionsPerRangeBitField()) < 0)
				return -1;

			pBits = (unsigned char *) (&m_Instructions[p+1]);
			memset(pBits, 0x00, 256/8);
		}
		else
		{
			p = AddInstruction(bNot ? RE_NOTRANGE_EX : RE_RANGE_EX);
			if (p < 0)
				return -1;
		}

		RECHAR chStart;
		RECHAR chEnd;

		while (**ppszRE && **ppszRE != ']')
		{
			if (ParseCharItem(ppszRE, &chStart, &chEnd))
				return -1;

			if (CharTraits::UseBitFieldForRange())
			{
				for (int i=chStart; i<=chEnd; i++)
					pBits[i >> 3] |= 1 << (i & 0x7);
			}
			else
			{
				int nStart = AddInstruction(RE_NOP);
				if (nStart < 0)
					return -1;

				int nEnd = AddInstruction(RE_NOP);
				if (nEnd < 0)
					return -1;

				GetInstruction(nStart).memory.nIndex = (int) chStart;
				GetInstruction(nEnd).memory.nIndex = (int) chEnd;
			}
		}

		if (!CharTraits::UseBitFieldForRange())
			GetInstruction(p).range.nTarget = m_Instructions.GetCount();

		return p;
	}

	// ParseCharClass: parse grammar rule CharClass
	int ParseCharClass(const RECHAR **ppszRE, bool &bEmpty)
	{
		bEmpty = false;
		if (MatchToken(ppszRE, ']'))
		{
			SetLastParseError(REPARSE_ERROR_EMPTY_RANGE);
			return -1;
		}

		BOOL bNot = FALSE;
		if (MatchToken(ppszRE, '^'))
			bNot = TRUE;

		if (MatchToken(ppszRE, ']'))
		{
			SetLastParseError(REPARSE_ERROR_EMPTY_RANGE);
			return -1;
		}

		int p = ParseCharSet(ppszRE, bNot);
		if (p < 0)
			return p;
		if (!MatchToken(ppszRE, ']'))
		{
			SetLastParseError(REPARSE_ERROR_BRACKET_EXPECTED);
			return -1;
		}

		return p;
	}

	int AddMemInstruction(REInstructionType type)
	{
		int p = AddInstruction(type);
		if (p < 0)
			return p;
		GetInstruction(p).memory.nIndex = m_uRequiredMem++;
		return p;
	}

	// helper for parsing !SE
	int ParseNot(const RECHAR **ppszRE, bool &bEmpty)
	{
		int nStoreCP = AddMemInstruction(RE_STORE_CHARPOS);
		int nStoreSP = AddMemInstruction(RE_STORE_STACKPOS);

		int nCall = AddInstruction(RE_CALL);
		if (nCall < 0)
			return -1;

		int nGetCP = AddInstruction(RE_GET_CHARPOS);
		if (nGetCP < 0)
			return -1;
		GetInstruction(nGetCP).memory.nIndex = GetInstruction(nStoreCP).memory.nIndex;

		int nGetSP = AddInstruction(RE_GET_STACKPOS);
		if (nGetSP < 0)
			return -1;
		GetInstruction(nGetSP).memory.nIndex = GetInstruction(nStoreSP).memory.nIndex;

		int nJmp = AddInstruction(RE_JMP);
		if (nJmp < 0)
			return -1;

		int nSE = ParseSE(ppszRE, bEmpty);
		if (nSE < 0)
			return nSE;

		// patch the call
		GetInstruction(nCall).call.nTarget = nSE;

		int nGetCP1 = AddInstruction(RE_GET_CHARPOS);
		if (nGetCP1 < 0)
			return -1;
		GetInstruction(nGetCP1).memory.nIndex = GetInstruction(nStoreCP).memory.nIndex;

		int nGetSP1 = AddInstruction(RE_GET_STACKPOS);
		if (nGetSP1 < 0)
			return -1;
		GetInstruction(nGetSP1).memory.nIndex = GetInstruction(nStoreSP).memory.nIndex;

		int nRet = AddInstruction(RE_RETURN);
		if (nRet < 0)
			return -1;

		GetInstruction(nJmp).jmp.nTarget = nRet+1;

		return nStoreCP;
	}

	// ParseAbbrev: parse grammar rule Abbrev
	int ParseAbbrev(const RECHAR **ppszRE, bool &bEmpty)
	{
		const RECHAR **szAbbrevs = CharTraits::GetAbbrevs();

		while (*szAbbrevs)
		{
			if (**ppszRE == **szAbbrevs)
			{
				const RECHAR *szAbbrev = (*szAbbrevs)+1;
				int p = ParseE(&szAbbrev, bEmpty);
				if (p < 0)
				{
					SetLastParseError(REPARSE_ERROR_UNEXPECTED);
					return p;
				}
				*ppszRE = CharTraits::Next(*ppszRE);
				return p;
			}
			szAbbrevs++;
		}
		return -1;
	}

	// ParseSE: parse grammar rule SE (simple expression)
	int ParseSE(const RECHAR **ppszRE, bool &bEmpty)
	{

		if (MatchToken(ppszRE, '{'))
			return ParseArg(ppszRE, bEmpty);
		if (MatchToken(ppszRE, '('))
			return ParseGroup(ppszRE, bEmpty);
		if (MatchToken(ppszRE, '['))
			return ParseCharClass(ppszRE, bEmpty);

		if (MatchToken(ppszRE, '\\'))
		{
			if (!CharTraits::Isdigit(**ppszRE))
			{
				// check for abbreviations
				int p;
				p = ParseAbbrev(ppszRE, bEmpty);
				if (p >= 0)
					return p;

				if (GetLastParseError())
					return -1;

				// escaped char
				p = AddInstruction(RE_SYMBOL);
				if (p < 0)
					return -1;
				GetInstruction(p).symbol.nSymbol = (int) **ppszRE;
				*ppszRE = CharTraits::Next(*ppszRE);
				return p;
			}
			// previous match
			bEmpty = false;
			int nPrev = AddInstruction(RE_PREVIOUS);
			if (nPrev < 0)
				return -1;

			UINT uValue = (UINT) CharTraits::Strtol(*ppszRE, (RECHAR **) ppszRE, 10);
			if (uValue >= m_uNumGroups)
			{
				SetLastParseError(REPARSE_ERROR_INVALID_GROUP);
				return -1;
			}
			GetInstruction(nPrev).prev.nGroup = (size_t) uValue;
			return nPrev;
		}

		if (MatchToken(ppszRE, '!'))
			return ParseNot(ppszRE, bEmpty);

		if (**ppszRE == '}' || **ppszRE == ']' || **ppszRE == ')')
		{
			return -1;
		}

		if (**ppszRE == '\0')
		{
			return -1;
		}

		int p;
		if (**ppszRE == '.')
		{
			p = AddInstruction(RE_ANY);
			if (p < 0)
				return -1;
			bEmpty = false;
		}
		else if (**ppszRE == '$' && (*ppszRE)[1] == '\0')
		{
			p = AddInstruction(RE_SYMBOL);
			if (p < 0)
				return -1;
			GetInstruction(p).symbol.nSymbol = 0;
			bEmpty = false;
		}
		else
		{
			p = AddInstruction(RE_SYMBOL);
			if (p < 0)
				return -1;
			GetInstruction(p).symbol.nSymbol = (int) **ppszRE;
			bEmpty = false;
		}
		*ppszRE = CharTraits::Next(*ppszRE);
		return p;
	}

	// ParseE: parse grammar rule E (expression)
	int ParseE(const RECHAR **ppszRE, bool &bEmpty)
	{
		CParseState ParseState(this);
		const RECHAR *sz = *ppszRE;

		int nSE;

		int nFirst = ParseSE(ppszRE, bEmpty);
		if (nFirst < 0)
			return nFirst;

		REInstructionType type = RE_MATCH;

		if (MatchToken(ppszRE, '*'))
			if(MatchToken(ppszRE, '?'))
				type = RE_NG_STAR_BEGIN;
			else
				type = RE_STAR_BEGIN;


		else if (MatchToken(ppszRE, '+'))
			if(MatchToken(ppszRE, '?'))
				type = RE_NG_PLUS;
			else
				type = RE_PLUS;

		else if (MatchToken(ppszRE, '?'))
			if(MatchToken(ppszRE, '?'))
				type = RE_NG_QUESTION;
			else
				type = RE_QUESTION;


		if (type == RE_MATCH)
			return nFirst;

		if (type == RE_STAR_BEGIN || type == RE_QUESTION|| type == RE_NG_STAR_BEGIN || type == RE_NG_QUESTION)
		{
			ParseState.Restore(this);
		}
		else
		{
			m_uNumGroups = ParseState.m_uNumGroups;
		}
		*ppszRE = sz;

		int nE;

		if (type == RE_NG_STAR_BEGIN || type == RE_NG_PLUS || type == RE_NG_QUESTION) // Non-Greedy
		{			
			int nCall = AddInstruction(RE_CALL);
			if (nCall < 0)
				return -1;

			bEmpty = false;

			nSE = ParseSE(ppszRE, bEmpty);
			if (nSE < 0)
				return nSE;

			if (bEmpty && (type == RE_NG_STAR_BEGIN || type == RE_NG_PLUS))
			{
				SetLastParseError(REPARSE_ERROR_EMPTY_REPEATOP);
				return -1;
			}
			bEmpty = true;

			*ppszRE = CharTraits::Next(*ppszRE);
			*ppszRE = CharTraits::Next(*ppszRE);

			if (type == RE_NG_STAR_BEGIN || type == RE_NG_PLUS)
			{
				int nJmp = AddInstruction(RE_JMP);
				if (nJmp < 0)
					return -1;
				GetInstruction(nCall).call.nTarget = nJmp+1;
				GetInstruction(nJmp).jmp.nTarget = nCall;
			}
			else
				GetInstruction(nCall).call.nTarget = nSE+1;

			if (type == RE_NG_PLUS)
				nE = nFirst;
			else
				nE = nCall;
		}
		else // Greedy
		{

			int nPushMem = AddInstruction(RE_PUSH_MEMORY);
			if (nPushMem < 0)
				return -1;

			int nStore = AddInstruction(RE_STORE_CHARPOS);
			if (nStore < 0)
				return -1;

			if (AddInstruction(RE_PUSH_CHARPOS) < 0)
				return -1;

			int nCall = AddInstruction(RE_CALL);
			if (nCall < 0)
				return -1;

			if (AddInstruction(RE_POP_CHARPOS) < 0)
				return -1;

			int nPopMem = AddInstruction(RE_POP_MEMORY);
			if (nPopMem < 0)
				return -1;

			int nJmp = AddInstruction(RE_JMP);
			if (nJmp < 0)
				return -1;

			GetInstruction(nPushMem).memory.nIndex = m_uRequiredMem++;
			GetInstruction(nStore).memory.nIndex = GetInstruction(nPushMem).memory.nIndex;
			GetInstruction(nCall).call.nTarget = nJmp+1;
			GetInstruction(nPopMem).memory.nIndex = GetInstruction(nPushMem).memory.nIndex;

			bEmpty = false;

			nSE = ParseSE(ppszRE, bEmpty);
			if (nSE < 0)
				return nSE;

			if (bEmpty && (type == RE_STAR_BEGIN || type == RE_PLUS))
			{
				SetLastParseError(REPARSE_ERROR_EMPTY_REPEATOP);
				return -1;
			}

			if (type != RE_PLUS && type != RE_NG_PLUS)
				bEmpty = true;

			*ppszRE = CharTraits::Next(*ppszRE);


			int nRetNoMatch = AddInstruction(RE_RET_NOMATCH);
			if (nRetNoMatch < 0)
				return -1;

			int nStore1 = AddInstruction(RE_STORE_CHARPOS);
			if (nStore1 < 0)
				return -1;

			GetInstruction(nRetNoMatch).memory.nIndex = GetInstruction(nPushMem).memory.nIndex;
			GetInstruction(nStore1).memory.nIndex = GetInstruction(nPushMem).memory.nIndex;

			if (type != RE_QUESTION)
			{
				int nJmp1 = AddInstruction(RE_JMP);
				if (nJmp1 < 0)
					return -1;
				GetInstruction(nJmp1).jmp.nTarget = nPushMem;
			}

			GetInstruction(nJmp).jmp.nTarget = m_Instructions.GetCount();
			if (type == RE_PLUS)
				nE = nFirst;
			else
				nE = nPushMem;
		}

		return nE;
	}


	// ParseAltE: parse grammar rule AltE
	int ParseAltE(const RECHAR **ppszRE, bool &bEmpty)
	{
		const RECHAR *sz = *ppszRE;
		CParseState ParseState(this);

		int nPush = AddInstruction(RE_PUSH_CHARPOS);
		if (nPush < 0)
			return -1;

		int nCall = AddInstruction(RE_CALL);
		if (nCall < 0)
			return -1;

		GetInstruction(nCall).call.nTarget = nPush+4;
		if (AddInstruction(RE_POP_CHARPOS) < 0)
			return -1;

		int nJmpNext = AddInstruction(RE_JMP);
		if (nJmpNext < 0)
			return -1;

		int nE = ParseE(ppszRE, bEmpty);
		if (nE < 0)
		{
			if (GetLastParseError())
				return -1;
			ParseState.Restore(this);
			return nE;
		}

		int nJmpEnd = AddInstruction(RE_JMP);
		if (nJmpEnd < 0)
			return -1;

		GetInstruction(nJmpNext).jmp.nTarget = nJmpEnd+1;

		if (!MatchToken(ppszRE, '|'))
		{
			ParseState.Restore(this);
			*ppszRE = sz;

			return ParseE(ppszRE, bEmpty);
		}

		bool bEmptyAltE;
		int nAltE = ParseAltE(ppszRE, bEmptyAltE);
		GetInstruction(nJmpEnd).jmp.nTarget = m_Instructions.GetCount();
		GetInstruction(nJmpNext).jmp.nTarget = nAltE;
		if (nAltE < 0)
		{
			if (GetLastParseError())
				return -1;
			ParseState.Restore(this);
			return nAltE;
		}
		bEmpty = bEmpty | bEmptyAltE;
		return nPush;
	}

	// ParseRE: parse grammar rule RE (regular expression)
	int ParseRE(const RECHAR **ppszRE, bool &bEmpty)
	{
		if (**ppszRE == '\0')
			return -1;

		int p = ParseAltE(ppszRE, bEmpty);
		if (p < 0)
			return p;

		bool bEmptyRE = true;
		ParseRE(ppszRE, bEmptyRE);
		if (GetLastParseError())
			return -1;
		bEmpty = bEmpty && bEmptyRE;
		return p;
	}

	//pointers to the matched string and matched groups, currently point into an internal allocated 
	//buffer that hold a copy of the input string.
	//This function fix these pointers to point into the original, user supplied buffer (first param to Match method).
	//Example: If a ptr (szStart) currently point to <internal buffer>+3, it is fixed to <user supplied buffer>+3
	void FixupMatchContext(CAtlREMatchContext<CharTraits> *pContext, const RECHAR *szOrig, const RECHAR *szNew) 
	{
		ATLENSURE(pContext);
		ATLASSERT(szOrig);
		ATLASSERT(szNew);

		pContext->m_Match.szStart = szOrig + (pContext->m_Match.szStart - szNew);
		pContext->m_Match.szEnd = szOrig + (pContext->m_Match.szEnd - szNew);
		for (UINT i=0; i<pContext->m_uNumGroups; i++)
		{
			if (pContext->m_Matches[i].szStart==NULL || pContext->m_Matches[i].szEnd==NULL)
			{
				continue; //Do not fix unmatched groups.
			}
			pContext->m_Matches[i].szStart = szOrig + (pContext->m_Matches[i].szStart - szNew);
			pContext->m_Matches[i].szEnd = szOrig + (pContext->m_Matches[i].szEnd - szNew);
		}
	}
	// implementation
	// helpers for dumping and debugging the rx engine
public:
#ifdef ATL_REGEXP_DUMP
	size_t DumpInstruction(size_t ip)
	{
		printf("%08x ", ip);
		switch (GetInstruction(ip).type)
		{
		case RE_NOP:
			printf("NOP\n");
			ip++;
			break;

		case RE_SYMBOL:
			AtlprintfT<RECHAR>(CAToREChar<RECHAR>("Symbol %c\n"),GetInstruction(ip).symbol.nSymbol);			
			ip++;
			break;

		case RE_ANY:
			printf("Any\n");
			ip++;
			break;

		case RE_RANGE:
			printf("Range\n");
			ip++;
			ip += InstructionsPerRangeBitField();
			break;

		case RE_NOTRANGE:
			printf("NOT Range\n");
			ip++;
			ip += InstructionsPerRangeBitField();
			break;

		case RE_RANGE_EX:
			printf("RangeEx %08x\n", GetInstruction(ip).range.nTarget);
			ip++;
			break;

		case RE_NOTRANGE_EX:
			printf("NotRangeEx %08x\n", GetInstruction(ip).range.nTarget);
			ip++;
			break;

		case RE_GROUP_START:
			printf("Start group %d\n", GetInstruction(ip).group.nGroup);
			ip++;
			break;

		case RE_GROUP_END:
			printf("Group end %d\n", GetInstruction(ip).group.nGroup);
			ip++;
			break;

		case RE_PUSH_CHARPOS:
			printf("Push char pos\n");
			ip++;
			break;

		case RE_POP_CHARPOS:
			printf("Pop char pos\n");
			ip++;
			break;

		case RE_STORE_CHARPOS:
			printf("Store char pos %d\n", GetInstruction(ip).memory.nIndex);
			ip++;
			break;

		case RE_GET_CHARPOS:
			printf("Get char pos %d\n", GetInstruction(ip).memory.nIndex);
			ip++;
			break;

		case RE_STORE_STACKPOS:
			printf("Store stack pos %d\n", GetInstruction(ip).memory.nIndex);
			ip++;
			break;

		case RE_GET_STACKPOS:
			printf("Get stack pos %d\n", GetInstruction(ip).memory.nIndex);
			ip++;
			break;

		case RE_CALL:
			printf("Call %08x\n", GetInstruction(ip).call.nTarget);
			ip++;
			break;

		case RE_JMP:
			printf("Jump %08x\n", GetInstruction(ip).jmp.nTarget);
			ip++;
			break;

		case RE_RETURN:
			printf("return\n");
			ip++;
			break;

		case RE_PUSH_MEMORY:
			printf("Push memory %08x\n", GetInstruction(ip).memory.nIndex);
			ip++;
			break;

		case RE_POP_MEMORY:
			printf("Pop memory %08x\n", GetInstruction(ip).memory.nIndex);
			ip++;
			break;

		case RE_RET_NOMATCH:
			printf("Return no match %08x\n", GetInstruction(ip).memory.nIndex);
			ip++;
			break;

		case RE_MATCH:
			printf("END\n");
			ip++;
			break;

		case RE_ADVANCE:
			printf("ADVANCE\n");
			ip++;
			break;

		case RE_FAIL:
			printf("FAIL\n");
			ip++;
			break;

		case RE_PREVIOUS:
			printf("Prev %d\n", GetInstruction(ip).prev.nGroup);
			ip++;
			break;

		case RE_PUSH_GROUP:
			printf("Push group %d\n", GetInstruction(ip).group.nGroup);
			ip++;
			break;

		case RE_POP_GROUP:
			printf("Pop group %d\n", GetInstruction(ip).group.nGroup);
			ip++;
			break;


		default:
			printf("????\n");
			ip++;
			break;
		}
		return ip;
	}

	void Dump(size_t ipCurrent = 0)
	{
		size_t ip = 0;

		while (ip < m_Instructions.GetCount())
		{
			if (ip == ipCurrent)
				printf("->");
			ip = DumpInstruction(ip);
		}
	}
#endif

#ifdef ATLRX_DEBUG
	 void cls( HANDLE hConsole )
	{
		COORD coordScreen = { 0, 0 };    /* here's where we'll home the
											cursor */ 
		BOOL bSuccess;
		DWORD cCharsWritten;
		CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */ 
		DWORD dwConSize;                 /* number of character cells in
											the current buffer */ 

		/* get the number of character cells in the current buffer */ 

		bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
		dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

		/* fill the entire screen with blanks */ 

		bSuccess = FillConsoleOutputCharacter( hConsole, (TCHAR) ' ',
		   dwConSize, coordScreen, &cCharsWritten );

		/* get the current text attribute */ 

		bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );

		/* now set the buffer's attributes accordingly */ 

		bSuccess = FillConsoleOutputAttribute( hConsole, csbi.wAttributes,
		   dwConSize, coordScreen, &cCharsWritten );

		/* put the cursor at (0, 0) */ 

		bSuccess = SetConsoleCursorPosition( hConsole, coordScreen );
		return;
	} 

	void DumpStack(CAtlREMatchContext<CharTraits> *pContext)
	{
		for (size_t i=pContext->m_nTos; i>0; i--)
		{
			if (pContext->m_stack[i] < (void *) m_Instructions.GetCount())
				printf("0x%p\n", pContext->m_stack[i]);
			else
			{
				// assume a pointer into the input
				AtlprintfT<RECHAR>(CAToREChar<RECHAR>("%s\n"), pContext->m_stack[i]);
			}
		}
	}

	void DumpMemory(CAtlREMatchContext<CharTraits> *pContext)
	{
		for (UINT i=0; i<m_uRequiredMem; i++)
		{
			AtlprintfT<RECHAR>(CAToREChar<RECHAR>("%d: %s\n"), i, pContext->m_Mem.m_p[i]);
		}
	}

	virtual void OnDebugEvent(size_t ip, const RECHAR *szIn, const RECHAR *sz, CAtlREMatchContext<CharTraits> *pContext)
	{
		cls(GetStdHandle(STD_OUTPUT_HANDLE));
		printf("----------Code---------\n");
		Dump(ip);
		printf("----------Input---------\n");		
		AtlprintfT<RECHAR>(CAToREChar<RECHAR>("%s\n"), szIn);
		for (int s=0; szIn+s < sz; s++)
		{
			printf(" ");
		}		
		printf("^\n");
		printf("----------Memory---------\n");
		DumpMemory(pContext);
		printf("----------Stack---------\n");		
		DumpStack(pContext);
		getchar();
	}
#endif

};

} // namespace ATL
#pragma pack(pop)

#endif // __ATLRX_H__
