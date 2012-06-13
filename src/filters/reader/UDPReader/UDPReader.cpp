/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include "UDPReader.h"
#include "../../../DSUtil/DSUtil.h"

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudOpPin[] = {
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CUDPReader), UDPReaderName, MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CUDPReader>, NULL, &sudFilter[0]}
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    SetRegKeyValue(_T("udp"), 0, _T("Source Filter"), CStringFromGUID(__uuidof(CUDPReader)));
    SetRegKeyValue(_T("tévé"), 0, _T("Source Filter"), CStringFromGUID(__uuidof(CUDPReader)));

    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    // TODO

    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

#define BUFF_SIZE (256*1024)
#define BUFF_SIZE_FIRST (4*BUFF_SIZE)

//
// CUDPReader
//

CUDPReader::CUDPReader(IUnknown* pUnk, HRESULT* phr)
    : CAsyncReader(NAME("CUDPReader"), pUnk, &m_stream, phr, __uuidof(this))
{
    if (phr) {
        *phr = S_OK;
    }
}

CUDPReader::~CUDPReader()
{
}

STDMETHODIMP CUDPReader::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IFileSourceFilter)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// IFileSourceFilter

STDMETHODIMP CUDPReader::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
    if (!m_stream.Load(pszFileName)) {
        return E_FAIL;
    }

    m_fn = pszFileName;

    CMediaType mt;
    mt.majortype = MEDIATYPE_Stream;
    mt.subtype = m_stream.GetSubType();
    m_mt = mt;

    return S_OK;
}

STDMETHODIMP CUDPReader::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
    if (!ppszFileName) {
        return E_POINTER;
    }

    *ppszFileName = (LPOLESTR)CoTaskMemAlloc((m_fn.GetLength() + 1) * sizeof(WCHAR));
    if (!(*ppszFileName)) {
        return E_OUTOFMEMORY;
    }

    wcscpy_s(*ppszFileName, m_fn.GetLength() + 1, m_fn);

    return S_OK;
}

// CUDPStream

CUDPStream::CUDPStream()
{
    m_port = 0;
    m_socket = INVALID_SOCKET;
    m_subtype = MEDIASUBTYPE_NULL;
}

CUDPStream::~CUDPStream()
{
    Clear();
}

void CUDPStream::Clear()
{
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    if (CAMThread::ThreadExists()) {
        CAMThread::CallWorker(CMD_EXIT);
        CAMThread::Close();
    }
    while (!m_packets.IsEmpty()) {
        delete m_packets.RemoveHead();
    }
    m_pos = m_len = 0;
    m_drop = false;
}

void CUDPStream::Append(BYTE* buff, int len)
{
    CAutoLock cAutoLock(&m_csLock);

    if (m_packets.GetCount() > 1) {
        __int64 size = m_packets.GetTail()->m_end - m_packets.GetHead()->m_start;

        if (!m_drop && (m_pos >= BUFF_SIZE_FIRST && size >= BUFF_SIZE_FIRST || size >= 2 * BUFF_SIZE_FIRST)) {
            m_drop = true;
            TRACE(_T("DROP ON\n"));
        } else if (m_drop && size <= BUFF_SIZE_FIRST) {
            m_drop = false;
            TRACE(_T("DROP OFF\n"));
        }

        if (m_drop) {
            return;
        }
    }

    m_packets.AddTail(DNew packet_t(buff, m_len, m_len + len));
    m_len += len;
}

bool CUDPStream::Load(const WCHAR* fnw)
{
    Clear();

    CStringW url = CStringW(fnw);

    CAtlList<CStringW> sl;
    Explode(url, sl, ':');
    if (sl.GetCount() != 3) {
        return false;
    }

    CStringW protocol = sl.RemoveHead();
    // if (protocol != L"udp") return false;

    m_ip = CString(sl.RemoveHead()).TrimLeft('/');

    int port = _wtoi(Explode(sl.RemoveHead(), sl, '/', 2));
    if (port < 0 || port > 0xffff) {
        return false;
    }
    m_port = port;

    if (sl.GetCount() != 2 || FAILED(GUIDFromCString(CString(sl.GetTail()), m_subtype))) {
        m_subtype = MEDIASUBTYPE_NULL;    // TODO: detect subtype
    }

    CAMThread::Create();
    if (FAILED(CAMThread::CallWorker(CMD_RUN))) {
        Clear();
        return false;
    }

    clock_t start = clock();
    while (clock() - start < 3000 && m_len < 1000000) {
        Sleep(100);
    }

    return true;
}

HRESULT CUDPStream::SetPointer(LONGLONG llPos)
{
    CAutoLock cAutoLock(&m_csLock);

    if (m_packets.IsEmpty() && llPos != 0
            || !m_packets.IsEmpty() && llPos < m_packets.GetHead()->m_start
            || !m_packets.IsEmpty() && llPos > m_packets.GetTail()->m_end) {
        TRACE(_T("CUDPStream: SetPointer error\n"));
        return E_FAIL;
    }

    m_pos = llPos;

    return S_OK;
}

HRESULT CUDPStream::Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead)
{
    CAutoLock cAutoLock(&m_csLock);

    DWORD len = dwBytesToRead;
    BYTE* ptr = pbBuffer;

    while (len > 0 && !m_packets.IsEmpty()) {
        POSITION pos = m_packets.GetHeadPosition();
        while (pos && len > 0) {
            packet_t* p = m_packets.GetNext(pos);

            if (p->m_start <= m_pos && m_pos < p->m_end) {
                DWORD size;

                if (m_pos < p->m_start) {
                    ASSERT(0);
                    size = (DWORD)min(len, p->m_start - m_pos);
                    memset(ptr, 0, size);
                } else {
                    size = (DWORD)min(len, p->m_end - m_pos);
                    memcpy(ptr, &p->m_buff[m_pos - p->m_start], size);
                }

                m_pos += size;

                ptr += size;
                len -= size;
            }

            if (p->m_end <= m_pos - 2048 && BUFF_SIZE_FIRST <= m_pos) {
                while (m_packets.GetHeadPosition() != pos) {
                    delete m_packets.RemoveHead();
                }
            }

        }
    }

    if (pdwBytesRead) {
        *pdwBytesRead = ptr - pbBuffer;
    }

    return S_OK;
}

LONGLONG CUDPStream::Size(LONGLONG* pSizeAvailable)
{
    CAutoLock cAutoLock(&m_csLock);
    if (pSizeAvailable) {
        *pSizeAvailable = m_len;
    }
    return 0;
}

DWORD CUDPStream::Alignment()
{
    return 1;
}

void CUDPStream::Lock()
{
    m_csLock.Lock();
}

void CUDPStream::Unlock()
{
    m_csLock.Unlock();
}

DWORD CUDPStream::ThreadProc()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((u_short)m_port);

    ip_mreq imr;
    imr.imr_multiaddr.s_addr = inet_addr(CStringA(m_ip));
    imr.imr_interface.s_addr = INADDR_ANY;

    if ((m_socket = socket(AF_INET, SOCK_DGRAM, 0)) != INVALID_SOCKET) {
        /*      u_long argp = 1;
                ioctlsocket(m_socket, FIONBIO, &argp);
        */
        DWORD dw = TRUE;
        if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&dw, sizeof(dw)) == SOCKET_ERROR) {
            closesocket(m_socket);
            m_socket =  INVALID_SOCKET;
        }

        if (bind(m_socket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            closesocket(m_socket);
            m_socket =  INVALID_SOCKET;
        }

        if (IN_MULTICAST(htonl(imr.imr_multiaddr.s_addr))) {
            int ret = setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&imr, sizeof(imr));
            if (ret < 0) {
                ret = ::WSAGetLastError();
            }
            ret = ret;
        }
    }

    SetThreadPriority(m_hThread, THREAD_PRIORITY_TIME_CRITICAL);

#ifdef _DEBUG
    FILE* dump = NULL;
    //  dump = _tfopen(_T("c:\\udp.ts"), _T("wb"));
    FILE* log = NULL;
    //  log = _tfopen(_T("c:\\udp.txt"), _T("wt"));
#endif

    for (;;) {
        DWORD cmd = GetRequest();

        switch (cmd) {
            default:
            case CMD_EXIT:
                if (m_socket != INVALID_SOCKET) {
                    closesocket(m_socket);
                    m_socket =  INVALID_SOCKET;
                }
                WSACleanup();
#ifdef _DEBUG
                if (dump) {
                    fclose(dump);
                }
                if (log) {
                    fclose(log);
                }
#endif
                Reply(S_OK);
                return 0;
            case CMD_RUN:
                Reply(m_socket != INVALID_SOCKET ? S_OK : E_FAIL);

                {
                    char buff[65536 * 2];
                    int buffsize = 0;

                    for (unsigned int i = 0; ; i++) {
                        if (!(i & 0xff)) {
                            if (CheckRequest(NULL)) {
                                break;
                            }
                        }

                        int fromlen = sizeof(addr);
                        int len = recvfrom(m_socket, &buff[buffsize], 65536, 0, (SOCKADDR*)&addr, &fromlen);
                        if (len <= 0) {
                            Sleep(1);
                            continue;
                        }

#ifdef _DEBUG
                        if (log) {
                            if (buffsize >= len && !memcmp(&buff[buffsize - len], &buff[buffsize], len)) {
                                DWORD pid = ((buff[buffsize + 1] << 8) | buff[buffsize + 2]) & 0x1fff;
                                DWORD counter = buff[buffsize + 3] & 0xf;
                                _ftprintf_s(log, _T("%04d %2d DUP\n"), pid, counter);
                            }
                        }
#endif

                        buffsize += len;

                        if (buffsize >= 65536 || m_len == 0) {
#ifdef _DEBUG
                            if (dump) {
                                fwrite(buff, buffsize, 1, dump);
                            }

                            if (log) {
                                static BYTE pid2counter[0x2000];
                                static bool init = false;
                                if (!init) {
                                    memset(pid2counter, 0, sizeof(pid2counter));
                                    init = true;
                                }

                                for (int i = 0; i < buffsize; i += 188) {
                                    DWORD pid = ((buff[i + 1] << 8) | buff[i + 2]) & 0x1fff;
                                    BYTE counter = buff[i + 3] & 0xf;
                                    if (pid2counter[pid] != ((counter - 1 + 16) & 15)) {
                                        _ftprintf_s(log, _T("%04x %2d -> %2d\n"), pid, pid2counter[pid], counter);
                                    }
                                    pid2counter[pid] = counter;
                                }
                            }
#endif

                            Append((BYTE*)buff, buffsize);
                            buffsize = 0;
                        }
                    }
                }
                break;
        }
    }

    ASSERT(0);
    return (DWORD) - 1;
}

CUDPStream::packet_t::packet_t(BYTE* p, __int64 start, __int64 end)
    : m_start(start)
    , m_end(end)
{
    size_t size = (size_t)(end - start);
    m_buff = DNew BYTE[size];
    memcpy(m_buff, p, size);
}
