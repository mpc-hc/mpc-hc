#pragma once

#include <atlbase.h>
#include <atlcoll.h>
#include <afxtempl.h>
#include "../BaseSplitter/BaseSplitter.h"

class CDiracSplitterFile : public CBaseSplitterFile
{
	CMediaType m_mt;
	REFERENCE_TIME m_rtDuration;
	CArray<BYTE> m_pBuff;

	HRESULT Init();

public:
	CDiracSplitterFile(IAsyncReader* pAsyncReader, HRESULT& hr);

//	using CBaseSplitterFile::Read;

	bool Next(BYTE& code, __int64 len = -1);
	const BYTE* NextBlock(BYTE& code, int& size, int& fnum);
	UINT64 UnsignedGolombDecode();

	const CMediaType& GetMediaType() {return m_mt;}
	REFERENCE_TIME GetDuration() {return m_rtDuration;}
};
