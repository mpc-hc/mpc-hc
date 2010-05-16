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

#include <afxcview.h>
#include "PlayerListCtrl.h"
#include "../../subtitles/RTS.h"
#include "../../subtitles/VobSubFile.h"


// CPlayerSubresyncBar

class CPlayerSubresyncBar : public CSizingControlBarG
{
    DECLARE_DYNAMIC(CPlayerSubresyncBar)

private:
    CPlayerListCtrl m_list;

    CFont m_font;

    CCritSec* m_pSubLock;
    CComPtr<ISubStream> m_pSubStream;

    int m_lastSegment;
    __int64 m_rt;

    enum
    {
        // TEXTSUB
        COL_START=0, COL_END, COL_PREVSTART, COL_PREVEND, COL_TEXT, COL_STYLE, COL_FONT, COL_CHARSET, COL_UNICODE, COL_LAYER, COL_ACTOR, COL_EFFECT,
        // VOBSUB
        /* ........... same as TEXTSUB ............. */	  COL_VOBID=COL_TEXT, COL_CELLID, COL_FORCED,
    };

    enum {NONE = 0, VOBSUB, TEXTSUB};
    int m_mode;

    bool m_fUnlink;

    typedef struct
    {
        int orgstart, newstart, orgend, newend;
    } SubTime;
    CAtlArray<SubTime> m_subtimes;

//	CRenderedTextSubtitle m_sts;
    CSimpleTextSubtitle m_sts;

    int GetStartTime(int iItem), GetEndTime(int iItem);
    void FormatTime(int iItem, TCHAR* buff, int time /* 0:start, 1:newstart, 2:preview */, bool fEnd);

    void UpdatePreview(), UpdateStrings();

	enum {TSMOD=1, TEMOD=2, TSADJ=4, TEADJ=8, TSEP=0x80000000};

	void SetSTS0( int &start, int end, int ti0 );
	void SetSTS1( int &start, int end, int ti0, double m, int i0 );

    void GetCheck(int iItem, bool& fStartMod, bool& fEndMod, bool& fStartAdj, bool& fEndAdj);
    void SetCheck(int iItem, bool fStart, bool fEnd);

    bool ModStart(int iItem, int t, bool fReset = false);
    bool ModEnd(int iItem, int t, bool fReset = false);

public:
    CPlayerSubresyncBar();
    virtual ~CPlayerSubresyncBar();

    BOOL Create(CWnd* pParentWnd, CCritSec* pSubLock);

    void SetTime(__int64 rt);

    void SetSubtitle(ISubStream* pSubStream, double fps);
    void ResetSubtitle();
    void SaveSubtitle();

    int FindNearestSub(__int64& rtPos, bool bForward);
    bool ShiftSubtitle(int nItem, long lValue, __int64& rtPos);
    bool SaveToDisk();


protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    bool IsShortCut(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBeginlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDolabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRclickList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLvnKeydownList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult);
};
