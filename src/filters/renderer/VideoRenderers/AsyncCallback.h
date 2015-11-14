/*
* (C) 2015 see Authors.txt
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

#pragma once

#include <mfobjects.h>

// T: Type of the parent object
template <class T>
class AsyncCallback : public IMFAsyncCallback
{
public:
    typedef HRESULT(T::*InvokeFn)(IMFAsyncResult* pAsyncResult);

    AsyncCallback(T* pParent, InvokeFn fn) : m_pParent(pParent), m_pInvokeFn(fn) {}
    virtual ~AsyncCallback() = default;

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv) override {
        if (!ppv) {
            return E_POINTER;
        }
        if (iid == __uuidof(IUnknown)) {
            *ppv = static_cast<IUnknown*>(static_cast<IMFAsyncCallback*>(this));
        } else if (iid == __uuidof(IMFAsyncCallback)) {
            *ppv = static_cast<IMFAsyncCallback*>(this);
        } else {
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    STDMETHODIMP_(ULONG) AddRef() override {
        return m_pParent->AddRef();
    }

    STDMETHODIMP_(ULONG) Release() override {
        return m_pParent->Release();
    }

    // IMFAsyncCallback
    STDMETHODIMP GetParameters(DWORD*, DWORD*) override {
        return E_NOTIMPL;
    }

    STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) override {
        return (m_pParent->*m_pInvokeFn)(pAsyncResult);
    }

    T* m_pParent;
    InvokeFn m_pInvokeFn;
};
