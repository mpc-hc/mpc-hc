/*
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

// Internal codec list (used to enable/disable codec in standalone mode)
typedef enum {
    MPCVD_H264       = 1 <<  0,
    MPCVD_VC1        = 1 <<  1,
    MPCVD_XVID       = 1 <<  2,
    MPCVD_DIVX       = 1 <<  3,
    MPCVD_WMV        = 1 <<  4,
    MPCVD_MSMPEG4    = 1 <<  5,
    MPCVD_H263       = 1 <<  6,
    MPCVD_SVQ3       = 1 <<  7,
    MPCVD_THEORA     = 1 <<  8,
    MPCVD_AMVV       = 1 <<  9,
    MPCVD_FLASH      = 1 << 10,
    MPCVD_H264_DXVA  = 1 << 11,
    MPCVD_VC1_DXVA   = 1 << 12,
    MPCVD_VP356      = 1 << 13,
    MPCVD_VP8        = 1 << 14,
    MPCVD_MJPEG      = 1 << 15,
    MPCVD_INDEO      = 1 << 16,
    MPCVD_RV         = 1 << 17,
    MPCVD_WMV3_DXVA  = 1 << 19,
    MPCVD_MPEG2_DXVA = 1 << 20
} MPC_VIDEO_CODEC;

// Interlaced flag handling
enum MPCVD_INTERLACED_FLAG {
    MPCVC_INTERLACED_AUTO,
    MPCVC_INTERLACED_PROGRESSIVE,
    MPCVC_INTERLACED_TOP_FIELD_FIRST,
    MPCVC_INTERLACED_BOTTOM_FIELD_FIRST
};

interface __declspec(uuid("CDC3B5B3-A8B0-4c70-A805-9FC80CDEF262"))
    IMPCVideoDecFilter :
    public IUnknown
{
    STDMETHOD(Apply()) PURE;

    STDMETHOD(SetThreadNumber(int nValue)) PURE;
    STDMETHOD_(int, GetThreadNumber()) PURE;

    STDMETHOD(SetDiscardMode(int nValue)) PURE;
    STDMETHOD_(int, GetDiscardMode()) PURE;

    STDMETHOD_(GUID*, GetDXVADecoderGuid()) PURE;

    STDMETHOD(SetActiveCodecs(MPC_VIDEO_CODEC nValue)) PURE;
    STDMETHOD_(MPC_VIDEO_CODEC, GetActiveCodecs()) PURE;

    STDMETHOD_(LPCTSTR, GetVideoCardDescription()) PURE;

    STDMETHOD(SetARMode(int nValue)) PURE;
    STDMETHOD_(int, GetARMode()) PURE;

    STDMETHOD(SetDXVACheckCompatibility(int nValue)) PURE;
    STDMETHOD_(int, GetDXVACheckCompatibility()) PURE;

    STDMETHOD(SetDXVA_SD(int nValue)) PURE;
    STDMETHOD_(int, GetDXVA_SD()) PURE;
};

interface __declspec(uuid("F0ABC515-19ED-4D65-9D5F-59E36AE7F2AF"))
    IMPCVideoDecFilter2 :
    public IMPCVideoDecFilter
{
    STDMETHOD_(int, GetFrameType()) PURE;

    STDMETHOD(SetInterlacedFlag(MPCVD_INTERLACED_FLAG interlacedFlag)) PURE;
    STDMETHOD_(MPCVD_INTERLACED_FLAG, GetInterlacedFlag()) PURE;
};
