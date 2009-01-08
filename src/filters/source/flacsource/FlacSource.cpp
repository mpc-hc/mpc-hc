/* 
 *	Copyright (C) 2003-2006 Gabest
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

#include "stdafx.h"
#include <mmreg.h>
#include <ks.h>
#include <initguid.h>
#include <uuids.h>
#include <moreuuids.h>
#include "flacsource.h"
#include "..\..\..\DSUtil\DSUtil.h"
#include "..\..\..\DSUtil\GolombBuffer.h"

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] =
{
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_FLAC_FRAMED}
};

const AMOVIESETUP_PIN sudOpPin[] =
{
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CFlacSource), L"MPC - Flac Source", MERIT_NORMAL, countof(sudOpPin), sudOpPin}
};

CFactoryTemplate g_Templates[] =
{
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CFlacSource>, NULL, &sudFilter[0]}
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"), 
		_T("0"), _T("0,4,,664C6143"));

	SetRegKeyValue(
		_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"), 
		_T("Source Filter"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"));

	SetRegKeyValue(
		_T("Media Type\\Extensions"), _T(".flac"), 
		_T("Source Filter"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"));

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	DeleteRegKey(_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{1930D8FF-4739-4e42-9199-3B2EDEAA3BF2}"));
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".flac"));

	return AMovieDllRegisterServer2(FALSE);
}

#include "..\..\FilterApp.h"

CFilterApp theApp;

#endif


#define FLAC_FRAME_HEADER_SIZE			15

//
// CFlacSource
//

CFlacSource::CFlacSource(LPUNKNOWN lpunk, HRESULT* phr)
	: CBaseSource<CFlacStream>(NAME("CFlacSource"), lpunk, phr, __uuidof(this))
{
}

CFlacSource::~CFlacSource()
{
}

// CFlacStream

CFlacStream::CFlacStream(const WCHAR* wfn, CSource* pParent, HRESULT* phr) 
	: CBaseStream(NAME("CFlacStream"), pParent, phr)
	, m_nFileOffset(0)
{
	CAutoLock		cAutoLock(&m_cSharedState);
	CString			fn(wfn);
	int				nFrameNumber;
	int				nOffset;
	unsigned char	locBuff[64];

	if(!m_file.Open(fn, CFile::modeRead|CFile::shareDenyWrite))
	{
		if(phr) *phr = E_FAIL;
		return;
	}

	//CT> Added header check (for FLAC files with ID3 v1/2 tags in them)
	//    We'll look in the first 128kb of the file
	unsigned long locStart = 0;
	int locHeaderFound = 0;
	for(int j = 0; !locHeaderFound && j < 128; j++)	
	{
		unsigned char locTempBuf[1024]={0,};
		m_file.Read((char*)&locTempBuf, sizeof(locTempBuf));
		unsigned char* locPtr = locTempBuf;
		for(int i = 0; i < 1023; i++) 
		{
			if(locPtr[i]=='f' && locPtr[i+1]=='L' && locPtr[i+2]=='a' && locPtr[i+3]=='C')
			{
				locHeaderFound = 1;
				locStart = i + (j * 1024);
				break;
			}
		}
	}
	if(!locHeaderFound)
	{
		if(phr) *phr = E_FAIL;
		return;
	}


	// === Read Metadata info
	m_file.Seek (locStart, CFile::begin);
	m_file.Read((char*)&locBuff, 64);
	CGolombBuffer	Buffer (locBuff, countof(locBuff));

	Buffer.ReadDword();		// fLaC
	Buffer.BitRead(1);		// Last-metadata-block flag
	
	if (Buffer.BitRead(7) != 0)		// Should be a STREAMINFO block
	{
		if(phr) *phr = VFW_E_INVALID_FILE_FORMAT;
		return;
	}

	m_nFileOffset			= Buffer.BitRead(24) + locStart;		// Length (in bytes) of metadata to follow
	m_nMinBlocksize			= Buffer.ReadShort();
	m_nMaxBlocksize			= Buffer.ReadShort();
	m_nMinFrameSize			= (int)Buffer.BitRead(24);
	m_nMaxFrameSize			= (int)Buffer.BitRead(24);
	m_nSamplesPerSec		= (int)Buffer.BitRead(20);
	m_nChannels				= (int)Buffer.BitRead(3)  + 1;
	m_wBitsPerSample		= (WORD)Buffer.BitRead(5) + 1;
	m_i64TotalNumSamples	= Buffer.BitRead(36);
	m_nAvgBytesPerSec		= (m_nChannels * (m_wBitsPerSample >> 3)) * m_nSamplesPerSec;	
	
	// === Init members from base classes
	m_rtDuration			= (m_i64TotalNumSamples * UNITS) / m_nSamplesPerSec;
	m_rtStop				= m_rtDuration;

	GetFileSizeEx (m_file.m_hFile, (LARGE_INTEGER*)&m_llFileSize);
	m_AvgTimePerFrame	= (m_nMaxFrameSize + m_nMinFrameSize) * m_rtDuration / 2 / m_llFileSize;
	m_nTotalFrame		= 0;

	m_nFrameBufferSize	= m_nMaxFrameSize*2;
	m_pFrameBuffer		= new BYTE[m_nFrameBufferSize];
	Buffer.Reset(m_pFrameBuffer, m_nFrameBufferSize);

	// Find first frame
	m_file.Seek (m_nFileOffset, CFile::begin);
	m_file.Read((char*)m_pFrameBuffer, m_nFrameBufferSize);
	if (FindFrameStart (&Buffer, nFrameNumber, nOffset) && (nFrameNumber == 0))
	{
		m_nFileOffset += nOffset;
	}

	// Find last frame
	m_file.Seek (-m_nMaxFrameSize, CFile::end);
	m_file.Read((char*)m_pFrameBuffer, m_nFrameBufferSize);
	Buffer.Reset();
	if (FindFrameStart (&Buffer, m_nTotalFrame, nOffset))
	{
		m_AvgTimePerFrame = m_rtDuration / m_nTotalFrame;
	}

	m_nCurFrame	= -1;
	m_llCurPos	= 0;
	ASSERT (m_nTotalFrame != 0);
}

CFlacStream::~CFlacStream()
{
	delete[] m_pFrameBuffer;
}


bool CFlacStream::ReadUTF8Uint32(CGolombBuffer* pBuffer, int& val)
{
	int v = 0;
	int x;
	unsigned i;

	x = pBuffer->ReadByte();
	if (pBuffer->IsEOF()) return false;

	if(!(x & 0x80)) { /* 0xxxxxxx */
		v = x;
		i = 0;
	}
	else if(x & 0xC0 && !(x & 0x20)) { /* 110xxxxx */
		v = x & 0x1F;
		i = 1;
	}
	else if(x & 0xE0 && !(x & 0x10)) { /* 1110xxxx */
		v = x & 0x0F;
		i = 2;
	}
	else if(x & 0xF0 && !(x & 0x08)) { /* 11110xxx */
		v = x & 0x07;
		i = 3;
	}
	else if(x & 0xF8 && !(x & 0x04)) { /* 111110xx */
		v = x & 0x03;
		i = 4;
	}
	else if(x & 0xFC && !(x & 0x02)) { /* 1111110x */
		v = x & 0x01;
		i = 5;
	}
	else {
		val = 0xffffffff;
		return true;
	}
	for( ; i; i--) {
		x = pBuffer->ReadByte();
		if (pBuffer->IsEOF()) return false;

		if(!(x & 0x80) || (x & 0x40)) { /* 10xxxxxx */
			val = 0xffffffff;
			return true;
		}
		v <<= 6;
		v |= (x & 0x3F);
	}
	
	val = v;
	return true;
}

bool CFlacStream::FindFrameStart (CGolombBuffer* pBuffer, int& nFrameNumber, int& nOffset)
{
	int		nPos = 0;
	int		nPrevFrame[3]  = { -1, -1, -1};
	int		nPrevOffset[3];

	while (!pBuffer->IsEOF() & (nPos<3))
	{
		if ( pBuffer->BitRead (8) == 0xFF && 
			(pBuffer->BitRead (8, true) & 0xFE) == 0xF8)	// Frame header sync word
		{
			nPrevOffset[nPos] = pBuffer->GetPos()-1;
			bool	bBlockingStrategy = !!(pBuffer->BitRead (8) & 1);
			int		nBlockSize		  = pBuffer->BitRead (4);
			int		nSampleRate		  = pBuffer->BitRead (4);
			int		nChannelAssign	  = pBuffer->BitRead (4);
			int		nSampleSize		  = pBuffer->BitRead (3);


			if (pBuffer->BitRead (1) != 0) continue;	// Mandatory value !
			if (nSampleRate == 15) continue;

			if (!bBlockingStrategy)
			{
				if (ReadUTF8Uint32 (pBuffer, nPrevFrame[nPos]))
				{
					if (nPrevFrame[nPos] == -1) continue;
					if ( (nPrevFrame[0]+1 == nPrevFrame[1]) )
					{
						nFrameNumber = nPrevFrame[0];
						nOffset		 = nPrevOffset[0];
						return true;
					}
					else if (nPrevFrame[0]+1 == nPrevFrame[2])
					{
						nFrameNumber = nPrevFrame[0];
						nOffset		 = nPrevOffset[0];
						return true;
					}
					else if (nPrevFrame[1]+1 == nPrevFrame[2])
					{
						nFrameNumber = nPrevFrame[1];
						nOffset		 = nPrevOffset[1];
						return true;
					}
				}
			}
			nPos++;
		}
	}

	return false;
}


bool CFlacStream::FindNextFrameStart (CGolombBuffer* pBuffer, int nFrameNumber, int& nOffset)
{
	int		nCurFrame;

	// Buffer should be on a sync word already!
	ASSERT ((pBuffer->BitRead(16, true) & 0xFFFE) == 0xFFF8);
	if ((pBuffer->BitRead(16, true) & 0xFFFE) != 0xFFF8)
		return false;

	pBuffer->BitRead (8);
	while (!pBuffer->IsEOF())
	{
		if ( pBuffer->BitRead (8) == 0xFF && 
			(pBuffer->BitRead (8, true) & 0xFE) == 0xF8)	// Frame header sync word
		{
			int		nCurOffset		  = pBuffer->GetPos()-1;
			bool	bBlockingStrategy = !!(pBuffer->BitRead (8) & 1);
			int		nBlockSize		  = pBuffer->BitRead (4);
			int		nSampleRate		  = pBuffer->BitRead (4);
			int		nChannelAssign	  = pBuffer->BitRead (4);
			int		nSampleSize		  = pBuffer->BitRead (3);


			if (pBuffer->BitRead (1) != 0) continue;	// Mandatory value !
			if (nSampleRate == 15) continue;

			if (!bBlockingStrategy)
			{
				if (ReadUTF8Uint32 (pBuffer, nCurFrame))
				{
					if (nCurFrame == nFrameNumber+1)
					{
						nOffset		 = nCurOffset;
						return true;
					}
				}
			}
		}
	}

	return false;
}

HRESULT CFlacStream::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
    ASSERT(pAlloc);
    ASSERT(pProperties);

    HRESULT hr = NOERROR;

	pProperties->cBuffers = 1;
	pProperties->cbBuffer = m_nMaxFrameSize + FLAC_FRAME_HEADER_SIZE;	// <= Need more bytes to check sync word!

    ALLOCATOR_PROPERTIES Actual;
    if(FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))) return hr;

    if(Actual.cbBuffer < pProperties->cbBuffer) return E_FAIL;
    ASSERT(Actual.cBuffers == pProperties->cBuffers);

    return NOERROR;
}

HRESULT CFlacStream::FillBuffer(IMediaSample* pSample, int nFrame, BYTE* pOut, long& len)
{
	CGolombBuffer	Buffer(pOut, len);
	LONGLONG		llPos;
	int				nNumber;
	int				nOffset = 0;

	if (nFrame > m_nTotalFrame)
		return S_FALSE;

	// Find frame position
	llPos = m_nFileOffset + nFrame * (m_llFileSize - m_nFileOffset) / m_nTotalFrame;
	while (m_nCurFrame==-1 || nFrame != m_nCurFrame)
	{
		m_file.Seek (llPos, CFile::begin);
		m_file.Read((char*)m_pFrameBuffer, m_nFrameBufferSize);
		Buffer.Reset(m_pFrameBuffer, m_nFrameBufferSize);

		nNumber = 0;
		if (FindFrameStart (&Buffer, nNumber, nOffset) && (nNumber <= nFrame))
		{
			m_nCurFrame = nNumber;
			m_llCurPos	= llPos + nOffset;
			break;
		}
		else
		{
			if (nNumber != 0)
				llPos = max (0, llPos - (nNumber - nFrame) * m_nMaxFrameSize);
			else
				llPos = max (0, llPos - m_nMaxFrameSize);
		}
	}

	// Fill the buffer with one frame
	do
	{
		m_file.Seek (m_llCurPos, CFile::begin);
		m_file.Read((char*)pOut, len);
		Buffer.Reset(pOut, len);
		if (FindNextFrameStart (&Buffer, m_nCurFrame, nOffset))
		{
			m_nCurFrame++;
			m_llCurPos	+= nOffset;
		}
		else
		{
			ASSERT (FALSE);
			return E_FAIL;
		}
	} while (m_nCurFrame < nFrame);

	len	= nOffset;
	TRACE (" Fill buffer N° %04d (%04d) - %06d    %02x %02x %02x %02x %02x\n", m_nCurFrame, nFrame, nOffset, pOut[0], pOut[1], pOut[2], pOut[3], pOut[4]);

	return S_OK;
}


HRESULT CFlacStream::GetMediaType(int iPosition, CMediaType* pmt)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

	if(iPosition == 0)
	{
		pmt->majortype			= MEDIATYPE_Audio;
		pmt->subtype			= MEDIASUBTYPE_FLAC_FRAMED;
		pmt->formattype			= FORMAT_WaveFormatEx;
		WAVEFORMATEX* wfe		= (WAVEFORMATEX*)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
		memset(wfe, 0, sizeof(WAVEFORMATEX));
		wfe->cbSize = sizeof(WAVEFORMATEX);
		wfe->wFormatTag			= WAVE_FORMAT_FLAC;
		wfe->nSamplesPerSec		= m_nSamplesPerSec;
		wfe->nAvgBytesPerSec	= m_nAvgBytesPerSec;
		wfe->nChannels			= m_nChannels;
		wfe->nBlockAlign		= 1;
		wfe->wBitsPerSample		= m_wBitsPerSample;
	}
	else
	{
		return VFW_S_NO_MORE_ITEMS;
	}

    pmt->SetTemporalCompression(FALSE);

	return S_OK;
}

HRESULT CFlacStream::CheckMediaType(const CMediaType* pmt)
{
	if (   pmt->majortype  == MEDIATYPE_Audio
		&& pmt->subtype    == MEDIASUBTYPE_FLAC_FRAMED
		&& pmt->formattype == FORMAT_WaveFormatEx
		&& ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_FLAC)
		return S_OK;
	else
		return E_INVALIDARG;
}
