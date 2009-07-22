/* 
 * $Id$
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
#include "..\..\..\DSUtil\DSUtil.h"
#include "DXVADecoderH264.h"
#include "MPCVideoDecFilter.h"
#include "VideoDecDXVAAllocator.h"
#include "PODtypes.h"
#include "avcodec.h"

extern "C"
{
	#include "FfmpegContext.h"
}


#define SE_HEADER           0

static UINT g_UsedForReferenceFlags[] =
{
	0x00000001,
	0x00000003,
	0x00000007,
	0x0000000F,
	0x0000001F,
	0x0000003F,
	0x0000007F,
	0x000000FF,
	0x000001FF,
	0x000003FF,
	0x000007FF,
	0x00000FFF,
	0x00001FFF,
	0x00003FFF,
	0x00007FFF,
	0x0000FFFF,
	0x0001FFFF,
	0x0003FFFF,
	0x0007FFFF,
	0x000FFFFF,
	0x001FFFFF,
	0x003FFFFF,
	0x007FFFFF,
	0x00FFFFFF,
	0x01FFFFFF,
	0x03FFFFFF,
	0x07FFFFFF,
	0x0FFFFFFF,
	0x1FFFFFFF,
	0x3FFFFFFF,
	0x7FFFFFFF,
	0xFFFFFFFF,
};

CDXVADecoderH264::CDXVADecoderH264 (CMPCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, DXVAMode nMode, int nPicEntryNumber)
				: CDXVADecoder (pFilter, pAMVideoAccelerator, nMode, nPicEntryNumber)
{
	m_bUseLongSlice = (GetDXVA1Config()->bConfigBitstreamRaw != 2);
	Init();
}

CDXVADecoderH264::CDXVADecoderH264 (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber, DXVA2_ConfigPictureDecode* pDXVA2Config)
				: CDXVADecoder (pFilter, pDirectXVideoDec, nMode, nPicEntryNumber, pDXVA2Config)
{
	m_bUseLongSlice = (m_pFilter->GetDXVA2Config()->ConfigBitstreamRaw != 2);
	Init();
}

CDXVADecoderH264::~CDXVADecoderH264()
{
}

void CDXVADecoderH264::Init()
{
	memset (&m_DXVAPicParams,	0, sizeof(m_DXVAPicParams));
	memset (&m_DXVAPicParams, 0, sizeof (DXVA_PicParams_H264));
	
	m_DXVAPicParams.MbsConsecutiveFlag					= 1;
	if(m_pFilter->GetPCIVendor() == 0x8086) 
		m_DXVAPicParams.Reserved16Bits					= 0x534c;
	else
		m_DXVAPicParams.Reserved16Bits					= 0;
	m_DXVAPicParams.ContinuationFlag					= 1;
	m_DXVAPicParams.Reserved8BitsA						= 0;
	m_DXVAPicParams.Reserved8BitsB						= 0;
	m_DXVAPicParams.MinLumaBipredSize8x8Flag			= 1;	// Improve accelerator performances
	m_DXVAPicParams.StatusReportFeedbackNumber			= 0;	// Use to report status

	for (int i =0; i<16; i++)
	{
		m_DXVAPicParams.RefFrameList[i].AssociatedFlag	= 1;
		m_DXVAPicParams.RefFrameList[i].bPicEntry		= 255;
		m_DXVAPicParams.RefFrameList[i].Index7Bits		= 127;
	}


	m_nCurRefFrame		= 0;
	m_nNALLength		= 4;
	m_nMaxSlices		= 0;

	switch (GetMode())
	{
	case H264_VLD :
		AllocExecuteParams (3);
		break;
	default :
		ASSERT(FALSE);
	}
}


void CDXVADecoderH264::CopyBitstream(BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize)
{
	CH264Nalu		Nalu;
	int				nDummy;

	Nalu.SetBuffer (pBuffer, nSize, m_nNALLength);
	nSize = 0;

	while (Nalu.ReadNext())
	{
		switch (Nalu.GetType())
		{
		case NALU_TYPE_SLICE:
		case NALU_TYPE_IDR:
			// For AVC1, put startcode 0x000001
			pDXVABuffer[0]=pDXVABuffer[1]=0;pDXVABuffer[2]=1;
			
			// Copy NALU
			memcpy (pDXVABuffer+3, Nalu.GetDataBuffer(), Nalu.GetDataLength());
			
			// Add trailing bit
//			pDXVABuffer[Nalu.GetDataLength()+3] = 0x00;		// ???????

			pDXVABuffer	+= Nalu.GetDataLength() + 3;
			nSize       += Nalu.GetDataLength() + 3;

			break;
		}
	}

	// Complete with zero padding (buffer size should be a multiple of 128)
	nDummy  = 128 - (nSize %128);

	memset (pDXVABuffer, 0, nDummy);
	nSize  += nDummy;
}


void CDXVADecoderH264::Flush()
{
	ClearRefFramesList();
	m_DXVAPicParams.UsedForReferenceFlags	= 0;
	m_nOutPOC								= -1;

	__super::Flush();
}

HRESULT CDXVADecoderH264::DecodeFrame (BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{
	HRESULT						hr			= S_FALSE;
	CH264Nalu					Nalu;
	UINT						nSlices		= 0;
	int							nSurfaceIndex;
	int							nFieldType;
	int							nSliceType;
	int							nFramePOC;
	CComPtr<IMediaSample>		pSampleToDeliver;
	CComQIPtr<IMPCDXVA2Sample>	pDXVA2Sample;
	int							nDXIndex	= 0;
	UINT						nNalOffset	= 0;

	Nalu.SetBuffer (pDataIn, nSize, m_nNALLength); 
	FFH264DecodeBuffer (m_pFilter->GetAVCtx(), pDataIn, nSize, true,&nFramePOC, &m_nOutPOC, &m_rtOutStart);			

	while (Nalu.ReadNext())
	{
		switch (Nalu.GetType())
		{
		case NALU_TYPE_SLICE:
		case NALU_TYPE_IDR:
				if(m_bUseLongSlice) 
				{
					m_pSliceLong[nSlices].BSNALunitDataLocation	= nNalOffset;
					m_pSliceLong[nSlices].SliceBytesInBuffer	= Nalu.GetDataLength()+3; //.GetRoundedDataLength();
					m_pSliceLong[nSlices].wBadSliceChopping		= 0;
					m_pSliceLong[nSlices].slice_id				= nSlices;
					FF264UpdateRefFrameSliceLong(&m_DXVAPicParams, &m_pSliceLong[nSlices], m_pFilter->GetAVCtx());

					if (nSlices>0)
						m_pSliceLong[nSlices-1].NumMbsForSlice = m_pSliceLong[nSlices].NumMbsForSlice = m_pSliceLong[nSlices].first_mb_in_slice - m_pSliceLong[nSlices-1].first_mb_in_slice;

					nSlices++; 
				}
				else 
				{
					m_pSliceShort[nSlices].BSNALunitDataLocation	= nNalOffset;
					m_pSliceShort[nSlices].SliceBytesInBuffer		= Nalu.GetDataLength()+3;
					m_pSliceShort[nSlices].wBadSliceChopping		= 0;
					
					nSlices++;
				}
				nNalOffset += (UINT)(Nalu.GetDataLength() + 3);
				if (nSlices > MAX_SLICES) break;
				break;
		}
	}
	if (nSlices == 0) return S_FALSE;

	m_nMaxWaiting	= min (max (m_DXVAPicParams.num_ref_frames, 3), 8);

	// If parsing fail (probably no PPS/SPS), continue anyway it may arrived later (happen on truncated streams)
	if (FAILED (FFH264BuildPicParams (&m_DXVAPicParams, &m_DXVAScalingMatrix, &nFieldType, &nSliceType, m_pFilter->GetAVCtx(), m_pFilter->GetPCIVendor())))
		return S_FALSE;

	// Wait I frame after a flush
	if (m_bFlushed && !m_DXVAPicParams.IntraPicFlag)
		return S_FALSE;

	
	CHECK_HR (GetFreeSurfaceIndex (nSurfaceIndex, &pSampleToDeliver, rtStart, rtStop));

	m_DXVAPicParams.CurrPic.Index7Bits			= nSurfaceIndex;

	CHECK_HR (BeginFrame(nSurfaceIndex, pSampleToDeliver));
	
	m_DXVAPicParams.StatusReportFeedbackNumber++;

//	TRACE("CDXVADecoderH264 : Decode frame %u\n", m_DXVAPicParams.StatusReportFeedbackNumber);

	// Send picture parameters
	CHECK_HR (AddExecuteBuffer (DXVA2_PictureParametersBufferType, sizeof(m_DXVAPicParams), &m_DXVAPicParams));
	CHECK_HR (Execute());

	// Add bitstream, slice control and quantization matrix
	CHECK_HR (AddExecuteBuffer (DXVA2_BitStreamDateBufferType, nSize, pDataIn, &nSize));

	switch (m_pFilter->GetPCIVendor())
	{
	case 4098 :
		// The ATI way, only one DXVA_Slice_H264_Short structure pointing to the whole buffer
		m_pSliceShort[0].BSNALunitDataLocation = 0;
		m_pSliceShort[0].SliceBytesInBuffer = nSize;
		CHECK_HR (AddExecuteBuffer (DXVA2_SliceControlBufferType, sizeof (DXVA_Slice_H264_Short), m_pSliceShort));
		break;

	default :
		// The NVIDIA way (the compliant way ??), one DXVA_Slice_H264_Short structure for each slice
		if (m_bUseLongSlice)
		{
 			CHECK_HR(AddExecuteBuffer(DXVA2_SliceControlBufferType,  sizeof(DXVA_Slice_H264_Long)*nSlices, m_pSliceLong));
		}
		else
		{
			CHECK_HR (AddExecuteBuffer (DXVA2_SliceControlBufferType, sizeof (DXVA_Slice_H264_Short)*nSlices, m_pSliceShort));
		}
		break;
	}

	CHECK_HR (AddExecuteBuffer (DXVA2_InverseQuantizationMatrixBufferType, sizeof (DXVA_Qmatrix_H264), (void*)&m_DXVAScalingMatrix));

	// Decode bitstream
	CHECK_HR (Execute());

	CHECK_HR (EndFrame(nSurfaceIndex));

#ifdef _DEBUG
//	DisplayStatus();
#endif

	bool bAdded		= AddToStore (nSurfaceIndex, pSampleToDeliver, Nalu.IsRefFrame(), rtStart, rtStop,
								  m_DXVAPicParams.field_pic_flag, (FF_FIELD_TYPE)nFieldType, 
								  (FF_SLICE_TYPE)nSliceType, nFramePOC);

	if (bAdded) hr = DisplayNextFrame();

	if ((Nalu.GetType() == NALU_TYPE_IDR) && (!bAdded || !m_DXVAPicParams.field_pic_flag))
		ClearRefFramesList();

	pDXVA2Sample = pSampleToDeliver; 
	if (pDXVA2Sample) nDXIndex = pDXVA2Sample->GetDXSurfaceId(); 
	
	UpdateRefFramesList (m_DXVAPicParams.frame_num, Nalu.IsRefFrame(), bAdded, nDXIndex);

	m_bFlushed = false;
	return hr;
}


void CDXVADecoderH264::SetExtraData (BYTE* pDataIn, UINT nSize)
{
	AVCodecContext*		pAVCtx = m_pFilter->GetAVCtx();
	m_nNALLength	= pAVCtx->nal_length_size;
	FFH264DecodeBuffer (pAVCtx, pDataIn, nSize, FALSE, NULL, NULL, NULL);
	FFH264SetDxvaSliceLong (pAVCtx, m_pSliceLong);
}


void CDXVADecoderH264::ClearRefFramesList()
{
	int		i;

	for (i=0; i<m_DXVAPicParams.num_ref_frames; i++)
	{
		if (m_DXVAPicParams.RefFrameList[i].bPicEntry != 255)
			RemoveRefFrame (m_DXVAPicParams.RefFrameList[i].Index7Bits);

		m_DXVAPicParams.RefFrameList[i].AssociatedFlag	= 1;
		m_DXVAPicParams.RefFrameList[i].bPicEntry		= 255;
		m_DXVAPicParams.RefFrameList[i].Index7Bits		= 127;
		
		m_DXVAPicParams.FieldOrderCntList[i][0]			= 0;
		m_DXVAPicParams.FieldOrderCntList[i][1]			= 0;

		m_DXVAPicParams.FrameNumList[i]					= 0;
	}

	m_nCurRefFrame = 0;
}


void CDXVADecoderH264::UpdateRefFramesList (int nFrameNum, bool bRefFrame, bool bAdded, int nDXIndex)
{
	int			i;

	if (bRefFrame)
	{
		// Shift buffers if needed
		if (!m_DXVAPicParams.RefFrameList[m_nCurRefFrame].AssociatedFlag && (!bAdded || !m_DXVAPicParams.field_pic_flag))
		{
			if (m_DXVAPicParams.RefFrameList[0].bPicEntry != 255)
				RemoveRefFrame (m_DXVAPicParams.RefFrameList[0].Index7Bits);

			for (i=1; i<m_DXVAPicParams.num_ref_frames; i++)
			{
				m_DXVAPicParams.FrameNumList[i-1] = m_DXVAPicParams.FrameNumList[i];
				memcpy (&m_DXVAPicParams.RefFrameList[i-1], &m_DXVAPicParams.RefFrameList[i], sizeof (DXVA_PicEntry_H264));

				m_DXVAPicParams.FieldOrderCntList[i-1][0] = m_DXVAPicParams.FieldOrderCntList[i][0];
				m_DXVAPicParams.FieldOrderCntList[i-1][1] = m_DXVAPicParams.FieldOrderCntList[i][1];
			}
			m_DXVAPicParams.FieldOrderCntList[m_nCurRefFrame][0] = 0;
			m_DXVAPicParams.FieldOrderCntList[m_nCurRefFrame][1] = 0;
		}

		m_DXVAPicParams.FrameNumList[m_nCurRefFrame] = nFrameNum;

		// Update current frame parameters
		m_DXVAPicParams.RefFrameList[m_nCurRefFrame].AssociatedFlag	= 0;
//es
		if(m_pFilter->GetPCIVendor()== 0x8086)
		{ 
			TRACE("CurrPic Index7Bits:  %u  nDXIndex: %u\n",m_DXVAPicParams.CurrPic.Index7Bits, nDXIndex); 
			m_DXVAPicParams.RefFrameList[m_nCurRefFrame].Index7Bits = nDXIndex; 
		}
		else
		{
			m_DXVAPicParams.RefFrameList[m_nCurRefFrame].Index7Bits		= m_DXVAPicParams.CurrPic.Index7Bits;
		}

		if (m_DXVAPicParams.CurrFieldOrderCnt[0])
			m_DXVAPicParams.FieldOrderCntList[m_nCurRefFrame][0]		= m_DXVAPicParams.CurrFieldOrderCnt[0];

		if (m_DXVAPicParams.CurrFieldOrderCnt[1])
			m_DXVAPicParams.FieldOrderCntList[m_nCurRefFrame][1]		= m_DXVAPicParams.CurrFieldOrderCnt[1];


		if (m_DXVAPicParams.field_pic_flag)
			m_DXVAPicParams.UsedForReferenceFlags	= g_UsedForReferenceFlags [m_nCurRefFrame*2+(bAdded?1:0)];
		else
			m_DXVAPicParams.UsedForReferenceFlags	= g_UsedForReferenceFlags [m_nCurRefFrame*2+1];

		if (bAdded)
			m_nCurRefFrame = min (m_nCurRefFrame+1, (UINT)(m_DXVAPicParams.num_ref_frames-1));
	}
}


HRESULT CDXVADecoderH264::DisplayStatus()
{
	HRESULT 			hr = E_INVALIDARG;
	DXVA_Status_H264 	Status;

	memset (&Status, 0, sizeof(Status));

	CHECK_HR (hr = CDXVADecoder::QueryStatus(&Status, sizeof(Status)));

	TRACE ("CDXVADecoderH264 : Status for the frame %u : bBufType = %u, bStatus = %u, wNumMbsAffected = %u\n", 
		Status.StatusReportFeedbackNumber,
		Status.bBufType,
		Status.bStatus,
		Status.wNumMbsAffected);

	return hr;
}


int CDXVADecoderH264::FindOldestFrame()
{
	for (int i=0; i<m_nPicEntryNumber; i++)
	{
		if (m_pPictureStore[i].bInUse && !m_pPictureStore[i].bDisplayed)
		{
			if (m_pPictureStore[i].nCodecSpecific == m_nOutPOC)
			{
				m_pPictureStore[i].rtStart	= m_rtOutStart;
				m_pPictureStore[i].rtStop	= m_rtOutStart + m_pFilter->GetAvrTimePerFrame();
				m_pFilter->ReorderBFrames (m_pPictureStore[i].rtStart, m_pPictureStore[i].rtStop);
				return i;
			}
		}
	}

	return -1;
}
