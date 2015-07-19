/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015 see Authors.txt
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
#include "../../../mpc-hc/AppSettings.h"
#include "../../../mpc-hc/mplayerc.h"
#include "../../../DSUtil/SysVersion.h"
#include "version.h"
#include <d3dx9.h>

void CRenderersSettings::UpdateData(bool fSave)
{
    AfxGetAppSettings().UpdateRenderersData(fSave);
}

void CRenderersSettings::CAdvRendererSettings::SetDefault()
{
    bVMR9AlterativeVSync              = false;
    iVMR9VSyncOffset                  = 0;
    bVMR9VSyncAccurate                = false;
    bVMR9FullscreenGUISupport         = false;
    bVMR9VSync                        = !SysVersion::IsVistaOrLater();
    bVMR9FullFloatingPointProcessing  = false;
    bVMR9HalfFloatingPointProcessing  = false;
    bVMR9ColorManagementEnable        = false;
    iVMR9ColorManagementInput         = VIDEO_SYSTEM_UNKNOWN;
    iVMR9ColorManagementAmbientLight  = AMBIENT_LIGHT_BRIGHT;
    iVMR9ColorManagementIntent        = COLOR_RENDERING_INTENT_PERCEPTUAL;
    bVMRDisableDesktopComposition     = false;
    bVMRFlushGPUBeforeVSync           = true;
    bVMRFlushGPUAfterPresent          = true;
    bVMRFlushGPUWait                  = false;
    bEVRHighColorResolution           = false;
    bEVRForceInputHighColorResolution = false;
    bEVREnableFrameTimeCorrection     = false;
    iEVROutputRange                   = 0;
    bSynchronizeVideo                 = false;
    bSynchronizeDisplay               = false;
    bSynchronizeNearest               = true;
    iLineDelta                        = 0;
    iColumnDelta                      = 0;
    fCycleDelta                       = 0.0012;
    fTargetSyncOffset                 = 12.0;
    fControlLimit                     = 2.0;
}

void CRenderersSettings::CAdvRendererSettings::SetOptimal()
{
    bVMR9AlterativeVSync              = true;
    iVMR9VSyncOffset                  = 0;
    bVMR9VSyncAccurate                = true;
    bVMR9FullscreenGUISupport         = false;
    bVMR9VSync                        = true;
    bVMR9FullFloatingPointProcessing  = true;
    bVMR9HalfFloatingPointProcessing  = false;
    bVMR9ColorManagementEnable        = false;
    iVMR9ColorManagementInput         = VIDEO_SYSTEM_UNKNOWN;
    iVMR9ColorManagementAmbientLight  = AMBIENT_LIGHT_BRIGHT;
    iVMR9ColorManagementIntent        = COLOR_RENDERING_INTENT_PERCEPTUAL;
    bVMRDisableDesktopComposition     = true;
    bVMRFlushGPUBeforeVSync           = true;
    bVMRFlushGPUAfterPresent          = true;
    bVMRFlushGPUWait                  = false;
    bEVRHighColorResolution           = false;
    bEVRForceInputHighColorResolution = false;
    bEVREnableFrameTimeCorrection     = false;
    iEVROutputRange                   = 0;
    bSynchronizeVideo                 = false;
    bSynchronizeDisplay               = false;
    bSynchronizeNearest               = true;
    iLineDelta                        = 0;
    iColumnDelta                      = 0;
    fCycleDelta                       = 0.0012;
    fTargetSyncOffset                 = 12.0;
    fControlLimit                     = 2.0;
}

/////////////////////////////////////////////////////////////////////////////
// CRenderersData construction

CRenderersData::CRenderersData()
    : m_hD3DX9Dll(nullptr)
    , m_nDXSdkRelease(0)
    , m_bTearingTest(false)
    , m_iDisplayStats(0)
    , m_bResetStats(false)
      // Don't disable hardware features before initializing a renderer
    , m_bFP16Support(true)
    , m_bFP32Support(true)
    , m_b10bitSupport(true)
{
    QueryPerformanceFrequency(&llPerfFrequency);
}

LONGLONG CRenderersData::GetPerfCounter() const
{
    LARGE_INTEGER i64Ticks100ns;

    QueryPerformanceCounter(&i64Ticks100ns);
    return llMulDiv(i64Ticks100ns.QuadPart, 10000000, llPerfFrequency.QuadPart, 0);
}

HINSTANCE CRenderersData::GetD3X9Dll()
{
#if D3DX_SDK_VERSION < MPC_DX_SDK_NUMBER
#pragma message("ERROR: DirectX SDK " MPC_DX_SDK_MONTH " " MAKE_STR(MPC_DX_SDK_YEAR) " (v" MAKE_STR(MPC_DX_SDK_NUMBER) ") or newer is required to build MPC-HC")
#endif

    if (m_hD3DX9Dll == nullptr) {
        m_nDXSdkRelease = 0;

        // load latest compatible version of the DLL that is available
        for (UINT i = D3DX_SDK_VERSION; i >= MPC_DX_SDK_NUMBER; i--) {
            m_strD3DX9Version.Format(_T("d3dx9_%u.dll"), i);
            m_hD3DX9Dll = LoadLibrary(m_strD3DX9Version);
            if (m_hD3DX9Dll) {
                m_nDXSdkRelease = i;
                break;
            }
        }
    }

    return m_hD3DX9Dll;
}
