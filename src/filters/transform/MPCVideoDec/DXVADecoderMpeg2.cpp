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
#include "DXVADecoderMpeg2.h"
#include "MPCVideoDecFilter.h"

extern "C"
{
#include "FfmpegContext.h"
}

#if 0
	#define TRACE_MPEG2	TRACE
#else
	#define TRACE_MPEG2(...)
#endif

CDXVADecoderMpeg2::CDXVADecoderMpeg2 (CMPCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, DXVAMode nMode, int nPicEntryNumber)
	: CDXVADecoder (pFilter, pAMVideoAccelerator, nMode, nPicEntryNumber)
{
	Init();
}

CDXVADecoderMpeg2::CDXVADecoderMpeg2 (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber, DXVA2_ConfigPictureDecode* pDXVA2Config)
	: CDXVADecoder (pFilter, pDirectXVideoDec, nMode, nPicEntryNumber, pDXVA2Config)
{
	Init();
}

CDXVADecoderMpeg2::~CDXVADecoderMpeg2(void)
{
	Flush();
}

void CDXVADecoderMpeg2::Init()
{
	memset (&m_PictureParams,	0, sizeof(m_PictureParams));
	memset (&m_SliceInfo,		0, sizeof(m_SliceInfo));
	memset (&m_QMatrixData,		0, sizeof(m_QMatrixData));

	m_nMaxWaiting			= 5;
	m_wRefPictureIndex[0]	= NO_REF_FRAME;
	m_wRefPictureIndex[1]	= NO_REF_FRAME;
	m_nSliceCount			= 0;

	switch (GetMode()) {
		case MPEG2_VLD :
			AllocExecuteParams (4);
			break;
		default :
			ASSERT(FALSE);
	}
}

// === Public functions
HRESULT CDXVADecoderMpeg2::DecodeFrame (BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{
	HRESULT						hr;
	int							nSurfaceIndex;
	CComPtr<IMediaSample>		pSampleToDeliver;
	int							nFieldType;
	int							nSliceType;

	FFMpeg2DecodeFrame (&m_PictureParams, &m_QMatrixData, m_SliceInfo, &m_nSliceCount, m_pFilter->GetAVCtx(),
						m_pFilter->GetFrame(), &m_nNextCodecIndex, &nFieldType, &nSliceType, pDataIn, nSize);

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

	UpdatePictureParams(nSurfaceIndex);

	TRACE_MPEG2 ("CDXVADecoderMpeg2::DecodeFrame() : PictureType = %d, rtStart = %I64d, Surf = %d\n", nSliceType, rtStart, nSurfaceIndex);

	TRACE_MPEG2 ("CDXVADecoderMpeg2::DecodeFrame() : Decode frame %i\n", m_PictureParams.bPicScanMethod);

	CHECK_HR (AddExecuteBuffer (DXVA2_PictureParametersBufferType, sizeof(m_PictureParams), &m_PictureParams));

	CHECK_HR (AddExecuteBuffer (DXVA2_InverseQuantizationMatrixBufferType, sizeof(m_QMatrixData), &m_QMatrixData));

	// Send bitstream to accelerator
	CHECK_HR (AddExecuteBuffer (DXVA2_SliceControlBufferType, sizeof (DXVA_SliceInfo)*m_nSliceCount, &m_SliceInfo));
	CHECK_HR (AddExecuteBuffer (DXVA2_BitStreamDateBufferType, nSize, pDataIn, &nSize));

	// Decode frame
	CHECK_HR (Execute());
	CHECK_HR (EndFrame(nSurfaceIndex));

	AddToStore (nSurfaceIndex, pSampleToDeliver, (m_PictureParams.bPicBackwardPrediction != 1), rtStart, rtStop,
				false,(FF_FIELD_TYPE)nFieldType, (FF_SLICE_TYPE)nSliceType, FFGetCodedPicture(m_pFilter->GetAVCtx()));
	m_bFlushed = false;

	return DisplayNextFrame();
}

void CDXVADecoderMpeg2::UpdatePictureParams(int nSurfaceIndex)
{
	DXVA2_ConfigPictureDecode*	cpd = GetDXVA2Config();		// Ok for DXVA1 too (parameters have been copied)

	m_PictureParams.wDecodedPictureIndex	= nSurfaceIndex;

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

	// Shall be 0 if bConfigResidDiffHost is 0 or if BPP > 8
	if (cpd->ConfigResidDiffHost == 0 || m_PictureParams.bBPPminus1 > 7) {
		m_PictureParams.bPicSpatialResid8 = 0;
	} else {
		if (m_PictureParams.bBPPminus1 == 7 && m_PictureParams.bPicIntra && cpd->ConfigResidDiffHost)
			// Shall be 1 if BPP is 8 and bPicIntra is 1 and bConfigResidDiffHost is 1
		{
			m_PictureParams.bPicSpatialResid8 = 1;
		} else
			// Shall be 1 if bConfigSpatialResid8 is 1
		{
			m_PictureParams.bPicSpatialResid8 = cpd->ConfigSpatialResid8;
		}
	}

	// Shall be 0 if bConfigResidDiffHost is 0 or if bConfigSpatialResid8 is 0 or if BPP > 8
	if (cpd->ConfigResidDiffHost == 0 || cpd->ConfigSpatialResid8 == 0 || m_PictureParams.bBPPminus1 > 7) {
		m_PictureParams.bPicOverflowBlocks = 0;
	}

	// Shall be 1 if bConfigHostInverseScan is 1 or if bConfigResidDiffAccelerator is 0.

	if (cpd->ConfigHostInverseScan == 1 || cpd->ConfigResidDiffAccelerator == 0) {
		m_PictureParams.bPicScanFixed	= 1;

		if (cpd->ConfigHostInverseScan != 0) {
			m_PictureParams.bPicScanMethod	= 3;    // 11 = Arbitrary scan with absolute coefficient address.
		} else if (FFGetAlternateScan(m_pFilter->GetAVCtx())) {
			m_PictureParams.bPicScanMethod	= 1;    // 00 = Zig-zag scan (MPEG-2 Figure 7-2)
		} else {
			m_PictureParams.bPicScanMethod	= 0;    // 01 = Alternate-vertical (MPEG-2 Figure 7-3),
		}
	}
}

void CDXVADecoderMpeg2::SetExtraData (BYTE* pDataIn, UINT nSize)
{
	m_PictureParams.wPicWidthInMBminus1				= m_pFilter->PictWidth()  - 1;
	m_PictureParams.wPicHeightInMBminus1			= m_pFilter->PictHeight() - 1;
}

void CDXVADecoderMpeg2::CopyBitstream(BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize)
{
	while (*((DWORD*)pBuffer) != 0x01010000) {
		pBuffer++;
		nSize--;

		if (nSize <= 0) {
			return;
		}
	}

	memcpy (pDXVABuffer, pBuffer, nSize);
}

void CDXVADecoderMpeg2::Flush()
{
	m_nNextCodecIndex		= INT_MIN;

	if (m_wRefPictureIndex[0] != NO_REF_FRAME) {
		RemoveRefFrame (m_wRefPictureIndex[0]);
	}
	if (m_wRefPictureIndex[1] != NO_REF_FRAME) {
		RemoveRefFrame (m_wRefPictureIndex[1]);
	}

	m_wRefPictureIndex[0] = NO_REF_FRAME;
	m_wRefPictureIndex[1] = NO_REF_FRAME;

	m_rtLastStart = 0;

	__super::Flush();
}

int CDXVADecoderMpeg2::FindOldestFrame()
{
	int					nPos	= -1;

	for (int i=0; i<m_nPicEntryNumber; i++) {
		if (!m_pPictureStore[i].bDisplayed &&
				m_pPictureStore[i].bInUse &&
				(m_pPictureStore[i].nCodecSpecific == m_nNextCodecIndex)) {
			m_nNextCodecIndex	= INT_MIN;
			nPos	= i;
		}
	}

	if (nPos != -1) {
		UpdateFrameTime(m_pPictureStore[nPos].rtStart, m_pPictureStore[nPos].rtStop);
	}

	return nPos;
}

void CDXVADecoderMpeg2::UpdateFrameTime (REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
	if (m_rtLastStart && (rtStart == _I64_MIN || (rtStart < m_rtLastStart))) {
		rtStart = m_rtLastStart;
	}
	rtStop  = rtStart + m_pFilter->GetAvrTimePerFrame() / m_pFilter->GetRate();

	m_rtLastStart = rtStop;
}
