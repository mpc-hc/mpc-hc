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

#pragma once

#include "../SubPic/ISubpic.h"
#include "Rasterizer.h"

class CGolombBuffer;

class CHdmvSub
{
public:

	static const REFERENCE_TIME INVALID_TIME = _I64_MIN;

	enum HDMV_SEGMENT_TYPE
	{
		NO_SEGMENT			= 0xFFFF,
		PALETTE				= 0x14,
		OBJECT				= 0x15,
		PRESENTATION_SEG	= 0x16,
		WINDOW_DEF			= 0x17,
		INTERACTIVE_SEG		= 0x18,
		END_OF_DISPLAY		= 0x80,
		HDMV_SUB1			= 0x81,
		HDMV_SUB2			= 0x82
	};

	
	struct VIDEO_DESCRIPTOR
	{
		SHORT		nVideoWidth;
		SHORT		nVideoHeight;
		BYTE		bFrameRate;		// <= Frame rate here!
	};

	struct COMPOSITION_DESCRIPTOR
	{
		SHORT		nNumber;
		BYTE		bState;
	};

	struct SEQUENCE_DESCRIPTOR
	{
		BYTE		bFirstIn  : 1;
		BYTE		bLastIn	  : 1;
		BYTE		bReserved : 8;
	};

	class CompositionObject : Rasterizer
	{
	public :
		SHORT				m_object_id_ref;
		BYTE				m_window_id_ref;
		bool				m_object_cropped_flag;
		bool				m_forced_on_flag;
		BYTE				m_version_number;
		BYTE				m_sequence_desc;

		SHORT				m_horizontal_position;
		SHORT				m_vertical_position;
		SHORT				m_width;
		SHORT				m_height;

		SHORT				m_cropping_horizontal_position;
		SHORT				m_cropping_vertical_position;
		SHORT				m_cropping_width;
		SHORT				m_cropping_height;
		CComPtr<ISubPic>	m_pSubPic;

		~CompositionObject();

		void				Init(CHdmvSub* pSub);
		void				SetRLEData(BYTE* pBuffer, int nSize);
		void				Render(SubPicDesc& spd);
		void				WriteSeg (SubPicDesc& spd, SHORT nX, SHORT nY, SHORT nCount, SHORT nPaletteIndex);


	private :
		CHdmvSub*	m_pSub;
		BYTE*		m_pRLEData;
		int			m_nRLEDataSize;
	};

	struct HDMV_PALETTE
	{
		BYTE		entry_id;
		BYTE		Y;
		BYTE		Cr;
		BYTE		Cb;
		BYTE		T;
	};

	CHdmvSub();
	~CHdmvSub();

	HRESULT			ParseSample (IMediaSample* pSample);
	long			GetColor(int nIndex);

	int				GetActiveObjects()  { return m_nActiveObjects; };
	REFERENCE_TIME	GetStart()			{ return m_rtStart; };
	REFERENCE_TIME	GetStop()			{ return m_rtStop;  };

	void			Render(SubPicDesc& spd, RECT& bbox);
	HRESULT			GetTextureSize (SIZE& TextureSize, SIZE& VideoSize, POINT& VideoTopLeft);
	HRESULT			SetSubPic (ISubPic* pSubPic);
	void			Reset();

private :

	HDMV_SEGMENT_TYPE		m_nCurSegment;
	BYTE*					m_pSegBuffer;
	int						m_nTotalSegBuffer;
	int						m_nSegBufferPos;
	int						m_nSegSize;

	VIDEO_DESCRIPTOR		m_VideoDescriptor;

	CompositionObject*		m_pObjects;
	int						m_nActiveObjects;
	int						m_nTotalObjects;
	REFERENCE_TIME			m_rtStart;
	REFERENCE_TIME			m_rtStop;

	int						m_nColorNumber;
	long*					m_pColors;


	void				ParsePresentationSegment(CGolombBuffer* pGBuffer);
	void				ParsePalette(CGolombBuffer* pGBuffer, USHORT nSize);
	void				ParseObject(CGolombBuffer* pGBuffer, USHORT nUnitSize);

	void				ParseVideoDescriptor(CGolombBuffer* pGBuffer, VIDEO_DESCRIPTOR* pVideoDescriptor);
	void				ParseCompositionDescriptor(CGolombBuffer* pGBuffer, COMPOSITION_DESCRIPTOR* pCompositionDescriptor);
	void				ParseCompositionObject(CGolombBuffer* pGBuffer, CompositionObject* pCompositionObject);

	void				AllocObjects(int nCount);
	void				AllocSegment(int nSize);
	CompositionObject*	FindObject (SHORT id_ref);
	void				WriteSeg (SubPicDesc& spd, SHORT nX, SHORT nY, SHORT nCount, SHORT nPaletteIndex);
};
