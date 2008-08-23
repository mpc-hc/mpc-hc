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

#define INDEX_TRANSPARENT	0xFF

#if (0)		// Set to 1 to activate HDMV subtitles traces
	#define TRACE_HDMVSUB		TRACE
#else
	#define TRACE_HDMVSUB
#endif


CHdmvSub::CHdmvSub(void)
{
	m_pColors			= NULL;
	m_nColorNumber		= 0;

	m_nCurSegment		= NO_SEGMENT;
	m_pSegBuffer		= NULL;
	m_nTotalSegBuffer	= 0;
	m_nSegBufferPos		= 0;
	m_nSegSize			= 0;
	m_pCurrentObject	= NULL;

	memset (&m_VideoDescriptor, 0, sizeof(VIDEO_DESCRIPTOR));
}

CHdmvSub::~CHdmvSub()
{
	m_pObjects.RemoveAll();
	delete[] m_pColors;
	delete[] m_pSegBuffer;
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

POSITION CHdmvSub::GetStartPosition(REFERENCE_TIME rt, double fps)
{
	CompositionObject*	pObject;

	// Cleanup old PG
	while (m_pObjects.GetCount()>0)
	{
		pObject = m_pObjects.GetHead();
		if (pObject->m_rtStop < rt)
		{
			TRACE_HDMVSUB ("CHdmvSub:HDMV remove object %d  %S => %S (rt=%S)\n", pObject->GetRLEDataSize(), 
						   ReftimeToString (pObject->m_rtStart), ReftimeToString(pObject->m_rtStop), ReftimeToString(rt));
			m_pObjects.RemoveHead();
			delete pObject;
		}
		else
			break;
	}

	return m_pObjects.GetHeadPosition(); 
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
					// TRACE_HDMVSUB ("CHdmvSub:PALETTE            rtStart=%10I64d\n", rtStart);
					ParsePalette(&SegmentBuffer, m_nSegSize);
					break;
				case OBJECT :
					//TRACE_HDMVSUB ("CHdmvSub:OBJECT             %S\n", ReftimeToString(rtStart));
					ParseObject(&SegmentBuffer, m_nSegSize);
					break;
				case PRESENTATION_SEG :
					TRACE_HDMVSUB ("CHdmvSub:PRESENTATION_SEG   %S (size=%d)\n", ReftimeToString(rtStart), m_nSegSize);
					
					if (ParsePresentationSegment(&SegmentBuffer) > 0)
					{
						m_pCurrentObject->m_rtStart	= rtStart;
						m_pCurrentObject->m_rtStop	= rtStart + 1;
					}
					else
					{
						if (m_pCurrentObject)
						{
//							CompositionObject*	pObject = m_pObjects.GetHead();
							m_pCurrentObject->m_rtStop = rtStart;
//if (rtStart < m_pCurrentObject->m_rtStart)
//	m_pCurrentObject->m_rtStop = m_pCurrentObject->m_rtStart+400000*50;
							m_pObjects.AddTail (m_pCurrentObject);
							TRACE_HDMVSUB ("CHdmvSub:HDMV : %S => %S\n", ReftimeToString (m_pCurrentObject->m_rtStart), ReftimeToString(rtStart));
							m_pCurrentObject = NULL;
						}
					}
					break;
				case WINDOW_DEF :
					TRACE_HDMVSUB ("CHdmvSub:WINDOW_DEF         %S\n", ReftimeToString(rtStart));
					break;
				case END_OF_DISPLAY :
					TRACE_HDMVSUB ("CHdmvSub:END_OF_DISPLAY     %S\n", ReftimeToString(rtStart));
					if (m_pCurrentObject)
						m_pCurrentObject->SetColors (m_nColorNumber, m_pColors);
					//{
					//	m_pObjects.AddTail (m_pCurrentObject);
					//	m_pCurrentObject = NULL;
					//}
//					hr = m_nActiveObjects>0 ? S_OK : VFW_S_NO_MORE_ITEMS;
					break;
				default :
					TRACE_HDMVSUB ("CHdmvSub:UNKNOWN Seg %d     rtStart=0x%10dd\n", m_nCurSegment, rtStart);
				}

				m_nCurSegment = NO_SEGMENT;
				return hr;
			}
		}
	}

	return hr;
}

int CHdmvSub::ParsePresentationSegment(CGolombBuffer* pGBuffer)
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

	if (nObjectNumber > 0)
	{
		delete m_pCurrentObject;
		m_pCurrentObject = new CompositionObject();
		ParseCompositionObject (pGBuffer, m_pCurrentObject);
	}

	return nObjectNumber;
}

void CHdmvSub::ParsePalette(CGolombBuffer* pGBuffer, USHORT nSize)		// #497
{
	int		nNbEntry;
	BYTE	palette_id				= pGBuffer->ReadByte();
	BYTE	palette_version_number	= pGBuffer->ReadByte();

	ASSERT ((nSize-2) % sizeof(HDMV_PALETTE) == 0);
	nNbEntry = (nSize-2) / sizeof(HDMV_PALETTE);
	if (nNbEntry>0 && nNbEntry!=m_nColorNumber)
	{
		delete[] m_pColors;
		m_nColorNumber	= nNbEntry;
		m_pColors		= new long [m_nColorNumber];
	}

	HDMV_PALETTE*	pPalette = (HDMV_PALETTE*)pGBuffer->GetBufferPos();
	for (int i=0; i<m_nColorNumber; i++)
	{
		BYTE	R = pPalette[i].Y + 1.402*(pPalette[i].Cr-128);
		BYTE	G = pPalette[i].Y - 0.34414*(pPalette[i].Cb-128) - 0.71414*(pPalette[i].Cr-128);
		BYTE	B = pPalette[i].Y + 1.772*(pPalette[i].Cb-128);

		m_pColors[i] = pPalette[i].T<<24|R<<16|G<<8|B;
	}
}

void CHdmvSub::ParseObject(CGolombBuffer* pGBuffer, USHORT nUnitSize)	// #498
{
	SHORT	object_id	= pGBuffer->ReadShort();
	BYTE	m_sequence_desc;

	ASSERT (m_pCurrentObject != NULL);
	if (m_pCurrentObject && m_pCurrentObject->m_object_id_ref == object_id)
	{
		m_pCurrentObject->m_version_number	= pGBuffer->ReadByte();
		m_sequence_desc						= pGBuffer->ReadByte();

		if (m_sequence_desc & 0x80)
		{
			DWORD	object_data_length  = (DWORD)pGBuffer->BitRead(24);
			
			m_pCurrentObject->m_width			= pGBuffer->ReadShort();
			m_pCurrentObject->m_height 			= pGBuffer->ReadShort();

			m_pCurrentObject->SetRLEData (pGBuffer->GetBufferPos(), nUnitSize-11, object_data_length-4);

			TRACE_HDMVSUB ("CHdmvSub:NewObject	size=%ld, total obj=%d, %dx%d\n", object_data_length, m_pObjects.GetCount(),
						   m_pCurrentObject->m_width, m_pCurrentObject->m_height);
		}
		else
			m_pCurrentObject->AppendRLEData (pGBuffer->GetBufferPos(), nUnitSize-4);
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

void CHdmvSub::Render(SubPicDesc& spd, REFERENCE_TIME rt, RECT& bbox)
{
	CompositionObject*	pObject = FindObject (rt);

	ASSERT (pObject!=NULL && spd.w >= pObject->m_width && spd.h >= pObject->m_height);

	if (pObject && spd.w >= pObject->m_width && spd.h >= pObject->m_height)
	{
		TRACE_HDMVSUB ("CHdmvSub:Render	    size=%ld,  ObjRes=%dx%d,  SPDRes=%dx%d\n", pObject->GetRLEDataSize(), 
					   pObject->m_width, pObject->m_height, spd.w, spd.h);
		pObject->Render(spd);

		bbox.left	= 0;
		bbox.top	= 0;
		bbox.right	= bbox.left + pObject->m_width;
		bbox.bottom	= bbox.top  + pObject->m_height;
	}
}

HRESULT CHdmvSub::GetTextureSize (POSITION pos, SIZE& MaxTextureSize, SIZE& VideoSize, POINT& VideoTopLeft)
{
	CompositionObject*	pObject = m_pObjects.GetAt (pos);
	if (pObject)
	{
		// Texture size should be video size width. Height is limited (to prevent performances issues with
		// more than 1024x768 pixels)
		MaxTextureSize.cx	= min (m_VideoDescriptor.nVideoWidth, 1920);
		MaxTextureSize.cy	= min (m_VideoDescriptor.nVideoHeight, 1024*768/MaxTextureSize.cx);

		VideoSize.cx	= m_VideoDescriptor.nVideoWidth;
		VideoSize.cy	= m_VideoDescriptor.nVideoHeight;

		VideoTopLeft.x	= pObject->m_horizontal_position;
		VideoTopLeft.y	= pObject->m_vertical_position;

		return S_OK;
	}

	ASSERT (FALSE);
	return E_INVALIDARG;
}


void CHdmvSub::Reset()
{
	CompositionObject*	pObject;
	while (m_pObjects.GetCount() > 0)
	{
		pObject = m_pObjects.RemoveHead();
		delete pObject;
	}
}

CHdmvSub::CompositionObject*	CHdmvSub::FindObject(REFERENCE_TIME rt)
{
	POSITION	pos = m_pObjects.GetHeadPosition();

	while (pos)
	{
		CompositionObject*	pObject = m_pObjects.GetAt (pos);

		if (rt >= pObject->m_rtStart && rt < pObject->m_rtStop)
			return pObject;

		m_pObjects.GetNext(pos);
	}

	return NULL;
}


// ===== CHdmvSub::CompositionObject

CHdmvSub::CompositionObject::CompositionObject()
{
	m_rtStart		= 0;
	m_rtStop		= 0;
	m_pRLEData		= NULL;
	m_nRLEDataSize	= 0;
	m_nRLEPos		= 0;
	m_pColors		= NULL;
	m_nColorNumber	= 0;
}

CHdmvSub::CompositionObject::~CompositionObject()
{
	delete[] m_pRLEData;
	delete[] m_pColors;
}

void CHdmvSub::CompositionObject::SetColors (int nColorNumber, long* pColors)
{
	ASSERT (m_pColors == NULL);		// Memory leak if not null...
	m_nColorNumber	= nColorNumber;
	m_pColors		= new long[nColorNumber];
	memcpy (m_pColors, pColors, nColorNumber*sizeof(long));
}

long CHdmvSub::CompositionObject::GetColor(int nIndex)
{
	if ((nIndex != INDEX_TRANSPARENT) && (nIndex>0) && (nIndex < m_nColorNumber-1))
		return m_pColors[nIndex];
	else
		return 0x00000000;	// Transparent!
}

void CHdmvSub::CompositionObject::SetRLEData(BYTE* pBuffer, int nSize, int nTotalSize)
{
	delete[] m_pRLEData;
	m_pRLEData		= new BYTE[nTotalSize];
	m_nRLEDataSize	= nTotalSize;
	m_nRLEPos		= nSize;

	memcpy (m_pRLEData, pBuffer, nSize);
}

void CHdmvSub::CompositionObject::AppendRLEData(BYTE* pBuffer, int nSize)
{
	ASSERT (m_nRLEPos+nSize <= m_nRLEDataSize);
	if (m_nRLEPos+nSize <= m_nRLEDataSize)
	{
		memcpy (m_pRLEData+m_nRLEPos, pBuffer, nSize);
		m_nRLEPos += nSize;
	}
}

void CHdmvSub::CompositionObject::WriteSeg (SubPicDesc& spd, SHORT nX, SHORT nY, SHORT nCount, SHORT nPaletteIndex)
{
	long nColor = GetColor (nPaletteIndex);
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
							nPaletteIndex	= INDEX_TRANSPARENT;
					}
					else
					{
						nCount			= (bSwitch&0x3F) <<8 | (SHORT)GBuffer.ReadByte();
						nPaletteIndex	= INDEX_TRANSPARENT;
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