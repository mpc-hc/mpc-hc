/*
 * (C) 2009-2015 see Authors.txt
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
#include <list>
#include <memory>

class CGolombBuffer;

class CDVBSub : public CRLECodedSubtitle
{
public:
    CDVBSub(CCritSec* pLock, const CString& name, LCID lcid);
    ~CDVBSub();

    // ISubPicProvider
    STDMETHODIMP_(POSITION)       GetStartPosition(REFERENCE_TIME rt, double fps);
    STDMETHODIMP_(POSITION)       GetNext(POSITION pos);
    STDMETHODIMP_(REFERENCE_TIME) GetStart(POSITION pos, double fps);
    STDMETHODIMP_(REFERENCE_TIME) GetStop(POSITION pos, double fps);
    STDMETHODIMP_(bool)           IsAnimated(POSITION pos);
    STDMETHODIMP                  Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox);
    STDMETHODIMP                  GetTextureSize(POSITION pos, SIZE& MaxTextureSize, SIZE& VirtualSize, POINT& VirtualTopLeft);

    virtual HRESULT ParseSample(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, BYTE* pData, size_t nLen);
    virtual void    EndOfStream();
    virtual void    Reset();

private:
    // EN 300-743, table 2
    enum DVB_SEGMENT_TYPE {
        NO_SEGMENT     = 0xFFFF,
        PAGE           = 0x10,
        REGION         = 0x11,
        CLUT           = 0x12,
        OBJECT         = 0x13,
        DISPLAY        = 0x14,
        END_OF_DISPLAY = 0x80
    };

    // EN 300-743, table 6
    enum DVB_OBJECT_TYPE {
        OT_BASIC_BITMAP     = 0x00,
        OT_BASIC_CHAR       = 0x01,
        OT_COMPOSITE_STRING = 0x02
    };

    enum DVB_PAGE_STATE {
        DPS_NORMAL      = 0x00,
        DPS_ACQUISITION = 0x01,
        DPS_MODE_CHANGE = 0x02,
        DPS_RESERVED    = 0x03
    };

    struct DVB_CLUT {
        BYTE id = 0;
        BYTE version_number = 0;
        WORD size = 0;

        std::array<HDMV_PALETTE, 256> palette;

        DVB_CLUT()
            : palette() {
        }
    };

    struct DVB_DISPLAY {
        // Default value (section 5.1.3)
        BYTE  version_number = 0;
        BYTE  display_window_flag = 0;
        short width = 720;
        short height = 576;
        short horizontal_position_minimun = 0;
        short horizontal_position_maximum = 0;
        short vertical_position_minimun = 0;
        short vertical_position_maximum = 0;
    };

    struct DVB_OBJECT {
        short object_id = 0;
        BYTE  object_type = 0;
        BYTE  object_provider_flag = 0;
        short object_horizontal_position = 0;
        short object_vertical_position = 0;
        BYTE  foreground_pixel_code = 0;
        BYTE  background_pixel_code = 0;
    };

    struct DVB_REGION_POS {
        BYTE id = 0;
        WORD horizAddr = 0;
        WORD vertAddr = 0;
    };

    struct DVB_REGION {
        BYTE id = 0;
        BYTE version_number = 0;
        BYTE fill_flag = 0;
        WORD width = 0;
        WORD height = 0;
        BYTE level_of_compatibility = 0;
        BYTE depth = 0;
        BYTE CLUT_id = 0;
        BYTE _8_bit_pixel_code = 0;
        BYTE _4_bit_pixel_code = 0;
        BYTE _2_bit_pixel_code = 0;
        std::list<DVB_OBJECT> objects;
    };

    using RegionList = std::list<std::unique_ptr<DVB_REGION>>;
    using CompositionObjectList = std::list<std::unique_ptr<CompositionObject>>;
    using ClutList = std::list<std::unique_ptr<DVB_CLUT>>;

    class DVB_PAGE
    {
    public:
        REFERENCE_TIME rtStart = 0;
        REFERENCE_TIME rtStop = 0;
        BYTE           pageTimeOut = 0;
        BYTE           pageVersionNumber = 0;
        BYTE           pageState = 0;
        std::list<DVB_REGION_POS> regionsPos;
        RegionList                regions;
        CompositionObjectList     objects;
        ClutList                  CLUTs;
        bool           rendered = false;
    };

    size_t                 m_nBufferSize;
    size_t                 m_nBufferReadPos;
    size_t                 m_nBufferWritePos;
    BYTE*                  m_pBuffer;
    CAutoPtrList<DVB_PAGE> m_pages;
    CAutoPtr<DVB_PAGE>     m_pCurrentPage;
    DVB_DISPLAY            m_displayInfo;

    HRESULT  AddToBuffer(BYTE* pData, size_t nSize);

    POSITION FindPage(REFERENCE_TIME rt) const;
    RegionList::const_iterator FindRegion(const CAutoPtr<DVB_PAGE>& pPage, BYTE bRegionId) const;
    ClutList::const_iterator   FindClut(const CAutoPtr<DVB_PAGE>& pPage, BYTE bClutId) const;
    CompositionObjectList::const_iterator FindObject(const CAutoPtr<DVB_PAGE>& pPage, short sObjectId) const;

    HRESULT  ParsePage(CGolombBuffer& gb, WORD wSegLength, CAutoPtr<DVB_PAGE>& pPage);
    HRESULT  ParseDisplay(CGolombBuffer& gb, WORD wSegLength);
    HRESULT  ParseRegion(CGolombBuffer& gb, WORD wSegLength);
    HRESULT  ParseClut(CGolombBuffer& gb, WORD wSegLength);
    HRESULT  ParseObject(CGolombBuffer& gb, WORD wSegLength);

    HRESULT  EnqueuePage(REFERENCE_TIME rtStop);
    HRESULT  UpdateTimeStamp(REFERENCE_TIME rtStop);

    void     RemoveOldPages(REFERENCE_TIME rt);
};
