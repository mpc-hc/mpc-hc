/*
 * (C) 2006-2011 see AUTHORS
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

	InitDialogPrivate();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageSync::OnSetActive()
{
	InitDialogPrivate();

	return __super::OnSetActive();
}

void CPPageSync::InitDialogPrivate()
{
	AppSettings& s = AfxGetAppSettings();
	CMainFrame * pFrame;
	pFrame = (CMainFrame *)(AfxGetApp()->m_pMainWnd);
	if ((s.iDSVideoRendererType == VIDRNDT_DS_SYNC) && (pFrame->GetPlaybackMode() == PM_NONE)) {
		GetDlgItem(IDC_SYNCVIDEO)->EnableWindow(TRUE);
		GetDlgItem(IDC_SYNCDISPLAY)->EnableWindow(TRUE);
		GetDlgItem(IDC_SYNCNEAREST)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_SYNCVIDEO)->EnableWindow(FALSE);
		GetDlgItem(IDC_SYNCDISPLAY)->EnableWindow(FALSE);
		GetDlgItem(IDC_SYNCNEAREST)->EnableWindow(FALSE);
	}

	CRenderersSettings::CRendererSettingsEVR& rendererSettings = s.m_RenderersSettings.m_RenderSettings;
	m_bSynchronizeVideo = rendererSettings.bSynchronizeVideo;
	m_bSynchronizeDisplay = rendererSettings.bSynchronizeDisplay;
	m_bSynchronizeNearest = rendererSettings.bSynchronizeNearest;
	m_iLineDelta = rendererSettings.iLineDelta;
	m_iColumnDelta = rendererSettings.iColumnDelta;
	m_fCycleDelta = rendererSettings.fCycleDelta;
	m_fTargetSyncOffset = rendererSettings.fTargetSyncOffset;
	m_fControlLimit = rendererSettings.fControlLimit;

	UpdateData(FALSE);
}

BOOL CPPageSync::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	CRenderersSettings::CRendererSettingsEVR& rendererSettings = s.m_RenderersSettings.m_RenderSettings;
	rendererSettings.bSynchronizeVideo = !!m_bSynchronizeVideo;
	rendererSettings.bSynchronizeDisplay = !!m_bSynchronizeDisplay;
	rendererSettings.bSynchronizeNearest = !!m_bSynchronizeNearest;
	rendererSettings.iLineDelta = m_iLineDelta;
	rendererSettings.iColumnDelta = m_iColumnDelta;
	rendererSettings.fCycleDelta = m_fCycleDelta;
	rendererSettings.fTargetSyncOffset = m_fTargetSyncOffset;
	rendererSettings.fControlLimit = m_fControlLimit;
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
	if (m_bSynchronizeVideo) {
		m_bSynchronizeDisplay = FALSE;
		m_bSynchronizeNearest = FALSE;
	}
	UpdateData(FALSE);
}

void CPPageSync::OnBnClickedSyncDisplay()
{
	m_bSynchronizeDisplay = !m_bSynchronizeDisplay;
	if (m_bSynchronizeDisplay) {
		m_bSynchronizeVideo = FALSE;
		m_bSynchronizeNearest = FALSE;
	}
	UpdateData(FALSE);
}

void CPPageSync::OnBnClickedSyncNearest()
{
	m_bSynchronizeNearest = !m_bSynchronizeNearest;
	if (m_bSynchronizeNearest) {
		m_bSynchronizeVideo = FALSE;
		m_bSynchronizeDisplay = FALSE;
	}
	UpdateData(FALSE);
}
