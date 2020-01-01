/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015 see Authors.txt
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

#pragma once

#include "mplayerc.h"
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
#include "PPageAudioRenderer.h"
#include "PPageExternalFilters.h"
#include "PPageSubtitles.h"
#include "PPageSubStyle.h"
#include "PPageSubMisc.h"
#include "PPageTweaks.h"
#include "PPageMisc.h"
#include "PPageCapture.h"
#include "PPageShaders.h"
#include "PPageAdvanced.h"
#include "TreePropSheet/TreePropSheet.h"
#include "DpiHelper.h"
#include "CMPCThemeUtil.h"
#include "CMPCThemePropPageFrame.h"
#include "CMPCThemeTreeCtrl.h"

// CTreePropSheetTreeCtrl

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

class CPPageSheet : public TreePropSheet::CTreePropSheet, public CMPCThemeUtil
{
    DECLARE_DYNAMIC(CPPageSheet)

public:
    enum {
        APPLY_LANGUAGE_CHANGE = 100, // 100 is a magic number than won't collide with WinAPI constants
        RESET_SETTINGS
    };
    CPtrArray& getPages() { return m_pages; };
private:
    bool m_bLockPage;
    bool m_bLanguageChanged;

    CPPagePlayer m_player;
    CPPageFormats m_formats;
    CPPageAccelTbl m_acceltbl;
    CPPageLogo m_logo;
    CPPageWebServer m_webserver;
    CPPagePlayback m_playback;
    CPPageDVD m_dvd;
    CPPageOutput m_output;
    CPPageShaders m_shaders;
    CPPageFullscreen m_fullscreen;
    CPPageSync m_sync;
    CPPageCapture m_tuner;
#ifndef MPCHC_LITE
    CPPageInternalFilters m_internalfilters;
#endif
    CPPageAudioSwitcher m_audioswitcher;
    CPPageAudioRenderer m_audiorenderer;
    CPPageExternalFilters m_externalfilters;
    CPPageSubtitles m_subtitles;
    CPPageSubStyle m_substyle;
    CPPageSubMisc m_subMisc;
    CPPageTweaks m_tweaks;
    CPPageMisc m_misc;
    CPPageAdvanced m_advance;

    EventClient m_eventc;
    void EventCallback(MpcEvent ev);

    CMPCThemeTreeCtrl* CreatePageTreeObject();
    virtual void SetTreeCtrlTheme(CTreeCtrl * ctrl);
public:
    CPPageSheet(LPCTSTR pszCaption, IFilterGraph* pFG, CWnd* pParentWnd, UINT idPage = 0);
    virtual ~CPPageSheet();
    void fulfillThemeReqs();

    void LockPage() { m_bLockPage = true; };

protected:
    DpiHelper m_dpi;

    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnApply();
    virtual TreePropSheet::CPropPageFrame * CreatePageFrame();
public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
