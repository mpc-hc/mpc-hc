/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include "RenderersSettings.h"
#include "../../../mpc-hc/mplayerc.h"
#include <Version.h>
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
	iVMR9HalfFloatingPointProcessing = 0;
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
	iVMR9HalfFloatingPointProcessing = 0;
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
	m_fTearingTest  = false;
	m_fDisplayStats = false;
	m_bResetStats   = false;
	m_hD3DX9Dll     = NULL;
	m_nDXSdkRelease = 0;

	// Don't disable hardware features before initializing a renderer
	m_bFP16Support  = true;
	m_b10bitSupport = true;
}

LONGLONG CRenderersData::GetPerfCounter()
{
	LARGE_INTEGER i64Ticks100ns;
	LARGE_INTEGER llPerfFrequency;

	QueryPerformanceFrequency(&llPerfFrequency);
	if (llPerfFrequency.QuadPart != 0) {
		QueryPerformanceCounter(&i64Ticks100ns);
		return llMulDiv(i64Ticks100ns.QuadPart, 10000000, llPerfFrequency.QuadPart, 0);
	} else {
		// ms to 100ns units
		return timeGetTime() * 10000;
	}
}

HINSTANCE CRenderersData::GetD3X9Dll()
{
#if D3DX_SDK_VERSION < MPC_DX_SDK_NUMBER
#pragma message("ERROR: DirectX SDK " MPC_DX_SDK_MONTH " " MAKE_STR(MPC_DX_SDK_YEAR) " (v" MAKE_STR(MPC_DX_SDK_NUMBER) ") or newer is required to build MPC-HC")
#endif

	if (m_hD3DX9Dll == NULL) {
		m_nDXSdkRelease = 0;

		// load latest compatible version of the DLL that is available
		for (int i=D3DX_SDK_VERSION; i>=MPC_DX_SDK_NUMBER; i--) {
			m_strD3DX9Version.Format(_T("d3dx9_%d.dll"), i);
			m_hD3DX9Dll = LoadLibrary(m_strD3DX9Version);
			if (m_hD3DX9Dll) {
				m_nDXSdkRelease = i;
				break;
			}
		}
	}

	return m_hD3DX9Dll;
}
