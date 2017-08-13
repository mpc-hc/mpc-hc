/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015, 2017 see Authors.txt
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
#include "VobSubImage.h"
#include "../SubPic/SubPicProviderImpl.h"

#define VOBSUBIDXVER 7

#define ReadBEb(var)              \
    f.Read(&((BYTE*)&var)[0], 1);

#define ReadBEw(var)              \
    f.Read(&((BYTE*)&var)[1], 1); \
    f.Read(&((BYTE*)&var)[0], 1);

#define ReadBEdw(var)             \
    f.Read(&((BYTE*)&var)[3], 1); \
    f.Read(&((BYTE*)&var)[2], 1); \
    f.Read(&((BYTE*)&var)[1], 1); \
    f.Read(&((BYTE*)&var)[0], 1);

extern CString FindLangFromId(WORD id);

class CVobSubSettings
{
protected:
    HRESULT Render(SubPicDesc& spd, RECT& bbox);

public:
    CSize m_size;
    int m_x, m_y;
    CPoint m_org;
    int m_scale_x, m_scale_y;   // % (don't set it to unsigned because as a side effect it will mess up negative coordinates in GetDestrect())
    int m_alpha;                // %
    int m_iSmooth;              // 0: OFF, 1: ON, 2: OLD (means no filtering at all)
    int m_fadein, m_fadeout;    // ms
    bool m_bAlign;
    int m_alignhor, m_alignver; // 0: left/top, 1: center, 2: right/bottom
    unsigned int m_toff;        // ms
    bool m_bOnlyShowForcedSubs;
    bool m_bCustomPal;
    int m_tridx;
    RGBQUAD m_orgpal[16], m_cuspal[4];

    CVobSubImage m_img;

    CVobSubSettings() { InitSettings(); }
    void InitSettings();

    bool GetCustomPal(RGBQUAD* cuspal, int& tridx);
    void SetCustomPal(const RGBQUAD* cuspal, int tridx);

    void GetDestrect(CRect& r); // destrect of m_img, considering the current alignment mode
    void GetDestrect(CRect& r, int w, int h); // this will scale it to the frame size of (w, h)

    void SetAlignment(bool bAlign, int x, int y, int hor = 1, int ver = 1);
};

class __declspec(uuid("998D4C9A-460F-4de6-BDCD-35AB24F94ADF"))
    CVobSubFile : public CVobSubSettings, public ISubStream, public CSubPicProviderImpl
{
public:
    struct SubPos {
        __int64 filepos       = 0i64;
        __int64 start         = 0i64;
        __int64 stop          = 0i64;
        bool bForced          = false;
        bool bAnimated        = false;
        char vobid            = 0;
        char cellid           = 0;
        __int64 celltimestamp = 0i64;
        bool bValid           = false;

        bool operator <(const SubPos& rhs) const {
            return start < rhs.start;
        }
    };

    struct SubLang {
        WORD id = 0;
        CString name, alt;
        CAtlArray<SubPos> subpos;
    };

protected:
    CString m_title;

    void TrimExtension(CString& fn);
    bool ReadIdx(CString fn, int& ver), ReadSub(CString fn), ReadRar(CString fn), ReadIfo(CString fn);
    bool WriteIdx(CString fn, int delay), WriteSub(CString fn);

    CMemFile m_sub;

    BYTE* GetPacket(size_t idx, size_t& packetSize, size_t& dataSize, size_t nLang = SIZE_T_ERROR);
    const SubPos* GetFrameInfo(size_t idx, size_t nLang = SIZE_T_ERROR) const;
    bool GetFrame(size_t idx, size_t nLang = SIZE_T_ERROR, REFERENCE_TIME rt = -1);
    bool GetFrameByTimeStamp(__int64 time);
    size_t GetFrameIdxByTimeStamp(__int64 time);

    bool SaveVobSub(CString fn, int delay);
    bool SaveWinSubMux(CString fn, int delay);
    bool SaveScenarist(CString fn, int delay);
    bool SaveMaestro(CString fn, int delay);

public:
    size_t m_nLang;
    std::array<SubLang, 32> m_langs;

    CVobSubFile(CCritSec* pLock);
    virtual ~CVobSubFile();

    bool Copy(CVobSubFile& vsf);

    enum SubFormat {
        None,
        VobSub,
        WinSubMux,
        Scenarist,
        Maestro
    };

    bool Open(CString fn);
    bool Save(CString fn, int delay = 0, SubFormat sf = VobSub);
    void Close();

    CString GetTitle() { return m_title; }

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // ISubPicProvider
    STDMETHODIMP_(POSITION) GetStartPosition(REFERENCE_TIME rt, double fps);
    STDMETHODIMP_(POSITION) GetNext(POSITION pos);
    STDMETHODIMP_(REFERENCE_TIME) GetStart(POSITION pos, double fps);
    STDMETHODIMP_(REFERENCE_TIME) GetStop(POSITION pos, double fps);
    STDMETHODIMP_(bool) IsAnimated(POSITION pos);
    STDMETHODIMP Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox);
    STDMETHODIMP GetRelativeTo(POSITION pos, RelativeTo& relativeTo);

    // IPersist
    STDMETHODIMP GetClassID(CLSID* pClassID);

    // ISubStream
    STDMETHODIMP_(int) GetStreamCount();
    STDMETHODIMP GetStreamInfo(int i, WCHAR** ppName, LCID* pLCID);
    STDMETHODIMP_(int) GetStream();
    STDMETHODIMP SetStream(int iStream);
    STDMETHODIMP Reload();
    STDMETHODIMP SetSourceTargetInfo(CString yuvMatrix, int targetBlackLevel, int targetWhiteLevel) { return E_NOTIMPL; };
};

class __declspec(uuid("D7FBFB45-2D13-494F-9B3D-FFC9557D5C45"))
    CVobSubStream : public CVobSubSettings, public ISubStream, public CSubPicProviderImpl
{
    CString m_name;

    CCritSec m_csSubPics;
    struct SubPic {
        REFERENCE_TIME tStart, tStop;
        bool bAnimated;
        CAtlArray<BYTE> pData;
    };
    CAutoPtrList<SubPic> m_subpics;

public:
    CVobSubStream(CCritSec* pLock);
    virtual ~CVobSubStream();

    void Open(CString name, BYTE* pData, int len);

    void Add(REFERENCE_TIME tStart, REFERENCE_TIME tStop, BYTE* pData, int len);
    void RemoveAll();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // ISubPicProvider
    STDMETHODIMP_(POSITION) GetStartPosition(REFERENCE_TIME rt, double fps);
    STDMETHODIMP_(POSITION) GetNext(POSITION pos);
    STDMETHODIMP_(REFERENCE_TIME) GetStart(POSITION pos, double fps);
    STDMETHODIMP_(REFERENCE_TIME) GetStop(POSITION pos, double fps);
    STDMETHODIMP_(bool) IsAnimated(POSITION pos);
    STDMETHODIMP Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox);
    STDMETHODIMP GetRelativeTo(POSITION pos, RelativeTo& relativeTo);

    // IPersist
    STDMETHODIMP GetClassID(CLSID* pClassID);

    // ISubStream
    STDMETHODIMP_(int) GetStreamCount();
    STDMETHODIMP GetStreamInfo(int i, WCHAR** ppName, LCID* pLCID);
    STDMETHODIMP_(int) GetStream();
    STDMETHODIMP SetStream(int iStream);
    STDMETHODIMP Reload() { return E_NOTIMPL; }
    STDMETHODIMP SetSourceTargetInfo(CString yuvMatrix, int targetBlackLevel, int targetWhiteLevel) { return E_NOTIMPL; }
};
