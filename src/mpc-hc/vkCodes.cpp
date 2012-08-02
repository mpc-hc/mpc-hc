/*
 * (C) 2011-2012 see Authors.txt
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
#include "vkCodes.h"

PCTSTR GetKeyName(UINT vkCode)
{
    ASSERT(vkCode < 256);
    vkCode &= 0xff;

    static PCTSTR s_pszKeys[256] = {
        _T("Unused"),
        _T("Left mouse button"),
        _T("Right mouse button"),
        _T("Control-break"),
        _T("Middle mouse button"),
        _T("X1 mouse button"),
        _T("X2 mouse button"),
        _T("Undefined"),
        _T("Backspace"),
        _T("Tab"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Clear"),
        _T("Enter"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Shift"),
        _T("Control"),
        _T("Alt"),
        _T("Pause"),
        _T("Caps Lock"),
        _T("IME Kana mode"),
        _T("Unknown"),
        _T("IME Junja mode"),
        _T("IME final mode"),
        _T("IME Hanja mode"),
        _T("Unknown"),
        _T("Esc"),
        _T("IME convert"),
        _T("IME nonconvert"),
        _T("IME accept"),
        _T("IME mode change"),
        _T("Space"),
        _T("Page Up"),
        _T("Page Down"),
        _T("End"),
        _T("Home"),
        _T("Left Arrow"),
        _T("Up Arrow"),
        _T("Right Arrow"),
        _T("Down Arrow"),
        _T("Select"),
        _T("Print"),
        _T("Execute"),
        _T("Print Screen"),
        _T("Ins"),
        _T("Del"),
        _T("Help"),
        _T("0"),
        _T("1"),
        _T("2"),
        _T("3"),
        _T("4"),
        _T("5"),
        _T("6"),
        _T("7"),
        _T("8"),
        _T("9"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("A"),
        _T("B"),
        _T("C"),
        _T("D"),
        _T("E"),
        _T("F"),
        _T("G"),
        _T("H"),
        _T("I"),
        _T("J"),
        _T("K"),
        _T("L"),
        _T("M"),
        _T("N"),
        _T("O"),
        _T("P"),
        _T("Q"),
        _T("R"),
        _T("S"),
        _T("T"),
        _T("U"),
        _T("V"),
        _T("W"),
        _T("X"),
        _T("Y"),
        _T("Z"),
        _T("Left Win"),
        _T("Right Win"),
        _T("App"),
        _T("Unknown"),
        _T("Sleep"),
        _T("Num 0"),
        _T("Num 1"),
        _T("Num 2"),
        _T("Num 3"),
        _T("Num 4"),
        _T("Num 5"),
        _T("Num 6"),
        _T("Num 7"),
        _T("Num 8"),
        _T("Num 9"),
        _T("Mul"),
        _T("Add"),
        _T("Separator"),
        _T("Sub"),
        _T("Decimal"),
        _T("Div"),
        _T("F1"),
        _T("F2"),
        _T("F3"),
        _T("F4"),
        _T("F5"),
        _T("F6"),
        _T("F7"),
        _T("F8"),
        _T("F9"),
        _T("F10"),
        _T("F11"),
        _T("F12"),
        _T("F13"),
        _T("F14"),
        _T("F15"),
        _T("F16"),
        _T("F17"),
        _T("F18"),
        _T("F19"),
        _T("F20"),
        _T("F21"),
        _T("F22"),
        _T("F23"),
        _T("F24"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Num Lock"),
        _T("Scroll Lock"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Left Shift"),
        _T("Right Shift"),
        _T("Left Control"),
        _T("Right Control"),
        _T("Left Alt"),
        _T("Right Alt"),
        _T("Browser Back"),
        _T("Browser Forward"),
        _T("Browser Refresh"),
        _T("Browser Stop"),
        _T("Browser Search"),
        _T("Browser Favorites"),
        _T("Browser Home"),
        _T("Volume Mute"),
        _T("Volume Down"),
        _T("Volume Up"),
        _T("Next Track"),
        _T("Previous Track"),
        _T("Stop Media"),
        _T("Play/Pause Media"),
        _T("Start Mail"),
        _T("Select Media"),
        _T("Start App 1"),
        _T("Start App 2"),
        _T("Unknown"),
        _T("Unknown"),
        _T(";"),
        _T("="),
        _T(","),
        _T("_"),
        _T("."),
        _T("/"),
        _T("`"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("Unknown"),
        _T("["),
        _T("\\"),
        _T("]"),
        _T("'"),
        _T("OEM"),
        _T("Unknown"),
        _T("OEM"),
        _T("<> or \\|"),
        _T("OEM"),
        _T("OEM"),
        _T("IME Process key"),
        _T("OEM"),
        _T("VK_PACKET"),
        _T("Unknown"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("OEM"),
        _T("Attn"),
        _T("CrSel"),
        _T("ExSel"),
        _T("Erase EOF"),
        _T("Play"),
        _T("Zoom"),
        _T("Unknown"),
        _T("PA1"),
        _T("Clear"),
        _T("Unknown")
    };

    return s_pszKeys[vkCode];
}

//-----------------------------------------------------------------

BOOL HotkeyToString(UINT vkCode, UINT fModifiers, CString& s)
{

    s.Empty();
    if (fModifiers & MOD_CONTROL) {
        s += _T("Ctrl + ");
    }
    if (fModifiers & MOD_ALT) {
        s += _T("Alt + ");
    }
    if (fModifiers & MOD_SHIFT) {
        s += _T("Shift + ");
    }
    if (vkCode) {
        s += GetKeyName(vkCode);
    }

    return !s.IsEmpty();
}

BOOL HotkeyModToString(UINT vkCode, BYTE fModifiers, CString& s)
{
    s.Empty();

    if (fModifiers & FCONTROL) {
        s += _T("Ctrl + ");
    }
    if (fModifiers & FALT) {
        s += _T("Alt + ");
    }
    if (fModifiers & FSHIFT) {
        s += _T("Shift + ");
    }
    if (vkCode) {
        s += GetKeyName(vkCode);
    }

    return !s.IsEmpty();
}

BOOL HotkeyToString(DWORD dwHk, CString& s)
{
    return HotkeyToString(LOBYTE(LOWORD(dwHk)), HIBYTE(LOWORD(dwHk)), s);
}
