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
#include "FakeFilterMapper2.h"
#include "MacrovisionKicker.h"
#include "DSUtil.h"

#include "MhookHelper.h"


HRESULT(__stdcall* Real_CoCreateInstance)(CONST IID& a0,
                                          LPUNKNOWN a1,
                                          DWORD a2,
                                          CONST IID& a3,
                                          LPVOID* a4)
    = CoCreateInstance;

LONG(WINAPI* Real_RegCreateKeyExA)(HKEY a0,
                                   LPCSTR a1,
                                   DWORD a2,
                                   LPSTR a3,
                                   DWORD a4,
                                   REGSAM a5,
                                   LPSECURITY_ATTRIBUTES a6,
                                   PHKEY a7,
                                   LPDWORD a8)
    = RegCreateKeyExA;

LONG(WINAPI* Real_RegCreateKeyExW)(HKEY a0,
                                   LPCWSTR a1,
                                   DWORD a2,
                                   LPWSTR a3,
                                   DWORD a4,
                                   REGSAM a5,
                                   LPSECURITY_ATTRIBUTES a6,
                                   PHKEY a7,
                                   LPDWORD a8)
    = RegCreateKeyExW;

LONG(WINAPI* Real_RegDeleteKeyA)(HKEY a0,
                                 LPCSTR a1)
    = RegDeleteKeyA;

LONG(WINAPI* Real_RegDeleteKeyW)(HKEY a0,
                                 LPCWSTR a1)
    = RegDeleteKeyW;

LONG(WINAPI* Real_RegDeleteValueA)(HKEY a0,
                                   LPCSTR a1)
    = RegDeleteValueA;


LONG(WINAPI* Real_RegDeleteValueW)(HKEY a0,
                                   LPCWSTR a1)
    = RegDeleteValueW;

LONG(WINAPI* Real_RegEnumKeyExA)(HKEY a0,
                                 DWORD a1,
                                 LPSTR a2,
                                 LPDWORD a3,
                                 LPDWORD a4,
                                 LPSTR a5,
                                 LPDWORD a6,
                                 struct _FILETIME* a7)
    = RegEnumKeyExA;

LONG(WINAPI* Real_RegEnumKeyExW)(HKEY a0,
                                 DWORD a1,
                                 LPWSTR a2,
                                 LPDWORD a3,
                                 LPDWORD a4,
                                 LPWSTR a5,
                                 LPDWORD a6,
                                 struct _FILETIME* a7)
    = RegEnumKeyExW;

LONG(WINAPI* Real_RegEnumValueA)(HKEY a0,
                                 DWORD a1,
                                 LPSTR a2,
                                 LPDWORD a3,
                                 LPDWORD a4,
                                 LPDWORD a5,
                                 LPBYTE a6,
                                 LPDWORD a7)
    = RegEnumValueA;

LONG(WINAPI* Real_RegEnumValueW)(HKEY a0,
                                 DWORD a1,
                                 LPWSTR a2,
                                 LPDWORD a3,
                                 LPDWORD a4,
                                 LPDWORD a5,
                                 LPBYTE a6,
                                 LPDWORD a7)
    = RegEnumValueW;

LONG(WINAPI* Real_RegOpenKeyExA)(HKEY a0,
                                 LPCSTR a1,
                                 DWORD a2,
                                 REGSAM a3,
                                 PHKEY a4)
    = RegOpenKeyExA;

LONG(WINAPI* Real_RegOpenKeyExW)(HKEY a0,
                                 LPCWSTR a1,
                                 DWORD a2,
                                 REGSAM a3,
                                 PHKEY a4)
    = RegOpenKeyExW;

LONG(WINAPI* Real_RegQueryInfoKeyA)(HKEY a0,
                                    LPSTR a1,
                                    LPDWORD a2,
                                    LPDWORD a3,
                                    LPDWORD a4,
                                    LPDWORD a5,
                                    LPDWORD a6,
                                    LPDWORD a7,
                                    LPDWORD a8,
                                    LPDWORD a9,
                                    LPDWORD a10,
                                    struct _FILETIME* a11)
    = RegQueryInfoKeyA;

LONG(WINAPI* Real_RegQueryInfoKeyW)(HKEY a0,
                                    LPWSTR a1,
                                    LPDWORD a2,
                                    LPDWORD a3,
                                    LPDWORD a4,
                                    LPDWORD a5,
                                    LPDWORD a6,
                                    LPDWORD a7,
                                    LPDWORD a8,
                                    LPDWORD a9,
                                    LPDWORD a10,
                                    struct _FILETIME* a11)
    = RegQueryInfoKeyW;

LONG(WINAPI* Real_RegQueryValueExA)(HKEY a0,
                                    LPCSTR a1,
                                    LPDWORD a2,
                                    LPDWORD a3,
                                    LPBYTE a4,
                                    LPDWORD a5)
    = RegQueryValueExA;

LONG(WINAPI* Real_RegQueryValueExW)(HKEY a0,
                                    LPCWSTR a1,
                                    LPDWORD a2,
                                    LPDWORD a3,
                                    LPBYTE a4,
                                    LPDWORD a5)
    = RegQueryValueExW;

LONG(WINAPI* Real_RegSetValueExA)(HKEY a0,
                                  LPCSTR a1,
                                  DWORD a2,
                                  DWORD a3,
                                  const BYTE* a4,
                                  DWORD a5)
    = RegSetValueExA;

LONG(WINAPI* Real_RegSetValueExW)(HKEY a0,
                                  LPCWSTR a1,
                                  DWORD a2,
                                  DWORD a3,
                                  const BYTE* a4,
                                  DWORD a5)
    = RegSetValueExW;


LONG(WINAPI* Real_RegCloseKey)(HKEY a0)
    = RegCloseKey;

LONG(WINAPI* Real_RegFlushKey)(HKEY a0)
    = RegFlushKey;

LONG(WINAPI* Real_RegCreateKeyA)(HKEY a0, LPCSTR a1, PHKEY a2)
    = RegCreateKeyA;

LONG(WINAPI* Real_RegCreateKeyW)(HKEY a0, LPCWSTR a1, PHKEY a2)
    = RegCreateKeyW;

LONG(WINAPI* Real_RegOpenKeyA)(HKEY a0, LPCSTR a1, PHKEY a2)
    = RegOpenKeyA;

LONG(WINAPI* Real_RegOpenKeyW)(HKEY a0, LPCWSTR a1, PHKEY a2)
    = RegOpenKeyW;

LONG(WINAPI* Real_RegQueryValueA)(HKEY a0, LPCSTR a1, LPSTR a2, PLONG a3)
    = RegQueryValueA;

LONG(WINAPI* Real_RegQueryValueW)(HKEY a0, LPCWSTR a1, LPWSTR a2, PLONG a3)
    = RegQueryValueW;

LONG(WINAPI* Real_RegSetValueW)(HKEY a0, LPCWSTR a1, DWORD a2, LPCWSTR a3, DWORD a4)
    = RegSetValueW;

LONG(WINAPI* Real_RegSetValueA)(HKEY a0, LPCSTR a1, DWORD a2, LPCSTR a3, DWORD a4)
    = RegSetValueA;



HRESULT WINAPI Mine_CoCreateInstance(IN REFCLSID rclsid, IN LPUNKNOWN pUnkOuter,
                                     IN DWORD dwClsContext, IN REFIID riid, OUT LPVOID FAR* ppv)
{
    if (CFilterMapper2::s_pFilterMapper2) {
        CheckPointer(ppv, E_POINTER);

        if (rclsid == CLSID_FilterMapper) {
            ASSERT(FALSE);
            return REGDB_E_CLASSNOTREG; // sorry...
        }

        if (rclsid == CLSID_FilterMapper2) {
            if (pUnkOuter) {
                return CLASS_E_NOAGGREGATION;
            }

            if (riid == __uuidof(IUnknown)) {
                CFilterMapper2::s_pFilterMapper2->AddRef();
                *ppv = (IUnknown*)CFilterMapper2::s_pFilterMapper2;
                return S_OK;
            } else if (riid == __uuidof(IFilterMapper2)) {
                CFilterMapper2::s_pFilterMapper2->AddRef();
                *ppv = (IFilterMapper2*)CFilterMapper2::s_pFilterMapper2;
                return S_OK;
            } else {
                return E_NOINTERFACE;
            }
        }
    }
    /*  else
        {
            if (rclsid == CLSID_FilterMapper2)
            {
                CFilterMapper2* pFM2 = DEBUG_NEW CFilterMapper2(true, false, pUnkOuter);
                CComPtr<IUnknown> pUnk = (IUnknown*)pFM2;
                return pUnk->QueryInterface(riid, ppv);
            }
        }
    */
    if (!pUnkOuter)
        if (rclsid == CLSID_VideoMixingRenderer || rclsid == CLSID_VideoMixingRenderer9
                || rclsid == CLSID_VideoRenderer || rclsid == CLSID_VideoRendererDefault
                || rclsid == CLSID_OverlayMixer) { // || rclsid == CLSID_OverlayMixer2 - where is this declared?)
            CMacrovisionKicker* pMK = DEBUG_NEW CMacrovisionKicker(NAME("CMacrovisionKicker"), nullptr);
            CComPtr<IUnknown> pUnk = (IUnknown*)(INonDelegatingUnknown*)pMK;
            CComPtr<IUnknown> pInner;

            if (SUCCEEDED(Real_CoCreateInstance(rclsid, pUnk, dwClsContext, IID_PPV_ARGS(&pInner)))) {
                pMK->SetInner(pInner);
                return pUnk->QueryInterface(riid, ppv);
            }
        }

    return Real_CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

#define FAKEHKEY (HKEY)0x12345678

LONG WINAPI Mine_RegCloseKey(HKEY a0)
{
    if (CFilterMapper2::s_pFilterMapper2 && a0 == FAKEHKEY) {
        return ERROR_SUCCESS;
    }
    return Real_RegCloseKey(a0);
}

LONG WINAPI Mine_RegFlushKey(HKEY a0)
{
    if (CFilterMapper2::s_pFilterMapper2 && a0 == FAKEHKEY) {
        return ERROR_SUCCESS;
    }
    return Real_RegFlushKey(a0);
}

LONG WINAPI Mine_RegCreateKeyA(HKEY a0, LPCSTR a1, PHKEY a2)
{
    if (CFilterMapper2::s_pFilterMapper2) {
        *a2 = FAKEHKEY;
        return ERROR_SUCCESS;
    }
    return Real_RegCreateKeyA(a0, a1, a2);
}

LONG WINAPI Mine_RegCreateKeyW(HKEY a0, LPCWSTR a1, PHKEY a2)
{
    if (CFilterMapper2::s_pFilterMapper2) {
        *a2 = FAKEHKEY;
        return ERROR_SUCCESS;
    }
    return Real_RegCreateKeyW(a0, a1, a2);
}

LONG WINAPI Mine_RegCreateKeyExA(HKEY a0, LPCSTR a1, DWORD a2, LPSTR a3, DWORD a4, REGSAM a5, LPSECURITY_ATTRIBUTES a6, PHKEY a7, LPDWORD a8)
{
    if (CFilterMapper2::s_pFilterMapper2) {
        *a7 = FAKEHKEY;
        return ERROR_SUCCESS;
    }
    return Real_RegCreateKeyExA(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

LONG WINAPI Mine_RegCreateKeyExW(HKEY a0, LPCWSTR a1, DWORD a2, LPWSTR a3, DWORD a4, REGSAM a5, LPSECURITY_ATTRIBUTES a6, PHKEY a7, LPDWORD a8)
{
    if (CFilterMapper2::s_pFilterMapper2) {
        *a7 = FAKEHKEY;
        return ERROR_SUCCESS;
    }
    return Real_RegCreateKeyExW(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

LONG WINAPI Mine_RegDeleteKeyA(HKEY a0, LPCSTR a1)
{
    // (INT_PTR)a0 < 0 will catch all attempts to use a predefined HKEY directly
    if (CFilterMapper2::s_pFilterMapper2 && (a0 == FAKEHKEY || (INT_PTR)a0 < 0)) {
        return ERROR_SUCCESS;
    }
    return Real_RegDeleteKeyA(a0, a1);
}

LONG WINAPI Mine_RegDeleteKeyW(HKEY a0, LPCWSTR a1)
{
    // (INT_PTR)a0 < 0 will catch all attempts to use a predefined HKEY directly
    if (CFilterMapper2::s_pFilterMapper2 && (a0 == FAKEHKEY || (INT_PTR)a0 < 0)) {
        return ERROR_SUCCESS;
    }
    return Real_RegDeleteKeyW(a0, a1);
}

LONG WINAPI Mine_RegDeleteValueA(HKEY a0, LPCSTR a1)
{
    // (INT_PTR)a0 < 0 will catch all attempts to use a predefined HKEY directly
    if (CFilterMapper2::s_pFilterMapper2 && (a0 == FAKEHKEY || (INT_PTR)a0 < 0)) {
        return ERROR_SUCCESS;
    }
    return Real_RegDeleteValueA(a0, a1);
}

LONG WINAPI Mine_RegDeleteValueW(HKEY a0, LPCWSTR a1)
{
    // (INT_PTR)a0 < 0 will catch all attempts to use a predefined HKEY directly
    if (CFilterMapper2::s_pFilterMapper2 && (a0 == FAKEHKEY || (INT_PTR)a0 < 0)) {
        return ERROR_SUCCESS;
    }
    return Real_RegDeleteValueW(a0, a1);
}

LONG WINAPI Mine_RegEnumKeyExA(HKEY a0, DWORD a1, LPSTR a2, LPDWORD a3, LPDWORD a4, LPSTR a5, LPDWORD a6, struct _FILETIME* a7)
{
    if (CFilterMapper2::s_pFilterMapper2 && a0 == FAKEHKEY) {
        return ERROR_NO_MORE_ITEMS;
    }
    return Real_RegEnumKeyExA(a0, a1, a2, a3, a4, a5, a6, a7);
}

LONG WINAPI Mine_RegEnumKeyExW(HKEY a0, DWORD a1, LPWSTR a2, LPDWORD a3, LPDWORD a4, LPWSTR a5, LPDWORD a6, struct _FILETIME* a7)
{
    if (CFilterMapper2::s_pFilterMapper2 && a0 == FAKEHKEY) {
        return ERROR_NO_MORE_ITEMS;
    }
    return Real_RegEnumKeyExW(a0, a1, a2, a3, a4, a5, a6, a7);
}

LONG WINAPI Mine_RegEnumValueA(HKEY a0, DWORD a1, LPSTR a2, LPDWORD a3, LPDWORD a4, LPDWORD a5, LPBYTE a6, LPDWORD a7)
{
    if (CFilterMapper2::s_pFilterMapper2 && a0 == FAKEHKEY) {
        return ERROR_NO_MORE_ITEMS;
    }
    return Real_RegEnumValueA(a0, a1, a2, a3, a4, a5, a6, a7);
}

LONG WINAPI Mine_RegEnumValueW(HKEY a0, DWORD a1, LPWSTR a2, LPDWORD a3, LPDWORD a4, LPDWORD a5, LPBYTE a6, LPDWORD a7)
{
    if (CFilterMapper2::s_pFilterMapper2 && a0 == FAKEHKEY) {
        return ERROR_NO_MORE_ITEMS;
    }
    return Real_RegEnumValueW(a0, a1, a2, a3, a4, a5, a6, a7);
}

LONG WINAPI Mine_RegOpenKeyA(HKEY a0, LPCSTR a1, PHKEY a2)
{
    if (CFilterMapper2::s_pFilterMapper2) {
        *a2 = FAKEHKEY;
        return ERROR_SUCCESS;
    }
    return Real_RegOpenKeyA(a0, a1, a2);
}

LONG WINAPI Mine_RegOpenKeyW(HKEY a0, LPCWSTR a1, PHKEY a2)
{
    if (CFilterMapper2::s_pFilterMapper2) {
        *a2 = FAKEHKEY;
        return ERROR_SUCCESS;
    }
    return Real_RegOpenKeyW(a0, a1, a2);
}

LONG WINAPI Mine_RegOpenKeyExA(HKEY a0, LPCSTR a1, DWORD a2, REGSAM a3, PHKEY a4)
{
    if (CFilterMapper2::s_pFilterMapper2 && (a3 & (KEY_SET_VALUE | KEY_CREATE_SUB_KEY))) {
        *a4 = FAKEHKEY;
        return ERROR_SUCCESS;
    }
    return Real_RegOpenKeyExA(a0, a1, a2, a3, a4);
}

LONG WINAPI Mine_RegOpenKeyExW(HKEY a0, LPCWSTR a1, DWORD a2, REGSAM a3, PHKEY a4)
{
    if (CFilterMapper2::s_pFilterMapper2 && (a3 & (KEY_SET_VALUE | KEY_CREATE_SUB_KEY))) {
        *a4 = FAKEHKEY;
        return ERROR_SUCCESS;
    }
    return Real_RegOpenKeyExW(a0, a1, a2, a3, a4);
}

LONG WINAPI Mine_RegQueryInfoKeyA(HKEY a0, LPSTR a1, LPDWORD a2, LPDWORD a3, LPDWORD a4, LPDWORD a5, LPDWORD a6, LPDWORD a7, LPDWORD a8, LPDWORD a9, LPDWORD a10, struct _FILETIME* a11)
{
    if (CFilterMapper2::s_pFilterMapper2 && a0 == FAKEHKEY) {
        return ERROR_INVALID_HANDLE;
    }
    return Real_RegQueryInfoKeyA(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

LONG WINAPI Mine_RegQueryInfoKeyW(HKEY a0, LPWSTR a1, LPDWORD a2, LPDWORD a3, LPDWORD a4, LPDWORD a5, LPDWORD a6, LPDWORD a7, LPDWORD a8, LPDWORD a9, LPDWORD a10, struct _FILETIME* a11)
{
    if (CFilterMapper2::s_pFilterMapper2 && a0 == FAKEHKEY) {
        return ERROR_INVALID_HANDLE;
    }
    return Real_RegQueryInfoKeyW(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

LONG WINAPI Mine_RegQueryValueA(HKEY a0, LPCSTR a1, LPSTR a2, PLONG a3)
{
    if (CFilterMapper2::s_pFilterMapper2 && a0 == FAKEHKEY) {
        *a3 = 0;
        return ERROR_SUCCESS;
    }
    return Real_RegQueryValueA(a0, a1, a2, a3);
}

LONG WINAPI Mine_RegQueryValueW(HKEY a0, LPCWSTR a1, LPWSTR a2, PLONG a3)
{
    if (CFilterMapper2::s_pFilterMapper2 && a0 == FAKEHKEY) {
        *a3 = 0;
        return ERROR_SUCCESS;
    }
    return Real_RegQueryValueW(a0, a1, a2, a3);
}

LONG WINAPI Mine_RegQueryValueExA(HKEY a0, LPCSTR a1, LPDWORD a2, LPDWORD a3, LPBYTE a4, LPDWORD a5)
{
    if (CFilterMapper2::s_pFilterMapper2 && a0 == FAKEHKEY) {
        *a5 = 0;
        return ERROR_SUCCESS;
    }
    return Real_RegQueryValueExA(a0, a1, a2, a3, a4, a5);
}

LONG WINAPI Mine_RegQueryValueExW(HKEY a0, LPCWSTR a1, LPDWORD a2, LPDWORD a3, LPBYTE a4, LPDWORD a5)
{
    if (CFilterMapper2::s_pFilterMapper2 && a0 == FAKEHKEY) {
        *a5 = 0;
        return ERROR_SUCCESS;
    }
    return Real_RegQueryValueExW(a0, a1, a2, a3, a4, a5);
}

LONG WINAPI Mine_RegSetValueA(HKEY a0, LPCSTR a1, DWORD a2, LPCSTR a3, DWORD a4)
{
    // (INT_PTR)a0 < 0 will catch all attempts to use a predefined HKEY directly
    if (CFilterMapper2::s_pFilterMapper2 && (a0 == FAKEHKEY || (INT_PTR)a0 < 0)) {
        return ERROR_SUCCESS;
    }
    return Real_RegSetValueA(a0, a1, a2, a3, a4);
}

LONG WINAPI Mine_RegSetValueW(HKEY a0, LPCWSTR a1, DWORD a2, LPCWSTR a3, DWORD a4)
{
    // (INT_PTR)a0 < 0 will catch all attempts to use a predefined HKEY directly
    if (CFilterMapper2::s_pFilterMapper2 && (a0 == FAKEHKEY || (INT_PTR)a0 < 0)) {
        return ERROR_SUCCESS;
    }
    return Real_RegSetValueW(a0, a1, a2, a3, a4);
}

LONG WINAPI Mine_RegSetValueExA(HKEY a0, LPCSTR a1, DWORD a2, DWORD a3, const BYTE* a4, DWORD a5)
{
    // (INT_PTR)a0 < 0 will catch all attempts to use a predefined HKEY directly
    if (CFilterMapper2::s_pFilterMapper2 && (a0 == FAKEHKEY || (INT_PTR)a0 < 0)) {
        return ERROR_SUCCESS;
    }
    return Real_RegSetValueExA(a0, a1, a2, a3, a4, a5);
}

LONG WINAPI Mine_RegSetValueExW(HKEY a0, LPCWSTR a1, DWORD a2, DWORD a3, const BYTE* a4, DWORD a5)
{
    // (INT_PTR)a0 < 0 will catch all attempts to use a predefined HKEY directly
    if (CFilterMapper2::s_pFilterMapper2 && (a0 == FAKEHKEY || (INT_PTR)a0 < 0)) {
        return ERROR_SUCCESS;
    }
    return Real_RegSetValueExW(a0, a1, a2, a3, a4, a5);
}

//
// CFilterMapper2
//

IFilterMapper2* CFilterMapper2::s_pFilterMapper2 = nullptr;

bool CFilterMapper2::s_bInitialized = false;

void CFilterMapper2::Init()
{
    if (!s_bInitialized) {
        // In case of error, we don't report the failure immediately since the hooks might not be needed
        s_bInitialized = Mhook_SetHookEx(&Real_CoCreateInstance, Mine_CoCreateInstance)
                         && Mhook_SetHookEx(&Real_RegCloseKey, Mine_RegCloseKey)
                         && Mhook_SetHookEx(&Real_RegFlushKey, Mine_RegFlushKey)
                         && Mhook_SetHookEx(&Real_RegCreateKeyA, Mine_RegCreateKeyA)
                         && Mhook_SetHookEx(&Real_RegCreateKeyW, Mine_RegCreateKeyW)
                         && Mhook_SetHookEx(&Real_RegCreateKeyExA, Mine_RegCreateKeyExA)
                         && Mhook_SetHookEx(&Real_RegCreateKeyExW, Mine_RegCreateKeyExW)
                         && Mhook_SetHookEx(&Real_RegDeleteKeyA, Mine_RegDeleteKeyA)
                         && Mhook_SetHookEx(&Real_RegDeleteKeyW, Mine_RegDeleteKeyW)
                         && Mhook_SetHookEx(&Real_RegDeleteValueA, Mine_RegDeleteValueA)
                         && Mhook_SetHookEx(&Real_RegDeleteValueW, Mine_RegDeleteValueW)
                         && Mhook_SetHookEx(&Real_RegEnumKeyExA, Mine_RegEnumKeyExA)
                         && Mhook_SetHookEx(&Real_RegEnumKeyExW, Mine_RegEnumKeyExW)
                         && Mhook_SetHookEx(&Real_RegEnumValueA, Mine_RegEnumValueA)
                         && Mhook_SetHookEx(&Real_RegEnumValueW, Mine_RegEnumValueW)
                         && Mhook_SetHookEx(&Real_RegOpenKeyA, Mine_RegOpenKeyA)
                         && Mhook_SetHookEx(&Real_RegOpenKeyW, Mine_RegOpenKeyW)
                         && Mhook_SetHookEx(&Real_RegOpenKeyExA, Mine_RegOpenKeyExA)
                         && Mhook_SetHookEx(&Real_RegOpenKeyExW, Mine_RegOpenKeyExW)
                         && Mhook_SetHookEx(&Real_RegQueryInfoKeyA, Mine_RegQueryInfoKeyA)
                         && Mhook_SetHookEx(&Real_RegQueryInfoKeyW, Mine_RegQueryInfoKeyW)
                         && Mhook_SetHookEx(&Real_RegQueryValueA, Mine_RegQueryValueA)
                         && Mhook_SetHookEx(&Real_RegQueryValueW, Mine_RegQueryValueW)
                         && Mhook_SetHookEx(&Real_RegQueryValueExA, Mine_RegQueryValueExA)
                         && Mhook_SetHookEx(&Real_RegQueryValueExW, Mine_RegQueryValueExW)
                         && Mhook_SetHookEx(&Real_RegSetValueA, Mine_RegSetValueA)
                         && Mhook_SetHookEx(&Real_RegSetValueW, Mine_RegSetValueW)
                         && Mhook_SetHookEx(&Real_RegSetValueExA, Mine_RegSetValueExA)
                         && Mhook_SetHookEx(&Real_RegSetValueExW, Mine_RegSetValueExW);
        s_bInitialized &= MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
    }
}

CFilterMapper2::CFilterMapper2(bool bRefCounted, bool bAllowUnreg /*= false*/, LPUNKNOWN pUnkOuter /*= nullptr*/)
    : CUnknown(NAME("CFilterMapper2"), pUnkOuter)
    , m_bRefCounted(bRefCounted)
    , m_bAllowUnreg(bAllowUnreg)
{
    m_cRef = bRefCounted ? 0 : 1;

    // We can't call Init() if more than one thread is running due to a custom optimization
    // we use for mhook. Instead we ensure that the hooks were properly set up at startup
    ENSURE(s_bInitialized);

    HRESULT hr = Real_CoCreateInstance(CLSID_FilterMapper2, (IUnknown*)(INonDelegatingUnknown*)this,
                                       CLSCTX_ALL, IID_PPV_ARGS(&m_pFM2));
    if (FAILED(hr) || !m_pFM2) {
        ASSERT(FALSE);
    }
}

CFilterMapper2::~CFilterMapper2()
{
    POSITION pos = m_filters.GetHeadPosition();
    while (pos) {
        delete m_filters.GetNext(pos);
    }
}

STDMETHODIMP CFilterMapper2::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    if (riid == __uuidof(IFilterMapper2)) {
        return GetInterface((IFilterMapper2*)this, ppv);
    }

    HRESULT hr = m_pFM2 ? m_pFM2->QueryInterface(riid, ppv) : E_NOINTERFACE;

    return SUCCEEDED(hr) ? hr : __super::NonDelegatingQueryInterface(riid, ppv);
}

void CFilterMapper2::Register(CString path)
{
    // Load filter
    if (HMODULE h = LoadLibraryEx(path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH)) {
        typedef HRESULT(__stdcall * PDllRegisterServer)();
        if (PDllRegisterServer fpDllRegisterServer = (PDllRegisterServer)GetProcAddress(h, "DllRegisterServer")) {
            ASSERT(!CFilterMapper2::s_pFilterMapper2);

            CFilterMapper2::s_pFilterMapper2 = this;
            m_path = path;
            fpDllRegisterServer();
            m_path.Empty();
            CFilterMapper2::s_pFilterMapper2 = nullptr;
        }

        FreeLibrary(h);
    }
}

// IFilterMapper2

STDMETHODIMP CFilterMapper2::CreateCategory(REFCLSID clsidCategory, DWORD dwCategoryMerit, LPCWSTR Description)
{
    if (!m_path.IsEmpty()) {
        return S_OK;
    } else if (CComQIPtr<IFilterMapper2> pFM2 = m_pFM2) {
        return pFM2->CreateCategory(clsidCategory, dwCategoryMerit, Description);
    }

    return E_NOTIMPL;
}

STDMETHODIMP CFilterMapper2::UnregisterFilter(const CLSID* pclsidCategory, const OLECHAR* szInstance, REFCLSID Filter)
{
    if (!m_path.IsEmpty()) {
        return S_OK;
    } else if (CComQIPtr<IFilterMapper2> pFM2 = m_pFM2) {
        return m_bAllowUnreg ? pFM2->UnregisterFilter(pclsidCategory, szInstance, Filter) : S_OK;
    }

    return E_NOTIMPL;
}

STDMETHODIMP CFilterMapper2::RegisterFilter(REFCLSID clsidFilter, LPCWSTR Name, IMoniker** ppMoniker, const CLSID* pclsidCategory, const OLECHAR* szInstance, const REGFILTER2* prf2)
{
    if (!m_path.IsEmpty()) {
        if (FilterOverride* f = DEBUG_NEW FilterOverride) {
            f->fDisabled = false;
            f->type = FilterOverride::EXTERNAL;
            f->path = m_path;
            f->name = CStringW(Name);
            f->clsid = clsidFilter;
            f->iLoadType = FilterOverride::MERIT;
            f->dwMerit = prf2->dwMerit;

            if (prf2->dwVersion == 1) {
                for (ULONG i = 0; i < prf2->cPins; i++) {
                    const REGFILTERPINS& rgPin = prf2->rgPins[i];
                    if (rgPin.bOutput) {
                        continue;
                    }

                    for (UINT j = 0; j < rgPin.nMediaTypes; j++) {
                        if (!rgPin.lpMediaType[j].clsMajorType || !rgPin.lpMediaType[j].clsMinorType) {
                            break;
                        }
                        f->guids.AddTail(*rgPin.lpMediaType[j].clsMajorType);
                        f->guids.AddTail(*rgPin.lpMediaType[j].clsMinorType);
                    }
                }
            } else if (prf2->dwVersion == 2) {
                for (ULONG i = 0; i < prf2->cPins2; i++) {
                    const REGFILTERPINS2& rgPin = prf2->rgPins2[i];
                    if (rgPin.dwFlags & REG_PINFLAG_B_OUTPUT) {
                        continue;
                    }

                    for (UINT j = 0; j < rgPin.nMediaTypes; j++) {
                        if (!rgPin.lpMediaType[j].clsMajorType || !rgPin.lpMediaType[j].clsMinorType) {
                            break;
                        }
                        f->guids.AddTail(*rgPin.lpMediaType[j].clsMajorType);
                        f->guids.AddTail(*rgPin.lpMediaType[j].clsMinorType);
                    }
                }
            }

            f->backup.AddTailList(&f->guids);

            m_filters.AddTail(f);
        }

        return S_OK;
    } else if (CComQIPtr<IFilterMapper2> pFM2 = m_pFM2) {
        return pFM2->RegisterFilter(clsidFilter, Name, ppMoniker, pclsidCategory, szInstance, prf2);
    }

    return E_NOTIMPL;
}

STDMETHODIMP CFilterMapper2::EnumMatchingFilters(IEnumMoniker** ppEnum, DWORD dwFlags, BOOL bExactMatch, DWORD dwMerit,
                                                 BOOL bInputNeeded, DWORD cInputTypes, const GUID* pInputTypes, const REGPINMEDIUM* pMedIn, const CLSID* pPinCategoryIn, BOOL bRender,
                                                 BOOL bOutputNeeded, DWORD cOutputTypes, const GUID* pOutputTypes, const REGPINMEDIUM* pMedOut, const CLSID* pPinCategoryOut)
{
    if (CComQIPtr<IFilterMapper2> pFM2 = m_pFM2) {
        return pFM2->EnumMatchingFilters(ppEnum, dwFlags, bExactMatch, dwMerit,
                                         bInputNeeded, cInputTypes, pInputTypes, pMedIn, pPinCategoryIn, bRender,
                                         bOutputNeeded, cOutputTypes, pOutputTypes, pMedOut, pPinCategoryOut);
    }

    return E_NOTIMPL;
}
