/*
 * (C) 2006-2007 see AUTHORS
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

#include "stdafx.h"
#include "mplayerc.h"
#include "MainFrm.h"
#include "PPageSync.h"

IMPLEMENT_DYNAMIC(CPPageSync, CPPageBase)

CPPageSync::CPPageSync()
	: CPPageBase(CPPageSync::IDD, CPPageSync::IDD)
	, m_bSynchronizeVideo(0)
	, m_bSynchronizeDisplay(0)
	, m_bSynchronizeNearest(0)
	, m_iLineDelta(0)
	, m_iColumnDelta(0)
	, m_fCycleDelta(0.0012)
	, m_fTargetSyncOffset(10.0)
	, m_fControlLimit(2.0)
{
}

CPPageSync::~CPPageSync()
{
}

void CPPageSync::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_SYNCVIDEO, m_bSynchronizeVideo);
	DDX_Check(pDX, IDC_SYNCDISPLAY, m_bSynchronizeDisplay);
	DDX_Check(pDX, IDC_SYNCNEAREST, m_bSynchronizeNearest);
	DDX_Text(pDX, IDC_CYCLEDELTA, m_fCycleDelta);
	DDX_Text(pDX, IDC_LINEDELTA, m_iLineDelta);
	DDX_Text(pDX, IDC_COLUMNDELTA, m_iColumnDelta);
	DDX_Text(pDX, IDC_TARGETSYNCOFFSET, m_fTargetSyncOffset);
	DDX_Text(pDX, IDC_CONTROLLIMIT, m_fControlLimit);
}

BOOL CPPageSync::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();
	CMainFrame * pFrame;
	pFrame = (CMainFrame *)(AfxGetApp()->m_pMainWnd);
	if ((s.iDSVideoRendererType == VIDRNDT_DS_SYNC) && (pFrame->GetPlaybackMode() == PM_NONE))
	{
		GetDlgItem(IDC_SYNCVIDEO)->EnableWindow(TRUE);
		GetDlgItem(IDC_SYNCDISPLAY)->EnableWindow(TRUE);
		GetDlgItem(IDC_SYNCNEAREST)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_SYNCVIDEO)->EnableWindow(FALSE);
		GetDlgItem(IDC_SYNCDISPLAY)->EnableWindow(FALSE);
		GetDlgItem(IDC_SYNCNEAREST)->EnableWindow(FALSE);
	}

	m_bSynchronizeVideo = s.m_RenderSettings.bSynchronizeVideo;
	m_bSynchronizeDisplay = s.m_RenderSettings.bSynchronizeDisplay;
	m_bSynchronizeNearest = s.m_RenderSettings.bSynchronizeNearest;
	m_iLineDelta = s.m_RenderSettings.iLineDelta;
	m_iColumnDelta = s.m_RenderSettings.iColumnDelta;
	m_fCycleDelta = s.m_RenderSettings.fCycleDelta;
	m_fTargetSyncOffset = s.m_RenderSettings.fTargetSyncOffset;
	m_fControlLimit = s.m_RenderSettings.fControlLimit;

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageSync::OnSetActive() 
{
	AppSettings& s = AfxGetAppSettings();
	CMainFrame * pFrame;
	pFrame = (CMainFrame *)(AfxGetApp()->m_pMainWnd);
	if ((s.iDSVideoRendererType == VIDRNDT_DS_SYNC) && (pFrame->GetPlaybackMode() == PM_NONE))
	{
		GetDlgItem(IDC_SYNCVIDEO)->EnableWindow(TRUE);
		GetDlgItem(IDC_SYNCDISPLAY)->EnableWindow(TRUE);
		GetDlgItem(IDC_SYNCNEAREST)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_SYNCVIDEO)->EnableWindow(FALSE);
		GetDlgItem(IDC_SYNCDISPLAY)->EnableWindow(FALSE);
		GetDlgItem(IDC_SYNCNEAREST)->EnableWindow(FALSE);
	}

	m_bSynchronizeVideo = s.m_RenderSettings.bSynchronizeVideo;
	m_bSynchronizeDisplay = s.m_RenderSettings.bSynchronizeDisplay;
	m_bSynchronizeNearest = s.m_RenderSettings.bSynchronizeNearest;
	m_iLineDelta = s.m_RenderSettings.iLineDelta;
	m_iColumnDelta = s.m_RenderSettings.iColumnDelta;
	m_fCycleDelta = s.m_RenderSettings.fCycleDelta;
	m_fTargetSyncOffset = s.m_RenderSettings.fTargetSyncOffset;
	m_fControlLimit = s.m_RenderSettings.fControlLimit;

	UpdateData(FALSE);

   return CPropertyPage::OnSetActive();
}

BOOL CPPageSync::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.m_RenderSettings.bSynchronizeVideo = !!m_bSynchronizeVideo;
	s.m_RenderSettings.bSynchronizeDisplay = !!m_bSynchronizeDisplay;
	s.m_RenderSettings.bSynchronizeNearest = !!m_bSynchronizeNearest;
	s.m_RenderSettings.iLineDelta = m_iLineDelta;
	s.m_RenderSettings.iColumnDelta = m_iColumnDelta;
	s.m_RenderSettings.fCycleDelta = m_fCycleDelta;
	s.m_RenderSettings.fTargetSyncOffset = m_fTargetSyncOffset;
	s.m_RenderSettings.fControlLimit = m_fControlLimit;
	return __super::OnApply();
}

BEGIN_MESSAGE_MAP(CPPageSync, CPPageBase)
	ON_BN_CLICKED(IDC_SYNCVIDEO, OnBnClickedSyncVideo)
	ON_BN_CLICKED(IDC_SYNCDISPLAY, OnBnClickedSyncDisplay)
	ON_BN_CLICKED(IDC_SYNCNEAREST, OnBnClickedSyncNearest)
END_MESSAGE_MAP()

void CPPageSync::OnBnClickedSyncVideo()
{
	m_bSynchronizeVideo = !m_bSynchronizeVideo;
	if (m_bSynchronizeVideo)
	{
		m_bSynchronizeDisplay = FALSE;
		m_bSynchronizeNearest = FALSE;
	}
	UpdateData(FALSE);
}

void CPPageSync::OnBnClickedSyncDisplay()
{
	m_bSynchronizeDisplay = !m_bSynchronizeDisplay;
	if (m_bSynchronizeDisplay)
	{
		m_bSynchronizeVideo = FALSE;
		m_bSynchronizeNearest = FALSE;
	}
	UpdateData(FALSE);
}

void CPPageSync::OnBnClickedSyncNearest()
{
	m_bSynchronizeNearest = !m_bSynchronizeNearest;
	if (m_bSynchronizeNearest)
	{
		m_bSynchronizeVideo = FALSE;
		m_bSynchronizeDisplay = FALSE;
	}
	UpdateData(FALSE);
}
