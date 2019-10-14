/*
 * (C) 2010-2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
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


IMPLEMENT_DYNAMIC(CPPageSync, CMPCThemePPageBase)

CPPageSync::CPPageSync()
    : CMPCThemePPageBase(CPPageSync::IDD, CPPageSync::IDD)
    , m_bSynchronizeVideo(FALSE)
    , m_bSynchronizeDisplay(FALSE)
    , m_bSynchronizeNearest(FALSE)
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
    const CRenderersSettings::CAdvRendererSettings& ars = AfxGetAppSettings().m_RenderersSettings.m_AdvRendSets;
    m_bSynchronizeVideo = ars.bSynchronizeVideo;
    m_bSynchronizeDisplay = ars.bSynchronizeDisplay;
    m_bSynchronizeNearest = ars.bSynchronizeNearest;
    m_iLineDelta = ars.iLineDelta;
    m_iColumnDelta = ars.iColumnDelta;
    m_fCycleDelta = ars.fCycleDelta;
    m_fTargetSyncOffset = ars.fTargetSyncOffset;
    m_fControlLimit = ars.fControlLimit;

    UpdateData(FALSE);
}

BOOL CPPageSync::OnApply()
{
    UpdateData();

    CRenderersSettings::CAdvRendererSettings& ars = AfxGetAppSettings().m_RenderersSettings.m_AdvRendSets;
    ars.bSynchronizeVideo = !!m_bSynchronizeVideo;
    ars.bSynchronizeDisplay = !!m_bSynchronizeDisplay;
    ars.bSynchronizeNearest = !!m_bSynchronizeNearest;
    ars.iLineDelta = m_iLineDelta;
    ars.iColumnDelta = m_iColumnDelta;
    ars.fCycleDelta = m_fCycleDelta;
    ars.fTargetSyncOffset = m_fTargetSyncOffset;
    ars.fControlLimit = m_fControlLimit;

    return __super::OnApply();
}

BEGIN_MESSAGE_MAP(CPPageSync, CMPCThemePPageBase)
    ON_BN_CLICKED(IDC_SYNCVIDEO, OnBnClickedSyncVideo)
    ON_BN_CLICKED(IDC_SYNCDISPLAY, OnBnClickedSyncDisplay)
    ON_BN_CLICKED(IDC_SYNCNEAREST, OnBnClickedSyncNearest)
    ON_UPDATE_COMMAND_UI(IDC_SYNCVIDEO, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_STATIC1, OnUpdateSyncVideo)
    ON_UPDATE_COMMAND_UI(IDC_CYCLEDELTA, OnUpdateSyncVideo)
    ON_UPDATE_COMMAND_UI(IDC_SYNCDISPLAY, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_STATIC2, OnUpdateSyncDisplay)
    ON_UPDATE_COMMAND_UI(IDC_LINEDELTA, OnUpdateSyncDisplay)
    ON_UPDATE_COMMAND_UI(IDC_STATIC3, OnUpdateSyncDisplay)
    ON_UPDATE_COMMAND_UI(IDC_COLUMNDELTA, OnUpdateSyncDisplay)
    ON_UPDATE_COMMAND_UI(IDC_STATIC4, OnUpdateSyncDisplay)
    ON_UPDATE_COMMAND_UI(IDC_SYNCNEAREST, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_CYCLEDELTA, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_LINEDELTA, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_COLUMNDELTA, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_TARGETSYNCOFFSET, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_CONTROLLIMIT, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_STATIC5, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_STATIC6, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_STATIC7, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_STATIC8, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_STATIC9, OnUpdateRenderer)
    ON_UPDATE_COMMAND_UI(IDC_STATIC10, OnUpdateRenderer)
END_MESSAGE_MAP()

void CPPageSync::OnBnClickedSyncVideo()
{
    m_bSynchronizeVideo = !m_bSynchronizeVideo;
    if (m_bSynchronizeVideo) {
        m_bSynchronizeDisplay = FALSE;
        m_bSynchronizeNearest = FALSE;
    }
    UpdateData(FALSE);
    SetModified();
}

void CPPageSync::OnBnClickedSyncDisplay()
{
    m_bSynchronizeDisplay = !m_bSynchronizeDisplay;
    if (m_bSynchronizeDisplay) {
        m_bSynchronizeVideo = FALSE;
        m_bSynchronizeNearest = FALSE;
    }
    UpdateData(FALSE);
    SetModified();
}

void CPPageSync::OnBnClickedSyncNearest()
{
    m_bSynchronizeNearest = !m_bSynchronizeNearest;
    if (m_bSynchronizeNearest) {
        m_bSynchronizeVideo = FALSE;
        m_bSynchronizeDisplay = FALSE;
    }
    UpdateData(FALSE);
    SetModified();
}

void CPPageSync::OnUpdateRenderer(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(AfxGetAppSettings().iDSVideoRendererType == VIDRNDT_DS_SYNC);
}

void CPPageSync::OnUpdateSyncDisplay(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(AfxGetAppSettings().iDSVideoRendererType == VIDRNDT_DS_SYNC && m_bSynchronizeDisplay);
}

void CPPageSync::OnUpdateSyncVideo(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(AfxGetAppSettings().iDSVideoRendererType == VIDRNDT_DS_SYNC && m_bSynchronizeVideo);
}
