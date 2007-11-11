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
#include "DXVADecoderH264.h"

CDXVADecoderH264::CDXVADecoderH264 (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDecoder, DXVA2Mode nMode)
				: CDXVADecoder (pFilter, pDecoder, nMode)
{
	memset (&m_PicParams, 0, sizeof(m_PicParams));
	memset (&m_nQMatrix,  0, sizeof(m_nQMatrix));
	m_nQMatrix	= JVTMatrix;	// TODO : get from config!

	switch (nMode)
	{
	case H264_VLD :
		AllocExecuteParams (4);
		m_ExecuteParams.pCompressedBuffers[0].CompressedBufferType = DXVA2_PictureParametersBufferType;
		m_ExecuteParams.pCompressedBuffers[1].CompressedBufferType = DXVA2_SliceControlBufferType;
		m_ExecuteParams.pCompressedBuffers[2].CompressedBufferType = DXVA2_InverseQuantizationMatrixBufferType;
		m_ExecuteParams.pCompressedBuffers[3].CompressedBufferType = DXVA2_BitStreamDateBufferType;
		break;
	}
}


HRESULT CDXVADecoderH264::DecodeFrame (BYTE* pDataIn, UINT nSize, IMediaSample* pOut)
{
	HRESULT						hr;
	CComQIPtr<IMFGetService>	pSampleService;
	CComPtr<IDirect3DSurface9>	pDecoderRenderTarget;
	BYTE*						pDXVABuffer;
	UINT						nDXVASize;

	pSampleService = pOut;

	pSampleService->GetService (MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void**) &pDecoderRenderTarget);
	LOG(_T("pSampleService->GetService  hr=0x%08x"), hr);
	m_pDXDecoder->BeginFrame(pDecoderRenderTarget, NULL);
	LOG(_T("m_pDXDecoder->BeginFrame  hr=0x%08x"), hr);

	// ==>> http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=1659948&SiteID=1
	// TODO !!	==>>  DXVA_PicParams_H264
	hr = m_pDXDecoder->GetBuffer (DXVA2_PictureParametersBufferType, (void**)&pDXVABuffer, &nDXVASize);
	LOG(_T("GetBuffer DXVA2_PictureParametersBufferType :  hr=0x%08x   DXsize=%d  BuffSize=%d"), hr, nDXVASize, sizeof(m_PicParams));
	memcpy (pDXVABuffer, &m_PicParams, sizeof (m_PicParams));
	hr = m_pDXDecoder->ReleaseBuffer (DXVA2_PictureParametersBufferType);

	// TODO !!  ==> is bConfigBitstreamRaw =2 alors DXVA_Slice_H264_Short, sinon DXVA_Slice_H264_Long
	hr = m_pDXDecoder->GetBuffer (DXVA2_SliceControlBufferType, (void**)&pDXVABuffer, &nDXVASize);
	LOG(_T("GetBuffer DXVA2_SliceControlBufferType :  hr=0x%08x   size=%d"), hr, nDXVASize);

	m_pSliceShort.BSNALunitDataLocation	= 0;			// TODO ???????????????????????????????????
	m_pSliceShort.SliceBytesInBuffer	= nSize;
	m_pSliceShort.wBadSliceChopping		= 0;
	memcpy (pDXVABuffer, &m_pSliceShort, sizeof (m_pSliceShort));
	hr = m_pDXDecoder->ReleaseBuffer (DXVA2_SliceControlBufferType);

	// TODO !!	==>> DXVA_Qmatrix_H264
	hr = m_pDXDecoder->GetBuffer (DXVA2_InverseQuantizationMatrixBufferType, (void**)&pDXVABuffer, &nDXVASize);
	LOG(_T("GetBuffer DXVA2_InverseQuantizationMatrixBufferType :  hr=0x%08x   size=%d"), hr, nDXVASize);
	memcpy (pDXVABuffer, &g_QMatrixH264[m_nQMatrix], sizeof (DXVA_Qmatrix_H264));
	hr = m_pDXDecoder->ReleaseBuffer (DXVA2_InverseQuantizationMatrixBufferType);

	hr = m_pDXDecoder->GetBuffer (DXVA2_BitStreamDateBufferType, (void**)&pDXVABuffer, &nDXVASize);
	LOG(_T("GetBuffer DXVA2_BitStreamDateBufferType :  hr=0x%08x   size=%d"), hr, nDXVASize);
	if (SUCCEEDED (hr))
	//if (SUCCEEDED (hr = pSampleService->GetService (MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void**) &pDecoderRenderTarget)) &&
	//	SUCCEEDED (hr = pDecoder->BeginFrame(pDecoderRenderTarget, NULL)) &&
	//	SUCCEEDED (hr = pDecoder->GetBuffer (DXVA2_BitStreamDateBufferType, (void**)&pDXVABuffer, &nDXVASize)) )
	{
		ASSERT (nDXVASize > nSize);
		if (nSize > nDXVASize)	 return VFW_E_BUFFER_OVERFLOW;
		CheckPointer (pDataIn, E_POINTER);
		memcpy (pDXVABuffer, pDataIn, nSize);
		hr = m_pDXDecoder->ReleaseBuffer (DXVA2_BitStreamDateBufferType);
		LOG(_T("m_pDXDecoder->ReleaseBuffer  hr=0x%08x"), hr);

		// TODO ????
		m_ExecuteParams.NumCompBuffers = 1;
		m_ExecuteParams.pCompressedBuffers[m_ExecuteParams.NumCompBuffers].DataSize = nSize;

		hr = m_pDXDecoder->Execute(&m_ExecuteParams);
		LOG(_T("m_pDXDecoder->Execute  hr=0x%08x"), hr);
	}

	return hr;
}