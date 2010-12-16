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

#include "stdafx.h"
#include "RenderersSettings.h"
#include "../../../apps/mplayerc/mplayerc.h"
#include <d3dx9.h>

void CRenderersSettings::UpdateData(bool fSave)
{
	AfxGetAppSettings().UpdateRenderersData(fSave);
}

void CRenderersSettings::CRendererSettingsShared::SetDefault()
{
	fVMR9AlterativeVSync = 0;
	iVMR9VSyncOffset = 0;
	iVMR9VSyncAccurate = 1;
	iVMR9FullscreenGUISupport = 0;
	iVMR9VSync = 1;
	iVMR9FullFloatingPointProcessing = 0;
	iVMR9ColorManagementEnable = 0;
	iVMR9ColorManagementInput = VIDEO_SYSTEM_UNKNOWN;
	iVMR9ColorManagementAmbientLight = AMBIENT_LIGHT_BRIGHT;
	iVMR9ColorManagementIntent = COLOR_RENDERING_INTENT_PERCEPTUAL;
	iVMRDisableDesktopComposition = 0;
	iVMRFlushGPUBeforeVSync = 1;
	iVMRFlushGPUAfterPresent = 1;
	iVMRFlushGPUWait = 0;
	bSynchronizeVideo = 0;
	bSynchronizeDisplay = 0;
	bSynchronizeNearest = 1;
	iLineDelta = 0;
	iColumnDelta = 0;
	fCycleDelta = 0.0012;
	fTargetSyncOffset = 12.0;
	fControlLimit = 2.0;
}

void CRenderersSettings::CRendererSettingsShared::SetOptimal()
{
	fVMR9AlterativeVSync = 1;
	iVMR9VSyncAccurate = 1;
	iVMR9VSync = 1;
	iVMR9FullFloatingPointProcessing = 1;
	iVMR9ColorManagementEnable = 0;
	iVMR9ColorManagementInput = VIDEO_SYSTEM_UNKNOWN;
	iVMR9ColorManagementAmbientLight = AMBIENT_LIGHT_BRIGHT;
	iVMR9ColorManagementIntent = COLOR_RENDERING_INTENT_PERCEPTUAL;
	iVMRDisableDesktopComposition = 1;
	iVMRFlushGPUBeforeVSync = 1;
	iVMRFlushGPUAfterPresent = 1;
	iVMRFlushGPUWait = 0;
	bSynchronizeVideo = 0;
	bSynchronizeDisplay = 0;
	bSynchronizeNearest = 1;
	iLineDelta = 0;
	iColumnDelta = 0;
	fCycleDelta = 0.0012;
	fTargetSyncOffset = 12.0;
	fControlLimit = 2.0;
}

void CRenderersSettings::CRendererSettingsEVR::SetDefault()
{
	CRendererSettingsShared::SetDefault();
	iEVRHighColorResolution = 0;
	iEVRForceInputHighColorResolution = 0;
	iEVREnableFrameTimeCorrection = 0;
	iEVROutputRange = 0;
}

void CRenderersSettings::CRendererSettingsEVR::SetOptimal()
{
	CRendererSettingsShared::SetOptimal();
	iEVRHighColorResolution = 0;
	iEVRForceInputHighColorResolution = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CRenderersData construction

CRenderersData::CRenderersData()
{
	m_fTearingTest	= false;
	m_fDisplayStats	= false;
	m_bResetStats	= false;
	m_hD3DX9Dll		= NULL;
	m_nDXSdkRelease	= 0;

	// Don't disable hardware features before initializing a renderer
	m_bFP16Support  = true;
	m_b10bitSupport = true;
}

LONGLONG CRenderersData::GetPerfCounter()
{
	LARGE_INTEGER		i64Ticks100ns;
	LARGE_INTEGER		llPerfFrequency;

	QueryPerformanceFrequency (&llPerfFrequency);
	if (llPerfFrequency.QuadPart != 0)
	{
		QueryPerformanceCounter (&i64Ticks100ns);
		return llMulDiv (i64Ticks100ns.QuadPart, 10000000, llPerfFrequency.QuadPart, 0);
	}
	else
	{
		// ms to 100ns units
		return timeGetTime() * 10000;
	}
}

HINSTANCE CRenderersData::GetD3X9Dll()
{
	if (m_hD3DX9Dll == NULL)
	{
		int min_ver = D3DX_SDK_VERSION;
		int max_ver = D3DX_SDK_VERSION;

		m_nDXSdkRelease = 0;

		if(D3DX_SDK_VERSION >= 42)
		{
			// August 2009 SDK (v42) is not compatible with older versions
			min_ver = 42;
		}
		else
		{
			if(D3DX_SDK_VERSION > 33)
			{
				// versions between 34 and 41 have no known compatibility issues
				min_ver = 34;
			}
			else
			{
				// The minimum version that supports the functionality required by MPC is 24
				min_ver = 24;

				if(D3DX_SDK_VERSION == 33)
				{
					// The April 2007 SDK (v33) should not be used (crash sometimes during shader compilation)
					max_ver = 32;
				}
			}
		}

		// load latest compatible version of the DLL that is available
		for (int i=max_ver; i>=min_ver; i--)
		{
			m_strD3DX9Version.Format(_T("d3dx9_%d.dll"), i);
			m_hD3DX9Dll = LoadLibrary (m_strD3DX9Version);
			if (m_hD3DX9Dll)
			{
				m_nDXSdkRelease = i;
				break;
			}
		}
	}

	return m_hD3DX9Dll;
}
