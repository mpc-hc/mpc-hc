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

class COpenDirHelper {
public:
	static WNDPROC CBProc;
	static bool m_incl_subdir;
	static CString strLastOpenDir;

	static void SetFont(HWND hwnd,LPTSTR FontName,int FontSize);
	// Subclass procedure
	static LRESULT APIENTRY CheckBoxSubclassProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

	static int __stdcall BrowseCallbackProcDIR(HWND  hwnd,UINT  uMsg,LPARAM  lParam,LPARAM  lpData);

	static void RecurseAddDir(CString path, CAtlList<CString>* sl);
};
