/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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
#include <Vfw.h>
#include "winddk/devioctl.h"
#include "winddk/ntddcdrm.h"
#include "DSUtil.h"
#include "Mpeg2Def.h"
#include "vd.h"
#include "moreuuids.h"
#include <emmintrin.h>
#include <math.h>
#include <InitGuid.h>
#include <d3d9types.h>
#include <dxva.h>
#include <dxva2api.h>
#include <intrin.h>

int CountPins(IBaseFilter* pBF, int& nIn, int& nOut, int& nInC, int& nOutC)
{
    nIn = nOut = 0;
    nInC = nOutC = 0;

    BeginEnumPins(pBF, pEP, pPin) {
        PIN_DIRECTION dir;
        if (SUCCEEDED(pPin->QueryDirection(&dir))) {
            CComPtr<IPin> pPinConnectedTo;
            pPin->ConnectedTo(&pPinConnectedTo);

            if (dir == PINDIR_INPUT) {
                nIn++;
                if (pPinConnectedTo) {
                    nInC++;
                }
            } else if (dir == PINDIR_OUTPUT) {
                nOut++;
                if (pPinConnectedTo) {
                    nOutC++;
                }
            }
        }
    }
    EndEnumPins;

    return (nIn + nOut);
}

bool IsSplitter(IBaseFilter* pBF, bool fCountConnectedOnly)
{
    int nIn, nOut, nInC, nOutC;
    CountPins(pBF, nIn, nOut, nInC, nOutC);
    return (fCountConnectedOnly ? nOutC > 1 : nOut > 1);
}

bool IsMultiplexer(IBaseFilter* pBF, bool fCountConnectedOnly)
{
    int nIn, nOut, nInC, nOutC;
    CountPins(pBF, nIn, nOut, nInC, nOutC);
    return (fCountConnectedOnly ? nInC > 1 : nIn > 1);
}

bool IsStreamStart(IBaseFilter* pBF)
{
    CComQIPtr<IAMFilterMiscFlags> pAMMF(pBF);
    if (pAMMF && pAMMF->GetMiscFlags()&AM_FILTER_MISC_FLAGS_IS_SOURCE) {
        return true;
    }

    int nIn, nOut, nInC, nOutC;
    CountPins(pBF, nIn, nOut, nInC, nOutC);
    AM_MEDIA_TYPE mt;
    CComPtr<IPin> pIn = GetFirstPin(pBF);
    return ((nOut > 1)
            || (nOut > 0 && nIn == 1 && pIn && SUCCEEDED(pIn->ConnectionMediaType(&mt)) && mt.majortype == MEDIATYPE_Stream));
}

bool IsStreamEnd(IBaseFilter* pBF)
{
    int nIn, nOut, nInC, nOutC;
    CountPins(pBF, nIn, nOut, nInC, nOutC);
    return (nOut == 0);
}

bool IsVideoRenderer(IBaseFilter* pBF)
{
    int nIn, nOut, nInC, nOutC;
    CountPins(pBF, nIn, nOut, nInC, nOutC);

    if (nInC > 0 && nOut == 0) {
        BeginEnumPins(pBF, pEP, pPin) {
            AM_MEDIA_TYPE mt;
            if (S_OK != pPin->ConnectionMediaType(&mt)) {
                continue;
            }

            FreeMediaType(mt);

            return !!(mt.majortype == MEDIATYPE_Video);
            /*&& (mt.formattype == FORMAT_VideoInfo || mt.formattype == FORMAT_VideoInfo2));*/
        }
        EndEnumPins;
    }

    CLSID clsid;
    memcpy(&clsid, &GUID_NULL, sizeof(clsid));
    pBF->GetClassID(&clsid);

    return (clsid == CLSID_VideoRenderer || clsid == CLSID_VideoRendererDefault);
}

bool IsAudioWaveRenderer(IBaseFilter* pBF)
{
    int nIn, nOut, nInC, nOutC;
    CountPins(pBF, nIn, nOut, nInC, nOutC);

    if (nInC > 0 && nOut == 0 && CComQIPtr<IBasicAudio>(pBF)) {
        BeginEnumPins(pBF, pEP, pPin) {
            AM_MEDIA_TYPE mt;
            if (S_OK != pPin->ConnectionMediaType(&mt)) {
                continue;
            }

            FreeMediaType(mt);

            return !!(mt.majortype == MEDIATYPE_Audio);
            /*&& mt.formattype == FORMAT_WaveFormatEx);*/
        }
        EndEnumPins;
    }

    CLSID clsid;
    memcpy(&clsid, &GUID_NULL, sizeof(clsid));
    pBF->GetClassID(&clsid);

    return (clsid == CLSID_DSoundRender || clsid == CLSID_AudioRender || clsid == CLSID_ReClock
            || clsid == __uuidof(CNullAudioRenderer) || clsid == __uuidof(CNullUAudioRenderer));
}

IBaseFilter* GetUpStreamFilter(IBaseFilter* pBF, IPin* pInputPin)
{
    return GetFilterFromPin(GetUpStreamPin(pBF, pInputPin));
}

IPin* GetUpStreamPin(IBaseFilter* pBF, IPin* pInputPin)
{
    BeginEnumPins(pBF, pEP, pPin) {
        if (pInputPin && pInputPin != pPin) {
            continue;
        }

        PIN_DIRECTION dir;
        CComPtr<IPin> pPinConnectedTo;
        if (SUCCEEDED(pPin->QueryDirection(&dir)) && dir == PINDIR_INPUT
                && SUCCEEDED(pPin->ConnectedTo(&pPinConnectedTo))) {
            IPin* pRet = pPinConnectedTo.Detach();
            pRet->Release();
            return pRet;
        }
    }
    EndEnumPins;

    return nullptr;
}

IPin* GetFirstPin(IBaseFilter* pBF, PIN_DIRECTION dir)
{
    if (pBF) {
        BeginEnumPins(pBF, pEP, pPin) {
            PIN_DIRECTION dir2;
            if (SUCCEEDED(pPin->QueryDirection(&dir2)) && dir == dir2) {
                IPin* pRet = pPin.Detach();
                pRet->Release();
                return pRet;
            }
        }
        EndEnumPins;
    }

    return nullptr;
}

IPin* GetFirstDisconnectedPin(IBaseFilter* pBF, PIN_DIRECTION dir)
{
    if (pBF) {
        BeginEnumPins(pBF, pEP, pPin) {
            PIN_DIRECTION dir2;
            CComPtr<IPin> pPinTo;
            if (SUCCEEDED(pPin->QueryDirection(&dir2)) && dir == dir2 && (S_OK != pPin->ConnectedTo(&pPinTo))) {
                IPin* pRet = pPin.Detach();
                pRet->Release();
                return pRet;
            }
        }
        EndEnumPins;
    }

    return nullptr;
}

IBaseFilter* FindFilter(LPCWSTR clsid, IFilterGraph* pFG)
{
    CLSID clsid2;
    CLSIDFromString(CComBSTR(clsid), &clsid2);
    return FindFilter(clsid2, pFG);
}

IBaseFilter* FindFilter(const CLSID& clsid, IFilterGraph* pFG)
{
    BeginEnumFilters(pFG, pEF, pBF) {
        CLSID clsid2;
        if (SUCCEEDED(pBF->GetClassID(&clsid2)) && clsid == clsid2) {
            return pBF;
        }
    }
    EndEnumFilters;

    return nullptr;
}

IPin* FindPin(IBaseFilter* pBF, PIN_DIRECTION direction, const AM_MEDIA_TYPE* pRequestedMT)
{
    PIN_DIRECTION pindir;
    BeginEnumPins(pBF, pEP, pPin) {
        CComPtr<IPin> pFellow;

        if (SUCCEEDED(pPin->QueryDirection(&pindir)) &&
                pindir == direction &&
                pPin->ConnectedTo(&pFellow) == VFW_E_NOT_CONNECTED) {
            BeginEnumMediaTypes(pPin, pEM, pmt) {
                if (pmt->majortype == pRequestedMT->majortype && pmt->subtype == pRequestedMT->subtype) {
                    return (pPin);
                }
            }
            EndEnumMediaTypes(pmt);
        }
    }
    EndEnumPins;
    return nullptr;
}

CStringW GetFilterName(IBaseFilter* pBF)
{
    CStringW name = _T("");

    if (pBF) {
        CFilterInfo fi;
        if (SUCCEEDED(pBF->QueryFilterInfo(&fi))) {
            name = fi.achName;
        }
    }

    return name;
}

CStringW GetPinName(IPin* pPin)
{
    CStringW name;
    CPinInfo pi;
    if (pPin && SUCCEEDED(pPin->QueryPinInfo(&pi))) {
        name = pi.achName;
    }

    return name;
}

IFilterGraph* GetGraphFromFilter(IBaseFilter* pBF)
{
    if (!pBF) {
        return nullptr;
    }
    IFilterGraph* pGraph = nullptr;
    CFilterInfo fi;
    if (pBF && SUCCEEDED(pBF->QueryFilterInfo(&fi))) {
        pGraph = fi.pGraph;
    }
    return pGraph;
}

IBaseFilter* GetFilterFromPin(IPin* pPin)
{
    if (!pPin) {
        return nullptr;
    }
    IBaseFilter* pBF = nullptr;
    CPinInfo pi;
    if (pPin && SUCCEEDED(pPin->QueryPinInfo(&pi))) {
        pBF = pi.pFilter;
    }
    return pBF;
}

IPin* AppendFilter(IPin* pPin, CString DisplayName, IGraphBuilder* pGB)
{
    IPin* pRet = pPin;

    CInterfaceList<IBaseFilter> pFilters;

    do {
        if (!pPin || DisplayName.IsEmpty() || !pGB) {
            break;
        }

        CComPtr<IPin> pPinTo;
        PIN_DIRECTION dir;
        if (FAILED(pPin->QueryDirection(&dir)) || dir != PINDIR_OUTPUT || SUCCEEDED(pPin->ConnectedTo(&pPinTo))) {
            break;
        }

        CComPtr<IBindCtx> pBindCtx;
        CreateBindCtx(0, &pBindCtx);

        CComPtr<IMoniker> pMoniker;
        ULONG chEaten;
        if (S_OK != MkParseDisplayName(pBindCtx, CComBSTR(DisplayName), &chEaten, &pMoniker)) {
            break;
        }

        CComPtr<IBaseFilter> pBF;
        if (FAILED(pMoniker->BindToObject(pBindCtx, 0, IID_PPV_ARGS(&pBF))) || !pBF) {
            break;
        }

        CComPtr<IPropertyBag> pPB;
        if (FAILED(pMoniker->BindToStorage(pBindCtx, 0, IID_PPV_ARGS(&pPB)))) {
            break;
        }

        CComVariant var;
        if (FAILED(pPB->Read(CComBSTR(_T("FriendlyName")), &var, nullptr))) {
            break;
        }

        pFilters.AddTail(pBF);
        BeginEnumFilters(pGB, pEnum, pBF2);
        pFilters.AddTail(pBF2);
        EndEnumFilters;

        if (FAILED(pGB->AddFilter(pBF, CStringW(var.bstrVal)))) {
            break;
        }

        BeginEnumFilters(pGB, pEnum, pBF2);
        if (!pFilters.Find(pBF2) && SUCCEEDED(pGB->RemoveFilter(pBF2))) {
            pEnum->Reset();
        }
        EndEnumFilters;

        pPinTo = GetFirstPin(pBF, PINDIR_INPUT);
        if (!pPinTo) {
            pGB->RemoveFilter(pBF);
            break;
        }

        if (FAILED(pGB->ConnectDirect(pPin, pPinTo, nullptr))) {
            pGB->Connect(pPin, pPinTo);
            pGB->RemoveFilter(pBF);
            break;
        }

        BeginEnumFilters(pGB, pEnum, pBF2);
        if (!pFilters.Find(pBF2) && SUCCEEDED(pGB->RemoveFilter(pBF2))) {
            pEnum->Reset();
        }
        EndEnumFilters;

        pRet = GetFirstPin(pBF, PINDIR_OUTPUT);
        if (!pRet) {
            pRet = pPin;
            pGB->RemoveFilter(pBF);
            break;
        }
    } while (false);

    return pRet;
}

IPin* InsertFilter(IPin* pPin, CString DisplayName, IGraphBuilder* pGB)
{
    do {
        if (!pPin || DisplayName.IsEmpty() || !pGB) {
            break;
        }

        PIN_DIRECTION dir;
        if (FAILED(pPin->QueryDirection(&dir))) {
            break;
        }

        CComPtr<IPin> pFrom, pTo;

        if (dir == PINDIR_INPUT) {
            pPin->ConnectedTo(&pFrom);
            pTo = pPin;
        } else if (dir == PINDIR_OUTPUT) {
            pFrom = pPin;
            pPin->ConnectedTo(&pTo);
        }

        if (!pFrom || !pTo) {
            break;
        }

        CComPtr<IBindCtx> pBindCtx;
        CreateBindCtx(0, &pBindCtx);

        CComPtr<IMoniker> pMoniker;
        ULONG chEaten;
        if (S_OK != MkParseDisplayName(pBindCtx, CComBSTR(DisplayName), &chEaten, &pMoniker)) {
            break;
        }

        CComPtr<IBaseFilter> pBF;
        if (FAILED(pMoniker->BindToObject(pBindCtx, 0, IID_PPV_ARGS(&pBF))) || !pBF) {
            break;
        }

        CComPtr<IPropertyBag> pPB;
        if (FAILED(pMoniker->BindToStorage(pBindCtx, 0, IID_PPV_ARGS(&pPB)))) {
            break;
        }

        CComVariant var;
        if (FAILED(pPB->Read(CComBSTR(_T("FriendlyName")), &var, nullptr))) {
            break;
        }

        if (FAILED(pGB->AddFilter(pBF, CStringW(var.bstrVal)))) {
            break;
        }

        CComPtr<IPin> pFromTo = GetFirstPin(pBF, PINDIR_INPUT);
        if (!pFromTo) {
            pGB->RemoveFilter(pBF);
            break;
        }

        if (FAILED(pGB->Disconnect(pFrom)) || FAILED(pGB->Disconnect(pTo))) {
            pGB->RemoveFilter(pBF);
            pGB->ConnectDirect(pFrom, pTo, nullptr);
            break;
        }

        if (FAILED(pGB->ConnectDirect(pFrom, pFromTo, nullptr))) {
            pGB->RemoveFilter(pBF);
            pGB->ConnectDirect(pFrom, pTo, nullptr);
            break;
        }

        CComPtr<IPin> pToFrom = GetFirstPin(pBF, PINDIR_OUTPUT);
        if (!pToFrom) {
            pGB->RemoveFilter(pBF);
            pGB->ConnectDirect(pFrom, pTo, nullptr);
            break;
        }

        if (FAILED(pGB->ConnectDirect(pToFrom, pTo, nullptr))) {
            pGB->RemoveFilter(pBF);
            pGB->ConnectDirect(pFrom, pTo, nullptr);
            break;
        }

        pPin = pToFrom;
    } while (false);

    return pPin;
}

void ExtractMediaTypes(IPin* pPin, CAtlArray<GUID>& types)
{
    types.RemoveAll();

    BeginEnumMediaTypes(pPin, pEM, pmt) {
        bool fFound = false;

        for (ptrdiff_t i = 0; !fFound && i < (int)types.GetCount(); i += 2) {
            if (types[i] == pmt->majortype && types[i + 1] == pmt->subtype) {
                fFound = true;
            }
        }

        if (!fFound) {
            types.Add(pmt->majortype);
            types.Add(pmt->subtype);
        }
    }
    EndEnumMediaTypes(pmt);
}

void ExtractMediaTypes(IPin* pPin, CAtlList<CMediaType>& mts)
{
    mts.RemoveAll();

    BeginEnumMediaTypes(pPin, pEM, pmt) {
        bool fFound = false;

        POSITION pos = mts.GetHeadPosition();
        while (!fFound && pos) {
            CMediaType& mt = mts.GetNext(pos);
            if (mt.majortype == pmt->majortype && mt.subtype == pmt->subtype) {
                fFound = true;
            }
        }

        if (!fFound) {
            mts.AddTail(CMediaType(*pmt));
        }
    }
    EndEnumMediaTypes(pmt);
}

int Eval_Exception(int n_except)
{
    if (n_except == STATUS_ACCESS_VIOLATION) {
        AfxMessageBox(_T("The property page of this filter has just caused a\nmemory access violation. The application will gently die now :)"));
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

void MyOleCreatePropertyFrame(HWND hwndOwner, UINT x, UINT y, LPCOLESTR lpszCaption, ULONG cObjects, LPUNKNOWN FAR* lplpUnk, ULONG cPages, LPCLSID lpPageClsID, LCID lcid, DWORD dwReserved, LPVOID lpvReserved)
{
    __try {
        OleCreatePropertyFrame(hwndOwner, x, y, lpszCaption, cObjects, lplpUnk, cPages, lpPageClsID, lcid, dwReserved, lpvReserved);
    } __except (Eval_Exception(GetExceptionCode())) {
        // No code; this block never executed.
    }
}

void ShowPPage(CString DisplayName, HWND hParentWnd)
{
    CComPtr<IBindCtx> pBindCtx;
    CreateBindCtx(0, &pBindCtx);

    CComPtr<IMoniker> pMoniker;
    ULONG chEaten;
    if (S_OK != MkParseDisplayName(pBindCtx, CStringW(DisplayName), &chEaten, &pMoniker)) {
        return;
    }

    CComPtr<IBaseFilter> pBF;
    if (FAILED(pMoniker->BindToObject(pBindCtx, 0, IID_PPV_ARGS(&pBF))) || !pBF) {
        return;
    }

    ShowPPage(pBF, hParentWnd);
}

void ShowPPage(IUnknown* pUnk, HWND hParentWnd)
{
    CComQIPtr<ISpecifyPropertyPages> pSPP = pUnk;
    if (!pSPP) {
        return;
    }

    CString str;

    CComQIPtr<IBaseFilter> pBF = pSPP;
    CFilterInfo fi;
    CComQIPtr<IPin> pPin = pSPP;
    CPinInfo pi;
    if (pBF && SUCCEEDED(pBF->QueryFilterInfo(&fi))) {
        str = fi.achName;
    } else if (pPin && SUCCEEDED(pPin->QueryPinInfo(&pi))) {
        str = pi.achName;
    }

    CAUUID caGUID;
    caGUID.pElems = nullptr;
    if (SUCCEEDED(pSPP->GetPages(&caGUID))) {
        IUnknown* lpUnk = nullptr;
        pSPP.QueryInterface(&lpUnk);
        MyOleCreatePropertyFrame(
            hParentWnd, 0, 0, CStringW(str),
            1, (IUnknown**)&lpUnk,
            caGUID.cElems, caGUID.pElems,
            0, 0, nullptr);
        lpUnk->Release();

        if (caGUID.pElems) {
            CoTaskMemFree(caGUID.pElems);
        }
    }
}

CLSID GetCLSID(IBaseFilter* pBF)
{
    CLSID clsid = GUID_NULL;
    if (pBF) {
        pBF->GetClassID(&clsid);
    }
    return clsid;
}

CLSID GetCLSID(IPin* pPin)
{
    return GetCLSID(GetFilterFromPin(pPin));
}

bool IsCLSIDRegistered(LPCTSTR clsid)
{
    CString rootkey1(_T("CLSID\\"));
    CString rootkey2(_T("CLSID\\{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance\\"));

    return ERROR_SUCCESS == CRegKey().Open(HKEY_CLASSES_ROOT, rootkey1 + clsid, KEY_READ)
           || ERROR_SUCCESS == CRegKey().Open(HKEY_CLASSES_ROOT, rootkey2 + clsid, KEY_READ);
}

bool IsCLSIDRegistered(const CLSID& clsid)
{
    bool fRet = false;

    LPOLESTR pStr = nullptr;
    if (S_OK == StringFromCLSID(clsid, &pStr) && pStr) {
        fRet = IsCLSIDRegistered(CString(pStr));
        CoTaskMemFree(pStr);
    }

    return fRet;
}

CString GetFilterPath(LPCTSTR clsid)
{
    CString rootkey1(_T("CLSID\\"));
    CString rootkey2(_T("CLSID\\{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance\\"));

    CRegKey key;
    CString path;

    if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, rootkey1 + clsid + _T("\\InprocServer32"), KEY_READ)
            || ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, rootkey2 + clsid + _T("\\InprocServer32"), KEY_READ)) {
        ULONG nCount = MAX_PATH;
        key.QueryStringValue(nullptr, path.GetBuffer(nCount), &nCount);
        path.ReleaseBuffer(nCount);
    }

    return path;
}

CString GetFilterPath(const CLSID& clsid)
{
    CString path;

    LPOLESTR pStr = nullptr;
    if (S_OK == StringFromCLSID(clsid, &pStr) && pStr) {
        path = GetFilterPath(CString(pStr));
        CoTaskMemFree(pStr);
    }

    return path;
}

void CStringToBin(CString str, CAtlArray<BYTE>& data)
{
    str.Trim();
    ASSERT((str.GetLength() & 1) == 0);
    data.SetCount(str.GetLength() / 2);

    BYTE b = 0;

    str.MakeUpper();
    for (int i = 0, j = str.GetLength(); i < j; i++) {
        TCHAR c = str[i];
        if (c >= _T('0') && c <= _T('9')) {
            if (!(i & 1)) {
                b = ((char(c - _T('0')) << 4) & 0xf0) | (b & 0x0f);
            } else {
                b = (char(c - _T('0')) & 0x0f) | (b & 0xf0);
            }
        } else if (c >= _T('A') && c <= _T('F')) {
            if (!(i & 1)) {
                b = ((char(c - _T('A') + 10) << 4) & 0xf0) | (b & 0x0f);
            } else {
                b = (char(c - _T('A') + 10) & 0x0f) | (b & 0xf0);
            }
        } else {
            break;
        }

        if (i & 1) {
            data[i >> 1] = b;
            b = 0;
        }
    }
}

CString BinToCString(const BYTE* ptr, size_t len)
{
    CString ret;

    while (len-- > 0) {
        TCHAR high, low;

        high = (*ptr >> 4) >= 10 ? (*ptr >> 4) - 10 + _T('A') : (*ptr >> 4) + _T('0');
        low = (*ptr & 0xf) >= 10 ? (*ptr & 0xf) - 10 + _T('A') : (*ptr & 0xf) + _T('0');

        ret.AppendFormat(_T("%c%c"), high, low);

        ptr++;
    }

    return ret;
}

void FindFiles(CString fn, CAtlList<CString>& files)
{
    CString path = fn;
    path.Replace('/', '\\');
    path = path.Left(path.ReverseFind('\\') + 1);

    WIN32_FIND_DATA findData;
    HANDLE h = FindFirstFile(fn, &findData);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            files.AddTail(path + findData.cFileName);
        } while (FindNextFile(h, &findData));

        FindClose(h);
    }
}

OpticalDiskType_t GetOpticalDiskType(TCHAR drive, CAtlList<CString>& files)
{
    files.RemoveAll();

    CString path;
    path.Format(_T("%c:"), drive);

    if (GetDriveType(path + _T("\\")) == DRIVE_CDROM) {
        // CDROM_DVDVideo
        FindFiles(path + _T("\\VIDEO_TS\\video_ts.ifo"), files);
        if (!files.IsEmpty()) {
            return OpticalDisk_DVDVideo;
        }

        // CDROM_BD
        FindFiles(path + _T("\\BDMV\\index.bdmv"), files);
        if (!files.IsEmpty()) {
            return OpticalDisk_BD;
        }

        // CDROM_VideoCD
        FindFiles(path + _T("\\mpegav\\avseq??.dat"), files);
        FindFiles(path + _T("\\mpegav\\avseq??.mpg"), files);
        FindFiles(path + _T("\\mpeg2\\avseq??.dat"), files);
        FindFiles(path + _T("\\mpeg2\\avseq??.mpg"), files);
        FindFiles(path + _T("\\mpegav\\music??.dat"), files);
        FindFiles(path + _T("\\mpegav\\music??.mpg"), files);
        FindFiles(path + _T("\\mpeg2\\music??.dat"), files);
        FindFiles(path + _T("\\mpeg2\\music??.mpg"), files);
        if (!files.IsEmpty()) {
            return OpticalDisk_VideoCD;
        }

        // CDROM_Audio
        HANDLE hDrive = CreateFile(CString(_T("\\\\.\\")) + path, GENERIC_READ, FILE_SHARE_READ, nullptr,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)nullptr);
        if (hDrive != INVALID_HANDLE_VALUE) {
            DWORD BytesReturned;
            CDROM_TOC TOC;
            if (DeviceIoControl(hDrive, IOCTL_CDROM_READ_TOC, nullptr, 0, &TOC, sizeof(TOC), &BytesReturned, 0)) {
                ASSERT(TOC.FirstTrack >= 1u && TOC.LastTrack <= _countof(TOC.TrackData));
                TOC.FirstTrack = std::max(TOC.FirstTrack, UCHAR(1));
                TOC.LastTrack = std::min(TOC.LastTrack, UCHAR(_countof(TOC.TrackData)));
                for (ptrdiff_t i = TOC.FirstTrack; i <= TOC.LastTrack; i++) {
                    // MMC-3 Draft Revision 10g: Table 222 - Q Sub-channel control field
                    auto& trackData = TOC.TrackData[i - 1];
                    trackData.Control &= 5;
                    if (trackData.Control == 0 || trackData.Control == 1) {
                        CString fn;
                        fn.Format(_T("%s\\track%02Id.cda"), path, i);
                        files.AddTail(fn);
                    }
                }
            }

            CloseHandle(hDrive);
        }
        if (!files.IsEmpty()) {
            return OpticalDisk_Audio;
        }

        // it is a cdrom but nothing special
        return OpticalDisk_Unknown;
    }

    return OpticalDisk_NotFound;
}

CString GetDriveLabel(TCHAR drive)
{
    CString path;
    path.Format(_T("%c:\\"), drive);

    return GetDriveLabel(CPath(path));
}

CString GetDriveLabel(CPath path)
{
    CString label;
    path.StripToRoot();

    TCHAR VolumeNameBuffer[MAX_PATH], FileSystemNameBuffer[MAX_PATH];
    DWORD VolumeSerialNumber, MaximumComponentLength, FileSystemFlags;
    if (GetVolumeInformation(path,
                             VolumeNameBuffer, MAX_PATH, &VolumeSerialNumber, &MaximumComponentLength,
                             &FileSystemFlags, FileSystemNameBuffer, MAX_PATH)) {
        label = VolumeNameBuffer;
    }

    return label;
}

bool GetKeyFrames(CString fn, CUIntArray& kfs)
{
    kfs.RemoveAll();

    CString fn2 = CString(fn).MakeLower();
    if (fn2.Mid(fn2.ReverseFind('.') + 1) == _T("avi")) {
        AVIFileInit();

        PAVIFILE pfile;
        if (AVIFileOpen(&pfile, fn, OF_SHARE_DENY_WRITE, 0L) == 0) {
            AVIFILEINFO afi;
            ZeroMemory(&afi, sizeof(afi));
            AVIFileInfo(pfile, &afi, sizeof(AVIFILEINFO));

            CComPtr<IAVIStream> pavi;
            if (AVIFileGetStream(pfile, &pavi, streamtypeVIDEO, 0) == AVIERR_OK) {
                AVISTREAMINFO si;
                AVIStreamInfo(pavi, &si, sizeof(si));

                if (afi.dwCaps & AVIFILECAPS_ALLKEYFRAMES) {
                    kfs.SetSize(si.dwLength);
                    for (DWORD kf = 0; kf < si.dwLength; kf++) {
                        kfs[kf] = kf;
                    }
                } else {
                    for (LONG kf = 0; ; kf++) {
                        kf = pavi->FindSample(kf, FIND_KEY | FIND_NEXT);
                        if (kf < 0 || !kfs.IsEmpty() && kfs[kfs.GetCount() - 1] >= (UINT)kf) {
                            break;
                        }
                        kfs.Add(kf);
                    }

                    if (!kfs.IsEmpty() && kfs[kfs.GetCount() - 1] < si.dwLength - 1) {
                        kfs.Add(si.dwLength - 1);
                    }
                }
            }

            AVIFileRelease(pfile);
        }

        AVIFileExit();
    }

    return !kfs.IsEmpty();
}

DVD_HMSF_TIMECODE RT2HMSF(REFERENCE_TIME rt, double fps /*= 0.0*/) // use to remember the current position
{
    DVD_HMSF_TIMECODE hmsf = {
        (BYTE)((rt / 10000000 / 60 / 60)),
        (BYTE)((rt / 10000000 / 60) % 60),
        (BYTE)((rt / 10000000) % 60),
        (BYTE)(1.0 * ((rt / 10000) % 1000) * fps / 1000)
    };

    return hmsf;
}

DVD_HMSF_TIMECODE RT2HMS_r(REFERENCE_TIME rt) // used only for information (for display on the screen)
{
    rt = (rt + 5000000) / 10000000;
    DVD_HMSF_TIMECODE hmsf = {
        (BYTE)(rt / 3600),
        (BYTE)(rt / 60 % 60),
        (BYTE)(rt % 60),
        0
    };

    return hmsf;
}

REFERENCE_TIME HMSF2RT(DVD_HMSF_TIMECODE hmsf, double fps /*= -1.0*/)
{
    if (fps <= 0.0) {
        hmsf.bFrames = 0;
        fps = 1.0;
    }
    return (REFERENCE_TIME)((((REFERENCE_TIME)hmsf.bHours * 60 + hmsf.bMinutes) * 60 + hmsf.bSeconds) * 1000 + 1.0 * hmsf.bFrames * 1000 / fps) * 10000;
}

void memsetd(void* dst, unsigned int c, size_t nbytes)
{
    size_t n = nbytes / 4;

#ifndef _WIN64
    if (!(g_cpuid.m_flags & g_cpuid.sse2)) { // No SSE2
        __stosd((unsigned long*)dst, c, n);
        return;
    }
#endif

    size_t o = n - (n % 4);

    __m128i val = _mm_set1_epi32((int)c);
    if (((uintptr_t)dst & 0x0F) == 0) { // 16-byte aligned
        for (size_t i = 0; i < o; i += 4) {
            _mm_store_si128((__m128i*) & (((DWORD*)dst)[i]), val);
        }
    } else {
        for (size_t i = 0; i < o; i += 4) {
            _mm_storeu_si128((__m128i*) & (((DWORD*)dst)[i]), val);
        }
    }

    switch (n - o) {
        case 3:
            ((DWORD*)dst)[o + 2] = c;
        case 2:
            ((DWORD*)dst)[o + 1] = c;
        case 1:
            ((DWORD*)dst)[o + 0] = c;
    }
}

void memsetw(void* dst, unsigned short c, size_t nbytes)
{
    memsetd(dst, c << 16 | c, nbytes);

    size_t n = nbytes / 2;
    size_t o = (n / 2) * 2;
    if ((n - o) == 1) {
        ((WORD*)dst)[o] = c;
    }
}

bool ExtractBIH(const AM_MEDIA_TYPE* pmt, BITMAPINFOHEADER* bih)
{
    if (pmt && bih) {
        ZeroMemory(bih, sizeof(*bih));

        if (pmt->formattype == FORMAT_VideoInfo) {
            VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->pbFormat;
            memcpy(bih, &vih->bmiHeader, sizeof(BITMAPINFOHEADER));
            return true;
        } else if (pmt->formattype == FORMAT_VideoInfo2) {
            VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)pmt->pbFormat;
            memcpy(bih, &vih->bmiHeader, sizeof(BITMAPINFOHEADER));
            return true;
        } else if (pmt->formattype == FORMAT_MPEGVideo) {
            VIDEOINFOHEADER* vih = &((MPEG1VIDEOINFO*)pmt->pbFormat)->hdr;
            memcpy(bih, &vih->bmiHeader, sizeof(BITMAPINFOHEADER));
            return true;
        } else if (pmt->formattype == FORMAT_MPEG2_VIDEO) {
            VIDEOINFOHEADER2* vih = &((MPEG2VIDEOINFO*)pmt->pbFormat)->hdr;
            memcpy(bih, &vih->bmiHeader, sizeof(BITMAPINFOHEADER));
            return true;
        } else if (pmt->formattype == FORMAT_DiracVideoInfo) {
            VIDEOINFOHEADER2* vih = &((DIRACINFOHEADER*)pmt->pbFormat)->hdr;
            memcpy(bih, &vih->bmiHeader, sizeof(BITMAPINFOHEADER));
            return true;
        }
    }

    return false;
}

bool ExtractAvgTimePerFrame(const AM_MEDIA_TYPE* pmt, REFERENCE_TIME& rtAvgTimePerFrame)
{
    if (pmt->formattype == FORMAT_VideoInfo) {
        rtAvgTimePerFrame = ((VIDEOINFOHEADER*)pmt->pbFormat)->AvgTimePerFrame;
    } else if (pmt->formattype == FORMAT_VideoInfo2) {
        rtAvgTimePerFrame = ((VIDEOINFOHEADER2*)pmt->pbFormat)->AvgTimePerFrame;
    } else if (pmt->formattype == FORMAT_MPEGVideo) {
        rtAvgTimePerFrame = ((MPEG1VIDEOINFO*)pmt->pbFormat)->hdr.AvgTimePerFrame;
    } else if (pmt->formattype == FORMAT_MPEG2Video) {
        rtAvgTimePerFrame = ((MPEG2VIDEOINFO*)pmt->pbFormat)->hdr.AvgTimePerFrame;
    } else {
        return false;
    }

    return true;
}

bool ExtractBIH(IMediaSample* pMS, BITMAPINFOHEADER* bih)
{
    AM_MEDIA_TYPE* pmt = nullptr;
    if (SUCCEEDED(pMS->GetMediaType(&pmt)) && pmt) {
        bool fRet = ExtractBIH(pmt, bih);
        DeleteMediaType(pmt);
        return fRet;
    }

    return false;
}

bool ExtractDim(const AM_MEDIA_TYPE* pmt, int& w, int& h, int& arx, int& ary)
{
    w = h = arx = ary = 0;

    if (pmt->formattype == FORMAT_VideoInfo || pmt->formattype == FORMAT_MPEGVideo) {
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->pbFormat;
        w = vih->bmiHeader.biWidth;
        h = abs(vih->bmiHeader.biHeight);
        arx = w * vih->bmiHeader.biYPelsPerMeter;
        ary = h * vih->bmiHeader.biXPelsPerMeter;
    } else if (pmt->formattype == FORMAT_VideoInfo2 || pmt->formattype == FORMAT_MPEG2_VIDEO || pmt->formattype == FORMAT_DiracVideoInfo) {
        VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)pmt->pbFormat;
        w = vih->bmiHeader.biWidth;
        h = abs(vih->bmiHeader.biHeight);
        arx = vih->dwPictAspectRatioX;
        ary = vih->dwPictAspectRatioY;
    } else {
        return false;
    }

    if (!arx || !ary) {
        BYTE* ptr = nullptr;
        DWORD len = 0;

        if (pmt->formattype == FORMAT_MPEGVideo) {
            ptr = ((MPEG1VIDEOINFO*)pmt->pbFormat)->bSequenceHeader;
            len = ((MPEG1VIDEOINFO*)pmt->pbFormat)->cbSequenceHeader;

            if (ptr && len >= 8 && *(DWORD*)ptr == 0xb3010000) {
                w = (ptr[4] << 4) | (ptr[5] >> 4);
                h = ((ptr[5] & 0xf) << 8) | ptr[6];
                float ar[] = {
                    1.0000f, 1.0000f, 0.6735f, 0.7031f,
                    0.7615f, 0.8055f, 0.8437f, 0.8935f,
                    0.9157f, 0.9815f, 1.0255f, 1.0695f,
                    1.0950f, 1.1575f, 1.2015f, 1.0000f,
                };
                arx = (int)((float)w / ar[ptr[7] >> 4] + 0.5);
                ary = h;
            }
        } else if (pmt->formattype == FORMAT_MPEG2_VIDEO) {
            ptr = (BYTE*)((MPEG2VIDEOINFO*)pmt->pbFormat)->dwSequenceHeader;
            len = ((MPEG2VIDEOINFO*)pmt->pbFormat)->cbSequenceHeader;

            if (ptr && len >= 8 && *(DWORD*)ptr == 0xb3010000) {
                w = (ptr[4] << 4) | (ptr[5] >> 4);
                h = ((ptr[5] & 0xf) << 8) | ptr[6];
                struct {
                    int x, y;
                } ar[] = {{w, h}, {4, 3}, {16, 9}, {221, 100}, {w, h}};
                int i = std::min(std::max(ptr[7] >> 4, 1), 5) - 1;
                arx = ar[i].x;
                ary = ar[i].y;
            }
        }
    }

    if (!arx || !ary) {
        arx = w;
        ary = h;
    }

    DWORD a = arx, b = ary;
    while (a) {
        int tmp = a;
        a = b % tmp;
        b = tmp;
    }
    if (b) {
        arx /= b, ary /= b;
    }

    return true;
}

bool CreateFilter(CStringW DisplayName, IBaseFilter** ppBF, CStringW& FriendlyName)
{
    if (!ppBF) {
        return false;
    }

    *ppBF = nullptr;
    FriendlyName.Empty();

    CComPtr<IBindCtx> pBindCtx;
    CreateBindCtx(0, &pBindCtx);

    CComPtr<IMoniker> pMoniker;
    ULONG chEaten;
    if (S_OK != MkParseDisplayName(pBindCtx, CComBSTR(DisplayName), &chEaten, &pMoniker)) {
        return false;
    }

    if (FAILED(pMoniker->BindToObject(pBindCtx, 0, IID_PPV_ARGS(ppBF))) || !*ppBF) {
        return false;
    }

    CComPtr<IPropertyBag> pPB;
    CComVariant var;
    if (SUCCEEDED(pMoniker->BindToStorage(pBindCtx, 0, IID_PPV_ARGS(&pPB)))
            && SUCCEEDED(pPB->Read(CComBSTR(_T("FriendlyName")), &var, nullptr))) {
        FriendlyName = var.bstrVal;
    }

    return true;
}

IBaseFilter* AppendFilter(IPin* pPin, IMoniker* pMoniker, IGraphBuilder* pGB)
{
    do {
        if (!pPin || !pMoniker || !pGB) {
            break;
        }

        CComPtr<IPin> pPinTo;
        PIN_DIRECTION dir;
        if (FAILED(pPin->QueryDirection(&dir)) || dir != PINDIR_OUTPUT || SUCCEEDED(pPin->ConnectedTo(&pPinTo))) {
            break;
        }

        CComPtr<IBindCtx> pBindCtx;
        CreateBindCtx(0, &pBindCtx);

        CComPtr<IPropertyBag> pPB;
        if (FAILED(pMoniker->BindToStorage(pBindCtx, 0, IID_PPV_ARGS(&pPB)))) {
            break;
        }

        CComVariant var;
        if (FAILED(pPB->Read(CComBSTR(_T("FriendlyName")), &var, nullptr))) {
            break;
        }

        CComPtr<IBaseFilter> pBF;
        if (FAILED(pMoniker->BindToObject(pBindCtx, 0, IID_PPV_ARGS(&pBF))) || !pBF) {
            break;
        }

        if (FAILED(pGB->AddFilter(pBF, CStringW(var.bstrVal)))) {
            break;
        }

        BeginEnumPins(pBF, pEP, pPinTo) {
            PIN_DIRECTION dir2;
            if (FAILED(pPinTo->QueryDirection(&dir2)) || dir2 != PINDIR_INPUT) {
                continue;
            }

            if (SUCCEEDED(pGB->ConnectDirect(pPin, pPinTo, nullptr))) {
                return pBF;
            }
        }
        EndEnumFilters;

        pGB->RemoveFilter(pBF);
    } while (false);

    return nullptr;
}

CStringW GetFriendlyName(CStringW displayName)
{
    CStringW friendlyName;

    CComPtr<IBindCtx> pBindCtx;
    CreateBindCtx(0, &pBindCtx);

    CComPtr<IMoniker> pMoniker;
    ULONG chEaten;
    if (S_OK == MkParseDisplayName(pBindCtx, CComBSTR(displayName), &chEaten, &pMoniker)) {
        CComPtr<IPropertyBag> pPB;
        CComVariant var;
        if (SUCCEEDED(pMoniker->BindToStorage(pBindCtx, 0, IID_PPV_ARGS(&pPB)))
                && SUCCEEDED(pPB->Read(CComBSTR(_T("FriendlyName")), &var, nullptr))) {
            friendlyName = var.bstrVal;
        }
    }

    return friendlyName;
}

typedef HRESULT(__stdcall* fDllCanUnloadNow)(void);

struct ExternalObject {
    CString path;
    HINSTANCE hInst;
    CLSID clsid;
    fDllCanUnloadNow fpDllCanUnloadNow;
    bool bUnloadOnNextCheck;
};

static CAtlList<ExternalObject> s_extObjs;
static CCritSec s_csExtObjs;

HRESULT LoadExternalObject(LPCTSTR path, REFCLSID clsid, REFIID iid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    CAutoLock lock(&s_csExtObjs);

    CString fullpath = MakeFullPath(path);

    HINSTANCE hInst = nullptr;
    bool fFound = false;

    POSITION pos = s_extObjs.GetHeadPosition();
    while (pos) {
        ExternalObject& eo = s_extObjs.GetNext(pos);
        if (!eo.path.CompareNoCase(fullpath)) {
            hInst = eo.hInst;
            fFound = true;
            eo.bUnloadOnNextCheck = false;
            break;
        }
    }

    HRESULT hr = E_FAIL;

    if (!hInst) {
        hInst = LoadLibraryEx(fullpath, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    }
    if (hInst) {
        typedef HRESULT(__stdcall * PDllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID * ppv);
        PDllGetClassObject p = (PDllGetClassObject)GetProcAddress(hInst, "DllGetClassObject");

        if (p && FAILED(hr = p(clsid, iid, ppv))) {
            CComPtr<IClassFactory> pCF;
            if (SUCCEEDED(hr = p(clsid, IID_PPV_ARGS(&pCF)))) {
                hr = pCF->CreateInstance(nullptr, iid, ppv);
            }
        }
    }

    if (FAILED(hr) && hInst && !fFound) {
        FreeLibrary(hInst);
        return hr;
    }

    if (hInst && !fFound) {
        ExternalObject eo;
        eo.path = fullpath;
        eo.hInst = hInst;
        eo.clsid = clsid;
        eo.fpDllCanUnloadNow = (fDllCanUnloadNow)GetProcAddress(hInst, "DllCanUnloadNow");
        eo.bUnloadOnNextCheck = false;
        s_extObjs.AddTail(eo);
    }

    return hr;
}

HRESULT LoadExternalFilter(LPCTSTR path, REFCLSID clsid, IBaseFilter** ppBF)
{
    return LoadExternalObject(path, clsid, IID_PPV_ARGS(ppBF));
}

HRESULT LoadExternalPropertyPage(IPersist* pP, REFCLSID clsid, IPropertyPage** ppPP)
{
    CAutoLock lock(&s_csExtObjs);

    CLSID clsid2 = GUID_NULL;
    if (FAILED(pP->GetClassID(&clsid2))) {
        return E_FAIL;
    }

    POSITION pos = s_extObjs.GetHeadPosition();
    while (pos) {
        ExternalObject& eo = s_extObjs.GetNext(pos);
        if (eo.clsid == clsid2) {
            return LoadExternalObject(eo.path, clsid, IID_PPV_ARGS(ppPP));
        }
    }

    return E_FAIL;
}

bool UnloadUnusedExternalObjects()
{
    CAutoLock lock(&s_csExtObjs);

    POSITION pos = s_extObjs.GetHeadPosition(), currentPos;
    while (pos) {
        currentPos = pos;
        ExternalObject& eo = s_extObjs.GetNext(pos);

        if (eo.fpDllCanUnloadNow && eo.fpDllCanUnloadNow() == S_OK) {
            // Before actually unloading it, we require that the library reports
            // that it can be unloaded safely twice in a row with a 60s delay
            // between the two checks.
            if (eo.bUnloadOnNextCheck) {
                FreeLibrary(eo.hInst);
                s_extObjs.RemoveAt(currentPos);
            } else {
                eo.bUnloadOnNextCheck = true;
            }
        } else {
            eo.bUnloadOnNextCheck = false;
        }
    }

    return s_extObjs.IsEmpty();
}

CString MakeFullPath(LPCTSTR path)
{
    CString full(path);
    full.Replace('/', '\\');

    CString fn;
    fn.ReleaseBuffer(GetModuleFileName(AfxGetInstanceHandle(), fn.GetBuffer(MAX_PATH), MAX_PATH));
    CPath p(fn);

    if (full.GetLength() >= 2 && full[0] == '\\') {
        if (full[1] != '\\') {
            p.StripToRoot();
            full = CString(p) + full.Mid(1);
        }
    } else if (full.Find(_T(":\\")) < 0) {
        p.RemoveFileSpec();
        p.AddBackslash();
        full = CString(p) + full;
    }

    CPath c(full);
    c.Canonicalize();
    return CString(c);
}

//

CString GetMediaTypeName(const GUID& guid)
{
    CString ret = guid == GUID_NULL
                  ? _T("Any type")
                  : CString(GuidNames[guid]);

    if (ret == _T("FOURCC GUID")) {
        CString str;
        if (guid.Data1 >= 0x10000) {
            str.Format(_T("Video: %c%c%c%c"), (guid.Data1 >> 0) & 0xff, (guid.Data1 >> 8) & 0xff, (guid.Data1 >> 16) & 0xff, (guid.Data1 >> 24) & 0xff);
        } else {
            str.Format(_T("Audio: 0x%08x"), guid.Data1);
        }
        ret = str;
    } else if (ret == _T("Unknown GUID Name")) {
        WCHAR null[128] = {0}, buff[128];
        StringFromGUID2(GUID_NULL, null, _countof(null) - 1);
        ret = CString(CStringW(StringFromGUID2(guid, buff, _countof(buff) - 1) ? buff : null));
    }

    return ret;
}

GUID GUIDFromCString(CString str)
{
    GUID guid = GUID_NULL;
    HRESULT hr = CLSIDFromString(CComBSTR(str), &guid);
    ASSERT(SUCCEEDED(hr));
    UNREFERENCED_PARAMETER(hr);
    return guid;
}

HRESULT GUIDFromCString(CString str, GUID& guid)
{
    guid = GUID_NULL;
    return CLSIDFromString(CComBSTR(str), &guid);
}

CString CStringFromGUID(const GUID& guid)
{
    WCHAR null[128] = {0}, buff[128];
    StringFromGUID2(GUID_NULL, null, _countof(null) - 1);
    return CString(StringFromGUID2(guid, buff, _countof(buff) - 1) > 0 ? buff : null);
}

CStringW UTF8To16(LPCSTR utf8)
{
    CStringW str;
    int n = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0) - 1;
    if (n < 0) {
        return str;
    }
    str.ReleaseBuffer(MultiByteToWideChar(CP_UTF8, 0, utf8, -1, str.GetBuffer(n), n + 1) - 1);
    return str;
}

CStringA UTF16To8(LPCWSTR utf16)
{
    CStringA str;
    int n = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, nullptr, 0, nullptr, nullptr) - 1;
    if (n < 0) {
        return str;
    }
    str.ReleaseBuffer(WideCharToMultiByte(CP_UTF8, 0, utf16, -1, str.GetBuffer(n), n + 1, nullptr, nullptr) - 1);
    return str;
}

CStringW UTF8ToStringW(const char* S)
{
    CStringW str;
    if (S == nullptr) {
        return str;
    }

    // Don't use MultiByteToWideChar(), some characters are not well decoded
    const unsigned char* Z = (const unsigned char*)S;
    while (*Z) { //0 is end
        //1 byte
        if (*Z < 0x80) {
            str += (wchar_t)(*Z);
            Z++;
        }
        //2 bytes
        else if ((*Z & 0xE0) == 0xC0) {
            if ((*(Z + 1) & 0xC0) == 0x80) {
                str += (wchar_t)((((wchar_t)(*Z & 0x1F)) << 6) | (*(Z + 1) & 0x3F));
                Z += 2;
            } else {
                str.Empty();
                return str; //Bad character
            }
        }
        //3 bytes
        else if ((*Z & 0xF0) == 0xE0) {
            if ((*(Z + 1) & 0xC0) == 0x80 && (*(Z + 2) & 0xC0) == 0x80) {
                str += (wchar_t)((((wchar_t)(*Z & 0x0F)) << 12) | ((*(Z + 1) & 0x3F) << 6) | (*(Z + 2) & 0x3F));
                Z += 3;
            } else {
                str.Empty();
                return str; //Bad character
            }
        }
        //4 bytes
        else if ((*Z & 0xF8) == 0xF0) {
            if ((*(Z + 1) & 0xC0) == 0x80 && (*(Z + 2) & 0xC0) == 0x80 && (*(Z + 3) & 0xC0) == 0x80) {
                str += (wchar_t)((((wchar_t)(*Z & 0x0F)) << 18) | ((*(Z + 1) & 0x3F) << 12) || ((*(Z + 2) & 0x3F) << 6) | (*(Z + 3) & 0x3F));
                Z += 4;
            } else {
                str.Empty();
                return str; //Bad character
            }
        } else {
            str.Empty();
            return str; //Bad character
        }
    }
    return str;
}

CStringW LocalToStringW(const char* S)
{
    CStringW str;
    if (S == nullptr) {
        return str;
    }

    int Size = MultiByteToWideChar(CP_ACP, 0, S, -1, nullptr, 0);
    if (Size != 0) {
        str.ReleaseBuffer(MultiByteToWideChar(CP_ACP, 0, S, -1, str.GetBuffer(Size), Size + 1) - 1);
    }
    return str;
}

static struct {
    LPCSTR name, iso6392, iso6391;
    LCID lcid;
} s_isolangs[] = {  // TODO : fill LCID !!!
    {"Abkhazian", "abk", "ab"},
    {"Achinese", "ace", ""},
    {"Acoli", "ach", ""},
    {"Adangme", "ada", ""},
    {"Afar", "aar", "aa"},
    {"Afrihili", "afh", ""},
    {"Afrikaans", "afr", "af",                  MAKELCID(MAKELANGID(LANG_AFRIKAANS, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Afro-Asiatic (Other)", "afa", ""},
    {"Akan", "aka", "ak"},
    {"Akkadian", "akk", ""},
    {"Albanian", "alb", "sq"},
    {"Albanian", "sqi", "sq",                   MAKELCID(MAKELANGID(LANG_ALBANIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Aleut", "ale", ""},
    {"Algonquian languages", "alg", ""},
    {"Altaic (Other)", "tut", ""},
    {"Amharic", "amh", "am"},
    {"Apache languages", "apa", ""},
    {"Arabic", "ara", "ar",                     MAKELCID(MAKELANGID(LANG_ARABIC, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Aragonese", "arg", "an"},
    {"Aramaic", "arc", ""},
    {"Arapaho", "arp", ""},
    {"Araucanian", "arn", ""},
    {"Arawak", "arw", ""},
    {"Armenian", "arm", "hy",                   MAKELCID(MAKELANGID(LANG_ARMENIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Armenian", "hye", "hy",                   MAKELCID(MAKELANGID(LANG_ARMENIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Artificial (Other)", "art", ""},
    {"Assamese", "asm", "as",                   MAKELCID(MAKELANGID(LANG_ASSAMESE, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Asturian; Bable", "ast", ""},
    {"Athapascan languages", "ath", ""},
    {"Australian languages", "aus", ""},
    {"Austronesian (Other)", "map", ""},
    {"Avaric", "ava", "av"},
    {"Avestan", "ave", "ae"},
    {"Awadhi", "awa", ""},
    {"Aymara", "aym", "ay"},
    {"Azerbaijani", "aze", "az",                MAKELCID(MAKELANGID(LANG_AZERI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Bable; Asturian", "ast", ""},
    {"Balinese", "ban", ""},
    {"Baltic (Other)", "bat", ""},
    {"Baluchi", "bal", ""},
    {"Bambara", "bam", "bm"},
    {"Bamileke languages", "bai", ""},
    {"Banda", "bad", ""},
    {"Bantu (Other)", "bnt", ""},
    {"Basa", "bas", ""},
    {"Bashkir", "bak", "ba",                    MAKELCID(MAKELANGID(LANG_BASHKIR, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Basque", "baq", "eu",                     MAKELCID(MAKELANGID(LANG_BASQUE, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Basque", "eus", "eu",                     MAKELCID(MAKELANGID(LANG_BASQUE, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Batak (Indonesia)", "btk", ""},
    {"Beja", "bej", ""},
    {"Belarusian", "bel", "be",                 MAKELCID(MAKELANGID(LANG_BELARUSIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Bemba", "bem", ""},
    {"Bengali", "ben", "bn",                    MAKELCID(MAKELANGID(LANG_BENGALI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Berber (Other)", "ber", ""},
    {"Bhojpuri", "bho", ""},
    {"Bihari", "bih", "bh"},
    {"Bikol", "bik", ""},
    {"Bini", "bin", ""},
    {"Bislama", "bis", "bi"},
    {"Bokmål, Norwegian; Norwegian Bokmål", "nob", "nb"},
    {"Bosnian", "bos", "bs"},
    {"Braj", "bra", ""},
    {"Brazilian", "pob", "pb"},                 // unofficial codes for Brazilian Portuguese language
    {"Breton", "bre", "br",                     MAKELCID(MAKELANGID(LANG_BRETON, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Buginese", "bug", ""},
    {"Bulgarian", "bul", "bg",                  MAKELCID(MAKELANGID(LANG_BULGARIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Buriat", "bua", ""},
    {"Burmese", "bur", "my"},
    {"Burmese", "mya", "my"},
    {"Caddo", "cad", ""},
    {"Carib", "car", ""},
    {"Spanish; Castilian", "spa", "es",         MAKELCID(MAKELANGID(LANG_SPANISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Catalan", "cat", "ca",                    MAKELCID(MAKELANGID(LANG_CATALAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Caucasian (Other)", "cau", ""},
    {"Cebuano", "ceb", ""},
    {"Celtic (Other)", "cel", ""},
    {"Central American Indian (Other)", "cai", ""},
    {"Chagatai", "chg", ""},
    {"Chamic languages", "cmc", ""},
    {"Chamorro", "cha", "ch"},
    {"Chechen", "che", "ce"},
    {"Cherokee", "chr", ""},
    {"Chewa; Chichewa; Nyanja", "nya", "ny"},
    {"Cheyenne", "chy", ""},
    {"Chibcha", "chb", ""},
    {"Chichewa; Chewa; Nyanja", "nya", "ny"},
    {"Chinese", "chi", "zh",                    MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Chinese", "zho", "zh"},
    {"Chinook jargon", "chn", ""},
    {"Chipewyan", "chp", ""},
    {"Choctaw", "cho", ""},
    {"Chuang; Zhuang", "zha", "za"},
    {"Church Slavic; Old Church Slavonic", "chu", "cu"},
    {"Old Church Slavonic; Old Slavonic; ", "chu", "cu"},
    {"Church Slavonic; Old Bulgarian; Church Slavic;", "chu", "cu"},
    {"Old Slavonic; Church Slavonic; Old Bulgarian;", "chu", "cu"},
    {"Church Slavic; Old Church Slavonic", "chu", "cu"},
    {"Chuukese", "chk", ""},
    {"Chuvash", "chv", "cv"},
    {"Coptic", "cop", ""},
    {"Cornish", "cor", "kw"},
    {"Corsican", "cos", "co",                   MAKELCID(MAKELANGID(LANG_CORSICAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Cree", "cre", "cr"},
    {"Creek", "mus", ""},
    {"Creoles and pidgins (Other)", "crp", ""},
    {"Creoles and pidgins,", "cpe", ""},
    //   {"English-based (Other)", "", ""},
    {"Creoles and pidgins,", "cpf", ""},
    //   {"French-based (Other)", "", ""},
    {"Creoles and pidgins,", "cpp", ""},
    //   {"Portuguese-based (Other)", "", ""},
    {"Croatian", "scr", "hr",                   MAKELCID(MAKELANGID(LANG_CROATIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Croatian", "hrv", "hr",                   MAKELCID(MAKELANGID(LANG_CROATIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Cushitic (Other)", "cus", ""},
    {"Czech", "cze", "cs",                      MAKELCID(MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Czech", "ces", "cs",                      MAKELCID(MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Dakota", "dak", ""},
    {"Danish", "dan", "da",                     MAKELCID(MAKELANGID(LANG_DANISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Dargwa", "dar", ""},
    {"Dayak", "day", ""},
    {"Delaware", "del", ""},
    {"Dinka", "din", ""},
    {"Divehi", "div", "dv",                     MAKELCID(MAKELANGID(LANG_DIVEHI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Dogri", "doi", ""},
    {"Dogrib", "dgr", ""},
    {"Dravidian (Other)", "dra", ""},
    {"Duala", "dua", ""},
    {"Dutch; Flemish", "dut", "nl",             MAKELCID(MAKELANGID(LANG_DUTCH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Dutch; Flemish", "nld", "nl",             MAKELCID(MAKELANGID(LANG_DUTCH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Dutch, Middle (ca. 1050-1350)", "dum", ""},
    {"Dyula", "dyu", ""},
    {"Dzongkha", "dzo", "dz"},
    {"Efik", "efi", ""},
    {"Egyptian (Ancient)", "egy", ""},
    {"Ekajuk", "eka", ""},
    {"Elamite", "elx", ""},
    {"English", "eng", "en",                    MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"English, Middle (1100-1500)", "enm", ""},
    {"English, Old (ca.450-1100)", "ang", ""},
    {"Esperanto", "epo", "eo"},
    {"Estonian", "est", "et",                   MAKELCID(MAKELANGID(LANG_ESTONIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Ewe", "ewe", "ee"},
    {"Ewondo", "ewo", ""},
    {"Fang", "fan", ""},
    {"Fanti", "fat", ""},
    {"Faroese", "fao", "fo",                    MAKELCID(MAKELANGID(LANG_FAEROESE, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Fijian", "fij", "fj"},
    {"Finnish", "fin", "fi",                    MAKELCID(MAKELANGID(LANG_FINNISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Finno-Ugrian (Other)", "fiu", ""},
    {"Flemish; Dutch", "dut", "nl"},
    {"Flemish; Dutch", "nld", "nl"},
    {"Fon", "fon", ""},
    {"French", "fre", "fr",                     MAKELCID(MAKELANGID(LANG_FRENCH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"French", "fra*", "fr",                    MAKELCID(MAKELANGID(LANG_FRENCH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"French", "fra", "fr",                     MAKELCID(MAKELANGID(LANG_FRENCH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"French, Middle (ca.1400-1600)", "frm", ""},
    {"French, Old (842-ca.1400)", "fro", ""},
    {"Frisian", "fry", "fy",                    MAKELCID(MAKELANGID(LANG_FRISIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Friulian", "fur", ""},
    {"Fulah", "ful", "ff"},
    {"Ga", "gaa", ""},
    {"Gaelic; Scottish Gaelic", "gla", "gd",    MAKELCID(MAKELANGID(LANG_GALICIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Gallegan", "glg", "gl"},
    {"Ganda", "lug", "lg"},
    {"Gayo", "gay", ""},
    {"Gbaya", "gba", ""},
    {"Geez", "gez", ""},
    {"Georgian", "geo", "ka",                   MAKELCID(MAKELANGID(LANG_GEORGIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Georgian", "kat", "ka",                   MAKELCID(MAKELANGID(LANG_GEORGIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"German", "ger", "de",                     MAKELCID(MAKELANGID(LANG_GERMAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"German", "deu", "de",                     MAKELCID(MAKELANGID(LANG_GERMAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"German, Low; Saxon, Low; Low German; Low Saxon", "nds", ""},
    {"German, Middle High (ca.1050-1500)", "gmh", ""},
    {"German, Old High (ca.750-1050)", "goh", ""},
    {"Germanic (Other)", "gem", ""},
    {"Gikuyu; Kikuyu", "kik", "ki"},
    {"Gilbertese", "gil", ""},
    {"Gondi", "gon", ""},
    {"Gorontalo", "gor", ""},
    {"Gothic", "got", ""},
    {"Grebo", "grb", ""},
    {"Greek", "ell", "el",                      MAKELCID(MAKELANGID(LANG_GREEK, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Greek", "gre", "el",                      MAKELCID(MAKELANGID(LANG_GREEK, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Greek, Ancient (to 1453)", "grc", "",     MAKELCID(MAKELANGID(LANG_GREEK, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Greenlandic; Kalaallisut", "kal", "kl",   MAKELCID(MAKELANGID(LANG_GREENLANDIC, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Guarani", "grn", "gn"},
    {"Gujarati", "guj", "gu",                   MAKELCID(MAKELANGID(LANG_GUJARATI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Gwich’in", "gwi", ""},
    {"Haida", "hai", ""},
    {"Hausa", "hau", "ha",                      MAKELCID(MAKELANGID(LANG_HAUSA, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Hawaiian", "haw", ""},
    {"Hebrew", "heb", "he",                     MAKELCID(MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Herero", "her", "hz"},
    {"Hiligaynon", "hil", ""},
    {"Himachali", "him", ""},
    {"Hindi", "hin", "hi",                      MAKELCID(MAKELANGID(LANG_HINDI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Hiri Motu", "hmo", "ho"},
    {"Hittite", "hit", ""},
    {"Hmong", "hmn", ""},
    {"Hungarian", "hun", "hu",                  MAKELCID(MAKELANGID(LANG_HUNGARIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Hupa", "hup", ""},
    {"Iban", "iba", ""},
    {"Icelandic", "ice", "is",                  MAKELCID(MAKELANGID(LANG_ICELANDIC, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Icelandic", "isl", "is",                  MAKELCID(MAKELANGID(LANG_ICELANDIC, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Ido", "ido", "io"},
    {"Igbo", "ibo", "ig",                       MAKELCID(MAKELANGID(LANG_IGBO, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Ijo", "ijo", ""},
    {"Iloko", "ilo", ""},
    {"Inari Sami", "smn", ""},
    {"Indic (Other)", "inc", ""},
    {"Indo-European (Other)", "ine", ""},
    {"Indonesian", "ind", "id",                 MAKELCID(MAKELANGID(LANG_INDONESIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Ingush", "inh", ""},
    {"Interlingua (International", "ina", "ia"},
    //   {"Auxiliary Language Association)", "", ""},
    {"Interlingue", "ile", "ie"},
    {"Inuktitut", "iku", "iu",                  MAKELCID(MAKELANGID(LANG_INUKTITUT, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Inupiaq", "ipk", "ik"},
    {"Iranian (Other)", "ira", ""},
    {"Irish", "gle", "ga",                      MAKELCID(MAKELANGID(LANG_IRISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Irish, Middle (900-1200)", "mga", "",     MAKELCID(MAKELANGID(LANG_IRISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Irish, Old (to 900)", "sga", "",          MAKELCID(MAKELANGID(LANG_IRISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Iroquoian languages", "iro", ""},
    {"Italian", "ita", "it",                    MAKELCID(MAKELANGID(LANG_ITALIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Japanese", "jpn", "ja",                   MAKELCID(MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Javanese", "jav", "jv"},
    {"Judeo-Arabic", "jrb", ""},
    {"Judeo-Persian", "jpr", ""},
    {"Kabardian", "kbd", ""},
    {"Kabyle", "kab", ""},
    {"Kachin", "kac", ""},
    {"Kalaallisut; Greenlandic", "kal", "kl"},
    {"Kamba", "kam", ""},
    {"Kannada", "kan", "kn",                    MAKELCID(MAKELANGID(LANG_KANNADA, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Kanuri", "kau", "kr"},
    {"Kara-Kalpak", "kaa", ""},
    {"Karen", "kar", ""},
    {"Kashmiri", "kas", "ks",                   MAKELCID(MAKELANGID(LANG_KASHMIRI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Kawi", "kaw", ""},
    {"Kazakh", "kaz", "kk",                     MAKELCID(MAKELANGID(LANG_KAZAK, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Khasi", "kha", ""},
    {"Khmer", "khm", "km",                      MAKELCID(MAKELANGID(LANG_KHMER, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Khoisan (Other)", "khi", ""},
    {"Khotanese", "kho", ""},
    {"Kikuyu; Gikuyu", "kik", "ki"},
    {"Kimbundu", "kmb", ""},
    {"Kinyarwanda", "kin", "rw",                MAKELCID(MAKELANGID(LANG_KINYARWANDA, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Kirghiz", "kir", "ky"},
    {"Komi", "kom", "kv"},
    {"Kongo", "kon", "kg"},
    {"Konkani", "kok", "",                      MAKELCID(MAKELANGID(LANG_KONKANI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Korean", "kor", "ko",                     MAKELCID(MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Kosraean", "kos", ""},
    {"Kpelle", "kpe", ""},
    {"Kru", "kro", ""},
    {"Kuanyama; Kwanyama", "kua", "kj"},
    {"Kumyk", "kum", ""},
    {"Kurdish", "kur", "ku"},
    {"Kurukh", "kru", ""},
    {"Kutenai", "kut", ""},
    {"Kwanyama, Kuanyama", "kua", "kj"},
    {"Ladino", "lad", ""},
    {"Lahnda", "lah", ""},
    {"Lamba", "lam", ""},
    {"Lao", "lao", "lo",                        MAKELCID(MAKELANGID(LANG_LAO, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Latin", "lat", "la"},
    {"Latvian", "lav", "lv",                    MAKELCID(MAKELANGID(LANG_LATVIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Letzeburgesch; Luxembourgish", "ltz", "lb"},
    {"Lezghian", "lez", ""},
    {"Limburgan; Limburger; Limburgish", "lim", "li"},
    {"Limburger; Limburgan; Limburgish;", "lim", "li"},
    {"Limburgish; Limburger; Limburgan", "lim", "li"},
    {"Lingala", "lin", "ln"},
    {"Lithuanian", "lit", "lt",                 MAKELCID(MAKELANGID(LANG_LITHUANIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Low German; Low Saxon; German, Low; Saxon, Low", "nds", ""},
    {"Low Saxon; Low German; Saxon, Low; German, Low", "nds", ""},
    {"Lozi", "loz", ""},
    {"Luba-Katanga", "lub", "lu"},
    {"Luba-Lulua", "lua", ""},
    {"Luiseno", "lui", ""},
    {"Lule Sami", "smj", ""},
    {"Lunda", "lun", ""},
    {"Luo (Kenya and Tanzania)", "luo", ""},
    {"Lushai", "lus", ""},
    {"Luxembourgish; Letzeburgesch", "ltz", "lb",   MAKELCID(MAKELANGID(LANG_LUXEMBOURGISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Macedonian", "mac", "mk",                 MAKELCID(MAKELANGID(LANG_MACEDONIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Macedonian", "mkd", "mk",                 MAKELCID(MAKELANGID(LANG_MACEDONIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Madurese", "mad", ""},
    {"Magahi", "mag", ""},
    {"Maithili", "mai", ""},
    {"Makasar", "mak", ""},
    {"Malagasy", "mlg", "mg"},
    {"Malay", "may", "ms",                      MAKELCID(MAKELANGID(LANG_MALAY, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Malay", "msa", "ms",                      MAKELCID(MAKELANGID(LANG_MALAY, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Malayalam", "mal", "ml",                  MAKELCID(MAKELANGID(LANG_MALAYALAM, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Maltese", "mlt", "mt",                    MAKELCID(MAKELANGID(LANG_MALTESE, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Manchu", "mnc", ""},
    {"Mandar", "mdr", ""},
    {"Mandingo", "man", ""},
    {"Manipuri", "mni", "",                     MAKELCID(MAKELANGID(LANG_MANIPURI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Manobo languages", "mno", ""},
    {"Manx", "glv", "gv"},
    {"Maori", "mao", "mi",                      MAKELCID(MAKELANGID(LANG_MAORI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Maori", "mri", "mi",                      MAKELCID(MAKELANGID(LANG_MAORI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Marathi", "mar", "mr",                    MAKELCID(MAKELANGID(LANG_MARATHI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Mari", "chm", ""},
    {"Marshallese", "mah", "mh"},
    {"Marwari", "mwr", ""},
    {"Masai", "mas", ""},
    {"Mayan languages", "myn", ""},
    {"Mende", "men", ""},
    {"Micmac", "mic", ""},
    {"Minangkabau", "min", ""},
    {"Miscellaneous languages", "mis", ""},
    {"Mohawk", "moh", "",                       MAKELCID(MAKELANGID(LANG_MOHAWK, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Moldavian", "mol", "mo"},
    {"Mon-Khmer (Other)", "mkh", ""},
    {"Mongo", "lol", ""},
    {"Mongolian", "mon", "mn",                  MAKELCID(MAKELANGID(LANG_MONGOLIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Mossi", "mos", ""},
    {"Multiple languages", "mul", ""},
    {"Munda languages", "mun", ""},
    {"Nahuatl", "nah", ""},
    {"Nauru", "nau", "na"},
    {"Navaho, Navajo", "nav", "nv"},
    {"Navajo; Navaho", "nav", "nv"},
    {"Ndebele, North", "nde", "nd"},
    {"Ndebele, South", "nbl", "nr"},
    {"Ndonga", "ndo", "ng"},
    {"Neapolitan", "nap", ""},
    {"Nepali", "nep", "ne",                     MAKELCID(MAKELANGID(LANG_NEPALI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Newari", "new", ""},
    {"Nias", "nia", ""},
    {"Niger-Kordofanian (Other)", "nic", ""},
    {"Nilo-Saharan (Other)", "ssa", ""},
    {"Niuean", "niu", ""},
    {"Norse, Old", "non", ""},
    {"North American Indian (Other)", "nai", ""},
    {"Northern Sami", "sme", "se"},
    {"North Ndebele", "nde", "nd"},
    {"Norwegian", "nor", "no",                  MAKELCID(MAKELANGID(LANG_NORWEGIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Norwegian Bokmål; Bokmål, Norwegian", "nob", "nb",  MAKELCID(MAKELANGID(LANG_NORWEGIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Norwegian Nynorsk; Nynorsk, Norwegian", "nno", "nn",  MAKELCID(MAKELANGID(LANG_NORWEGIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Nubian languages", "nub", ""},
    {"Nyamwezi", "nym", ""},
    {"Nyanja; Chichewa; Chewa", "nya", "ny"},
    {"Nyankole", "nyn", ""},
    {"Nynorsk, Norwegian; Norwegian Nynorsk", "nno", "nn"},
    {"Nyoro", "nyo", ""},
    {"Nzima", "nzi", ""},
    {"Occitan (post 1500},; Provençal", "oci", "oc",       MAKELCID(MAKELANGID(LANG_OCCITAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Ojibwa", "oji", "oj"},
    {"Old Bulgarian; Old Slavonic; Church Slavonic;", "chu", "cu"},
    {"Oriya", "ori", "or"},
    {"Oromo", "orm", "om"},
    {"Osage", "osa", ""},
    {"Ossetian; Ossetic", "oss", "os"},
    {"Ossetic; Ossetian", "oss", "os"},
    {"Otomian languages", "oto", ""},
    {"Pahlavi", "pal", ""},
    {"Palauan", "pau", ""},
    {"Pali", "pli", "pi"},
    {"Pampanga", "pam", ""},
    {"Pangasinan", "pag", ""},
    {"Panjabi", "pan", "pa"},
    {"Papiamento", "pap", ""},
    {"Papuan (Other)", "paa", ""},
    {"Persian", "per", "fa",                MAKELCID(MAKELANGID(LANG_PERSIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Persian", "fas", "fa",                MAKELCID(MAKELANGID(LANG_PERSIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Persian, Old (ca.600-400 B.C.)", "peo", ""},
    {"Philippine (Other)", "phi", ""},
    {"Phoenician", "phn", ""},
    {"Pohnpeian", "pon", ""},
    {"Polish", "pol", "pl",                 MAKELCID(MAKELANGID(LANG_POLISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Portuguese", "por", "pt",             MAKELCID(MAKELANGID(LANG_PORTUGUESE, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Prakrit languages", "pra", ""},
    {"Provençal; Occitan (post 1500)", "oci", "oc"},
    {"Provençal, Old (to 1500)", "pro", ""},
    {"Pushto", "pus", "ps"},
    {"Quechua", "que", "qu",                MAKELCID(MAKELANGID(LANG_QUECHUA, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Raeto-Romance", "roh", "rm"},
    {"Rajasthani", "raj", ""},
    {"Rapanui", "rap", ""},
    {"Rarotongan", "rar", ""},
    {"Reserved for local use", "qaa-qtz", ""},
    {"Romance (Other)", "roa", ""},
    {"Romanian", "rum", "ro",               MAKELCID(MAKELANGID(LANG_ROMANIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Romanian", "ron", "ro",               MAKELCID(MAKELANGID(LANG_ROMANIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Romany", "rom", ""},
    {"Rundi", "run", "rn"},
    {"Russian", "rus", "ru",                MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Salishan languages", "sal", ""},
    {"Samaritan Aramaic", "sam", ""},
    {"Sami languages (Other)", "smi", ""},
    {"Samoan", "smo", "sm"},
    {"Sandawe", "sad", ""},
    {"Sango", "sag", "sg"},
    {"Sanskrit", "san", "sa",               MAKELCID(MAKELANGID(LANG_SANSKRIT, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Santali", "sat", ""},
    {"Sardinian", "srd", "sc"},
    {"Sasak", "sas", ""},
    {"Saxon, Low; German, Low; Low Saxon; Low German", "nds", ""},
    {"Scots", "sco", ""},
    {"Scottish Gaelic; Gaelic", "gla", "gd"},
    {"Selkup", "sel", ""},
    {"Semitic (Other)", "sem", ""},
    {"Serbian", "scc", "sr",                MAKELCID(MAKELANGID(LANG_SERBIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Serbian", "srp", "sr",                MAKELCID(MAKELANGID(LANG_SERBIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Serer", "srr", ""},
    {"Shan", "shn", ""},
    {"Shona", "sna", "sn"},
    {"Sichuan Yi", "iii", "ii"},
    {"Sidamo", "sid", ""},
    {"Sign languages", "sgn", ""},
    {"Siksika", "bla", ""},
    {"Sindhi", "snd", "sd",                 MAKELCID(MAKELANGID(LANG_SINDHI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Sinhalese", "sin", "si",              MAKELCID(MAKELANGID(LANG_SINHALESE, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Sino-Tibetan (Other)", "sit", ""},
    {"Siouan languages", "sio", ""},
    {"Skolt Sami", "sms", ""},
    {"Slave (Athapascan)", "den", ""},
    {"Slavic (Other)", "sla", ""},
    {"Slovak", "slo", "sk",                 MAKELCID(MAKELANGID(LANG_SLOVAK, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Slovak", "slk", "sk",                 MAKELCID(MAKELANGID(LANG_SLOVAK, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Slovenian", "slv", "sl",              MAKELCID(MAKELANGID(LANG_SLOVENIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Sogdian", "sog", ""},
    {"Somali", "som", "so"},
    {"Songhai", "son", ""},
    {"Soninke", "snk", ""},
    {"Sorbian languages", "wen", ""},
    {"Sotho, Northern", "nso", "",          MAKELCID(MAKELANGID(LANG_SOTHO, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Sotho, Southern", "sot", "st",        MAKELCID(MAKELANGID(LANG_SOTHO, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"South American Indian (Other)", "sai", ""},
    {"Southern Sami", "sma", ""},
    {"South Ndebele", "nbl", "nr"},
    {"Spanish; Castilian", "spa", "es",     MAKELCID(MAKELANGID(LANG_SPANISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Sukuma", "suk", ""},
    {"Sumerian", "sux", ""},
    {"Sundanese", "sun", "su"},
    {"Susu", "sus", ""},
    {"Swahili", "swa", "sw",                MAKELCID(MAKELANGID(LANG_SWAHILI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Swati", "ssw", "ss"},
    {"Swedish", "swe", "sv",                MAKELCID(MAKELANGID(LANG_SWEDISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Syriac", "syr", "",                   MAKELCID(MAKELANGID(LANG_SYRIAC, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Tagalog", "tgl", "tl"},
    {"Tahitian", "tah", "ty"},
    {"Tai (Other)", "tai", ""},
    {"Tajik", "tgk", "tg",                  MAKELCID(MAKELANGID(LANG_TAJIK, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Tamashek", "tmh", ""},
    {"Tamil", "tam", "ta",                  MAKELCID(MAKELANGID(LANG_TAMIL, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Tatar", "tat", "tt",                  MAKELCID(MAKELANGID(LANG_TATAR, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Telugu", "tel", "te",                 MAKELCID(MAKELANGID(LANG_TELUGU, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Tereno", "ter", ""},
    {"Tetum", "tet", ""},
    {"Thai", "tha", "th",                   MAKELCID(MAKELANGID(LANG_THAI, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Tibetan", "tib", "bo",                MAKELCID(MAKELANGID(LANG_TIBETAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Tibetan", "bod", "bo",                MAKELCID(MAKELANGID(LANG_TIBETAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Tigre", "tig", ""},
    {"Tigrinya", "tir", "ti",               MAKELCID(MAKELANGID(LANG_TIGRIGNA, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Timne", "tem", ""},
    {"Tiv", "tiv", ""},
    {"Tlingit", "tli", ""},
    {"Tok Pisin", "tpi", ""},
    {"Tokelau", "tkl", ""},
    {"Tonga (Nyasa)", "tog", ""},
    {"Tonga (Tonga Islands)", "ton", "to"},
    {"Tsimshian", "tsi", ""},
    {"Tsonga", "tso", "ts"},
    {"Tswana", "tsn", "tn",                 MAKELCID(MAKELANGID(LANG_TSWANA, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Tumbuka", "tum", ""},
    {"Tupi languages", "tup", ""},
    {"Turkish", "tur", "tr",                MAKELCID(MAKELANGID(LANG_TURKISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Turkish, Ottoman (1500-1928)", "ota", "", MAKELCID(MAKELANGID(LANG_TURKISH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Turkmen", "tuk", "tk",                MAKELCID(MAKELANGID(LANG_TURKMEN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Tuvalu", "tvl", ""},
    {"Tuvinian", "tyv", ""},
    {"Twi", "twi", "tw"},
    {"Ugaritic", "uga", ""},
    {"Uighur", "uig", "ug",                 MAKELCID(MAKELANGID(LANG_UIGHUR, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Ukrainian", "ukr", "uk",              MAKELCID(MAKELANGID(LANG_UKRAINIAN, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Umbundu", "umb", ""},
    {"Undetermined", "und", ""},
    {"Urdu", "urd", "ur",                   MAKELCID(MAKELANGID(LANG_URDU, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Uzbek", "uzb", "uz",                  MAKELCID(MAKELANGID(LANG_UZBEK, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Vai", "vai", ""},
    {"Venda", "ven", "ve"},
    {"Vietnamese", "vie", "vi",             MAKELCID(MAKELANGID(LANG_VIETNAMESE, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Volapuk", "vol", "vo"},
    {"Votic", "vot", ""},
    {"Wakashan languages", "wak", ""},
    {"Walamo", "wal", ""},
    {"Walloon", "wln", "wa"},
    {"Waray", "war", ""},
    {"Washo", "was", ""},
    {"Welsh", "wel", "cy",                  MAKELCID(MAKELANGID(LANG_WELSH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Welsh", "cym", "cy",                  MAKELCID(MAKELANGID(LANG_WELSH, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Wolof", "wol", "wo",                  MAKELCID(MAKELANGID(LANG_WOLOF, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Xhosa", "xho", "xh",                  MAKELCID(MAKELANGID(LANG_XHOSA, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Yakut", "sah", "",                    MAKELCID(MAKELANGID(LANG_YAKUT, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Yao", "yao", ""},
    {"Yapese", "yap", ""},
    {"Yiddish", "yid", "yi"},
    {"Yoruba", "yor", "yo",                 MAKELCID(MAKELANGID(LANG_YORUBA, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Yupik languages", "ypk", ""},
    {"Zande", "znd", ""},
    {"Zapotec", "zap", ""},
    {"Zenaga", "zen", ""},
    {"Zhuang; Chuang", "zha", "za"},
    {"Zulu", "zul", "zu",                   MAKELCID(MAKELANGID(LANG_ZULU, SUBLANG_DEFAULT), SORT_DEFAULT)},
    {"Zuni", "zun", ""},
    {"Classical Newari", "nwc", ""},
    {"Klingon", "tlh", ""},
    {"Blin", "byn", ""},
    {"Lojban", "jbo", ""},
    {"Lower Sorbian", "dsb", ""},
    {"Upper Sorbian", "hsb", ""},
    {"Kashubian", "csb", ""},
    {"Crimean Turkish", "crh", ""},
    {"Erzya", "myv", ""},
    {"Moksha", "mdf", ""},
    {"Karachay-Balkar", "krc", ""},
    {"Adyghe", "ady", ""},
    {"Udmurt", "udm", ""},
    {"Dargwa", "dar", ""},
    {"Ingush", "inh", ""},
    {"Nogai", "nog", ""},
    {"Haitian", "hat", "ht"},
    {"Kalmyk", "xal", ""},
    {"", "", ""},
    {"No subtitles", "---", "", (LCID)LCID_NOSUBTITLES},
};

CString ISO6391ToLanguage(LPCSTR code)
{
    CHAR tmp[2 + 1];
    strncpy_s(tmp, code, 2);
    tmp[2] = 0;
    _strlwr_s(tmp);
    for (size_t i = 0, cnt = _countof(s_isolangs); i < cnt; i++) {
        if (!strcmp(s_isolangs[i].iso6391, tmp)) {
            CString ret = CString(CStringA(s_isolangs[i].name));
            int k = ret.Find(';');
            if (k > 0) {
                ret = ret.Left(k);
            }
            return ret;
        }
    }
    return _T("");
}

CString ISO6392ToLanguage(LPCSTR code)
{
    CHAR tmp[3 + 1];
    strncpy_s(tmp, code, 3);
    tmp[3] = 0;
    _strlwr_s(tmp);
    for (size_t i = 0, cnt = _countof(s_isolangs); i < cnt; i++) {
        if (!strcmp(s_isolangs[i].iso6392, tmp)) {
            CString ret = CString(CStringA(s_isolangs[i].name));
            int k = ret.Find(';');
            if (k > 0) {
                ret = ret.Left(k);
            }
            return ret;
        }
    }
    return CString(code);
}

bool IsISO639Language(LPCSTR code)
{
    size_t nLen = strlen(code) + 1;
    LPSTR tmp = DEBUG_NEW CHAR[nLen];
    strncpy_s(tmp, nLen, code, nLen);
    _strlwr_s(tmp, nLen);
    tmp[0] = (CHAR)toupper(tmp[0]);

    bool bFound = false;
    for (size_t i = 0, cnt = _countof(s_isolangs); i < cnt; i++) {
        if (!strcmp(s_isolangs[i].name, tmp)) {
            bFound = true;
            break;
        }
    }

    delete [] tmp;

    return bFound;
}

CString ISO639XToLanguage(LPCSTR code, bool bCheckForFullLangName /*= false*/)
{
    CString lang;

    switch (size_t nLen = strlen(code)) {
        case 2:
            lang = ISO6391ToLanguage(code);
            break;
        case 3:
            lang = ISO6392ToLanguage(code);
            if (lang == code) { // When it can't find a match, ISO6392ToLanguage returns the input string
                lang.Empty();
            }
            break;
        default:
            if (bCheckForFullLangName && nLen > 3) {
                if (IsISO639Language(code)) {
                    lang = code;
                }
            }
    }

    return lang;
}

LCID ISO6391ToLcid(LPCSTR code)
{
    CHAR tmp[3 + 1];
    strncpy_s(tmp, code, 3);
    tmp[3] = 0;
    _strlwr_s(tmp);
    for (size_t i = 0, cnt = _countof(s_isolangs); i < cnt; i++) {
        if (!strcmp(s_isolangs[i].iso6391, code)) {
            return s_isolangs[i].lcid;
        }
    }
    return 0;
}

LCID ISO6392ToLcid(LPCSTR code)
{
    CHAR tmp[3 + 1];
    strncpy_s(tmp, code, 3);
    tmp[3] = 0;
    _strlwr_s(tmp);
    for (size_t i = 0, cnt = _countof(s_isolangs); i < cnt; i++) {
        if (!strcmp(s_isolangs[i].iso6392, tmp)) {
            return s_isolangs[i].lcid;
        }
    }
    return 0;
}

CStringA ISO6391To6392(LPCSTR code)
{
    CHAR tmp[2 + 1];
    strncpy_s(tmp, code, 2);
    tmp[2] = 0;
    _strlwr_s(tmp);
    for (size_t i = 0, cnt = _countof(s_isolangs); i < cnt; i++) {
        if (!strcmp(s_isolangs[i].iso6391, tmp)) {
            return CStringA(s_isolangs[i].iso6392);
        }
    }
    return "";
}

CString ISO6392To6391(LPCSTR code)
{
    CHAR tmp[3 + 1];
    strncpy_s(tmp, code, 3);
    tmp[3] = 0;
    _strlwr_s(tmp);
    for (size_t i = 0, cnt = _countof(s_isolangs); i < cnt; i++) {
        if (!strcmp(s_isolangs[i].iso6392, tmp)) {
            return CString(s_isolangs[i].iso6391);
        }
    }
    return _T("");
}

CString LanguageToISO6392(LPCTSTR lang)
{
    CString str = lang;
    str.MakeLower();
    for (size_t i = 0, cnt = _countof(s_isolangs); i < cnt; i++) {
        CAtlList<CString> sl;
        Explode(CString(s_isolangs[i].name), sl, ';');
        POSITION pos = sl.GetHeadPosition();
        while (pos) {
            if (!str.CompareNoCase(sl.GetNext(pos))) {
                return CString(s_isolangs[i].iso6392);
            }
        }
    }
    return _T("");
}

BOOL CFileGetStatus(LPCTSTR lpszFileName, CFileStatus& status)
{
    try {
        return CFile::GetStatus(lpszFileName, status);
    } catch (CException* e) {
        // MFCBUG: E_INVALIDARG / "Parameter is incorrect" is thrown for certain cds (vs2003)
        // http://groups.google.co.uk/groups?hl=en&lr=&ie=UTF-8&threadm=OZuXYRzWDHA.536%40TK2MSFTNGP10.phx.gbl&rnum=1&prev=/groups%3Fhl%3Den%26lr%3D%26ie%3DISO-8859-1
        TRACE(_T("CFile::GetStatus has thrown an exception\n"));
        e->Delete();
        return false;
    }
}

// filter registration helpers

bool DeleteRegKey(LPCTSTR pszKey, LPCTSTR pszSubkey)
{
    bool bOK = false;

    HKEY hKey;
    LONG ec = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, pszKey, 0, KEY_ALL_ACCESS, &hKey);
    if (ec == ERROR_SUCCESS) {
        if (pszSubkey != 0) {
            ec = ::RegDeleteKey(hKey, pszSubkey);
        }

        bOK = (ec == ERROR_SUCCESS);

        ::RegCloseKey(hKey);
    }

    return bOK;
}

bool SetRegKeyValue(LPCTSTR pszKey, LPCTSTR pszSubkey, LPCTSTR pszValueName, LPCTSTR pszValue)
{
    bool bOK = false;

    CString szKey(pszKey);
    if (pszSubkey != 0) {
        szKey += CString(_T("\\")) + pszSubkey;
    }

    HKEY hKey;
    LONG ec = ::RegCreateKeyEx(HKEY_CLASSES_ROOT, szKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, 0);
    if (ec == ERROR_SUCCESS) {
        if (pszValue != 0) {
            ec = ::RegSetValueEx(hKey, pszValueName, 0, REG_SZ,
                                 reinterpret_cast<BYTE*>(const_cast<LPTSTR>(pszValue)),
                                 (DWORD)(_tcslen(pszValue) + 1) * sizeof(TCHAR));
        }

        bOK = (ec == ERROR_SUCCESS);

        ::RegCloseKey(hKey);
    }

    return bOK;
}

bool SetRegKeyValue(LPCTSTR pszKey, LPCTSTR pszSubkey, LPCTSTR pszValue)
{
    return SetRegKeyValue(pszKey, pszSubkey, 0, pszValue);
}

void RegisterSourceFilter(const CLSID& clsid, const GUID& subtype2, LPCTSTR chkbytes, LPCTSTR ext, ...)
{
    CString null = CStringFromGUID(GUID_NULL);
    CString majortype = CStringFromGUID(MEDIATYPE_Stream);
    CString subtype = CStringFromGUID(subtype2);

    SetRegKeyValue(_T("Media Type\\") + majortype, subtype, _T("0"), chkbytes);
    SetRegKeyValue(_T("Media Type\\") + majortype, subtype, _T("Source Filter"), CStringFromGUID(clsid));

    DeleteRegKey(_T("Media Type\\") + null, subtype);

    va_list marker;
    va_start(marker, ext);
    for (; ext; ext = va_arg(marker, LPCTSTR)) {
        DeleteRegKey(_T("Media Type\\Extensions"), ext);
    }
    va_end(marker);
}

void RegisterSourceFilter(const CLSID& clsid, const GUID& subtype2, const CAtlList<CString>& chkbytes, LPCTSTR ext, ...)
{
    CString null = CStringFromGUID(GUID_NULL);
    CString majortype = CStringFromGUID(MEDIATYPE_Stream);
    CString subtype = CStringFromGUID(subtype2);

    POSITION pos = chkbytes.GetHeadPosition();
    for (ptrdiff_t i = 0; pos; i++) {
        CString idx;
        idx.Format(_T("%Id"), i);
        SetRegKeyValue(_T("Media Type\\") + majortype, subtype, idx, chkbytes.GetNext(pos));
    }

    SetRegKeyValue(_T("Media Type\\") + majortype, subtype, _T("Source Filter"), CStringFromGUID(clsid));

    DeleteRegKey(_T("Media Type\\") + null, subtype);

    va_list marker;
    va_start(marker, ext);
    for (; ext; ext = va_arg(marker, LPCTSTR)) {
        DeleteRegKey(_T("Media Type\\Extensions"), ext);
    }
    va_end(marker);
}

void UnRegisterSourceFilter(const GUID& subtype)
{
    DeleteRegKey(_T("Media Type\\") + CStringFromGUID(MEDIATYPE_Stream), CStringFromGUID(subtype));
}

struct DXVA2_DECODER {
    const GUID* Guid;
    LPCTSTR Description;
};

static const DXVA2_DECODER DXVA2Decoder[] = {
    {&GUID_NULL,                        _T("Unknown")},
    {&GUID_NULL,                        _T("Not using DXVA")},
    {&DXVA_Intel_H264_ClearVideo,       _T("H.264 bitstream decoder, ClearVideo(tm)")},  // Intel ClearVideo H264 bitstream decoder
    {&DXVA_Intel_VC1_ClearVideo,        _T("VC-1 bitstream decoder, ClearVideo(tm)")},   // Intel ClearVideo VC-1 bitstream decoder
    {&DXVA_Intel_VC1_ClearVideo_2,      _T("VC-1 bitstream decoder 2, ClearVideo(tm)")}, // Intel ClearVideo VC-1 bitstream decoder 2
    {&DXVA_MPEG4_ASP,                   _T("MPEG-4 ASP bitstream decoder")},             // NVIDIA MPEG-4 ASP bitstream decoder
    {&DXVA_ModeNone,                    _T("Mode none")},
    {&DXVA_ModeH261_A,                  _T("H.261 A, post processing")},
    {&DXVA_ModeH261_B,                  _T("H.261 B, deblocking")},
    {&DXVA_ModeH263_A,                  _T("H.263 A, motion compensation, no FGT")},
    {&DXVA_ModeH263_B,                  _T("H.263 B, motion compensation, FGT")},
    {&DXVA_ModeH263_C,                  _T("H.263 C, IDCT, no FGT")},
    {&DXVA_ModeH263_D,                  _T("H.263 D, IDCT, FGT")},
    {&DXVA_ModeH263_E,                  _T("H.263 E, bitstream decoder, no FGT")},
    {&DXVA_ModeH263_F,                  _T("H.263 F, bitstream decoder, FGT")},
    {&DXVA_ModeMPEG1_A,                 _T("MPEG-1 A, post processing")},
    {&DXVA_ModeMPEG2_A,                 _T("MPEG-2 A, motion compensation")},
    {&DXVA_ModeMPEG2_B,                 _T("MPEG-2 B, motion compensation + blending")},
    {&DXVA_ModeMPEG2_C,                 _T("MPEG-2 C, IDCT")},
    {&DXVA_ModeMPEG2_D,                 _T("MPEG-2 D, IDCT + blending")},
    {&DXVA_ModeH264_A,                  _T("H.264 A, motion compensation, no FGT")},
    {&DXVA_ModeH264_B,                  _T("H.264 B, motion compensation, FGT")},
    {&DXVA_ModeH264_C,                  _T("H.264 C, IDCT, no FGT")},
    {&DXVA_ModeH264_D,                  _T("H.264 D, IDCT, FGT")},
    {&DXVA_ModeH264_E,                  _T("H.264 E, bitstream decoder, no FGT")},
    {&DXVA_ModeH264_F,                  _T("H.264 F, bitstream decoder, FGT")},
    {&DXVA_ModeWMV8_A,                  _T("WMV8 A, post processing")},
    {&DXVA_ModeWMV8_B,                  _T("WMV8 B, motion compensation")},
    {&DXVA_ModeWMV9_A,                  _T("WMV9 A, post processing")},
    {&DXVA_ModeWMV9_B,                  _T("WMV9 B, motion compensation")},
    {&DXVA_ModeWMV9_C,                  _T("WMV9 C, IDCT")},
    {&DXVA_ModeVC1_A,                   _T("VC-1 A, post processing")},
    {&DXVA_ModeVC1_B,                   _T("VC-1 B, motion compensation")},
    {&DXVA_ModeVC1_C,                   _T("VC-1 C, IDCT")},
    {&DXVA_ModeVC1_D,                   _T("VC-1 D, bitstream decoder")},
    {&DXVA_NoEncrypt,                   _T("No encryption")},
    {&DXVA2_ModeMPEG2_MoComp,           _T("MPEG-2 motion compensation")},
    {&DXVA2_ModeMPEG2_IDCT,             _T("MPEG-2 IDCT")},
    {&DXVA2_ModeMPEG2_VLD,              _T("MPEG-2 variable-length decoder")},
    {&DXVA2_ModeH264_A,                 _T("H.264 A, motion compensation, no FGT")},
    {&DXVA2_ModeH264_B,                 _T("H.264 B, motion compensation, FGT")},
    {&DXVA2_ModeH264_C,                 _T("H.264 C, IDCT, no FGT")},
    {&DXVA2_ModeH264_D,                 _T("H.264 D, IDCT, FGT")},
    {&DXVA2_ModeH264_E,                 _T("H.264 E, bitstream decoder, no FGT")},
    {&DXVA2_ModeH264_F,                 _T("H.264 F, bitstream decoder, FGT")},
    {&DXVA2_ModeWMV8_A,                 _T("WMV8 A, post processing")},
    {&DXVA2_ModeWMV8_B,                 _T("WMV8 B, motion compensation")},
    {&DXVA2_ModeWMV9_A,                 _T("WMV9 A, post processing")},
    {&DXVA2_ModeWMV9_B,                 _T("WMV9 B, motion compensation")},
    {&DXVA2_ModeWMV9_C,                 _T("WMV9 C, IDCT")},
    {&DXVA2_ModeVC1_A,                  _T("VC-1 A, post processing")},
    {&DXVA2_ModeVC1_B,                  _T("VC-1 B, motion compensation")},
    {&DXVA2_ModeVC1_C,                  _T("VC-1 C, IDCT")},
    {&DXVA2_ModeVC1_D,                  _T("VC-1 D, bitstream decoder")},
    {&DXVA2_NoEncrypt,                  _T("No encryption")},
    {&DXVA2_VideoProcProgressiveDevice, _T("Progressive scan")},
    {&DXVA2_VideoProcBobDevice,         _T("Bob deinterlacing")},
    {&DXVA2_VideoProcSoftwareDevice,    _T("Software processing")}
};

LPCTSTR GetDXVAMode(const GUID* guidDecoder)
{
    int nPos = 0;

    for (int i = 1; i < _countof(DXVA2Decoder); i++) {
        if (*guidDecoder == *DXVA2Decoder[i].Guid) {
            nPos = i;
            break;
        }
    }

    return DXVA2Decoder[nPos].Description;
}

// hour, minute, second, millisec
CString ReftimeToString(const REFERENCE_TIME& rtVal)
{
    if (rtVal == _I64_MIN) {
        return _T("INVALID TIME");
    }

    CString strTemp;
    LONGLONG llTotalMs = ConvertToMilliseconds(rtVal);
    int lHour     = (int)(llTotalMs / (1000 * 60 * 60));
    int lMinute   = (llTotalMs / (1000 * 60)) % 60;
    int lSecond   = (llTotalMs /  1000) % 60;
    int lMillisec = llTotalMs  %  1000;

    strTemp.Format(_T("%02d:%02d:%02d,%03d"), lHour, lMinute, lSecond, lMillisec);
    return strTemp;
}

// hour, minute, second (round)
CString ReftimeToString2(const REFERENCE_TIME& rtVal)
{
    CString strTemp;
    LONGLONG seconds = (rtVal + 5000000) / 10000000;
    int lHour   = (int)(seconds / 3600);
    int lMinute = (int)(seconds / 60 % 60);
    int lSecond = (int)(seconds % 60);

    strTemp.Format(_T("%02d:%02d:%02d"), lHour, lMinute, lSecond);
    return strTemp;
}

CString DVDtimeToString(const DVD_HMSF_TIMECODE& rtVal, bool bAlwaysShowHours)
{
    CString strTemp;
    if (rtVal.bHours > 0 || bAlwaysShowHours) {
        strTemp.Format(_T("%02u:%02u:%02u"), rtVal.bHours, rtVal.bMinutes, rtVal.bSeconds);
    } else {
        strTemp.Format(_T("%02u:%02u"), rtVal.bMinutes, rtVal.bSeconds);
    }
    return strTemp;
}

REFERENCE_TIME StringToReftime(LPCTSTR strVal)
{
    REFERENCE_TIME rt = 0;
    int lHour = 0;
    int lMinute = 0;
    int lSecond = 0;
    int lMillisec = 0;

    if (_stscanf_s(strVal, _T("%02d:%02d:%02d,%03d"), &lHour, &lMinute, &lSecond, &lMillisec) == 4) {
        rt = ((((lHour * 24) + lMinute) * 60 + lSecond) * MILLISECONDS + lMillisec) * (UNITS / MILLISECONDS);
    }

    return rt;
}

const double Rec601_Kr = 0.299;
const double Rec601_Kb = 0.114;
const double Rec601_Kg = 0.587;

COLORREF YCrCbToRGB_Rec601(BYTE Y, BYTE Cr, BYTE Cb, double sourceBlackLevel, double sourceWhiteLevel, double targetBlackLevel, double targetWhiteLevel)
{
    double targetRange = targetWhiteLevel - targetBlackLevel;
    double chromaChannelRange = (sourceWhiteLevel == 235.0 ? 224.0 /* 4:2:0 */ : sourceBlackLevel == 0.0 ? 255.0 /* 4:4:4 */ : 239.0 /* 4:2:2 */) / 2.0;
    double chromaScaleFactor = targetRange / chromaChannelRange;
    double scaledY = (Y - (sourceBlackLevel - targetBlackLevel)) * (targetRange / (sourceWhiteLevel - sourceBlackLevel));

    double rp = scaledY + chromaScaleFactor * (Cr - 128) * (1.0 - Rec601_Kr);
    double gp = scaledY - chromaScaleFactor * (Cb - 128) * (1.0 - Rec601_Kb) * Rec601_Kb / Rec601_Kg - chromaScaleFactor * (Cr - 128) * (1.0 - Rec601_Kr) * Rec601_Kr / Rec601_Kg;
    double bp = scaledY + chromaScaleFactor * (Cb - 128) * (1.0 - Rec601_Kb);

    auto R = (BYTE)std::max(targetBlackLevel, std::min(std::abs(std::round(rp)), targetWhiteLevel));
    auto G = (BYTE)std::max(targetBlackLevel, std::min(std::abs(std::round(gp)), targetWhiteLevel));
    auto B = (BYTE)std::max(targetBlackLevel, std::min(std::abs(std::round(bp)), targetWhiteLevel));

    return RGB(R, G, B);
}

DWORD YCrCbToRGB_Rec601(BYTE A, BYTE Y, BYTE Cr, BYTE Cb, double sourceBlackLevel, double sourceWhiteLevel, double targetBlackLevel, double targetWhiteLevel)
{
    double targetRange = targetWhiteLevel - targetBlackLevel;
    double chromaChannelRange = (sourceWhiteLevel == 235.0 ? 224.0 /* 4:2:0 */ : sourceBlackLevel == 0.0 ? 255.0 /* 4:4:4 */ : 239.0 /* 4:2:2 */) / 2.0;
    double chromaScaleFactor = targetRange / chromaChannelRange;
    double scaledY = (Y - (sourceBlackLevel - targetBlackLevel)) * (targetRange / (sourceWhiteLevel - sourceBlackLevel));

    double rp = scaledY + chromaScaleFactor * (Cr - 128) * (1.0 - Rec601_Kr);
    double gp = scaledY - chromaScaleFactor * (Cb - 128) * (1.0 - Rec601_Kb) * Rec601_Kb / Rec601_Kg - chromaScaleFactor * (Cr - 128) * (1.0 - Rec601_Kr) * Rec601_Kr / Rec601_Kg;
    double bp = scaledY + chromaScaleFactor * (Cb - 128) * (1.0 - Rec601_Kb);

    auto R = (BYTE)std::max(targetBlackLevel, std::min(std::abs(std::round(rp)), targetWhiteLevel));
    auto G = (BYTE)std::max(targetBlackLevel, std::min(std::abs(std::round(gp)), targetWhiteLevel));
    auto B = (BYTE)std::max(targetBlackLevel, std::min(std::abs(std::round(bp)), targetWhiteLevel));

    return D3DCOLOR_ARGB(A, R, G, B);
}


const double Rec709_Kr = 0.2126;
const double Rec709_Kb = 0.0722;
const double Rec709_Kg = 0.7152;

COLORREF YCrCbToRGB_Rec709(BYTE Y, BYTE Cr, BYTE Cb, double sourceBlackLevel, double sourceWhiteLevel, double targetBlackLevel, double targetWhiteLevel)
{
    double targetRange = targetWhiteLevel - targetBlackLevel;
    double chromaChannelRange = (sourceWhiteLevel == 235.0 ? 224.0 /* 4:2:0 */ : sourceBlackLevel == 0.0 ? 255.0 /* 4:4:4 */ : 239.0 /* 4:2:2 */) / 2.0;
    double chromaScaleFactor = targetRange / chromaChannelRange;
    double scaledY = (Y - (sourceBlackLevel - targetBlackLevel)) * (targetRange / (sourceWhiteLevel - sourceBlackLevel));

    double rp = scaledY + chromaScaleFactor * (Cr - 128) * (1.0 - Rec709_Kr);
    double gp = scaledY - chromaScaleFactor * (Cb - 128) * (1.0 - Rec709_Kb) * Rec709_Kb / Rec709_Kg - chromaScaleFactor * (Cr - 128) * (1.0 - Rec709_Kr) * Rec709_Kr / Rec709_Kg;
    double bp = scaledY + chromaScaleFactor * (Cb - 128) * (1.0 - Rec709_Kb);

    auto R = (BYTE)std::max(targetBlackLevel, std::min(std::abs(std::round(rp)), targetWhiteLevel));
    auto G = (BYTE)std::max(targetBlackLevel, std::min(std::abs(std::round(gp)), targetWhiteLevel));
    auto B = (BYTE)std::max(targetBlackLevel, std::min(std::abs(std::round(bp)), targetWhiteLevel));

    return RGB(R, G, B);
}

DWORD YCrCbToRGB_Rec709(BYTE A, BYTE Y, BYTE Cr, BYTE Cb, double sourceBlackLevel, double sourceWhiteLevel, double targetBlackLevel, double targetWhiteLevel)
{
    double targetRange = targetWhiteLevel - targetBlackLevel;
    double chromaChannelRange = (sourceWhiteLevel == 235.0 ? 224.0 /* 4:2:0 */ : sourceBlackLevel == 0.0 ? 255.0 /* 4:4:4 */ : 239.0 /* 4:2:2 */) / 2.0;
    double chromaScaleFactor = targetRange / chromaChannelRange;
    double scaledY = (Y - (sourceBlackLevel - targetBlackLevel)) * (targetRange / (sourceWhiteLevel - sourceBlackLevel));

    double rp = scaledY + chromaScaleFactor * (Cr - 128) * (1.0 - Rec709_Kr);
    double gp = scaledY - chromaScaleFactor * (Cb - 128) * (1.0 - Rec709_Kb) * Rec709_Kb / Rec709_Kg - chromaScaleFactor * (Cr - 128) * (1.0 - Rec709_Kr) * Rec709_Kr / Rec709_Kg;
    double bp = scaledY + chromaScaleFactor * (Cb - 128) * (1.0 - Rec709_Kb);

    auto R = (BYTE)std::max(targetBlackLevel, std::min(std::abs(std::round(rp)), targetWhiteLevel));
    auto G = (BYTE)std::max(targetBlackLevel, std::min(std::abs(std::round(gp)), targetWhiteLevel));
    auto B = (BYTE)std::max(targetBlackLevel, std::min(std::abs(std::round(bp)), targetWhiteLevel));

    return D3DCOLOR_ARGB(A, R, G, B);
}

const wchar_t* StreamTypeToName(PES_STREAM_TYPE _Type)
{
    switch (_Type) {
        case VIDEO_STREAM_MPEG1:
            return L"MPEG-1";
        case VIDEO_STREAM_MPEG2:
            return L"MPEG-2";
        case AUDIO_STREAM_MPEG1:
            return L"MPEG-1";
        case AUDIO_STREAM_MPEG2:
            return L"MPEG-2";
        case VIDEO_STREAM_H264:
            return L"H264";
        case AUDIO_STREAM_LPCM:
            return L"LPCM";
        case AUDIO_STREAM_AC3:
            return L"Dolby Digital";
        case AUDIO_STREAM_DTS:
            return L"DTS";
        case AUDIO_STREAM_AC3_TRUE_HD:
            return L"Dolby TrueHD";
        case AUDIO_STREAM_AC3_PLUS:
            return L"Dolby Digital Plus";
        case AUDIO_STREAM_DTS_HD:
            return L"DTS-HD High Resolution Audio";
        case AUDIO_STREAM_DTS_HD_MASTER_AUDIO:
            return L"DTS-HD Master Audio";
        case PRESENTATION_GRAPHICS_STREAM:
            return L"Presentation Graphics Stream";
        case INTERACTIVE_GRAPHICS_STREAM:
            return L"Interactive Graphics Stream";
        case SUBTITLE_STREAM:
            return L"Subtitle";
        case SECONDARY_AUDIO_AC3_PLUS:
            return L"Secondary Dolby Digital Plus";
        case SECONDARY_AUDIO_DTS_HD:
            return L"Secondary DTS-HD High Resolution Audio";
        case VIDEO_STREAM_VC1:
            return L"VC-1";
    }
    return nullptr;
}

//
// Usage: SetThreadName (-1, "MainThread");
// Code from http://msdn.microsoft.com/en-us/library/xcb2z8hs%28v=vs.110%29.aspx
//

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO {
    DWORD  dwType;      // must be 0x1000
    LPCSTR szName;      // pointer to name (in user addr space)
    DWORD  dwThreadID;  // thread ID (-1 caller thread)
    DWORD  dwFlags;     // reserved for future use, must be zero
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
    THREADNAME_INFO info;
    info.dwType     = 0x1000;
    info.szName     = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags    = 0;

    __try {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

void CorrectComboListWidth(CComboBox& m_pComboBox)
{
    // Find the longest string in the combo box.
    if (m_pComboBox.GetCount() <= 0) {
        return;
    }

    CString    str;
    CSize      sz;
    int        dx = 0;
    TEXTMETRIC tm;
    CDC*       pDC = m_pComboBox.GetDC();
    CFont*     pFont = m_pComboBox.GetFont();

    // Select the listbox font, save the old font
    CFont* pOldFont = pDC->SelectObject(pFont);
    // Get the text metrics for avg char width
    pDC->GetTextMetrics(&tm);

    for (int i = 0; i < m_pComboBox.GetCount(); i++) {
        m_pComboBox.GetLBText(i, str);
        sz = pDC->GetTextExtent(str);

        // Add the avg width to prevent clipping
        sz.cx += tm.tmAveCharWidth;

        if (sz.cx > dx) {
            dx = sz.cx;
        }
    }
    // Select the old font back into the DC
    pDC->SelectObject(pOldFont);
    m_pComboBox.ReleaseDC(pDC);

    // Get the scrollbar width if it exists
    int min_visible = m_pComboBox.GetMinVisible();
    int scroll_width = (m_pComboBox.GetCount() > min_visible) ?
                       ::GetSystemMetrics(SM_CXVSCROLL) : 0;

    // Adjust the width for the vertical scroll bar and the left and right border.
    dx += scroll_width + 2 *::GetSystemMetrics(SM_CXEDGE);

    // Set the width of the list box so that every item is completely visible.
    m_pComboBox.SetDroppedWidth(dx);
}

void CorrectComboBoxHeaderWidth(CWnd* pComboBox)
{
    if (!pComboBox) {
        return;
    }

    CDC*   pDC = pComboBox->GetDC();
    CFont* pFont = pComboBox->GetFont();
    CFont* pOldFont = pDC->SelectObject(pFont);

    CString str;
    pComboBox->GetWindowText(str);
    CSize szText = pDC->GetTextExtent(str);

    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);
    pDC->SelectObject(pOldFont);
    pComboBox->ReleaseDC(pDC);

    CRect r;
    pComboBox->GetWindowRect(r);
    pComboBox->GetOwner()->ScreenToClient(r);

    r.right = r.left + ::GetSystemMetrics(SM_CXMENUCHECK) + ::GetSystemMetrics(SM_CXEDGE) + szText.cx + tm.tmAveCharWidth;
    pComboBox->MoveWindow(r);
}
