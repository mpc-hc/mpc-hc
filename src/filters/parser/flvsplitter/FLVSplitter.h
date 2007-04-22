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

#pragma once

#include <atlcoll.h>
#include "..\BaseSplitter\BaseSplitter.h"

[uuid("47E792CF-0BBE-4F7A-859C-194B0768650A")]
class CFLVSplitterFilter : public CBaseSplitterFilter
{
	UINT32 m_DataOffset;

	bool Sync(__int64& pos);

	struct Tag
	{
		UINT32 PreviousTagSize;
		BYTE TagType;
		UINT32 DataSize;
		UINT32 TimeStamp;
		UINT32 Reserved;
	};

	bool ReadTag(Tag& t);

	struct AudioTag
	{
		BYTE SoundFormat;
		BYTE SoundRate;
		BYTE SoundSize;
		BYTE SoundType;
	};

	bool ReadTag(AudioTag& at);

	struct VideoTag
	{
		BYTE FrameType;
		BYTE CodecID;
	};

	bool ReadTag(VideoTag& vt);

protected:
	CAutoPtr<CBaseSplitterFileEx> m_pFile;
	HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

	bool DemuxInit();
	void DemuxSeek(REFERENCE_TIME rt);
	bool DemuxLoop();

public:
	CFLVSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);
};

[uuid("C9ECE7B3-1D8E-41F5-9F24-B255DF16C087")]
class CFLVSourceFilter : public CFLVSplitterFilter
{
public:
	CFLVSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};

#include "..\..\transform\BaseVideoFilter\BaseVideoFilter.h"
//#include "VP62.h" // comment this out if you don't have VP62.cpp/h

/*

getYUV defined as:

void VP62::getYUV(BYTE** yuv, int* pitch)
{
    yuv[0] = yuvLastFrame + yStride * 48 + 48;
    yuv[1] = yuvLastFrame + ySize + uvStride * 24 + 24;
    yuv[2] = yuvLastFrame + ySize + uvSize + uvStride * 24 + 24;
	*pitch = yStride;
}

*/

__if_exists(VP62) {

[uuid("7CEEEECF-3FEE-4548-B529-C254CAF4D182")]
class CFLVVideoDecoder : public CBaseVideoFilter
{
	VP62 m_dec;

protected:
	HRESULT Transform(IMediaSample* pIn);

public:
	CFLVVideoDecoder(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CFLVVideoDecoder();

	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    HRESULT CheckInputType(const CMediaType* mtIn);
/*
	// TODO
	bool m_fDropFrames;
	HRESULT AlterQuality(Quality q);
*/
};

}