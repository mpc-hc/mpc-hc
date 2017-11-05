#pragma once
#include "stdafx.h"

struct  CUtf16JSON;

class CYoutubeDLInstance
{
public:
    CYoutubeDLInstance();
    ~CYoutubeDLInstance();

    bool Run(CString url);
    bool GetHttpStreams(CAtlList<CString>& videos, CAtlList<CString>& audio, CAtlList<CString>& names);

private:
    CUtf16JSON* pJSON;
    bool bIsPlaylist;
    HANDLE hStdout_r, hStdout_w;
    HANDLE hStderr_r, hStderr_w;
    char* buf_out;
    char* buf_err;
    DWORD idx_out;
    DWORD idx_err;
    DWORD capacity_out, capacity_err;

    bool loadJSON();
    static DWORD WINAPI BuffOutThread(void* ydl_inst);
    static DWORD WINAPI BuffErrThread(void* ydl_inst);
};
