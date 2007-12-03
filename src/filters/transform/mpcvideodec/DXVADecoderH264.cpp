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
	memset (&m_PicParams,  0, sizeof(m_PicParams));
	memset (&m_nQMatrix,   0, sizeof(m_nQMatrix));
	memset (&m_SliceShort, 0, sizeof(m_SliceShort));
	m_nQMatrix	= Flat16;	// TODO : get from config!

	switch (nMode)
	{
	case H264_VLD :
		AllocExecuteParams (3);

		m_PicParams.wFrameWidthInMbsMinus1	= 79;
		m_PicParams.wFrameHeightInMbsMinus1	= 44;
		m_PicParams.num_ref_frames			= 6;
		m_PicParams.wBitFields				= 64592;

		for (int i =0; i<16; i++)
		{
			m_PicParams.RefFrameList[i].AssociatedFlag	= 1;
			m_PicParams.RefFrameList[i].bPicEntry			= 255;
			m_PicParams.RefFrameList[i].Index7Bits		= 127;
		}

		m_PicParams.ContinuationFlag						= 1;

		m_PicParams.log2_max_frame_num_minus4				= 5;
		m_PicParams.pic_order_cnt_type						= 0;
		m_PicParams.log2_max_pic_order_cnt_lsb_minus4		= 6;
		m_PicParams.delta_pic_order_always_zero_flag		= 0;
		m_PicParams.direct_8x8_inference_flag				= 1;
		m_PicParams.entropy_coding_mode_flag				= 1;
		m_PicParams.pic_order_present_flag					= 0;
		m_PicParams.num_slice_groups_minus1					= 0;
		m_PicParams.slice_group_map_type					= 0;
		m_PicParams.deblocking_filter_control_present_flag	= 1;
		m_PicParams.redundant_pic_cnt_present_flag			= 0;
		m_PicParams.Reserved8BitsB							= 0;
		m_PicParams.slice_group_change_rate_minus1			= 0;

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
	int							nBuffCount;

	pSampleService = pOut;

	hr = pSampleService->GetService (MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void**) &pDecoderRenderTarget);
	LOG(_T("pSampleService->GetService  hr=0x%08x"), hr);
	hr = m_pDXDecoder->BeginFrame(pDecoderRenderTarget, NULL);
	LOG(_T("m_pDXDecoder->BeginFrame  hr=0x%08x"), hr);

	// ==>> http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=1659948&SiteID=1

	m_ExecuteParams.NumCompBuffers	= 0;
	hr = AddExecuteBuffer (DXVA2_PictureParametersBufferType, sizeof(m_PicParams), &m_PicParams);
	hr = m_pDXDecoder->Execute(&m_ExecuteParams);

	m_ExecuteParams.NumCompBuffers	= 0;
	// TODO !!  ==> is bConfigBitstreamRaw =2 alors DXVA_Slice_H264_Short, sinon DXVA_Slice_H264_Long

	//FILE*	hFile = fopen ("BitStream.bin", "rb");
	//if (hFile)
	//{
	//	BYTE		pBuff[30000];
	//	nSize = fread (pBuff, 1, sizeof(pBuff), hFile);
	//	fclose (hFile);
	//	hr = AddExecuteBuffer (DXVA2_BitStreamDateBufferType, nSize, pBuff);
	//	m_SliceShort.SliceBytesInBuffer = nSize;
	//}
	hr = AddExecuteBuffer (DXVA2_BitStreamDateBufferType, nSize, pDataIn);

	m_SliceShort.SliceBytesInBuffer = nSize;
	hr = AddExecuteBuffer (DXVA2_SliceControlBufferType, sizeof (m_SliceShort), &m_SliceShort);
	hr = AddExecuteBuffer (DXVA2_InverseQuantizationMatrixBufferType, sizeof (DXVA_Qmatrix_H264), (void*)&g_QMatrixH264[m_nQMatrix]);
//m_ExecuteParams.pCompressedBuffers[0].NumMBsInBuffer = 1;
//m_ExecuteParams.pCompressedBuffers[1].NumMBsInBuffer = 1;
	hr = m_pDXDecoder->Execute(&m_ExecuteParams);
	LOG(_T("m_pDXDecoder->Execute  hr=0x%08x"), hr);

	hr = m_pDXDecoder->EndFrame(NULL);

	/*
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
		m_ExecuteParams.pCompressedBuffers[1].DataSize = nSize;

		m_ExecuteParams.NumCompBuffers = 3;
		hr = m_pDXDecoder->Execute(&m_ExecuteParams);
		LOG(_T("m_pDXDecoder->Execute  hr=0x%08x"), hr);
	}
*/
	CComPtr<IDirect3DDevice9>		pD3DDev;
	if (SUCCEEDED (pDecoderRenderTarget->GetDevice (&pD3DDev)))
	{
		RECT		rcTearing;
		
		rcTearing.left		= 0;
		rcTearing.top		= 0;
		rcTearing.right		= 80;
		rcTearing.bottom	= 80;

		pD3DDev->ColorFill (pDecoderRenderTarget, &rcTearing, D3DCOLOR_ARGB (255,0,0,255));
	}

	return hr;
}