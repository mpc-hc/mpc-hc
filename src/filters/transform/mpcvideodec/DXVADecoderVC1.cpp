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
#include "DXVADecoderVC1.h"
#include "MPCVideoDecFilter.h"

extern "C"
{
	#include "FfmpegContext.h"
}

#define VC1_NO_REF			0xFFFF

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


CDXVADecoderVC1::CDXVADecoderVC1 (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber)
			   : CDXVADecoder (pFilter, pDirectXVideoDec, nMode, nPicEntryNumber)
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

	m_wRefPictureIndex[0] = VC1_NO_REF;
	m_wRefPictureIndex[1] = VC1_NO_REF;

	switch (GetMode())
	{
	case VC1_VLD :
		AllocExecuteParams (3);
		break;
	default :
		ASSERT(FALSE);
	}
}

// === Public functions
HRESULT CDXVADecoderVC1::DecodeFrame (BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{
	HRESULT						hr;
	int							nPictType;
	int							nSurfaceIndex;
	CComPtr<IMediaSample>		pSampleToDeliver;

	nPictType = FFVC1UpdatePictureParam (&m_PictureParams, m_pFilter->GetAVCtx(), pDataIn, nSize);

	// Wait I frame after a flush
	if (m_bFlushed && ! m_PictureParams.bPicIntra)
		return S_FALSE;

	hr = GetFreeSurfaceIndex (nSurfaceIndex, &pSampleToDeliver, rtStart, rtStop);
	if (FAILED (hr))
	{
		ASSERT (hr == VFW_E_NOT_COMMITTED);		// Normal when stop playing
		return hr;
	}

	CHECK_HR (BeginFrame(nSurfaceIndex, pSampleToDeliver));

	TRACE ("=> %s   %I64d  Surf=%d\n", GetFFMpegPictureType(nPictType), rtStart, nSurfaceIndex);

	m_PictureParams.wDecodedPictureIndex			= nSurfaceIndex;
	m_PictureParams.wDeblockedPictureIndex			= m_PictureParams.wDecodedPictureIndex;

	// Manage reference picture list
	if (!m_PictureParams.bPicBackwardPrediction)
	{
		if (m_wRefPictureIndex[0] != VC1_NO_REF) RemoveRefFrame (m_wRefPictureIndex[0]);
		m_wRefPictureIndex[0] = m_wRefPictureIndex[1];
		m_wRefPictureIndex[1] = nSurfaceIndex;
	}
	m_PictureParams.wForwardRefPictureIndex		= (m_PictureParams.bPicIntra == 0)				? m_wRefPictureIndex[0] : VC1_NO_REF;
	m_PictureParams.wBackwardRefPictureIndex	= (m_PictureParams.bPicBackwardPrediction == 1) ? m_wRefPictureIndex[1] : VC1_NO_REF;
	m_PictureParams.bPicScanMethod++;					// Use for status reporting sections 3.8.1 and 3.8.2

	// Send picture params to accelerator
	m_PictureParams.wDecodedPictureIndex	= nSurfaceIndex;
	CHECK_HR (AddExecuteBuffer (DXVA2_PictureParametersBufferType, sizeof(m_PictureParams), &m_PictureParams));
	CHECK_HR (Execute());


	// Send bitstream to accelerator
	m_SliceInfo.wQuantizerScaleCode	= 1;		// TODO : 1->31 ???
	CHECK_HR (AddExecuteBuffer (DXVA2_BitStreamDateBufferType, nSize, pDataIn, &nSize));
	m_SliceInfo.dwSliceBitsInBuffer	= nSize * 8;
	CHECK_HR (AddExecuteBuffer (DXVA2_SliceControlBufferType, sizeof (m_SliceInfo), &m_SliceInfo));

	// Decode frame
	CHECK_HR (Execute());
	CHECK_HR (EndFrame(nSurfaceIndex));

	// Re-order B frames
	if (m_PictureParams.bPicBackwardPrediction == 1)
	{
		SwapRT (rtStart, m_rtStartDelayed);
		SwapRT (rtStop,  m_rtStopDelayed);
	}
	else
	{
		// Save I or P reference time (swap later)
		if (!m_bFlushed)
		{
			if (m_nDelayedSurfaceIndex != -1)
				UpdateStore (m_nDelayedSurfaceIndex, m_rtStartDelayed, m_rtStopDelayed);
			m_rtStartDelayed = m_rtStopDelayed = _I64_MAX;
			SwapRT (rtStart, m_rtStartDelayed);
			SwapRT (rtStop,  m_rtStopDelayed);
			m_nDelayedSurfaceIndex	= nSurfaceIndex;
		}
	}

	AddToStore (nSurfaceIndex, pSampleToDeliver, (m_PictureParams.bPicBackwardPrediction != 1), rtStart, rtStop);
	m_bFlushed = false;

	return DisplayNextFrame();
}

void CDXVADecoderVC1::SetExtraData (BYTE* pDataIn, UINT nSize)
{
	m_PictureParams.wPicWidthInMBminus1				= m_pFilter->PictWidth()  - 1;
	m_PictureParams.wPicHeightInMBminus1			= m_pFilter->PictHeight() - 1;
	m_PictureParams.bMacroblockWidthMinus1			= 15;
	m_PictureParams.bMacroblockHeightMinus1			= 15;
	m_PictureParams.bBlockWidthMinus1				= 7;
	m_PictureParams.bBlockHeightMinus1				= 7;
	m_PictureParams.bBPPminus1						= 7;

	m_PictureParams.bMVprecisionAndChromaRelation	= 0;
	m_PictureParams.bChromaFormat					= VC1_CHROMA_420;

	m_PictureParams.bPicScanFixed					= 0;	// Use for status reporting sections 3.8.1 and 3.8.2
	m_PictureParams.bPicReadbackRequests			= 0;

	m_PictureParams.bRcontrol						= 0;
	m_PictureParams.bPicExtrapolation				= 0;

	m_PictureParams.bPicDeblocked					= 2;	// TODO ???
	m_PictureParams.bPic4MVallowed					= 0;	// TODO
	m_PictureParams.bPicOBMC						= 0;
	m_PictureParams.bPicBinPB						= 0;	// TODO
	m_PictureParams.bMV_RPS							= 0;	// TODO

	m_PictureParams.bReservedBits					= 0;
	m_PictureParams.wBitstreamFcodes				= 32;	// TODO

}


BYTE* CDXVADecoderVC1::FindNextStartCode(BYTE* pBuffer, UINT nSize, UINT& nPacketSize)
{
	BYTE*		pStart	= pBuffer;
	BYTE		bCode	= 0;
	for (int i=0; i<nSize-4; i++)
	{
		if ( ((*((DWORD*)(pBuffer+i)) & 0x00FFFFFF) == 0x00010000) || (i >= nSize-5) )
		{
			if (bCode == 0) 
				bCode = pBuffer[i+3];
			else
			{
				if (bCode == 0x0D)
				{
					// Start code found!
					nPacketSize = i - (pStart - pBuffer) + (i >= nSize-5 ? 5 : 1);
					return pStart;
				}
				else
				{
					// Other stuff, ignore it
					pStart = pBuffer + i;
					bCode  = pBuffer[i+3];
				}
			}
		}
	}

	return NULL;
}

void CDXVADecoderVC1::CopyBitstream(BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize)
{
	int		nDummy;

	if ( (*((DWORD*)pBuffer) & 0x00FFFFFF) != 0x00010000)
	{
		// Some splitter have remove startcode (Haali)
		pDXVABuffer[0]=pDXVABuffer[1]=0; pDXVABuffer[2]=1; pDXVABuffer[3]=0x0D;
		pDXVABuffer	+=4;
		// Copy bitstream buffer, with zero padding (buffer is rounded to multiple of 128)
		memcpy (pDXVABuffer, (BYTE*)pBuffer, nSize);
		nSize  +=4;
	}
	else
	{
		BYTE*	pStart;
		UINT	nPacketSize;

		pStart = FindNextStartCode (pBuffer, nSize, nPacketSize);
		if (pStart)
		{
			// Startcode already present
			memcpy (pDXVABuffer, (BYTE*)pStart, nPacketSize);
			nSize = nPacketSize;
		}
	}
	
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

	if (m_wRefPictureIndex[0] != VC1_NO_REF) RemoveRefFrame (m_wRefPictureIndex[0]);
	if (m_wRefPictureIndex[1] != VC1_NO_REF) RemoveRefFrame (m_wRefPictureIndex[1]);

	m_wRefPictureIndex[0] = VC1_NO_REF;
	m_wRefPictureIndex[1] = VC1_NO_REF;

	__super::Flush();
}
