#include "stdafx.h"
#include "Ap4AsyncReaderStream.h"

AP4_AsyncReaderStream::AP4_AsyncReaderStream(CBaseSplitterFile* pFile) 
	: m_refs(1)
	, m_pFile(pFile)
{
	ASSERT(pFile);
}

AP4_AsyncReaderStream::~AP4_AsyncReaderStream()
{
	ASSERT(m_refs == 0);
}

void AP4_AsyncReaderStream::AddReference()
{
	ASSERT(m_refs > 0); 
	++m_refs;
}

void AP4_AsyncReaderStream::Release()
{
	ASSERT(m_refs > 0); 
	if(--m_refs == 0) delete this;
}

AP4_Result AP4_AsyncReaderStream::Read(void* buffer, AP4_Size bytesToRead, AP4_Size* bytesRead)
{
	__int64 bytesAvail = m_pFile->GetRemaining();

	if(bytesAvail < bytesToRead)
	{
		if(bytesRead) *bytesRead = bytesAvail;
		bytesToRead = bytesAvail;
	}

	if(bytesAvail == 0)
	{
		return AP4_ERROR_EOS;
	}

	if(FAILED(m_pFile->ByteRead((BYTE*)buffer, bytesToRead)))
	{
		if(bytesRead) *bytesRead = 0;
		return AP4_ERROR_READ_FAILED;
	}

	if(bytesRead) *bytesRead = bytesToRead;

	return AP4_SUCCESS;
}

AP4_Result AP4_AsyncReaderStream::Write(const void* buffer, AP4_Size bytesToWrite, AP4_Size* bytesWritten)
{
    return AP4_ERROR_WRITE_FAILED;
}

AP4_Result AP4_AsyncReaderStream::Seek(AP4_Offset offset)
{
	m_pFile->Seek(offset);
	return m_pFile->GetPos() == offset ? AP4_SUCCESS : AP4_FAILURE;
}

AP4_Result AP4_AsyncReaderStream::Tell(AP4_Offset& offset)
{
	offset = (AP4_Offset)m_pFile->GetPos();
	return AP4_SUCCESS;
}

AP4_Result AP4_AsyncReaderStream::GetSize(AP4_Size& size)
{
	size = (AP4_Size)m_pFile->GetLength();
	return AP4_SUCCESS;
}