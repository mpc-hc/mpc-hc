/*
 * (C) 2006-2015 see Authors.txt
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

#include "RLECodedSubtitle.h"
#include "CompositionObject.h"
#include <thread>
#include <list>
#include <memory>

class CGolombBuffer;

class CPGSSub : public CRLECodedSubtitle
{
public:
    CPGSSub(CCritSec* pLock, const CString& name, LCID lcid);
    virtual ~CPGSSub();

    // ISubPicProvider
    STDMETHODIMP_(POSITION)       GetStartPosition(REFERENCE_TIME rt, double fps);
    STDMETHODIMP_(POSITION)       GetNext(POSITION pos);
    STDMETHODIMP_(REFERENCE_TIME) GetStart(POSITION pos, double fps);
    STDMETHODIMP_(REFERENCE_TIME) GetStop(POSITION pos, double fps);
    STDMETHODIMP_(bool)           IsAnimated(POSITION pos);
    STDMETHODIMP                  Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox);
    STDMETHODIMP                  GetTextureSize(POSITION pos, SIZE& MaxTextureSize, SIZE& VirtualSize, POINT& VirtualTopLeft);
    STDMETHODIMP                  GetRelativeTo(POSITION pos, RelativeTo& relativeTo);

    virtual HRESULT ParseSample(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, BYTE* pData, size_t nLen);
    virtual void    EndOfStream() { /* Nothing to do */ };
    virtual void    Reset();

protected:
    HRESULT Render(SubPicDesc& spd, REFERENCE_TIME rt, RECT& bbox, bool bRemoveOldSegments);

private:
    enum HDMV_SEGMENT_TYPE {
        NO_SEGMENT       = 0xFFFF,
        PALETTE          = 0x14,
        OBJECT           = 0x15,
        PRESENTATION_SEG = 0x16,
        WINDOW_DEF       = 0x17,
        INTERACTIVE_SEG  = 0x18,
        END_OF_DISPLAY   = 0x80,
        HDMV_SUB1        = 0x81,
        HDMV_SUB2        = 0x82
    };


    struct VIDEO_DESCRIPTOR {
        int  nVideoWidth;
        int  nVideoHeight;
        BYTE bFrameRate;
    };

    struct COMPOSITION_DESCRIPTOR {
        short nNumber;
        BYTE  bState;
    };

    struct SEQUENCE_DESCRIPTOR {
        BYTE bFirstIn  : 1;
        BYTE bLastIn   : 1;
        BYTE bReserved : 6;
    };

    struct HDMV_CLUT {
        BYTE id = 0;
        BYTE version_number = 0;
        WORD size = 0;

        std::array<HDMV_PALETTE, 256> palette;

        HDMV_CLUT()
            : palette() {
        }
    };

    struct HDMV_PRESENTATION_SEGMENT {
        REFERENCE_TIME rtStart;
        REFERENCE_TIME rtStop;

        VIDEO_DESCRIPTOR video_descriptor;
        COMPOSITION_DESCRIPTOR composition_descriptor;

        byte palette_update_flag;
        HDMV_CLUT CLUT;

        int objectCount;

        std::list<std::unique_ptr<CompositionObject>> objects;
    };

    HDMV_SEGMENT_TYPE m_nCurSegment;
    BYTE*             m_pSegBuffer;
    size_t            m_nTotalSegBuffer;
    size_t            m_nSegBufferPos;
    size_t            m_nSegSize;

    CAutoPtr<HDMV_PRESENTATION_SEGMENT>     m_pCurrentPresentationSegment;
    CAutoPtrList<HDMV_PRESENTATION_SEGMENT> m_pPresentationSegments;

    std::array<HDMV_CLUT, 256> m_CLUTs;
    std::array<CompositionObject, 64> m_compositionObjects;

    void AllocSegment(size_t nSize);
    int  ParsePresentationSegment(REFERENCE_TIME rt, CGolombBuffer* pGBuffer);
    void EnqueuePresentationSegment();
    void UpdateTimeStamp(REFERENCE_TIME rtStop);

    void ParsePalette(CGolombBuffer* pGBuffer, size_t nSize);
    void ParseObject(CGolombBuffer* pGBuffer, size_t nUnitSize);
    void ParseVideoDescriptor(CGolombBuffer* pGBuffer, VIDEO_DESCRIPTOR* pVideoDescriptor);
    void ParseCompositionDescriptor(CGolombBuffer* pGBuffer, COMPOSITION_DESCRIPTOR* pCompositionDescriptor);
    bool ParseCompositionObject(CGolombBuffer* pGBuffer, const std::unique_ptr<CompositionObject>& pCompositionObject);

    POSITION FindPresentationSegment(REFERENCE_TIME rt) const;

    void RemoveOldSegments(REFERENCE_TIME rt);
};

class CPGSSubFile : public CPGSSub
{
public:
    CPGSSubFile(CCritSec* pLock);
    virtual ~CPGSSubFile();

    // ISubPicProvider
    STDMETHODIMP Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox);

    bool Open(CString fn, CString name = _T(""), CString videoName = _T(""));

private:
    static const WORD PGS_SYNC_CODE = 'PG';

    bool m_bStopParsing;
    std::thread m_parsingThread;

    void ParseFile(CString fn);
};
