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

enum
{
	WM_REARRANGERENDERLESS = WM_APP+1,
	WM_RESET_DEVICE,
};

#define WM_MYMOUSELAST WM_XBUTTONDBLCLK

enum
{
	VIDRNDT_RM_DEFAULT,
	VIDRNDT_RM_DX7,
	VIDRNDT_RM_DX9,
};

enum
{
	VIDRNDT_QT_DEFAULT,
	VIDRNDT_QT_DX7,
	VIDRNDT_QT_DX9,
};

enum
{
	VIDRNDT_AP_SURFACE,
	VIDRNDT_AP_TEXTURE2D,
	VIDRNDT_AP_TEXTURE3D,
};

enum VideoSystem
{
	VIDEO_SYSTEM_UNKNOWN,
	VIDEO_SYSTEM_HDTV,
	VIDEO_SYSTEM_SDTV_NTSC,
	VIDEO_SYSTEM_SDTV_PAL,
};

enum AmbientLight
{
	AMBIENT_LIGHT_BRIGHT,
	AMBIENT_LIGHT_DIM,
	AMBIENT_LIGHT_DARK,
};

enum ColorRenderingIntent
{
	COLOR_RENDERING_INTENT_PERCEPTUAL,
	COLOR_RENDERING_INTENT_RELATIVE_COLORIMETRIC,
	COLOR_RENDERING_INTENT_SATURATION,
	COLOR_RENDERING_INTENT_ABSOLUTE_COLORIMETRIC,
};

class CRenderersSettings
{

public:
	bool fResetDevice;

	class CRendererSettingsShared
	{
	public:
		CRendererSettingsShared()
		{
			SetDefault();
		}
		bool fVMR9AlterativeVSync;
		int iVMR9VSyncOffset;
		bool iVMR9VSyncAccurate;
		bool iVMR9FullscreenGUISupport;
		bool iVMR9VSync;
		bool iVMR9FullFloatingPointProcessing;
		bool iVMR9ColorManagementEnable;
		int iVMR9ColorManagementInput;
		int iVMR9ColorManagementAmbientLight;
		int iVMR9ColorManagementIntent;
		bool iVMRDisableDesktopComposition;
		int iVMRFlushGPUBeforeVSync;
		int iVMRFlushGPUAfterPresent;
		int iVMRFlushGPUWait;

		// SyncRenderer settings
		int bSynchronizeVideo;
		int bSynchronizeDisplay;
		int bSynchronizeNearest;
		int iLineDelta;
		int iColumnDelta;
		double fCycleDelta;
		double fTargetSyncOffset;
		double fControlLimit;

		void SetDefault()
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
		void SetOptimal()
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
	};
	class CRendererSettingsEVR : public CRendererSettingsShared
	{
	public:
		bool iEVRHighColorResolution;
		bool iEVRForceInputHighColorResolution;
		bool iEVREnableFrameTimeCorrection;
		int iEVROutputRange;

		CRendererSettingsEVR()
		{
			SetDefault();
		}
		void SetDefault()
		{
			CRendererSettingsShared::SetDefault();
			iEVRHighColorResolution = 0;
			iEVRForceInputHighColorResolution = 0;
			iEVREnableFrameTimeCorrection = 0;
			iEVROutputRange = 0;
		}
		void SetOptimal()
		{
			CRendererSettingsShared::SetOptimal();
			iEVRHighColorResolution = 0;
			iEVRForceInputHighColorResolution = 0;
		}
	};

	CRendererSettingsEVR m_RenderSettings;

	int			iAPSurfaceUsage;
	//bool		fVMRSyncFix;
	int			iDX9Resizer;
	bool		fVMR9MixerMode;
	bool		fVMR9MixerYUV;
	int			iEvrBuffers;

	int			nSPCSize;
	int			nSPCMaxRes;
	bool		fSPCPow2Tex;
	bool		fSPCAllowAnimationWhenBuffering;

	CString		D3D9RenderDevice;
};


class CRenderersData
{
	HINSTANCE	m_hD3DX9Dll;
	int			m_nDXSdkRelease;

public:
	CRenderersData();

	// Casimir666
	bool		m_fTearingTest;
	int			m_fDisplayStats;
	bool		m_bResetStats; // Set to reset the presentation statistics
	CString		m_strD3DX9Version;

	// Hardware feature support
	bool		m_bFP16Support;
	bool		m_b10bitSupport;

	LONGLONG	GetPerfCounter();
	HINSTANCE	GetD3X9Dll();
	int			GetDXSdkRelease()
	{
		return	m_nDXSdkRelease;
	};
};

extern CRenderersData*		GetRenderersData();
extern CRenderersSettings&	GetRenderersSettings();

extern bool LoadResource(UINT resid, CStringA& str, LPCTSTR restype);
extern "C" BOOL	IsVistaOrAbove();
