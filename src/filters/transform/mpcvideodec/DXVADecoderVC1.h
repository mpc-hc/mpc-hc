/* 
 * $Id: DXVADecoderVC1.h 249 2007-09-26 11:07:22Z casimir666 $
 *
 * (C) 2006-2007 see AUTHORS
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

class CDXVADecoderVC1 :	public CDXVADecoder
{
public:
	CDXVADecoderVC1 (CMPCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, DXVAMode nMode, int nPicEntryNumber);
	CDXVADecoderVC1 (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber);
	virtual ~CDXVADecoderVC1(void);

	// === Public functions
	virtual HRESULT DecodeFrame   (BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop);
	virtual void	SetExtraData  (BYTE* pDataIn, UINT nSize);
	virtual void	CopyBitstream (BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize);
	virtual void	Flush();

	typedef enum
	{
		VC1_PS_TOP_FIELD				= 1,
		VC1_PS_BOTTOM_FIELD				= 2,
		VC1_PS_PROGRESSIVE				= 3
	};

	typedef enum
	{
		VC1_CHROMA_420					= 1,
		VC1_CHROMA_422					= 2,
		VC1_CHROMA_444					= 3
	};

	typedef enum
	{
		VC1_CR_BICUBIC_QUARTER_CHROMA	= 4,
		VC1_CR_BICUBIC_HALF_CHROMA		= 5,
		VC1_CR_BILINEAR_QUARTER_CHROMA	= 12,
		VC1_CR_BILINEAR_HALF_CHROMA		= 13,
	};

	typedef enum
	{
		VC1_SCAN_ZIGZAG					= 0,
		VC1_SCAN_ALTERNATE_VERTICAL		= 1,
		VC1_SCAN_ALTERNATE_HORIZONTAL	= 2,
		VC1_SCAN_ARBITRARY				= 3		// Use when bConfigHostInverseScan = 1
	};

protected :

private:
	DXVA_PictureParameters		m_PictureParams;
	DXVA_SliceInfo				m_SliceInfo;

	// Private functions
	void					Init();

};
