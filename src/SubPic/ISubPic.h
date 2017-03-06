/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2016 see Authors.txt
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
#include <atlcoll.h>
#include "CoordGeom.h"

#pragma pack(push, 1)
struct SubPicDesc {
    int type;
    int w, h, bpp, pitch, pitchUV;
    BYTE* bits;
    BYTE* bitsU;
    BYTE* bitsV;
    RECT vidrect; // video rectangle

    SubPicDesc()
        : type(0)
        , w(0)
        , h(0)
        , bpp(0)
        , pitch(0)
        , pitchUV(0)
        , bits(nullptr)
        , bitsU(nullptr)
        , bitsV(nullptr) {
        ZeroMemory(&vidrect, sizeof(vidrect));
    }
};
#pragma pack(pop)

enum RelativeTo {
    WINDOW,
    VIDEO,
    BEST_FIT
};

//
// ISubPic
//

interface __declspec(uuid("DA3A5B51-958C-4C28-BF66-68D7947577A2"))
ISubPic :
public IUnknown {
    static const REFERENCE_TIME INVALID_TIME = -1;

    STDMETHOD_(void*, GetObject)() PURE;

    STDMETHOD_(REFERENCE_TIME, GetStart)() const PURE;
    STDMETHOD_(REFERENCE_TIME, GetStop)() const PURE;
    STDMETHOD_(void, SetStart)(REFERENCE_TIME rtStart) PURE;
    STDMETHOD_(void, SetStop)(REFERENCE_TIME rtStop) PURE;

    STDMETHOD(GetDesc)(SubPicDesc& spd /*[out]*/) PURE;
    STDMETHOD(CopyTo)(ISubPic* pSubPic /*[in]*/) PURE;

    STDMETHOD(ClearDirtyRect)(DWORD color /*[in]*/) PURE;
    STDMETHOD(GetDirtyRect)(RECT* pDirtyRect /*[out]*/) const PURE;
    STDMETHOD(SetDirtyRect)(const RECT* pDirtyRect /*[in]*/) PURE;

    STDMETHOD(GetMaxSize)(SIZE* pMaxSize /*[out]*/) const PURE;
    STDMETHOD(SetSize)(SIZE pSize /*[in]*/, RECT vidrect /*[in]*/) PURE;

    STDMETHOD(Lock)(SubPicDesc& spd /*[out]*/) PURE;
    STDMETHOD(Unlock)(RECT* pDirtyRect /*[in]*/) PURE;

    STDMETHOD(AlphaBlt)(RECT * pSrc, RECT * pDst, SubPicDesc* pTarget = nullptr /*[in]*/) PURE;
    STDMETHOD(GetSourceAndDest)(RECT rcWindow /*[in]*/, RECT rcVideo /*[in]*/,
                                RECT* pRcSource /*[out]*/,  RECT* pRcDest /*[out]*/,
                                const double videoStretchFactor = 1.0 /*[in]*/,
                                int xOffsetInPixels = 1 /*[in]*/) const PURE;
    STDMETHOD(SetVirtualTextureSize)(const SIZE pSize, const POINT pTopLeft) PURE;
    STDMETHOD(GetRelativeTo)(RelativeTo* pRelativeTo /*[out]*/) const PURE;
    STDMETHOD(SetRelativeTo)(RelativeTo relativeTo /*[in]*/) PURE;

    STDMETHOD_(REFERENCE_TIME, GetSegmentStart)() const PURE;
    STDMETHOD_(REFERENCE_TIME, GetSegmentStop)() const PURE;
    STDMETHOD_(void, SetSegmentStart)(REFERENCE_TIME rtStart) PURE;
    STDMETHOD_(void, SetSegmentStop)(REFERENCE_TIME rtStop) PURE;

    STDMETHOD_(bool, GetInverseAlpha)() const PURE;
    STDMETHOD_(void, SetInverseAlpha)(bool bInverted) PURE;
};

//
// ISubPicAllocator
//

interface __declspec(uuid("CF7C3C23-6392-4a42-9E72-0736CFF793CB"))
ISubPicAllocator :
public IUnknown {
    STDMETHOD(SetCurSize)(SIZE size /*[in]*/) PURE;
    STDMETHOD(SetCurVidRect)(RECT curvidrect) PURE;

    STDMETHOD(GetStatic)(ISubPic** ppSubPic /*[out]*/) PURE;
    STDMETHOD(AllocDynamic)(ISubPic** ppSubPic /*[out]*/) PURE;

    STDMETHOD_(bool, IsDynamicWriteOnly)() const PURE;

    STDMETHOD(ChangeDevice)(IUnknown * pDev) PURE;
    STDMETHOD(SetMaxTextureSize)(SIZE maxTextureSize) PURE;

    STDMETHOD(FreeStatic)() PURE;
};

//
// ISubPicProvider
//

interface __declspec(uuid("D62B9A1A-879A-42db-AB04-88AA8F243CFD"))
ISubPicProvider :
public IUnknown {
    static const REFERENCE_TIME UNKNOWN_TIME = _I64_MAX;

    STDMETHOD(Lock)() PURE;
    STDMETHOD(Unlock)() PURE;

    STDMETHOD_(POSITION, GetStartPosition)(REFERENCE_TIME rt, double fps) PURE;
    STDMETHOD_(POSITION, GetNext)(POSITION pos) PURE;

    STDMETHOD_(REFERENCE_TIME, GetStart)(POSITION pos, double fps) PURE;
    STDMETHOD_(REFERENCE_TIME, GetStop)(POSITION pos, double fps) PURE;

    STDMETHOD_(bool, IsAnimated)(POSITION pos) PURE;

    STDMETHOD(Render)(SubPicDesc & spd, REFERENCE_TIME rt, double fps, RECT & bbox) PURE;
    STDMETHOD(GetTextureSize)(POSITION pos, SIZE & MaxTextureSize, SIZE & VirtualSize, POINT & VirtualTopLeft) PURE;
    STDMETHOD(GetRelativeTo)(POSITION pos, RelativeTo & relativeTo) PURE;
};

//
// ISubPicQueue
//

interface __declspec(uuid("C8334466-CD1E-4ad1-9D2D-8EE8519BD180"))
ISubPicQueue :
public IUnknown {
    STDMETHOD(SetSubPicProvider)(ISubPicProvider* pSubPicProvider /*[in]*/) PURE;
    STDMETHOD(GetSubPicProvider)(ISubPicProvider** pSubPicProvider /*[out]*/) PURE;

    STDMETHOD(SetFPS)(double fps /*[in]*/) PURE;
    STDMETHOD(SetTime)(REFERENCE_TIME rtNow /*[in]*/) PURE;

    STDMETHOD(Invalidate)(REFERENCE_TIME rtInvalidate = -1) PURE;
    STDMETHOD_(bool, LookupSubPic)(REFERENCE_TIME rtNow /*[in]*/, CComPtr<ISubPic>& pSubPic /*[out]*/) PURE;

    STDMETHOD(GetStats)(int& nSubPics, REFERENCE_TIME & rtNow, REFERENCE_TIME & rtStart, REFERENCE_TIME& rtStop /*[out]*/) PURE;
    STDMETHOD(GetStats)(int nSubPic /*[in]*/, REFERENCE_TIME & rtStart, REFERENCE_TIME& rtStop /*[out]*/) PURE;

    STDMETHOD_(bool, LookupSubPic)(REFERENCE_TIME rtNow /*[in]*/, bool bAdviseBlocking, CComPtr<ISubPic>& pSubPic /*[out]*/) PURE;
};

//
// ISubPicAllocatorPresenter
//

interface __declspec(uuid("CF75B1F0-535C-4074-8869-B15F177F944E"))
ISubPicAllocatorPresenter :
public IUnknown {
    STDMETHOD(CreateRenderer)(IUnknown** ppRenderer) PURE;

    STDMETHOD_(SIZE, GetVideoSize)(bool bCorrectAR = true) const PURE;
    STDMETHOD_(void, SetPosition)(RECT w, RECT v) PURE;
    STDMETHOD_(bool, Paint)(bool bAll) PURE;

    STDMETHOD_(void, SetTime)(REFERENCE_TIME rtNow) PURE;
    STDMETHOD_(void, SetSubtitleDelay)(int delayMs) PURE;
    STDMETHOD_(int, GetSubtitleDelay)() const PURE;
    STDMETHOD_(double, GetFPS)() const PURE;

    STDMETHOD_(void, SetSubPicProvider)(ISubPicProvider * pSubPicProvider) PURE;
    STDMETHOD_(void, Invalidate)(REFERENCE_TIME rtInvalidate = -1) PURE;

    STDMETHOD(GetDIB)(BYTE * lpDib, DWORD * size) PURE;

    STDMETHOD(SetVideoAngle)(Vector v) PURE;
    STDMETHOD(SetPixelShader)(LPCSTR pSrcData, LPCSTR pTarget) PURE;

    STDMETHOD_(bool, ResetDevice)() PURE;
    STDMETHOD_(bool, DisplayChange)() PURE;
};

interface __declspec(uuid("767AEBA8-A084-488a-89C8-F6B74E53A90F"))
ISubPicAllocatorPresenter2 :
public ISubPicAllocatorPresenter {
    STDMETHOD(SetPixelShader2)(LPCSTR pSrcData, LPCSTR pTarget, bool bScreenSpace) PURE;
    STDMETHOD_(SIZE, GetVisibleVideoSize)() const PURE;

    STDMETHOD_(bool, IsRendering)() PURE;
    STDMETHOD(SetIsRendering)(bool bIsRendering) PURE;

    STDMETHOD(SetDefaultVideoAngle)(Vector v) PURE;
};

//
// ISubStream
//

interface __declspec(uuid("DE11E2FB-02D3-45e4-A174-6B7CE2783BDB"))
ISubStream :
public IPersist {
    STDMETHOD_(int, GetStreamCount)() PURE;
    STDMETHOD(GetStreamInfo)(int i, WCHAR** ppName, LCID * pLCID) PURE;
    STDMETHOD_(int, GetStream)() PURE;
    STDMETHOD(SetStream)(int iStream) PURE;
    STDMETHOD(Reload)() PURE;
    STDMETHOD(SetSourceTargetInfo)(CString yuvMatrix, int targetBlackLevel, int targetWhiteLevel) PURE;

    // TODO: get rid of IPersist to identify type and use only
    // interface functions to modify the settings of the substream
};
