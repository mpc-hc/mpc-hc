// multisz.h: interface for the CMultiSz class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MULTISZ_H__17638488_0C54_431D_A8DC_85E5B85B89DB__INCLUDED_)
#define AFX_MULTISZ_H__17638488_0C54_431D_A8DC_85E5B85B89DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMultiSz  
{
public:
	void DisplayBuffer();
	BOOL FindString(const char *str) const;
	BOOL RemoveString(const char * str);
	BOOL AddString(const char * str);
	int m_MultiSzPos;

	void FirstPosition(int * position) const;
	void First();

	const char * NextPosition(int * position) const;
	const char * Next();

	inline const DWORD  GetBufferLen() const { return m_MultiSzLen; };
	inline const BYTE * GetBuffer() const { return m_MultiSzBuf; };
	BOOL SetBuffer(BYTE * buf, DWORD len);

	CMultiSz();
	CMultiSz(const CMultiSz& other);
	virtual ~CMultiSz();

	const CMultiSz& operator = (const CMultiSz& other);

private:
	DWORD m_MultiSzLen;
	BYTE * m_MultiSzBuf;
};

#endif // !defined(AFX_MULTISZ_H__17638488_0C54_431D_A8DC_85E5B85B89DB__INCLUDED_)
