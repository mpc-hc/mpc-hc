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
#include "NutSplitter.h"
#include <moreuuids.h>

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_Nut},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CNutSplitterFilter), L"MPC - Nut Splitter", MERIT_NORMAL+1, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CNutSourceFilter), L"MPC - Nut Source", MERIT_NORMAL+1, 0, NULL, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] =
{
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CNutSplitterFilter>, NULL, &sudFilter[0]},
	{sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CNutSourceFilter>, NULL, &sudFilter[1]},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	RegisterSourceFilter(CLSID_AsyncReader, MEDIASUBTYPE_Nut, _T("0,8,,F9526A624E55544D"), _T(".nut"), NULL);

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	UnRegisterSourceFilter(MEDIASUBTYPE_Nut);

	return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CNutSplitterFilter
//

CNutSplitterFilter::CNutSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseSplitterFilter(NAME("CNutSplitterFilter"), pUnk, phr, __uuidof(this))
{
}

HRESULT CNutSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;

	m_pFile.Free();

	m_pFile.Attach(DNew CNutFile(pAsyncReader, hr));
	if(!m_pFile) return E_OUTOFMEMORY;
	if(FAILED(hr)) {m_pFile.Free(); return hr;}

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = 0;

	// pins

	POSITION pos = m_pFile->m_streams.GetHeadPosition();
	while(pos)
	{
		CNutFile::stream_header* sh = m_pFile->m_streams.GetNext(pos);

		CAtlArray<CMediaType> mts;
		CMediaType mt;

		if(sh->stream_class == CNutFile::SC_VIDEO)
		{
			CNutFile::video_stream_header& vsh = sh->vsh;

			mt.majortype = MEDIATYPE_Video;
			mt.subtype = FOURCCMap(*(DWORD*)sh->fourcc.GetData());
			mt.formattype = FORMAT_VideoInfo;

			VIDEOINFOHEADER vih;
			memset(&vih, 0, sizeof(vih));
			vih.dwBitRate = (DWORD)sh->average_bitrate;
			vih.AvgTimePerFrame = 10000000i64 * sh->time_base_nom / sh->time_base_denom;
			vih.bmiHeader.biSize = sizeof(vih.bmiHeader);
			vih.bmiHeader.biCompression = mt.subtype.Data1;
			vih.bmiHeader.biWidth = (LONG)vsh.width;
			vih.bmiHeader.biHeight = (LONG)vsh.height;
			mt.SetFormat((BYTE*)&vih, sizeof(vih));

			mts.Add(mt);

			if(vsh.sample_width && vsh.sample_height)
			{
				VIDEOINFOHEADER2 vih2;
				memset(&vih2, 0, sizeof(vih2));
				vih2.dwBitRate = vih.dwBitRate;
				vih2.AvgTimePerFrame = vih.AvgTimePerFrame;
				vih2.dwPictAspectRatioX = (DWORD)vsh.sample_width;
				vih2.dwPictAspectRatioY = (DWORD)vsh.sample_height;
				vih2.bmiHeader = vih.bmiHeader;
				mt.SetFormat((BYTE*)&vih2, sizeof(vih2));

				mts.InsertAt(0, mt);
			}
		}
		else if(sh->stream_class == CNutFile::SC_AUDIO)
		{
			CNutFile::audio_stream_header& ash = sh->ash;
			UNUSED_ALWAYS(ash);
			// TODO
		}
		else if(sh->stream_class == CNutFile::SC_SUBTITLE)
		{
			// TODO
		}

		if(mts.GetCount() > 0)
		{
			CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CBaseSplitterOutputPin(mts, L"Video", this, this, &hr));
			AddOutputPin((DWORD)sh->stream_id, pPinOut);
		}
	}

	// TODO
	m_rtNewStop = m_rtStop = m_rtDuration = 0;

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CNutSplitterFilter::DemuxInit()
{
	SetThreadName((DWORD)-1, "CNutSplitterFilter");
	if(!m_pFile) return(false);
	m_pFile->Seek(0);
	return(true);
}

void CNutSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	POSITION pos = m_pFile->m_streams.GetHeadPosition();
	while(pos) m_pFile->m_streams.GetNext(pos)->msb_timestamp = 0;

    if(rt <= 0)
	{
		m_pFile->Seek(0);
	}
	else
	{
		m_pFile->Seek(0);
		// TODO
	}
}

bool CNutSplitterFilter::DemuxLoop()
{
	bool fKeyFrame = false;

	while(!CheckRequest(NULL) && m_pFile->GetRemaining())
	{
		CNutFile::frame_header fh;
		fh.checksum_flag = 1;

		UINT64 id = 0;

		if(m_pFile->BitRead(1, true) == 0)
		{
			fh.zero_bit = m_pFile->BitRead(1);
			fh.priority = m_pFile->BitRead(2);
	        fh.checksum_flag = m_pFile->BitRead(1);
			fh.msb_timestamp_flag = m_pFile->BitRead(1);
			fh.subpacket_type = m_pFile->BitRead(2);
			fh.reserved = m_pFile->BitRead(1);
		}
		else
		{
			if((id = m_pFile->BitRead(64)) == NUTK)
			{
				fKeyFrame = true;
				continue;
			}
		}

		CNutFile::packet_header ph;
		m_pFile->Read(ph);

		if(id == 0)
		{
			CNutFile::vint stream_id;
			m_pFile->Read(stream_id);

			CNutFile::stream_header* sh = NULL;

			POSITION pos = m_pFile->m_streams.GetHeadPosition();
			while(pos)
			{
				CNutFile::stream_header* tmp = m_pFile->m_streams.GetNext(pos);
				if(tmp->stream_id == stream_id) {sh = tmp; break;}
			}

			if(sh)
			{
				if(fh.msb_timestamp_flag)
					m_pFile->Read(sh->msb_timestamp);

				CNutFile::svint lsb_timestamp = 0;
				m_pFile->Read(lsb_timestamp);

TRACE(_T("[%I64d]: %I64d:%I64d\n"), stream_id, sh->msb_timestamp, lsb_timestamp);

				CAutoPtr<Packet> p(DNew Packet());
				p->TrackNumber = (DWORD)stream_id;
				p->bSyncPoint = fKeyFrame;
				p->rtStart = 10000i64 * ((sh->msb_timestamp << sh->msb_timestamp_shift) + lsb_timestamp) 
					* sh->time_base_nom / sh->time_base_denom;
				p->rtStop = p->rtStart+1;

				fKeyFrame = false;

				CNutFile::vint len = ph.fptr - (m_pFile->GetPos() - ph.pos);
				if(fh.checksum_flag) len -= 4;

				if(fh.subpacket_type == 1)
				{
					p->SetCount(len);
					m_pFile->ByteRead(p->GetData(), p->GetCount());

					if(FAILED(DeliverPacket(p)))
						break;
				}
				else
				{
					// TODO
/*
					vint subpacket_count;
					Read(subpacket_count);

					if(fh.subpacket_type & 1)
					{
						CArray<vint> keyframe_run;
						keyframe_run.SetSize(subpacket_count);
						for(int i = 0; i < subpacket_count; i++)
							Read(keyframe_run[i]);
					}

					CArray<vint> timestamp_diff;
					timestamp_diff.SetSize(subpacket_count);
					CArray<vint> timestamp_diff_run;
					timestamp_diff_run.SetSize(subpacket_count);
					for(int i = 0; i < subpacket_count; i++)
					{
						Read(timestamp_diff[i]);
						if(timestamp_diff[i] & 1)
							Read(timestamp_diff_run[i]);
					}

					if(fh.subpacket_type & 2)
					{
						CArray<string> subpacket_size_diff;
						for(int i = 0; i < subpacket_count-1; i++)
						{
							Read(subpacket_size_diff[i]);
						}
					}

					for(int i = 0; i < subpacket_count; i++)
					{
					}
*/
				}
			}
		}

		if(fh.checksum_flag)
		{
			m_pFile->Seek(ph.pos + ph.fptr - 4);
			ph.checksum = (UINT32)m_pFile->BitRead(32);
		}

		m_pFile->Seek(ph.pos + ph.fptr);
	}

	return(true);
}

// IMediaSeeking

STDMETHODIMP CNutSplitterFilter::GetDuration(LONGLONG* pDuration)
{
	CheckPointer(pDuration,	E_POINTER);
	*pDuration = m_rtDuration;
	return S_OK;
}

//
// CNutSourceFilter
//

CNutSourceFilter::CNutSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CNutSplitterFilter(pUnk, phr)
{
	m_clsid = __uuidof(this);
	m_pInput.Free();
}
