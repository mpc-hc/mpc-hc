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
#include <dxva2api.h>
#include "DXVADecoderH264.h"
#include "DXVADecoderVC1.h"
#include "MPCVideoDecFilter.h"
#include "VideoDecDXVAAllocator.h"

#define MAX_RETRY_ON_PENDING		50
#define DO_DXVA_PENDING_LOOP(x)		nTry = 0; \
									while (FAILED(hr = x) && nTry<MAX_RETRY_ON_PENDING) \
									{ \
										if (hr != E_PENDING) break; \
										TRACE ("Pending loop %d\n", nTry); \
										Sleep(1); \
										nTry++; \
									}



CDXVADecoder::CDXVADecoder (CMPCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, DXVAMode nMode, int nPicEntryNumber)
{
	m_nEngine				= ENGINE_DXVA1;
	m_pAMVideoAccelerator	= pAMVideoAccelerator;
	m_dwBufferIndex			= 0;
	m_nMaxWaiting			= 3;

	Init (pFilter, nMode, nPicEntryNumber);
}


CDXVADecoder::CDXVADecoder (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber)	
{
	m_nEngine		= ENGINE_DXVA2;
	m_pDirectXVideoDec	= pDirectXVideoDec;

	Init (pFilter, nMode, nPicEntryNumber);
};

CDXVADecoder::~CDXVADecoder()
{
	SAFE_DELETE_ARRAY (m_pPictureStore);
	SAFE_DELETE_ARRAY (m_ExecuteParams.pCompressedBuffers);
}

void CDXVADecoder::Init(CMPCVideoDecFilter* pFilter, DXVAMode nMode, int nPicEntryNumber)
{
	m_pFilter			= pFilter;
	m_nMode				= nMode;
	m_nPicEntryNumber	= nPicEntryNumber;
	m_pPictureStore		= new PICTURE_STORE[nPicEntryNumber];
	m_dwNumBuffersInfo	= 0;

	memset (&m_DXVA1Config, 0, sizeof(m_DXVA1Config));
	memset (&m_DXVA1BufferDesc, 0, sizeof(m_DXVA1BufferDesc));
	m_DXVA1Config.guidConfigBitstreamEncryption	= DXVA_NoEncrypt;
	m_DXVA1Config.guidConfigMBcontrolEncryption	= DXVA_NoEncrypt;
	m_DXVA1Config.guidConfigResidDiffEncryption	= DXVA_NoEncrypt;
	m_DXVA1Config.bConfigBitstreamRaw			= 2;

	memset (&m_DXVA1BufferInfo, 0, sizeof(m_DXVA1BufferInfo));
	memset (&m_ExecuteParams, 0, sizeof(m_ExecuteParams));
	Flush();
}

// === Public functions
void CDXVADecoder::AllocExecuteParams (int nSize)
{
	m_ExecuteParams.pCompressedBuffers	= new DXVA2_DecodeBufferDesc[nSize];

	for (int i=0; i<nSize; i++)
		memset (&m_ExecuteParams.pCompressedBuffers[i], 0, sizeof(DXVA2_DecodeBufferDesc));
}

void CDXVADecoder::SetExtraData (BYTE* pDataIn, UINT nSize)
{
	// Extradata is codec dependant
	UNREFERENCED_PARAMETER (pDataIn);
	UNREFERENCED_PARAMETER (nSize);
}

void CDXVADecoder::CopyBitstream(BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize)
{
	memcpy (pDXVABuffer, (BYTE*)pBuffer, nSize);
}

void CDXVADecoder::Flush()
{
	TRACE ("CDXVADecoder::Flush\n");
	for (int i=0; i<m_nPicEntryNumber; i++)
	{
		m_pPictureStore[i].bRefPicture	= false;
		m_pPictureStore[i].bInUse		= false;
		m_pPictureStore[i].bDisplayed	= false;
		m_pPictureStore[i].pSample		= NULL;
	}

	m_nWaitingPics	= 0;
	m_bFlushed		= true;
	m_nFieldSurface = -1;
	m_pFieldSample	= NULL;
}

HRESULT CDXVADecoder::ConfigureDXVA1()
{
	HRESULT							hr = S_FALSE;
	DXVA_ConfigPictureDecode		ConfigRequested;

	if (m_pAMVideoAccelerator)
	{
		memset (&ConfigRequested, 0, sizeof(ConfigRequested));		
		ConfigRequested.guidConfigBitstreamEncryption	= DXVA_NoEncrypt;
		ConfigRequested.guidConfigMBcontrolEncryption	= DXVA_NoEncrypt;
		ConfigRequested.guidConfigResidDiffEncryption	= DXVA_NoEncrypt;
		ConfigRequested.bConfigBitstreamRaw				= 2;

		writeDXVA_QueryOrReplyFunc (&ConfigRequested.dwFunction, DXVA_QUERYORREPLYFUNCFLAG_DECODER_PROBE_QUERY, DXVA_PICTURE_DECODING_FUNCTION);
		hr = m_pAMVideoAccelerator->Execute (ConfigRequested.dwFunction, &ConfigRequested, sizeof(DXVA_ConfigPictureDecode), &m_DXVA1Config, sizeof(DXVA_ConfigPictureDecode), 0, NULL);

		if (SUCCEEDED (hr))
		{
			writeDXVA_QueryOrReplyFunc (&m_DXVA1Config.dwFunction, DXVA_QUERYORREPLYFUNCFLAG_DECODER_LOCK_QUERY, DXVA_PICTURE_DECODING_FUNCTION);
			hr = m_pAMVideoAccelerator->Execute (m_DXVA1Config.dwFunction, &m_DXVA1Config, sizeof(DXVA_ConfigPictureDecode), &ConfigRequested, sizeof(DXVA_ConfigPictureDecode), 0, NULL);

			// TODO : check config!
//			ASSERT (ConfigRequested.bConfigBitstreamRaw == 2);

			AMVAUncompDataInfo		DataInfo;
			DWORD					dwNum = COMP_BUFFER_COUNT;
			DataInfo.dwUncompWidth	= m_pFilter->PictWidth();
			DataInfo.dwUncompHeight	= m_pFilter->PictHeightRounded(); 
			memcpy (&DataInfo.ddUncompPixelFormat, m_pFilter->GetPixelFormat(), sizeof (DDPIXELFORMAT));
			hr = m_pAMVideoAccelerator->GetCompBufferInfo (m_pFilter->GetDXVADecoderGuid(), &DataInfo, &dwNum, m_ComBufferInfo);
		}
	}
	return hr;
}

CDXVADecoder* CDXVADecoder::CreateDecoder (CMPCVideoDecFilter* pFilter, IAMVideoAccelerator* pAMVideoAccelerator, const GUID* guidDecoder, int nPicEntryNumber)
{
	CDXVADecoder*		pDecoder = NULL;

	if ((*guidDecoder == DXVA2_ModeH264_E) || (*guidDecoder == DXVA2_ModeH264_F))
		pDecoder	= new CDXVADecoderH264 (pFilter, pAMVideoAccelerator, H264_VLD, nPicEntryNumber);
	else if (*guidDecoder == DXVA2_ModeVC1_D)
		pDecoder	= new CDXVADecoderVC1 (pFilter, pAMVideoAccelerator, VC1_VLD, nPicEntryNumber);
	else
		ASSERT (FALSE);	// Unknown decoder !!

	return pDecoder;
}


CDXVADecoder* CDXVADecoder::CreateDecoder (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, const GUID* guidDecoder, int nPicEntryNumber)
{
	CDXVADecoder*		pDecoder = NULL;

	if ((*guidDecoder == DXVA2_ModeH264_E) || (*guidDecoder == DXVA2_ModeH264_F))
		pDecoder	= new CDXVADecoderH264 (pFilter, pDirectXVideoDec, H264_VLD, nPicEntryNumber);
	else if (*guidDecoder == DXVA2_ModeVC1_D)
		pDecoder	= new CDXVADecoderVC1 (pFilter, pDirectXVideoDec, VC1_VLD, nPicEntryNumber);
	else
		ASSERT (FALSE);	// Unknown decoder !!

	return pDecoder;
}

// === DXVA functions

HRESULT CDXVADecoder::AddExecuteBuffer (DWORD CompressedBufferType, UINT nSize, void* pBuffer, UINT* pRealSize)
{
	HRESULT						hr = E_INVALIDARG;
	BYTE*						pDXVABuffer;

	switch (m_nEngine)
	{
	case ENGINE_DXVA1 :
		DWORD		dwTypeIndex;
		LONG		lStride;
		dwTypeIndex = GetDXVA1CompressedType (CompressedBufferType);

//		TRACE ("Fill : %d - %d\n", dwTypeIndex, m_dwBufferIndex);
		hr = m_pAMVideoAccelerator->GetBuffer(dwTypeIndex, m_dwBufferIndex, FALSE, (void**)&pDXVABuffer, &lStride);
		ASSERT (SUCCEEDED (hr));

		if (SUCCEEDED (hr))
		{
			if (CompressedBufferType == DXVA2_BitStreamDateBufferType)
				CopyBitstream (pDXVABuffer, (BYTE*)pBuffer, nSize);
			else
				memcpy (pDXVABuffer, (BYTE*)pBuffer, nSize);
			m_DXVA1BufferInfo[m_dwNumBuffersInfo].dwTypeIndex		= dwTypeIndex;
			m_DXVA1BufferInfo[m_dwNumBuffersInfo].dwBufferIndex		= m_dwBufferIndex;
			m_DXVA1BufferInfo[m_dwNumBuffersInfo].dwDataSize		= nSize;

			m_DXVA1BufferDesc[m_dwNumBuffersInfo].dwTypeIndex		= dwTypeIndex;
			m_DXVA1BufferDesc[m_dwNumBuffersInfo].dwBufferIndex		= m_dwBufferIndex;
			m_DXVA1BufferDesc[m_dwNumBuffersInfo].dwDataSize		= nSize;
			m_DXVA1BufferDesc[m_dwNumBuffersInfo].dwNumMBsInBuffer	= (CompressedBufferType == DXVA2_SliceControlBufferType) || (CompressedBufferType == DXVA2_BitStreamDateBufferType);

			m_dwNumBuffersInfo++;
		}
		break;

	case ENGINE_DXVA2 :
		UINT						nDXVASize;
		hr = m_pDirectXVideoDec->GetBuffer (CompressedBufferType, (void**)&pDXVABuffer, &nDXVASize);
		ASSERT (nSize <= nDXVASize);

		if (SUCCEEDED (hr) && (nSize <= nDXVASize))
		{
			if (CompressedBufferType == DXVA2_BitStreamDateBufferType)
				CopyBitstream (pDXVABuffer, (BYTE*)pBuffer, nSize);
			else
				memcpy (pDXVABuffer, (BYTE*)pBuffer, nSize);

			m_ExecuteParams.pCompressedBuffers[m_ExecuteParams.NumCompBuffers].CompressedBufferType = CompressedBufferType;
			m_ExecuteParams.pCompressedBuffers[m_ExecuteParams.NumCompBuffers].DataSize				= nSize;
			m_ExecuteParams.pCompressedBuffers[m_ExecuteParams.NumCompBuffers].NumMBsInBuffer		= (CompressedBufferType == DXVA2_SliceControlBufferType) || (CompressedBufferType == DXVA2_BitStreamDateBufferType);
			m_ExecuteParams.NumCompBuffers++;

		}
		break;
	default :
		ASSERT (FALSE);
		break;
	}
	if (pRealSize) *pRealSize = nSize;

	return hr;
}


HRESULT CDXVADecoder::GetDeliveryBuffer(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, IMediaSample** ppSampleToDeliver)
{
	HRESULT					hr;
	CComPtr<IMediaSample>	pNewSample;

	hr = m_pFilter->GetOutputPin()->GetDeliveryBuffer(&pNewSample, 0, 0, 0);
	if (SUCCEEDED (hr))
	{
		pNewSample->SetTime(&rtStart, &rtStop);
		pNewSample->SetMediaTime(NULL, NULL);
		*ppSampleToDeliver = pNewSample.Detach();
	}
	return hr;
}

HRESULT CDXVADecoder::Execute()
{
	HRESULT		hr = E_INVALIDARG;

	switch (m_nEngine)
	{
	case ENGINE_DXVA1 :
		DWORD			dwFunction;
		HRESULT			hr2;

//		writeDXVA_QueryOrReplyFunc (&dwFunction, DXVA_QUERYORREPLYFUNCFLAG_DECODER_LOCK_QUERY, DXVA_PICTURE_DECODING_FUNCTION);
//		hr = m_pAMVideoAccelerator->Execute (dwFunction, &m_DXVA1Config, sizeof(DXVA_ConfigPictureDecode), NULL, 0, m_dwNumBuffersInfo, m_DXVA1BufferInfo);

		DWORD		dwResult;
		dwFunction = 0x01000000;
		hr = m_pAMVideoAccelerator->Execute (dwFunction, m_DXVA1BufferDesc, sizeof(DXVA_BufferDescription)*m_dwNumBuffersInfo,&dwResult, sizeof(dwResult), m_dwNumBuffersInfo, m_DXVA1BufferInfo);
		ASSERT (SUCCEEDED (hr));

		for (DWORD i=0; i<m_dwNumBuffersInfo; i++)
		{
			hr2 = m_pAMVideoAccelerator->ReleaseBuffer (m_DXVA1BufferInfo[i].dwTypeIndex, m_DXVA1BufferInfo[i].dwBufferIndex);
			ASSERT (SUCCEEDED (hr2));
		}

		m_dwNumBuffersInfo = 0;
		break;

	case ENGINE_DXVA2 :

		for (DWORD i=0; i<m_ExecuteParams.NumCompBuffers; i++)
		{
			hr2 = m_pDirectXVideoDec->ReleaseBuffer (m_ExecuteParams.pCompressedBuffers[i].CompressedBufferType);
			ASSERT (SUCCEEDED (hr2));
		}

		hr = m_pDirectXVideoDec->Execute(&m_ExecuteParams);
		m_ExecuteParams.NumCompBuffers	= 0;
		break;
	default :
		ASSERT (FALSE);
		break;
	}

	return hr;
}

DWORD CDXVADecoder::QueryStatus()
{
	HRESULT						hr = E_INVALIDARG;
	DXVA2_DecodeExecuteParams	ExecuteParams;
	DXVA2_DecodeExtensionData	ExtensionData;
	DXVA_Status_H264			Status;

	switch (m_nEngine)
	{
	case ENGINE_DXVA1 :
		break;

	case ENGINE_DXVA2 :
		memset (&ExecuteParams, 0, sizeof(ExecuteParams));
		memset (&ExtensionData, 0, sizeof(ExtensionData));
		memset (&Status,        0, sizeof(Status));
		ExecuteParams.pExtensionData		= &ExtensionData;
		ExtensionData.pPrivateOutputData	= &Status;
		ExtensionData.PrivateOutputDataSize	= sizeof (Status);
		ExtensionData.Function		= 7;
		Status.bDXVA_Func			= 1;
		Status.field_pic_flag		= 1;
		hr = m_pDirectXVideoDec->Execute(&ExecuteParams);
		break;
	default :
		ASSERT (FALSE);
		break;
	}

	return hr;
}


DWORD CDXVADecoder::GetDXVA1CompressedType (DWORD dwDXVA2CompressedType)
{
	if (dwDXVA2CompressedType <= DXVA2_BitStreamDateBufferType)
		return dwDXVA2CompressedType + 1;
	else
	{
		switch (dwDXVA2CompressedType)
		{
		case DXVA2_MotionVectorBuffer :
			return DXVA_MOTION_VECTOR_BUFFER;
			break;
		case DXVA2_FilmGrainBuffer :
			return DXVA_FILM_GRAIN_BUFFER;
			break;
		default :
			ASSERT (FALSE);
			return DXVA_COMPBUFFER_TYPE_THAT_IS_NOT_USED;
		}
	}
}

HRESULT CDXVADecoder::FindFreeDXVA1Buffer(DWORD dwTypeIndex, DWORD& dwBufferIndex)
{
	HRESULT		hr;
	int			nTry;

	dwBufferIndex	= 0; //(dwBufferIndex + 1) % m_ComBufferInfo[DXVA_PICTURE_DECODE_BUFFER].dwNumCompBuffers;

	DO_DXVA_PENDING_LOOP (m_pAMVideoAccelerator->QueryRenderStatus (-1, dwBufferIndex, 0));
	
	return hr;
}

HRESULT CDXVADecoder::BeginFrame(int nSurfaceIndex, IMediaSample* pSampleToDeliver)
{
	HRESULT				hr   = E_INVALIDARG;
	int					nTry = 0;

	for (int i=0; i<20; i++)
	{
		switch (m_nEngine)
		{
		case ENGINE_DXVA1 :
			AMVABeginFrameInfo			BeginFrameInfo;

			BeginFrameInfo.dwDestSurfaceIndex	= nSurfaceIndex;
			BeginFrameInfo.dwSizeInputData		= sizeof(nSurfaceIndex);
			BeginFrameInfo.pInputData			= &nSurfaceIndex;
			BeginFrameInfo.dwSizeOutputData		= 0;
			BeginFrameInfo.pOutputData			= NULL;

			DO_DXVA_PENDING_LOOP (m_pAMVideoAccelerator->BeginFrame(&BeginFrameInfo));

			ASSERT (SUCCEEDED (hr));
	//		TRACE ("BeginFrame  %d\n",nSurfaceIndex);
			if (SUCCEEDED (hr))
				hr = FindFreeDXVA1Buffer (-1, m_dwBufferIndex);
			break;

		case ENGINE_DXVA2 :
			{
				CComQIPtr<IMFGetService>	pSampleService;
				CComPtr<IDirect3DSurface9>	pDecoderRenderTarget;
				pSampleService = pSampleToDeliver;
				if (pSampleService)
				{
					hr = pSampleService->GetService (MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void**) &pDecoderRenderTarget);
					if (SUCCEEDED (hr)) 
						DO_DXVA_PENDING_LOOP (m_pDirectXVideoDec->BeginFrame(pDecoderRenderTarget, NULL));
				}
			}
			break;
		default :
			ASSERT (FALSE);
			break;
		}

		// For slow accelerator wait a little...
		if (hr != E_PENDING) break;
		Sleep(1);
	}

	return hr;
}


HRESULT CDXVADecoder::EndFrame(int nSurfaceIndex)
{
	HRESULT		hr		= E_INVALIDARG;
	DWORD		dwDummy	= nSurfaceIndex;

	switch (m_nEngine)
	{
	case ENGINE_DXVA1 :
		AMVAEndFrameInfo			EndFrameInfo;

		EndFrameInfo.dwSizeMiscData	= sizeof (dwDummy);		// TODO : usefull ??
		EndFrameInfo.pMiscData		= &dwDummy;
		hr = m_pAMVideoAccelerator->EndFrame(&EndFrameInfo);
//		TRACE ("EndFrame  %d\n",nSurfaceIndex);
		ASSERT (SUCCEEDED (hr));
		break;

	case ENGINE_DXVA2 :
		hr = m_pDirectXVideoDec->EndFrame(NULL);
		break;
	default :
		ASSERT (FALSE);
		break;
	}

	return hr;
}

// === Picture store functions
bool CDXVADecoder::AddToStore (int nSurfaceIndex, IMediaSample* pSample, bool bRefPicture, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, bool bIsField)
{
	if (bIsField && (m_nFieldSurface == -1))
	{
		m_nFieldSurface = nSurfaceIndex;
		m_pFieldSample	= pSample;
		return false;
	}
	else
	{
	//	TRACE ("Add Stor: %10I64d - %10I64d   Ind = %d\n", rtStart, rtStop, nSurfaceIndex);
		ASSERT ((nSurfaceIndex < m_nPicEntryNumber) && (m_pPictureStore[nSurfaceIndex].pSample == NULL));

		m_pPictureStore[nSurfaceIndex].bRefPicture		= bRefPicture;
		m_pPictureStore[nSurfaceIndex].bInUse			= true;
		m_pPictureStore[nSurfaceIndex].bDisplayed		= false;
		m_pPictureStore[nSurfaceIndex].pSample			= pSample;
		m_pPictureStore[nSurfaceIndex].rtStart			= rtStart;
		m_pPictureStore[nSurfaceIndex].rtStop			= rtStop;
		
		m_nFieldSurface	= -1;
		m_nWaitingPics++;
		return true;
	}
}

void CDXVADecoder::UpdateStore (int nSurfaceIndex, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{
	ASSERT ((nSurfaceIndex < m_nPicEntryNumber) && m_pPictureStore[nSurfaceIndex].bInUse && !m_pPictureStore[nSurfaceIndex].bDisplayed);
	m_pPictureStore[nSurfaceIndex].rtStart			= rtStart;
	m_pPictureStore[nSurfaceIndex].rtStop			= rtStop;
}

void CDXVADecoder::RemoveRefFrame (int nSurfaceIndex)
{
	ASSERT ((nSurfaceIndex < m_nPicEntryNumber) && m_pPictureStore[nSurfaceIndex].bInUse);

	m_pPictureStore[nSurfaceIndex].bRefPicture	= false;
	if (m_pPictureStore[nSurfaceIndex].bDisplayed)
		FreePictureSlot (nSurfaceIndex);	
}


int CDXVADecoder::FindOldestFrame()
{
	REFERENCE_TIME		rtMin	= _I64_MAX;
	int					nPos	= -1;

	// TODO : find better solution...
	if (m_nWaitingPics > m_nMaxWaiting)
	{
		for (int i=0; i<m_nPicEntryNumber; i++)
		{
			if (!m_pPictureStore[i].bDisplayed &&
				 m_pPictureStore[i].bInUse &&
				(m_pPictureStore[i].rtStart < rtMin))
			{
				rtMin	= m_pPictureStore[i].rtStart;
				nPos	= i;
			}
		}
	}
	return nPos;
}


HRESULT CDXVADecoder::DisplayNextFrame()
{
	HRESULT						hr = S_FALSE;
	CComPtr<IMediaSample>		pSampleToDeliver;
	int							nPicIndex;

	nPicIndex = FindOldestFrame();
	if (nPicIndex != -1)
	{
		if (m_pPictureStore[nPicIndex].rtStart >= 0)
		{
			switch (m_nEngine)
			{
			case ENGINE_DXVA1 :
				// For DXVA1, query a media sample at the last time (only one in the allocator)
				hr = GetDeliveryBuffer (m_pPictureStore[nPicIndex].rtStart, m_pPictureStore[nPicIndex].rtStop, &pSampleToDeliver);
				if (SUCCEEDED (hr)) hr = m_pAMVideoAccelerator->DisplayFrame(nPicIndex, pSampleToDeliver);
				break;
			case ENGINE_DXVA2 :
				// For DXVA2 media sample is in the picture store
				m_pPictureStore[nPicIndex].pSample->SetTime (&m_pPictureStore[nPicIndex].rtStart, &m_pPictureStore[nPicIndex].rtStop);
				hr = m_pFilter->GetOutputPin()->Deliver(m_pPictureStore[nPicIndex].pSample);
				break;
			}


#ifdef _DEBUG
			static REFERENCE_TIME	rtLast = 0;
			TRACE ("Deliver : %10I64d - %10I64d   (Dur = %10I64d)  {Delta prev = %10I64d}   Ind = %d\n", 
						m_pPictureStore[nPicIndex].rtStart, 
						m_pPictureStore[nPicIndex].rtStop, 
						m_pPictureStore[nPicIndex].rtStop - m_pPictureStore[nPicIndex].rtStart, 
						m_pPictureStore[nPicIndex].rtStart - rtLast, nPicIndex);
			rtLast = m_pPictureStore[nPicIndex].rtStart;
#endif
		}

		m_pPictureStore[nPicIndex].bDisplayed	= true;
		if (!m_pPictureStore[nPicIndex].bRefPicture) 
			FreePictureSlot (nPicIndex);
	}

	return hr;
}

HRESULT CDXVADecoder::GetFreeSurfaceIndex(int& nSurfaceIndex, IMediaSample** ppSampleToDeliver, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{
	HRESULT		hr = E_UNEXPECTED;

	if (m_nFieldSurface != -1)
	{
		nSurfaceIndex		= m_nFieldSurface;
		*ppSampleToDeliver	= m_pFieldSample.Detach();
		return S_FALSE;
	}

	switch (m_nEngine)
	{
	case ENGINE_DXVA1 :
		for (int i=0; i<m_nPicEntryNumber; i++)
		{
			if (!m_pPictureStore[i].bInUse)
			{
				nSurfaceIndex = i;
				return S_OK;
			}
		}
		// Ho ho... 
		ASSERT (FALSE);
		Flush();
		break;
	case ENGINE_DXVA2 :
		CComPtr<IMediaSample>		pNewSample;
		CComQIPtr<IMPCDXVA2Sample>	pMPCDXVA2Sample;
		// TODO : test  IDirect3DDeviceManager9::TestDevice !!!
//		TRACE ("==> Try get buffer...\n");
		if (SUCCEEDED (hr = GetDeliveryBuffer(rtStart, rtStop, &pNewSample)))
		{
			pMPCDXVA2Sample	 = pNewSample;
			nSurfaceIndex    = pMPCDXVA2Sample ? pMPCDXVA2Sample->GetDXSurfaceId() : 0;
			*ppSampleToDeliver = pNewSample.Detach();
//			TRACE ("GetFreeSurfaceIndex : %d\n", nSurfaceIndex);
		}
		break;
	}

	return hr;
}


void CDXVADecoder::FreePictureSlot (int nSurfaceIndex)
{
//	TRACE ("Free    : %d\n", nSurfaceIndex);
	m_pPictureStore[nSurfaceIndex].bInUse		= false;
	m_pPictureStore[nSurfaceIndex].bDisplayed	= false;
	m_pPictureStore[nSurfaceIndex].pSample		= NULL;
	m_nWaitingPics--;
}
