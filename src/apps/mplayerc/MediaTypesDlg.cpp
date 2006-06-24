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

// MediaTypesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "MediaTypesDlg.h"
#include "..\..\DSUtil\DSUtil.h"
#include "..\..\..\include\moreuuids.h"

#pragma pack(push, 1)
typedef struct
{
	WAVEFORMATEX Format;
	BYTE bBigEndian;
	BYTE bsid;
	BYTE lfeon;
	BYTE copyrightb;
	BYTE nAuxBitsCode;  //  Aux bits per frame
} DOLBYAC3WAVEFORMAT;
#pragma pack(pop)

// CMediaTypesDlg dialog

//IMPLEMENT_DYNAMIC(CMediaTypesDlg, CResizableDialog)
CMediaTypesDlg::CMediaTypesDlg(IGraphBuilderDeadEnd* pGBDE, CWnd* pParent /*=NULL*/)
	: CResizableDialog(CMediaTypesDlg::IDD, pParent)
	, m_pGBDE(pGBDE)
{
	m_subtype = GUID_NULL;
	m_type = UNKNOWN;
}

CMediaTypesDlg::~CMediaTypesDlg()
{
}

void CMediaTypesDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_pins);
	DDX_Control(pDX, IDC_EDIT1, m_report);
}

void CMediaTypesDlg::AddLine(CString str)
{
	str.Replace(_T("\n"), _T("\r\n"));
	int len = m_report.GetWindowTextLength();
	m_report.SetSel(len, len, TRUE);
	m_report.ReplaceSel(str);
}

void CMediaTypesDlg::AddMediaType(AM_MEDIA_TYPE* pmt)
{
	CString major = CStringFromGUID(pmt->majortype);
	CString sub = CStringFromGUID(pmt->subtype);
	CString format = CStringFromGUID(pmt->formattype);

	m_subtype = pmt->subtype;
	if(pmt->majortype == MEDIATYPE_Video) m_type = VIDEO;
	else if(pmt->majortype == MEDIATYPE_Audio) m_type = AUDIO;
	else m_type = UNKNOWN;

	CString str;

	AddLine(_T("AM_MEDIA_TYPE: (") + CMediaTypeEx(*pmt).ToString() + _T(")\n"));
	str.Format(_T("majortype: %s %s\n"), CString(GuidNames[pmt->majortype]), major);
	AddLine(str);
	str.Format(_T("subtype: %s %s\n"), CString(GuidNames[pmt->subtype]), sub);
	AddLine(str);
	str.Format(_T("formattype: %s %s\n"), CString(GuidNames[pmt->formattype]), format);
	AddLine(str);
	str.Format(_T("bFixedSizeSamples: %d\n"), pmt->bFixedSizeSamples);
	AddLine(str);
	str.Format(_T("bTemporalCompression: %d\n"), pmt->bTemporalCompression);
	AddLine(str);
	str.Format(_T("lSampleSize: %d\n"), pmt->lSampleSize);
	AddLine(str);
	str.Format(_T("cbFormat: %d\n"), pmt->cbFormat);
	AddLine(str);

	AddLine();

	if(pmt->formattype == FORMAT_VideoInfo || pmt->formattype == FORMAT_VideoInfo2)
	{
		VIDEOINFOHEADER& vih = *((VIDEOINFOHEADER*)pmt->pbFormat);
		BITMAPINFOHEADER* bih = &vih.bmiHeader;

		AddLine(_T("VIDEOINFOHEADER:\n"));
		str.Format(_T("rcSource: (%d,%d)-(%d,%d)\n"), vih.rcSource);
		AddLine(str);
		str.Format(_T("rcTarget: (%d,%d)-(%d,%d)\n"), vih.rcTarget);
		AddLine(str);
		str.Format(_T("dwBitRate: %d\n"), vih.dwBitRate);
		AddLine(str);
		str.Format(_T("dwBitErrorRate: %d\n"), vih.dwBitErrorRate);
		AddLine(str);
		str.Format(_T("AvgTimePerFrame: %I64d\n"), vih.AvgTimePerFrame);
		AddLine(str);

		if(pmt->formattype == FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2& vih = *((VIDEOINFOHEADER2*)pmt->pbFormat);
			bih = &vih.bmiHeader;

			AddLine(_T("VIDEOINFOHEADER2:\n"));
			str.Format(_T("dwInterlaceFlags: 0x%08x\n"), vih.dwInterlaceFlags);
			AddLine(str);
			str.Format(_T("dwCopyProtectFlags: 0x%08x\n"), vih.dwCopyProtectFlags);
			AddLine(str);
			str.Format(_T("dwPictAspectRatioX: %d\n"), vih.dwPictAspectRatioX);
			AddLine(str);
			str.Format(_T("dwPictAspectRatioY: %d\n"), vih.dwPictAspectRatioY);
			AddLine(str);
			str.Format(_T("dwControlFlags: 0x%08x\n"), vih.dwControlFlags);
			AddLine(str);
			str.Format(_T("dwReserved2: 0x%08x\n"), vih.dwReserved2);
			AddLine(str);
		}

		AddLine();

		AddLine(_T("BITMAPINFOHEADER:\n"));
		str.Format(_T("biSize: %d\n"), bih->biSize);
		AddLine(str);
		str.Format(_T("biWidth: %d\n"), bih->biWidth);
		AddLine(str);
		str.Format(_T("biHeight: %d\n"), bih->biHeight);
		AddLine(str);
		str.Format(_T("biPlanes: %d\n"), bih->biPlanes);
		AddLine(str);
		str.Format(_T("biBitCount: %d\n"), bih->biBitCount);
		AddLine(str);
		if(bih->biCompression < 256) str.Format(_T("biCompression: %d\n"), bih->biCompression);
		else str.Format(_T("biCompression: %4.4hs\n"), &bih->biCompression);
		AddLine(str);
		str.Format(_T("biSizeImage: %d\n"), bih->biSizeImage);
		AddLine(str);
		str.Format(_T("biXPelsPerMeter: %d\n"), bih->biXPelsPerMeter);
		AddLine(str);
		str.Format(_T("biYPelsPerMeter: %d\n"), bih->biYPelsPerMeter);
		AddLine(str);
		str.Format(_T("biYPelsPerMeter: %d\n"), bih->biYPelsPerMeter);
		AddLine(str);
		str.Format(_T("biClrUsed: %d\n"), bih->biClrUsed);
		AddLine(str);
		str.Format(_T("biClrImportant: %d\n"), bih->biClrImportant);
		AddLine(str);

		AddLine();
    }
	else if(pmt->formattype == FORMAT_WaveFormatEx)
	{
		WAVEFORMATEX& wfe = *((WAVEFORMATEX*)pmt->pbFormat);

		AddLine(_T("WAVEFORMATEX:\n"));
		str.Format(_T("wFormatTag: 0x%04x\n"), wfe.wFormatTag);
		AddLine(str);
		str.Format(_T("nChannels: %d\n"), wfe.nChannels);
		AddLine(str);
		str.Format(_T("nSamplesPerSec: %d\n"), wfe.nSamplesPerSec);
		AddLine(str);
		str.Format(_T("nAvgBytesPerSec: %d\n"), wfe.nAvgBytesPerSec);
		AddLine(str);
		str.Format(_T("nBlockAlign: %d\n"), wfe.nBlockAlign);
		AddLine(str);
		str.Format(_T("wBitsPerSample: %d\n"), wfe.wBitsPerSample);
		AddLine(str);

		if(wfe.wFormatTag != WAVE_FORMAT_PCM && wfe.cbSize > 0)
		{
			str.Format(_T("cbSize: %d (extra bytes)\n"), wfe.cbSize);
			AddLine(str);

			if(wfe.wFormatTag == WAVE_FORMAT_EXTENSIBLE && wfe.cbSize == sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX))
			{
				WAVEFORMATEXTENSIBLE& wfe = *((WAVEFORMATEXTENSIBLE*)pmt->pbFormat);

				AddLine(_T("WAVEFORMATEXTENSIBLE:\n"));
				if(wfe.Format.wBitsPerSample != 0) str.Format(_T("wValidBitsPerSample: %d\n"), wfe.Samples.wValidBitsPerSample);
				else str.Format(_T("wSamplesPerBlock: %d\n"), wfe.Samples.wSamplesPerBlock);
				AddLine(str);
				str.Format(_T("dwChannelMask: 0x%08x\n"), wfe.dwChannelMask);
				AddLine(str);
				str.Format(_T("SubFormat: %s\n"), CStringFromGUID(wfe.SubFormat));
				AddLine(str);
			}
			else if(wfe.wFormatTag == WAVE_FORMAT_DOLBY_AC3 && wfe.cbSize == sizeof(DOLBYAC3WAVEFORMAT)-sizeof(WAVEFORMATEX))
			{
				DOLBYAC3WAVEFORMAT& wfe = *((DOLBYAC3WAVEFORMAT*)pmt->pbFormat);

				AddLine(_T("DOLBYAC3WAVEFORMAT:\n"));
				str.Format(_T("bBigEndian: %d\n"), wfe.bBigEndian);
				AddLine(str);
				str.Format(_T("bsid: %d\n"), wfe.bsid);
				AddLine(str);
				str.Format(_T("lfeon: %d\n"), wfe.lfeon);
				AddLine(str);
				str.Format(_T("copyrightb: %d\n"), wfe.copyrightb);
				AddLine(str);
				str.Format(_T("nAuxBitsCode: %d\n"), wfe.nAuxBitsCode);
				AddLine(str);
			}
		}

		AddLine();
	}

	if(pmt->cbFormat > 0)
	{
		AddLine(_T("pbFormat:\n"));

		for(int i = 0, j = (pmt->cbFormat + 15) & ~15; i < j; i += 16)
		{
			str.Format(_T("%08x:"), i);
			for(int k = i, l = min(i+16, pmt->cbFormat); k < l; k++)
			{
				CString byte;
				byte.Format(_T(" %02x"), pmt->pbFormat[k]);
				str += byte;
			}
			str += '\n';
			AddLine(str);
		}

		AddLine();
	}
}

BEGIN_MESSAGE_MAP(CMediaTypesDlg, CResizableDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON1, OnUpdateButton1)
END_MESSAGE_MAP()


// CMediaTypesDlg message handlers

BOOL CMediaTypesDlg::OnInitDialog()
{
	__super::OnInitDialog();

	CAtlList<CStringW> path;
	CAtlList<CMediaType> mts;

	for(int i = 0; S_OK == m_pGBDE->GetDeadEnd(i, path, mts); i++)
	{
		if(!path.GetCount()) continue;
		m_pins.SetItemData(m_pins.AddString(CString(path.GetTail())), (DWORD_PTR)i);
	}

	m_pins.SetCurSel(0);
	OnCbnSelchangeCombo1();

	AddAnchor(IDC_STATIC1, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_STATIC2, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_COMBO1, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_EDIT1, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_BUTTON1, BOTTOM_LEFT);
	AddAnchor(IDOK, BOTTOM_RIGHT);

	SetMinTrackSize(CSize(300, 200));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMediaTypesDlg::OnCbnSelchangeCombo1()
{
	m_report.SetWindowText(_T(""));

	int i = m_pins.GetCurSel();
	if(i < 0) return;

	CAtlList<CStringW> path;
	CAtlList<CMediaType> mts;

	if(FAILED(m_pGBDE->GetDeadEnd(i, path, mts)) || !path.GetCount()) 
		return;

	POSITION pos = path.GetHeadPosition();
	while(pos)
	{
		AddLine(CString(path.GetNext(pos)) + _T("\n"));
		if(!pos) AddLine(_T("\n"));
	}

	pos = mts.GetHeadPosition();
	for(int j = 0; pos; j++)
	{
		CString str;
		str.Format(_T("Media Type %d:\n"), j);
		AddLine(str);
		AddLine(_T("--------------------------\n"));
		AddMediaType(&mts.GetNext(pos));
		AddLine();
	}

	m_report.SetSel(0, 0);
}

void CMediaTypesDlg::OnBnClickedButton1()
{
	if(m_subtype.Data2 == 0x0000 && m_subtype.Data3 == 0x0010
	&& *(unsigned __int64*)m_subtype.Data4 == 0x719b3800aa000080i64)
	{
		BYTE* p = (BYTE*)&m_subtype.Data1;
		for(int i = 0; i < 4; i++, p++)
			if(*p >= 'a' && *p <= 'z') *p -= 0x20;
	}

	CString str;
	str.Format(_T("http://gabest.org/codec.php?type=%d&guid=%s"), m_type, CStringFromGUID(m_subtype));
	ShellExecute(NULL, _T("open"), str, NULL, NULL, SW_SHOW);
}

void CMediaTypesDlg::OnUpdateButton1(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_subtype != GUID_NULL);
}
