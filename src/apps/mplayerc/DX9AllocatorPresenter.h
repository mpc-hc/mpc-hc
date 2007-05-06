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

#include "..\..\SubPic\ISubPic.h"

// {4E4834FA-22C2-40e2-9446-F77DD05D245E}
DEFINE_GUID(CLSID_VMR9AllocatorPresenter,
0x4e4834fa, 0x22c2, 0x40e2, 0x94, 0x46, 0xf7, 0x7d, 0xd0, 0x5d, 0x24, 0x5e);

// {A1542F93-EB53-4e11-8D34-05C57ABA9207}
DEFINE_GUID(CLSID_RM9AllocatorPresenter,
0xa1542f93, 0xeb53, 0x4e11, 0x8d, 0x34, 0x5, 0xc5, 0x7a, 0xba, 0x92, 0x7);

// {622A4032-70CE-4040-8231-0F24F2886618}
DEFINE_GUID(CLSID_QT9AllocatorPresenter,
0x622a4032, 0x70ce, 0x4040, 0x82, 0x31, 0xf, 0x24, 0xf2, 0x88, 0x66, 0x18);

// {B72EBDD4-831D-440f-A656-B48F5486CD82}
DEFINE_GUID(CLSID_DXRAllocatorPresenter, 
0xb72ebdd4, 0x831d, 0x440f, 0xa6, 0x56, 0xb4, 0x8f, 0x54, 0x86, 0xcd, 0x82);

extern HRESULT CreateAP9(const CLSID& clsid, HWND hWnd, ISubPicAllocatorPresenter** ppAP);

extern bool IsVMR9InGraph(IFilterGraph* pFG);

// Support ffdshow queueing.
// This interface is used to check version of Media Player Classic.
// {A273C7F6-25D4-46b0-B2C8-4F7FADC44E37}
DEFINE_GUID(IID_IVMRffdshow9,
0xa273c7f6, 0x25d4, 0x46b0, 0xb2, 0xc8, 0x4f, 0x7f, 0xad, 0xc4, 0x4e, 0x37);

MIDL_INTERFACE("A273C7F6-25D4-46b0-B2C8-4F7FADC44E37")
IVMRffdshow9 : public IUnknown
{
public:
	virtual STDMETHODIMP support_ffdshow(void) = 0;
};

extern CCritSec g_ffdshowReceive;
extern bool queueu_ffdshow_support;
