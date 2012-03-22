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

#include "stdafx.h"
#include "DXVADecoderVC1.h"
#include "MPCVideoDecFilter.h"
#include "avcodec.h"

extern "C"
{
#include "FfmpegContext.h"
}

#if 0
	#define TRACE_VC1	TRACE
#else
	#define TRACE_VC1(...)
#endif

inline void SwapRT(REFERENCE_TIME& rtFirst, REFERENCE_TIME& rtSecond)
{
	REFERENCE_TIME	rtTemp = rtFirst;
	rtFirst		= rtSecond;
	rtSecond	= rtTemp;
}

CDXVADecoderVC1::CDXVADecoderVC1 (CMPCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, DXVAMode nMode, int nPicEntryNumber)
	: CDXVADecoder (pFilter, pAMVideoAccelerator, nMode, nPicEntryNumber)
{
	Init();
}

CDXVADecoderVC1::CDXVADecoderVC1 (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber, DXVA2_ConfigPictureDecode* pDXVA2Config)
	: CDXVADecoder (pFilter, pDirectXVideoDec, nMode, nPicEntryNumber, pDXVA2Config)
{
	Init();
}

CDXVADecoderVC1::~CDXVADecoderVC1(void)
{
	Flush();
}

void CDXVADecoderVC1::Init()
{
	memset (&m_PictureParams, 0, sizeof(m_PictureParams));
	memset (&m_SliceInfo,     0, sizeof(m_SliceInfo));

	m_nMaxWaiting		  = 5;
	m_wRefPictureIndex[0] = NO_REF_FRAME;
	m_wRefPictureIndex[1] = NO_REF_FRAME;

	switch (GetMode()) {
		case VC1_VLD :
			AllocExecuteParams (3);
			break;
		default :
			ASSERT(FALSE);
	}

	m_bFrame_repeat_pict = FALSE;
}

// === Public functions
HRESULT CDXVADecoderVC1::DecodeFrame (BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{
	HRESULT						hr;
	int							nSurfaceIndex;
	CComPtr<IMediaSample>		pSampleToDeliver;
	int							nFieldType, nSliceType;
	UINT						nFrameSize, nSize_Result;

	FFVC1UpdatePictureParam (&m_PictureParams, m_pFilter->GetAVCtx(), &nFieldType, &nSliceType, pDataIn, nSize, &nFrameSize, FALSE, &m_bFrame_repeat_pict);

	if (FFIsSkipped (m_pFilter->GetAVCtx())) {
		return S_OK;
	}

	// Wait I frame after a flush
	if (m_bFlushed && ! m_PictureParams.bPicIntra) {
		return S_FALSE;
	}

	hr = GetFreeSurfaceIndex (nSurfaceIndex, &pSampleToDeliver, rtStart, rtStop);
	if (FAILED (hr)) {
		ASSERT (hr == VFW_E_NOT_COMMITTED);		// Normal when stop playing
		return hr;
	}

	CHECK_HR (BeginFrame(nSurfaceIndex, pSampleToDeliver));

	TRACE_VC1 ("CDXVADecoderVC1::DecodeFrame() : PictureType = %d, rtStart = %I64d, Surf = %d\n", nSliceType, rtStart, nSurfaceIndex);

	m_PictureParams.wDecodedPictureIndex	= nSurfaceIndex;
	m_PictureParams.wDeblockedPictureIndex	= m_PictureParams.wDecodedPictureIndex;

	// Manage reference picture list
	if (!m_PictureParams.bPicBackwardPrediction) {
		if (m_wRefPictureIndex[0] != NO_REF_FRAME) {
			RemoveRefFrame (m_wRefPictureIndex[0]);
		}
		m_wRefPictureIndex[0] = m_wRefPictureIndex[1];
		m_wRefPictureIndex[1] = nSurfaceIndex;
	}
	m_PictureParams.wForwardRefPictureIndex		= (m_PictureParams.bPicIntra == 0)				? m_wRefPictureIndex[0] : NO_REF_FRAME;
	m_PictureParams.wBackwardRefPictureIndex	= (m_PictureParams.bPicBackwardPrediction == 1) ? m_wRefPictureIndex[1] : NO_REF_FRAME;

	m_PictureParams.bPic4MVallowed				= (m_PictureParams.wBackwardRefPictureIndex == NO_REF_FRAME && m_PictureParams.bPicStructure == 3) ? 1 : 0;
	m_PictureParams.bPicDeblockConfined			|= (m_PictureParams.wBackwardRefPictureIndex == NO_REF_FRAME) ? 0x04 : 0;

	m_PictureParams.bPicScanMethod++;					// Use for status reporting sections 3.8.1 and 3.8.2

	TRACE_VC1 ("CDXVADecoderVC1::DecodeFrame() : Decode frame %i\n", m_PictureParams.bPicScanMethod);

	// Send picture params to accelerator
	CHECK_HR (AddExecuteBuffer (DXVA2_PictureParametersBufferType, sizeof(m_PictureParams), &m_PictureParams));

	// Send bitstream to accelerator
	CHECK_HR (AddExecuteBuffer (DXVA2_BitStreamDateBufferType, nFrameSize ? nFrameSize : nSize, pDataIn, &nSize_Result));

	m_SliceInfo.wQuantizerScaleCode	= 1;		// TODO : 1->31 ???
	m_SliceInfo.dwSliceBitsInBuffer	= nSize_Result * 8;
	CHECK_HR (AddExecuteBuffer (DXVA2_SliceControlBufferType, sizeof (m_SliceInfo), &m_SliceInfo));

	// Decode frame
	CHECK_HR (Execute());
	CHECK_HR (EndFrame(nSurfaceIndex));

	// ***************
	if (nFrameSize) { // Decoding Second Field
		FFVC1UpdatePictureParam (&m_PictureParams, m_pFilter->GetAVCtx(), NULL, NULL, pDataIn, nSize, NULL, TRUE, &m_bFrame_repeat_pict);

		CHECK_HR (BeginFrame(nSurfaceIndex, pSampleToDeliver));

		TRACE_VC1 ("CDXVADecoderVC1::DecodeFrame() : PictureType = %d\n", nSliceType);

		CHECK_HR (AddExecuteBuffer (DXVA2_PictureParametersBufferType, sizeof(m_PictureParams), &m_PictureParams));

		// Send bitstream to accelerator
		CHECK_HR (AddExecuteBuffer (DXVA2_BitStreamDateBufferType, nSize - nFrameSize, pDataIn + nFrameSize, &nSize_Result));

		m_SliceInfo.wQuantizerScaleCode	= 1;		// TODO : 1->31 ???
		m_SliceInfo.dwSliceBitsInBuffer	= nSize_Result * 8;
		CHECK_HR (AddExecuteBuffer (DXVA2_SliceControlBufferType, sizeof (m_SliceInfo), &m_SliceInfo));

		// Decode frame
		CHECK_HR (Execute());
		CHECK_HR (EndFrame(nSurfaceIndex));
	}
	// ***************

#ifdef _DEBUG
	DisplayStatus();
#endif

	// Update timestamp & Re-order B frames
	if (m_bFrame_repeat_pict || m_pFilter->IsReorderBFrame()) {
		if (m_bFrame_repeat_pict) {
			m_pFilter->UpdateFrameTime(rtStart, rtStop, !!m_bFrame_repeat_pict);
		}
		if (m_pFilter->IsReorderBFrame() || m_pFilter->IsEvo()) {
			if (m_PictureParams.bPicBackwardPrediction == 1) {
				SwapRT (rtStart, m_rtStartDelayed);
				SwapRT (rtStop,  m_rtStopDelayed);
			} else {
				// Save I or P reference time (swap later)
				if (!m_bFlushed) {
					if (m_nDelayedSurfaceIndex != -1) {
						UpdateStore (m_nDelayedSurfaceIndex, m_rtStartDelayed, m_rtStopDelayed);
					}
					m_rtStartDelayed = m_rtStopDelayed = _I64_MAX;
					SwapRT (rtStart, m_rtStartDelayed);
					SwapRT (rtStop,  m_rtStopDelayed);
					m_nDelayedSurfaceIndex	= nSurfaceIndex;
				}
			}
		}
	}

	AddToStore (nSurfaceIndex, pSampleToDeliver, (m_PictureParams.bPicBackwardPrediction != 1), rtStart, rtStop,
				false,(FF_FIELD_TYPE)nFieldType, (FF_SLICE_TYPE)nSliceType, 0);
	m_bFlushed = false;

	return DisplayNextFrame();
}

void CDXVADecoderVC1::SetExtraData (BYTE* pDataIn, UINT nSize)
{
	m_PictureParams.bMacroblockWidthMinus1			= 15;
	m_PictureParams.bMacroblockHeightMinus1			= 15;
	m_PictureParams.bBlockWidthMinus1				= 7;
	m_PictureParams.bBlockHeightMinus1				= 7;
	m_PictureParams.bBPPminus1						= 7;

	m_PictureParams.bChromaFormat					= VC1_CHROMA_420;

	m_PictureParams.bPicScanFixed					= 0;	// Use for status reporting sections 3.8.1 and 3.8.2
	m_PictureParams.bPicReadbackRequests			= 0;

	m_PictureParams.bPicBinPB						= 0;	// TODO
	m_PictureParams.bMV_RPS							= 0;	// TODO

	m_PictureParams.bReservedBits					= 0;

	// iWMV9 - i9IRU - iOHIT - iINSO - iWMVA - 0 - 0 - 0		| Section 3.2.5
	m_PictureParams.bBidirectionalAveragingMode		= (1 << 7) |
						(GetConfigIntraResidUnsigned()   << 6) |	// i9IRU
						(GetConfigResidDiffAccelerator() << 5);		// iOHIT
}

BYTE* CDXVADecoderVC1::FindNextStartCode(BYTE* pBuffer, UINT nSize, UINT& nPacketSize)
{
	BYTE*		pStart	= pBuffer;
	BYTE		bCode	= 0;
	for (UINT i=0; i<nSize-4; i++) {
		if ( ((*((DWORD*)(pBuffer+i)) & 0x00FFFFFF) == 0x00010000) || (i >= nSize-5) ) {
			if (bCode == 0) {
				bCode = pBuffer[i+3];
				if ((nSize == 5) && (bCode == 0x0D)) {
					nPacketSize = nSize;
					return pBuffer;
				}
			} else {
				if (bCode == 0x0D) {
					// Start code found!
					nPacketSize = i - (pStart - pBuffer) + (i >= nSize-5 ? 5 : 1);
					return pStart;
				} else {
					// Other stuff, ignore it
					pStart = pBuffer + i;
					bCode  = pBuffer[i+3];
				}
			}
		}
	}

	ASSERT (FALSE);		// Should never happen!

	return NULL;
}

void CDXVADecoderVC1::CopyBitstream(BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize)
{
	int		nDummy;

	if (m_PictureParams.bSecondField) {
		memcpy (pDXVABuffer, (BYTE*)pBuffer, nSize);
	} else {
		if ( (*((DWORD*)pBuffer) & 0x00FFFFFF) != 0x00010000) {
			if (m_pFilter->GetCodec() == CODEC_ID_WMV3) {
				memcpy (pDXVABuffer, (BYTE*)pBuffer, nSize);
			} else {
				pDXVABuffer[0]=pDXVABuffer[1]=0;
				pDXVABuffer[2]=1;
				pDXVABuffer[3]=0x0D;
				pDXVABuffer	+=4;
				memcpy (pDXVABuffer, (BYTE*)pBuffer, nSize);
				nSize  +=4;
			}
		} else {
			BYTE*	pStart;
			UINT	nPacketSize;

			pStart = FindNextStartCode (pBuffer, nSize, nPacketSize);
			if (pStart) {
				// Startcode already present
				memcpy (pDXVABuffer, (BYTE*)pStart, nPacketSize);
				nSize = nPacketSize;
			}
		}
	}

	// Copy bitstream buffer, with zero padding (buffer is rounded to multiple of 128)
	nDummy  = 128 - (nSize %128);

	pDXVABuffer += nSize;
	memset (pDXVABuffer, 0, nDummy);
	nSize  += nDummy;
}

void CDXVADecoderVC1::Flush()
{
	m_nDelayedSurfaceIndex	= -1;
	m_rtStartDelayed		= _I64_MAX;
	m_rtStopDelayed			= _I64_MAX;

	if (m_wRefPictureIndex[0] != NO_REF_FRAME) {
		RemoveRefFrame (m_wRefPictureIndex[0]);
	}
	if (m_wRefPictureIndex[1] != NO_REF_FRAME) {
		RemoveRefFrame (m_wRefPictureIndex[1]);
	}

	m_wRefPictureIndex[0] = NO_REF_FRAME;
	m_wRefPictureIndex[1] = NO_REF_FRAME;

	__super::Flush();
}

HRESULT CDXVADecoderVC1::DisplayStatus()
{
	HRESULT			hr = E_INVALIDARG;
	DXVA_Status_VC1 Status;

	memset (&Status, 0, sizeof(Status));

	if (SUCCEEDED (hr = CDXVADecoder::QueryStatus(&Status, sizeof(Status)))) {
		Status.StatusReportFeedbackNumber = 0x00FF & Status.StatusReportFeedbackNumber;

		TRACE_VC1 ("CDXVADecoderVC1::DisplayStatus() : Status for the frame %u : bBufType = %u, bStatus = %u, wNumMbsAffected = %u\n",
				   Status.StatusReportFeedbackNumber,
				   Status.bBufType,
				   Status.bStatus,
				   Status.wNumMbsAffected);
	}

	return hr;
}
