//	VDXFrame - Helper library for VirtualDub plugins
//	Copyright (C) 2008 Avery Lee
//
//	The plugin headers in the VirtualDub plugin SDK are licensed differently
//	differently than VirtualDub and the Plugin SDK themselves.  This
//	particular file is thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#ifndef f_VD2_VDXFRAME_VIDEOFILTERDIALOG_H
#define f_VD2_VDXFRAME_VIDEOFILTERDIALOG_H

#include <windows.h>

class VDXVideoFilterDialog {
public:
	VDXVideoFilterDialog();

protected:
	HWND	mhdlg;

	LRESULT Show(HINSTANCE hInst, LPCSTR templName, HWND parent);
	LRESULT Show(HINSTANCE hInst, LPCWSTR templName, HWND parent);
	HWND ShowModeless(HINSTANCE hInst, LPCSTR templName, HWND parent);
	HWND ShowModeless(HINSTANCE hInst, LPCWSTR templName, HWND parent);

	static INT_PTR CALLBACK StaticDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
