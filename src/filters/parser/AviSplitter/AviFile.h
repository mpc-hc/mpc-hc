#pragma once

#include <Aviriff.h> // conflicts with vfw.h...
#include "../BaseSplitter/BaseSplitter.h"

class CAviFile : public CBaseSplitterFile
{
	HRESULT Init();
	HRESULT Parse(DWORD parentid, __int64 end);
	HRESULT BuildAMVIndex();

public:
	CAviFile(IAsyncReader* pAsyncReader, HRESULT& hr);

	//using CBaseSplitterFile::Read;
	template<typename T>
	HRESULT Read(T& var, int offset = 0)
	{
		memset(&var, 0, sizeof(var));
		HRESULT hr = ByteRead((BYTE*)&var + offset, sizeof(var) - offset);
		return hr;
	}

	AVIMAINHEADER m_avih;
	struct ODMLExtendedAVIHeader {DWORD dwTotalFrames;} m_dmlh;
//	VideoPropHeader m_vprp;
	struct strm_t
	{
		AVISTREAMHEADER strh;
		CAtlArray<BYTE> strf;
		CStringA strn;
		CAutoPtr<AVISUPERINDEX> indx;
		struct chunk {UINT64 fKeyFrame:1, fChunkHdr:1, size:62; UINT64 filepos; DWORD orgsize;};
		CAtlArray<chunk> cs;
		UINT64 totalsize;
		REFERENCE_TIME GetRefTime(DWORD frame, UINT64 size);
		int GetTime(DWORD frame, UINT64 size);
		int GetFrame(REFERENCE_TIME rt);
		int GetKeyFrame(REFERENCE_TIME rt);
		DWORD GetChunkSize(DWORD size);
		bool IsRawSubtitleStream();

		// tmp
		struct chunk2 {DWORD t; DWORD n;};
		CAtlArray<chunk2> cs2;
	};
	CAutoPtrArray<strm_t> m_strms;
	CAtlMap<DWORD, CStringA> m_info;
	CAutoPtr<AVIOLDINDEX> m_idx1;

	CAtlList<UINT64> m_movis;
	bool			 m_isamv;
    
	REFERENCE_TIME GetTotalTime();
	HRESULT BuildIndex();
	void EmptyIndex();
	bool IsInterleaved(bool fKeepInfo = false);
};

#define TRACKNUM(fcc) (10*((fcc&0xff)-0x30) + (((fcc>>8)&0xff)-0x30))
#define TRACKTYPE(fcc) ((WORD)((((DWORD)fcc>>24)&0xff)|((fcc>>8)&0xff00)))
