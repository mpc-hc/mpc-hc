/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
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

#pragma once

#include "PPagePlayer.h"
#include "PPageFormats.h"
#include "PPageAccelTbl.h"
#include "PPageLogo.h"
#include "PPagePlayback.h"
#include "PPageDVD.h"
#include "PPageOutput.h"
#include "PPageFullscreen.h"
#include "PPageSync.h"
#include "PPageWebServer.h"
#include "PPageInternalFilters.h"
#include "PPageAudioSwitcher.h"
#include "PPageExternalFilters.h"
#include "PPageSubtitles.h"
#include "PPageSubStyle.h"
#include "PPageSubDB.h"
#include "PPageSubMisc.h"
#include "PPageTweaks.h"
#include "PPageCasimir.h"
#include "PPageCapture.h"


// CTreePropSheetTreeCtrl
using namespace TreePropSheet;

class CTreePropSheetTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CTreePropSheetTreeCtrl)

public:
	CTreePropSheetTreeCtrl();
	virtual ~CTreePropSheetTreeCtrl();

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};

// CPPageSheet

class CPPageSheet : public CTreePropSheet
{
	DECLARE_DYNAMIC(CPPageSheet)

private:
	bool m_bLockPage;

	CPPagePlayer m_player;
	CPPageFormats m_formats;
	CPPageAccelTbl m_acceltbl;
	CPPageLogo m_logo;
	CPPagePlayback m_playback;
	CPPageDVD m_dvd;
	CPPageOutput m_output;
	CPPageFullscreen m_fullscreen;
	CPPageSync m_sync;
	CPPageWebServer m_webserver;
	CPPageSubtitles m_subtitles;
	CPPageSubStyle m_substyle;
	CPPageSubDB m_subdb;
	CPPageSubMisc m_subMisc;
	CPPageInternalFilters m_internalfilters;
	CPPageAudioSwitcher m_audioswitcher;
	CPPageExternalFilters m_externalfilters;
	CPPageTweaks m_tweaks;
	CPPageCasimir m_casimir;
	CPPageCapture m_tuner;

	CTreeCtrl* CreatePageTreeObject();

public:
	CPPageSheet(LPCTSTR pszCaption, IFilterGraph* pFG, CWnd* pParentWnd, UINT idPage = 0);
	virtual ~CPPageSheet();
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint point);

	void LockPage()
	{
		m_bLockPage = true;
	};
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};
