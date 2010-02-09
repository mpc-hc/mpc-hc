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

#define MAX_SLICE		175			// Max slice number for Mpeg2 streams

class CDXVADecoderMpeg2 :	public CDXVADecoder
{
public:
	CDXVADecoderMpeg2 (CMPCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, DXVAMode nMode, int nPicEntryNumber);
	CDXVADecoderMpeg2 (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber, DXVA2_ConfigPictureDecode* pDXVA2Config);
	virtual ~CDXVADecoderMpeg2(void);

	// === Public functions
	virtual HRESULT DecodeFrame   (BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop);
	virtual void	SetExtraData  (BYTE* pDataIn, UINT nSize);
	virtual void	CopyBitstream (BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize);
	virtual void	Flush();

protected :

	virtual int		FindOldestFrame();
private:
	DXVA_PictureParameters		m_PictureParams;
	DXVA_QmatrixData			m_QMatrixData;	
	WORD						m_wRefPictureIndex[2];
	DXVA_SliceInfo				m_SliceInfo[MAX_SLICE];
	int							m_nSliceCount;

	int							m_nNextCodecIndex;

	// Private functions
	void					Init();
	void					UpdatePictureParams(int nSurfaceIndex);
};
