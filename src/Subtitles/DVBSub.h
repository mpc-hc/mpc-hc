/*
 * (C) 2009-2014 see Authors.txt
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

    virtual HRESULT ParseSample(IMediaSample* pSample);
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
        BYTE    id;
        BYTE    version_number;
        BYTE    size;

        HDMV_PALETTE palette[256];

        DVB_CLUT()
            : id(0)
            , version_number(0)
            , size(0) {
            ZeroMemory(palette, sizeof(palette));
        }
    };

    struct DVB_DISPLAY {
        BYTE        version_number;
        BYTE        display_window_flag;
        short       width;
        short       height;
        short       horizontal_position_minimun;
        short       horizontal_position_maximum;
        short       vertical_position_minimun;
        short       vertical_position_maximum;

        DVB_DISPLAY()
        // Default value (section 5.1.3)
            : version_number(0)
            , display_window_flag(0)
            , width(720)
            , height(576)
            , horizontal_position_minimun(0)
            , horizontal_position_maximum(0)
            , vertical_position_minimun(0)
            , vertical_position_maximum(0) {
        }
    };

    struct DVB_OBJECT {
        short       object_id;
        BYTE        object_type;
        BYTE        object_provider_flag;
        short       object_horizontal_position;
        short       object_vertical_position;
        BYTE        foreground_pixel_code;
        BYTE        background_pixel_code;

        DVB_OBJECT()
            : object_id(0xFF)
            , object_type(0)
            , object_provider_flag(0)
            , object_horizontal_position(0)
            , object_vertical_position(0)
            , foreground_pixel_code(0)
            , background_pixel_code(0) {
        }
    };

    struct DVB_REGION_POS {
        BYTE       id;
        WORD       horizAddr;
        WORD       vertAddr;

        DVB_REGION_POS()
            : id(0)
            , horizAddr(0)
            , vertAddr(0) {
        }
    };

    struct DVB_REGION {
        BYTE       id;
        BYTE       version_number;
        BYTE       fill_flag;
        WORD       width;
        WORD       height;
        BYTE       level_of_compatibility;
        BYTE       depth;
        BYTE       CLUT_id;
        BYTE       _8_bit_pixel_code;
        BYTE       _4_bit_pixel_code;
        BYTE       _2_bit_pixel_code;
        CAtlList<DVB_OBJECT> objects;

        DVB_REGION()
            : id(0)
            , version_number(0)
            , fill_flag(0)
            , width(0)
            , height(0)
            , level_of_compatibility(0)
            , depth(0)
            , CLUT_id(0)
            , _8_bit_pixel_code(0)
            , _4_bit_pixel_code(0)
            , _2_bit_pixel_code(0) {
        }

        DVB_REGION(const CDVBSub::DVB_REGION& region)
            : id(region.id)
            , version_number(region.version_number)
            , fill_flag(region.fill_flag)
            , width(region.width)
            , height(region.height)
            , level_of_compatibility(region.level_of_compatibility)
            , depth(region.depth)
            , CLUT_id(region.CLUT_id)
            , _8_bit_pixel_code(region._8_bit_pixel_code)
            , _4_bit_pixel_code(region._4_bit_pixel_code)
            , _2_bit_pixel_code(region._2_bit_pixel_code)  {
            objects.AddHeadList(&region.objects);
        }
    };

    class DVB_PAGE
    {
    public:
        REFERENCE_TIME                  rtStart;
        REFERENCE_TIME                  rtStop;
        BYTE                            pageTimeOut;
        BYTE                            pageVersionNumber;
        BYTE                            pageState;
        CAtlList<DVB_REGION_POS>        regionsPos;
        CAutoPtrList<DVB_REGION>        regions;
        CAutoPtrList<CompositionObject> objects;
        CAutoPtrList<DVB_CLUT>          CLUTs;
        bool                            rendered;

        DVB_PAGE()
            : pageTimeOut(0)
            , pageVersionNumber(0)
            , pageState(0)
            , rendered(false)
            , rtStart(0)
            , rtStop(0) {
        }
    };

    int                    m_nBufferSize;
    int                    m_nBufferReadPos;
    int                    m_nBufferWritePos;
    BYTE*                  m_pBuffer;
    CAutoPtrList<DVB_PAGE> m_pages;
    CAutoPtr<DVB_PAGE>     m_pCurrentPage;
    DVB_DISPLAY            m_displayInfo;
    REFERENCE_TIME         m_rtStart;
    REFERENCE_TIME         m_rtStop;

    HRESULT  AddToBuffer(BYTE* pData, int nSize);

    POSITION FindPage(REFERENCE_TIME rt) const;
    POSITION FindRegion(const CAutoPtr<DVB_PAGE>& pPage, BYTE bRegionId) const;
    POSITION FindClut(const CAutoPtr<DVB_PAGE>& pPage, BYTE bClutId) const;
    POSITION FindObject(const CAutoPtr<DVB_PAGE>& pPage, short sObjectId) const;

    HRESULT  ParsePage(CGolombBuffer& gb, WORD wSegLength, CAutoPtr<DVB_PAGE>& pPage);
    HRESULT  ParseDisplay(CGolombBuffer& gb, WORD wSegLength);
    HRESULT  ParseRegion(CGolombBuffer& gb, WORD wSegLength);
    HRESULT  ParseClut(CGolombBuffer& gb, WORD wSegLength);
    HRESULT  ParseObject(CGolombBuffer& gb, WORD wSegLength);

    HRESULT  EnqueuePage(REFERENCE_TIME rtStop);
    HRESULT  UpdateTimeStamp(REFERENCE_TIME rtStop);

    void     RemoveOldPages(REFERENCE_TIME rt);
};
