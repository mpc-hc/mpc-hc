/*
 * $Id$
 *
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// TunerScanDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "MainFrm.h"
#include "TunerScanDlg.h"
#include "DVBChannel.h"

enum TSC_COLUMN
{
	TSCC_NUMBER,
	TSCC_NAME,
	TSCC_FREQUENCY,
	TSCC_CHANNEL
};

// CTunerScanDlg dialog

IMPLEMENT_DYNAMIC(CTunerScanDlg, CDialog)

CTunerScanDlg::CTunerScanDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTunerScanDlg::IDD, pParent)
	, m_ulFrequencyStart(474000)
	, m_ulFrequencyEnd(858000)
	, m_ulBandwidth(8000)
	, m_bInProgress(false)
{
}

CTunerScanDlg::~CTunerScanDlg()
{
}

BOOL CTunerScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();


	m_ChannelList.InsertColumn(TSCC_NUMBER, _T("N"), LVCFMT_LEFT, 50);
	m_ChannelList.InsertColumn(TSCC_NAME, _T("Name"), LVCFMT_LEFT, 250);
	m_ChannelList.InsertColumn(TSCC_FREQUENCY, _T("Frequency"), LVCFMT_LEFT, 100);
	m_ChannelList.InsertColumn(TSCC_CHANNEL, _T("Channel"), LVCFMT_LEFT, 0);

	m_Progress.SetRange(0, 100);
	m_Strength.SetRange(0, 100);
	m_Quality.SetRange(0, 100);
	m_btnSave.EnableWindow(FALSE);

	return TRUE;
}

void CTunerScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FREQ_START, m_ulFrequencyStart);
	DDX_Text(pDX, IDC_FREQ_END, m_ulFrequencyEnd);
	DDX_Text(pDX, IDC_BANDWIDTH, m_ulBandwidth);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_STRENGTH, m_Strength);
	DDX_Control(pDX, IDC_QUALITY, m_Quality);
	DDX_Control(pDX, IDC_CHANNEL_LIST, m_ChannelList);
	DDX_Control(pDX, ID_START, m_btnStart);
	DDX_Control(pDX, ID_SAVE, m_btnSave);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CTunerScanDlg, CDialog)
	ON_MESSAGE(WM_TUNER_SCAN_PROGRESS, OnScanProgress)
	ON_MESSAGE(WM_TUNER_SCAN_END, OnScanEnd)
	ON_MESSAGE(WM_TUNER_STATS, OnStats)
	ON_MESSAGE(WM_TUNER_NEW_CHANNEL, OnNewChannel)
	ON_BN_CLICKED(ID_SAVE, &CTunerScanDlg::OnBnClickedSave)
	ON_BN_CLICKED(ID_START, &CTunerScanDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDCANCEL, &CTunerScanDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CTunerScanDlg message handlers

void CTunerScanDlg::OnBnClickedSave()
{
	AppSettings& s = AfxGetAppSettings();

	s.DVBChannels.RemoveAll();

	for (int i=0; i <m_ChannelList.GetItemCount(); i++)
	{
		CDVBChannel		Channel;
		Channel.FromString (m_ChannelList.GetItemText (i, TSCC_CHANNEL));
		Channel.SetPrefNumber(i);
		s.DVBChannels.AddTail (Channel);
	}

	OnOK();
}

void CTunerScanDlg::OnBnClickedStart()
{
	if (!m_bInProgress)
	{
		UpdateData(true);
		CAutoPtr<TunerScanData>		pTSD (DNew TunerScanData);
		pTSD->Hwnd				= m_hWnd;
		pTSD->FrequencyStart	= m_ulFrequencyStart;
		pTSD->FrequencyStop		= m_ulFrequencyEnd;
		pTSD->Bandwidth			= m_ulBandwidth;

		m_ChannelList.DeleteAllItems();
		((CMainFrame*)AfxGetMainWnd())->StartTunerScan (pTSD);

		SetProgress (true);
	}
	else
		((CMainFrame*)AfxGetMainWnd())->StopTunerScan();
}

void CTunerScanDlg::OnBnClickedCancel()
{
	if (m_bInProgress)
		((CMainFrame*)AfxGetMainWnd())->StopTunerScan();

	OnCancel();
}


LRESULT CTunerScanDlg::OnScanProgress(WPARAM wParam, LPARAM lParam)
{
	m_Progress.SetPos(wParam);
	return TRUE;
}

LRESULT CTunerScanDlg::OnScanEnd(WPARAM wParam, LPARAM lParam)
{
	SetProgress (false);
	return TRUE;
}

LRESULT CTunerScanDlg::OnStats(WPARAM wParam, LPARAM lParam)
{
	m_Strength.SetPos ((int)wParam);
	m_Quality.SetPos  ((int)lParam);
	return TRUE;
}

LRESULT CTunerScanDlg::OnNewChannel(WPARAM wParam, LPARAM lParam)
{
	CDVBChannel		Channel;
	CString			strTemp;
	int				nItem;
	int				nChannelNumber;
	Channel.FromString ((LPCTSTR) lParam);

	if (Channel.GetOriginNumber() != 0) // LCN is available
	{
		nChannelNumber = Channel.GetOriginNumber();
		// Insert new channel so that channels are sorted by their logical number
		for (nItem=0; nItem<m_ChannelList.GetItemCount(); nItem++)
		{
			if (m_ChannelList.GetItemData(nItem) > nChannelNumber)
				break;
		}
	}
	else
		nChannelNumber = nItem = m_ChannelList.GetItemCount();

	strTemp.Format(_T("%03d"), nChannelNumber);
	nItem = m_ChannelList.InsertItem (nItem, strTemp);

	m_ChannelList.SetItemData (nItem, Channel.GetOriginNumber());

	strTemp.Format(_T("%d"), nChannelNumber);
	m_ChannelList.SetItemText (nItem, TSCC_NUMBER, strTemp);

	m_ChannelList.SetItemText (nItem, TSCC_NAME, Channel.GetName());

	strTemp.Format(_T("%d"), Channel.GetFrequency());
	m_ChannelList.SetItemText (nItem, TSCC_FREQUENCY, strTemp);

	m_ChannelList.SetItemText (nItem, TSCC_CHANNEL, (LPCTSTR) lParam);


	return TRUE;
}


void CTunerScanDlg::SetProgress (bool bState)
{
	if (bState)
	{
		m_btnStart.SetWindowTextW(_T("Stop"));
		m_btnSave.EnableWindow(FALSE);
	}
	else
	{
		m_btnStart.SetWindowTextW(_T("Start"));
		m_Progress.SetPos (0);
		m_btnSave.EnableWindow(TRUE);
	}

	m_bInProgress = bState;
}
