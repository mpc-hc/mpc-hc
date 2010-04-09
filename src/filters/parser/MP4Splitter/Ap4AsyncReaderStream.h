#pragma once

#include "../BaseSplitter/BaseSplitter.h"

#include "Ap4.h"
#include "Ap4File.h"
//#include "Ap4Utils.h"
//#include "Ap4Sample.h"
#include "Ap4ByteStream.h"
//#include "Ap4SampleEntry.h"
//#include "Ap4IsmaCryp.h"
//#include "Ap4AvcCAtom.h"
//#include "Ap4FtabAtom.h"
//#include "Ap4MdhdAtom.h"
//#include "Ap4HdlrAtom.h"

class AP4_AsyncReaderStream : public AP4_ByteStream
{
	int m_refs;
	CBaseSplitterFile* m_pFile;

public:
	AP4_AsyncReaderStream(CBaseSplitterFile* pFile);
	virtual ~AP4_AsyncReaderStream();

	void AddReference();
	void Release();

	virtual AP4_Result ReadPartial(void* buffer, AP4_Size bytesToRead, AP4_Size& bytesRead);
	virtual AP4_Result WritePartial(const void* buffer, AP4_Size bytesToWrite, AP4_Size& bytesWritten);
	virtual AP4_Result Seek(AP4_Position offset);
	virtual AP4_Result Tell(AP4_Position& offset);
	virtual AP4_Result GetSize(AP4_LargeSize& size);
};