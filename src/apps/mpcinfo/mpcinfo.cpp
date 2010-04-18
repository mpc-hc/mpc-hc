/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of mpcinfo.
 *
 * Mpcinfo is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mpcinfo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "mpcinfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CmpcinfoApp

BEGIN_MESSAGE_MAP(CmpcinfoApp, CWinApp)
END_MESSAGE_MAP()


// CmpcinfoApp construction

CmpcinfoApp::CmpcinfoApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}


// The one and only CmpcinfoApp object

CmpcinfoApp theApp;


// CmpcinfoApp initialization

BOOL CmpcinfoApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}

#include <dshow.h>
#include <streams.h>
#include <atlbase.h>

static bool GetFilterGraph(IFilterGraph** ppFG)
{
    if(!ppFG) return(false);

    CComPtr<IRunningObjectTable> pROT;
    if(FAILED(GetRunningObjectTable(0, &pROT)))
        return 1;

    CComPtr<IEnumMoniker> pEM;
    if(FAILED(pROT->EnumRunning(&pEM)))
        return 1;

    CComPtr<IBindCtx> pBindCtx;
    CreateBindCtx(0, &pBindCtx);

    for(CComPtr<IMoniker> pMoniker; S_OK == pEM->Next(1, &pMoniker, NULL); pMoniker = NULL)
    {
        LPOLESTR pDispName = NULL;
        if(FAILED(pMoniker->GetDisplayName(pBindCtx, NULL, &pDispName)))
            continue;

        CStringW strw(pDispName);

        CComPtr<IMalloc> pMalloc;
        if(FAILED(CoGetMalloc(1, &pMalloc)))
            continue;
        pMalloc->Free(pDispName);

        if(strw.Find(L"(MPC)") < 0)
            continue;

        CComPtr<IUnknown> pUnk;
        if(S_OK != pROT->GetObject(pMoniker, &pUnk))
            continue;

        CComQIPtr<IFilterGraph> pFG = pUnk;
        if(!pFG)
            continue;

        *ppFG = pFG.Detach();

        break;
    }

    return(!!*ppFG);
}

extern "C" int WINAPI file(HWND,HWND,char *data,char*,BOOL,BOOL)
{
    CComPtr<IFilterGraph> pFG;
    if(!GetFilterGraph(&pFG))
        return 1;

    CString fn;

    CComPtr<IEnumFilters> pEF;
    if(FAILED(pFG->EnumFilters(&pEF)))
        return 1;

    ULONG cFetched = 0;
    for(CComPtr<IBaseFilter> pBF; S_OK == pEF->Next(1, &pBF, &cFetched); pBF = NULL)
    {
        if(CComQIPtr<IFileSourceFilter> pFSF = pBF)
        {
            LPOLESTR pFileName = NULL;
            AM_MEDIA_TYPE mt;
            if(FAILED(pFSF->GetCurFile(&pFileName, &mt)))
                continue;

            fn = CStringW(pFileName);

            CoTaskMemFree(pFileName);
            FreeMediaType(mt);

            break;
        }
    }

    if(fn.IsEmpty())
        return 1;

    sprintf(data, _T("%s"), fn);

    return 3;
}

extern "C" int WINAPI size(HWND,HWND,char *data,char*,BOOL,BOOL)
{
    if(file(0,0,data,0,0,0) != 3)
        return 1;

    CString fn = CStringA(data);
    data[0] = 0;

    HANDLE hFile = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
    if(hFile == INVALID_HANDLE_VALUE)
        return 1;

    LARGE_INTEGER size;
    size.QuadPart = 0;
    size.LowPart = GetFileSize(hFile, (DWORD*)&size.HighPart);

    sprintf(data, _T("%I64d"), size.QuadPart);

    CloseHandle(hFile);

    return 3;
}

extern "C" int WINAPI pos(HWND,HWND,char *data,char*,BOOL,BOOL)
{
    CComPtr<IFilterGraph> pFG;
    if(!GetFilterGraph(&pFG))
        return 1;

    CComQIPtr<IMediaSeeking> pMS = pFG;
    REFERENCE_TIME pos, dur;
    if(FAILED(pMS->GetCurrentPosition(&pos)) || FAILED(pMS->GetDuration(&dur)))
        return 1;

    if(dur > 10000000i64*60*60)
    {
        sprintf(data, _T("%02d:%02d:%02d/%02d:%02d:%02d"),
                (int)(pos/10000000/60/60), (int)(pos/10000000/60)%60, (int)(pos/10000000)%60,
                (int)(dur/10000000/60/60), (int)(dur/10000000/60)%60, (int)(dur/10000000)%60);
    }
    else
    {
        sprintf(data, _T("%02d:%02d/%02d:%02d"),
                (int)(pos/10000000/60)%60, (int)(pos/10000000)%60,
                (int)(dur/10000000/60)%60, (int)(dur/10000000)%60);
    }

    return 3;
}

extern "C" int WINAPI info(HWND,HWND,char *data,char*,BOOL,BOOL)
{
    CStringA ret;
    if(file(0,0,data,0,0,0)!=3) return 1;
    ret += data;
    ret += ", ";
    if(size(0,0,data,0,0,0)!=3) return 1;
    ret += data;
    ret += ", ";
    if(pos(0,0,data,0,0,0)!=3) return 1;
    ret += data;

    strcpy(data, ret);

    return 3;
}

extern "C" int WINAPI stopped(HWND,HWND,char *data,char*,BOOL,BOOL)
{
    sprintf(data, _T("2"));

    CComPtr<IFilterGraph> pFG;
    CComQIPtr<IMediaControl> pMC;
    OAFilterState fs;
    if(!GetFilterGraph(&pFG) || !(pMC = pFG) || FAILED(pMC->GetState(0, &fs)))
        return 3;

    sprintf(data, _T("%d"), fs == State_Stopped ? 1 : 0);

    return 3;
}

extern "C" int WINAPI paused(HWND,HWND,char *data,char*,BOOL,BOOL)
{
    sprintf(data, _T("2"));

    CComPtr<IFilterGraph> pFG;
    CComQIPtr<IMediaControl> pMC;
    OAFilterState fs;
    if(!GetFilterGraph(&pFG) || !(pMC = pFG) || FAILED(pMC->GetState(0, &fs)))
        return 3;

    sprintf(data, _T("%d"), fs == State_Paused ? 1 : 0);

    return 3;
}

extern "C" int WINAPI running(HWND,HWND,char *data,char*,BOOL,BOOL)
{
    sprintf(data, _T("2"));

    CComPtr<IFilterGraph> pFG;
    CComQIPtr<IMediaControl> pMC;
    OAFilterState fs;
    if(!GetFilterGraph(&pFG) || !(pMC = pFG) || FAILED(pMC->GetState(0, &fs)))
        return 3;

    sprintf(data, _T("%d"), fs == State_Running ? 1 : 0);

    return 3;
}
