/* 
 *	Copyright (C) 2003-2005 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include <afxtempl.h>
#include "..\..\..\..\include\qt\qt.h"

//
// CQTDec
//

// {2D261619-3822-4856-A422-DC77BF0FB947}
DEFINE_GUID(CLSID_QTDec, 
0x2d261619, 0x3822, 0x4856, 0xa4, 0x22, 0xdc, 0x77, 0xbf, 0xf, 0xb9, 0x47);

class CQTDec : public CTransformFilter
{
	bool m_fQtInitialized;
	CArray<CMediaType> m_mts;

	QT::GWorldPtr m_pImageGWorld;
	QT::GWorldPtr MakeGWorld();
	void FreeGWorld(QT::GWorldPtr& pImageGWorld);

	QT::ComponentInstance m_cinst;
	QT::CodecCapabilities m_ccap;
	QT::CodecDecompressParams m_cdpar;	
	QT::ImageDescriptionHandle m_hImageDesc;
	bool InitComponent();

public:
	CQTDec(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CQTDec();

#ifdef REGISTER_FILTER
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT* phr);
#endif

    HRESULT BreakConnect(PIN_DIRECTION dir);
	HRESULT CompleteConnect(PIN_DIRECTION dir, IPin* pReceivePin);

	HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut);
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);
};
