/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include <atlisapi.h>
#include "mplayerc.h"
#include "resource.h"
#include "MainFrm.h"
#include "../Subtitles/TextFile.h"
#include "WebServer.h"
#include "WebClientSocket.h"

#define MAX_HEADER_SIZE 512 * 1024
#define MAX_DATA_SIZE 2 * 1024 * 1024

CWebClientSocket::CWebClientSocket(CWebServer* pWebServer, CMainFrame* pMainFrame)
    : m_pWebServer(pWebServer)
    , m_pMainFrame(pMainFrame)
    , m_buffLen(0)
    , m_buffLenProcessed(0)
    , m_buffMaxLen(2048)
    , m_parsingState(PARSING_HEADER)
    , m_dataLen(0)
{
    m_buff = DEBUG_NEW char[m_buffMaxLen];
}

CWebClientSocket::~CWebClientSocket()
{
    delete [] m_buff;
}

bool CWebClientSocket::SetCookie(CStringA name, CString value, __time64_t expire, CString path, CString domain)
{
    if (name.IsEmpty()) {
        return false;
    }
    if (value.IsEmpty()) {
        m_cookie.RemoveKey(name);
        return true;
    }

    m_cookie[name] = value;

    m_cookieattribs[name].path = path;
    m_cookieattribs[name].domain = domain;

    if (expire >= 0) {
        CTime t(expire);
        SYSTEMTIME st;
        t.GetAsSystemTime(st);
        CStringA str;
        SystemTimeToHttpDate(st, str);
        m_cookieattribs[name].expire = str;
    }

    return true;
}

void CWebClientSocket::Clear()
{
    m_buffLen = 0;
    m_buffLenProcessed = 0;

    m_parsingState = PARSING_HEADER;

    m_dataLen = 0;

    m_hdrlines.RemoveAll();
    m_data.Empty();

    m_cmd.Empty();
    m_path.Empty();
    m_ver.Empty();
    m_get.RemoveAll();
    m_post.RemoveAll();
    m_cookie.RemoveAll();
    m_request.RemoveAll();
}

void CWebClientSocket::HandleRequest()
{
    // remember new cookies

    CStringA value;
    if (m_hdrlines.Lookup("cookie", value)) {
        CAtlList<CStringA> sl;
        Explode(value, sl, ';');
        POSITION pos = sl.GetHeadPosition();
        while (pos) {
            CAtlList<CStringA> sl2;
            Explode(sl.GetNext(pos), sl2, '=', 2);
            m_cookie[sl2.GetHead()] = sl2.GetCount() == 2 ? UTF8To16(sl2.GetTail()) : _T("");
        }
    }

    // start new session

    if (!m_cookie.Lookup("MPCSESSIONID", m_sessid)) {
        srand((unsigned int)time(nullptr));
        m_sessid.Format(_T("%08x"), rand() * 0x12345678);
        SetCookie("MPCSESSIONID", m_sessid);
    } else {
        // TODO: load session
    }

    CStringA reshdr, resbody;

    if (m_cmd == _T("GET") || m_cmd == _T("HEAD") || m_cmd == _T("POST")) {
        int k = m_path.Find('?');
        if (k >= 0) {
            m_query = m_path.Mid(k + 1);
            m_path.Truncate(k);
        }
        m_path = UrlDecode(m_path);

        if (!m_query.IsEmpty()) {
            int k = m_query.Find('#');
            if (k >= 0) {
                m_query.Truncate(k);
            }

            CAtlList<CStringA> sl;
            Explode(m_query, sl, '&');
            POSITION pos = sl.GetHeadPosition();
            while (pos) {
                CAtlList<CStringA> sl2;
                Explode(sl.GetNext(pos), sl2, '=', 2);
                if (sl2.GetCount() == 2) {
                    m_get[sl2.GetHead()] = UTF8To16(UrlDecode(sl2.GetTail()));
                } else {
                    m_get[sl2.GetHead()] = _T("");
                }
            }
        }

        // m_request <-- m_get+m_post+m_cookie
        {
            CStringA key;
            CString value;
            POSITION pos;
            pos = m_get.GetStartPosition();
            while (pos) {
                m_get.GetNextAssoc(pos, key, value);
                m_request[key] = value;
            }
            pos = m_post.GetStartPosition();
            while (pos) {
                m_post.GetNextAssoc(pos, key, value);
                m_request[key] = value;
            }
            pos = m_cookie.GetStartPosition();
            while (pos) {
                m_cookie.GetNextAssoc(pos, key, value);
                m_request[key] = value;
            }
        }

        m_pWebServer->OnRequest(this, reshdr, resbody);
    } else {
        reshdr = "HTTP/1.0 400 Bad Request\r\n";
    }

    if (!reshdr.IsEmpty()) {
        // cookies
        {
            POSITION pos = m_cookie.GetStartPosition();
            while (pos) {
                CStringA key;
                CString value;
                m_cookie.GetNextAssoc(pos, key, value);
                reshdr += "Set-Cookie: " + key + "=" + TToA(value);
                POSITION pos2 = m_cookieattribs.GetStartPosition();
                while (pos2) {
                    cookie_attribs value;
                    m_cookieattribs.GetNextAssoc(pos2, key, value);
                    if (!value.path.IsEmpty()) {
                        reshdr += "; path=" + value.path;
                    }
                    if (!value.expire.IsEmpty()) {
                        reshdr += "; expire=" + value.expire;
                    }
                    if (!value.domain.IsEmpty()) {
                        reshdr += "; domain=" + value.domain;
                    }
                }
                reshdr += "\r\n";
            }
        }

        reshdr +=
            "Server: MPC-HC WebServer\r\n"
            "Connection: close\r\n"
            "\r\n";

        Send(reshdr, reshdr.GetLength());

        if (m_cmd != _T("HEAD") && reshdr.Find("HTTP/1.0 200 OK") == 0 && !resbody.IsEmpty()) {
            Send(resbody, resbody.GetLength());
        }

        CStringA connection = "close";
        m_hdrlines.Lookup("connection", connection);

        Clear();

        // TODO
        // if (connection == _T("close"))
        OnClose(0);
    }
}

void CWebClientSocket::ParseHeader(char* headerEnd)
{
    char* start = m_buff, *end;

    // Parse the request type
    end = strchr(start, ' ');
    m_cmd.SetString(start, int(end - start));
    m_cmd.MakeUpper();
    start = end + 1;
    end = strchr(start, ' ');
    m_path.SetString(start, int(end - start));
    start = end + 1;
    end = strstr(start, "\r\n");
    m_ver.SetString(start, int(end - start));
    m_ver.MakeUpper();

    CStringA key, val;
    start = end + 2;
    while (start < headerEnd) {
        // Parse the header fields
        end = strchr(start, ':');
        key.SetString(start, int(end - start));
        start = end + 1;
        end = strstr(start, "\r\n");
        val.SetString(start, int(end - start));
        start = end + 2;

        m_hdrlines[key.MakeLower()] = val;
    }

    if (m_cmd == _T("POST")) {
        CStringA str;
        if (m_hdrlines.Lookup("content-length", str)) {
            m_dataLen = strtol(str, nullptr, 10);
        }
    }
    m_parsingState = (m_dataLen > 0) ? PARSING_POST_DATA : PARSING_DONE;
}

void CWebClientSocket::ParsePostData()
{
    char* start = m_buff, *end;
    char* endData = m_buff + m_buffLen;
    CStringA key, val;

    while (start < endData) {
        end = strchr(start, '=');
        key.SetString(start, int(end - start));
        start = end + 1;
        end = strchr(start, '&');
        if (!end) {
            end = endData;
        }
        val.SetString(start, int(end - start));
        start = end + 1;

        m_post[key.MakeLower()] = UTF8To16(UrlDecode(val));
    }

    m_parsingState = PARSING_DONE;
}
//

void CWebClientSocket::OnReceive(int nErrorCode)
{
    if (nErrorCode == 0 && m_parsingState != PARSING_DONE) {
        if (m_buffMaxLen - m_buffLen <= 1) {
            char* buff = (char*)realloc(m_buff, 2 * m_buffMaxLen * sizeof(char));
            if (buff) {
                m_buff = buff;
                m_buffMaxLen *= 2;
            } else {
                ASSERT(0);
            }
        }

        int nRead = Receive(m_buff + m_buffLen, m_buffMaxLen - m_buffLen - 1);
        if (nRead > 0) {
            m_buff[nRead] = '\0';
            m_buffLen += nRead;

            switch (m_parsingState) {
                case PARSING_HEADER: {
                    // Search the header end
                    char* headerEnd = strstr(m_buff + m_buffLenProcessed, "\r\n\r\n");

                    if (headerEnd) {
                        ParseHeader(headerEnd);
                        if (m_dataLen > MAX_DATA_SIZE) {
                            // Refuse the connection if someone tries to send
                            // more than MAX_DATA_SIZE of size.
                            OnClose(0);
                            return;
                        }

                        headerEnd += 4;
                        m_buffLen = max(int(m_buff + m_buffLen - headerEnd), 0);
                        if (m_buffLen > 0) {
                            memcpy(m_buff, headerEnd, m_buffLen + 1);
                            if (m_buffLen >= m_dataLen) {
                                ParsePostData();
                            }
                        }
                    } else if (m_buffLen > MAX_HEADER_SIZE) {
                        // If we got more than MAX_HEADER_SIZE of data without finding
                        // the end of the header we close the connection.
                        OnClose(0);
                        return;
                    } else {
                        // Start next search from current end of the file
                        m_buffLenProcessed += nRead;
                    }
                }
                break;
                case PARSING_POST_DATA: {
                    if (m_buffLen >= m_dataLen) {
                        ParsePostData();
                    }
                }
                break;
            }

            if (m_parsingState == PARSING_DONE) {
                HandleRequest();
            }
        }
    }
}

void CWebClientSocket::OnClose(int nErrorCode)
{
    // TODO: save session
    m_pWebServer->OnClose(this);
    __super::OnClose(nErrorCode);
}

////////////////////

bool CWebClientSocket::OnCommand(CStringA& hdr, CStringA& body, CStringA& mime)
{
    CString arg;
    if (m_request.Lookup("wm_command", arg)) {
        int id = _ttol(arg);

        if (id > 0) {
            if (id == ID_FILE_EXIT) {
                m_pMainFrame->PostMessage(WM_COMMAND, id);
            } else {
                m_pMainFrame->SendMessage(WM_COMMAND, id);
            }
        } else {
            if (arg == CMD_SETPOS && m_request.Lookup("position", arg)) {
                int h, m, s, ms = 0;
                TCHAR c;
                if (_stscanf_s(arg, _T("%d%c%d%c%d%c%d"), &h, &c, 1, &m, &c, 1, &s, &c, 1, &ms) >= 5) {
                    REFERENCE_TIME rtPos = 10000i64 * (((h * 60 + m) * 60 + s) * 1000 + ms);
                    m_pMainFrame->SeekTo(rtPos);
                    for (int retries = 20; retries-- > 0; Sleep(50)) {
                        if (abs((int)((rtPos - m_pMainFrame->GetPos()) / 10000)) < 100) {
                            break;
                        }
                    }
                }
            } else if (arg == CMD_SETPOS && m_request.Lookup("percent", arg)) {
                float percent = 0;
                if (_stscanf_s(arg, _T("%f"), &percent) == 1) {
                    m_pMainFrame->SeekTo((REFERENCE_TIME)(percent / 100 * m_pMainFrame->GetDur()));
                }
            } else if (arg == CMD_SETVOLUME && m_request.Lookup("volume", arg)) {
                int volume = _tcstol(arg, nullptr, 10);
                m_pMainFrame->m_wndToolBar.Volume = min(max(volume, 0), 100);
                m_pMainFrame->OnPlayVolume(0);
            }
        }
    }

    CStringA ref;
    if (!m_hdrlines.Lookup("referer", ref)) {
        return true;
    }

    hdr =
        "HTTP/1.0 302 Found\r\n"
        "Location: " + ref + "\r\n";

    return true;
}

bool CWebClientSocket::OnIndex(CStringA& hdr, CStringA& body, CStringA& mime)
{
    CStringA wmcoptions;

    // generate page

    const CAppSettings& s = AfxGetAppSettings();
    POSITION pos = s.wmcmds.GetHeadPosition();
    while (pos) {
        const wmcmd& wc = s.wmcmds.GetNext(pos);
        CStringA str;
        str.Format("%d", wc.cmd);
        CStringA valueName(UTF8(wc.GetName()));
        valueName.Replace("&", "&amp;");
        wmcoptions += "<option value=\"" + str + "\">" + valueName + "</option>\r\n";
    }

    m_pWebServer->LoadPage(IDR_HTML_INDEX, body, AToT(m_path));
    body.Replace("[wmcoptions]", wmcoptions);

    return true;
}

bool CWebClientSocket::OnInfo(CStringA& hdr, CStringA& body, CStringA& mime)
{
    int pos = (int)(m_pMainFrame->GetPos() / 10000);
    int dur = (int)(m_pMainFrame->GetDur() / 10000);

    CString positionstring, durationstring, versionstring, sizestring;
    versionstring.Format(L"%s", AfxGetMyApp()->m_strVersion);
    CPath file(m_pMainFrame->m_wndPlaylistBar.GetCurFileName());
    file.StripPath();
    file.RemoveExtension();

    positionstring.Format(_T("%02d:%02d:%02d"), (pos / 3600000), (pos / 60000) % 60, (pos / 1000) % 60);
    durationstring.Format(_T("%02d:%02d:%02d"), (dur / 3600000), (dur / 60000) % 60, (dur / 1000) % 60);

    WIN32_FIND_DATA wfd;
    HANDLE hFind = FindFirstFile(m_pMainFrame->m_wndPlaylistBar.GetCurFileName(), &wfd);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        __int64 size = (__int64(wfd.nFileSizeHigh) << 32) | wfd.nFileSizeLow;
        const int MAX_FILE_SIZE_BUFFER = 65;
        TCHAR szFileSize[MAX_FILE_SIZE_BUFFER];
        StrFormatByteSizeW(size, szFileSize, MAX_FILE_SIZE_BUFFER);
        sizestring.Format(L"%s", szFileSize);
    }

    m_pWebServer->LoadPage(IDR_HTML_INFO, body, AToT(m_path));
    body.Replace("[version]", UTF8(versionstring));
    body.Replace("[file]", UTF8(file));
    body.Replace("[position]", UTF8(positionstring));
    body.Replace("[duration]", UTF8(durationstring));
    body.Replace("[size]", UTF8(sizestring));
    return true;
}

bool CWebClientSocket::OnBrowser(CStringA& hdr, CStringA& body, CStringA& mime)
{
    CAtlList<CStringA> rootdrives;
    for (TCHAR drive[] = _T("A:"); drive[0] <= 'Z'; drive[0]++) {
        if (GetDriveType(drive) != DRIVE_NO_ROOT_DIR) {
            rootdrives.AddTail(CStringA(drive) + '\\');
        }
    }
    // process GET

    CString path;
    CFileStatus fs;
    if (m_get.Lookup("path", path)) {

        if (CFileGetStatus(path, fs) && !(fs.m_attribute & CFile::directory)) {
            // TODO: make a new message for just opening files, this is a bit overkill now...

            CAtlList<CString> cmdln;

            cmdln.AddTail(path);

            CString focus;
            if (m_get.Lookup("focus", focus) && !focus.CompareNoCase(_T("no"))) {
                cmdln.AddTail(_T("/nofocus"));
            }

            int len = 0;

            POSITION pos = cmdln.GetHeadPosition();
            while (pos) {
                CString& str = cmdln.GetNext(pos);
                len += (str.GetLength() + 1) * sizeof(TCHAR);
            }

            CAutoVectorPtr<BYTE> buff;
            if (buff.Allocate(4 + len)) {
                BYTE* p = buff;
                *(DWORD*)p = (DWORD)cmdln.GetCount();
                p += sizeof(DWORD);

                POSITION pos = cmdln.GetHeadPosition();
                while (pos) {
                    CString& str = cmdln.GetNext(pos);
                    len = (str.GetLength() + 1) * sizeof(TCHAR);
                    memcpy(p, (LPCTSTR)str, len);
                    p += len;
                }

                COPYDATASTRUCT cds;
                cds.dwData = 0x6ABE51;
                cds.cbData = DWORD(p - buff);
                cds.lpData = (void*)(BYTE*)buff;
                m_pMainFrame->SendMessage(WM_COPYDATA, (WPARAM)nullptr, (LPARAM)&cds);
            }

            CPath p(path);
            p.RemoveFileSpec();
            path = (LPCTSTR)p;
        }
    } else {
        path = m_pMainFrame->m_wndPlaylistBar.GetCurFileName();

        if (CFileGetStatus(path, fs) && !(fs.m_attribute & CFile::directory)) {
            CPath p(path);
            p.RemoveFileSpec();
            path = (LPCTSTR)p;
        }
    }

    if (path.Find(_T("://")) >= 0) {
        path.Empty();
    }

    if (CFileGetStatus(path, fs) && (fs.m_attribute & CFile::directory)
            || path.Find(_T("\\")) == 0) { // FIXME
        CPath p(path);
        p.Canonicalize();
        p.MakePretty();
        p.AddBackslash();
        path = (LPCTSTR)p;
    }

    CStringA files;

    if (path.IsEmpty()) {
        POSITION pos = rootdrives.GetHeadPosition();
        while (pos) {
            CStringA& drive = rootdrives.GetNext(pos);

            files += "<tr>\r\n";
            files +=
                "<td class=\"dirname\"><a href=\"[path]?path=" + UrlEncode(drive) + "\">" + drive + "</a></td>\r\n"
                "<td class=\"dirtype\">Directory</td>\r\n"
                "<td class=\"dirsize\">&nbsp;</td>\r\n"
                "<td class=\"dirdate\">&nbsp;</td>\r\n";
            files += "</tr>\r\n";
        }

        path = "Root";
    } else {
        CString parent;

        if (path.GetLength() > 3) {
            CPath p(path + "..");
            p.Canonicalize();
            p.AddBackslash();
            parent = (LPCTSTR)p;
        }

        files += "<tr>\r\n";
        files +=
            "<td class=\"dirname\"><a href=\"[path]?path=" + UTF8Arg(parent) + "\">..</a></td>\r\n"
            "<td class=\"dirtype\">Directory</td>\r\n"
            "<td class=\"dirsize\">&nbsp;</td>\r\n"
            "<td class=\"dirdate\">&nbsp;</td>\r\n";
        files += "</tr>\r\n";

        WIN32_FIND_DATA fd = {0};

        HANDLE hFind = FindFirstFile(path + "*.*", &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.cFileName[0] == '.') {
                    continue;
                }

                CString fullpath = path + fd.cFileName;

                files += "<tr>\r\n";
                files +=
                    "<td class=\"dirname\"><a href=\"[path]?path=" + UTF8Arg(fullpath) + "\">" + UTF8(fd.cFileName) + "</a></td>\r\n"
                    "<td class=\"dirtype\">Directory</td>\r\n"
                    "<td class=\"dirsize\">&nbsp;</td>\r\n"
                    "<td class=\"dirdate\"><span class=\"nobr\">" + CStringA(CTime(fd.ftLastWriteTime).Format(_T("%Y.%m.%d %H:%M"))) + "</span></td>\r\n";
                files += "</tr>\r\n";
            } while (FindNextFile(hFind, &fd));

            FindClose(hFind);
        }

        hFind = FindFirstFile(path + "*.*", &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    continue;
                }

                CString fullpath = path + fd.cFileName;
                TCHAR* ext = _tcsrchr(fd.cFileName, '.');
                if (ext != nullptr) {
                    ext++;
                }

                CStringA size;
                size.Format("%I64dK", ((UINT64)fd.nFileSizeHigh << 22) | (fd.nFileSizeLow >> 10));

                CString type(_T("&nbsp;"));
                LoadType(fullpath, type);

                if (ext != nullptr) {
                    files += "<tr class=\"" + UTF8(ext) + "\">\r\n";
                } else {
                    files += "<tr class=\"noext\">\r\n";
                }
                files +=
                    "<td><a href=\"[path]?path=" + UTF8Arg(fullpath) + "\">" + UTF8(fd.cFileName) + "</a></td>\r\n"
                    "<td><span class=\"nobr\">" + UTF8(type) + "</span></td>\r\n"
                    "<td align=\"right\"><span class=\"nobr\">" + size + "</span></td>\r\n"
                    "<td><span class=\"nobr\">" + CStringA(CTime(fd.ftLastWriteTime).Format(_T("%Y.%m.%d %H:%M"))) + "</span></td>\r\n";
                files += "</tr>\r\n";
            } while (FindNextFile(hFind, &fd));

            FindClose(hFind);
        }
    }

    m_pWebServer->LoadPage(IDR_HTML_BROWSER, body, AToT(m_path));
    body.Replace("[currentdir]", HtmlSpecialChars(UTF8(path)));
    body.Replace("[currentfiles]", files);

    return true;
}

bool CWebClientSocket::OnControls(CStringA& hdr, CStringA& body, CStringA& mime)
{
    CString path = m_pMainFrame->m_wndPlaylistBar.GetCurFileName();
    CString dir;

    if (!path.IsEmpty()) {
        CPath p(path);
        p.RemoveFileSpec();
        dir = (LPCTSTR)p;
    }

    OAFilterState fs = m_pMainFrame->GetMediaState();
    CString state;
    state.Format(_T("%d"), fs);
    CString statestring;
    switch (fs) {
        case State_Stopped:
            statestring.LoadString(IDS_CONTROLS_STOPPED);
            break;
        case State_Paused:
            statestring.LoadString(IDS_CONTROLS_PAUSED);
            break;
        case State_Running:
            statestring.LoadString(IDS_CONTROLS_PLAYING);
            break;
        default:
            statestring = _T("N/A");
            break;
    }

    int pos = (int)(m_pMainFrame->GetPos() / 10000);
    int dur = (int)(m_pMainFrame->GetDur() / 10000);

    CString position, duration;
    position.Format(_T("%d"), pos);
    duration.Format(_T("%d"), dur);

    CString positionstring, durationstring, playbackrate;
    //  positionstring.Format(_T("%02d:%02d:%02d.%03d"), (pos/3600000), (pos/60000)%60, (pos/1000)%60, pos%1000);
    //  durationstring.Format(_T("%02d:%02d:%02d.%03d"), (dur/3600000), (dur/60000)%60, (dur/1000)%60, dur%1000);
    positionstring.Format(_T("%02d:%02d:%02d"), (pos / 3600000), (pos / 60000) % 60, (pos / 1000) % 60);
    durationstring.Format(_T("%02d:%02d:%02d"), (dur / 3600000), (dur / 60000) % 60, (dur / 1000) % 60);
    playbackrate = _T("1"); // TODO

    CString volumelevel, muted;
    volumelevel.Format(_T("%d"), m_pMainFrame->m_wndToolBar.m_volctrl.GetPos());
    muted.Format(_T("%d"), m_pMainFrame->m_wndToolBar.Volume == -10000 ? 1 : 0);

    CString reloadtime(_T("0")); // TODO

    m_pWebServer->LoadPage(IDR_HTML_CONTROLS, body, AToT(m_path));
    body.Replace("[filepatharg]", UTF8Arg(path));
    body.Replace("[filepath]", UTF8(path));
    body.Replace("[filedirarg]", UTF8Arg(dir));
    body.Replace("[filedir]", UTF8(dir));
    body.Replace("[state]", UTF8(state));
    body.Replace("[statestring]", UTF8(statestring));
    body.Replace("[position]", UTF8(position));
    body.Replace("[positionstring]", UTF8(positionstring));
    body.Replace("[duration]", UTF8(duration));
    body.Replace("[durationstring]", UTF8(durationstring));
    body.Replace("[volumelevel]", UTF8(volumelevel));
    body.Replace("[muted]", UTF8(muted));
    body.Replace("[playbackrate]", UTF8(playbackrate));
    body.Replace("[reloadtime]", UTF8(reloadtime));

    return true;
}

bool CWebClientSocket::OnVariables(CStringA& hdr, CStringA& body, CStringA& mime)
{
    CString path = m_pMainFrame->m_wndPlaylistBar.GetCurFileName();
    CString dir;

    if (!path.IsEmpty()) {
        CPath p(path);
        p.RemoveFileSpec();
        dir = (LPCTSTR)p;
    }

    OAFilterState fs = m_pMainFrame->GetMediaState();
    CString state;
    state.Format(_T("%d"), fs);
    CString statestring;
    switch (fs) {
        case State_Stopped:
            statestring.LoadString(IDS_CONTROLS_STOPPED);
            break;
        case State_Paused:
            statestring.LoadString(IDS_CONTROLS_PAUSED);
            break;
        case State_Running:
            statestring.LoadString(IDS_CONTROLS_PLAYING);
            break;
        default:
            statestring = _T("N/A");
            break;
    }

    int pos = (int)(m_pMainFrame->GetPos() / 10000);
    int dur = (int)(m_pMainFrame->GetDur() / 10000);

    CString position, duration;
    position.Format(_T("%d"), pos);
    duration.Format(_T("%d"), dur);

    CString positionstring, durationstring, playbackrate;
    //  positionstring.Format(_T("%02d:%02d:%02d.%03d"), (pos/3600000), (pos/60000)%60, (pos/1000)%60, pos%1000);
    //  durationstring.Format(_T("%02d:%02d:%02d.%03d"), (dur/3600000), (dur/60000)%60, (dur/1000)%60, dur%1000);
    positionstring.Format(_T("%02d:%02d:%02d"), (pos / 3600000), (pos / 60000) % 60, (pos / 1000) % 60);
    durationstring.Format(_T("%02d:%02d:%02d"), (dur / 3600000), (dur / 60000) % 60, (dur / 1000) % 60);
    playbackrate = _T("1"); // TODO

    CString volumelevel, muted;
    volumelevel.Format(_T("%d"), m_pMainFrame->m_wndToolBar.m_volctrl.GetPos());
    muted.Format(_T("%d"), m_pMainFrame->m_wndToolBar.Volume == -10000 ? 1 : 0);

    CString reloadtime(_T("0")); // TODO

    m_pWebServer->LoadPage(IDR_HTML_VARIABLES, body, AToT(m_path));
    body.Replace("[filepatharg]", UTF8Arg(path));
    body.Replace("[filepath]", UTF8(path));
    body.Replace("[filedirarg]", UTF8Arg(dir));
    body.Replace("[filedir]", UTF8(dir));
    body.Replace("[state]", UTF8(state));
    body.Replace("[statestring]", UTF8(statestring));
    body.Replace("[position]", UTF8(position));
    body.Replace("[positionstring]", UTF8(positionstring));
    body.Replace("[duration]", UTF8(duration));
    body.Replace("[durationstring]", UTF8(durationstring));
    body.Replace("[volumelevel]", UTF8(volumelevel));
    body.Replace("[muted]", UTF8(muted));
    body.Replace("[playbackrate]", UTF8(playbackrate));
    body.Replace("[reloadtime]", UTF8(reloadtime));

    return true;
}

bool CWebClientSocket::OnStatus(CStringA& hdr, CStringA& body, CStringA& mime)
{
    CString title;
    m_pMainFrame->GetWindowText(title);

    CPath file(m_pMainFrame->m_wndPlaylistBar.GetCurFileName());

    CString status;
    OAFilterState fs = m_pMainFrame->GetMediaState();
    switch (fs) {
        case State_Stopped:
            status.LoadString(IDS_CONTROLS_STOPPED);
            break;
        case State_Paused:
            status.LoadString(IDS_CONTROLS_PAUSED);
            break;
        case State_Running:
            status.LoadString(IDS_CONTROLS_PLAYING);
            break;
        default:
            status = _T("N/A");
            break;
    }

    int pos = (int)(m_pMainFrame->GetPos() / 10000);
    int dur = (int)(m_pMainFrame->GetDur() / 10000);

    CString posstr, durstr;
    posstr.Format(_T("%02d:%02d:%02d"), (pos / 3600000), (pos / 60000) % 60, (pos / 1000) % 60);
    durstr.Format(_T("%02d:%02d:%02d"), (dur / 3600000), (dur / 60000) % 60, (dur / 1000) % 60);

    title.Replace(_T("'"), _T("\\'"));
    status.Replace(_T("'"), _T("\\'"));

    body.Format("OnStatus(\"%s\", \"%s\", %d, \"%s\", %d, \"%s\", %d, %d, \"%s\")", // , \"%s\"
                UTF8(title), UTF8(status),
                pos, UTF8(posstr), dur, UTF8(durstr),
                m_pMainFrame->IsMuted(), m_pMainFrame->GetVolume(),
                UTF8(file)/*, UTF8(dir)*/);

    return true;
}

bool CWebClientSocket::OnError404(CStringA& hdr, CStringA& body, CStringA& mime)
{
    m_pWebServer->LoadPage(IDR_HTML_404, body, AToT(m_path));
    return true;
}

bool CWebClientSocket::OnPlayer(CStringA& hdr, CStringA& body, CStringA& mime)
{
    m_pWebServer->LoadPage(IDR_HTML_PLAYER, body, AToT(m_path));
    return true;
}

bool CWebClientSocket::OnSnapShotJpeg(CStringA& hdr, CStringA& body, CStringA& mime)
{
    // TODO: add quality control and return logo when nothing is loaded

    bool fRet = false;

    BYTE* pData = nullptr;
    long size = 0;
    if (m_pMainFrame->GetDIB(&pData, size, true)) {
        PBITMAPINFO bi = reinterpret_cast<PBITMAPINFO>(pData);
        PBITMAPINFOHEADER bih = &bi->bmiHeader;

        int bpp = bih->biBitCount;
        if (bpp != 16 && bpp != 24 && bpp != 32) {
            return false;
        }
        int w = bih->biWidth;
        int h = abs(bih->biHeight);
        BYTE* p = DEBUG_NEW BYTE[w * h * 4];

        const BYTE* src = pData + sizeof(*bih);
        if (bpp <= 8) {
            if (bih->biClrUsed) {
                src += bih->biClrUsed * sizeof(bi->bmiColors[0]);
            } else {
                src += (1 << bpp) * DWORD(sizeof(bi->bmiColors[0]));
            }
        }

        int srcpitch = w * (bpp >> 3);
        int dstpitch = w * 4;

        BitBltFromRGBToRGB(w, h, p, dstpitch, 32, (BYTE*)src + srcpitch * (h - 1), -srcpitch, bpp);

        {
            CBitmap bmp;
            bmp.CreateBitmap(w, h, bih->biPlanes, bpp, p);
            delete [] p;

            CImage img;
            img.Attach(bmp);
            IStream* pStream = nullptr;
            CByteArray ba;
            if (SUCCEEDED(CreateStreamOnHGlobal(nullptr, TRUE, &pStream))) {
                if (SUCCEEDED(img.Save(pStream, Gdiplus::ImageFormatJPEG))) {
                    ULARGE_INTEGER ulnSize;
                    LARGE_INTEGER lnOffset;
                    lnOffset.QuadPart = 0;
                    if (SUCCEEDED(pStream->Seek(lnOffset, STREAM_SEEK_END, &ulnSize))) {
                        if (SUCCEEDED(pStream->Seek(lnOffset, STREAM_SEEK_SET, 0))) {
                            ULONG ulBytesRead;
                            ba.SetSize((INT_PTR)ulnSize.QuadPart);
                            pStream->Read(ba.GetData(), (INT_PTR)ulnSize.QuadPart, &ulBytesRead);
                        }
                    }
                }
            }

            pStream->Release();
            delete [] pData;

            hdr +=
                "Expires: Thu, 19 Nov 1981 08:52:00 GMT\r\n"
                "Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0\r\n"
                "Pragma: no-cache\r\n";
            body = CStringA((char*)ba.GetData(), (int)ba.GetCount());
            mime = "image/jpeg";
            fRet = true;
        }
    }

    return fRet;
}

bool CWebClientSocket::OnViewRes(CStringA& hdr, CStringA& body, CStringA& mime)
{
    CString id;
    if (!m_get.Lookup("id", id)) {
        return false;
    }

    uintptr_t key = 0;
    if (1 != _stscanf_s(id, _T("%Ix"), &key) || key == 0) {
        return false;
    }

    CAutoLock cAutoLock(&CDSMResource::m_csResources);

    CDSMResource* res = nullptr;
    if (!CDSMResource::m_resources.Lookup(key, res) || !res) {
        return false;
    }

    body = CStringA((const char*)res->data.GetData(), (int)res->data.GetCount());
    mime = CStringA(res->mime);

    return true;
}
