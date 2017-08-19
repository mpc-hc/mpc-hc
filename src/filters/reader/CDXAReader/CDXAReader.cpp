/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2017 see Authors.txt
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
#include "CDXAReader.h"
#include "../../../DSUtil/DSUtil.h"
#ifdef STANDALONE_FILTER
#include <InitGuid.h>
#endif
#include <uuids.h>
#include "moreuuids.h"

/////////

static constexpr DWORD EDC_crctable[256] = {
    0x00000000l, 0x90910101l, 0x91210201l, 0x01b00300l,
    0x92410401l, 0x02d00500l, 0x03600600l, 0x93f10701l,
    0x94810801l, 0x04100900l, 0x05a00a00l, 0x95310b01l,
    0x06c00c00l, 0x96510d01l, 0x97e10e01l, 0x07700f00l,
    0x99011001l, 0x09901100l, 0x08201200l, 0x98b11301l,
    0x0b401400l, 0x9bd11501l, 0x9a611601l, 0x0af01700l,
    0x0d801800l, 0x9d111901l, 0x9ca11a01l, 0x0c301b00l,
    0x9fc11c01l, 0x0f501d00l, 0x0ee01e00l, 0x9e711f01l,
    0x82012001l, 0x12902100l, 0x13202200l, 0x83b12301l,
    0x10402400l, 0x80d12501l, 0x81612601l, 0x11f02700l,
    0x16802800l, 0x86112901l, 0x87a12a01l, 0x17302b00l,
    0x84c12c01l, 0x14502d00l, 0x15e02e00l, 0x85712f01l,
    0x1b003000l, 0x8b913101l, 0x8a213201l, 0x1ab03300l,
    0x89413401l, 0x19d03500l, 0x18603600l, 0x88f13701l,
    0x8f813801l, 0x1f103900l, 0x1ea03a00l, 0x8e313b01l,
    0x1dc03c00l, 0x8d513d01l, 0x8ce13e01l, 0x1c703f00l,
    0xb4014001l, 0x24904100l, 0x25204200l, 0xb5b14301l,
    0x26404400l, 0xb6d14501l, 0xb7614601l, 0x27f04700l,
    0x20804800l, 0xb0114901l, 0xb1a14a01l, 0x21304b00l,
    0xb2c14c01l, 0x22504d00l, 0x23e04e00l, 0xb3714f01l,
    0x2d005000l, 0xbd915101l, 0xbc215201l, 0x2cb05300l,
    0xbf415401l, 0x2fd05500l, 0x2e605600l, 0xbef15701l,
    0xb9815801l, 0x29105900l, 0x28a05a00l, 0xb8315b01l,
    0x2bc05c00l, 0xbb515d01l, 0xbae15e01l, 0x2a705f00l,
    0x36006000l, 0xa6916101l, 0xa7216201l, 0x37b06300l,
    0xa4416401l, 0x34d06500l, 0x35606600l, 0xa5f16701l,
    0xa2816801l, 0x32106900l, 0x33a06a00l, 0xa3316b01l,
    0x30c06c00l, 0xa0516d01l, 0xa1e16e01l, 0x31706f00l,
    0xaf017001l, 0x3f907100l, 0x3e207200l, 0xaeb17301l,
    0x3d407400l, 0xadd17501l, 0xac617601l, 0x3cf07700l,
    0x3b807800l, 0xab117901l, 0xaaa17a01l, 0x3a307b00l,
    0xa9c17c01l, 0x39507d00l, 0x38e07e00l, 0xa8717f01l,
    0xd8018001l, 0x48908100l, 0x49208200l, 0xd9b18301l,
    0x4a408400l, 0xdad18501l, 0xdb618601l, 0x4bf08700l,
    0x4c808800l, 0xdc118901l, 0xdda18a01l, 0x4d308b00l,
    0xdec18c01l, 0x4e508d00l, 0x4fe08e00l, 0xdf718f01l,
    0x41009000l, 0xd1919101l, 0xd0219201l, 0x40b09300l,
    0xd3419401l, 0x43d09500l, 0x42609600l, 0xd2f19701l,
    0xd5819801l, 0x45109900l, 0x44a09a00l, 0xd4319b01l,
    0x47c09c00l, 0xd7519d01l, 0xd6e19e01l, 0x46709f00l,
    0x5a00a000l, 0xca91a101l, 0xcb21a201l, 0x5bb0a300l,
    0xc841a401l, 0x58d0a500l, 0x5960a600l, 0xc9f1a701l,
    0xce81a801l, 0x5e10a900l, 0x5fa0aa00l, 0xcf31ab01l,
    0x5cc0ac00l, 0xcc51ad01l, 0xcde1ae01l, 0x5d70af00l,
    0xc301b001l, 0x5390b100l, 0x5220b200l, 0xc2b1b301l,
    0x5140b400l, 0xc1d1b501l, 0xc061b601l, 0x50f0b700l,
    0x5780b800l, 0xc711b901l, 0xc6a1ba01l, 0x5630bb00l,
    0xc5c1bc01l, 0x5550bd00l, 0x54e0be00l, 0xc471bf01l,
    0x6c00c000l, 0xfc91c101l, 0xfd21c201l, 0x6db0c300l,
    0xfe41c401l, 0x6ed0c500l, 0x6f60c600l, 0xfff1c701l,
    0xf881c801l, 0x6810c900l, 0x69a0ca00l, 0xf931cb01l,
    0x6ac0cc00l, 0xfa51cd01l, 0xfbe1ce01l, 0x6b70cf00l,
    0xf501d001l, 0x6590d100l, 0x6420d200l, 0xf4b1d301l,
    0x6740d400l, 0xf7d1d501l, 0xf661d601l, 0x66f0d700l,
    0x6180d800l, 0xf111d901l, 0xf0a1da01l, 0x6030db00l,
    0xf3c1dc01l, 0x6350dd00l, 0x62e0de00l, 0xf271df01l,
    0xee01e001l, 0x7e90e100l, 0x7f20e200l, 0xefb1e301l,
    0x7c40e400l, 0xecd1e501l, 0xed61e601l, 0x7df0e700l,
    0x7a80e800l, 0xea11e901l, 0xeba1ea01l, 0x7b30eb00l,
    0xe8c1ec01l, 0x7850ed00l, 0x79e0ee00l, 0xe971ef01l,
    0x7700f000l, 0xe791f101l, 0xe621f201l, 0x76b0f300l,
    0xe541f401l, 0x75d0f500l, 0x7460f600l, 0xe4f1f701l,
    0xe381f801l, 0x7310f900l, 0x72a0fa00l, 0xe231fb01l,
    0x71c0fc00l, 0xe151fd01l, 0xe0e1fe01l, 0x7070ff00l
};

static DWORD build_edc(const void* in, unsigned from, unsigned upto)
{
    const BYTE* p = (BYTE*)in + from;
    DWORD result = 0;

    for (; from < upto; from++) {
        result = EDC_crctable[(result ^ *p++) & 0xffL] ^ (result >> 8);
    }

    return result;
}

/////////

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL}
};

const AMOVIESETUP_PIN sudOpPin[] = {
    {const_cast<LPWSTR>(L"Output"), FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CCDXAReader), CCDXAReaderName, MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CCDXAReader>, nullptr, &sudFilter[0]}
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    SetRegKeyValue(
        _T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{D367878E-F3B8-4235-A968-F378EF1B9A44}"),
        _T("0"), _T("0,4,,52494646,8,4,,43445841")); // "RIFFxxxxCDXA"

    SetRegKeyValue(
        _T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{D367878E-F3B8-4235-A968-F378EF1B9A44}"),
        _T("Source Filter"), _T("{D367878E-F3B8-4235-A968-F378EF1B9A44}"));

    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    DeleteRegKey(_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{D367878E-F3B8-4235-A968-F378EF1B9A44}"));

    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CCDXAReader
//

CCDXAReader::CCDXAReader(IUnknown* pUnk, HRESULT* phr)
    : CAsyncReader(NAME("CCDXAReader"), pUnk, &m_stream, phr, __uuidof(this))
{
    if (phr) {
        *phr = S_OK;
    }
}

CCDXAReader::~CCDXAReader()
{
}

STDMETHODIMP CCDXAReader::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI(IFileSourceFilter)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CCDXAReader::QueryFilterInfo(FILTER_INFO* pInfo)
{
    CheckPointer(pInfo, E_POINTER);
    ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));

    wcscpy_s(pInfo->achName, CCDXAReaderName);
    pInfo->pGraph = m_pGraph;
    if (m_pGraph) {
        m_pGraph->AddRef();
    }

    return S_OK;
}

STDMETHODIMP CCDXAReader::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
    CMediaType mt;
    m_mt = mt;

    if (!m_stream.Load(pszFileName)) {
        return E_FAIL;
    }

    m_fn = pszFileName;

    mt.majortype = MEDIATYPE_Stream;
    mt.subtype = m_stream.m_subtype;
    m_mt = mt;

    return S_OK;
}

STDMETHODIMP CCDXAReader::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
    CheckPointer(ppszFileName, E_POINTER);

    *ppszFileName = (LPOLESTR)CoTaskMemAlloc((m_fn.GetLength() + 1) * sizeof(WCHAR));
    if (!(*ppszFileName)) {
        return E_OUTOFMEMORY;
    }

    wcscpy_s(*ppszFileName, m_fn.GetLength() + 1, m_fn);

    return S_OK;
}

// CCDXAStream

CCDXAStream::CCDXAStream()
{
    m_subtype = MEDIASUBTYPE_NULL;

    m_hFile = INVALID_HANDLE_VALUE;

    m_llPosition = m_llLength = 0;
    m_nFirstSector = 0;
    m_nBufferedSector = -1;
    ZeroMemory(m_sector, sizeof(m_sector));
}

CCDXAStream::~CCDXAStream()
{
    if (m_hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

bool CCDXAStream::Load(const WCHAR* fnw)
{
    if (m_hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }

    m_hFile = CreateFile(CString(fnw), GENERIC_READ, FILE_SHARE_READ, nullptr,
                         OPEN_EXISTING, FILE_ATTRIBUTE_READONLY | FILE_FLAG_SEQUENTIAL_SCAN, (HANDLE)nullptr);
    if (m_hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    BYTE hdr[RIFFCDXA_HEADER_SIZE];
    DWORD NumberOfBytesRead;
    if (!ReadFile(m_hFile, (LPVOID)hdr, RIFFCDXA_HEADER_SIZE, &NumberOfBytesRead, nullptr)
            || *((DWORD*)&hdr[0]) != 'FFIR' || *((DWORD*)&hdr[8]) != 'AXDC'
            || *((DWORD*)&hdr[4]) != (*((DWORD*)&hdr[0x28]) + 0x24)) {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
        return false;
    }

    LARGE_INTEGER size = {0, 0};
    GetFileSizeEx(m_hFile, &size);

    m_llLength = ((size.QuadPart - RIFFCDXA_HEADER_SIZE) / RAW_SECTOR_SIZE) * RAW_DATA_SIZE;

    if (!LookForMediaSubType()) {
        m_llPosition = m_llLength = 0;
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
        return false;
    }

    m_llPosition = 0;

    m_nBufferedSector = -1;

    return true;
}

HRESULT CCDXAStream::SetPointer(LONGLONG llPos)
{
    HRESULT hr = S_FALSE;

    if (llPos >= 0 && llPos < m_llLength) {
        m_llPosition = llPos;
        hr = S_OK;
    }

    return hr;
}

HRESULT CCDXAStream::Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead)
{
    CAutoLock lck(&m_csLock);

    PBYTE pbBufferOrg = pbBuffer;
    LONGLONG pos = m_llPosition;

    while (pos >= 0 && pos < m_llLength && dwBytesToRead > 0) {
        UINT sector = m_nFirstSector + int(pos / RAW_DATA_SIZE);
        __int64 offset = pos % RAW_DATA_SIZE;

        if (m_nBufferedSector != (int)sector) {
            LARGE_INTEGER FilePointer;
            FilePointer.QuadPart = RIFFCDXA_HEADER_SIZE + sector * RAW_SECTOR_SIZE;
            SetFilePointerEx(m_hFile, FilePointer, &FilePointer, FILE_BEGIN);

            ZeroMemory(m_sector, sizeof(m_sector));

            DWORD NumberOfBytesRead = 0;

            int nRetries = 3;
            while (nRetries--) {
                NumberOfBytesRead = 0;
                if (!ReadFile(m_hFile, m_sector, RAW_SECTOR_SIZE, &NumberOfBytesRead, nullptr)
                        || NumberOfBytesRead != RAW_SECTOR_SIZE) {
                    break;
                }

                if (*(DWORD*)&m_sector[RAW_SECTOR_SIZE - 4] == 0) { // no CRC? it happens...
                    break;
                }

                if (build_edc(m_sector, RAW_SYNC_SIZE + RAW_HEADER_SIZE, RAW_SECTOR_SIZE) == 0) {
                    break;
                }

                TRACE(_T("CCDXAStream: CRC error at sector %u (fp=0x%I64x, retriesleft=%d)\n"), sector, FilePointer.QuadPart, nRetries);
            }

            m_nBufferedSector = sector;
        }

        DWORD l = std::min(dwBytesToRead, (DWORD)std::min(RAW_DATA_SIZE - offset, m_llLength - pos));
        memcpy(pbBuffer, &m_sector[RAW_SYNC_SIZE + RAW_HEADER_SIZE + RAW_SUBHEADER_SIZE + offset], l);

        pbBuffer += l;
        pos += l;
        dwBytesToRead -= l;
    }

    if (pdwBytesRead) {
        *pdwBytesRead = DWORD(pbBuffer - pbBufferOrg);
    }
    m_llPosition += pbBuffer - pbBufferOrg;

    if (dwBytesToRead != 0) {
        return S_FALSE;
    }

    return S_OK;
}

LONGLONG CCDXAStream::Size(LONGLONG* pSizeAvailable)
{
    if (pSizeAvailable) {
        *pSizeAvailable = m_llLength;
    }
    return m_llLength;
}

DWORD CCDXAStream::Alignment()
{
    return 1;
}

void CCDXAStream::Lock()
{
    m_csLock.Lock();
}

void CCDXAStream::Unlock()
{
    m_csLock.Unlock();
}

//

bool CCDXAStream::LookForMediaSubType()
{
    BYTE buff[RAW_DATA_SIZE];

    m_subtype = MEDIASUBTYPE_NULL;

    m_llPosition = 0;

    for (int iSectorsRead = 0;
            Read(buff, RAW_DATA_SIZE, 1, nullptr) == S_OK && iSectorsRead < 1000;
            iSectorsRead++) {
        if (*((DWORD*)&buff[0]) == 0xba010000) {
            m_llPosition = 0;
            m_llLength -= iSectorsRead * RAW_DATA_SIZE;
            m_nFirstSector = iSectorsRead;

            if ((buff[4] & 0xc4) == 0x44) {
                m_subtype = MEDIASUBTYPE_MPEG2_PROGRAM;
            } else if ((buff[4] & 0xf1) == 0x21) {
                m_subtype = MEDIASUBTYPE_MPEG1System;
            }

            return !!(m_subtype != MEDIASUBTYPE_NULL);
        } else if (*((DWORD*)&buff[0]) == 'SggO') {
            m_llPosition = 0;
            m_llLength -= iSectorsRead * RAW_DATA_SIZE;
            m_nFirstSector = iSectorsRead;

            m_subtype = MEDIASUBTYPE_Ogg;

            return true;
        } else if (*((DWORD*)&buff[0]) == 0xA3DF451A) {
            m_llPosition = 0;
            m_llLength -= iSectorsRead * RAW_DATA_SIZE;
            m_nFirstSector = iSectorsRead;

            m_subtype = MEDIASUBTYPE_Matroska;

            return true;
        } else if (*((DWORD*)&buff[0]) == 'FMR.') {
            m_llPosition = 0;
            m_llLength -= iSectorsRead * RAW_DATA_SIZE;
            m_nFirstSector = iSectorsRead;

            m_subtype = MEDIASUBTYPE_RealMedia;

            return true;
        } else if (*((DWORD*)&buff[0]) == 'FFIR' && *((DWORD*)&buff[8]) == ' IVA') {
            m_llPosition = 0;
            m_llLength = std::min(m_llLength, LONGLONG(*((DWORD*)&buff[4])) + 8);
            m_nFirstSector = iSectorsRead;

            m_subtype = MEDIASUBTYPE_Avi;

            return true;
        } else if (*((DWORD*)&buff[4]) == 'voom' || *((DWORD*)&buff[4]) == 'tadm'
                   || *((DWORD*)&buff[4]) == 'pytf' && *((DWORD*)&buff[8]) == 'mosi' && *((DWORD*)&buff[16]) == '14pm') {
            m_llPosition = 0;
            m_llLength -= iSectorsRead * RAW_DATA_SIZE;
            m_nFirstSector = iSectorsRead;

            m_subtype = MEDIASUBTYPE_QTMovie;

            return true;
        }
    }

    m_llPosition = 0;

    CRegKey majorkey;
    CString majortype = _T("\\Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}");
    if (ERROR_SUCCESS == majorkey.Open(HKEY_CLASSES_ROOT, majortype, KEY_READ)) {
        TCHAR subtype[256 + 1];
        DWORD len = 256;
        for (int i = 0; ERROR_SUCCESS == majorkey.EnumKey(i, subtype, &len); i++, len = 256) {
            CRegKey subkey;
            if (ERROR_SUCCESS != subkey.Open(HKEY_CLASSES_ROOT, majortype + _T("\\") + subtype, KEY_READ)) {
                continue;
            }

            for (int j = 0; true; j++) {
                TCHAR number[10];
                _stprintf_s(number, _countof(number), _T("%d"), j);

                TCHAR pattern[256 + 1];
                ULONG lenValue = 256;
                if (ERROR_SUCCESS != subkey.QueryStringValue(number, pattern, &lenValue)) {
                    break;
                }

                CString p = pattern;
                p += _T(',');

                __int64 offset = 0;
                DWORD cb = 0;
                CAtlArray<BYTE> mask, val;

                int nMatches = 0, nTries = 0;

                for (int k = 0, l; nTries >= 0 && (l = p.Find(',', k)) >= 0; k = l + 1, nTries++) {
                    CString s = p.Mid(k, l - k);
                    TRACE(s + '\n');

                    TCHAR* end = nullptr;

                    switch (nTries & 3) {
                        case 0:
                            offset = _tcstol(s, &end, 10);
                            break;
                        case 1:
                            cb = _tcstol(s, &end, 10);
                            break;
                        case 2:
                            CStringToBin(s, mask);
                            break;
                        case 3:
                            CStringToBin(s, val);
                            break;
                        default:
                            ASSERT(FALSE); // Shouldn't happen
                            nTries = -1;
                            break;
                    }

                    if (nTries >= 0 && (nTries & 3) == 3) {
                        if (cb > 0 && !val.IsEmpty() && cb == val.GetCount()) {
                            if (offset >= 0 && S_OK == SetPointer(offset)
                                    || S_OK == SetPointer(m_llLength + offset)) {
                                CAutoVectorPtr<BYTE> pData;
                                if (pData.Allocate(cb)) {
                                    DWORD BytesRead = 0;
                                    if (S_OK == Read(pData, cb, 1, &BytesRead) && cb == BytesRead) {
                                        if (mask.GetCount() < cb) {
                                            size_t x = mask.GetCount();
                                            mask.SetCount(cb);
                                            for (; x < cb; x++) {
                                                mask[x] = 0xff;
                                            }
                                        }

                                        for (unsigned int x = 0; x < cb; x++) {
                                            pData[x] &= (BYTE)mask[x];
                                        }

                                        if (memcmp(pData, val.GetData(), cb) == 0) {
                                            nMatches++;
                                        }
                                    }
                                }
                            }

                            offset = 0;
                            cb = 0;
                            mask.RemoveAll();
                            val.RemoveAll();
                        }
                    }
                }

                if (nMatches > 0 && nMatches * 4 == nTries) {
                    m_subtype = GUIDFromCString(subtype);
                    return S_OK;
                }
            }
        }
    }

    return false;
}
