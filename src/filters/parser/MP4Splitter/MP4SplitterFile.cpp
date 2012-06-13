#include "stdafx.h"
#include "MP4SplitterFile.h"
#include "Ap4AsyncReaderStream.h"

CMP4SplitterFile::CMP4SplitterFile(IAsyncReader* pReader, HRESULT& hr)
    : CBaseSplitterFileEx(pReader, hr, DEFAULT_CACHE_LENGTH, false)
    , m_pAp4File(NULL)
{
    if (FAILED(hr)) {
        return;
    }

    hr = Init();
}

CMP4SplitterFile::~CMP4SplitterFile()
{
    delete(AP4_File*)m_pAp4File;
}

void* /* AP4_Movie* */ CMP4SplitterFile::GetMovie()
{
    ASSERT(m_pAp4File);
    return m_pAp4File ? ((AP4_File*)m_pAp4File)->GetMovie() : NULL;
}

HRESULT CMP4SplitterFile::Init()
{
    Seek(0);

    delete(AP4_File*)m_pAp4File;

    AP4_ByteStream* stream = DNew AP4_AsyncReaderStream(this);

    m_pAp4File = DNew AP4_File(*stream);

    AP4_Movie* movie = ((AP4_File*)m_pAp4File)->GetMovie();

    stream->Release();

    return movie ? S_OK : E_FAIL;
}
