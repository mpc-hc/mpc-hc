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


#include "stdafx.h"
#include "WinHotkeyCtrl.h"
#include "vkCodes.h"

#define WM_KEY	(WM_USER + 444)

// CWinHotkeyCtrl

HHOOK CWinHotkeyCtrl::sm_hhookKb = NULL;
CWinHotkeyCtrl* CWinHotkeyCtrl::sm_pwhcFocus = NULL;


IMPLEMENT_DYNAMIC(CWinHotkeyCtrl, CEdit)
CWinHotkeyCtrl::CWinHotkeyCtrl(): 
	m_vkCode(0),
	m_fModSet(0),
	m_fModRel(0),
	m_fIsPressed(FALSE)
{
}

CWinHotkeyCtrl::~CWinHotkeyCtrl()
{
}

BEGIN_MESSAGE_MAP(CWinHotkeyCtrl, CEdit)
	ON_MESSAGE(WM_KEY, OnKey)
	ON_WM_CHAR()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()	
END_MESSAGE_MAP()

// CWinHotkeyCtrl

void CWinHotkeyCtrl::PreSubclassWindow()
{
	CEdit::PreSubclassWindow();
	UpdateText();
}

#if _WIN32_WINNT < 0x500

LRESULT CALLBACK CWinHotkeyCtrl::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
	if (nCode == HC_ACTION && sm_pwhcFocus) {
		if(((PKBDLLHOOKSTRUCT)lParam)->vkCode == VK_ESCAPE) {
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		}
		sm_pwhcFocus->PostMessage(WM_KEY, wParam, (lParam & 0x80000000));
	}
	return(1);
}

#else // _WIN32_WINNT >= 0x500

LRESULT CALLBACK CWinHotkeyCtrl::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN ||
			wParam == WM_KEYUP || wParam == WM_SYSKEYUP) && sm_pwhcFocus) {
		if(((PKBDLLHOOKSTRUCT)lParam)->vkCode == VK_ESCAPE) {
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		}
		sm_pwhcFocus->PostMessage(WM_KEY, ((PKBDLLHOOKSTRUCT)lParam)->vkCode, (wParam & 1));
	}
	return(1);
}

#endif // _WIN32_WINNT >= 0x500


BOOL CWinHotkeyCtrl::InstallKbHook()
{
	if (sm_pwhcFocus && sm_hhookKb) {
		sm_pwhcFocus->UninstallKbHook();
	}
	sm_pwhcFocus = this;

#if _WIN32_WINNT < 0x500
	sm_hhookKb = ::SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardProc, NULL, GetCurrentThreadId());
#else // _WIN32_WINNT >= 0x500
	sm_hhookKb = ::SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, GetModuleHandle(NULL), NULL);
#endif // _WIN32_WINNT >= 0x500

	return(sm_hhookKb != NULL);
}

BOOL CWinHotkeyCtrl::UninstallKbHook()
{
	BOOL fOk = FALSE;
	if (sm_hhookKb) {
		fOk = ::UnhookWindowsHookEx(sm_hhookKb);
		sm_hhookKb = NULL;
	}
	sm_pwhcFocus = NULL;
	return(fOk);
}


void CWinHotkeyCtrl::UpdateText()
{
	CString sText;
	
	SetWindowText(HotkeyToString(m_vkCode, m_fModSet, sText) ? (LPCTSTR)sText : _T("None"));
	SetSel(0x8fffffff, 0x8fffffff, FALSE);
}

DWORD CWinHotkeyCtrl::GetWinHotkey()
{
	return(MAKEWORD(m_vkCode, m_fModSet));
}

BOOL CWinHotkeyCtrl::GetWinHotkey(UINT* pvkCode, UINT* pfModifiers)
{
	*pvkCode = m_vkCode;
	*pfModifiers = m_fModSet;
	return(m_vkCode != 0);
}

void CWinHotkeyCtrl::SetWinHotkey(DWORD dwHk)
{
	m_vkCode = LOBYTE(LOWORD(dwHk));
	m_fModSet = m_fModRel = HIBYTE(LOWORD(dwHk));
	m_fIsPressed = FALSE;
	UpdateText();
}

void CWinHotkeyCtrl::SetWinHotkey(UINT vkCode, UINT fModifiers)
{
	m_vkCode = vkCode;
	m_fModSet = m_fModRel = fModifiers;
	m_fIsPressed = FALSE;
	UpdateText();
}

LRESULT CWinHotkeyCtrl::OnKey(WPARAM wParam, LPARAM lParam)
{
	DWORD fMod = 0;
	BOOL fRedraw = TRUE;

	switch (wParam) {
		case VK_CONTROL:
		case VK_LCONTROL:
		case VK_RCONTROL: fMod = MOD_CONTROL; break;
		case VK_MENU:
		case VK_LMENU: 
		case VK_RMENU: fMod = MOD_ALT; break;
		case VK_SHIFT:
		case VK_LSHIFT:
		case VK_RSHIFT: fMod = MOD_SHIFT; break;
	}

	if (fMod) { // modifier
		if (!lParam) { // press
			if (!m_fIsPressed && m_vkCode) {
				m_fModSet = m_fModRel = 0;
				m_vkCode = 0;
			} 
			m_fModRel &= ~fMod;
		} else if (m_fModSet & fMod) // release
			m_fModRel |= fMod;

		if (m_fIsPressed || !m_vkCode) {
			if(!lParam) { // press
				if(!(m_fModSet & fMod)) { // new modifier
					m_fModSet |= fMod;
				} else
					fRedraw = FALSE;
			} else m_fModSet &= ~fMod;
		}
	} else { // another key
		if (wParam == VK_DELETE && m_fModSet == (MOD_CONTROL | MOD_ALT) || // skip "Ctrl + Alt + Del"
			 (wParam == VK_LWIN || wParam == VK_RWIN)) { // skip "Win"
			m_fModSet = m_fModRel = 0;
			m_vkCode = 0;
			m_fIsPressed = FALSE;
		} else if (wParam == m_vkCode && lParam) {
			m_fIsPressed = FALSE;
			fRedraw = FALSE;
		} else {
			if (!m_fIsPressed && !lParam) { // pressed a another key
				if (m_fModRel & m_fModSet) {
					m_fModSet = m_fModRel = 0;
				}
				m_vkCode = (UINT)wParam;
				m_fIsPressed = TRUE;
			}
		}
	}
	if (fRedraw)
		UpdateText();

	return(0L);
}

// CWinHotkeyCtrl message handlers

void CWinHotkeyCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

BOOL CWinHotkeyCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	return(FALSE);
}

void CWinHotkeyCtrl::OnSetFocus(CWnd* pOldWnd)
{
	InstallKbHook();
	CEdit::OnSetFocus(pOldWnd);
}

void CWinHotkeyCtrl::OnKillFocus(CWnd* pNewWnd)
{
	UninstallKbHook();
	CEdit::OnKillFocus(pNewWnd);
}

void CWinHotkeyCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint pt)
{
	HMENU hmenu = CreatePopupMenu();

	AppendMenu(hmenu, MF_STRING, 1, ResStr(IDS_PLAYLIST_CLEAR));

	UINT uMenuID = TrackPopupMenu(hmenu, 
		TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
		pt.x, pt.y, 0, GetSafeHwnd(), NULL);

	if (uMenuID) {
		switch (uMenuID) {
			case 1:
				m_fModSet = m_fModRel = 0;
				m_vkCode = 0;
				m_fIsPressed = FALSE;
				break;
		}
		UpdateText();
		SetFocus();
	}
	DestroyMenu(hmenu);
}

void CWinHotkeyCtrl::OnDestroy()
{
	if (sm_pwhcFocus == this)
		sm_pwhcFocus->UninstallKbHook();
	CEdit::OnDestroy();
}
