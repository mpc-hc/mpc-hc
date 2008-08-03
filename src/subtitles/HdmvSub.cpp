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

#include "StdAfx.h"
#include "HdmvSub.h"
#include "..\DSUtil\GolombBuffer.h"

#define MAX_PG_DURATION		25*400000*15	//200000000		// Max duration for ST

CHdmvSub::CHdmvSub(void)
{
	m_pColors			= NULL;
	m_nColorNumber		= 0;

	m_nCurSegment		= NO_SEGMENT;
	m_pSegBuffer		= NULL;
	m_nTotalSegBuffer	= 0;
	m_nSegBufferPos		= 0;
	m_nSegSize			= 0;

	m_pObjects			= NULL;
	m_nActiveObjects	= 0;
	m_nTotalObjects		= 0;
	m_rtStart			= 0;
	m_rtStop			= 0;

	memset (&m_VideoDescriptor, 0, sizeof(VIDEO_DESCRIPTOR));
}

CHdmvSub::~CHdmvSub()
{
	delete[] m_pColors;
	delete[] m_pObjects;
	delete[] m_pSegBuffer;
}


void CHdmvSub::AllocObjects(int nCount)
{
	if (nCount > m_nTotalObjects)
	{
		delete[] m_pObjects;
		m_pObjects		= new CompositionObject[nCount];
		m_nTotalObjects	= nCount;
		for (int i=0; i<nCount; i++)
			m_pObjects[i].Init(this);
	}
	m_nActiveObjects = nCount;
}

void CHdmvSub::AllocSegment(int nSize)
{
	if (nSize > m_nTotalSegBuffer)
	{
		delete[] m_pSegBuffer;
		m_pSegBuffer		= new BYTE[nSize];
		m_nTotalSegBuffer	= nSize;
	}
	m_nSegBufferPos	 = 0;
	m_nSegSize       = nSize;
}

CHdmvSub::CompositionObject* CHdmvSub::FindObject (SHORT id_ref)
{
	for (int i=0; i<m_nActiveObjects; i++)
	{
		if (m_pObjects[i].m_object_id_ref == id_ref)
			return &m_pObjects[i];
	}
	return NULL;
}

long CHdmvSub::GetColor(int nIndex)
{
	if ((nIndex != 0xFF) && (nIndex>0) && (nIndex-1 < m_nColorNumber))
		return m_pColors[nIndex-1];
	else
		return 0x00000000;	// Transparent!
}

HRESULT CHdmvSub::ParseSample(IMediaSample* pSample)
{
	CheckPointer (pSample, E_POINTER);
	HRESULT				hr;
	REFERENCE_TIME		rtStart = INVALID_TIME, rtStop = INVALID_TIME;
	BYTE*				pData = NULL;
	int					lSampleLen;

    hr = pSample->GetPointer(&pData);
    if(FAILED(hr) || pData == NULL) return hr;
	lSampleLen = pSample->GetActualDataLength();

	pSample->GetTime(&rtStart, &rtStop);
	if (pData)
	{
		CGolombBuffer		SampleBuffer (pData, lSampleLen);
		if (m_nCurSegment == NO_SEGMENT)
		{
			HDMV_SEGMENT_TYPE	nSegType	= (HDMV_SEGMENT_TYPE)SampleBuffer.ReadByte();
			USHORT				nUnitSize	= SampleBuffer.ReadShort();
			lSampleLen -=3;

			switch (nSegType)
			{
			case PALETTE :
			case OBJECT :
			case PRESENTATION_SEG :
			case END_OF_DISPLAY :
				m_nCurSegment = nSegType;
				AllocSegment (nUnitSize);
				break;
			default :
				return VFW_E_SAMPLE_REJECTED;
			}
		}

		if (m_nCurSegment != NO_SEGMENT)
		{
			if (m_nSegBufferPos < m_nSegSize)
			{
				int		nSize = min (m_nSegSize-m_nSegBufferPos, lSampleLen);
				SampleBuffer.ReadBuffer (m_pSegBuffer+m_nSegBufferPos, nSize);
				m_nSegBufferPos += nSize;
			}
			
			if (m_nSegBufferPos >= m_nSegSize)
			{
				CGolombBuffer	SegmentBuffer (m_pSegBuffer, m_nSegSize);

				switch (m_nCurSegment)
				{
				case PALETTE :
					TRACE ("PALETTE            rtStart=%10I64d\n", rtStart);
					ParsePalette(&SegmentBuffer, m_nSegSize);
					break;
				case OBJECT :
					TRACE ("OBJECT             rtStart=%10I64d\n", rtStart);
					ParseObject(&SegmentBuffer, m_nSegSize);
					break;
				case PRESENTATION_SEG :
					TRACE ("PRESENTATION_SEG   rtStart=%10I64d (size=%d)\n", rtStart, m_nSegSize);
					ParsePresentationSegment(&SegmentBuffer);
					if (m_nActiveObjects>0)
						m_rtStart	= rtStart;
					break;
				case WINDOW_DEF :
					TRACE ("WINDOW_DEF         rtStart=%10I64d\n", rtStart);
					break;
				case END_OF_DISPLAY :
					TRACE ("END_OF_DISPLAY     rtStart=%10I64d\n", rtStart);
					m_rtStop	= rtStart + MAX_PG_DURATION;
					hr = m_nActiveObjects>0 ? S_OK : VFW_S_NO_MORE_ITEMS;
					
					if (m_nActiveObjects==0 && m_pObjects[0].m_pSubPic!=NULL)
					{
						m_pObjects[0].m_pSubPic->SetStop (rtStart);
						m_pObjects[0].m_pSubPic = NULL;
					}

					break;
				default :
					TRACE ("UNKNOWN Seg %d     rtStart=0x%10dd\n", m_nCurSegment, rtStart);
				}

				m_nCurSegment = NO_SEGMENT;
				return hr;
			}
		}
	}

	return hr;
}

void CHdmvSub::ParsePresentationSegment(CGolombBuffer* pGBuffer)
{
	COMPOSITION_DESCRIPTOR	CompositionDescriptor;
	BYTE					nObjectNumber;
	bool					palette_update_flag;
	BYTE					palette_id_ref;

	ParseVideoDescriptor(pGBuffer, &m_VideoDescriptor);
	ParseCompositionDescriptor(pGBuffer, &CompositionDescriptor);
	palette_update_flag	= !!(pGBuffer->ReadByte() & 0x80);
	palette_id_ref		= pGBuffer->ReadByte();
	nObjectNumber		= pGBuffer->ReadByte();

	AllocObjects (nObjectNumber);
	for (int i=0; i<nObjectNumber; i++)
	{
		ParseCompositionObject (pGBuffer, &m_pObjects[i]);
	}
}

void CHdmvSub::ParsePalette(CGolombBuffer* pGBuffer, USHORT nSize)		// #497
{
	int		nNbEntry;
	BYTE	palette_id				= pGBuffer->ReadByte();
	BYTE	palette_version_number	= pGBuffer->ReadByte();

	ASSERT ((nSize-2) % sizeof(HDMV_PALETTE) == 0);
	nNbEntry = (nSize-2) / sizeof(HDMV_PALETTE);
	if (nNbEntry != m_nColorNumber)
	{
		delete[] m_pColors;
		m_nColorNumber	= nNbEntry;
		m_pColors		= new long [m_nColorNumber];
	}

	HDMV_PALETTE*	pPalette = (HDMV_PALETTE*)pGBuffer->GetBufferPos();
	for (int i=0; i<m_nColorNumber; i++)
	{
		BYTE	R = pPalette[i].Y + 1.402*(pPalette[i].Cr-128);;
		BYTE	G = pPalette[i].Y - 0.34414*(pPalette[i].Cb-128) - 0.71414*(pPalette[i].Cr-128);
		BYTE	B = pPalette[i].Y + 1.772*(pPalette[i].Cb-128);

		m_pColors[i] = pPalette[i].T<<24|R<<16|G<<8|B;
	}
}

void CHdmvSub::ParseObject(CGolombBuffer* pGBuffer, USHORT nUnitSize)	// #498
{
	int			nEnd;
	SHORT							object_id	= pGBuffer->ReadShort();
	CHdmvSub::CompositionObject*	pObject		= FindObject (object_id);

	if (pObject != NULL)
	{
		pObject->m_version_number	= pGBuffer->ReadByte();
		pObject->m_sequence_desc	= pGBuffer->ReadByte();

		DWORD	object_data_length  = (DWORD)pGBuffer->BitRead(24);
		
		pObject->m_width			= pGBuffer->ReadShort();
		pObject->m_height 			= pGBuffer->ReadShort();

		nEnd = (DWORD)pGBuffer->GetPos() + object_data_length;
		ASSERT (nUnitSize-object_data_length == 7);

		pObject->SetRLEData (pGBuffer->GetBufferPos(), object_data_length);
	}
}

void CHdmvSub::ParseCompositionObject(CGolombBuffer* pGBuffer, CompositionObject* pCompositionObject)
{
	BYTE	bTemp;
	pCompositionObject->m_object_id_ref	= pGBuffer->ReadShort();
	pCompositionObject->m_window_id_ref	= pGBuffer->ReadByte();
	bTemp = pGBuffer->ReadByte();
	pCompositionObject->m_object_cropped_flag	= !!(bTemp & 0x80);
	pCompositionObject->m_forced_on_flag		= !!(bTemp & 0x40);
	pCompositionObject->m_horizontal_position	= pGBuffer->ReadShort();
	pCompositionObject->m_vertical_position		= pGBuffer->ReadShort();

	if (pCompositionObject->m_object_cropped_flag)
	{
		pCompositionObject->m_cropping_horizontal_position	= pGBuffer->ReadShort();
		pCompositionObject->m_cropping_vertical_position	= pGBuffer->ReadShort();
		pCompositionObject->m_cropping_width				= pGBuffer->ReadShort();
		pCompositionObject->m_cropping_height				= pGBuffer->ReadShort();
	}
}

void CHdmvSub::ParseVideoDescriptor(CGolombBuffer* pGBuffer, VIDEO_DESCRIPTOR* pVideoDescriptor)
{
	pVideoDescriptor->nVideoWidth   = pGBuffer->ReadShort();
	pVideoDescriptor->nVideoHeight  = pGBuffer->ReadShort();
	pVideoDescriptor->bFrameRate	= pGBuffer->ReadByte();
}

void CHdmvSub::ParseCompositionDescriptor(CGolombBuffer* pGBuffer, COMPOSITION_DESCRIPTOR* pCompositionDescriptor)
{
	pCompositionDescriptor->nNumber	= pGBuffer->ReadShort();
	pCompositionDescriptor->bState	= pGBuffer->ReadByte();
}

void CHdmvSub::Render(SubPicDesc& spd, RECT& bbox)
{
	ASSERT (spd.w >= m_pObjects[0].m_width && spd.h >= m_pObjects[0].m_height);

	if (spd.w >= m_pObjects[0].m_width && spd.h >= m_pObjects[0].m_height)
	{
		for (int i=0; i<m_nActiveObjects; i++)
			m_pObjects[i].Render(spd);

		bbox.left	= 0;
		bbox.top	= 0;
		bbox.right	= bbox.left + m_pObjects[0].m_width;
		bbox.bottom	= bbox.top  + m_pObjects[0].m_height;
	}
}

HRESULT CHdmvSub::GetTextureSize (SIZE& TextureSize, SIZE& VideoSize, POINT& VideoTopLeft)
{
	TextureSize.cx	= m_pObjects[0].m_width;
	TextureSize.cy	= m_pObjects[0].m_height;

	VideoSize.cx	= m_VideoDescriptor.nVideoWidth;
	VideoSize.cy	= m_VideoDescriptor.nVideoHeight;

	VideoTopLeft.x	= m_pObjects[0].m_horizontal_position;
	VideoTopLeft.y	= m_pObjects[0].m_vertical_position;

	return S_OK;
}

HRESULT CHdmvSub::SetSubPic (ISubPic* pSubPic)
{
	m_pObjects[0].m_pSubPic = pSubPic;
	return S_OK;
}

void CHdmvSub::Reset()
{
	if (m_nTotalObjects > 0) m_pObjects[0].m_pSubPic = NULL;	// TODO : urk!
	m_nActiveObjects		= 0;
}

// ===== CHdmvSub::CompositionObject

void CHdmvSub::CompositionObject::Init(CHdmvSub* pSub)
{
	m_pSub			= pSub;
	m_pRLEData		= NULL;
	m_nRLEDataSize	= 0;
}

CHdmvSub::CompositionObject::~CompositionObject()
{
	delete[] m_pRLEData;
}


void CHdmvSub::CompositionObject::SetRLEData(BYTE* pBuffer, int nSize)
{
	delete[] m_pRLEData;
	m_pRLEData = new BYTE[nSize];
	memcpy (m_pRLEData, pBuffer, nSize);
	m_nRLEDataSize = nSize;
}

void CHdmvSub::CompositionObject::WriteSeg (SubPicDesc& spd, SHORT nX, SHORT nY, SHORT nCount, SHORT nPaletteIndex)
{
	long nColor = m_pSub->GetColor (nPaletteIndex);
	FillSolidRect (spd, nX, nY, nCount, 1, nColor, 0xFFFFFFFF);
}


void CHdmvSub::CompositionObject::Render(SubPicDesc& spd)
{
	if (m_pRLEData)
	{
		CGolombBuffer	GBuffer (m_pRLEData, m_nRLEDataSize);
		BYTE			bTemp;
		BYTE			bSwitch;
		bool			bEndOfLine = false;

		BYTE			nPaletteIndex;
		SHORT			nCount;
		SHORT			nX	= 0;
		SHORT			nY	= 0;

		while ((nY < m_height) && !GBuffer.IsEOF())
		{
			bTemp = GBuffer.ReadByte();
			if (bTemp != 0)
			{
				nPaletteIndex = bTemp;
				nCount		  = 1;
			}
			else
			{
				bSwitch = GBuffer.ReadByte();
				if (!(bSwitch & 0x80))
				{
					if (!(bSwitch & 0x40))
					{
						nCount		= bSwitch & 0x3F;
						if (nCount > 0)
							nPaletteIndex	= GBuffer.ReadByte();
					}
					else
					{
						nCount			= (bSwitch&0x3F) <<8 | (SHORT)GBuffer.ReadByte();
						nPaletteIndex	= GBuffer.ReadByte();
					}
				}
				else
				{
					if (!(bSwitch & 0x40))
					{
						nCount			= bSwitch & 0x3F;
						nPaletteIndex	= GBuffer.ReadByte();
					}
					else
					{
						nCount			= (bSwitch&0x3F) <<8 | (SHORT)GBuffer.ReadByte();
						nPaletteIndex	= GBuffer.ReadByte();
					}
				}
			}

			if (nCount>0)
			{
				WriteSeg (spd, nX, nY, nCount, nPaletteIndex);
				nX += nCount;
			}
			else
			{
				nY++;
				nX = 0;
			}
		}
	}
}