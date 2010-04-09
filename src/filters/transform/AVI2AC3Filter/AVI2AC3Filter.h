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

/*  AC3 audio

    wFormatTag          WAVE_FORMAT_DOLBY_AC3
    nChannels           1 -6 channels valid
    nSamplesPerSec      48000, 44100, 32000
    nAvgByesPerSec      4000 to 80000
    nBlockAlign         128 - 3840
    wBitsPerSample      Up to 24 bits - (in the original)

*/

typedef struct tagDOLBYAC3WAVEFORMAT
{
    WAVEFORMATEX     wfx;
    BYTE             bBigEndian;       // TRUE = Big Endian, FALSE little endian
    BYTE             bsid;
    BYTE             lfeon;
    BYTE             copyrightb;
    BYTE             nAuxBitsCode;  //  Aux bits per frame
} DOLBYAC3WAVEFORMAT;

//
// CAVI2AC3Filter
//

[uuid("93230DD0-7B3C-4efb-AFBB-DC380FEC9E6B")]
class CAVI2AC3Filter : public CTransformFilter
{
	bool CheckAC3(const CMediaType* pmt);
	bool CheckDTS(const CMediaType* pmt);
	bool CheckWAVEAC3(const CMediaType* pmt);
	bool CheckWAVEDTS(const CMediaType* pmt);

public:
	CAVI2AC3Filter(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CAVI2AC3Filter();

	HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut);
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);
};
