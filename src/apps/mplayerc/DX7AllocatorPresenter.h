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

// {495CF191-810D-44c7-92C5-E7D46AE00F44}
DEFINE_GUID(CLSID_VMR7AllocatorPresenter, 
0x495cf191, 0x810d, 0x44c7, 0x92, 0xc5, 0xe7, 0xd4, 0x6a, 0xe0, 0xf, 0x44);

// {97B3462E-1752-4dfb-A038-271060BC7A94}
DEFINE_GUID(CLSID_RM7AllocatorPresenter, 
0x97b3462e, 0x1752, 0x4dfb, 0xa0, 0x38, 0x27, 0x10, 0x60, 0xbc, 0x7a, 0x94);

// {36CC5A71-441C-462a-9D10-48A19485938D}
DEFINE_GUID(CLSID_QT7AllocatorPresenter, 
0x36cc5a71, 0x441c, 0x462a, 0x9d, 0x10, 0x48, 0xa1, 0x94, 0x85, 0x93, 0x8d);

extern HRESULT CreateAP7(const CLSID& clsid, HWND hWnd, ISubPicAllocatorPresenter** ppAP);

extern bool IsVMR7InGraph(IFilterGraph* pFG);
