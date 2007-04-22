#include "StdAfx.h"
#include <mmreg.h>
#include "DiracSplitterFile.h"

#include <initguid.h>
#include "..\..\..\..\include\moreuuids.h"

CDiracSplitterFile::CDiracSplitterFile(IAsyncReader* pAsyncReader, HRESULT& hr)
	: CBaseSplitterFile(pAsyncReader, hr)
	, m_rtDuration(0)
{
	if(SUCCEEDED(hr)) hr = Init();
	m_pBuff.SetCount(1024, 1024);
}

HRESULT CDiracSplitterFile::Init()
{
	HRESULT hr = E_FAIL;

	Seek(0);

	UINT64 hdr;
	if(FAILED(ByteRead((BYTE*)&hdr, sizeof(hdr))) || hdr != 0x43415249442D574Bui64) // KW-DIRAC
		return E_FAIL;

	dirac_decoder_t* decoder = dirac_decoder_init(0);

	__int64 limit = min(GetLength(), 2048);

	while(GetPos() < limit)
	{
		BYTE b;
		if(!Next(b)) {ASSERT(0); break;}

		if(b == RAP_START_CODE)
		{
			__int64 pos = GetPos() - 5;
			if(!Next(b)) {ASSERT(0); break;}
			__int64 len = GetPos() - pos;
			Seek(pos);

			m_mt.majortype = MEDIATYPE_Video;
			m_mt.subtype = MEDIASUBTYPE_DiracVideo;
			m_mt.formattype = FORMAT_DiracVideoInfo;
			m_mt.SetSampleSize(1);

			DIRACINFOHEADER* dvih = (DIRACINFOHEADER*)m_mt.AllocFormatBuffer(FIELD_OFFSET(DIRACINFOHEADER, dwSequenceHeader) + len);
			memset(m_mt.Format(), 0, m_mt.FormatLength());

			dvih->cbSequenceHeader = len - 5;
			ByteRead((BYTE*)&dvih->dwSequenceHeader[0], len);

			dirac_buffer(decoder, (BYTE*)&dvih->dwSequenceHeader[0], (BYTE*)&dvih->dwSequenceHeader[0] + len);
			if(dirac_parse(decoder) != STATE_SEQUENCE) {ASSERT(0); break;}

			if(decoder->seq_params.frame_rate.denominator)
			dvih->hdr.AvgTimePerFrame = 10000000i64 * decoder->seq_params.frame_rate.denominator / decoder->seq_params.frame_rate.numerator;
			dvih->hdr.bmiHeader.biSize = sizeof(dvih->hdr.bmiHeader);
			dvih->hdr.bmiHeader.biWidth = decoder->seq_params.width;
			dvih->hdr.bmiHeader.biHeight = decoder->seq_params.height;
			dvih->hdr.bmiHeader.biCompression = m_mt.subtype.Data1;
			dvih->hdr.dwInterlaceFlags = 0;
			if(decoder->seq_params.interlace) dvih->hdr.dwInterlaceFlags |= AMINTERLACE_IsInterlaced;
			if(decoder->seq_params.topfieldfirst) dvih->hdr.dwInterlaceFlags |= AMINTERLACE_Field1First;
			dvih->hdr.dwPictAspectRatioX = dvih->hdr.bmiHeader.biWidth;
			dvih->hdr.dwPictAspectRatioY = dvih->hdr.bmiHeader.biHeight;

			m_rtDuration = 0;// dvih->hdr.AvgTimePerFrame * decoder->seq_params.num_frames; // WTF

			hr = S_OK;

			break;
		}
	}

    dirac_decoder_close(decoder);

	return hr;
}

UINT64 CDiracSplitterFile::UnsignedGolombDecode()
{    
    int M = 0;
	while(M < 64 && !BitRead(1))
		M++;

	UINT64 info = 0;
    for(int i = 0; i < M; i++)
		info |= BitRead(1) << i;

	return (1ui64<<M)-1 + info;
}

bool CDiracSplitterFile::Next(BYTE& code, __int64 len)
{
	BitByteAlign();
	UINT64 qw = -1;
	do
	{
		if(len-- == 0 || !GetRemaining()) return(false);
		qw = (qw << 8) | (BYTE)BitRead(8);
	}
	while((qw&0xffffffff00) != ((UINT64)START_CODE_PREFIX<<8));
	code = (BYTE)(qw & 0xff);
	return(true);
}

const BYTE* CDiracSplitterFile::NextBlock(BYTE& code, int& size, int& fnum)
{
	BYTE* pBuff = m_pBuff.GetData();
	code = NOT_START_CODE;
	size = 0;

	// TODO: make sure we are at a start code right now

	while(GetRemaining())
	{
		if(GetRemaining() >= 5)
		{
			UINT64 qw = BitRead(40, true);

			if(size == 0)
			{
				if((qw & 0xffffffff00) == ((UINT64)START_CODE_PREFIX<<8) && (qw & 0xff) != NOT_START_CODE)
					code = (BYTE)(qw & 0xff);

				if(isFrameStartCode(code))
				{
					__int64 pos = GetPos();
					Seek(pos + 5);
					fnum = (int)UnsignedGolombDecode();
					Seek(pos);
				}
			}
			else
			{
				if((qw & 0xffffffff00) == ((UINT64)START_CODE_PREFIX<<8) && (qw & 0xff) != NOT_START_CODE)
					break;
			}
		}

		if(size >= m_pBuff.GetCount())
		{
			int newsize = max(1024, size*2);
			m_pBuff.SetCount(newsize, newsize);
			pBuff = m_pBuff.GetData();
		}

		pBuff[size++] = (BYTE)BitRead(8);
	}

	return pBuff;
}
