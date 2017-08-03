/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2017 see Authors.txt
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

#include "MultiFiles.h"

interface __declspec(uuid("6DDB4EE7-45A0-4459-A508-BD77B32C91B2"))
    ISyncReader :
    public IUnknown
{
    STDMETHOD_(void, SetBreakEvent)(HANDLE hBreakEvent) PURE;
    STDMETHOD_(bool, HasErrors)() PURE;
    STDMETHOD_(void, ClearErrors)() PURE;
    STDMETHOD_(void, SetPTSOffset)(REFERENCE_TIME* rtPTSOffset) PURE;
};

interface __declspec(uuid("7D55F67A-826E-40B9-8A7D-3DF0CBBD272D"))
    IFileHandle :
    public IUnknown
{
    STDMETHOD_(HANDLE, GetFileHandle)() PURE;
    STDMETHOD_(LPCTSTR, GetFileName)() PURE;
};

class CAsyncFileReader : public CUnknown, public CMultiFiles, public IAsyncReader, public ISyncReader, public IFileHandle
{
protected:
    ULONGLONG m_len;
    HANDLE m_hBreakEvent;
    LONG m_lOsError; // CFileException::m_lOsError

public:
    CAsyncFileReader(CString fn, HRESULT& hr);
    CAsyncFileReader(CAtlList<CHdmvClipInfo::PlaylistItem>& Items, HRESULT& hr);

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // IAsyncReader

    STDMETHODIMP RequestAllocator(IMemAllocator* pPreferred, ALLOCATOR_PROPERTIES* pProps, IMemAllocator** ppActual) {
        return E_NOTIMPL;
    }
    STDMETHODIMP Request(IMediaSample* pSample, DWORD_PTR dwUser) {
        return E_NOTIMPL;
    }
    STDMETHODIMP WaitForNext(DWORD dwTimeout, IMediaSample** ppSample, DWORD_PTR* pdwUser) {
        return E_NOTIMPL;
    }
    STDMETHODIMP SyncReadAligned(IMediaSample* pSample) { return E_NOTIMPL; }
    STDMETHODIMP SyncRead(LONGLONG llPosition, LONG lLength, BYTE* pBuffer);
    STDMETHODIMP Length(LONGLONG* pTotal, LONGLONG* pAvailable);
    STDMETHODIMP BeginFlush() { return E_NOTIMPL; }
    STDMETHODIMP EndFlush() { return E_NOTIMPL; }

    // ISyncReader

    STDMETHODIMP_(void) SetBreakEvent(HANDLE hBreakEvent) { m_hBreakEvent = hBreakEvent; }
    STDMETHODIMP_(bool) HasErrors() { return m_lOsError != 0; }
    STDMETHODIMP_(void) ClearErrors() { m_lOsError = 0; }
    STDMETHODIMP_(void) SetPTSOffset(REFERENCE_TIME* rtPTSOffset) { m_pCurrentPTSOffset = rtPTSOffset; };

    // IFileHandle

    STDMETHODIMP_(HANDLE) GetFileHandle();
    STDMETHODIMP_(LPCTSTR) GetFileName();

};

class CAsyncUrlReader : public CAsyncFileReader, protected CAMThread
{
    CString m_url, m_fn;

protected:
    enum { CMD_EXIT, CMD_INIT };
    DWORD ThreadProc();

public:
    CAsyncUrlReader(CString url, HRESULT& hr);
    virtual ~CAsyncUrlReader();

    // IAsyncReader

    STDMETHODIMP Length(LONGLONG* pTotal, LONGLONG* pAvailable);
};
