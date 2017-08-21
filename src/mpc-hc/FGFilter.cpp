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
#include <mpconfig.h>
#include "FGFilter.h"
#include "MainFrm.h"
#include "DSUtil.h"
#include "IPinHook.h" // For the NVIDIA driver bug work-around
#include "moreuuids.h"
#include <mvrInterfaces.h>

#include <initguid.h>
#include "AllocatorCommon.h"
#include "SyncAllocatorPresenter.h"


//
// CFGFilter
//

CFGFilter::CFGFilter(const CLSID& clsid, CStringW name, UINT64 merit)
    : m_clsid(clsid)
    , m_name(name)
{
    m_merit.val = merit;
}

CFGFilter::~CFGFilter()
{
}

const CAtlList<GUID>& CFGFilter::GetTypes() const
{
    return m_types;
}

void CFGFilter::SetTypes(const CAtlList<GUID>& types)
{
    m_types.RemoveAll();
    m_types.AddTailList(&types);
}

void CFGFilter::AddType(const GUID& majortype, const GUID& subtype)
{
    m_types.AddTail(majortype);
    m_types.AddTail(subtype);
}

bool CFGFilter::CheckTypes(const CAtlArray<GUID>& types, bool fExactMatch)
{
    POSITION pos = m_types.GetHeadPosition();
    while (pos) {
        const GUID& majortype = m_types.GetNext(pos);
        if (!pos) {
            ASSERT(0);
            break;
        }
        const GUID& subtype = m_types.GetNext(pos);

        for (int i = 0, len = types.GetCount() & ~1; i < len; i += 2) {
            if (fExactMatch) {
                if (majortype == types[i] && majortype != GUID_NULL
                        && subtype == types[i + 1] && subtype != GUID_NULL) {
                    return true;
                }
            } else {
                if ((majortype == GUID_NULL || types[i] == GUID_NULL || majortype == types[i])
                        && (subtype == GUID_NULL || types[i + 1] == GUID_NULL || subtype == types[i + 1])) {
                    return true;
                }
            }
        }
    }

    return false;
}

//
// CFGFilterRegistry
//

CFGFilterRegistry::CFGFilterRegistry(IMoniker* pMoniker, UINT64 merit)
    : CFGFilter(GUID_NULL, L"", merit)
    , m_pMoniker(pMoniker)
{
    if (!m_pMoniker) {
        return;
    }

    CComHeapPtr<OLECHAR> str;
    if (FAILED(m_pMoniker->GetDisplayName(0, 0, &str))) {
        return;
    }
    m_DisplayName = m_name = str;

    QueryProperties();

    if (merit != MERIT64_DO_USE) {
        m_merit.val = merit;
    }
}

CFGFilterRegistry::CFGFilterRegistry(CStringW DisplayName, UINT64 merit)
    : CFGFilter(GUID_NULL, L"", merit)
    , m_DisplayName(DisplayName)
{
    if (m_DisplayName.IsEmpty()) {
        return;
    }

    CComPtr<IBindCtx> pBC;
    CreateBindCtx(0, &pBC);

    ULONG chEaten;
    if (S_OK != MkParseDisplayName(pBC, CComBSTR(m_DisplayName), &chEaten, &m_pMoniker)) {
        return;
    }

    QueryProperties();

    if (merit != MERIT64_DO_USE) {
        m_merit.val = merit;
    }
}

void CFGFilterRegistry::QueryProperties()
{
    ASSERT(m_pMoniker);
    CComPtr<IPropertyBag> pPB;
    if (SUCCEEDED(m_pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPB)))) {
        CComVariant var;
        if (SUCCEEDED(pPB->Read(_T("FriendlyName"), &var, nullptr))) {
            m_name = var.bstrVal;
            var.Clear();
        }

        if (SUCCEEDED(pPB->Read(_T("CLSID"), &var, nullptr))) {
            CLSIDFromString(var.bstrVal, &m_clsid);
            var.Clear();
        }

        if (SUCCEEDED(pPB->Read(_T("FilterData"), &var, nullptr))) {
            BSTR* pstr;
            if (SUCCEEDED(SafeArrayAccessData(var.parray, (void**)&pstr))) {
                ExtractFilterData((BYTE*)pstr, var.parray->cbElements * (var.parray->rgsabound[0].cElements));
                SafeArrayUnaccessData(var.parray);
            }

            var.Clear();
        }
    }
}

CFGFilterRegistry::CFGFilterRegistry(const CLSID& clsid, UINT64 merit)
    : CFGFilter(clsid, L"", merit)
{
    if (m_clsid == GUID_NULL) {
        return;
    }

    CString guid = CStringFromGUID(m_clsid);

    CRegKey key;

    if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("CLSID\\") + guid, KEY_READ)) {
        ULONG nChars = 0;
        if (ERROR_SUCCESS == key.QueryStringValue(nullptr, nullptr, &nChars)) {
            CString name;
            if (ERROR_SUCCESS == key.QueryStringValue(nullptr, name.GetBuffer(nChars), &nChars)) {
                name.ReleaseBuffer(nChars);
                m_name = name;
            }
        }

        key.Close();
    }

    CRegKey catkey;

    if (ERROR_SUCCESS == catkey.Open(HKEY_CLASSES_ROOT, _T("CLSID\\{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance"), KEY_READ)) {
        if (ERROR_SUCCESS != key.Open(catkey, guid, KEY_READ)) {
            // illiminable pack uses the name of the filter and not the clsid, have to enum all keys to find it...

            FILETIME ft;
            TCHAR buff[256];
            DWORD len = _countof(buff);
            for (DWORD i = 0; ERROR_SUCCESS == catkey.EnumKey(i, buff, &len, &ft); i++, len = _countof(buff)) {
                if (ERROR_SUCCESS == key.Open(catkey, buff, KEY_READ)) {
                    TCHAR clsidString[256];
                    len = _countof(clsidString);
                    if (ERROR_SUCCESS == key.QueryStringValue(_T("CLSID"), clsidString, &len)
                            && GUIDFromCString(clsidString) == m_clsid) {
                        break;
                    }

                    key.Close();
                }
            }
        }

        if (key) {
            ULONG nChars = 0;
            if (ERROR_SUCCESS == key.QueryStringValue(_T("FriendlyName"), nullptr, &nChars)) {
                CString name;
                if (ERROR_SUCCESS == key.QueryStringValue(_T("FriendlyName"), name.GetBuffer(nChars), &nChars)) {
                    name.ReleaseBuffer(nChars);
                    m_name = name;
                }
            }

            ULONG nBytes = 0;
            if (ERROR_SUCCESS == key.QueryBinaryValue(_T("FilterData"), nullptr, &nBytes)) {
                CAutoVectorPtr<BYTE> buff;
                if (buff.Allocate(nBytes) && ERROR_SUCCESS == key.QueryBinaryValue(_T("FilterData"), buff, &nBytes)) {
                    ExtractFilterData(buff, nBytes);
                }
            }

            key.Close();
        }
    }

    if (merit != MERIT64_DO_USE) {
        m_merit.val = merit;
    }
}

HRESULT CFGFilterRegistry::Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    CheckPointer(ppBF, E_POINTER);

    HRESULT hr = E_FAIL;

    if (m_pMoniker) {
        if (SUCCEEDED(hr = m_pMoniker->BindToObject(0, 0, IID_PPV_ARGS(ppBF)))) {
            m_clsid = ::GetCLSID(*ppBF);
        }
    } else if (m_clsid != GUID_NULL) {
        CComQIPtr<IBaseFilter> pBF;

        if (FAILED(pBF.CoCreateInstance(m_clsid))) {
            return E_FAIL;
        }

        *ppBF = pBF.Detach();

        hr = S_OK;
    }

    return hr;
};

interface __declspec(uuid("97f7c4d4-547b-4a5f-8332-536430ad2e4d"))
    IAMFilterData :
    public IUnknown
{
    STDMETHOD(ParseFilterData)(BYTE* rgbFilterData, ULONG cb, BYTE** prgbRegFilter2) PURE;
    STDMETHOD(CreateFilterData)(REGFILTER2* prf2, BYTE** prgbFilterData, ULONG* pcb) PURE;
};

void CFGFilterRegistry::ExtractFilterData(BYTE* p, UINT len)
{
    CComPtr<IAMFilterData> pFD;
    BYTE* ptr = nullptr;

    if (SUCCEEDED(pFD.CoCreateInstance(CLSID_FilterMapper2))
            && SUCCEEDED(pFD->ParseFilterData(p, len, (BYTE**)&ptr))) {
        REGFILTER2* prf = (REGFILTER2*) * (WPARAM*)ptr; // this is f*cked up

        m_merit.mid = prf->dwMerit;

        if (prf->dwVersion == 1) {
            for (UINT i = 0; i < prf->cPins; i++) {
                if (prf->rgPins[i].bOutput) {
                    continue;
                }

                for (UINT j = 0; j < prf->rgPins[i].nMediaTypes; j++) {
                    if (!prf->rgPins[i].lpMediaType[j].clsMajorType || !prf->rgPins[i].lpMediaType[j].clsMinorType) {
                        break;
                    }

                    const REGPINTYPES& rpt = prf->rgPins[i].lpMediaType[j];
                    AddType(*rpt.clsMajorType, *rpt.clsMinorType);
                }
            }
        } else if (prf->dwVersion == 2) {
            for (UINT i = 0; i < prf->cPins2; i++) {
                if (prf->rgPins2[i].dwFlags & REG_PINFLAG_B_OUTPUT) {
                    continue;
                }

                for (UINT j = 0; j < prf->rgPins2[i].nMediaTypes; j++) {
                    if (!prf->rgPins2[i].lpMediaType[j].clsMajorType || !prf->rgPins2[i].lpMediaType[j].clsMinorType) {
                        break;
                    }

                    const REGPINTYPES& rpt = prf->rgPins2[i].lpMediaType[j];
                    AddType(*rpt.clsMajorType, *rpt.clsMinorType);
                }
            }
        }

        CoTaskMemFree(prf);
    } else {
        BYTE* base = p;

#define ChkLen(size) if (p - base + size > (int)len) return;

        ChkLen(4)
        if (*(DWORD*)p != 0x00000002) {
            return;    // only version 2 supported, no samples found for 1
        }
        p += 4;

        ChkLen(4)
        m_merit.mid = *(DWORD*)p;
        p += 4;

        m_types.RemoveAll();

        ChkLen(8)
        DWORD nPins = *(DWORD*)p;
        p += 8;
        while (nPins-- > 0) {
            ChkLen(1)
            BYTE n = *p - 0x30;
            p++;
            UNREFERENCED_PARAMETER(n);

            ChkLen(2)
            WORD pi = *(WORD*)p;
            p += 2;
            ASSERT(pi == 'ip');
            UNREFERENCED_PARAMETER(pi);

            ChkLen(1)
            BYTE x33 = *p;
            p++;
            ASSERT(x33 == 0x33);
            UNREFERENCED_PARAMETER(x33);

            ChkLen(8)
            bool fOutput = !!(*p & REG_PINFLAG_B_OUTPUT);
            p += 8;

            ChkLen(12)
            DWORD nTypes = *(DWORD*)p;
            p += 12;
            while (nTypes-- > 0) {
                ChkLen(1)
                n = *p - 0x30;
                p++;
                UNREFERENCED_PARAMETER(n);

                ChkLen(2)
                WORD ty = *(WORD*)p;
                p += 2;
                ASSERT(ty == 'yt');
                UNREFERENCED_PARAMETER(ty);

                ChkLen(5)
                x33 = *p;
                p++;
                ASSERT(x33 == 0x33);
                UNREFERENCED_PARAMETER(x33);
                p += 4;

                ChkLen(8)
                if (*(DWORD*)p < (DWORD)(p - base + 8) || *(DWORD*)p >= len
                        || *(DWORD*)(p + 4) < (DWORD)(p - base + 8) || *(DWORD*)(p + 4) >= len) {
                    p += 8;
                    continue;
                }

                GUID majortype, subtype;
                memcpy(&majortype, &base[*(DWORD*)p], sizeof(GUID));
                p += 4;
                if (!fOutput) {
                    AddType(majortype, subtype);
                }
            }
        }

#undef ChkLen
    }
}

//
// CFGFilterFile
//

CFGFilterFile::CFGFilterFile(const CLSID& clsid, CString path, CStringW name, UINT64 merit)
    : CFGFilter(clsid, name, merit)
    , m_path(path)
    , m_hInst(nullptr)
{
}

HRESULT CFGFilterFile::Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    CheckPointer(ppBF, E_POINTER);

    return LoadExternalFilter(m_path, m_clsid, ppBF);
}

//
// CFGFilterVideoRenderer
//

CFGFilterVideoRenderer::CFGFilterVideoRenderer(HWND hWnd, const CLSID& clsid, CStringW name, UINT64 merit)
    : CFGFilter(clsid, name, merit)
    , m_hWnd(hWnd)
    , m_bHas10BitWorkAround(false)
{
    AddType(MEDIATYPE_Video, MEDIASUBTYPE_NULL);
}

CFGFilterVideoRenderer::~CFGFilterVideoRenderer()
{
    if (m_bHas10BitWorkAround) {
        UnhookWorkAround10BitBug();
    }
}

HRESULT CFGFilterVideoRenderer::Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    TRACE(_T("--> CFGFilterVideoRenderer::Create on thread: %lu\n"), GetCurrentThreadId());
    CheckPointer(ppBF, E_POINTER);

    HRESULT hr;
    CComPtr<ISubPicAllocatorPresenter> pCAP;

    auto isD3DFullScreenMode = []() {
        auto pMainFrame = dynamic_cast<const CMainFrame*>(AfxGetApp()->m_pMainWnd);
        ASSERT(pMainFrame);
        return pMainFrame && pMainFrame->IsD3DFullScreenMode();
    };

    if (m_clsid == CLSID_EVRAllocatorPresenter) {
        CheckNoLog(CreateEVR(m_clsid, m_hWnd, isD3DFullScreenMode(), &pCAP));
    } else if (m_clsid == CLSID_SyncAllocatorPresenter) {
        CheckNoLog(CreateSyncRenderer(m_clsid, m_hWnd, isD3DFullScreenMode(), &pCAP));
    } else if (m_clsid == CLSID_madVRAllocatorPresenter) {
        CheckNoLog(CreateAP9(m_clsid, m_hWnd, isD3DFullScreenMode(), &pCAP));

        if (CComQIPtr<IMadVRSubclassReplacement> pMVRSR = pCAP) {
            VERIFY(SUCCEEDED(pMVRSR->DisableSubclassing()));
        }
        // madVR supports calling IVideoWindow::put_Owner before the pins are connected
        if (CComQIPtr<IVideoWindow> pVW = pCAP) {
            VERIFY(SUCCEEDED(pVW->put_Owner((OAHWND)m_hWnd)));
        }
    } else if (m_clsid == CLSID_VMR9AllocatorPresenter || m_clsid == CLSID_DXRAllocatorPresenter) {
        CheckNoLog(CreateAP9(m_clsid, m_hWnd, isD3DFullScreenMode(), &pCAP));
    } else {
        CComPtr<IBaseFilter> pBF;
        CheckNoLog(pBF.CoCreateInstance(m_clsid));

        if (m_clsid == CLSID_EnhancedVideoRenderer) {
            CComQIPtr<IEVRFilterConfig> pConfig = pBF;
            pConfig->SetNumberOfStreams(3);

            if (CComQIPtr<IMFGetService> pMFGS = pBF) {
                CComPtr<IMFVideoDisplayControl> pMFVDC;
                if (SUCCEEDED(pMFGS->GetService(MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&pMFVDC)))) {
                    pMFVDC->SetVideoWindow(m_hWnd);
                }
            }
        }

        BeginEnumPins(pBF, pEP, pPin) {
            if (CComQIPtr<IMixerPinConfig, &IID_IMixerPinConfig> pMPC = pPin) {
                pUnks.AddTail(pMPC);
                break;
            }
        }
        EndEnumPins;

        *ppBF = pBF.Detach();
    }

    if (pCAP) {
        CComPtr<IUnknown> pRenderer;
        CheckNoLog(pCAP->CreateRenderer(&pRenderer));

        *ppBF = CComQIPtr<IBaseFilter>(pRenderer).Detach();
        pUnks.AddTail(pCAP);
        if (CComQIPtr<ISubPicAllocatorPresenter2> pCAP2 = pCAP) {
            pUnks.AddTail(pCAP2);
        }
    }

    CheckPointer(*ppBF, E_FAIL);

    if (m_clsid != CLSID_madVRAllocatorPresenter) {
        HookWorkAround10BitBug(*ppBF);
        m_bHas10BitWorkAround = true;
    }

    return hr;
}

//
// CFGFilterList
//

CFGFilterList::CFGFilterList()
{
}

CFGFilterList::~CFGFilterList()
{
    RemoveAll();
}

void CFGFilterList::RemoveAll()
{
    while (!m_filters.IsEmpty()) {
        const filter_t& f = m_filters.RemoveHead();
        if (f.autodelete) {
            delete f.pFGF;
        }
    }

    m_sortedfilters.RemoveAll();
}

void CFGFilterList::Insert(CFGFilter* pFGF, int group, bool exactmatch, bool autodelete)
{
    bool bInsert = true;

    TRACE(_T("FGM: Inserting %d %d %016I64x '%s' --> "), group, exactmatch, pFGF->GetMerit(),
          pFGF->GetName().IsEmpty() ? CStringFromGUID(pFGF->GetCLSID()).GetString() : CString(pFGF->GetName()).GetString());

    CFGFilterRegistry* pFGFR = dynamic_cast<CFGFilterRegistry*>(pFGF);

    POSITION pos = m_filters.GetHeadPosition();
    while (pos) {
        filter_t& f = m_filters.GetNext(pos);

        if (pFGF == f.pFGF) {
            TRACE(_T("Rejected (exact duplicate)\n"));
            bInsert = false;
            break;
        }

        if (group != f.group) {
            continue;
        }

        if (pFGF->GetCLSID() != GUID_NULL && pFGF->GetCLSID() == f.pFGF->GetCLSID()
                && f.pFGF->GetMerit() == MERIT64_DO_NOT_USE) {
            TRACE(_T("Rejected (same filter with merit DO_NOT_USE already in the list)\n"));
            bInsert = false;
            break;
        }

        if (CFGFilterRegistry* pFGFR2 = dynamic_cast<CFGFilterRegistry*>(f.pFGF)) {
            if (pFGFR && pFGFR->GetMoniker() && pFGFR2->GetMoniker() && S_OK == pFGFR->GetMoniker()->IsEqual(pFGFR2->GetMoniker())) {
                TRACE(_T("Rejected (duplicated moniker)\n"));
                bInsert = false;
                break;
            }
        }
    }

    if (bInsert) {
        TRACE(_T("Success\n"));

        filter_t f = {(int)m_filters.GetCount(), pFGF, group, exactmatch, autodelete};
        m_filters.AddTail(f);

        m_sortedfilters.RemoveAll();
    } else if (autodelete) {
        delete pFGF;
    }
}

POSITION CFGFilterList::GetHeadPosition()
{
    if (m_sortedfilters.IsEmpty()) {
        CAtlArray<filter_t> sort;
        sort.SetCount(m_filters.GetCount());
        POSITION pos = m_filters.GetHeadPosition();
        for (int i = 0; pos; i++) {
            sort[i] = m_filters.GetNext(pos);
        }
        std::sort(sort.GetData(), sort.GetData() + sort.GetCount());
        for (size_t i = 0; i < sort.GetCount(); i++) {
            if (sort[i].pFGF->GetMerit() >= MERIT64_DO_USE) {
                m_sortedfilters.AddTail(sort[i].pFGF);
            }
        }
    }

#ifdef _DEBUG
    TRACE(_T("FGM: Sorting filters\n"));

    POSITION pos = m_sortedfilters.GetHeadPosition();
    while (pos) {
        CFGFilter* pFGF = m_sortedfilters.GetNext(pos);
        TRACE(_T("FGM: - %016I64x '%s'\n"), pFGF->GetMerit(), pFGF->GetName().IsEmpty() ? CStringFromGUID(pFGF->GetCLSID()).GetString() : CString(pFGF->GetName()).GetString());
    }
#endif

    return m_sortedfilters.GetHeadPosition();
}

CFGFilter* CFGFilterList::GetNext(POSITION& pos)
{
    return m_sortedfilters.GetNext(pos);
}
