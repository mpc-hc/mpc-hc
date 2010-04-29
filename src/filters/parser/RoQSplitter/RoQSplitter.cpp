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
#include <initguid.h>
#include "RoQSplitter.h"
#include <moreuuids.h>

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_RoQ},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_MEDIATYPE sudPinTypesIn2[] =
{
	{&MEDIATYPE_Video, &MEDIASUBTYPE_RoQV},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut2[] =
{
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins2[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn2), sudPinTypesIn2},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut2), sudPinTypesOut2}
};

const AMOVIESETUP_MEDIATYPE sudPinTypesIn3[] =
{
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_RoQA},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut3[] =
{
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM},
};

const AMOVIESETUP_PIN sudpPins3[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn3), sudPinTypesIn3},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut3), sudPinTypesOut3}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CRoQSplitterFilter), L"MPC - RoQ Splitter", MERIT_NORMAL+1, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CRoQSourceFilter), L"MPC - RoQ Source", MERIT_NORMAL+1, 0, NULL, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CRoQVideoDecoder), L"MPC - RoQ Video Decoder", MERIT_NORMAL, countof(sudpPins2), sudpPins2, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CRoQAudioDecoder), L"MPC - RoQ Audio Decoder", MERIT_NORMAL, countof(sudpPins3), sudpPins3, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] =
{
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CRoQSplitterFilter>, NULL, &sudFilter[0]},
	{sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CRoQSourceFilter>, NULL, &sudFilter[1]},
    {sudFilter[2].strName, sudFilter[2].clsID, CreateInstance<CRoQVideoDecoder>, NULL, &sudFilter[2]},
    {sudFilter[3].strName, sudFilter[3].clsID, CreateInstance<CRoQAudioDecoder>, NULL, &sudFilter[3]},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	RegisterSourceFilter(CLSID_AsyncReader, MEDIASUBTYPE_RoQ, _T("0,8,,8410FFFFFFFF1E00"), _T(".roq"), NULL);

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	UnRegisterSourceFilter(MEDIASUBTYPE_RoQ);

	return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CRoQSplitterFilter
//

CRoQSplitterFilter::CRoQSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseSplitterFilter(NAME("CRoQSplitterFilter"), pUnk, phr, __uuidof(this))
{
}

HRESULT CRoQSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;

	m_pAsyncReader = pAsyncReader;

	UINT64 hdr = 0, hdrchk = 0x001effffffff1084;
	m_pAsyncReader->SyncRead(0, 8, (BYTE*)&hdr);
	if(hdr != hdrchk) return E_FAIL;

	//

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = 0;

	// pins

	CMediaType mt;
	CAtlArray<CMediaType> mts;

	int iHasVideo = 0;
	int iHasAudio = 0;

	m_index.RemoveAll();
	__int64 audiosamples = 0;

	roq_info ri;
	memset(&ri, 0, sizeof(ri));

	roq_chunk rc;

	UINT64 pos = 8;

	while(S_OK == m_pAsyncReader->SyncRead(pos, sizeof(rc), (BYTE*)&rc))
	{
		pos += sizeof(rc);

		if(rc.id == 0x1001)
		{
			if(S_OK != m_pAsyncReader->SyncRead(pos, sizeof(ri), (BYTE*)&ri) || ri.w == 0 || ri.h == 0)
				break;
		}
		else if(rc.id == 0x1002 || rc.id == 0x1011)
		{
			if(!iHasVideo && ri.w > 0 && ri.h > 0)
			{
				mts.RemoveAll();

				mt.InitMediaType();
				mt.majortype = MEDIATYPE_Video;
				mt.subtype = MEDIASUBTYPE_RoQV;
				mt.formattype = FORMAT_VideoInfo;
				VIDEOINFOHEADER vih;
				memset(&vih, 0, sizeof(vih));
				vih.AvgTimePerFrame = 10000000i64/30;
				vih.bmiHeader.biSize = sizeof(vih.bmiHeader.biSize);
				vih.bmiHeader.biWidth = ri.w;
				vih.bmiHeader.biHeight = ri.h;
				vih.bmiHeader.biCompression = MEDIASUBTYPE_RoQV.Data1;
				mt.SetFormat((BYTE*)&vih, sizeof(vih));
				mt.lSampleSize = 1;

				mts.Add(mt);

				CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CBaseSplitterOutputPin(mts, L"Video", this, this, &hr));
				AddOutputPin(0, pPinOut);
			}

			if(rc.id == 0x1002)
			{
				iHasVideo++;

				index i;
				i.rtv = 10000000i64*m_index.GetCount()/30;
				i.rta = 10000000i64*audiosamples/22050;
				i.fp = pos - sizeof(rc);
				m_index.AddTail(i);
			}
		}
		else if(rc.id == 0x1020 || rc.id == 0x1021)
		{
			if(!iHasAudio)
			{
				mts.RemoveAll();

				mt.InitMediaType();
				mt.majortype = MEDIATYPE_Audio;
				mt.subtype = MEDIASUBTYPE_RoQA;
				mt.formattype = FORMAT_WaveFormatEx;
				WAVEFORMATEX wfe;
				memset(&wfe, 0, sizeof(wfe));
				wfe.wFormatTag = (WORD)WAVE_FORMAT_RoQA; // cut into half, hehe, like anyone would care
				wfe.nChannels = (rc.id&1)+1;
				wfe.nSamplesPerSec = 22050;
				wfe.wBitsPerSample = 16;
				mt.SetFormat((BYTE*)&wfe, sizeof(wfe));
				mt.lSampleSize = 1;
				mts.Add(mt);

				CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CBaseSplitterOutputPin(mts, L"Audio", this, this, &hr));
				AddOutputPin(1, pPinOut);
			}

			iHasAudio++;

			audiosamples += rc.size / ((rc.id&1)+1);
		}

		pos += rc.size;
	}

	//

	m_rtNewStop = m_rtStop = m_rtDuration = 10000000i64*iHasVideo/30;

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CRoQSplitterFilter::DemuxInit()
{
	SetThreadName(-1, "CRoQSplitterFilter");
	m_indexpos = m_index.GetHeadPosition();

	return(true);
}

void CRoQSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	if(rt <= 0)
	{
		m_indexpos = m_index.GetHeadPosition();
	}
	else
	{
		m_indexpos = m_index.GetTailPosition();
		while(m_indexpos && m_index.GetPrev(m_indexpos).rtv > rt);
	}
}

bool CRoQSplitterFilter::DemuxLoop()
{
	if(!m_indexpos) return(true);

	index& i = m_index.GetAt(m_indexpos);

	REFERENCE_TIME rtVideo = i.rtv, rtAudio = i.rta;

	HRESULT hr = S_OK;

	UINT64 pos = i.fp;

	roq_chunk rc;
	while(S_OK == (hr = m_pAsyncReader->SyncRead(pos, sizeof(rc), (BYTE*)&rc))
		&& !CheckRequest(NULL))
	{
		pos += sizeof(rc);

		CAutoPtr<Packet> p(DNew Packet());

		if(rc.id == 0x1002 || rc.id == 0x1011 || rc.id == 0x1020 || rc.id == 0x1021)
		{
			p->SetCount(sizeof(rc) + rc.size);
			memcpy(p->GetData(), &rc, sizeof(rc));
			if(S_OK != (hr = m_pAsyncReader->SyncRead(pos, rc.size, p->GetData() + sizeof(rc))))
				break;
		}

		if(rc.id == 0x1002 || rc.id == 0x1011)
		{
			p->TrackNumber = 0;
			p->bSyncPoint = rtVideo == 0;
			p->rtStart = rtVideo;
			p->rtStop = rtVideo += (rc.id == 0x1011 ? 10000000i64/30 : 0);
			TRACE(_T("v: %I64d - %I64d (%d)\n"), p->rtStart/10000, p->rtStop/10000, p->GetCount());
		}
		else if(rc.id == 0x1020 || rc.id == 0x1021)
		{
			int nChannels = (rc.id&1)+1;

			p->TrackNumber = 1;
			p->bSyncPoint = TRUE;
			p->rtStart = rtAudio;
			p->rtStop = rtAudio += 10000000i64*rc.size/(nChannels*22050);
			TRACE(_T("a: %I64d - %I64d (%d)\n"), p->rtStart/10000, p->rtStop/10000, p->GetCount());
		}

		if(rc.id == 0x1002 || rc.id == 0x1011 || rc.id == 0x1020 || rc.id == 0x1021)
		{
			hr = DeliverPacket(p);
		}

		pos += rc.size;
	}

	return(true);
}

//
// CRoQSourceFilter
//

CRoQSourceFilter::CRoQSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CRoQSplitterFilter(pUnk, phr)
{
	m_clsid = __uuidof(this);
	m_pInput.Free();
}

//
// CRoQVideoDecoder
//

CRoQVideoDecoder::CRoQVideoDecoder(LPUNKNOWN lpunk, HRESULT* phr)
	: CTransformFilter(NAME("CRoQVideoDecoder"), lpunk, __uuidof(this))
{
	if(phr) *phr = S_OK;
}

CRoQVideoDecoder::~CRoQVideoDecoder()
{
}

void CRoQVideoDecoder::apply_vector_2x2(int x, int y, roq_cell* cell)
{
	unsigned char* yptr;
	yptr = m_y[0] + (y * m_pitch) + x;
	*yptr++ = cell->y0;
	*yptr++ = cell->y1;
	yptr += (m_pitch - 2);
	*yptr++ = cell->y2;
	*yptr++ = cell->y3;
	m_u[0][(y/2) * (m_pitch/2) + x/2] = cell->u;
	m_v[0][(y/2) * (m_pitch/2) + x/2] = cell->v;
}

void CRoQVideoDecoder::apply_vector_4x4(int x, int y, roq_cell* cell)
{
	unsigned long row_inc, c_row_inc;
	register unsigned char y0, y1, u, v;
	unsigned char *yptr, *uptr, *vptr;

	yptr = m_y[0] + (y * m_pitch) + x;
	uptr = m_u[0] + (y/2) * (m_pitch/2) + x/2;
	vptr = m_v[0] + (y/2) * (m_pitch/2) + x/2;

	row_inc = m_pitch - 4;
	c_row_inc = (m_pitch/2) - 2;
	*yptr++ = y0 = cell->y0; *uptr++ = u = cell->u; *vptr++ = v = cell->v;
	*yptr++ = y0;
	*yptr++ = y1 = cell->y1; *uptr++ = u; *vptr++ = v;
	*yptr++ = y1;

	yptr += row_inc;

	*yptr++ = y0;
	*yptr++ = y0;
	*yptr++ = y1;
	*yptr++ = y1;

	yptr += row_inc; uptr += c_row_inc; vptr += c_row_inc;

	*yptr++ = y0 = cell->y2; *uptr++ = u; *vptr++ = v;
	*yptr++ = y0;
	*yptr++ = y1 = cell->y3; *uptr++ = u; *vptr++ = v;
	*yptr++ = y1;

	yptr += row_inc;

	*yptr++ = y0;
	*yptr++ = y0;
	*yptr++ = y1;
	*yptr++ = y1;
}

void CRoQVideoDecoder::apply_motion_4x4(int x, int y, unsigned char mv, char mean_x, char mean_y)
{
	int i, mx, my;
	unsigned char *pa, *pb;

	mx = x + 8 - (mv >> 4) - mean_x;
	my = y + 8 - (mv & 0xf) - mean_y;

	pa = m_y[0] + (y * m_pitch) + x;
	pb = m_y[1] + (my * m_pitch) + mx;
	for(i = 0; i < 4; i++)
	{
		pa[0] = pb[0];
		pa[1] = pb[1];
		pa[2] = pb[2];
		pa[3] = pb[3];
		pa += m_pitch;
		pb += m_pitch;
	}

	pa = m_u[0] + (y/2) * (m_pitch/2) + x/2;
	pb = m_u[1] + (my/2) * (m_pitch/2) + (mx + 1)/2;
	for(i = 0; i < 2; i++)
	{
		pa[0] = pb[0];
		pa[1] = pb[1];
		pa += m_pitch/2;
		pb += m_pitch/2;
	}

	pa = m_v[0] + (y/2) * (m_pitch/2) + x/2;
	pb = m_v[1] + (my/2) * (m_pitch/2) + (mx + 1)/2;
	for(i = 0; i < 2; i++)
	{
		pa[0] = pb[0];
		pa[1] = pb[1];
		pa += m_pitch/2;
		pb += m_pitch/2;
	}
}

void CRoQVideoDecoder::apply_motion_8x8(int x, int y, unsigned char mv, char mean_x, char mean_y)
{
	int mx, my, i;
	unsigned char *pa, *pb;

	mx = x + 8 - (mv >> 4) - mean_x;
	my = y + 8 - (mv & 0xf) - mean_y;

	pa = m_y[0] + (y * m_pitch) + x;
	pb = m_y[1] + (my * m_pitch) + mx;
	for(i = 0; i < 8; i++)
	{
		pa[0] = pb[0];
		pa[1] = pb[1];
		pa[2] = pb[2];
		pa[3] = pb[3];
		pa[4] = pb[4];
		pa[5] = pb[5];
		pa[6] = pb[6];
		pa[7] = pb[7];
		pa += m_pitch;
		pb += m_pitch;
	}

	pa = m_u[0] + (y/2) * (m_pitch/2) + x/2;
	pb = m_u[1] + (my/2) * (m_pitch/2) + (mx + 1)/2;
	for(i = 0; i < 4; i++)
	{
		pa[0] = pb[0];
		pa[1] = pb[1];
		pa[2] = pb[2];
		pa[3] = pb[3];
		pa += m_pitch/2;
		pb += m_pitch/2;
	}

	pa = m_v[0] + (y/2) * (m_pitch/2) + x/2;
	pb = m_v[1] + (my/2) * (m_pitch/2) + (mx + 1)/2;
	for(i = 0; i < 4; i++)
	{
		pa[0] = pb[0];
		pa[1] = pb[1];
		pa[2] = pb[2];
		pa[3] = pb[3];
		pa += m_pitch/2;
		pb += m_pitch/2;
	}
}

HRESULT CRoQVideoDecoder::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	CAutoLock cAutoLock(&m_csReceive);

	m_rtStart = tStart;

	BITMAPINFOHEADER bih;
	ExtractBIH(&m_pInput->CurrentMediaType(), &bih);

	int size = bih.biWidth*bih.biHeight;

	memset(m_y[0], 0, size);
	memset(m_u[0], 0x80, size/2);
	memset(m_y[1], 0, size);
	memset(m_u[1], 0x80, size/2);

	return __super::NewSegment(tStart, tStop, dRate);
}

HRESULT CRoQVideoDecoder::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
	CAutoLock cAutoLock(&m_csReceive);

	HRESULT hr;

	AM_MEDIA_TYPE* pmt;
	if(SUCCEEDED(pOut->GetMediaType(&pmt)) && pmt)
	{
		CMediaType mt(*pmt);
		m_pOutput->SetMediaType(&mt);
		DeleteMediaType(pmt);
	}

	BYTE* pDataIn = NULL;
	if(FAILED(hr = pIn->GetPointer(&pDataIn))) return hr;

	long len = pIn->GetActualDataLength();
	if(len <= 0) return S_OK; // nothing to do

	REFERENCE_TIME rtStart = 0, rtStop = 0;
	pIn->GetTime(&rtStart, &rtStop);

	if(pIn->IsPreroll() == S_OK || rtStart < 0)
		return S_OK;

	BYTE* pDataOut = NULL;
	if(FAILED(hr = pOut->GetPointer(&pDataOut)))
		return hr;

	BITMAPINFOHEADER bih;
	ExtractBIH(&m_pInput->CurrentMediaType(), &bih);

	int w = bih.biWidth, h = bih.biHeight;

	// TODO: decode picture into m_pI420

	roq_chunk* rc = (roq_chunk*)pDataIn;

	pDataIn += sizeof(roq_chunk);

	if(rc->id == 0x1002)
	{
		DWORD nv1 = rc->arg>>8;
		if(nv1 == 0) nv1 = 256;

		DWORD nv2 = rc->arg&0xff;
		if(nv2 == 0 && nv1 * 6 < rc->size) nv2 = 256;

		memcpy(m_cells, pDataIn, sizeof(m_cells[0])*nv1);
		pDataIn += sizeof(m_cells[0])*nv1;

		for(int i = 0; i < (int)nv2; i++)
			for(int j = 0; j < 4; j++)
				m_qcells[i].idx[j] = &m_cells[*pDataIn++];

		return S_FALSE;
	}
	else if(rc->id == 0x1011)
	{
		int bpos = 0, xpos = 0, ypos = 0;
		int vqflg = 0, vqflg_pos = -1, vqid;
		roq_qcell* qcell = NULL;

		BYTE* buf = pDataIn;

		while(bpos < (int)rc->size && ypos < h)
		{
			for(int yp = ypos; yp < ypos + 16; yp += 8)
			{
				for(int xp = xpos; xp < xpos + 16; xp += 8)
				{
					if(vqflg_pos < 0)
					{
						vqflg = buf[bpos++];
						vqflg |= buf[bpos++]<<8;
						vqflg_pos = 7;
					}

					vqid = (vqflg >> (vqflg_pos * 2)) & 3;
					vqflg_pos--;

					switch(vqid)
					{
					case 0:
						break;
					case 1:
						apply_motion_8x8(xp, yp, buf[bpos++], rc->arg >> 8, rc->arg & 0xff);
						break;
					case 2:
						qcell = m_qcells + buf[bpos++];
						apply_vector_4x4(xp, yp, qcell->idx[0]);
						apply_vector_4x4(xp+4, yp, qcell->idx[1]);
						apply_vector_4x4(xp, yp+4, qcell->idx[2]);
						apply_vector_4x4(xp+4, yp+4, qcell->idx[3]);
						break;
					case 3:
						for(int k = 0; k < 4; k++)
						{
							int x = xp, y = yp;
							if(k&1) x += 4;
							if(k&2) y += 4;

							if(vqflg_pos < 0)
							{
								vqflg = buf[bpos++];
								vqflg |= buf[bpos++]<<8;
								vqflg_pos = 7;
							}

							vqid = (vqflg >> (vqflg_pos * 2)) & 3;
							vqflg_pos--;

							switch(vqid)
							{
							case 0:
								break;
							case 1:
								apply_motion_4x4(x, y, buf[bpos++], rc->arg >> 8, rc->arg & 0xff);
								break;
							case 2:
								qcell = m_qcells + buf[bpos++];
								apply_vector_2x2(x, y, qcell->idx[0]);
								apply_vector_2x2(x+2, y, qcell->idx[1]);
								apply_vector_2x2(x, y+2, qcell->idx[2]);
								apply_vector_2x2(x+2, y+2, qcell->idx[3]);
								break;
							case 3:
								apply_vector_2x2(x, y, &m_cells[buf[bpos++]]);
								apply_vector_2x2(x+2, y, &m_cells[buf[bpos++]]);
								apply_vector_2x2(x, y+2, &m_cells[buf[bpos++]]);
								apply_vector_2x2(x+2, y+2, &m_cells[buf[bpos++]]);
								break;
							}
						}
						break;
					}
				}
			}

			xpos += 16;
			if(xpos >= w) {xpos -= w; ypos += 16;}
		}

		if(m_rtStart+rtStart == 0)
		{
			memcpy(m_y[1], m_y[0], w*h*3/2);
		}
		else
		{
			BYTE* tmp;
			tmp = m_y[0]; m_y[0] = m_y[1]; m_y[1] = tmp;
			tmp = m_u[0]; m_u[0] = m_u[1]; m_u[1] = tmp;
			tmp = m_v[0]; m_v[0] = m_v[1]; m_v[1] = tmp;
		}
	}
	else
	{
		return E_UNEXPECTED;
	}

	if(rtStart < 0)
		return S_FALSE;

	Copy(pDataOut, m_y[1], w, h);

	pOut->SetTime(&rtStart, &rtStop);

	return S_OK;
}

void CRoQVideoDecoder::Copy(BYTE* pOut, BYTE* pIn, DWORD w, DWORD h)
{
	BITMAPINFOHEADER bihOut;
	ExtractBIH(&m_pOutput->CurrentMediaType(), &bihOut);

	int pitchIn = w;
	int pitchInUV = pitchIn>>1;
	BYTE* pInU = pIn + pitchIn*h;
	BYTE* pInV = pInU + pitchInUV*h/2;

	if(bihOut.biCompression == '2YUY')
	{
		BitBltFromI420ToYUY2(w, h, pOut, bihOut.biWidth*2, pIn, pInU, pInV, pitchIn);
	}
	else if(bihOut.biCompression == 'I420' || bihOut.biCompression == 'VUYI')
	{
		BitBltFromI420ToI420(w, h, pOut, pOut + bihOut.biWidth*h, pOut + bihOut.biWidth*h*5/4, bihOut.biWidth, pIn, pInU, pInV, pitchIn);
	}
	else if(bihOut.biCompression == '21VY')
	{
		BitBltFromI420ToI420(w, h, pOut, pOut + bihOut.biWidth*h*5/4, pOut + bihOut.biWidth*h, bihOut.biWidth, pIn, pInU, pInV, pitchIn);
	}
	else if(bihOut.biCompression == BI_RGB || bihOut.biCompression == BI_BITFIELDS)
	{
		int pitchOut = bihOut.biWidth*bihOut.biBitCount>>3;

		if(bihOut.biHeight > 0)
		{
			pOut += pitchOut*(h-1);
			pitchOut = -pitchOut;
		}

		if(!BitBltFromI420ToRGB(w, h, pOut, pitchOut, bihOut.biBitCount, pIn, pInU, pInV, pitchIn))
		{
			for(DWORD y = 0; y < h; y++, pIn += pitchIn, pOut += pitchOut)
				memset(pOut, 0, pitchOut);
		}
	}
}

HRESULT CRoQVideoDecoder::CheckInputType(const CMediaType* mtIn)
{
	return mtIn->majortype == MEDIATYPE_Video && mtIn->subtype == MEDIASUBTYPE_RoQV
		? S_OK
		: VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CRoQVideoDecoder::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	if(m_pOutput && m_pOutput->IsConnected())
	{
		BITMAPINFOHEADER bih1, bih2;
		if(ExtractBIH(mtOut, &bih1) && ExtractBIH(&m_pOutput->CurrentMediaType(), &bih2)
		&& abs(bih1.biHeight) != abs(bih2.biHeight))
			return VFW_E_TYPE_NOT_ACCEPTED;
	}

	return SUCCEEDED(CheckInputType(mtIn))
		&& mtOut->majortype == MEDIATYPE_Video && (mtOut->subtype == MEDIASUBTYPE_YUY2
												|| mtOut->subtype == MEDIASUBTYPE_YV12
												|| mtOut->subtype == MEDIASUBTYPE_I420
												|| mtOut->subtype == MEDIASUBTYPE_IYUV
												|| mtOut->subtype == MEDIASUBTYPE_ARGB32
												|| mtOut->subtype == MEDIASUBTYPE_RGB32
												|| mtOut->subtype == MEDIASUBTYPE_RGB24
												|| mtOut->subtype == MEDIASUBTYPE_RGB565
												|| mtOut->subtype == MEDIASUBTYPE_RGB555)
		? S_OK
		: VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CRoQVideoDecoder::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
	if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

	BITMAPINFOHEADER bih;
	ExtractBIH(&m_pOutput->CurrentMediaType(), &bih);

	pProperties->cBuffers = 1;
	pProperties->cbBuffer = bih.biSizeImage;
	pProperties->cbAlign = 1;
	pProperties->cbPrefix = 0;

	HRESULT hr;
	ALLOCATOR_PROPERTIES Actual;
    if(FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) 
		return hr;

    return(pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
		? E_FAIL
		: NOERROR);
}

HRESULT CRoQVideoDecoder::GetMediaType(int iPosition, CMediaType* pmt)
{
    if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

	struct {const GUID* subtype; WORD biPlanes, biBitCount; DWORD biCompression;} fmts[] =
	{
		{&MEDIASUBTYPE_YV12, 3, 12, '21VY'},
		{&MEDIASUBTYPE_I420, 3, 12, '024I'},
		{&MEDIASUBTYPE_IYUV, 3, 12, 'VUYI'},
		{&MEDIASUBTYPE_YUY2, 1, 16, '2YUY'},
		{&MEDIASUBTYPE_ARGB32, 1, 32, BI_RGB},
		{&MEDIASUBTYPE_RGB32, 1, 32, BI_RGB},
		{&MEDIASUBTYPE_RGB24, 1, 24, BI_RGB},
		{&MEDIASUBTYPE_RGB565, 1, 16, BI_RGB},
		{&MEDIASUBTYPE_RGB555, 1, 16, BI_RGB},
		{&MEDIASUBTYPE_ARGB32, 1, 32, BI_BITFIELDS},
		{&MEDIASUBTYPE_RGB32, 1, 32, BI_BITFIELDS},
		{&MEDIASUBTYPE_RGB24, 1, 24, BI_BITFIELDS},
		{&MEDIASUBTYPE_RGB565, 1, 16, BI_BITFIELDS},
		{&MEDIASUBTYPE_RGB555, 1, 16, BI_BITFIELDS},
	};

	if(iPosition < 0) return E_INVALIDARG;
	if(iPosition >= countof(fmts)) return VFW_S_NO_MORE_ITEMS;

	BITMAPINFOHEADER bih;
	ExtractBIH(&m_pInput->CurrentMediaType(), &bih);

	pmt->majortype = MEDIATYPE_Video;
	pmt->subtype = *fmts[iPosition].subtype;
	pmt->formattype = FORMAT_VideoInfo;

	BITMAPINFOHEADER bihOut;
	memset(&bihOut, 0, sizeof(bihOut));
	bihOut.biSize = sizeof(bihOut);
	bihOut.biWidth = bih.biWidth;
	bihOut.biHeight = bih.biHeight;
	bihOut.biPlanes = fmts[iPosition].biPlanes;
	bihOut.biBitCount = fmts[iPosition].biBitCount;
	bihOut.biCompression = fmts[iPosition].biCompression;
	bihOut.biSizeImage = bih.biWidth*bih.biHeight*bihOut.biBitCount>>3;

	VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
	memset(vih, 0, sizeof(VIDEOINFOHEADER));
	vih->bmiHeader = bihOut;

	CorrectMediaType(pmt);

	return S_OK;
}

HRESULT CRoQVideoDecoder::StartStreaming()
{
	BITMAPINFOHEADER bih;
	ExtractBIH(&m_pInput->CurrentMediaType(), &bih);

	int size = bih.biWidth*bih.biHeight;

	m_y[0] = DNew BYTE[size*3/2];
	m_u[0] = m_y[0] + size;
	m_v[0] = m_y[0] + size*5/4;
	m_y[1] = DNew BYTE[size*3/2];
	m_u[1] = m_y[1] + size;
	m_v[1] = m_y[1] + size*5/4;

	m_pitch = bih.biWidth;

	return __super::StartStreaming();
}

HRESULT CRoQVideoDecoder::StopStreaming()
{
	delete [] m_y[0]; m_y[0] = NULL;
	delete [] m_y[1]; m_y[1] = NULL;

	return __super::StopStreaming();
}

//
// CRealAudioDecoder
//

CRoQAudioDecoder::CRoQAudioDecoder(LPUNKNOWN lpunk, HRESULT* phr)
	: CTransformFilter(NAME("CRoQAudioDecoder"), lpunk, __uuidof(this))
{
	if(phr) *phr = S_OK;
}

CRoQAudioDecoder::~CRoQAudioDecoder()
{
}

HRESULT CRoQAudioDecoder::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
	HRESULT hr;

	AM_MEDIA_TYPE* pmt;
	if(SUCCEEDED(pOut->GetMediaType(&pmt)) && pmt)
	{
		CMediaType mt(*pmt);
		m_pOutput->SetMediaType(&mt);
		DeleteMediaType(pmt);
	}

	WAVEFORMATEX* pwfe = (WAVEFORMATEX*)m_pOutput->CurrentMediaType().Format();

	BYTE* pDataIn = NULL;
	if(FAILED(hr = pIn->GetPointer(&pDataIn)))
		return hr;

	long len = pIn->GetActualDataLength();
	if(len <= 0) return S_OK;

	REFERENCE_TIME rtStart, rtStop;
	pIn->GetTime(&rtStart, &rtStop);

	if(pIn->IsPreroll() == S_OK || rtStart < 0)
		return S_OK;

	BYTE* pDataOut = NULL;
	if(FAILED(hr = pOut->GetPointer(&pDataOut)))
		return hr;

	long size = pOut->GetSize();
	if(size <= 0)
		return E_FAIL;

	roq_chunk* rc = (roq_chunk*)pDataIn;

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pOutput->CurrentMediaType().Format();

	if(wfe->nChannels == 1)
	{
		int mono = (short)rc->arg;
		unsigned char* src = pDataIn + sizeof(roq_chunk);
		short* dst = (short*)pDataOut;
		for(int i = sizeof(roq_chunk); i < len; i++, src++, dst++)
		{
			short diff = (*src&0x7f)*(*src&0x7f);
			if(*src&0x80) diff = -diff;
			mono += diff;
			*dst = (short)mono;
		}
	}
	else if(wfe->nChannels == 2)
	{
		int left = (char)(rc->arg>>8)<<8;
		int right = (char)(rc->arg)<<8;
		unsigned char* src = pDataIn + sizeof(roq_chunk);
		short* dst = (short*)pDataOut;
		for(int i = sizeof(roq_chunk); i < len; i+=2, src++, dst++)
		{
			short diff = (*src&0x7f)*(*src&0x7f);
			if(*src&0x80) diff = -diff;
			ASSERT((int)left + diff <= SHRT_MAX && (int)left + diff >= SHRT_MIN);
			left += diff;
			*dst = (short)left;

			src++; dst++;

			diff = (*src&0x7f)*(*src&0x7f);
			if(*src&0x80) diff = -diff;
			ASSERT((int)right + diff <= SHRT_MAX && (int)right + diff >= SHRT_MIN);
			right += diff;
			*dst = (short)right;
		}
	}
	else
	{
		return E_UNEXPECTED;
	}

	pOut->SetTime(&rtStart, &rtStop);

	pOut->SetActualDataLength(2*(len-sizeof(roq_chunk)));

	return S_OK;
}

HRESULT CRoQAudioDecoder::CheckInputType(const CMediaType* mtIn)
{
	return mtIn->majortype == MEDIATYPE_Audio && mtIn->subtype == MEDIASUBTYPE_RoQA
		? S_OK
		: VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CRoQAudioDecoder::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	return SUCCEEDED(CheckInputType(mtIn))
		&& mtOut->majortype == MEDIATYPE_Audio && mtOut->subtype == MEDIASUBTYPE_PCM
		? S_OK
		: VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CRoQAudioDecoder::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
	if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

	CComPtr<IMemAllocator> pAllocatorIn;
	m_pInput->GetAllocator(&pAllocatorIn);
	if(!pAllocatorIn) return E_UNEXPECTED;

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pOutput->CurrentMediaType().Format();

	// ok, maybe this is too much...
	pProperties->cBuffers = 8;
	pProperties->cbBuffer = wfe->nChannels*wfe->nSamplesPerSec*wfe->wBitsPerSample>>3; // nAvgBytesPerSec;
	pProperties->cbAlign = 1;
	pProperties->cbPrefix = 0;

	HRESULT hr;
	ALLOCATOR_PROPERTIES Actual;
    if(FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) 
		return hr;

    return pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
		? E_FAIL
		: NOERROR;
}

HRESULT CRoQAudioDecoder::GetMediaType(int iPosition, CMediaType* pmt)
{
    if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

	if(iPosition < 0) return E_INVALIDARG;
	if(iPosition > 0) return VFW_S_NO_MORE_ITEMS;

	*pmt = m_pInput->CurrentMediaType();
	pmt->subtype = MEDIASUBTYPE_PCM;

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)pmt->ReallocFormatBuffer(sizeof(WAVEFORMATEX));
	wfe->cbSize = 0;
	wfe->wFormatTag = WAVE_FORMAT_PCM;
	wfe->nBlockAlign = wfe->nChannels*wfe->wBitsPerSample>>3;
	wfe->nAvgBytesPerSec = wfe->nSamplesPerSec*wfe->nBlockAlign;

	return S_OK;
}
