#pragma once

#include "..\BaseSplitter\BaseSplitter.h"
// #include "Ap4AsyncReaderStream.h" // FIXME

class CMP4SplitterFile : public CBaseSplitterFileEx
{
	void* /* AP4_File* */ m_pAp4File;

	HRESULT Init();

public:
	CMP4SplitterFile(IAsyncReader* pReader, HRESULT& hr);
	virtual ~CMP4SplitterFile();

	void* /* AP4_Movie* */ GetMovie();
};
