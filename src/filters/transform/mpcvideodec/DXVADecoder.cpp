/* 
 * $Id: DXVADecoderH264.cpp 249 2007-09-26 11:07:22Z casimir666 $
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

#include "stdafx.h"
#include <dxva2api.h>
#include "DXVADecoderH264.h"


CDXVADecoder* CDXVADecoder::CreateDecoder (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDXDecoder, const GUID* guidDecoder)
{
	CDXVADecoder*		pDecoder = NULL;

	if (*guidDecoder == DXVA2_ModeH264_E)
		pDecoder	= new CDXVADecoderH264 (pFilter, pDXDecoder, H264_VLD);
	else
		ASSERT (FALSE);	// Unknown decoder !!

	return pDecoder;
}

CDXVADecoder::CDXVADecoder (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDXDecoder, DXVA2Mode nMode)	
{
	m_pFilter		= pFilter;
	m_nMode			= nMode; 
	m_pDXDecoder	= pDXDecoder;

	memset (&m_ExecuteParams, 0, sizeof(m_ExecuteParams));
};


void CDXVADecoder::AllocExecuteParams (int nSize)
{
	m_ExecuteParams.NumCompBuffers		= nSize;
	m_ExecuteParams.pCompressedBuffers	= new DXVA2_DecodeBufferDesc[nSize];

	for (int i=0; i<nSize; i++)
		memset (&m_ExecuteParams.pCompressedBuffers[i], 0, sizeof(DXVA2_DecodeBufferDesc));
}


HRESULT CDXVADecoder::AddExecuteBuffer (DWORD CompressedBufferType, UINT nSize, void* pBuffer)
{
	HRESULT						hr;
	UINT						nDXVASize;
	BYTE*						pDXVABuffer;

	hr = m_pDXDecoder->GetBuffer (CompressedBufferType, (void**)&pDXVABuffer, &nDXVASize);
	ASSERT (nSize <= nDXVASize);

	if (SUCCEEDED (hr) && (nSize <= nDXVASize))
	{
		LOG(_T("GetBuffer %d :  hr=0x%08x   DXsize=%d  BuffSize=%d"), CompressedBufferType, hr, nDXVASize, nSize);

		//	TODO : patch pour H264 à faire !!
		if (CompressedBufferType == DXVA2_BitStreamDateBufferType)
		{
			pDXVABuffer[0]=pDXVABuffer[1]=0; pDXVABuffer[2]=1;
			pDXVABuffer += 3;
		}

		memcpy (pDXVABuffer, (BYTE*)pBuffer, nSize);

		if (CompressedBufferType == DXVA2_BitStreamDateBufferType)
		{
			// For H264 bitstream buffers should be multiple of 128
			nSize += 3;
			int		nDummy = 128 - (nSize %128);
			pDXVABuffer += nSize;
			memset (pDXVABuffer, 0, nDummy);
			nSize += nDummy;
		}

		hr = m_pDXDecoder->ReleaseBuffer (CompressedBufferType);
		m_ExecuteParams.pCompressedBuffers[m_ExecuteParams.NumCompBuffers].CompressedBufferType = CompressedBufferType;
		m_ExecuteParams.pCompressedBuffers[m_ExecuteParams.NumCompBuffers].DataSize				= nSize;
		m_ExecuteParams.pCompressedBuffers[m_ExecuteParams.NumCompBuffers].NumMBsInBuffer		= (CompressedBufferType == DXVA2_SliceControlBufferType) || (CompressedBufferType == DXVA2_BitStreamDateBufferType);
		m_ExecuteParams.NumCompBuffers++;
	}
	return hr;
}


void CDXVADecoder::SetExtraData (BYTE* pDataIn, UINT nSize)
{
	// Extradata is codec dependant
	UNREFERENCED_PARAMETER (pDataIn);
	UNREFERENCED_PARAMETER (nSize);
}