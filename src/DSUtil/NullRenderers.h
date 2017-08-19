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

#include <atlcoll.h>
#include "BaseClasses/streams.h"

class CNullRenderer : public CBaseRenderer
{
protected:
    virtual HRESULT DoRenderSample(IMediaSample* pSample) { return S_OK; }

public:
    CNullRenderer(REFCLSID clsid, LPCTSTR pName, LPUNKNOWN pUnk, HRESULT* phr);
};

class __declspec(uuid("579883A0-4E2D-481F-9436-467AAFAB7DE8"))
    CNullVideoRenderer : public CNullRenderer
{
protected:
    HRESULT CheckMediaType(const CMediaType* pmt);

public:
    CNullVideoRenderer(LPUNKNOWN pUnk, HRESULT* phr);
};

class __declspec(uuid("DD9ED57D-6ABF-42E8-89A2-11D04798DC58"))
    CNullUVideoRenderer : public CNullRenderer
{
protected:
    HRESULT CheckMediaType(const CMediaType* pmt);

public:
    CNullUVideoRenderer(LPUNKNOWN pUnk, HRESULT* phr);
    virtual HRESULT DoRenderSample(IMediaSample* pSample);
};

class __declspec(uuid("0C38BDFD-8C17-4E00-A344-F89397D3E22A"))
    CNullAudioRenderer : public CNullRenderer
{
protected:
    HRESULT CheckMediaType(const CMediaType* pmt);

public:
    CNullAudioRenderer(LPUNKNOWN pUnk, HRESULT* phr);
};

class __declspec(uuid("64A45125-7343-4772-9DA4-179FAC9D462C"))
    CNullUAudioRenderer : public CNullRenderer
{
protected:
    HRESULT CheckMediaType(const CMediaType* pmt);
    virtual HRESULT DoRenderSample(IMediaSample* pSample);

public:
    CNullUAudioRenderer(LPUNKNOWN pUnk, HRESULT* phr);
};

class __declspec(uuid("655D7613-C26C-4A25-BBBD-3C9C516122CC"))
    CNullTextRenderer : public CBaseFilter, public CCritSec
{
    class CTextInputPin : public CBaseInputPin
    {
    public:
        CTextInputPin(CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
            : CBaseInputPin(NAME("CTextInputPin"), pFilter, pLock, phr, L"In") {}
        HRESULT CheckMediaType(const CMediaType* pmt);
    };

    CAutoPtr<CTextInputPin> m_pInput;

public:
    CNullTextRenderer(LPUNKNOWN pUnk, HRESULT* phr);
    int GetPinCount() { return (int)!!m_pInput; }
    CBasePin* GetPin(int n) { return n == 0 ? (CBasePin*)m_pInput : nullptr; }
};
