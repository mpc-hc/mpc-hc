/*
 * $Id$
 *
 * (C) 2006-2012 see Authors.txt
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

// Internal codec list (use to enable/disable codec in standalone mode)
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
	MPCVD_H264_DXVA	 = 1 << 11,
	MPCVD_VC1_DXVA	 = 1 << 12,
	MPCVD_VP356      = 1 << 13,
	MPCVD_VP8        = 1 << 14,
	MPCVD_MJPEG      = 1 << 15,
	MPCVD_INDEO      = 1 << 16,
	MPCVD_RV         = 1 << 17,
	MPCVD_WMV3_DXVA  = 1 << 19,
	MPCVD_MPEG2_DXVA = 1 << 20,
} MPC_VIDEO_CODEC;

interface __declspec(uuid("CDC3B5B3-A8B0-4c70-A805-9FC80CDEF262"))
IMPCVideoDecFilter :
public IUnknown {
	STDMETHOD(Apply()) = 0;

	STDMETHOD(SetThreadNumber(int nValue)) = 0;
	STDMETHOD_(int, GetThreadNumber()) = 0;

	STDMETHOD(SetDiscardMode(int nValue)) = 0;
	STDMETHOD_(int, GetDiscardMode()) = 0;

	STDMETHOD(SetErrorRecognition(int nValue)) = 0;
	STDMETHOD_(int, GetErrorRecognition()) = 0;

	STDMETHOD(SetIDCTAlgo(int nValue)) = 0;
	STDMETHOD_(int, GetIDCTAlgo()) = 0;

	STDMETHOD_(GUID*, GetDXVADecoderGuid()) = 0;

	STDMETHOD(SetActiveCodecs(MPC_VIDEO_CODEC nValue)) = 0;
	STDMETHOD_(MPC_VIDEO_CODEC, GetActiveCodecs()) = 0;

	STDMETHOD_(LPCTSTR, GetVideoCardDescription()) = 0;

	STDMETHOD(SetARMode(int nValue)) = 0;
	STDMETHOD_(int, GetARMode()) = 0;

	STDMETHOD(SetDXVACheckCompatibility(int nValue)) = 0;
	STDMETHOD_(int, GetDXVACheckCompatibility()) = 0;

	STDMETHOD(SetDXVA_SD(int nValue)) = 0;
	STDMETHOD_(int, GetDXVA_SD()) = 0;
};

interface __declspec(uuid("F0ABC515-19ED-4D65-9D5F-59E36AE7F2AF"))
IMPCVideoDecFilter2 :
public IUnknown {
	STDMETHOD_(int, GetFrameType()) = 0;
};
