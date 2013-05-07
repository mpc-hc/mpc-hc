/*
 * (C) 2009-2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <MMReg.h>
#include <ks.h>
#ifdef STANDALONE_FILTER
#include <InitGuid.h>
#endif
#include <uuids.h>
#include "moreuuids.h"
#include "FLACSource.h"
#include "../../../DSUtil/DSUtil.h"
#include "libflac/src/libflac/include/protected/stream_decoder.h"

#define _DECODER_   (FLAC__StreamDecoder*)m_pDecoder

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_Audio, &MEDIASUBTYPE_FLAC_FRAMED}
};

const AMOVIESETUP_PIN sudOpPin[] = {
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CFLACSource), FlacSourceName, MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CFLACSource>, nullptr, &sudFilter[0]}
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    SetRegKeyValue(
        _T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"),
        _T("0"), _T("0,4,,664C6143"));

    SetRegKeyValue(
        _T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"),
        _T("Source Filter"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"));

    SetRegKeyValue(
        _T("Media Type\\Extensions"), _T(".flac"),
        _T("Source Filter"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"));

    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    DeleteRegKey(_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"));
    DeleteRegKey(_T("Media Type\\Extensions"), _T(".flac"));

    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif


// Declaration for FLAC callbacks
static FLAC__StreamDecoderReadStatus    StreamDecoderRead(const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes, void* client_data);
static FLAC__StreamDecoderSeekStatus    StreamDecoderSeek(const FLAC__StreamDecoder* decoder, FLAC__uint64 absolute_byte_offset, void* client_data);
static FLAC__StreamDecoderTellStatus    StreamDecoderTell(const FLAC__StreamDecoder* decoder, FLAC__uint64* absolute_byte_offset, void* client_data);
static FLAC__StreamDecoderLengthStatus  StreamDecoderLength(const FLAC__StreamDecoder* decoder, FLAC__uint64* stream_length, void* client_data);
static FLAC__bool                       StreamDecoderEof(const FLAC__StreamDecoder* decoder, void* client_data);
static FLAC__StreamDecoderWriteStatus   StreamDecoderWrite(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* client_data);
static void                             StreamDecoderError(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data);
static void                             StreamDecoderMetadata(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data);


//
// CFLACSource
//

CFLACSource::CFLACSource(LPUNKNOWN lpunk, HRESULT* phr)
    : CBaseSource<CFLACStream>(NAME("CFLACSource"), lpunk, phr, __uuidof(this))
{
}

CFLACSource::~CFLACSource()
{
}

STDMETHODIMP CFLACSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IDSMChapterBag)
        QI2(IAMMediaContent)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CFLACSource::QueryFilterInfo(FILTER_INFO* pInfo)
{
    CheckPointer(pInfo, E_POINTER);
    ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));
    wcscpy_s(pInfo->achName, FlacSourceName);
    pInfo->pGraph = m_pGraph;
    if (m_pGraph) {
        m_pGraph->AddRef();
    }

    return S_OK;
}

STDMETHODIMP CFLACSource::get_AuthorName(BSTR* pbstrAuthorName)
{
    CheckPointer(pbstrAuthorName, E_POINTER);
    HRESULT hr = VFW_E_NOT_FOUND;

    CString author;
    if (GetFLACStream()->GetComment(_T("artist"), author)) {
        *pbstrAuthorName = author.AllocSysString();
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CFLACSource::get_Title(BSTR* pbstrTitle)
{
    CheckPointer(pbstrTitle, E_POINTER);
    HRESULT hr = VFW_E_NOT_FOUND;

    CString title;
    if (GetFLACStream()->GetComment(_T("title"), title)) {
        *pbstrTitle = title.AllocSysString();
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CFLACSource::get_Description(BSTR* pbstrDescription)
{
    CheckPointer(pbstrDescription, E_POINTER);
    HRESULT hr = VFW_E_NOT_FOUND;

    CString desc;
    if (GetFLACStream()->GetComment(_T("description"), desc)) {
        *pbstrDescription = desc.AllocSysString();
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CFLACSource::get_Copyright(BSTR* pbstrCopyright)
{
    CheckPointer(pbstrCopyright, E_POINTER);
    HRESULT hr = VFW_E_NOT_FOUND;

    CString copyright;
    if (GetFLACStream()->GetComment(_T("copyright"), copyright)
            || GetFLACStream()->GetComment(_T("date"), copyright)) {
        *pbstrCopyright = copyright.AllocSysString();
        hr = S_OK;
    }

    return hr;
}

// CFLACStream

CFLACStream::CFLACStream(const WCHAR* wfn, CSource* pParent, HRESULT* phr)
    : CBaseStream(NAME("CFLACStream"), pParent, phr)
    , m_bIsEOF(false)
    , m_pDecoder(nullptr)
{
    CAutoLock cAutoLock(&m_cSharedState);
    CString fn(wfn);
    CFileException ex;
    HRESULT hr = E_FAIL;

    do {
        if (!m_file.Open(fn, CFile::modeRead | CFile::shareDenyNone, &ex)) {
            hr = AmHresultFromWin32(ex.m_lOsError);
            break;
        }

        m_pDecoder = FLAC__stream_decoder_new();
        if (!m_pDecoder) {
            break;
        }

        // We want to get the embedded CUE sheet and the Vorbis tags if available
        FLAC__stream_decoder_set_metadata_respond(_DECODER_, FLAC__METADATA_TYPE_CUESHEET);
        FLAC__stream_decoder_set_metadata_respond(_DECODER_, FLAC__METADATA_TYPE_VORBIS_COMMENT);

        if (FLAC__STREAM_DECODER_INIT_STATUS_OK != FLAC__stream_decoder_init_stream(_DECODER_,
                StreamDecoderRead,
                StreamDecoderSeek,
                StreamDecoderTell,
                StreamDecoderLength,
                StreamDecoderEof,
                StreamDecoderWrite,
                StreamDecoderMetadata,
                StreamDecoderError,
                this)) {
            break;
        }


        if (!FLAC__stream_decoder_process_until_end_of_metadata(_DECODER_) ||
                !FLAC__stream_decoder_seek_absolute(_DECODER_, 0)) {
            break;
        }

        FLAC__stream_decoder_get_decode_position(_DECODER_, &m_llOffset);

        hr = S_OK;
    } while (false);

    if (phr) {
        *phr = hr;
    }
}

CFLACStream::~CFLACStream()
{
    if (m_pDecoder) {
        FLAC__stream_decoder_delete(_DECODER_);
        m_pDecoder = nullptr;
    }
}

HRESULT CFLACStream::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
    ASSERT(pAlloc);
    ASSERT(pProperties);

    HRESULT hr = NOERROR;

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = m_nMaxFrameSize;

    ALLOCATOR_PROPERTIES Actual;
    if (FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))) {
        return hr;
    }

    if (Actual.cbBuffer < pProperties->cbBuffer) {
        return E_FAIL;
    }
    ASSERT(Actual.cBuffers == pProperties->cBuffers);

    return NOERROR;
}

HRESULT CFLACStream::FillBuffer(IMediaSample* pSample, int nFrame, BYTE* pOut, long& len)
{
    FLAC__uint64 llCurPos;
    FLAC__uint64 llNextPos;

    if (m_bDiscontinuity) {
        FLAC__stream_decoder_seek_absolute(_DECODER_, (m_rtPosition * m_i64TotalNumSamples) / m_rtDuration);
    }

    FLAC__stream_decoder_get_decode_position(_DECODER_, &llCurPos);

    FLAC__stream_decoder_skip_single_frame(_DECODER_);
    if (m_bIsEOF) {
        return S_FALSE;
    }
    FLAC__stream_decoder_get_decode_position(_DECODER_, &llNextPos);

    FLAC__uint64 llCurFile = m_file.GetPosition();
    len = (long)(llNextPos - llCurPos);
    ASSERT(len > 0);
    if (len <= 0) {
        return S_FALSE;
    } else if (len > m_nMaxFrameSize) { // Probably a corrupted file but try to play it anyway
        len = m_nMaxFrameSize;
    }

    m_file.Seek(llCurPos, CFile::begin);
    m_file.Read(pOut, len);
    m_file.Seek(llCurFile, CFile::begin);

    if ((_DECODER_)->protected_->blocksize > 0 && (_DECODER_)->protected_->sample_rate > 0) {
        m_AvgTimePerFrame = (_DECODER_)->protected_->blocksize * UNITS / (_DECODER_)->protected_->sample_rate;
    } else {
        m_AvgTimePerFrame = m_rtDuration * len / (m_llFileSize - m_llOffset);
    }

    return S_OK;
}

HRESULT CFLACStream::GetMediaType(int iPosition, CMediaType* pmt)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    if (iPosition == 0) {
        pmt->majortype          = MEDIATYPE_Audio;
        pmt->subtype            = MEDIASUBTYPE_FLAC_FRAMED;
        pmt->formattype         = FORMAT_WaveFormatEx;
        WAVEFORMATEX* wfe       = (WAVEFORMATEX*)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
        memset(wfe, 0, sizeof(WAVEFORMATEX));
        wfe->wFormatTag         = WAVE_FORMAT_FLAC;
        wfe->nChannels          = m_nChannels;
        wfe->nSamplesPerSec     = m_nSamplesPerSec;
        wfe->nAvgBytesPerSec    = m_nAvgBytesPerSec;
        wfe->nBlockAlign        = 1;
        wfe->wBitsPerSample     = m_wBitsPerSample;
        wfe->cbSize             = 0;
    } else {
        return VFW_S_NO_MORE_ITEMS;
    }

    pmt->SetTemporalCompression(FALSE);

    return S_OK;
}

HRESULT CFLACStream::CheckMediaType(const CMediaType* pmt)
{
    if (pmt->majortype  == MEDIATYPE_Audio
            && pmt->subtype == MEDIASUBTYPE_FLAC_FRAMED
            && pmt->formattype == FORMAT_WaveFormatEx
            && ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_FLAC) {
        return S_OK;
    } else {
        return E_INVALIDARG;
    }
}

void CFLACStream::UpdateFromMetadata(void* pBuffer)
{
    const FLAC__StreamMetadata* pMetadata = (const FLAC__StreamMetadata*) pBuffer;

    if (pMetadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        m_nMaxFrameSize         = pMetadata->data.stream_info.max_framesize;
        m_nSamplesPerSec        = pMetadata->data.stream_info.sample_rate;
        m_nChannels             = pMetadata->data.stream_info.channels;
        m_wBitsPerSample        = pMetadata->data.stream_info.bits_per_sample;
        m_i64TotalNumSamples    = pMetadata->data.stream_info.total_samples;
        m_nAvgBytesPerSec       = (m_nChannels * (m_wBitsPerSample >> 3)) * m_nSamplesPerSec;

        if (!m_nMaxFrameSize) { // Estimate a maximum frame size
            m_nMaxFrameSize = (2 * m_wBitsPerSample * m_nChannels * pMetadata->data.stream_info.max_blocksize + 7) / 8;
        }

        // === Init members from base classes
        GetFileSizeEx(m_file.m_hFile, (LARGE_INTEGER*)&m_llFileSize);
        m_rtDuration            = (m_i64TotalNumSamples * UNITS) / m_nSamplesPerSec;
        m_rtStop                = m_rtDuration;
        m_AvgTimePerFrame       = (m_nMaxFrameSize + pMetadata->data.stream_info.min_framesize) * m_rtDuration / 2 / m_llFileSize;
    } else if (pMetadata->type == FLAC__METADATA_TYPE_CUESHEET) {
        CString s;
        REFERENCE_TIME rt;
        for (unsigned int i = 0; i < pMetadata->data.cue_sheet.num_tracks; ++i) {
            FLAC__StreamMetadata_CueSheet_Track& track = pMetadata->data.cue_sheet.tracks[i];
            // Ignore non-audio tracks and lead-out track
            if (track.type != 0
                    || (pMetadata->data.cue_sheet.is_cd && track.number == 170)
                    || (!pMetadata->data.cue_sheet.is_cd && track.number == 255)) {
                continue;
            }

            rt = MILLISECONDS_TO_100NS_UNITS(1000 * track.offset / m_nSamplesPerSec);
            s.Format(_T("Track %02d"), i + 1);
            ((CFLACSource*)m_pFilter)->ChapAppend(rt, s);

            if (track.num_indices > 1) {
                for (int j = 0; j < track.num_indices; ++j) {
                    FLAC__StreamMetadata_CueSheet_Index& index = track.indices[j];
                    s.Format(_T("+ INDEX %02d"), index.number);
                    REFERENCE_TIME r = rt + MILLISECONDS_TO_100NS_UNITS(1000 * index.offset / m_nSamplesPerSec);
                    ((CFLACSource*)m_pFilter)->ChapAppend(r, s);
                }
            }
        }
    } else if (pMetadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
        for (unsigned int i = 0; i < pMetadata->data.vorbis_comment.num_comments; i++) {
            FLAC__StreamMetadata_VorbisComment_Entry& entry = pMetadata->data.vorbis_comment.comments[i];
            CStringA comment((char*)entry.entry, (int)entry.length);
            int nSepPos = comment.Find("=");

            if (nSepPos > 0) {
                CString tag = UTF8To16(comment.Left(nSepPos)).MakeLower();
                CString content = UTF8To16(comment.Mid(nSepPos + 1));

                CString oldContent;
                if (m_vorbisComments.Lookup(tag, oldContent)) {
                    m_vorbisComments[tag].Append(_T(", "));
                    m_vorbisComments[tag].Append(content);
                } else {
                    m_vorbisComments[tag] = content;
                }
            }
        }
    }
}

bool CFLACStream::GetComment(const CString& tag, CString& content) const
{
    return !!m_vorbisComments.Lookup(tag, content);
}

FLAC__StreamDecoderReadStatus StreamDecoderRead(const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes, void* client_data)
{
    CFLACStream* pThis = static_cast<CFLACStream*>(client_data);
    UINT nRead = pThis->GetFile()->Read(buffer, (UINT) * bytes);

    pThis->m_bIsEOF = (nRead != *bytes);
    *bytes = nRead;

    return (*bytes == 0) ? FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM : FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderSeekStatus StreamDecoderSeek(const FLAC__StreamDecoder* decoder, FLAC__uint64 absolute_byte_offset, void* client_data)
{
    CFLACStream* pThis = static_cast<CFLACStream*>(client_data);

    pThis->m_bIsEOF = false;
    pThis->GetFile()->Seek(absolute_byte_offset, CFile::begin);
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

FLAC__StreamDecoderTellStatus StreamDecoderTell(const FLAC__StreamDecoder* decoder, FLAC__uint64* absolute_byte_offset, void* client_data)
{
    CFLACStream* pThis = static_cast<CFLACStream*>(client_data);
    *absolute_byte_offset = pThis->GetFile()->GetPosition();
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus StreamDecoderLength(const FLAC__StreamDecoder* decoder, FLAC__uint64* stream_length, void* client_data)
{
    CFLACStream* pThis = static_cast<CFLACStream*>(client_data);
    CFile* pFile = pThis->GetFile();

    if (pFile == nullptr) {
        return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
    } else {
        *stream_length = pFile->GetLength();
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    }
}

FLAC__bool StreamDecoderEof(const FLAC__StreamDecoder* decoder, void* client_data)
{
    CFLACStream* pThis = static_cast<CFLACStream*>(client_data);

    return pThis->m_bIsEOF;
}

FLAC__StreamDecoderWriteStatus StreamDecoderWrite(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* client_data)
{
    CFLACStream* pThis = static_cast<CFLACStream*>(client_data);
    UNREFERENCED_PARAMETER(pThis);

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void StreamDecoderError(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data)
{
}

void StreamDecoderMetadata(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data)
{
    CFLACStream* pThis = static_cast<CFLACStream*>(client_data);

    if (pThis) {
        pThis->UpdateFromMetadata((void*)metadata);
    }
}
