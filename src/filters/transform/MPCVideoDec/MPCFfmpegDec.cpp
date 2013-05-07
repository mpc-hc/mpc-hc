/*
 * (C) 2007-2013 see Authors.txt
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
#include "MPCVideoDecFilter.h"
#include "../../../DSUtil/DSUtil.h"

#ifdef STANDALONE_FILTER

// Workaround: graphedit crashes when a filter exposes more than 115 input MediaTypes!
const AMOVIESETUP_PIN sudpPinsVideoDec[] = {
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, nullptr, CMPCVideoDecFilter::sudPinTypesInCount > 115 ? 115 : CMPCVideoDecFilter::sudPinTypesInCount,  CMPCVideoDecFilter::sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, CMPCVideoDecFilter::sudPinTypesOutCount, CMPCVideoDecFilter::sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilters[] = {
    {&__uuidof(CMPCVideoDecFilter), MPCVideoDecName, /*MERIT_DO_NOT_USE*/0x40000001, _countof(sudpPinsVideoDec), sudpPinsVideoDec, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
    {sudFilters[0].strName, &__uuidof(CMPCVideoDecFilter), CreateInstance<CMPCVideoDecFilter>, nullptr, &sudFilters[0]},
    {L"CMPCVideoDecPropertyPage", &__uuidof(CMPCVideoDecSettingsWnd), CreateInstance<CInternalPropertyPageTempl<CMPCVideoDecSettingsWnd>>},
    {L"CMPCVideoDecPropertyPage2", &__uuidof(CMPCVideoDecCodecWnd), CreateInstance<CInternalPropertyPageTempl<CMPCVideoDecCodecWnd>>},
};

int g_cTemplates = _countof(g_Templates);

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
