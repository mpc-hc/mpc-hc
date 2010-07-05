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

#pragma once

#include "DVBChannel.h"


#pragma pack(1)
typedef struct
{
	UINT8		TableID;
	WORD		SectionSyntaxIndicator  : 1;
	WORD		Reserved1				: 3;
	WORD		SectionLength			: 12;
	WORD		BouquetID;
	UINT8		Reserved2				: 1;
	UINT8		VersionNumber			: 5;
	UINT8		CurrentNextIndicator	: 1;
	UINT8		SectionNumber;
	UINT8		LastSectionNumber;
} SI_HEADER;



class CMpeg2DataParser
{
public :

	CMpeg2DataParser(IBaseFilter* pFilter);

	HRESULT		ParseSDT(ULONG ulFreq);
	HRESULT		ParsePAT();
	HRESULT		ParseNIT();

	static CString ConvertString (BYTE* pBuffer, int nLength);

	CAtlMap<int,CDVBChannel>	Channels;

private :

	CComQIPtr<IMpeg2Data>		m_pData;
	MPEG2_FILTER				m_Filter;


	DVB_STREAM_TYPE	ConvertToDVBType(PES_STREAM_TYPE nType);
	HRESULT			ParseSIHeader(CGolombBuffer& gb, DVB_SI SIType, WORD& wSectionLength, WORD& wTSID);
	HRESULT			ParsePMT(CDVBChannel& Channel);
};
