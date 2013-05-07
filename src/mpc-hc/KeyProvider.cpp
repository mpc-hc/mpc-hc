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
#include "KeyProvider.h"
#include "DSUtil.h"


CKeyProvider::CKeyProvider()
    : CUnknown(NAME("CKeyProvider"), nullptr)
{
}

CKeyProvider::~CKeyProvider()
{
}

STDMETHODIMP CKeyProvider::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI(IServiceProvider)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CKeyProvider::QueryService(REFIID siid, REFIID riid, void** ppv)
{
    /*
    if (siid == __uuidof(IWMReader) && riid == IID_IUnknown)
    {
        CComPtr<IUnknown> punkCert;
        HRESULT hr = WMCreateCertificate(&punkCert);
        if (SUCCEEDED(hr))
            *ppv = (void*)punkCert.Detach();
        return hr;
    }
    */

    return E_NOINTERFACE;
}
