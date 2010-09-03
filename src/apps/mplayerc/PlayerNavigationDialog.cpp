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

#include "stdafx.h"
#include "mplayerc.h"
#include "mainfrm.h"
#include "PlayerNavigationDialog.h"
#include "../../DSUtil/DSUtil.h"
#include <moreuuids.h>


// CPlayerNavigationDialog dialog

// IMPLEMENT_DYNAMIC(CPlayerNavigationDialog, CResizableDialog)
CPlayerNavigationDialog::CPlayerNavigationDialog()
	: CResizableDialog(CPlayerNavigationDialog::IDD, NULL)
{
}

CPlayerNavigationDialog::~CPlayerNavigationDialog()
{

}

BOOL CPlayerNavigationDialog::Create(CWnd* pParent)
{
	if(!__super::Create(IDD, pParent))
		return FALSE;
	m_pParent = pParent;
	return TRUE;
}


void CPlayerNavigationDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCHANNELS, m_ChannelList);
	DDX_Control(pDX, IDC_NAVIGATION_AUDIO, m_ComboAudio);
	DDX_Control(pDX, IDC_NAVIGATION_INFO, m_ButtonInfo);
	DDX_Control(pDX, IDC_NAVIGATION_SCAN, m_ButtonScan);
	DDX_Control(pDX, IDC_NAVIGATION_FILTERSTATIONS, m_ButtonFilterStations);
}

BOOL CPlayerNavigationDialog::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_RETURN)
		{
			CWnd* pFocused = GetFocus();
			UNUSED_ALWAYS(pFocused);
		}
	}
	return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CPlayerNavigationDialog, CResizableDialog)
	ON_WM_DESTROY()
	ON_LBN_SELCHANGE(IDC_LISTCHANNELS, OnChangeChannel)
	ON_CBN_SELCHANGE(IDC_NAVIGATION_AUDIO, OnSelChangeComboAudio)
	ON_BN_CLICKED (IDC_NAVIGATION_INFO, OnButtonInfo)
	ON_BN_CLICKED(IDC_NAVIGATION_SCAN, OnTunerScan)
	ON_BN_CLICKED(IDC_NAVIGATION_FILTERSTATIONS, OnTvRadioStations)

END_MESSAGE_MAP()


// CPlayerNavigationDialog message handlers

BOOL CPlayerNavigationDialog::OnInitDialog()
{
	__super::OnInitDialog();
	m_bTVStations = true;
	m_ButtonFilterStations.SetWindowText(_T(BTN_CAPTION_SEERADIO));
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPlayerNavigationDialog::OnDestroy()
{
	m_ChannelList.ResetContent();
	__super::OnDestroy();
}

void CPlayerNavigationDialog::OnChangeChannel()
{
	CWnd* TempWnd;
	int nItem;

	TempWnd = static_cast<CPlayerNavigationBar*> (m_pParent) -> m_pParent;
	nItem = p_nItems[m_ChannelList.GetCurSel()] + ID_NAVIGATE_CHAP_SUBITEM_START;
	static_cast<CMainFrame*> (TempWnd) -> OnNavigateChapters(nItem);
	SetupAudioSwitcherSubMenu();
}

void CPlayerNavigationDialog::SetupAudioSwitcherSubMenu(CDVBChannel* pChannel)
{
	bool bFound = FALSE;
	int nCurrentChannel;
	AppSettings& s = AfxGetAppSettings();

	if (!pChannel)
	{
		nCurrentChannel = s.DVBLastChannel;
		POSITION	pos = s.DVBChannels.GetHeadPosition();
		while (pos && !bFound)
		{
			pChannel = &s.DVBChannels.GetNext(pos);
			if (nCurrentChannel == pChannel->GetPrefNumber())
			{
				bFound = TRUE;
				break;
			}
		}
	}

	m_ComboAudio.ResetContent();
	for (int i=0; i < pChannel->GetAudioCount(); i++)
	{
		m_ComboAudio.AddString(pChannel->GetAudio(i)->Language);
		m_audios[i].PID = pChannel->GetAudio(i)-> PID;
		m_audios[i].Type = pChannel->GetAudio(i)->Type;
		m_audios[i].PesType = pChannel->GetAudio(i) -> PesType;
		m_audios[i].Language = pChannel->GetAudio(i) -> Language;
	}

	m_ComboAudio.SetCurSel(0);  // TODO: managing default languages

}

void CPlayerNavigationDialog::UpdateElementList()
{
	int nItem;
	int nCurrentChannel;
	AppSettings& s = AfxGetAppSettings();

	if (s.iDefaultCaptureDevice == 1)
	{
		m_ChannelList.ResetContent();

		nCurrentChannel = s.DVBLastChannel;

		POSITION	pos = s.DVBChannels.GetHeadPosition();
		while (pos)
		{
			CDVBChannel&	Channel = s.DVBChannels.GetNext(pos);
			if ((m_bTVStations && (Channel.GetVideoPID() != 0)) || 
				(!m_bTVStations && (Channel.GetAudioCount() > 0)) && (Channel.GetVideoPID() == 0))
			{
				nItem = m_ChannelList.AddString (Channel.GetName());
				if (nItem < MAX_CHANNELS_ALLOWED)
					p_nItems [nItem] = Channel.GetPrefNumber();
				if (nCurrentChannel == Channel.GetPrefNumber())
				{
					m_ChannelList.SetCurSel(nItem);
					SetupAudioSwitcherSubMenu(&Channel);
				}
			}
		}
	}

}

void CPlayerNavigationDialog::UpdatePos(int nID)
{
	for (int i=0; i < MAX_CHANNELS_ALLOWED; i++)
	{
		if (p_nItems [i] == nID)
		{
			m_ChannelList.SetCurSel(i);
			break;
		}

	}
}

void CPlayerNavigationDialog::OnTunerScan()
{
	CWnd* TempWnd;

	TempWnd = static_cast<CPlayerNavigationBar*> (m_pParent) -> m_pParent;
	static_cast<CMainFrame*> (TempWnd) -> OnTunerScan();
	UpdateElementList();
}

void CPlayerNavigationDialog::OnSelChangeComboAudio()
{
	UINT nID;
	CWnd* TempWnd;

	nID = m_ComboAudio.GetCurSel() + ID_NAVIGATE_AUDIO_SUBITEM_START;

	TempWnd = static_cast<CPlayerNavigationBar*> (m_pParent) -> m_pParent;
	static_cast<CMainFrame*> (TempWnd) -> OnNavigateAudio(nID);
}

void CPlayerNavigationDialog::OnButtonInfo()
{
	// TODO: Retrieve and show channel info

}

void CPlayerNavigationDialog::OnTvRadioStations()
{
	m_bTVStations = !m_bTVStations;
	UpdateElementList();
	if (m_bTVStations) 
		m_ButtonFilterStations.SetWindowText(_T(BTN_CAPTION_SEERADIO));
	else
		m_ButtonFilterStations.SetWindowText(_T(BTN_CAPTION_SEETV));
}
