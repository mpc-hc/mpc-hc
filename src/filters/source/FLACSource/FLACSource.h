/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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

#include <atlbase.h>
#include <qnetwork.h>
#include "../BaseSource/BaseSource.h"
#include "../../../DSUtil/DSMPropertyBag.h"

#define FlacSourceName   L"MPC FLAC Source"

class CFLACStream;

class __declspec(uuid("1930D8FF-4739-4e42-9199-3B2EDEAA3BF2"))
    CFLACSource
    : public CBaseSource<CFLACStream>
    , public IAMMediaContent
    , public IDSMChapterBagImpl
{
    CFLACStream* GetFLACStream() { return (CFLACStream*)m_paStreams[0]; }

public:
    CFLACSource(LPUNKNOWN lpunk, HRESULT* phr);
    virtual ~CFLACSource();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // CBaseFilter
    STDMETHODIMP QueryFilterInfo(FILTER_INFO* pInfo);

    // IDispatch

    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) { return E_NOTIMPL; }
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) { return E_NOTIMPL; }
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid) { return E_NOTIMPL; }
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr) { return E_NOTIMPL; }

    // IAMMediaContent

    STDMETHODIMP get_AuthorName(BSTR* pbstrAuthorName);
    STDMETHODIMP get_Title(BSTR* pbstrTitle);
    STDMETHODIMP get_Description(BSTR* pbstrDescription);
    STDMETHODIMP get_Copyright(BSTR* pbstrCopyright);
    STDMETHODIMP get_Rating(BSTR* pbstrRating) { return E_NOTIMPL; }
    STDMETHODIMP get_BaseURL(BSTR* pbstrBaseURL) { return E_NOTIMPL; }
    STDMETHODIMP get_LogoURL(BSTR* pbstrLogoURL) { return E_NOTIMPL; }
    STDMETHODIMP get_LogoIconURL(BSTR* pbstrLogoURL) { return E_NOTIMPL; }
    STDMETHODIMP get_WatermarkURL(BSTR* pbstrWatermarkURL) { return E_NOTIMPL; }
    STDMETHODIMP get_MoreInfoURL(BSTR* pbstrMoreInfoURL) { return E_NOTIMPL; }
    STDMETHODIMP get_MoreInfoBannerImage(BSTR* pbstrMoreInfoBannerImage) { return E_NOTIMPL; }
    STDMETHODIMP get_MoreInfoBannerURL(BSTR* pbstrMoreInfoBannerURL) { return E_NOTIMPL; }
    STDMETHODIMP get_MoreInfoText(BSTR* pbstrMoreInfoText) { return E_NOTIMPL; }
};

class CGolombBuffer;

class CFLACStream : public CBaseStream
{
    CFile m_file;
    void* m_pDecoder;

    int m_nMaxFrameSize;
    int m_nSamplesPerSec;
    int m_nChannels;
    WORD m_wBitsPerSample;
    __int64 m_i64TotalNumSamples;
    int m_nAvgBytesPerSec;

    ULONGLONG m_llOffset;             // Position of first frame in file
    ULONGLONG m_llFileSize;           // Size of the file

    CMapStringToString m_vorbisComments;

public:
    CFLACStream(const WCHAR* wfn, CSource* pParent, HRESULT* phr);
    virtual ~CFLACStream();

    HRESULT FillBuffer(IMediaSample* pSample, int nFrame, BYTE* pOut, long& len);

    HRESULT DecideBufferSize(IMemAllocator* pIMemAlloc, ALLOCATOR_PROPERTIES* pProperties);
    HRESULT CheckMediaType(const CMediaType* pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);

    void UpdateFromMetadata(void* pBuffer);
    inline CFile* GetFile() { return &m_file; };

    bool GetComment(const CString& tag, CString& content) const;

    bool m_bIsEOF;
};
