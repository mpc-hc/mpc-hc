#include "stdafx.h"
#include "YoutubeDL.h"
#include "rapidjson/include/rapidjson/document.h"

typedef rapidjson::GenericValue<rapidjson::UTF16<>> Value;

struct CUtf16JSON {
    rapidjson::GenericDocument<rapidjson::UTF16<>> d;
};


CYoutubeDLInstance::CYoutubeDLInstance()
    : idx_out(0), idx_err(0),
      buf_out(nullptr), buf_err(nullptr),
      capacity_out(0), capacity_err(0),
      pJSON(new CUtf16JSON)
{
}

CYoutubeDLInstance::~CYoutubeDLInstance()
{
    std::free(buf_out);
    std::free(buf_err);
    delete pJSON;
}

bool CYoutubeDLInstance::Run(CString url)
{
    const size_t bufsize = 2000;  //2KB initial buffer size

    /////////////////////////////
    // Set up youtube-dl process
    /////////////////////////////

    PROCESS_INFORMATION proc_info;
    STARTUPINFO startup_info;
    SECURITY_ATTRIBUTES sec_attrib;


    CString args = "youtube-dl -J -- \"" + url + "\"";

    ZeroMemory(&proc_info, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&startup_info, sizeof(STARTUPINFO));

    //child process must inherit the handles
    sec_attrib.nLength = sizeof(SECURITY_ATTRIBUTES);
    sec_attrib.lpSecurityDescriptor = NULL;
    sec_attrib.bInheritHandle = true;

    if (!CreatePipe(&hStdout_r, &hStdout_w, &sec_attrib, bufsize)) {
        return false;
    }
    if (!CreatePipe(&hStderr_r, &hStderr_w, &sec_attrib, bufsize)) {
        return false;
    }

    startup_info.cb = sizeof(STARTUPINFO);
    startup_info.hStdOutput = hStdout_w;
    startup_info.hStdError = hStderr_w;
    startup_info.wShowWindow = SW_HIDE;
    startup_info.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

    if (!CreateProcess(NULL, args.GetBuffer(), NULL, NULL, true, 0,
                       NULL, NULL, &startup_info, &proc_info)) {
        return false;
    }

    //we must close the parent process's write handles before calling ReadFile,
    // otherwise it will block forever.
    CloseHandle(hStdout_w);
    CloseHandle(hStderr_w);


    /////////////////////////////////////////////////////
    // Read in stdout and stderr through the pipe buffer
    /////////////////////////////////////////////////////

    buf_out = static_cast<char*>(std::malloc(bufsize));
    buf_err = static_cast<char*>(std::malloc(bufsize));
    capacity_out = bufsize;
    capacity_err = bufsize;

    HANDLE hThreadOut, hThreadErr;
    idx_out = 0;
    idx_err = 0;

    hThreadOut = CreateThread(NULL, 0, BuffOutThread, this, NULL, NULL);
    hThreadErr = CreateThread(NULL, 0, BuffErrThread, this, NULL, NULL);

    WaitForSingleObject(hThreadOut, INFINITE);
    WaitForSingleObject(hThreadErr, INFINITE);

    if (!buf_out || !buf_err) {
        throw std::bad_alloc();
    }

    //NULL-terminate the data
    char* tmp;
    if (idx_out == capacity_out) {
        tmp = static_cast<char*>(std::realloc(buf_out, capacity_out + 1));
        if (tmp) {
            buf_out = tmp;
        }
    }
    buf_out[idx_out] = '\0';

    if (idx_err == capacity_err) {
        tmp = static_cast<char*>(std::realloc(buf_err, capacity_err + 1));
        if (tmp) {
            buf_err = tmp;
        }
    }
    buf_err[idx_err] = '\0';

    CString err = buf_err;
    DWORD exitcode;
    GetExitCodeProcess(proc_info.hProcess, &exitcode);
    if (exitcode) {
        AfxMessageBox(err.GetBuffer(), MB_ICONERROR, 0);
        return false;
    }

    CloseHandle(proc_info.hProcess);
    CloseHandle(proc_info.hThread);
    CloseHandle(hThreadOut);
    CloseHandle(hThreadErr);
    CloseHandle(hStdout_r);
    CloseHandle(hStderr_r);

    return loadJSON();
}

DWORD WINAPI CYoutubeDLInstance::BuffOutThread(void* ydl_inst)
{
    auto ydl = static_cast<CYoutubeDLInstance*>(ydl_inst);
    DWORD read;

    while (ReadFile(ydl->hStdout_r, ydl->buf_out + ydl->idx_out, ydl->capacity_out - ydl->idx_out, &read, NULL)) {
        ydl->idx_out += read;
        if (ydl->idx_out == ydl->capacity_out) {
            ydl->capacity_out *= 2;
            char* tmp = static_cast<char*>(std::realloc(ydl->buf_out, ydl->capacity_out));
            if (tmp) {
                ydl->buf_out = tmp;
            } else {
                std::free(ydl->buf_out);
                ydl->buf_out = nullptr;
                return 0;
            }
        }
    }

    return GetLastError() == ERROR_BROKEN_PIPE ? 0 : GetLastError();
}

DWORD WINAPI CYoutubeDLInstance::BuffErrThread(void* ydl_inst)
{
    auto ydl = static_cast<CYoutubeDLInstance*>(ydl_inst);
    DWORD read;

    while (ReadFile(ydl->hStderr_r, ydl->buf_err + ydl->idx_err, ydl->capacity_err - ydl->idx_err, &read, NULL)) {
        ydl->idx_err += read;
        if (ydl->idx_err == ydl->capacity_err) {
            ydl->capacity_err *= 2;
            char* tmp = static_cast<char*>(std::realloc(ydl->buf_err, ydl->capacity_err));
            if (tmp) {
                ydl->buf_err = tmp;
            } else {
                std::free(ydl->buf_err);
                ydl->buf_err = nullptr;
                return 0;
            }
        }
    }

    return GetLastError() == ERROR_BROKEN_PIPE ? 0 : GetLastError();
}


//find highest resolution
void filterVideo(const Value& formats, CString& url)
{
    int maxheight = 0;

    //no heights to compare, just return last format
    if (formats[0].FindMember(_T("height")) == formats[0].MemberEnd()) {
        url = formats[formats.Size() - 1][_T("url")].GetString();
    }

    for (rapidjson::SizeType i = 0; i < formats.Size(); i++) {
        int curheight = 0;
        if (formats[i].FindMember(_T("height")) != formats[i].MemberEnd() && !formats[i][_T("height")].IsNull()) {
            curheight = formats[i][_T("height")].GetInt();
        }
        if (curheight >= maxheight) {
            maxheight = curheight;
            url = formats[i][_T("url")].GetString();
        }
    }
}

//find audio track with highest bitrate (some sites (youtube, vidme) have audio and videos streams separate)
bool filterAudio(const Value& formats, CString& url)
{
    float maxtbr = 0.0;
    bool found = false;

    for (rapidjson::SizeType i = 0; i < formats.Size(); i++) {
        //only want audio streams
        //youtube and vidme mark audio-only with vcodec = "none"
        if (formats[i].HasMember(_T("vcodec")) &&
                !formats[i][_T("vcodec")].IsNull() &&
                CString(formats[i][_T("vcodec")].GetString()) == _T("none") &&
                formats[i].HasMember(_T("tbr")) &&
                !formats[i][_T("tbr")].IsNull()) {
            float curtbr = formats[i][_T("tbr")].GetFloat();
            if (curtbr >= maxtbr) {
                maxtbr = curtbr;
                url = formats[i][_T("url")].GetString();
                found = true;
            }
        }
    }
    return found;
}

bool CYoutubeDLInstance::GetHttpStreams(CAtlList<CString>& videos, CAtlList<CString>& audio, CAtlList<CString>& names)
{
    CString url;
    CString extractor = pJSON->d[_T("extractor")].GetString();

    if (!bIsPlaylist) {
        names.AddTail(pJSON->d[_T("title")].GetString());

        //detect generic http link; JSON fields below may not exist
        if (extractor == _T("generic")) {
            videos.AddTail(pJSON->d[_T("formats")][0][_T("url")].GetString());
            return true;
        }

        filterVideo(pJSON->d[_T("formats")], url);
        videos.AddTail(url);

        //find separate audio stream, if applicable
        if (filterAudio(pJSON->d[_T("formats")], url)) {
            audio.AddTail(url);
        }
    } else {
        const Value& entries = pJSON->d[_T("entries")];

        for (rapidjson::SizeType i = 0; i < entries.Size(); i++) {
            filterVideo(entries[i][_T("formats")], url);
            videos.AddTail(url);
            names.AddTail(entries[i][_T("title")].GetString());

            if (filterAudio(entries[i][_T("formats")], url)) {
                audio.AddTail(url);
            }
        }
    }
    return true;
}


bool CYoutubeDLInstance::loadJSON()
{
    //the JSON buffer is ASCII with Unicode encoded with escape characters
    pJSON->d.Parse<rapidjson::kParseDefaultFlags, rapidjson::ASCII<>>(buf_out);
    if (pJSON->d.HasParseError()) {
        return false;
    }
    bIsPlaylist = pJSON->d.FindMember(_T("entries")) != pJSON->d.MemberEnd();
    return true;
}
