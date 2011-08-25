/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
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


#pragma once

// CWinHotkeyCtrl

class CWinHotkeyCtrl : public CEdit
{
	DECLARE_DYNAMIC(CWinHotkeyCtrl)

public:
	CWinHotkeyCtrl();
	virtual ~CWinHotkeyCtrl();

	void UpdateText();
	DWORD GetWinHotkey();
	BOOL GetWinHotkey(UINT* pvkCode, UINT* pfModifiers);
	void SetWinHotkey(DWORD dwHk);
	void SetWinHotkey(UINT vkCode, UINT fModifiers);

private:
	static HHOOK sm_hhookKb;
	static CWinHotkeyCtrl* sm_pwhcFocus;

	UINT m_vkCode;
	DWORD m_fModSet, m_fModRel;
	BOOL m_fIsPressed;

private:
	BOOL InstallKbHook();
	BOOL UninstallKbHook();
    
#if _WIN32_WINNT < 0x500
	static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
#else // _WIN32_WINNT >= 0x500
	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
#endif // _WIN32_WINNT >= 0x500

	afx_msg LRESULT OnKey(WPARAM wParam, LPARAM lParam);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnDestroy();
protected:
	virtual void PreSubclassWindow();
};
