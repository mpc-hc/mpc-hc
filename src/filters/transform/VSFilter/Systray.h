/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see AUTHORS
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

#pragma once

class CSystrayWindow : public CWnd
{
	SystrayIconData* m_tbid;

	void StepSub(int dir) {
		int iSelected, nLangs;
		if (FAILED(m_tbid->dvs->get_LanguageCount(&nLangs))) {
			return;
		}
		if (FAILED(m_tbid->dvs->get_SelectedLanguage(&iSelected))) {
			return;
		}
		if (nLangs > 0) {
			m_tbid->dvs->put_SelectedLanguage((iSelected+dir+nLangs)%nLangs);
		}
	}

	void ShowSub(bool fShow) {
		m_tbid->dvs->put_HideSubtitles(!fShow);
	}

	void ToggleSub() {
		bool fShow;
		if (FAILED(m_tbid->dvs->get_HideSubtitles(&fShow))) {
			return;
		}
		m_tbid->dvs->put_HideSubtitles(!fShow);
	}

public:
	CSystrayWindow(SystrayIconData* tbid) : m_tbid(tbid) {}

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDVSPrevSub(WPARAM, LPARAM);
	afx_msg LRESULT OnDVSNextSub(WPARAM, LPARAM);
	afx_msg LRESULT OnDVSHideSub(WPARAM, LPARAM);
	afx_msg LRESULT OnDVSShowSub(WPARAM, LPARAM);
	afx_msg LRESULT OnDVSShowHideSub(WPARAM, LPARAM);
	afx_msg LRESULT OnTaskBarRestart(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyIcon(WPARAM, LPARAM);
};

extern DWORD CALLBACK SystrayThreadProc(void* pParam);
