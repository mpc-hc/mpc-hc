/* 
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
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

#include "../BaseSplitter/BaseSplitter.h"

// {48B93619-A959-45d9-B5FD-E12A67A96CF1}
DEFINE_GUID(MEDIASUBTYPE_RoQ, 
0x48b93619, 0xa959, 0x45d9, 0xb5, 0xfd, 0xe1, 0x2a, 0x67, 0xa9, 0x6c, 0xf1);

// 56516F52-0000-0010-8000-00AA00389B71  'RoQV' == MEDIASUBTYPE_RoQV
DEFINE_GUID(MEDIASUBTYPE_RoQV,
0x56516F52, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define WAVE_FORMAT_RoQA 0x41516F52

// 41516F52-0000-0010-8000-00AA00389B71  'RoQA' == MEDIASUBTYPE_RoQA
DEFINE_GUID(MEDIASUBTYPE_RoQA,
WAVE_FORMAT_RoQA, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#pragma pack(push, 1)
struct roq_chunk {WORD id; DWORD size; WORD arg;};
struct roq_info {WORD w, h, unk1, unk2;};
#pragma pack(pop)

class __declspec(uuid("C73DF7C1-21F2-44C7-A430-D35FB9BB298F"))
CRoQSplitterFilter : public CBaseSplitterFilter
{
	CComPtr<IAsyncReader> m_pAsyncReader;

	struct index {REFERENCE_TIME rtv, rta; __int64 fp;};
	CAtlList<index> m_index;
	POSITION m_indexpos;

protected:
	HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

	bool DemuxInit();
	void DemuxSeek(REFERENCE_TIME rt);
	bool DemuxLoop();

public:
	CRoQSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);
};

class __declspec(uuid("02B8E5C2-4E1F-45D3-9A8E-B8F1EDE6DE09"))
CRoQSourceFilter : public CRoQSplitterFilter
{
public:
	CRoQSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};

class __declspec(uuid("FBEFC5EC-ABA0-4E6C-ACA3-D05FDFEFB853"))
CRoQVideoDecoder : public CTransformFilter
{
	CCritSec m_csReceive;

	REFERENCE_TIME m_rtStart;

	BYTE* m_y[2];
	BYTE* m_u[2];
	BYTE* m_v[2];
	int m_pitch;

	void Copy(BYTE* pOut, BYTE* pIn, DWORD w, DWORD h);

	#pragma pack(push, 1)
	struct roq_cell {BYTE y0, y1, y2, y3, u, v;} m_cells[256];
	struct roq_qcell {roq_cell* idx[4];} m_qcells[256];
	#pragma pack(pop)
	void apply_vector_2x2(int x, int y, roq_cell* cell);
	void apply_vector_4x4(int x, int y, roq_cell* cell);
	void apply_motion_4x4(int x, int y, unsigned char mv, char mean_x, char mean_y);
	void apply_motion_8x8(int x, int y, unsigned char mv, char mean_x, char mean_y);

public:
	CRoQVideoDecoder(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CRoQVideoDecoder();

    HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut);
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);

	HRESULT StartStreaming();
	HRESULT StopStreaming();
};

class __declspec(uuid("226FAF85-E358-4502-8C98-F4224BE76953"))
CRoQAudioDecoder : public CTransformFilter
{
public:
	CRoQAudioDecoder(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CRoQAudioDecoder();

	HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut);
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);
};

