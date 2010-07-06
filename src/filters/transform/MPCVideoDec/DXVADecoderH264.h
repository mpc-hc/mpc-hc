/*
 * $Id$
 *
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#pragma once

#include <dxva.h>
#include "DXVADecoder.h"
#include "H264QuantizationMatrix.h"


#define MAX_SLICES 16		// Also define in ffmpeg!

class CDXVADecoderH264 : public CDXVADecoder
{
public:
	CDXVADecoderH264 (CMPCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, DXVAMode nMode, int nPicEntryNumber);
	CDXVADecoderH264 (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber, DXVA2_ConfigPictureDecode* pDXVA2Config);
	virtual ~CDXVADecoderH264();

	virtual HRESULT DecodeFrame   (BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop);
	virtual void	SetExtraData  (BYTE* pDataIn, UINT nSize);
	virtual void	CopyBitstream (BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize);
	virtual void	Flush();

protected :
	virtual int		FindOldestFrame();

private:

	DXVA_PicParams_H264		m_DXVAPicParams;
	DXVA_Qmatrix_H264		m_DXVAScalingMatrix;
	DXVA_Slice_H264_Short	m_pSliceShort[MAX_SLICES];
	DXVA_Slice_H264_Long	m_pSliceLong[MAX_SLICES];
	UINT					m_nMaxSlices;
	int						m_nNALLength;
	bool					m_bUseLongSlice;
	int						m_nOutPOC;
	REFERENCE_TIME			m_rtOutStart;
	REFERENCE_TIME			m_rtLastFrameDisplayed;

	// Private functions
	void					Init();
	HRESULT					DisplayStatus();

	// DXVA functions
	void					RemoveUndisplayedFrame(int nPOC);
	void					ClearRefFramesList();
	void					ClearUnusedRefFrames();
};
