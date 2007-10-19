/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
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

#include "..\..\InternalPropertyPage.h"
#include "IMPCVideoDecFilter.h"
#include <afxcmn.h>

[uuid("D5AA0389-D274-48e1-BF50-ACB05A56DDE0")]
class CMPCVideoDecSettingsWnd : public CInternalPropertyPageWnd
{
	CComQIPtr<IMPCVideoDecFilter> m_pMDF;

public:
	CMPCVideoDecSettingsWnd();
	
	bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
	void OnDisconnect();
	bool OnActivate();
	void OnDeactivate();
	bool OnApply();

	static LPCTSTR GetWindowTitle() {return _T("Settings");}
	static CSize GetWindowSize() {return CSize(320, 240);}

	DECLARE_MESSAGE_MAP()
};