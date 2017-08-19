/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2017 see Authors.txt
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

struct VIDEO_OUTPUT_FORMATS {
    const GUID* subtype;
    WORD        biPlanes;
    WORD        biBitCount;
    DWORD       biCompression;
};

class CBaseVideoFilter : public CTransformFilter
{
private:
    HRESULT Receive(IMediaSample* pIn);

    // these are private for a reason, don't bother them
    int m_win, m_hin, m_arxin, m_aryin, m_cfin;
    int m_wout, m_hout, m_arxout, m_aryout, m_cfout;

    long m_cBuffers;

protected:
    CCritSec m_csReceive;

    int m_w, m_h, m_arx, m_ary, m_cf;

    HRESULT GetDeliveryBuffer(int w, int h, IMediaSample** ppOut);
    HRESULT CopyBuffer(BYTE* pOut, BYTE* pIn, int w, int h, int pitchIn, const GUID& subtype, bool fInterlaced = false);
    HRESULT CopyBuffer(BYTE* pOut, BYTE** ppIn, int w, int h, int pitchIn, const GUID& subtype, bool fInterlaced = false);

    virtual void GetOutputSize(int& w, int& h, int& arx, int& ary, int& RealWidth, int& RealHeight, int& vsfilter) {}
    virtual HRESULT Transform(IMediaSample* pIn) = 0;
    virtual void GetOutputFormats(int& nNumber, VIDEO_OUTPUT_FORMATS** ppFormats);
    bool ConnectionWhitelistedForExtendedFormat();

public:
    CBaseVideoFilter(LPCTSTR pName, LPUNKNOWN lpunk, HRESULT* phr, REFCLSID clsid, long cBuffers = 1);
    virtual ~CBaseVideoFilter();

    HRESULT ReconnectOutput(int w, int h, bool bSendSample = true, int realWidth = -1, int realHeight = -1);
    int GetPinCount();
    CBasePin* GetPin(int n);

    HRESULT CheckInputType(const CMediaType* mtIn);
    HRESULT CheckOutputType(const CMediaType& mtOut);
    HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
    HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
    HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);
    HRESULT SetMediaType(PIN_DIRECTION dir, const CMediaType* pmt);

    void SetAspect(CSize aspect);
};

class CBaseVideoInputAllocator : public CMemAllocator
{
    CMediaType m_mt;

public:
    CBaseVideoInputAllocator(HRESULT* phr);
    void SetMediaType(const CMediaType& mt);
    STDMETHODIMP GetBuffer(IMediaSample** ppBuffer, REFERENCE_TIME* pStartTime, REFERENCE_TIME* pEndTime, DWORD dwFlags);
};

class CBaseVideoInputPin : public CTransformInputPin
{
    CBaseVideoInputAllocator* m_pAllocator;

public:
    CBaseVideoInputPin(LPCTSTR pObjectName, CBaseVideoFilter* pFilter, HRESULT* phr, LPCWSTR pName);
    ~CBaseVideoInputPin();

    STDMETHODIMP GetAllocator(IMemAllocator** ppAllocator);
    STDMETHODIMP ReceiveConnection(IPin* pConnector, const AM_MEDIA_TYPE* pmt);
};

class CBaseVideoOutputPin : public CTransformOutputPin
{
public:
    CBaseVideoOutputPin(LPCTSTR pObjectName, CBaseVideoFilter* pFilter, HRESULT* phr, LPCWSTR pName);

    HRESULT CheckMediaType(const CMediaType* mtOut);
};
