/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015, 2017 see Authors.txt
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
#include <algorithm>
#include <vector>
#ifdef STANDALONE_FILTER
#include <InitGuid.h>
#endif
#include "CDDAReader.h"
#include "../../../DSUtil/DSUtil.h"

#define RAW_SECTOR_SIZE 2352
#define MSF2UINT(hgs)   ((hgs[1] * 4500) + (hgs[2] * 75) + (hgs[3]))

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_WAVE},
};

const AMOVIESETUP_PIN sudOpPin[] = {
    {const_cast<LPWSTR>(L"Output"), FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesOut), sudPinTypesOut},
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CCDDAReader), CCDDAReaderName, MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CCDDAReader>, nullptr, &sudFilter[0]}
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    SetRegKeyValue(
        _T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{54A35221-2C8D-4a31-A5DF-6D809847E393}"),
        _T("0"), _T("0,4,,52494646,8,4,,43444441")); // "RIFFxxxxCDDA"

    SetRegKeyValue(
        _T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{54A35221-2C8D-4a31-A5DF-6D809847E393}"),
        _T("Source Filter"), _T("{54A35221-2C8D-4a31-A5DF-6D809847E393}"));

    SetRegKeyValue(
        _T("Media Type\\Extensions"), _T(".cda"),
        _T("Source Filter"), _T("{54A35221-2C8D-4a31-A5DF-6D809847E393}"));

    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    DeleteRegKey(_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{54A35221-2C8D-4a31-A5DF-6D809847E393}"));
    DeleteRegKey(_T("Media Type\\Extensions"), _T(".cda"));

    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CCDDAReader
//

CCDDAReader::CCDDAReader(IUnknown* pUnk, HRESULT* phr)
    : CAsyncReader(NAME("CCDDAReader"), pUnk, &m_stream, phr, __uuidof(this))
{
    if (phr) {
        *phr = S_OK;
    }
}

CCDDAReader::~CCDDAReader()
{
}

STDMETHODIMP CCDDAReader::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IFileSourceFilter)
        QI2(IAMMediaContent)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CCDDAReader::QueryFilterInfo(FILTER_INFO* pInfo)
{
    CheckPointer(pInfo, E_POINTER);
    ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));

    wcscpy_s(pInfo->achName, CCDDAReaderName);
    pInfo->pGraph = m_pGraph;
    if (m_pGraph) {
        m_pGraph->AddRef();
    }

    return S_OK;
}

// IFileSourceFilter

STDMETHODIMP CCDDAReader::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
    if (!m_stream.Load(pszFileName)) {
        return E_FAIL;
    }

    m_fn = pszFileName;

    CMediaType mt;
    mt.majortype = MEDIATYPE_Stream;
    mt.subtype = m_stream.IsDTS() ? MEDIASUBTYPE_DTS : MEDIASUBTYPE_WAVE;
    m_mt = mt;

    return S_OK;
}

STDMETHODIMP CCDDAReader::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
    CheckPointer(ppszFileName, E_POINTER);

    *ppszFileName = (LPOLESTR)CoTaskMemAlloc((m_fn.GetLength() + 1) * sizeof(WCHAR));
    if (!(*ppszFileName)) {
        return E_OUTOFMEMORY;
    }

    wcscpy_s(*ppszFileName, m_fn.GetLength() + 1, m_fn);

    return S_OK;
}

// IAMMediaContent

STDMETHODIMP CCDDAReader::GetTypeInfoCount(UINT* pctinfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::get_AuthorName(BSTR* pbstrAuthorName)
{
    CheckPointer(pbstrAuthorName, E_POINTER);
    CString str = m_stream.m_trackArtist;
    if (str.IsEmpty()) {
        str = m_stream.m_discArtist;
    }
    *pbstrAuthorName = str.AllocSysString();
    return S_OK;
}

STDMETHODIMP CCDDAReader::get_Title(BSTR* pbstrTitle)
{
    CheckPointer(pbstrTitle, E_POINTER);
    CString str = m_stream.m_trackTitle;
    if (str.IsEmpty()) {
        str = m_stream.m_discTitle;
    }
    *pbstrTitle = str.AllocSysString();
    return S_OK;
}

STDMETHODIMP CCDDAReader::get_Rating(BSTR* pbstrRating)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::get_Description(BSTR* pbstrDescription)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::get_Copyright(BSTR* pbstrCopyright)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::get_BaseURL(BSTR* pbstrBaseURL)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::get_LogoURL(BSTR* pbstrLogoURL)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::get_LogoIconURL(BSTR* pbstrLogoURL)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::get_WatermarkURL(BSTR* pbstrWatermarkURL)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::get_MoreInfoURL(BSTR* pbstrMoreInfoURL)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::get_MoreInfoBannerImage(BSTR* pbstrMoreInfoBannerImage)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::get_MoreInfoBannerURL(BSTR* pbstrMoreInfoBannerURL)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCDDAReader::get_MoreInfoText(BSTR* pbstrMoreInfoText)
{
    return E_NOTIMPL;
}

// CCDDAStream

CCDDAStream::CCDDAStream()
{
    m_hDrive = INVALID_HANDLE_VALUE;

    m_llPosition = m_llLength = 0;

    ZeroMemory(&m_TOC, sizeof(m_TOC));
    m_nFirstSector = m_nStartSector = m_nStopSector = 0;

    ZeroMemory(&m_header, sizeof(m_header));
    m_header.riff.hdr.chunkID = RIFFID;
    m_header.riff.WAVE = WAVEID;
    m_header.frm.hdr.chunkID = FormatID;
    m_header.frm.hdr.chunkSize = sizeof(m_header.frm.pcm);
    m_header.frm.pcm.wf.wFormatTag = WAVE_FORMAT_PCM;
    m_header.frm.pcm.wf.nSamplesPerSec = 44100;
    m_header.frm.pcm.wf.nChannels = 2;
    m_header.frm.pcm.wBitsPerSample = 16;
    m_header.frm.pcm.wf.nBlockAlign = m_header.frm.pcm.wf.nChannels * m_header.frm.pcm.wBitsPerSample / 8;
    m_header.frm.pcm.wf.nAvgBytesPerSec = m_header.frm.pcm.wf.nSamplesPerSec * m_header.frm.pcm.wf.nBlockAlign;
    m_header.data.hdr.chunkID = DataID;

    m_bDTS = false;
}

CCDDAStream::~CCDDAStream()
{
    if (m_hDrive != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hDrive);
        m_hDrive = INVALID_HANDLE_VALUE;
    }
}

bool CCDDAStream::Load(const WCHAR* fnw)
{
    CString path(fnw);

    int iDriveLetter = path.Find(_T(":\\")) - 1;
    int iTrackIndex = CString(path).MakeLower().Find(_T(".cda")) - 1;
    if (iDriveLetter < 0 || iTrackIndex <= iDriveLetter) {
        return false;
    }

    CString drive = CString(_T("\\\\.\\")) + path[iDriveLetter] + _T(":");
    while (iTrackIndex > 0 && _istdigit(path[iTrackIndex - 1])) {
        iTrackIndex--;
    }
    if (1 != _stscanf_s(path.Mid(iTrackIndex), _T("%d"), &iTrackIndex)) {
        return false;
    }

    if (m_hDrive != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hDrive);
        m_hDrive = INVALID_HANDLE_VALUE;
    }

    m_hDrive = CreateFile(drive, GENERIC_READ, FILE_SHARE_READ, nullptr,
                          OPEN_EXISTING, FILE_ATTRIBUTE_READONLY | FILE_FLAG_SEQUENTIAL_SCAN, (HANDLE)nullptr);
    if (m_hDrive == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD BytesReturned;
    if (!DeviceIoControl(m_hDrive, IOCTL_CDROM_READ_TOC, nullptr, 0, &m_TOC, sizeof(m_TOC), &BytesReturned, 0)
            || !(m_TOC.FirstTrack <= iTrackIndex && iTrackIndex <= m_TOC.LastTrack)) {
        CloseHandle(m_hDrive);
        m_hDrive = INVALID_HANDLE_VALUE;
        return false;
    }

    // MMC-3 Draft Revision 10g: Table 222 - Q Sub-channel control field
    m_TOC.TrackData[iTrackIndex - 1].Control &= 5;
    if (!(m_TOC.TrackData[iTrackIndex - 1].Control == 0 || m_TOC.TrackData[iTrackIndex - 1].Control == 1)) {
        CloseHandle(m_hDrive);
        m_hDrive = INVALID_HANDLE_VALUE;
        return false;
    }

    if (m_TOC.TrackData[iTrackIndex - 1].Control & 8) {
        m_header.frm.pcm.wf.nChannels = 4;
    }

    m_nStartSector = MSF2UINT(m_TOC.TrackData[iTrackIndex - 1].Address) - 150;
    m_nStopSector = MSF2UINT(m_TOC.TrackData[iTrackIndex].Address) - 150;

    m_llLength = LONGLONG(m_nStopSector - m_nStartSector) * RAW_SECTOR_SIZE;

    m_header.riff.hdr.chunkSize = (long)(m_llLength + sizeof(m_header) - 8);
    m_header.data.hdr.chunkSize = (long)(m_llLength);

    // Detect DTS Music Disk
    m_bDTS = false;

    // DCA syncwords
    const DWORD DCA_MARKER_RAW_BE = 0x7FFE8001;
    const DWORD DCA_MARKER_RAW_LE = 0xFE7F0180;
    const DWORD DCA_MARKER_14B_BE = 0x1FFFE800;
    const DWORD DCA_MARKER_14B_LE = 0xFF1F00E8;
    UINT nMarkerFound = 0, nAttempt = 0;
    DWORD marker = DWORD_MAX;

    std::vector<BYTE> data(16384);
    DWORD dwSizeRead = 0;
    while (SUCCEEDED(Read(data.data(), (DWORD)data.size(), TRUE, &dwSizeRead)) && dwSizeRead && nAttempt < (4 + nMarkerFound)) {
        nAttempt++;

        for (DWORD i = 0; i < dwSizeRead; i++) {
            marker = (marker << 8) | data[i];
            if ((marker == DCA_MARKER_14B_LE && (i < dwSizeRead - 2) && (data[i + 1] & 0xF0) == 0xF0 && data[i + 2] == 0x07)
                    || (marker == DCA_MARKER_14B_BE && (i < dwSizeRead - 2) && data[i + 1] == 0x07 && (data[i + 2] & 0xF0) == 0xF0)
                    || marker == DCA_MARKER_RAW_LE || marker == DCA_MARKER_RAW_BE) {
                nMarkerFound++;
            }
        }
        dwSizeRead = 0;

        if (nMarkerFound >= 4) {
            m_bDTS = true;
            break;
        }
    }
    SetPointer(0);

    CDROM_READ_TOC_EX TOCEx;
    ZeroMemory(&TOCEx, sizeof(TOCEx));
    TOCEx.Format = CDROM_READ_TOC_EX_FORMAT_CDTEXT;
    BYTE header[4] = { 0 };
    static_assert(sizeof(header) >= MINIMUM_CDROM_READ_TOC_EX_SIZE, "sizeof(header) must be greater or equal to MINIMUM_CDROM_READ_TOC_EX_SIZE");
    if (!DeviceIoControl(m_hDrive, IOCTL_CDROM_READ_TOC_EX, &TOCEx, sizeof(TOCEx), header, sizeof(header), &BytesReturned, 0)) {
        return true;
    }

    DWORD size = 2 + (WORD(header[0]) << 8) + header[1];
    if (size <= 4) { // No cd-text information
        return true;
    }

    CAutoVectorPtr<BYTE> pCDTextData;
    if (!pCDTextData.Allocate(size)) {
        return true;
    }
    ZeroMemory(pCDTextData, size);

    if (!DeviceIoControl(m_hDrive, IOCTL_CDROM_READ_TOC_EX, &TOCEx, sizeof(TOCEx), pCDTextData, size, &BytesReturned, 0)) {
        return true;
    }

    size = (WORD)(BytesReturned - sizeof(CDROM_TOC_CD_TEXT_DATA));
    CDROM_TOC_CD_TEXT_DATA_BLOCK* pDesc = ((CDROM_TOC_CD_TEXT_DATA*)(BYTE*)pCDTextData)->Descriptors;

    CStringArray str[16];
    for (int i = 0; i < _countof(str); i++) {
        str[i].SetSize(1 + m_TOC.LastTrack);
    }
    CString last;

    for (int i = 0; size >= sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK); i++, size -= sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK), pDesc++) {
        if (pDesc->TrackNumber > m_TOC.LastTrack) {
            continue;
        }

        const int lenU = _countof(pDesc->Text);
        const int lenW = _countof(pDesc->WText);

        CString text = !pDesc->Unicode
                       ? CString(reinterpret_cast<CStringA::PCXSTR>(pDesc->Text), lenU)
                       : CString(pDesc->WText, lenW);

        if (pDesc->PackType < 0x80 || pDesc->PackType >= 0x80 + 0x10) {
            continue;
        }
        pDesc->PackType -= 0x80;

        if (pDesc->CharacterPosition == 0) {
            str[pDesc->PackType][pDesc->TrackNumber] = text;
        } else { // pDesc->CharacterPosition <= 0xf since CharacterPosition is a 4-bit field
            if (pDesc->CharacterPosition < 0xf && !last.IsEmpty()) {
                str[pDesc->PackType][pDesc->TrackNumber] = last + text;
            } else {
                str[pDesc->PackType][pDesc->TrackNumber] += text;
            }
        }

        const int tlen = text.GetLength();
        if (tlen < 12 - 1) {
            last = !pDesc->Unicode
                   ? CString(reinterpret_cast<CStringA::PCXSTR>(pDesc->Text) + tlen + 1, lenU - (tlen + 1))
                   : CString(static_cast<CStringW::PCXSTR>(pDesc->WText) + tlen + 1, lenW - (tlen + 1));
        } else {
            last.Empty();
        }
    }

    m_discTitle = str[0][0];
    m_trackTitle = str[0][iTrackIndex];
    m_discArtist = str[1][0];
    m_trackArtist = str[1][iTrackIndex];

    return true;
}

HRESULT CCDDAStream::SetPointer(LONGLONG llPos)
{
    if (llPos < 0 || llPos > m_llLength) {
        return S_FALSE;
    }
    m_llPosition = llPos;
    return S_OK;
}

HRESULT CCDDAStream::Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead)
{
    CAutoLock lck(&m_csLock);

    PBYTE pbBufferOrg = pbBuffer;
    LONGLONG pos = m_llPosition;
    size_t len = dwBytesToRead;

    if (!m_bDTS) {
        if (pos < sizeof(m_header) && len > 0) {
            size_t l = std::min(len, size_t(sizeof(m_header) - pos));
            memcpy(pbBuffer, &((BYTE*)&m_header)[pos], l);
            pbBuffer += l;
            pos += l;
            len -= l;
        }

        pos -= sizeof(m_header);
    }

    while (pos >= 0 && pos < m_llLength && len > 0) {
        RAW_READ_INFO rawreadinfo;
        rawreadinfo.TrackMode = CDDA;

        UINT sector = m_nStartSector + UINT(pos / RAW_SECTOR_SIZE);
        UINT offset = pos % RAW_SECTOR_SIZE;

        // Reading 20 sectors at once seems to be a good trade-off between performance and compatibility
        rawreadinfo.SectorCount = std::min(20u, m_nStopSector - sector);

        if (m_buff.size() < rawreadinfo.SectorCount * RAW_SECTOR_SIZE) {
            m_buff.resize(rawreadinfo.SectorCount * RAW_SECTOR_SIZE);
        }

        rawreadinfo.DiskOffset.QuadPart = sector * 2048;
        DWORD dwBytesReturned = 0;
        VERIFY(DeviceIoControl(m_hDrive, IOCTL_CDROM_RAW_READ,
                               &rawreadinfo, sizeof(rawreadinfo),
                               m_buff.data(), (DWORD)m_buff.size(),
                               &dwBytesReturned, 0));

        size_t l = std::min(std::min(len, m_buff.size() - offset), size_t(m_llLength - pos));
        memcpy(pbBuffer, &m_buff[offset], l);

        pbBuffer += l;
        pos += l;
        len -= l;
    }

    if (pdwBytesRead) {
        *pdwBytesRead = DWORD(pbBuffer - pbBufferOrg);
    }
    m_llPosition += pbBuffer - pbBufferOrg;

    return S_OK;
}

LONGLONG CCDDAStream::Size(LONGLONG* pSizeAvailable)
{
    LONGLONG size = m_llLength;
    if (!m_bDTS) {
        size += sizeof(m_header);
    }
    if (pSizeAvailable) {
        *pSizeAvailable = size;
    }
    return size;
}

DWORD CCDDAStream::Alignment()
{
    return 1;
}

void CCDDAStream::Lock()
{
    m_csLock.Lock();
}

void CCDDAStream::Unlock()
{
    m_csLock.Unlock();
}
