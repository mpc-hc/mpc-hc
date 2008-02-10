#pragma once

#include "..\BaseSplitter\BaseSplitter.h"

#pragma pack(push, 1)

struct OggPageHeader
{
	DWORD capture_pattern;
	BYTE stream_structure_version;
	BYTE header_type_flag; enum {continued=1, first=2, last=4};
	__int64 granule_position;
	DWORD bitstream_serial_number;
	DWORD page_sequence_number;
	DWORD CRC_checksum;
	BYTE number_page_segments;
};

struct OggVorbisIdHeader
{
	DWORD vorbis_version;
	BYTE audio_channels;
	DWORD audio_sample_rate;
	DWORD bitrate_maximum;
	DWORD bitrate_nominal;
	DWORD bitrate_minimum;
	BYTE blocksize_0:4;
	BYTE blocksize_1:4;
	BYTE framing_flag;
};

struct OggVideoHeader
{
	DWORD w, h;
};

struct OggAudioHeader
{
	WORD nChannels, nBlockAlign;
	DWORD nAvgBytesPerSec;
};

struct OggStreamHeader
{
	char streamtype[8], subtype[4];
	DWORD size;
	__int64 time_unit, samples_per_unit;
	DWORD default_len;
    DWORD buffersize;
	WORD bps;
	WORD alignmentfix1;
    union {OggVideoHeader v; OggAudioHeader a;};
	DWORD alignmentfix2;
};

struct TheoraStreamHeader
{
	USHORT			nSize;
	char			Type;
	char			streamtype[6];
	unsigned char	version_major;
	unsigned char	version_minor;
	unsigned char	version_subminor;
	unsigned char	mb_width[2];
	unsigned char	mb_height[2];
	unsigned char	frame_width[3];
	unsigned char	frame_height[3];
	unsigned char	picx;
	unsigned char	picy;
	unsigned char	framerate_numerator[4];
	unsigned char	framerate_denominator[4];
	unsigned char	aspectratio_numerator[3];
	unsigned char	aspectratio_denominator[3];
	unsigned char	colorspace;
	unsigned char	nombr[3];
	unsigned int	reserved : 3;
	unsigned int	pf : 2;
	unsigned int	kfgshift : 5;
	unsigned int	qual : 6;
};

#pragma pack(pop)

class OggPage : public CAtlArray<BYTE>
{
public:
	OggPageHeader m_hdr;
	CAtlList<int> m_lens;
	OggPage() {memset(&m_hdr, 0, sizeof(m_hdr));}
};

class COggFile : public CBaseSplitterFile
{
	HRESULT Init();

public:
	COggFile(IAsyncReader* pAsyncReader, HRESULT& hr);

	bool Sync(HANDLE hBreak = NULL);
	bool Read(OggPageHeader& hdr, HANDLE hBreak = NULL);
	bool Read(OggPage& page, bool fFull = true, HANDLE hBreak = NULL);
};
