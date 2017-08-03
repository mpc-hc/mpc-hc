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

#include <atlcoll.h>
#include "../DeCSS/VobFile.h"
#include "../DSUtil/DSUtil.h"
#include "VobSubFile.h"

#pragma pack(push, 1)
struct vidinfo {
    WORD perm_displ  : 2;
    WORD ratio       : 2;
    WORD system      : 2;
    WORD compression : 2;
    WORD mode        : 1;
    WORD letterboxed : 1;
    WORD source_res  : 2;
    WORD cbrvbr      : 2;
    WORD line21_2    : 1;
    WORD line21_1    : 1;
};

struct vc_t {
    BYTE vob, cell;
    DWORD tTime, tOffset, tTotal;
    DWORD start, end;
    int iAngle;
    bool bDiscontinuity;
};

struct PGC {
    int nAngles;
    CAtlArray<vc_t> angles[10];
    int iSelAngle;
    RGBQUAD pal[16];
    WORD ids[32];
};

struct VSFRipperData {
    CSize vidsize;
    vidinfo vidinfo;
    CAtlArray<PGC> pgcs;
    int iSelPGC;
    bool bResetTime, bClosedCaption, bForcedOnly;

    bool bClose, bBeep, bAuto; // only used by the UI externally, but may be set through the parameter file
    bool bCloseIgnoreError;

    CAtlArray<UINT> selvcs;
    CAtlMap<BYTE, bool> selids;

    void Reset();
    void Copy(struct VSFRipperData& rd);
};

struct vcchunk {
    __int64 start, end;
    DWORD vc;
};

#pragma pack(pop)

// note: these interfaces only meant to be used internally with static linking

//
// IVSFRipperCallback
//

interface __declspec(uuid("9E2EBB5C-AD7C-452f-A48B-38685716AC46"))
    IVSFRipperCallback :
    public IUnknown
{
    STDMETHOD(OnMessage)(LPCTSTR msg) PURE;
    STDMETHOD(OnProgress)(double progress /*0.0 -> 1.0*/) PURE;
    STDMETHOD(OnFinished)(bool bSucceeded) PURE;
};

// IVSFRipperCallbackImpl

class IVSFRipperCallbackImpl : public CUnknown, public IVSFRipperCallback
{
protected:
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) {
        return
            QI(IVSFRipperCallback)
            __super::NonDelegatingQueryInterface(riid, ppv);
    }

    // IVSFRipperCallback
    STDMETHODIMP OnMessage(LPCTSTR msg) { return S_FALSE; }
    STDMETHODIMP OnProgress(double progress /*0.0 -> 1.0*/) { return S_FALSE; }
    STDMETHODIMP OnFinished(bool bSucceeded) { return S_FALSE; }

public:
    IVSFRipperCallbackImpl() : CUnknown(NAME("IVSFRipperCallbackImpl"), nullptr) {}
};

//
// IVSFRipper
//

interface __declspec(uuid("69F935BB-B8D0-43f5-AA2E-BBD0851CC9A6"))
    IVSFRipper :
    public IUnknown
{
    STDMETHOD(SetCallBack)(IVSFRipperCallback* pCallback) PURE;
    STDMETHOD(LoadParamFile)(CString fn) PURE;
    STDMETHOD(SetInput)(CString infn) PURE;
    STDMETHOD(SetOutput)(CString outfn) PURE;
    STDMETHOD(GetRipperData)(VSFRipperData& rd) PURE;
    STDMETHOD(UpdateRipperData)(VSFRipperData& rd) PURE;
    STDMETHOD(Index)() PURE;
    STDMETHOD(IsIndexing)() PURE;
    STDMETHOD(Abort)(bool bSavePartial) PURE;
};

class CVobSubFileRipper : public CVobSubFile, protected CAMThread, public IVSFRipper
{
private:
    bool m_bThreadActive, m_bBreakThread, m_bIndexing;
    enum { CMD_EXIT, CMD_INDEX };
    DWORD ThreadProc();
    bool Create();

    //

    enum log_t {
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR
    };
    void Log(log_t type, LPCTSTR lpszFormat, ...);
    void Progress(double progress);
    void Finished(bool bSucceeded);

    //

    CCritSec m_csAccessLock;
    CString m_infn, m_outfn;
    CVobFile m_vob;
    VSFRipperData m_rd;

    bool LoadIfo(CString fn);
    bool LoadVob(CString fn);
    bool LoadChunks(CAtlArray<vcchunk>& chunks);
    bool SaveChunks(CAtlArray<vcchunk>& chunks);

    //

    CCritSec m_csCallback;
    CComPtr<IVSFRipperCallback> m_pCallback;

public:
    CVobSubFileRipper();
    virtual ~CVobSubFileRipper();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // IVSFRipper
    STDMETHODIMP SetCallBack(IVSFRipperCallback* pCallback);
    STDMETHODIMP LoadParamFile(CString fn);
    STDMETHODIMP SetInput(CString infn);
    STDMETHODIMP SetOutput(CString outfn);
    STDMETHODIMP GetRipperData(VSFRipperData& rd);
    STDMETHODIMP UpdateRipperData(VSFRipperData& rd);
    STDMETHODIMP Index();
    STDMETHODIMP IsIndexing();
    STDMETHODIMP Abort(bool bSavePartial);
};
