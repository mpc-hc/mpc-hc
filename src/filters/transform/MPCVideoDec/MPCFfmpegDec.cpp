/*
 * $Id$
 *
 * (C) 2006-2012 see Authors.txt
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
#include "MPCVideoDecFilter.h"
#include "../../../DSUtil/DSUtil.h"


#ifdef REGISTER_FILTER


// Workaround: graphedit crashes when a filter exposes more than 115 input MediaTypes!
const AMOVIESETUP_PIN sudpPinsVideoDec[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, CMPCVideoDecFilter::sudPinTypesInCount > 115 ? 115 : CMPCVideoDecFilter::sudPinTypesInCount,  CMPCVideoDecFilter::sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, CMPCVideoDecFilter::sudPinTypesOutCount, CMPCVideoDecFilter::sudPinTypesOut}
};


const AMOVIESETUP_FILTER sudFilters[] = {
	{&__uuidof(CMPCVideoDecFilter), MPCVideoDecName, /*MERIT_DO_NOT_USE*/0x40000001, countof(sudpPinsVideoDec), sudpPinsVideoDec, CLSID_LegacyAmFilterCategory}
};


CFactoryTemplate g_Templates[] = {
	{sudFilters[0].strName, &__uuidof(CMPCVideoDecFilter), CreateInstance<CMPCVideoDecFilter>, NULL, &sudFilters[0]},
	{L"CMPCVideoDecPropertyPage", &__uuidof(CMPCVideoDecSettingsWnd), CreateInstance<CInternalPropertyPageTempl<CMPCVideoDecSettingsWnd> >},
	{L"CMPCVideoDecPropertyPage2", &__uuidof(CMPCVideoDecCodecWnd), CreateInstance<CInternalPropertyPageTempl<CMPCVideoDecCodecWnd> >},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}


#include "../../FilterApp.h"

CFilterApp theApp;

#endif
