/*
* (C) 2018 Nicholas Parkanyi
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
#include "YoutubeDL.h"
#include "rapidjson/include/rapidjson/document.h"
#include "mplayerc.h"

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


    CString args = "youtube-dl -J \"" + url + "\"";

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

struct YDLStreamDetails {
    CString protocol;
    CString url;
    int width;
    int height;
    CString vcodec;
    CString acodec;
    bool has_video;
    bool has_audio;
    int vbr;
    int abr;
    int fps;
};

bool GetYDLStreamDetails(const Value& format, YDLStreamDetails& details) {
    details.protocol = format.HasMember(_T("protocol")) && !format[_T("protocol")].IsNull() ? format[_T("protocol")].GetString() : nullptr;
    if (details.protocol && details.protocol != _T("http_dash_segments")) {
        details.url       = format[_T("url")].GetString();
        details.width     = format.HasMember(_T("width"))  && !format[_T("width")].IsNull()  ? format[_T("width")].GetInt() : 0;
        details.height    = format.HasMember(_T("height")) && !format[_T("height")].IsNull() ? format[_T("height")].GetInt() : 0;
        details.vcodec    = format.HasMember(_T("vcodec")) && !format[_T("vcodec")].IsNull() ? format[_T("vcodec")].GetString() : _T("none");
        details.has_video = details.vcodec != _T("none");
        details.acodec    = format.HasMember(_T("acodec")) && !format[_T("acodec")].IsNull() ? format[_T("acodec")].GetString() : _T("none");
        details.has_audio = details.acodec != _T("none");
        if (details.has_video || details.has_audio) {
            details.vbr = details.has_video && format.HasMember(_T("vbr")) && !format[_T("vbr")].IsNull() ? (int) format[_T("vbr")].GetFloat() : 0;
            details.abr = details.has_audio && format.HasMember(_T("abr")) && !format[_T("abr")].IsNull() ? (int) format[_T("abr")].GetFloat() : 0;
            if (details.vbr == 0 && details.has_video) {
                details.vbr = format.HasMember(_T("tbr")) && !format[_T("tbr")].IsNull() ? (int) format[_T("tbr")].GetFloat() : 0;
            }
            if (details.abr == 0 && details.has_audio) {
                details.abr = format.HasMember(_T("tbr")) && !format[_T("tbr")].IsNull() ? (int) format[_T("tbr")].GetFloat() : 0;
            }
        } else {
            // format details unknown, make assumption
            details.has_video = (details.width > 0) || (details.height > 0);
            details.has_audio = true;
        }
        details.fps = details.has_video && format.HasMember(_T("fps")) && !format[_T("fps")].IsNull() ? format[_T("fps")].GetInt() : 0;
        return !details.url.IsEmpty();
    }
    return false;
}

#define YDL_FORMAT_AUTO      0
#define YDL_FORMAT_H264_30   1
#define YDL_FORMAT_H264_60   2
#define YDL_FORMAT_VP9_30    3
#define YDL_FORMAT_VP9_60    4
#define YDL_FORMAT_VP9P2_30  5
#define YDL_FORMAT_VP9P2_60  6
#define YDL_FORMAT_AV1_30    7
#define YDL_FORMAT_AV1_60    8

bool IsBetterYDLStream(YDLStreamDetails& first, YDLStreamDetails& second, int max_height, bool separate, int preferred_format) {  
    if (first.has_video) {
        // We want separate audio/video streams
        if (separate && first.has_audio && !second.has_audio) {
            return true;
        }

        // Video format
        CString vcodec1 = first.vcodec.Left(4);
        CString vcodec2 = second.vcodec.Left(4);
        if (vcodec1 != vcodec2) {
            // AV1
            if (vcodec1 == _T("av01")) {
                return (preferred_format != YDL_FORMAT_AV1_30 && preferred_format != YDL_FORMAT_AV1_60);
            } else {
                if (vcodec2 == _T("av01")) {
                    return (preferred_format == YDL_FORMAT_AV1_30 || preferred_format == YDL_FORMAT_AV1_60);
                }
            }
            // H.264
            if ((preferred_format == YDL_FORMAT_H264_30 || preferred_format == YDL_FORMAT_H264_60)) {
                if (vcodec1 == _T("avc1")) {
                    return false;
                } else {
                    if (vcodec2 == _T("avc1")) {
                        return true;
                    }
                }
            }
        }
        if (first.vcodec != second.vcodec) {
            // VP9P2
            if ((preferred_format == YDL_FORMAT_VP9P2_30 || preferred_format == YDL_FORMAT_VP9P2_60)) {
                if (first.vcodec == _T("vp9.2")) {
                    return false;
                }
                else {
                    if (second.vcodec == _T("vp9.2")) {
                        return true;
                    }
                }
                // Prefer VP9P0 over others
                if (first.vcodec.Left(3) == _T("vp9")) {
                    return false;
                } else {
                    if (second.vcodec.Left(3) == _T("vp9")) {
                        return true;
                    }
                }
            }
            // VP9
            if ((preferred_format == YDL_FORMAT_VP9_30 || preferred_format == YDL_FORMAT_VP9_60)) {
                if (first.vcodec == _T("vp9") || first.vcodec == _T("vp9.0")) {
                    return false;
                }
                else {
                    if (second.vcodec == _T("vp9") || second.vcodec == _T("vp9.0")) {
                        return true;
                    }
                }
            }
        }

        // Video resolution
        if (max_height > 0 && first.height > max_height && first.height > second.height) {
            return true;
        }
        if (max_height > 0 && second.height > max_height) {
            return false;
        }
        if (second.height > first.height) {
            if (max_height > 0) {
                // calculate maximum width based on 16:9 AR
                return (max_height * 16 / 9 + 1) >= second.width;
            }
            return true;
        } else {
            if (second.height == first.height) {
                if (second.width > first.width) {
                    return true;
                }
                if (second.width < first.width) {
                    return false;
                }
            } else {
                return false;
            }
        }

        // Framerate
        if (preferred_format != YDL_FORMAT_AUTO && first.fps != second.fps && first.fps > 0 && second.fps > 0) {
            if (preferred_format == YDL_FORMAT_H264_60 || preferred_format == YDL_FORMAT_VP9_60 || preferred_format == YDL_FORMAT_VP9P2_60 || preferred_format == YDL_FORMAT_AV1_60) {
                if (second.fps > first.fps) {
                    return true;
                } else if (first.fps > second.fps) {
                    return false;
                }
            } else if (preferred_format == YDL_FORMAT_H264_30 || preferred_format == YDL_FORMAT_VP9_30 || preferred_format == YDL_FORMAT_VP9P2_30 || preferred_format == YDL_FORMAT_AV1_30) {
                if (first.fps > 30 && first.fps > second.fps) {
                    return true;
                } else if (second.fps > 30 && second.fps > first.fps) {
                    return false;
                }
            }
        }
    } else {
        // Audio format
        if (first.vcodec.Left(4) == _T("opus")) {
            if (second.vcodec.Left(4) != _T("opus")) {
                return false;
            }
        } else {
            if (second.vcodec.Left(4) == _T("opus")) {
                return true;
            }
        }
    }

    // Prefer HTTP protocol
    if (first.protocol.Left(4) == _T("http")) {
        if (second.protocol.Left(4) != _T("http")) {
            return false;
        }
    } else {
        if (second.protocol.Left(4) == _T("http")) {
            return true;
        }
    }

    // Bitrate
    if (first.has_video) {
        if (second.vbr > first.vbr) {
            return true;
        }
    } else {
        if (second.abr > first.abr) {
            return true;
        }
    }
    return false;
}

// find best video track
bool filterVideo(const Value& formats, YDLStreamDetails& ydl_sd, int max_height, bool separate, int preferred_format)
{
    YDLStreamDetails current;
    bool found = false;

    for (rapidjson::SizeType i = 0; i < formats.Size(); i++) {
        if (GetYDLStreamDetails(formats[i], current) && current.has_video) {
            if (!found || IsBetterYDLStream(ydl_sd, current, max_height, separate, preferred_format)) {
                ydl_sd = current;
            }
            found = true;
        }
    }
    return found;
}

// find best audio track (in case we use separate streams)
bool filterAudio(const Value& formats, YDLStreamDetails& ydl_sd)
{
    YDLStreamDetails current;
    bool found = false;

    for (rapidjson::SizeType i = 0; i < formats.Size(); i++) {
        if (GetYDLStreamDetails(formats[i], current) && !current.has_video) {
            if (!found || IsBetterYDLStream(ydl_sd, current, 0, true, 0)) {
                ydl_sd = current;
            }
            found = true;
        }
    }
    return found;
}

bool CYoutubeDLInstance::GetHttpStreams(CAtlList<YDLStreamURL>& streams)
{
    CString url;
    CString extractor;
    YDLStreamDetails ydl_sd;
    YDLStreamURL stream;

    if (pJSON->d.IsObject() && pJSON->d.HasMember(_T("extractor"))) {
        extractor = pJSON->d[_T("extractor")].GetString();
    } else {
        return false;
    }

    auto& s = AfxGetAppSettings();

    if (!bIsPlaylist) {
        if (!pJSON->d.HasMember(_T("formats"))) {
            return false;
        }

        if (pJSON->d.HasMember(_T("title"))) {
            stream.title = pJSON->d[_T("title")].GetString();
        } else {
            stream.title = _T("");
        }

        // detect generic http link
        if (extractor == _T("generic")) {
            stream.video_url = pJSON->d[_T("formats")][0][_T("url")].GetString();
            stream.audio_url = _T("");
            streams.AddTail(stream);
            return true;
        }

        if (filterVideo(pJSON->d[_T("formats")], ydl_sd, s.iYDLMaxHeight, s.bYDLAudioOnly, s.iYDLVideoFormat)) {
            stream.video_url = ydl_sd.url;
            stream.audio_url = _T("");
            // find separate audio stream
            if (ydl_sd.has_video && !ydl_sd.has_audio) {
                if (filterAudio(pJSON->d[_T("formats")], ydl_sd)) {
                    stream.audio_url = ydl_sd.url;
                }
            }
            streams.AddTail(stream);
        } else {
            if (filterAudio(pJSON->d[_T("formats")], ydl_sd)) {
                stream.audio_url = ydl_sd.url;
                stream.video_url = _T("");
                streams.AddTail(stream);
            }
        }
    } else {
        if (pJSON->d.HasMember(_T("entries"))) {
            const Value& entries = pJSON->d[_T("entries")];

            for (rapidjson::SizeType i = 0; i < entries.Size(); i++) {
                if (filterVideo(entries[i][_T("formats")], ydl_sd, s.iYDLMaxHeight, s.bYDLAudioOnly, s.iYDLVideoFormat)) {
                    stream.video_url = ydl_sd.url;
                    stream.title = entries[i][_T("title")].GetString();
                    stream.audio_url = _T("");
                    if (ydl_sd.has_video && !ydl_sd.has_audio) {
                        if (filterAudio(entries[i][_T("formats")], ydl_sd)) {
                            stream.audio_url = ydl_sd.url;
                        }
                    }
                    streams.AddTail(stream);
                } else {
                    if (filterAudio(entries[i][_T("formats")], ydl_sd)) {
                        stream.audio_url = ydl_sd.url;
                        stream.title = entries[i][_T("title")].GetString();
                        stream.video_url = _T("");
                        streams.AddTail(stream);
                    }
                }
            }
        }
    }
    return !streams.IsEmpty();
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
