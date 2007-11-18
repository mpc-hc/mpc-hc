/* 
 * $Id: DXVADecoder.h 249 2007-09-26 11:07:22Z casimir666 $
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

#include <dxva2api.h>

typedef enum
{
	H264_VLD,
	VC1_VLD
} DXVA2Mode;


class CMPCVideoDecFilter;

class CDXVADecoder
{
public :
	CDXVADecoder (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDXDecoder, DXVA2Mode nMode);

	DXVA2Mode		GetMode()		{ return m_nMode; };
	void			AllocExecuteParams (int nSize);

	virtual HRESULT DecodeFrame (BYTE* pDataIn, UINT nSize, IMediaSample* pOut) = NULL;

	static CDXVADecoder* CreateDecoder (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDecoder, const GUID* guidDecoder);


protected :
	DXVA2Mode						m_nMode;
	CMPCVideoDecFilter*				m_pFilter;

	CComPtr<IDirectXVideoDecoder>	m_pDXDecoder;
	DXVA2_DecodeExecuteParams		m_ExecuteParams;


	HRESULT			AddExecuteBuffer (DWORD CompressedBufferType, UINT nSize, void* pBuffer);
};