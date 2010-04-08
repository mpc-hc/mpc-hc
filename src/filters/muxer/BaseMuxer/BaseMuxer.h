/*
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include "BaseMuxerInputPin.h"
#include "BaseMuxerOutputPin.h"

class CBaseMuxerFilter
    : public CBaseFilter
    , public CCritSec
    , public CAMThread
    , public IMediaSeeking
    , public IDSMPropertyBagImpl
    , public IDSMResourceBagImpl
    , public IDSMChapterBagImpl
{
private:
    CAutoPtrList<CBaseMuxerInputPin> m_pInputs;
    CAutoPtr<CBaseMuxerOutputPin> m_pOutput;
    CAutoPtrList<CBaseMuxerRawOutputPin> m_pRawOutputs;

    enum {CMD_EXIT, CMD_RUN};
    DWORD ThreadProc();

    REFERENCE_TIME m_rtCurrent;
    CAtlList<CBaseMuxerInputPin*> m_pActivePins;

    CAutoPtr<MuxerPacket> GetPacket();

    void MuxHeaderInternal();
    void MuxPacketInternal(const MuxerPacket* pPacket);
    void MuxFooterInternal();

protected:
    CAtlList<CBaseMuxerInputPin*> m_pPins;
    CBaseMuxerOutputPin* GetOutputPin()
    {
        return m_pOutput;
    }

    virtual void MuxInit() = 0;

    // only called when the output pin is connected
    virtual void MuxHeader(IBitStream* pBS) {}
    virtual void MuxPacket(IBitStream* pBS, const MuxerPacket* pPacket) {}
    virtual void MuxFooter(IBitStream* pBS) {}

    // always called (useful if the derived class wants to write somewhere else than downstream)
    virtual void MuxHeader() {}
    virtual void MuxPacket(const MuxerPacket* pPacket) {}
    virtual void MuxFooter() {}

    // allows customized pins in derived classes
    virtual HRESULT CreateInput(CStringW name, CBaseMuxerInputPin** ppPin);
    virtual HRESULT CreateRawOutput(CStringW name, CBaseMuxerRawOutputPin** ppPin);

public:
    CBaseMuxerFilter(LPUNKNOWN pUnk, HRESULT* phr, const CLSID& clsid);
    virtual ~CBaseMuxerFilter();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    void AddInput();

    int GetPinCount();
    CBasePin* GetPin(int n);

    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);

    // IMediaSeeking

    STDMETHODIMP GetCapabilities(DWORD* pCapabilities);
    STDMETHODIMP CheckCapabilities(DWORD* pCapabilities);
    STDMETHODIMP IsFormatSupported(const GUID* pFormat);
    STDMETHODIMP QueryPreferredFormat(GUID* pFormat);
    STDMETHODIMP GetTimeFormat(GUID* pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID* pFormat);
    STDMETHODIMP SetTimeFormat(const GUID* pFormat);
    STDMETHODIMP GetDuration(LONGLONG* pDuration);
    STDMETHODIMP GetStopPosition(LONGLONG* pStop);
    STDMETHODIMP GetCurrentPosition(LONGLONG* pCurrent);
    STDMETHODIMP ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat);
    STDMETHODIMP SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags);
    STDMETHODIMP GetPositions(LONGLONG* pCurrent, LONGLONG* pStop);
    STDMETHODIMP GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest);
    STDMETHODIMP SetRate(double dRate);
    STDMETHODIMP GetRate(double* pdRate);
    STDMETHODIMP GetPreroll(LONGLONG* pllPreroll);
};

