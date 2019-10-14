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
#include "mplayerc.h"
#include "PPageFileInfoDetails.h"
#include <atlbase.h>
#include "DSUtil.h"
#include "text.h"
#include <d3d9.h>
#include <vmr9.h>
#include "moreuuids.h"
#include "../SubPic/ISubPic.h"


// CPPageFileInfoDetails dialog

IMPLEMENT_DYNAMIC(CPPageFileInfoDetails, CMPCThemePropertyPage)
CPPageFileInfoDetails::CPPageFileInfoDetails(CString path, IFilterGraph* pFG, ISubPicAllocatorPresenter* pCAP, IFileSourceFilter* pFSF, IDvdInfo2* pDVDI)
    : CMPCThemePropertyPage(CPPageFileInfoDetails::IDD, CPPageFileInfoDetails::IDD)
    , m_hIcon(nullptr)
    , m_fn(path)
    , m_path(path)
    , m_type(StrRes(IDS_AG_NOT_KNOWN))
    , m_size(StrRes(IDS_AG_NOT_KNOWN))
    , m_duration(StrRes(IDS_AG_NOT_KNOWN))
    , m_resolution(StrRes(IDS_AG_NOT_KNOWN))
    , m_creationDate(StrRes(IDS_AG_NOT_KNOWN))
{
    if (pFSF) {
        CComHeapPtr<OLECHAR> pFN;
        if (SUCCEEDED(pFSF->GetCurFile(&pFN, nullptr))) {
            m_fn = pFN;
        }
    } else if (pDVDI) {
        ULONG len = 0;
        if (SUCCEEDED(pDVDI->GetDVDDirectory(m_path.GetBufferSetLength(MAX_PATH), MAX_PATH, &len)) && len) {
            m_path.ReleaseBuffer();
            m_fn = m_path += _T("\\VIDEO_TS.IFO");
        }
    }

    auto getProperty = [](IFilterGraph * pFG, LPCOLESTR propName, VARIANT * vt) {
        BeginEnumFilters(pFG, pEF, pBF) {
            if (CComQIPtr<IPropertyBag> pPB = pBF)
                if (SUCCEEDED(pPB->Read(propName, vt, nullptr))) {
                    return true;
                }
        }
        EndEnumFilters;

        return false;
    };

    auto formatDateTime = [](FILETIME tm) {
        SYSTEMTIME st, stLocal;
        VERIFY(FileTimeToSystemTime(&tm, &st));
        VERIFY(SystemTimeToTzSpecificLocalTime(nullptr, &st, &stLocal));

        CString formatedDateTime;
        // Compute the size need to hold the formated date and time
        int nLenght = GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &stLocal, nullptr, nullptr, 0);
        nLenght += GetTimeFormat(LOCALE_USER_DEFAULT, 0, &stLocal, nullptr, nullptr, 0);

        LPTSTR szFormatedDateTime = formatedDateTime.GetBuffer(nLenght);
        int nDateLenght = GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &stLocal, nullptr, szFormatedDateTime, nLenght);
        if (nDateLenght > 0) {
            szFormatedDateTime[nDateLenght - 1] = _T(' '); // Replace the end of string character by a space
            GetTimeFormat(LOCALE_USER_DEFAULT, 0, &stLocal, nullptr, &szFormatedDateTime[nDateLenght], nLenght - nDateLenght);
        }
        formatedDateTime.ReleaseBuffer();

        return formatedDateTime;
    };

    CString creationDate;
    CComVariant vt;
    if (getProperty(pFG, OLESTR("CurFile.TimeCreated"), &vt)) {
        if (V_VT(&vt) == VT_UI8) {
            ULARGE_INTEGER uli;
            uli.QuadPart = V_UI8(&vt);

            FILETIME ft;
            ft.dwLowDateTime = uli.LowPart;
            ft.dwHighDateTime = uli.HighPart;

            creationDate = formatDateTime(ft);
        }
    }

    LONGLONG size = 0;
    if (CComQIPtr<IBaseFilter> pBF = pFSF) {
        BeginEnumPins(pBF, pEP, pPin) {
            if (CComQIPtr<IAsyncReader> pAR = pPin) {
                LONGLONG total, available;
                if (SUCCEEDED(pAR->Length(&total, &available))) {
                    size = total;
                    break;
                }
            }
        }
        EndEnumPins;
    }

    WIN32_FIND_DATA wfd;
    HANDLE hFind = FindFirstFile(m_path, &wfd);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);

        if (size == 0) {
            size = (__int64(wfd.nFileSizeHigh) << 32) | wfd.nFileSizeLow;
        }

        if (creationDate.IsEmpty()) {
            creationDate = formatDateTime(wfd.ftCreationTime);
        }
    }

    if (size > 0) {
        const UINT MAX_FILE_SIZE_BUFFER = 16;
        WCHAR szFileSize[MAX_FILE_SIZE_BUFFER];
        StrFormatByteSizeW(size, szFileSize, MAX_FILE_SIZE_BUFFER);
        CString szByteSize;
        szByteSize.Format(_T("%I64d"), size);
        m_size.Format(_T("%s (%s %s)"), szFileSize, FormatNumber(szByteSize).GetString(), ResStr(IDS_SIZE_UNIT_BYTES).GetString());
    }

    if (!creationDate.IsEmpty()) {
        m_creationDate = creationDate;
    }

    REFERENCE_TIME rtDur = 0;
    CComQIPtr<IMediaSeeking> pMS = pFG;
    if (pMS && SUCCEEDED(pMS->GetDuration(&rtDur)) && rtDur > 0) {
        m_duration = ReftimeToString2(rtDur);
    }

    CSize wh, arxy;

    if (pCAP) {
        wh = pCAP->GetVideoSize(false);
        arxy = pCAP->GetVideoSize(true);
    } else {
        if (CComQIPtr<IBasicVideo> pBV = pFG) {
            if (SUCCEEDED(pBV->GetVideoSize(&wh.cx, &wh.cy))) {
                if (CComQIPtr<IBasicVideo2> pBV2 = pFG) {
                    pBV2->GetPreferredAspectRatio(&arxy.cx, &arxy.cy);
                }
            } else {
                wh.SetSize(0, 0);
            }
        }

        if (wh.cx == 0 && wh.cy == 0) {
            BeginEnumFilters(pFG, pEF, pBF) {
                if (CComQIPtr<IBasicVideo> pBV = pBF) {
                    pBV->GetVideoSize(&wh.cx, &wh.cy);
                    if (CComQIPtr<IBasicVideo2> pBV2 = pBF) {
                        pBV2->GetPreferredAspectRatio(&arxy.cx, &arxy.cy);
                    }
                    break;
                } else if (CComQIPtr<IVMRWindowlessControl> pWC = pBF) {
                    pWC->GetNativeVideoSize(&wh.cx, &wh.cy, &arxy.cx, &arxy.cy);
                    break;
                } else if (CComQIPtr<IVMRWindowlessControl9> pWC9 = pBF) {
                    pWC9->GetNativeVideoSize(&wh.cx, &wh.cy, &arxy.cx, &arxy.cy);
                    break;
                }
            }
            EndEnumFilters;
        }
    }

    if (wh.cx > 0 && wh.cy > 0) {
        m_resolution.Format(_T("%dx%d"), wh.cx, wh.cy);

        int gcd = GCD(arxy.cx, arxy.cy);
        if (gcd > 1) {
            arxy.cx /= gcd;
            arxy.cy /= gcd;
        }

        if (arxy.cx > 0 && arxy.cy > 0 && arxy.cx * wh.cy != arxy.cy * wh.cx) {
            m_resolution.AppendFormat(_T(" (") + ResStr(IDS_ASPECT_RATIO_FMT) + _T(")"), arxy.cx, arxy.cy);
        }
    }

    InitTrackInfoText(pFG);
}

void CPPageFileInfoDetails::InitTrackInfoText(IFilterGraph* pFG)
{
    CAtlList<CString> videoStreams;
    CAtlList<CString> otherStreams;

    auto addStream = [&](const AM_MEDIA_TYPE & mt, const CString & str) {
        if (mt.majortype == MEDIATYPE_Video) {
            videoStreams.AddTail(str);
        } else {
            otherStreams.AddTail(str);
        }
    };

    BeginEnumFilters(pFG, pEF, pBF) {
        CComPtr<IBaseFilter> pUSBF = GetUpStreamFilter(pBF);

        if (GetCLSID(pBF) == CLSID_NetShowSource) {
            continue;
        } else if (GetCLSID(pUSBF) != CLSID_NetShowSource) {
            if (IPin* pPin = GetFirstPin(pBF, PINDIR_INPUT)) {
                CMediaType mt;
                if (FAILED(pPin->ConnectionMediaType(&mt)) || mt.majortype != MEDIATYPE_Stream) {
                    continue;
                }
            }

            if (IPin* pPin = GetFirstPin(pBF, PINDIR_OUTPUT)) {
                CMediaType mt;
                if (SUCCEEDED(pPin->ConnectionMediaType(&mt)) && mt.majortype == MEDIATYPE_Stream) {
                    continue;
                }
            }
        }

        bool bUsePins = true;

        // If the filter claims to have tracks, we use that
        if (CComQIPtr<IAMStreamSelect> pSS = pBF) {
            DWORD nCount;
            if (FAILED(pSS->Count(&nCount))) {
                nCount = 0;
            }

            for (DWORD i = 0; i < nCount; i++) {
                AM_MEDIA_TYPE* pmt = nullptr;
                WCHAR* pszName = nullptr;
                if (SUCCEEDED(pSS->Info(i, &pmt, nullptr, nullptr, nullptr, &pszName, nullptr, nullptr)) && pmt) {
                    CMediaTypeEx mt = *pmt;
                    CString str = mt.ToString();

                    if (!str.IsEmpty()) {
                        if (pszName && wcslen(pszName)) {
                            str.AppendFormat(_T(" [%s]"), pszName);
                        }
                        addStream(mt, str);
                        bUsePins = false;
                    }
                }
                DeleteMediaType(pmt);
                CoTaskMemFree(pszName);
            }
        }
        // We fall back to listing the pins only if we could not get any info from IAMStreamSelect
        if (bUsePins) {
            BeginEnumPins(pBF, pEP, pPin) {
                CMediaTypeEx mt;
                PIN_DIRECTION dir;
                if (SUCCEEDED(pPin->QueryDirection(&dir)) && dir == PINDIR_OUTPUT
                        && SUCCEEDED(pPin->ConnectionMediaType(&mt))) {
                    CString str = mt.ToString();

                    if (!str.IsEmpty()) {
                        addStream(mt, str + CString(L" [" + GetPinName(pPin) + L"]"));
                    }
                }
            }
            EndEnumPins;
        }
    }
    EndEnumFilters;

    m_trackInfo = Implode(videoStreams, _T("\r\n"));
    if (!videoStreams.IsEmpty() && !otherStreams.IsEmpty()) {
        m_trackInfo += _T("\r\n");
    }
    m_trackInfo += Implode(otherStreams, _T("\r\n"));
}

CPPageFileInfoDetails::~CPPageFileInfoDetails()
{
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
    }
}

void CPPageFileInfoDetails::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEFAULTICON, m_icon);
    DDX_Text(pDX, IDC_EDIT1, m_fn);
    DDX_Text(pDX, IDC_EDIT4, m_type);
    DDX_Text(pDX, IDC_EDIT3, m_size);
    DDX_Text(pDX, IDC_EDIT2, m_duration);
    DDX_Text(pDX, IDC_EDIT5, m_resolution);
    DDX_Text(pDX, IDC_EDIT6, m_creationDate);
    DDX_Text(pDX, IDC_EDIT7, m_trackInfo);
}

BEGIN_MESSAGE_MAP(CPPageFileInfoDetails, CMPCThemePropertyPage)
END_MESSAGE_MAP()

// CPPageFileInfoDetails message handlers

BOOL CPPageFileInfoDetails::OnInitDialog()
{
    __super::OnInitDialog();

    if (m_path.IsEmpty()) {
        m_path = m_fn;
    }

    m_fn.TrimRight('/');
    m_fn.Replace('\\', '/');
    m_fn = m_fn.Mid(m_fn.ReverseFind('/') + 1);

    CString ext = m_fn.Left(m_fn.Find(_T("://")) + 1).TrimRight(':');
    if (ext.IsEmpty() || !ext.CompareNoCase(_T("file"))) {
        ext = _T(".") + m_fn.Mid(m_fn.ReverseFind('.') + 1);
    }

    m_hIcon = LoadIcon(m_fn, false);
    if (m_hIcon) {
        m_icon.SetIcon(m_hIcon);
    }

    if (!LoadType(ext, m_type)) {
        m_type.LoadString(IDS_AG_NOT_KNOWN);
    }

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageFileInfoDetails::OnSetActive()
{
    BOOL ret = __super::OnSetActive();

    PostMessage(WM_NEXTDLGCTL, (WPARAM)GetParentSheet()->GetTabControl()->GetSafeHwnd(), TRUE);
    GetDlgItem(IDC_EDIT1)->PostMessage(WM_KEYDOWN, VK_HOME);

    return ret;
}
