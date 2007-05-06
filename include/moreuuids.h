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
 *  Note: This interface was defined for the matroska container format 
 *  originally, but can be implemented for other formats as well.
 *
 */

#pragma once

#include <dvdmedia.h>

// 30323449-0000-0010-8000-00AA00389B71  'I420' == MEDIASUBTYPE_I420
DEFINE_GUID(MEDIASUBTYPE_I420,
0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define WAVE_FORMAT_DOLBY_AC3 0x2000
// {00002000-0000-0010-8000-00aa00389b71}
DEFINE_GUID(MEDIASUBTYPE_WAVE_DOLBY_AC3, 
WAVE_FORMAT_DOLBY_AC3, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define WAVE_FORMAT_DVD_DTS 0x2001
// {00002001-0000-0010-8000-00aa00389b71}
DEFINE_GUID(MEDIASUBTYPE_WAVE_DTS, 
WAVE_FORMAT_DVD_DTS, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// Be compatible with 3ivx
#define WAVE_FORMAT_AAC 0x00FF
// {000000FF-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_AAC,
WAVE_FORMAT_AAC, 0x000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// ... and also compatible with nero
// btw, older nero parsers use a lower-case fourcc, newer upper-case (why can't it just offer both?)
// {4134504D-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_MP4A,
0x4134504D, 0x000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// {6134706D-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_mp4a,
0x6134706D, 0x000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define WAVE_FORMAT_MP3 0x0055
// 00000055-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_MP3,
WAVE_FORMAT_MP3, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define WAVE_FORMAT_FLAC 0xF1AC
// 0000F1AC-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_FLAC,
WAVE_FORMAT_FLAC, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// {1541C5C0-CDDF-477d-BC0A-86F8AE7F8354}
DEFINE_GUID(MEDIASUBTYPE_FLAC_FRAMED,
0x1541c5c0, 0xcddf, 0x477d, 0xbc, 0xa, 0x86, 0xf8, 0xae, 0x7f, 0x83, 0x54);

#define WAVE_FORMAT_TTA1 0x77A1
// {000077A1-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_TTA1,
WAVE_FORMAT_TTA1, 0x000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define WAVE_FORMAT_WAVPACK4 0x5756
// {00005756-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_WAVPACK4,
WAVE_FORMAT_WAVPACK4, 0x000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// {DA5B82EE-6BD2-426f-BF1E-30112DA78AE1}
DEFINE_GUID(MEDIASUBTYPE_SVCD_SUBPICTURE, 
0xda5b82ee, 0x6bd2, 0x426f, 0xbf, 0x1e, 0x30, 0x11, 0x2d, 0xa7, 0x8a, 0xe1);

// {7B57308F-5154-4c36-B903-52FE76E184FC}
DEFINE_GUID(MEDIASUBTYPE_CVD_SUBPICTURE, 
0x7b57308f, 0x5154, 0x4c36, 0xb9, 0x3, 0x52, 0xfe, 0x76, 0xe1, 0x84, 0xfc);

// {0E3A2342-F6E2-4c91-BDAE-87C71EAD0D63}
DEFINE_GUID(MEDIASUBTYPE_MPEG2_PVA, 
0xe3a2342, 0xf6e2, 0x4c91, 0xbd, 0xae, 0x87, 0xc7, 0x1e, 0xad, 0xd, 0x63);

// {6B6D0800-9ADA-11d0-A520-00A0D10129C0}
DEFINE_GUID(CLSID_NetShowSource, 
0x6b6d0800, 0x9ada, 0x11d0, 0xa5, 0x20, 0x0, 0xa0, 0xd1, 0x1, 0x29, 0xc0);

// DirectShowMedia

// {5E9C9EE0-2E4A-4f22-9906-7BBBB75AA2B6}
DEFINE_GUID(MEDIASUBTYPE_DirectShowMedia, 
0x5e9c9ee0, 0x2e4a, 0x4f22, 0x99, 0x6, 0x7b, 0xbb, 0xb7, 0x5a, 0xa2, 0xb6);

// Dirac

// {A29DA00F-A22B-40ea-98DE-2F7FECADA5DE}
DEFINE_GUID(MEDIASUBTYPE_Dirac, 
0xa29da00f, 0xa22b, 0x40ea, 0x98, 0xde, 0x2f, 0x7f, 0xec, 0xad, 0xa5, 0xde);

// {64726376-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_DiracVideo,
0x64726376, 0x000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// {D2667A7E-4055-4244-A65F-DDDDF2B74BD7}
DEFINE_GUID(FORMAT_DiracVideoInfo, 
0xd2667a7e, 0x4055, 0x4244, 0xa6, 0x5f, 0xdd, 0xdd, 0xf2, 0xb7, 0x4b, 0xd7);

struct DIRACINFOHEADER
{
    VIDEOINFOHEADER2 hdr;
    DWORD cbSequenceHeader;
    DWORD dwSequenceHeader[1];
};

// MP4

// {08E22ADA-B715-45ed-9D20-7B87750301D4}
DEFINE_GUID(MEDIASUBTYPE_MP4, 
0x8e22ada, 0xb715, 0x45ed, 0x9d, 0x20, 0x7b, 0x87, 0x75, 0x3, 0x1, 0xd4);

// FLV

// {F2FAC0F1-3852-4670-AAC0-9051D400AC54}
DEFINE_GUID(MEDIASUBTYPE_FLV, 
0xf2fac0f1, 0x3852, 0x4670, 0xaa, 0xc0, 0x90, 0x51, 0xd4, 0x0, 0xac, 0x54);

// 34564c46-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_FLV4,
0x34564c46, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 30365056-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_VP60,
0x30365056, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 31365056-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_VP61,
0x31365056, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 32365056-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_VP62,
0x32365056, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

//
// RealMedia
//

// {57428EC6-C2B2-44a2-AA9C-28F0B6A5C48E}
DEFINE_GUID(MEDIASUBTYPE_RealMedia, 
0x57428ec6, 0xc2b2, 0x44a2, 0xaa, 0x9c, 0x28, 0xf0, 0xb6, 0xa5, 0xc4, 0x8e);

// 30315652-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_RV10,
0x30315652, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 30325652-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_RV20,
0x30325652, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 30335652-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_RV30,
0x30335652, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 30345652-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_RV40,
0x30345652, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 31345652-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_RV41,
0x31345652, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 345f3431-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_14_4,
0x345f3431, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 385f3832-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_28_8,
0x385f3832, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 43525441-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_ATRC,
0x43525441, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 4b4f4f43-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_COOK,
0x4b4f4f43, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 54454e44-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_DNET,
0x54454e44, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 52504953-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_SIPR,
0x52504953, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 43414152-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_RAAC,
0x43414152, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 50434152-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_RACP,
0x50434152, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

enum 
{
	WAVE_FORMAT_14_4 = 0x2002,
	WAVE_FORMAT_28_8 = 0x2003,
	WAVE_FORMAT_ATRC = 0x0270, //WAVE_FORMAT_SONY_SCX,
	WAVE_FORMAT_COOK = 0x2004,
	WAVE_FORMAT_DNET = 0x2005,
	WAVE_FORMAT_RAAC = 0x2006,
	WAVE_FORMAT_RACP = 0x2007,
	WAVE_FORMAT_SIPR = 0x0130, //WAVE_FORMAT_SIPROLAB_ACEPLNET,
};

//
// PS2
//

#define WAVE_FORMAT_PS2_PCM 0xF521
// 0000F521-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_PS2_PCM,
WAVE_FORMAT_PS2_PCM, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define WAVE_FORMAT_PS2_ADPCM 0xF522
// 0000F522-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_PS2_ADPCM,
WAVE_FORMAT_PS2_ADPCM, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

struct WAVEFORMATEXPS2 : public WAVEFORMATEX
{
    DWORD dwInterleave;

	struct WAVEFORMATEXPS2()
	{
		memset(this, 0, sizeof(*this)); 
		cbSize = sizeof(WAVEFORMATEXPS2) - sizeof(WAVEFORMATEX);
	}
};

// {4F3D3D21-6D7C-4f73-AA05-E397B5EAE0AA}
DEFINE_GUID(MEDIASUBTYPE_PS2_SUB, 
0x4f3d3d21, 0x6d7c, 0x4f73, 0xaa, 0x5, 0xe3, 0x97, 0xb5, 0xea, 0xe0, 0xaa);

// Haali's video renderer

// {760A8F35-97E7-479d-AAF5-DA9EFF95D751}
DEFINE_GUID(CLSID_DXR,
0x760a8f35, 0x97e7, 0x479d, 0xaa, 0xf5, 0xda, 0x9e, 0xff, 0x95, 0xd7, 0x51);

//
// Ogg
//

// f07e245f-5a1f-4d1e-8bff-dc31d84a55ab
DEFINE_GUID(CLSID_OggSplitter,
0xf07e245f, 0x5a1f, 0x4d1e, 0x8b, 0xff, 0xdc, 0x31, 0xd8, 0x4a, 0x55, 0xab);

// {078C3DAA-9E58-4d42-9E1C-7C8EE79539C5}
DEFINE_GUID(CLSID_OggSplitPropPage,
0x78c3daa, 0x9e58, 0x4d42, 0x9e, 0x1c, 0x7c, 0x8e, 0xe7, 0x95, 0x39, 0xc5);

// 8cae96b7-85b1-4605-b23c-17ff5262b296 
DEFINE_GUID(CLSID_OggMux,
0x8cae96b7, 0x85b1, 0x4605, 0xb2, 0x3c, 0x17, 0xff, 0x52, 0x62, 0xb2, 0x96);

// {AB97AFC3-D08E-4e2d-98E0-AEE6D4634BA4}
DEFINE_GUID(CLSID_OggMuxPropPage,
0xab97afc3, 0xd08e, 0x4e2d, 0x98, 0xe0, 0xae, 0xe6, 0xd4, 0x63, 0x4b, 0xa4);

// {889EF574-0656-4B52-9091-072E52BB1B80}
DEFINE_GUID(CLSID_VorbisEnc,
0x889ef574, 0x0656, 0x4b52, 0x90, 0x91, 0x07, 0x2e, 0x52, 0xbb, 0x1b, 0x80);

// {c5379125-fd36-4277-a7cd-fab469ef3a2f}
DEFINE_GUID(CLSID_VorbisEncPropPage,
0xc5379125, 0xfd36, 0x4277, 0xa7, 0xcd, 0xfa, 0xb4, 0x69, 0xef, 0x3a, 0x2f);

// 02391f44-2767-4e6a-a484-9b47b506f3a4
DEFINE_GUID(CLSID_VorbisDec,
0x02391f44, 0x2767, 0x4e6a, 0xa4, 0x84, 0x9b, 0x47, 0xb5, 0x06, 0xf3, 0xa4);

// 77983549-ffda-4a88-b48f-b924e8d1f01c
DEFINE_GUID(CLSID_OggDSAboutPage,
0x77983549, 0xffda, 0x4a88, 0xb4, 0x8f, 0xb9, 0x24, 0xe8, 0xd1, 0xf0, 0x1c);

// {D2855FA9-61A7-4db0-B979-71F297C17A04}
DEFINE_GUID(MEDIASUBTYPE_Ogg,
0xd2855fa9, 0x61a7, 0x4db0, 0xb9, 0x79, 0x71, 0xf2, 0x97, 0xc1, 0x7a, 0x4);

// cddca2d5-6d75-4f98-840e-737bedd5c63b
DEFINE_GUID(MEDIASUBTYPE_Vorbis,
0xcddca2d5, 0x6d75, 0x4f98, 0x84, 0x0e, 0x73, 0x7b, 0xed, 0xd5, 0xc6, 0x3b);

// 6bddfa7e-9f22-46a9-ab5e-884eff294d9f
DEFINE_GUID(FORMAT_VorbisFormat,
0x6bddfa7e, 0x9f22, 0x46a9, 0xab, 0x5e, 0x88, 0x4e, 0xff, 0x29, 0x4d, 0x9f);

typedef struct tagVORBISFORMAT
{
	WORD nChannels;
	DWORD nSamplesPerSec;
	DWORD nMinBitsPerSec;
	DWORD nAvgBitsPerSec;
	DWORD nMaxBitsPerSec;
	float fQuality;
} VORBISFORMAT, *PVORBISFORMAT, FAR *LPVORBISFORMAT;

// {8D2FD10B-5841-4a6b-8905-588FEC1ADED9}
DEFINE_GUID(MEDIASUBTYPE_Vorbis2, 
0x8d2fd10b, 0x5841, 0x4a6b, 0x89, 0x5, 0x58, 0x8f, 0xec, 0x1a, 0xde, 0xd9);

// {B36E107F-A938-4387-93C7-55E966757473}
DEFINE_GUID(FORMAT_VorbisFormat2, 
0xb36e107f, 0xa938, 0x4387, 0x93, 0xc7, 0x55, 0xe9, 0x66, 0x75, 0x74, 0x73);

typedef struct tagVORBISFORMAT2
{
	DWORD Channels;
	DWORD SamplesPerSec;
	DWORD BitsPerSample;	
	DWORD HeaderSize[3]; // 0: Identification, 1: Comment, 2: Setup
} VORBISFORMAT2, *PVORBISFORMAT2, FAR *LPVORBISFORMAT2;

//
// Matroska
//

// {1AC0BEBD-4D2B-45ad-BCEB-F2C41C5E3788}
DEFINE_GUID(MEDIASUBTYPE_Matroska, 
0x1ac0bebd, 0x4d2b, 0x45ad, 0xbc, 0xeb, 0xf2, 0xc4, 0x1c, 0x5e, 0x37, 0x88);

// {E487EB08-6B26-4be9-9DD3-993434D313FD}
DEFINE_GUID(MEDIATYPE_Subtitle, 
0xe487eb08, 0x6b26, 0x4be9, 0x9d, 0xd3, 0x99, 0x34, 0x34, 0xd3, 0x13, 0xfd);

// {87C0B230-03A8-4fdf-8010-B27A5848200D}
DEFINE_GUID(MEDIASUBTYPE_UTF8, 
0x87c0b230, 0x3a8, 0x4fdf, 0x80, 0x10, 0xb2, 0x7a, 0x58, 0x48, 0x20, 0xd);

// {3020560F-255A-4ddc-806E-6C5CC6DCD70A}
DEFINE_GUID(MEDIASUBTYPE_SSA, 
0x3020560f, 0x255a, 0x4ddc, 0x80, 0x6e, 0x6c, 0x5c, 0xc6, 0xdc, 0xd7, 0xa);

// {326444F7-686F-47ff-A4B2-C8C96307B4C2}
DEFINE_GUID(MEDIASUBTYPE_ASS, 
0x326444f7, 0x686f, 0x47ff, 0xa4, 0xb2, 0xc8, 0xc9, 0x63, 0x7, 0xb4, 0xc2);

// {370689E7-B226-4f67-978D-F10BC1A9C6AE}
DEFINE_GUID(MEDIASUBTYPE_ASS2, 
0x370689e7, 0xb226, 0x4f67, 0x97, 0x8d, 0xf1, 0xb, 0xc1, 0xa9, 0xc6, 0xae);

// {76C421C4-DB89-42ec-936E-A9FBC1794714}
DEFINE_GUID(MEDIASUBTYPE_SSF, 
0x76c421c4, 0xdb89, 0x42ec, 0x93, 0x6e, 0xa9, 0xfb, 0xc1, 0x79, 0x47, 0x14);

// {B753B29A-0A96-45be-985F-68351D9CAB90}
DEFINE_GUID(MEDIASUBTYPE_USF, 
0xb753b29a, 0xa96, 0x45be, 0x98, 0x5f, 0x68, 0x35, 0x1d, 0x9c, 0xab, 0x90);

// {F7239E31-9599-4e43-8DD5-FBAF75CF37F1}
DEFINE_GUID(MEDIASUBTYPE_VOBSUB, 
0xf7239e31, 0x9599, 0x4e43, 0x8d, 0xd5, 0xfb, 0xaf, 0x75, 0xcf, 0x37, 0xf1);

// {A33D2F7D-96BC-4337-B23B-A8B9FBC295E9}
DEFINE_GUID(FORMAT_SubtitleInfo, 
0xa33d2f7d, 0x96bc, 0x4337, 0xb2, 0x3b, 0xa8, 0xb9, 0xfb, 0xc2, 0x95, 0xe9);

#pragma pack(push, 1)
typedef struct {
	DWORD dwOffset;	
	CHAR IsoLang[4]; // three letter lang code + terminating zero
	WCHAR TrackName[256]; // 256 chars ought to be enough for everyone :)
} SUBTITLEINFO;
#pragma pack(pop)

// SUBTITLEINFO structure content starting at dwOffset (also the content of CodecPrivate)
// --------------------------------------------------------------------------------------
//
// Here the text should start with the Byte Order Mark, even though 
// UTF-8 is prefered, it also helps identifying the encoding type.
//
// MEDIASUBTYPE_USF: 
//
// <?xml version="1.0" encoding="UTF-8"?>
// <!-- DOCTYPE USFSubtitles SYSTEM "USFV100.dtd" -->
// <?xml-stylesheet type="text/xsl" href="USFV100.xsl"?>
// 
// <USFSubtitles version="1.0">
// ... every element excluding <subtitles></subtitles> ...
// </USFSubtitles>
//
// MEDIASUBTYPE_SSA/ASS:
//
// The file header and all sub-sections except [Events]
//
// MEDIATYPE_VOBSUB:
//
// TODO
//

// Data description of the media samples (everything is UTF-8 encoded here)
// ------------------------------------------------------------------------
//
// MEDIASUBTYPE_USF:
//
// The text _inside_ the <subtitle>..</subtitle> element. 
//
// Since timing is set on the sample, there is no need to put
// <subtitle start=".." stop=".." duration=".."> into the data.
//
// MEDIASUBTYPE_SSA/ASS:
//
// Comma separated values similar to the "Dialogue: ..." line with these fields:
// ReadOrder, Layer, Style, Name, MarginL, MarginR, MarginV, Effect, Text
//
// With the exception of ReadOrder every field can be found in ASS files. The
// ReadOrder field is needed for the decoder to be able to reorder the streamed 
// samples as they were placed originally in the file.
//
// If the source is only SSA, the Layer field can be left empty.
//
// MEDIATYPE_VOBSUB:
//
// Standard dvd subpic data, without the stream id at the beginning.
//

// Matroska CodecID mappings
// ------------------------
//
// S_TEXT/ASCII	<->	MEDIATYPE_Text		MEDIASUBTYPE_NULL	FORMAT_None
// S_TEXT/UTF8	<->	MEDIATYPE_Subtitle	MEDIASUBTYPE_UTF8	FORMAT_SubtitleInfo
// S_TEXT/SSA	<->	MEDIATYPE_Subtitle	MEDIASUBTYPE_SSA	FORMAT_SubtitleInfo
// S_TEXT/ASS	<->	MEDIATYPE_Subtitle	MEDIASUBTYPE_ASS	FORMAT_SubtitleInfo
// S_TEXT/USF	<->	MEDIATYPE_Subtitle	MEDIASUBTYPE_USF	FORMAT_SubtitleInfo
// S_VOBSUB		<-> MEDIATYPE_Subtitle	MEDIASUBTYPE_VOBSUB	FORMAT_SubtitleInfo
// S_VOBSUB/ZLIB<-> MEDIATYPE_Subtitle	MEDIASUBTYPE_VOBSUB	FORMAT_SubtitleInfo
//


DEFINE_GUID( MEDIATYPE_MPEG2_SECTIONS,
    0x455f176c, 0x4b06, 0x47ce, 0x9a, 0xef, 0x8c, 0xae, 0xf7, 0x3d, 0xf7, 0xb5);

DEFINE_GUID(MEDIASUBTYPE_ATSC_SI,
0xb3c7397c, 0xd303, 0x414d, 0xb3, 0x3c, 0x4e, 0xd2, 0xc9, 0xd2, 0x97, 0x33);

DEFINE_GUID(MEDIASUBTYPE_DVB_SI,
0xe9dd31a3, 0x221d, 0x4adb, 0x85, 0x32, 0x9a, 0xf3, 0x9, 0xc1, 0xa4, 0x8);


// {C892E55B-252D-42b5-A316-D997E7A5D995}
DEFINE_GUID(MEDIASUBTYPE_MPEG2DATA, 
0xc892e55b, 0x252d, 0x42b5, 0xa3, 0x16, 0xd9, 0x97, 0xe7, 0xa5, 0xd9, 0x95);
