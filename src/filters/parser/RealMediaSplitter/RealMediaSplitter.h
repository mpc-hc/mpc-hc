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

#include <atlbase.h>
#include <atlcoll.h>
#include "../BaseSplitter/BaseSplitter.h"
#include "../../transform/BaseVideoFilter/BaseVideoFilter.h"

#define RMSplitterName L"MPC RealMedia Splitter"
#define RMSourceName   L"MPC RealMedia Source"

#pragma pack(push, 1)

namespace RMFF
{
	typedef struct {
		union {
			char id[4];
			UINT32 object_id;
		};
		UINT32 size;
		UINT16 object_version;
	} ChunkHdr;
	typedef struct {
		UINT32 version, nHeaders;
	} FileHdr;
	typedef struct {
		UINT32 maxBitRate, avgBitRate;
		UINT32 maxPacketSize, avgPacketSize, nPackets;
		UINT32 tDuration, tPreroll;
		UINT32 ptrIndex, ptrData;
		UINT16 nStreams;
		enum flags_t {PN_SAVE_ENABLED=1, PN_PERFECT_PLAY_ENABLED=2, PN_LIVE_BROADCAST=4} flags;
	} Properies;
	typedef struct {
		UINT16 stream;
		UINT32 maxBitRate, avgBitRate;
		UINT32 maxPacketSize, avgPacketSize;
		UINT32 tStart, tPreroll, tDuration;
		CStringA name, mime;
		CAtlArray<BYTE> typeSpecData;
		UINT32 width, height;
		bool interlaced, top_field_first;
	} MediaProperies;
	typedef struct {
		CStringA title, author, copyright, comment;
	} ContentDesc;
	typedef struct {
		UINT64 pos;
		UINT32 nPackets, ptrNext;
	} DataChunk;
	typedef struct {
		UINT16 len, stream;
		UINT32 tStart;
		UINT8 reserved;
		enum flag_t {PN_RELIABLE_FLAG=1, PN_KEYFRAME_FLAG=2} flags; // UINT8
		CAtlArray<BYTE> pData;
	} MediaPacketHeader;
	typedef struct {
		UINT32 nIndices;
		UINT16 stream;
		UINT32 ptrNext;
	} IndexChunkHeader;
	typedef struct {
		UINT32 tStart, ptrFilePos, packet;
	} IndexRecord;
}

struct rvinfo {
	DWORD dwSize, fcc1, fcc2;
	WORD w, h, bpp;
	DWORD unk1, fps, type1, type2;
	BYTE morewh[14];
	void bswap();
};

struct rainfo {
	DWORD fourcc1;				// '.', 'r', 'a', 0xfd
	WORD version1;				// 4 or 5
	WORD unknown1;				// 00 000
	DWORD fourcc2;				// .ra4 or .ra5
	DWORD unknown2;				// ???
	WORD version2;				// 4 or 5
	DWORD header_size;			// == 0x4e
	WORD flavor;				// codec flavor id
	DWORD coded_frame_size;		// coded frame size
	DWORD unknown3;				// big number
	DWORD unknown4;				// bigger number
	DWORD unknown5;				// yet another number
	WORD sub_packet_h;
	WORD frame_size;
	WORD sub_packet_size;
	WORD unknown6;				// 00 00
	void bswap();
};

struct rainfo4 : rainfo {
	WORD sample_rate;
	WORD unknown8;				// 0
	WORD sample_size;
	WORD channels;
	void bswap();
};

struct rainfo5 : rainfo {
	BYTE unknown7[6];			// 0, srate, 0
	WORD sample_rate;
	WORD unknown8;				// 0
	WORD sample_size;
	WORD channels;
	DWORD genr;					// "genr"
	DWORD fourcc3;				// fourcc
	void bswap();
};

#pragma pack(pop)

class CRMFile : public CBaseSplitterFile
{
	// using CBaseSplitterFile::Read;

	HRESULT Init();
	void GetDimensions();

public:
	CRMFile(IAsyncReader* pAsyncReader, HRESULT& hr);

	template<typename T> HRESULT Read(T& var);
	HRESULT Read(RMFF::ChunkHdr& hdr);
	HRESULT Read(RMFF::MediaPacketHeader& mph, bool fFull = true);

	RMFF::FileHdr m_fh;
	RMFF::ContentDesc m_cd;
	RMFF::Properies m_p;
	CAutoPtrList<RMFF::MediaProperies> m_mps;
	CAutoPtrList<RMFF::DataChunk> m_dcs;
	CAutoPtrList<RMFF::IndexRecord> m_irs;

	typedef struct {
		CStringA name, data;
	} subtitle;
	CAtlList<subtitle> m_subs;

	int GetMasterStream();
};

class CRealMediaSplitterOutputPin : public CBaseSplitterOutputPin
{
private:
	typedef struct {
		CAtlArray<BYTE> data;
		DWORD offset;
	} segment;

	class CSegments : public CAutoPtrList<segment>, public CCritSec
	{
	public:
		REFERENCE_TIME rtStart;
		bool fDiscontinuity, fSyncPoint, fMerged;
		void Clear() {
			CAutoLock cAutoLock(this);
			rtStart = 0;
			fDiscontinuity = fSyncPoint = fMerged = false;
			RemoveAll();
		}
	} m_segments;

	CCritSec m_csQueue;

	HRESULT DeliverSegments();

protected:
	HRESULT DeliverPacket(CAutoPtr<Packet> p);

public:
	CRealMediaSplitterOutputPin(CAtlArray<CMediaType>& mts, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
	virtual ~CRealMediaSplitterOutputPin();

	HRESULT DeliverEndFlush();
};

class __declspec(uuid("E21BE468-5C18-43EB-B0CC-DB93A847D769"))
	CRealMediaSplitterFilter : public CBaseSplitterFilter
{
protected:
	CAutoPtr<CRMFile> m_pFile;
	HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

	bool DemuxInit();
	void DemuxSeek(REFERENCE_TIME rt);
	bool DemuxLoop();

	POSITION m_seekpos;
	UINT32 m_seekpacket;
	UINT64 m_seekfilepos;

public:
	CRealMediaSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);
	virtual ~CRealMediaSplitterFilter();

	// CBaseFilter

	STDMETHODIMP_(HRESULT) QueryFilterInfo(FILTER_INFO* pInfo);

	// IKeyFrameInfo

	STDMETHODIMP_(HRESULT) GetKeyFrameCount(UINT& nKFs);
	STDMETHODIMP_(HRESULT) GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs);
};

class __declspec(uuid("765035B3-5944-4A94-806B-20EE3415F26F"))
	CRealMediaSourceFilter : public CRealMediaSplitterFilter
{
public:
	CRealMediaSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};

////////////

class __declspec(uuid("238D0F23-5DC9-45A6-9BE2-666160C324DD"))
	CRealVideoDecoder : public CBaseVideoFilter
{
	typedef HRESULT (WINAPI *PRVCustomMessage)(void*, DWORD);
	typedef HRESULT (WINAPI *PRVFree)(DWORD);
	typedef HRESULT (WINAPI *PRVHiveMessage)(void*, DWORD);
	typedef HRESULT (WINAPI *PRVInit)(void*, DWORD* dwCookie);
	typedef HRESULT (WINAPI *PRVTransform)(BYTE*, BYTE*, void*, void*, DWORD);

	PRVCustomMessage RVCustomMessage;
	PRVFree RVFree;
	PRVHiveMessage RVHiveMessage;
	PRVInit RVInit;
	PRVTransform RVTransform;

	HMODULE m_hDrvDll;
	DWORD m_dwCookie;
	int m_lastBuffSizeDim;

	HRESULT InitRV(const CMediaType* pmt);
	void FreeRV();

	REFERENCE_TIME m_tStart;

	void Resize(BYTE* pIn, DWORD wi, DWORD hi, BYTE* pOut, DWORD wo, DWORD ho);
	void ResizeWidth(BYTE* pIn, DWORD wi, DWORD hi, BYTE* pOut, DWORD wo, DWORD ho);
	void ResizeHeight(BYTE* pIn, DWORD wi, DWORD hi, BYTE* pOut, DWORD wo, DWORD ho);
	void ResizeRow(BYTE* pIn, DWORD wi, DWORD dpi, BYTE* pOut, DWORD wo, DWORD dpo);

	BYTE* m_pI420, *m_pI420Tmp;

public:
	CRealVideoDecoder(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CRealVideoDecoder();

	HRESULT Transform(IMediaSample* pIn);
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);

	HRESULT StartStreaming();
	HRESULT StopStreaming();

	HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	DWORD m_timestamp;
	bool m_fDropFrames;
	HRESULT AlterQuality(Quality q);
};

class __declspec(uuid("941A4793-A705-4312-8DFC-C11CA05F397E"))
	CRealAudioDecoder : public CTransformFilter
{
	typedef HRESULT (WINAPI *PCloseCodec)(DWORD);
	typedef HRESULT (WINAPI *PDecode)(DWORD,BYTE*,long,BYTE*,long*,long);
	typedef HRESULT (WINAPI *PFlush)(DWORD,DWORD,DWORD);
	typedef HRESULT (WINAPI *PFreeDecoder)(DWORD);
	typedef void* (WINAPI *PGetFlavorProperty)(void*,DWORD,DWORD,int*);
	typedef HRESULT (WINAPI *PInitDecoder)(DWORD, void*);
	typedef HRESULT (WINAPI *POpenCodec)(void*);
	typedef HRESULT (WINAPI *POpenCodec2)(void*, const char*);
	typedef HRESULT (WINAPI *PSetFlavor)(DWORD, WORD);
	typedef void (WINAPI *PSetDLLAccessPath)(const char*);
	typedef void (WINAPI *PSetPwd)(DWORD, const char*);

	PCloseCodec RACloseCodec;
	PDecode RADecode;
	PFlush RAFlush;
	PFreeDecoder RAFreeDecoder;
	PGetFlavorProperty RAGetFlavorProperty;
	PInitDecoder RAInitDecoder;
	POpenCodec RAOpenCodec;
	POpenCodec2 RAOpenCodec2;
	PSetFlavor RASetFlavor;
	PSetDLLAccessPath RASetDLLAccessPath;
	PSetPwd RASetPwd;

	CStringA m_dllpath;
	HMODULE m_hDrvDll;
	DWORD m_dwCookie;

	HRESULT InitRA(const CMediaType* pmt);
	void FreeRA();

	REFERENCE_TIME m_tStart;

	rainfo m_rai;
	CAutoVectorPtr<BYTE> m_buff;
	int m_bufflen;
	REFERENCE_TIME m_rtBuffStart;
	bool m_fBuffDiscontinuity;

public:
	CRealAudioDecoder(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CRealAudioDecoder();

	HRESULT Receive(IMediaSample* pIn);
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);

	HRESULT StartStreaming();
	HRESULT StopStreaming();

	HRESULT EndOfStream();
	HRESULT BeginFlush();
	HRESULT EndFlush();
	HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
};
