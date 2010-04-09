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

AP4_Result AP4_AsyncReaderStream::ReadPartial(void* buffer, AP4_Size bytesToRead, AP4_Size& bytesRead)
{
	AP4_Size bytesAvail = (AP4_Size)m_pFile->GetRemaining();

	if(bytesAvail < bytesToRead)
	{
		bytesRead = bytesAvail;
		bytesToRead = bytesAvail;
	}

	if(bytesAvail == 0)
	{
		return AP4_ERROR_EOS;
	}

	if(FAILED(m_pFile->ByteRead((BYTE*)buffer, bytesToRead)))
	{
		bytesRead = 0;
		return AP4_ERROR_READ_FAILED;
	}

	bytesRead = bytesToRead;

	return AP4_SUCCESS;
}

AP4_Result AP4_AsyncReaderStream::WritePartial(const void* buffer, AP4_Size bytesToWrite, AP4_Size& bytesWritten)
{
    return AP4_ERROR_WRITE_FAILED;
}

AP4_Result AP4_AsyncReaderStream::Seek(AP4_Position offset)
{
	m_pFile->Seek(offset);
	return m_pFile->GetPos() == offset ? AP4_SUCCESS : AP4_FAILURE;
}

AP4_Result AP4_AsyncReaderStream::Tell(AP4_Position& offset)
{
	offset = (AP4_Offset)m_pFile->GetPos();
	return AP4_SUCCESS;
}

AP4_Result AP4_AsyncReaderStream::GetSize(AP4_LargeSize& size)
{
	size = m_pFile->GetLength();
	return AP4_SUCCESS;
}