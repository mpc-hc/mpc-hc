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
#include "mplayerc.h"
#include <atlsync.h>
#include <Tlhelp32.h>
#include "MainFrm.h"
#include "../../DSUtil/DSUtil.h"
#include "Struct.h"
#include "FileVersionInfo.h"
#include <psapi.h>
#include "Ifo.h"
#include "MiniDump.h"
#include "SettingsDefines.h"
#include "internal_filter_config.h"

#include "Monitors.h"
#include "AppSettings.h"
// Settings

CAppSettings::CAppSettings()
	: fInitialized(false)
	, MRU(0, _T("Recent File List"), _T("File%d"), 20)
	, MRUDub(0, _T("Recent Dub List"), _T("Dub%d"), 20)
	, hAccel(NULL)
	, nCmdlnWebServerPort(-1)
	, ShowDebugInfo(false)
{
}



void CAppSettings::CreateCommands()
{
#define ADDCMD(cmd) wmcmds.AddTail(wmcmd##cmd)
	ADDCMD((ID_FILE_OPENQUICK,					'Q', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_0));
	ADDCMD((ID_FILE_OPENMEDIA,					'O', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_OPEN_FILE));
	ADDCMD((ID_FILE_OPENDVD,					'D', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_OPEN_DVD));
	ADDCMD((ID_FILE_OPENDEVICE,					'V', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_OPEN_DEVICE));
	ADDCMD((ID_FILE_REOPEN,						'E', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_REOPEN));
	ADDCMD((ID_FILE_SAVE_COPY,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_SAVE_AS));
	ADDCMD((ID_FILE_SAVE_IMAGE,					'I', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_SAVE_IMAGE));
	ADDCMD((ID_FILE_SAVE_IMAGE_AUTO,		  VK_F5, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_6));
	ADDCMD((ID_FILE_LOAD_SUBTITLE,				'L', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_LOAD_SUBTITLE));
	ADDCMD((ID_FILE_SAVE_SUBTITLE,				'S', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_SAVE_SUBTITLE));
	ADDCMD((ID_FILE_CLOSEPLAYLIST,				'C', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_CLOSE));
	ADDCMD((ID_FILE_PROPERTIES,				 VK_F10, FVIRTKEY|FSHIFT|FNOINVERT,			IDS_AG_PROPERTIES));
	ADDCMD((ID_FILE_EXIT,						'X', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_EXIT));
	ADDCMD((ID_PLAY_PLAYPAUSE,			   VK_SPACE, FVIRTKEY|FNOINVERT,				IDS_AG_PLAYPAUSE,	APPCOMMAND_MEDIA_PLAY_PAUSE, wmcmd::LDOWN));
	ADDCMD((ID_PLAY_PLAY,						  0, FVIRTKEY|FNOINVERT,				IDS_AG_PLAY,		APPCOMMAND_MEDIA_PLAY));
	ADDCMD((ID_PLAY_PAUSE,						  0, FVIRTKEY|FNOINVERT,				IDS_AG_PAUSE,		APPCOMMAND_MEDIA_PAUSE));
	ADDCMD((ID_PLAY_STOP,			  VK_OEM_PERIOD, FVIRTKEY|FNOINVERT,				IDS_AG_STOP,		APPCOMMAND_MEDIA_STOP));
	ADDCMD((ID_PLAY_FRAMESTEP,			   VK_RIGHT, FVIRTKEY|FNOINVERT,				IDS_AG_FRAMESTEP));
	ADDCMD((ID_PLAY_FRAMESTEPCANCEL,		VK_LEFT, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_16));
	ADDCMD((ID_PLAY_GOTO,						'G', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_GO_TO));
	ADDCMD((ID_PLAY_INCRATE,				  VK_UP, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_INCREASE_RATE));
	ADDCMD((ID_PLAY_DECRATE,				VK_DOWN, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_DECREASE_RATE));
	ADDCMD((ID_PLAY_RESETRATE,					'R', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_RESET_RATE));
	ADDCMD((ID_PLAY_INCAUDDELAY,			 VK_ADD, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_21));
	ADDCMD((ID_PLAY_DECAUDDELAY,		VK_SUBTRACT, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_22));
	ADDCMD((ID_PLAY_SEEKFORWARDSMALL,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_23));
	ADDCMD((ID_PLAY_SEEKBACKWARDSMALL,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_24));
	ADDCMD((ID_PLAY_SEEKFORWARDMED,		   VK_RIGHT, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_25));
	ADDCMD((ID_PLAY_SEEKBACKWARDMED,		VK_LEFT, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_26));
	ADDCMD((ID_PLAY_SEEKFORWARDLARGE,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_27));
	ADDCMD((ID_PLAY_SEEKBACKWARDLARGE,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_28));
	ADDCMD((ID_PLAY_SEEKKEYFORWARD,		   VK_RIGHT, FVIRTKEY|FSHIFT|FNOINVERT,			IDS_MPLAYERC_29));
	ADDCMD((ID_PLAY_SEEKKEYBACKWARD,		VK_LEFT, FVIRTKEY|FSHIFT|FNOINVERT,			IDS_MPLAYERC_30));
	ADDCMD((ID_NAVIGATE_SKIPFORWARD,		VK_NEXT, FVIRTKEY|FNOINVERT,				IDS_AG_NEXT, APPCOMMAND_MEDIA_NEXTTRACK, wmcmd::X2DOWN));
	ADDCMD((ID_NAVIGATE_SKIPBACK,		   VK_PRIOR, FVIRTKEY|FNOINVERT,				IDS_AG_PREVIOUS, APPCOMMAND_MEDIA_PREVIOUSTRACK, wmcmd::X1DOWN));
	ADDCMD((ID_NAVIGATE_SKIPFORWARDPLITEM,  VK_NEXT, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_33));
	ADDCMD((ID_NAVIGATE_SKIPBACKPLITEM,	   VK_PRIOR, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_34));
	ADDCMD((ID_NAVIGATE_TUNERSCAN,				'T', FVIRTKEY|FSHIFT|FNOINVERT,			IDS_NAVIGATE_TUNERSCAN));
	ADDCMD((ID_FAVORITES_QUICKADDFAVORITE,		'Q', FVIRTKEY|FSHIFT|FNOINVERT,			IDS_FAVORITES_QUICKADDFAVORITE));
	ADDCMD((ID_VIEW_CAPTIONMENU,				'0', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_CAPTION));
	ADDCMD((ID_VIEW_SEEKER,						'1', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_SEEKER));
	ADDCMD((ID_VIEW_CONTROLS,					'2', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_CONTROLS));
	ADDCMD((ID_VIEW_INFORMATION,				'3', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_INFO));
	ADDCMD((ID_VIEW_STATISTICS,					'4', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_STATS));
	ADDCMD((ID_VIEW_STATUS,						'5', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_STATUS));
	ADDCMD((ID_VIEW_SUBRESYNC,					'6', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_SUBRESYNC));
	ADDCMD((ID_VIEW_PLAYLIST,					'7', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_PLAYLIST));
	ADDCMD((ID_VIEW_CAPTURE,					'8', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_CAPTURE));
	ADDCMD((ID_VIEW_SHADEREDITOR,				'9', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_SHADER));
	ADDCMD((ID_VIEW_PRESETS_MINIMAL,			'1', FVIRTKEY|FNOINVERT,				IDS_AG_VIEW_MINIMAL));
	ADDCMD((ID_VIEW_PRESETS_COMPACT,			'2', FVIRTKEY|FNOINVERT,				IDS_AG_VIEW_COMPACT));
	ADDCMD((ID_VIEW_PRESETS_NORMAL,				'3', FVIRTKEY|FNOINVERT,				IDS_AG_VIEW_NORMAL));
	ADDCMD((ID_VIEW_FULLSCREEN,			  VK_RETURN, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_FULLSCREEN, 0, wmcmd::LDBLCLK));
	ADDCMD((ID_VIEW_FULLSCREEN_SECONDARY, VK_RETURN, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_39));
	ADDCMD((ID_VIEW_ZOOM_50,					'1', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_ZOOM_50));
	ADDCMD((ID_VIEW_ZOOM_100,					'2', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_ZOOM_100));
	ADDCMD((ID_VIEW_ZOOM_200,					'3', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_ZOOM_200));
	ADDCMD((ID_VIEW_ZOOM_AUTOFIT,				'4', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_ZOOM_AUTO_FIT));
	ADDCMD((ID_ASPECTRATIO_NEXT,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_NEXT_AR_PRESET));
	ADDCMD((ID_VIEW_VF_HALF,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_HALF));
	ADDCMD((ID_VIEW_VF_NORMAL,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_NORMAL));
	ADDCMD((ID_VIEW_VF_DOUBLE,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_DOUBLE));
	ADDCMD((ID_VIEW_VF_STRETCH,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_STRETCH));
	ADDCMD((ID_VIEW_VF_FROMINSIDE,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_INSIDE));
	ADDCMD((ID_VIEW_VF_ZOOM1,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_ZOOM1));
	ADDCMD((ID_VIEW_VF_ZOOM2,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_ZOOM2));
	ADDCMD((ID_VIEW_VF_FROMOUTSIDE,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_OUTSIDE));
	ADDCMD((ID_VIEW_VF_SWITCHZOOM,				'P', FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_SWITCHZOOM));
	ADDCMD((ID_ONTOP_ALWAYS,					'A', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_ALWAYS_ON_TOP));
	ADDCMD((ID_VIEW_RESET,				 VK_NUMPAD5, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_RESET));
	ADDCMD((ID_VIEW_INCSIZE,			 VK_NUMPAD9, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_INC_SIZE));
	ADDCMD((ID_VIEW_INCWIDTH,			 VK_NUMPAD6, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_INC_WIDTH));
	ADDCMD((ID_VIEW_INCHEIGHT,			 VK_NUMPAD8, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_47));
	ADDCMD((ID_VIEW_DECSIZE,			 VK_NUMPAD1, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_DEC_SIZE));
	ADDCMD((ID_VIEW_DECWIDTH,			 VK_NUMPAD4, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_DEC_WIDTH));
	ADDCMD((ID_VIEW_DECHEIGHT,			 VK_NUMPAD2, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_50));
	ADDCMD((ID_PANSCAN_CENTER,			 VK_NUMPAD5, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_CENTER));
	ADDCMD((ID_PANSCAN_MOVELEFT,		 VK_NUMPAD4, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_LEFT));
	ADDCMD((ID_PANSCAN_MOVERIGHT,		 VK_NUMPAD6, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_RIGHT));
	ADDCMD((ID_PANSCAN_MOVEUP,			 VK_NUMPAD8, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_UP));
	ADDCMD((ID_PANSCAN_MOVEDOWN,		 VK_NUMPAD2, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_DOWN));
	ADDCMD((ID_PANSCAN_MOVEUPLEFT,		 VK_NUMPAD7, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_UPLEFT));
	ADDCMD((ID_PANSCAN_MOVEUPRIGHT,		 VK_NUMPAD9, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_UPRIGHT));
	ADDCMD((ID_PANSCAN_MOVEDOWNLEFT,	 VK_NUMPAD1, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_DOWNLEFT));
	ADDCMD((ID_PANSCAN_MOVEDOWNRIGHT,	 VK_NUMPAD3, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_59));
	ADDCMD((ID_PANSCAN_ROTATEXP,		 VK_NUMPAD8, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEX_P));
	ADDCMD((ID_PANSCAN_ROTATEXM,		 VK_NUMPAD2, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEX_M));
	ADDCMD((ID_PANSCAN_ROTATEYP,		 VK_NUMPAD4, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEY_P));
	ADDCMD((ID_PANSCAN_ROTATEYM,		 VK_NUMPAD6, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEY_M));
	ADDCMD((ID_PANSCAN_ROTATEZP,		 VK_NUMPAD1, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEZ_P));
	ADDCMD((ID_PANSCAN_ROTATEZM,		 VK_NUMPAD3, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEZ_M));
	ADDCMD((ID_VOLUME_UP,					  VK_UP, FVIRTKEY|FNOINVERT,				IDS_AG_VOLUME_UP,   APPCOMMAND_VOLUME_UP, wmcmd::WUP));
	ADDCMD((ID_VOLUME_DOWN,					VK_DOWN, FVIRTKEY|FNOINVERT,				IDS_AG_VOLUME_DOWN, APPCOMMAND_VOLUME_DOWN, wmcmd::WDOWN));
	ADDCMD((ID_VOLUME_MUTE,						'M', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_VOLUME_MUTE, APPCOMMAND_VOLUME_MUTE));
	ADDCMD((ID_VOLUME_BOOST_INC,				  0, FVIRTKEY|FNOINVERT,				IDS_VOLUME_BOOST_INC));
	ADDCMD((ID_VOLUME_BOOST_DEC,				  0, FVIRTKEY|FNOINVERT,				IDS_VOLUME_BOOST_DEC));
	ADDCMD((ID_VOLUME_BOOST_MIN,				  0, FVIRTKEY|FNOINVERT,				IDS_VOLUME_BOOST_MIN));
	ADDCMD((ID_VOLUME_BOOST_MAX,				  0, FVIRTKEY|FNOINVERT,				IDS_VOLUME_BOOST_MAX));
	ADDCMD((ID_NAVIGATE_TITLEMENU,				'T', FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_63));
	ADDCMD((ID_NAVIGATE_ROOTMENU,				'R', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_DVD_ROOT_MENU));
	ADDCMD((ID_NAVIGATE_SUBPICTUREMENU,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_65));
	ADDCMD((ID_NAVIGATE_AUDIOMENU,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_66));
	ADDCMD((ID_NAVIGATE_ANGLEMENU,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_67));
	ADDCMD((ID_NAVIGATE_CHAPTERMENU,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_68));
	ADDCMD((ID_NAVIGATE_MENU_LEFT,			VK_LEFT, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_DVD_MENU_LEFT));
	ADDCMD((ID_NAVIGATE_MENU_RIGHT,		   VK_RIGHT, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_70));
	ADDCMD((ID_NAVIGATE_MENU_UP,			  VK_UP, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_DVD_MENU_UP));
	ADDCMD((ID_NAVIGATE_MENU_DOWN,			VK_DOWN, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_DVD_MENU_DOWN));
	ADDCMD((ID_NAVIGATE_MENU_ACTIVATE,	   VK_SPACE, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_73));
	ADDCMD((ID_NAVIGATE_MENU_BACK,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_DVD_MENU_BACK));
	ADDCMD((ID_NAVIGATE_MENU_LEAVE,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_75));
	ADDCMD((ID_BOSS,							'B', FVIRTKEY|FNOINVERT,				IDS_AG_BOSS_KEY));
	ADDCMD((ID_MENU_PLAYER_SHORT,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_77, 0, wmcmd::RUP));
	ADDCMD((ID_MENU_PLAYER_LONG,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_78));
	ADDCMD((ID_MENU_FILTERS,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_FILTERS_MENU));
	ADDCMD((ID_VIEW_OPTIONS,					'O', FVIRTKEY|FNOINVERT,				IDS_AG_OPTIONS));
	ADDCMD((ID_STREAM_AUDIO_NEXT,				'A', FVIRTKEY|FNOINVERT,				IDS_AG_NEXT_AUDIO));
	ADDCMD((ID_STREAM_AUDIO_PREV,				'A', FVIRTKEY|FSHIFT|FNOINVERT,			IDS_AG_PREV_AUDIO));
	ADDCMD((ID_STREAM_SUB_NEXT,					'S', FVIRTKEY|FNOINVERT,				IDS_AG_NEXT_SUBTITLE));
	ADDCMD((ID_STREAM_SUB_PREV,					'S', FVIRTKEY|FSHIFT|FNOINVERT,			IDS_AG_PREV_SUBTITLE));
	ADDCMD((ID_STREAM_SUB_ONOFF,				'W', FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_85));
	ADDCMD((ID_SUBTITLES_SUBITEM_START+2,		  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_86));
	ADDCMD((ID_OGM_AUDIO_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_87));
	ADDCMD((ID_OGM_AUDIO_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_88));
	ADDCMD((ID_OGM_SUB_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_89));
	ADDCMD((ID_OGM_SUB_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_90));
	ADDCMD((ID_DVD_ANGLE_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_91));
	ADDCMD((ID_DVD_ANGLE_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_92));
	ADDCMD((ID_DVD_AUDIO_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_93));
	ADDCMD((ID_DVD_AUDIO_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_94));
	ADDCMD((ID_DVD_SUB_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_95));
	ADDCMD((ID_DVD_SUB_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_96));
	ADDCMD((ID_DVD_SUB_ONOFF,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_97));
	ADDCMD((ID_VIEW_TEARING_TEST,				'T', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TEARING_TEST));
	ADDCMD((ID_VIEW_REMAINING_TIME,				'I', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_98));
	ADDCMD((ID_SHADER_TOGGLE,					'P', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AT_TOGGLE_SHADER));
	ADDCMD((ID_SHADER_TOGGLESCREENSPACE,		'P', FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AT_TOGGLE_SHADERSCREENSPACE));
	ADDCMD((ID_D3DFULLSCREEN_TOGGLE,			'F', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_99));
	ADDCMD((ID_GOTO_PREV_SUB,					'Y', FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_100, APPCOMMAND_BROWSER_BACKWARD));
	ADDCMD((ID_GOTO_NEXT_SUB,					'U', FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_101,  APPCOMMAND_BROWSER_FORWARD));
	ADDCMD((ID_SHIFT_SUB_DOWN,				VK_NEXT, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_102));
	ADDCMD((ID_SHIFT_SUB_UP,			   VK_PRIOR, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_103));
	ADDCMD((ID_VIEW_DISPLAYSTATS,				'J', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_DISPLAY_STATS));
	ADDCMD((ID_VIEW_RESETSTATS,					'R', FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AG_RESET_STATS));
	ADDCMD((ID_VIEW_VSYNC,						'V', FVIRTKEY|FNOINVERT,				IDS_AG_VSYNC));
	ADDCMD((ID_VIEW_ENABLEFRAMETIMECORRECTION,  'C', FVIRTKEY|FNOINVERT,				IDS_AG_ENABLEFRAMETIMECORRECTION));
	ADDCMD((ID_VIEW_VSYNCACCURATE,				'V', FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AG_VSYNCACCURATE));
	ADDCMD((ID_VIEW_VSYNCOFFSET_DECREASE,	  VK_UP, FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AG_VSYNCOFFSET_DECREASE));
	ADDCMD((ID_VIEW_VSYNCOFFSET_INCREASE,	VK_DOWN, FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AG_VSYNCOFFSET_INCREASE));
	ADDCMD((ID_SUB_DELAY_DOWN,				  VK_F1, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_104));
	ADDCMD((ID_SUB_DELAY_UP,				  VK_F2, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_105));
	ADDCMD((ID_FILE_SAVE_THUMBNAILS,			  0, FVIRTKEY|FNOINVERT,				IDS_FILE_SAVE_THUMBNAILS));

	ADDCMD((ID_VIEW_EDITLISTEDITOR,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_TOGGLE_EDITLISTEDITOR));
	ADDCMD((ID_EDL_IN,							  0, FVIRTKEY|FNOINVERT,				IDS_AG_EDL_IN));
	ADDCMD((ID_EDL_OUT,							  0, FVIRTKEY|FNOINVERT,				IDS_AG_EDL_OUT));
	ADDCMD((ID_EDL_NEWCLIP,						  0, FVIRTKEY|FNOINVERT,				IDS_AG_EDL_NEW_CLIP));
	ADDCMD((ID_EDL_SAVE,						  0, FVIRTKEY|FNOINVERT,				IDS_AG_EDL_SAVE));

	ResetPositions();

#undef ADDCMD
}

CAppSettings::~CAppSettings()
{
	if(hAccel)
		DestroyAcceleratorTable(hAccel);
}

bool CAppSettings::IsD3DFullscreen()
{
	if(nCLSwitches&CLSW_D3DFULLSCREEN)
		return true;
	else
		return fD3DFullscreen;
}
CString CAppSettings::SelectedAudioRenderer()
{
	CString	strResult;
	if(AfxGetMyApp()->m_AudioRendererDisplayName_CL != _T(""))
		strResult = AfxGetMyApp()->m_AudioRendererDisplayName_CL;
	else
		strResult = AfxGetAppSettings().AudioRendererDisplayName;

	return strResult;
}

void CAppSettings::ResetPositions()
{
	nCurrentDvdPosition		= -1;
	nCurrentFilePosition	= -1;
}

DVD_POSITION* CAppSettings::CurrentDVDPosition()
{
	if (nCurrentDvdPosition != -1)
		return &DvdPosition[nCurrentDvdPosition];
	else
		return NULL;
}

bool CAppSettings::NewDvd(ULONGLONG llDVDGuid)
{
	int			i;

	// Look for the DVD position
	for (i=0; i<MAX_DVD_POSITION; i++)
	{
		if (DvdPosition[i].llDVDGuid == llDVDGuid)
		{
			nCurrentDvdPosition = i;
			return false;
		}
	}

	// If DVD is unknown, we put it first
	for (int i=MAX_DVD_POSITION-1; i>0; i--)
		memcpy (&DvdPosition[i], &DvdPosition[i-1], sizeof(DVD_POSITION));
	DvdPosition[0].llDVDGuid	= llDVDGuid;
	nCurrentDvdPosition			= 0;
	return true;
}

FILE_POSITION* CAppSettings::CurrentFilePosition()
{
	if (nCurrentFilePosition != -1)
		return &FilePosition[nCurrentFilePosition];
	else
		return NULL;
}

bool CAppSettings::NewFile(LPCTSTR strFileName)
{
	int			i;

	// Look for the file position
	for (i=0; i<MAX_FILE_POSITION; i++)
	{
		if (FilePosition[i].strFile == strFileName)
		{
			nCurrentFilePosition = i;
			return false;
		}
	}

	// If it is unknown, we put it first
	for (int i=MAX_FILE_POSITION-1; i>0; i--)
	{
		FilePosition[i].strFile		= FilePosition[i-1].strFile;
		FilePosition[i].llPosition	= FilePosition[i-1].llPosition;
	}
	FilePosition[0].strFile		= strFileName;
	FilePosition[0].llPosition	= 0;
	nCurrentFilePosition		= 0;
	return true;
}


void CAppSettings::DeserializeHex (LPCTSTR strVal, BYTE* pBuffer, int nBufSize)
{
	long		lRes;

	for (int i=0; i<nBufSize; i++)
	{
		_stscanf_s (strVal+(i*2), _T("%02x"), &lRes);
		pBuffer[i] = (BYTE)lRes;
	}
}

CString CAppSettings::SerializeHex (BYTE* pBuffer, int nBufSize)
{
	CString		strTemp;
	CString		strResult;

	for (int i=0; i<nBufSize; i++)
	{
		strTemp.Format (_T("%02x"), pBuffer[i]);
		strResult += strTemp;
	}

	return strResult;
}

void CAppSettings::UpdateData(bool fSave)
{
	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp);

	UINT len;
	BYTE* ptr = NULL;

	if(fSave)
	{
		if(!fInitialized) return;

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECAPTIONMENU, fHideCaptionMenu);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDENAVIGATION, fHideNavigation);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CONTROLSTATE, nCS);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULTVIDEOFRAME, iDefaultVideoSize);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPASPECTRATIO, fKeepAspectRatio);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COMPMONDESKARDIFF, fCompMonDeskARDiff);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUME, nVolume);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_BALANCE, nBalance);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MUTE, fMute);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOOPNUM, nLoops);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOOP, fLoopForever);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REWIND, fRewind);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ZOOM, iZoomLevel);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MULTIINST, fAllowMultipleInst);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTSTYLE, iTitleBarTextStyle);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTTITLE, fTitleBarTextTitle);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ONTOP, iOnTop);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TRAYICON, fTrayIcon);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOZOOM, fRememberZoomLevel);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLS, fShowBarsWhenFullScreen);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLSTIMEOUT, nShowBarsWhenFullScreenTimeOut);
		pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_FULLSCREENRES, (BYTE*)&AutoChangeFullscrRes, sizeof(AutoChangeFullscrRes));
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_EXITFULLSCREENATTHEEND, fExitFullScreenAtTheEnd);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("RestoreResAfterExit"), fRestoreResAfterExit);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWPOS, fRememberWindowPos);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWSIZE, fRememberWindowSize);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SNAPTODESKTOPEDGES, fSnapToDesktopEdges);
		pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_LASTWINDOWRECT, (BYTE*)&rcLastWindowPos, sizeof(rcLastWindowPos));
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTWINDOWTYPE, lastWindowType);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_X, AspectRatio.cx);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_Y, AspectRatio.cy);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPHISTORY, fKeepHistory);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE, iDSVideoRendererType);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_RMVIDEORENDERERTYPE, iRMVideoRendererType);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_QTVIDEORENDERERTYPE, iQTVideoRendererType);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_APSURACEFUSAGE, m_RenderersSettings.iAPSurfaceUsage);
//		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VMRSYNCFIX, m_RenderersSettings.fVMRSyncFix);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DX9_RESIZER, m_RenderersSettings.iDX9Resizer);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERMODE, m_RenderersSettings.fVMR9MixerMode);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERYUV, m_RenderersSettings.fVMR9MixerYUV);

		CRenderersSettings::CRendererSettingsEVR& rendererSettings = m_RenderersSettings.m_RenderSettings;
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRAlternateVSync"), rendererSettings.fVMR9AlterativeVSync);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSyncOffset"), rendererSettings.iVMR9VSyncOffset);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSyncAccurate2"), rendererSettings.iVMR9VSyncAccurate);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFullscreenGUISupport"), rendererSettings.iVMR9FullscreenGUISupport);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSync"), rendererSettings.iVMR9VSync);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRDisableDesktopComposition"), rendererSettings.iVMRDisableDesktopComposition);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFullFloatingPointProcessing"), rendererSettings.iVMR9FullFloatingPointProcessing);

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementEnable"), rendererSettings.iVMR9ColorManagementEnable);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementInput"), rendererSettings.iVMR9ColorManagementInput);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementAmbientLight"), rendererSettings.iVMR9ColorManagementAmbientLight);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementIntent"), rendererSettings.iVMR9ColorManagementIntent);

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVROutputRange"), rendererSettings.iEVROutputRange);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVRHighColorRes"), rendererSettings.iEVRHighColorResolution);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVRForceInputHighColorRes"), rendererSettings.iEVRForceInputHighColorResolution);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVREnableFrameTimeCorrection"), rendererSettings.iEVREnableFrameTimeCorrection);

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUBeforeVSync"), rendererSettings.iVMRFlushGPUBeforeVSync);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUAfterPresent"), rendererSettings.iVMRFlushGPUAfterPresent);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUWait"), rendererSettings.iVMRFlushGPUWait);

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeClock"), rendererSettings.bSynchronizeVideo);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeDisplay"), rendererSettings.bSynchronizeDisplay);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeNearest"), rendererSettings.bSynchronizeNearest);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("LineDelta"), rendererSettings.iLineDelta);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ColumnDelta"), rendererSettings.iColumnDelta);

		pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("CycleDelta"), (LPBYTE)&(rendererSettings.fCycleDelta), sizeof(rendererSettings.fCycleDelta));
		pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("TargetSyncOffset"), (LPBYTE)&(rendererSettings.fTargetSyncOffset), sizeof(rendererSettings.fTargetSyncOffset));
		pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("ControlLimit"), (LPBYTE)&(rendererSettings.fControlLimit), sizeof(rendererSettings.fControlLimit));

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ResetDevice"), m_RenderersSettings.fResetDevice);

		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_AUDIORENDERERTYPE, CString(AudioRendererDisplayName));
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADAUDIO, fAutoloadAudio);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADSUBTITLES, fAutoloadSubtitles);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANGORDER, CString(m_subtitlesLanguageOrder));
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOSLANGORDER, CString(m_audiosLanguageOrder));
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_BLOCKVSFILTER, fBlockVSFilter);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWORKERTHREADFOROPENING, fEnableWorkerThreadForOpening);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REPORTFAILEDPINS, fReportFailedPins);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_DVDPATH, sDVDPath);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USEDVDPATH, fUseDVDPath);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MENULANG, idMenuLang);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOLANG, idAudioLang);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANG, idSubtitlesLang);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOSPEAKERCONF, fAutoSpeakerConf);
		CString style;
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SPLOGFONT, style <<= subdefstyle);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPOVERRIDEPLACEMENT, fOverridePlacement);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPHORPOS, nHorPos);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPVERPOS, nVerPos);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPCSIZE, m_RenderersSettings.nSPCSize);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPCMAXRES, m_RenderersSettings.nSPCMaxRes);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBDELAYINTERVAL, nSubDelayInterval);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_POW2TEX, m_RenderersSettings.fSPCPow2Tex);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SPCAllowAnimationWhenBuffering"), m_RenderersSettings.fSPCAllowAnimationWhenBuffering);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLESUBTITLES, fEnableSubtitles);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITIZEEXTERNALSUBTITLES, fPrioritizeExternalSubtitles);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEINTERNALSUBTITLES, fDisableInternalSubtitles);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLEPATHS, szSubtitlePaths);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USEDEFAULTSUBTITLESSTYLE, fUseDefaultSubtitlesStyle);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOSWITCHER, fEnableAudioSwitcher);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOTIMESHIFT, fAudioTimeShift);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOTIMESHIFT, tAudioTimeShift);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DOWNSAMPLETO441, fDownSampleTo441);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CUSTOMCHANNELMAPPING, fCustomChannelMapping);
		pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_SPEAKERTOCHANNELMAPPING, (BYTE*)pSpeakerToChannelMap, sizeof(pSpeakerToChannelMap));
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZE, fAudioNormalize);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZERECOVER, fAudioNormalizeRecover);

		CString strTemp;
		strTemp.Format( _T("%f"), AudioBoost);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, strTemp);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPEAKERCHANNELS, fnChannels);

		// Multi-monitor code
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_FULLSCREENMONITOR, CString(f_hmonitor));
		// Prevent Minimize when in Fullscreen mode on non default monitor
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_PREVENT_MINIMIZE, m_fPreventMinimize);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_WIN7TASKBAR, m_fUseWin7TaskBar);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_EXIT_AFTER_PB, m_fExitAfterPlayback);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NEXT_AFTER_PB, m_fNextInDirAfterPlayback);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NO_SEARCH_IN_FOLDER, m_fDontUseSearchInFolder);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_OSD_SIZE, nOSD_Size);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_MPC_OSD_FONT, m_OSD_Font);

		// Associated types with icon or not...
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASSOCIATED_WITH_ICON, fAssociatedWithIcons);
		// Last Open Dir
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_LAST_OPEN_DIR, f_lastOpenDir);


		// CASIMIR666 : new settings
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_D3DFULLSCREEN, fD3DFullscreen);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MONITOR_AUTOREFRESHRATE, fMonitorAutoRefreshRate);

		strTemp.Format (_T("%f"), dBrightness);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_BRIGHTNESS, strTemp);
		strTemp.Format (_T("%f"), dContrast);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_CONTRAST, strTemp);
		strTemp.Format (_T("%f"), dHue);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_HUE, strTemp);
		strTemp.Format (_T("%f"), dSaturation);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_SATURATION, strTemp);

		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLIST, strShaderList);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLISTSCREENSPACE, strShaderListScreenSpace);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADER, (int)m_bToggleShader);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADERSSCREENSPACE, (int)m_bToggleShaderScreenSpace);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_EVR_BUFFERS, m_RenderersSettings.iEvrBuffers);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SHOWOSD, (int)fShowOSD);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEEDLEDITOR, (int)fEnableEDLEditor);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LANGUAGE, (int)iLanguage);

		// Save analog capture settings
		pApp->WriteProfileInt   (IDS_R_SETTINGS, IDS_RS_DEFAULT_CAPTURE, iDefaultCaptureDevice);
		pApp->WriteProfileString(IDS_RS_CAPTURE, IDS_RS_VIDEO_DISP_NAME, strAnalogVideo);
		pApp->WriteProfileString(IDS_RS_CAPTURE, IDS_RS_AUDIO_DISP_NAME, strAnalogAudio);
		pApp->WriteProfileInt   (IDS_RS_CAPTURE, IDS_RS_COUNTRY,		 iAnalogCountry);

		// Save digital capture settings (BDA)
		pApp->WriteProfileString(IDS_RS_DVB, IDS_RS_BDA_NETWORKPROVIDER, BDANetworkProvider);
		pApp->WriteProfileString(IDS_RS_DVB, IDS_RS_BDA_TUNER, BDATuner);
		pApp->WriteProfileString(IDS_RS_DVB, IDS_RS_BDA_RECEIVER, BDAReceiver);
		pApp->WriteProfileInt(IDS_RS_DVB, IDS_RS_DVB_LAST_CHANNEL, DVBLastChannel);

		int			iChannel = 0;
		POSITION	pos = DVBChannels.GetHeadPosition();
		while (pos)
		{
			CString			strTemp;
			CString			strChannel;
			CDVBChannel&	Channel = DVBChannels.GetNext(pos);
			strTemp.Format(_T("%d"), iChannel);
			pApp->WriteProfileString(IDS_RS_DVB, strTemp, Channel.ToString());
			iChannel++;
		}


		// playback positions for last played DVDs
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DVDPOS, (int)fRememberDVDPos);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOS, (int)fRememberFilePos);
		if (fKeepHistory)
		{
			for (int i=0; i<MAX_DVD_POSITION; i++)
			{
				CString		strDVDPos;
				CString		strValue;

				strDVDPos.Format (_T("DVD Position %d"), i);
				strValue = SerializeHex((BYTE*)&DvdPosition[i], sizeof(DVD_POSITION));
				pApp->WriteProfileString(IDS_R_SETTINGS, strDVDPos, strValue);
			}

			// playback positions for last played files
			for (int i=0; i<MAX_FILE_POSITION; i++)
			{
				CString		strFilePos;
				CString		strValue;

				strFilePos.Format (_T("File Name %d"), i);
				pApp->WriteProfileString(IDS_R_SETTINGS, strFilePos, FilePosition[i].strFile);
				strFilePos.Format (_T("File Position %d"), i);
				strValue.Format (_T("%I64d"), FilePosition[i].llPosition);
				pApp->WriteProfileString(IDS_R_SETTINGS, strFilePos, strValue);
			}
		}

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTFULLSCREEN, (int)fLastFullScreen);
		// CASIMIR666 : end of new settings

		{
			for(int i = 0; ; i++)
			{
				CString key;
				key.Format(_T("%s\\%04d"), IDS_R_FILTERS, i);
				int j = pApp->GetProfileInt(key, _T("Enabled"), -1);
				pApp->WriteProfileString(key, NULL, NULL);
				if(j < 0) break;
			}
			pApp->WriteProfileString(IDS_R_FILTERS, NULL, NULL);

			POSITION pos = filters.GetHeadPosition();
			for(int i = 0; pos; i++)
			{
				FilterOverride* f = filters.GetNext(pos);

				if(f->fTemporary)
					continue;

				CString key;
				key.Format(_T("%s\\%04d"), IDS_R_FILTERS, i);

				pApp->WriteProfileInt(key, _T("SourceType"), (int)f->type);
				pApp->WriteProfileInt(key, _T("Enabled"), (int)!f->fDisabled);
				if(f->type == FilterOverride::REGISTERED)
				{
					pApp->WriteProfileString(key, _T("DisplayName"), CString(f->dispname));
					pApp->WriteProfileString(key, _T("Name"), f->name);
				}
				else if(f->type == FilterOverride::EXTERNAL)
				{
					pApp->WriteProfileString(key, _T("Path"), f->path);
					pApp->WriteProfileString(key, _T("Name"), f->name);
					pApp->WriteProfileString(key, _T("CLSID"), CStringFromGUID(f->clsid));
				}
				POSITION pos2 = f->backup.GetHeadPosition();
				for(int i = 0; pos2; i++)
				{
					CString val;
					val.Format(_T("org%04d"), i);
					pApp->WriteProfileString(key, val, CStringFromGUID(f->backup.GetNext(pos2)));
				}
				pos2 = f->guids.GetHeadPosition();
				for(int i = 0; pos2; i++)
				{
					CString val;
					val.Format(_T("mod%04d"), i);
					pApp->WriteProfileString(key, val, CStringFromGUID(f->guids.GetNext(pos2)));
				}
				pApp->WriteProfileInt(key, _T("LoadType"), f->iLoadType);
				pApp->WriteProfileInt(key, _T("Merit"), f->dwMerit);
			}
		}

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_INTREALMEDIA, fIntRealMedia);
		// pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIARENDERLESS, fRealMediaRenderless);
		// pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_QUICKTIMERENDERER, iQuickTimeRenderer);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIAFPS, *((DWORD*)&RealMediaQuickTimeFPS));

		pApp->WriteProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, NULL, NULL);
		for(int i = 0, j = m_pnspresets.GetCount(); i < j; i++)
		{
			CString str;
			str.Format(_T("Preset%d"), i);
			pApp->WriteProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, str, m_pnspresets[i]);
		}

		pApp->WriteProfileString(IDS_R_COMMANDS, NULL, NULL);
		pos = wmcmds.GetHeadPosition();
		for(int i = 0; pos; )
		{
			wmcmd& wc = wmcmds.GetNext(pos);
			if(wc.IsModified())
			{
				CString str;
				str.Format(_T("CommandMod%d"), i);
				CString str2;
				str2.Format(_T("%d %x %x %s %d %d %d"),
							wc.cmd, wc.fVirt, wc.key,
							_T("\"") + CString(wc.rmcmd) +  _T("\""), wc.rmrepcnt,
							wc.mouse, wc.appcmd);
				pApp->WriteProfileString(IDS_R_COMMANDS, str, str2);
				i++;
			}
		}

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WINLIRC, fWinLirc);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WINLIRCADDR, WinLircAddr);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_UICE, fUIce);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_UICEADDR, UIceAddr);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_GLOBALMEDIA, fGlobalMedia);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEXPTOOLBARS, fDisableXPToolbars);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USEWMASFREADER, fUseWMASFReader);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTS, nJumpDistS);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTM, nJumpDistM);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTL, nJumpDistL);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LIMITWINDOWPROPORTIONS, fLimitWindowProportions);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_NOTIFYMSN, fNotifyMSN);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_NOTIFYGTSDLL, fNotifyGTSdll);

		Formats.UpdateData(true);

		pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_SRCFILTERS, SrcFilters|~(SRC_LAST-1));
		pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_TRAFILTERS, TraFilters|~(TRA_LAST-1));
		pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_DXVAFILTERS, DXVAFilters|~(DXVA_LAST-1));
		pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_FFMPEGFILTERS, FFmpegFilters|~(FFM_LAST-1));

		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_LOGOFILE, logofn);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOID, logoid);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOEXT, logoext);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECDROMSSUBMENU, fHideCDROMsSubMenu);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITY, priority);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LAUNCHFULLSCREEN, launchfullscreen);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWEBSERVER, fEnableWebServer);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPORT, nWebServerPort);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPRINTDEBUGINFO, fWebServerPrintDebugInfo);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERUSECOMPRESSION, fWebServerUseCompression);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERLOCALHOSTONLY, fWebServerLocalhostOnly);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBROOT, WebRoot);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBDEFINDEX, WebDefIndex);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBSERVERCGI, WebServerCGI);

		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTPATH, SnapShotPath);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTEXT, SnapShotExt);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBROWS, ThumbRows);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBCOLS, ThumbCols);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBWIDTH, ThumbWidth);

		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_ISDB, ISDb);

		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_D3D9RENDERDEVICE, D3D9RenderDevice);

		pApp->WriteProfileString(_T("Shaders"), NULL, NULL);
		pApp->WriteProfileInt(_T("Shaders"), _T("Initialized"), 1);
		pApp->WriteProfileString(_T("Shaders"), _T("Combine"), m_shadercombine);
		pApp->WriteProfileString(_T("Shaders"), _T("CombineScreenSpace"), m_shadercombineScreenSpace);


		pos = m_shaders.GetHeadPosition();
		for(int i = 0; pos; i++)
		{
			const Shader& s = m_shaders.GetNext(pos);

			if(!s.label.IsEmpty())
			{
				CString index;
				index.Format(_T("%d"), i);
				CString srcdata = s.srcdata;
				srcdata.Replace(_T("\r"), _T(""));
				srcdata.Replace(_T("\n"), _T("\\n"));
				srcdata.Replace(_T("\t"), _T("\\t"));
				AfxGetApp()->WriteProfileString(_T("Shaders"), index, s.label + _T("|") + s.target + _T("|") + srcdata);
			}
		}

		if(pApp->m_pszRegistryKey)
		{
			// WINBUG: on win2k this would crash WritePrivateProfileString
			pApp->WriteProfileInt(_T(""), _T(""), pApp->GetProfileInt(_T(""), _T(""), 0)?0:1);
		}
	}
	else
	{
		if(fInitialized) return;

		iDXVer = 0;
		CRegKey dxver;
		if(ERROR_SUCCESS == dxver.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\DirectX"), KEY_READ))
		{
			CString str;
			ULONG len = 64;
			if(ERROR_SUCCESS == dxver.QueryStringValue(_T("Version"), str.GetBuffer(len), &len))
			{
				str.ReleaseBuffer(len);
				int ver[4];
				_stscanf_s(str, _T("%d.%d.%d.%d"), ver+0, ver+1, ver+2, ver+3);
				iDXVer = ver[1];
			}
		}

		// Set interface language first!
		iLanguage  = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LANGUAGE, CMPlayerCApp::GetDefLanguage());
		if (iLanguage != 0) CMPlayerCApp::SetLanguage(iLanguage);
		CreateCommands();

		fHideCaptionMenu = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECAPTIONMENU, 0);
		fHideNavigation = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDENAVIGATION, 0);
		nCS = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CONTROLSTATE, CS_SEEKBAR|CS_TOOLBAR|CS_STATUSBAR);
		iDefaultVideoSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULTVIDEOFRAME, DVS_FROMINSIDE);
		fKeepAspectRatio = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPASPECTRATIO, TRUE);
		fCompMonDeskARDiff = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COMPMONDESKARDIFF, FALSE);
		nVolume = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUME, 100);
		nBalance = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_BALANCE, 0);
		fMute = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MUTE, 0);
		nLoops = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOOPNUM, 1);
		fLoopForever = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOOP, 0);
		fRewind = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REWIND, FALSE);
		iZoomLevel = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ZOOM, 1);
		iDSVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE, (IsVistaOrAbove() ? (CMPlayerCApp::HasEVR() ? VIDRNDT_DS_EVR_CUSTOM : VIDRNDT_DS_DEFAULT) : VIDRNDT_DS_OVERLAYMIXER) );
		iRMVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_RMVIDEORENDERERTYPE, VIDRNDT_RM_DEFAULT);
		iQTVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_QTVIDEORENDERERTYPE, VIDRNDT_QT_DEFAULT);

		m_RenderersSettings.iAPSurfaceUsage = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_APSURACEFUSAGE, (IsVistaOrAbove() ? VIDRNDT_AP_TEXTURE3D : VIDRNDT_AP_TEXTURE2D));
//		m_RenderersSettings.fVMRSyncFix = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VMRSYNCFIX, FALSE);
		m_RenderersSettings.iDX9Resizer = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DX9_RESIZER, 1);
		m_RenderersSettings.fVMR9MixerMode = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERMODE, TRUE);
		m_RenderersSettings.fVMR9MixerYUV = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERYUV, FALSE);

		CRenderersSettings::CRendererSettingsEVR& rendererSettings = m_RenderersSettings.m_RenderSettings;
		CRenderersSettings::CRendererSettingsEVR DefaultSettings;
		rendererSettings.fVMR9AlterativeVSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRAlternateVSync"), DefaultSettings.fVMR9AlterativeVSync);
		rendererSettings.iVMR9VSyncOffset = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSyncOffset"), DefaultSettings.iVMR9VSyncOffset);
		rendererSettings.iVMR9VSyncAccurate = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSyncAccurate2"), DefaultSettings.iVMR9VSyncAccurate);
		rendererSettings.iVMR9FullscreenGUISupport = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFullscreenGUISupport"), DefaultSettings.iVMR9FullscreenGUISupport);
		rendererSettings.iEVRHighColorResolution = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVRHighColorRes"), DefaultSettings.iEVRHighColorResolution);
		rendererSettings.iEVRForceInputHighColorResolution = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVRForceInputHighColorRes"), DefaultSettings.iEVRForceInputHighColorResolution);
		rendererSettings.iEVREnableFrameTimeCorrection = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVREnableFrameTimeCorrection"), DefaultSettings.iEVREnableFrameTimeCorrection);
		rendererSettings.iVMR9VSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSync"), DefaultSettings.iVMR9VSync);
		rendererSettings.iVMRDisableDesktopComposition = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRDisableDesktopComposition"), DefaultSettings.iVMRDisableDesktopComposition);
		rendererSettings.iVMR9FullFloatingPointProcessing = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFullFloatingPointProcessing"), DefaultSettings.iVMR9FullFloatingPointProcessing);

		rendererSettings.iVMR9ColorManagementEnable = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementEnable"), DefaultSettings.iVMR9ColorManagementEnable);
		rendererSettings.iVMR9ColorManagementInput = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementInput"), DefaultSettings.iVMR9ColorManagementInput);
		rendererSettings.iVMR9ColorManagementAmbientLight = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementAmbientLight"), DefaultSettings.iVMR9ColorManagementAmbientLight);
		rendererSettings.iVMR9ColorManagementIntent = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementIntent"), DefaultSettings.iVMR9ColorManagementIntent);

		rendererSettings.iEVROutputRange = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVROutputRange"), DefaultSettings.iEVROutputRange);

		rendererSettings.iVMRFlushGPUBeforeVSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUBeforeVSync"), DefaultSettings.iVMRFlushGPUBeforeVSync);
		rendererSettings.iVMRFlushGPUAfterPresent = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUAfterPresent"), DefaultSettings.iVMRFlushGPUAfterPresent);
		rendererSettings.iVMRFlushGPUWait = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUWait"), DefaultSettings.iVMRFlushGPUWait);

		rendererSettings.bSynchronizeVideo = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SynchronizeClock"), DefaultSettings.bSynchronizeVideo);
		rendererSettings.bSynchronizeDisplay = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SynchronizeDisplay"), DefaultSettings.bSynchronizeDisplay);
		rendererSettings.bSynchronizeNearest = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SynchronizeNearest"), DefaultSettings.bSynchronizeNearest);
		rendererSettings.iLineDelta = pApp->GetProfileInt(IDS_R_SETTINGS, _T("LineDelta"), DefaultSettings.iLineDelta);
		rendererSettings.iColumnDelta = pApp->GetProfileInt(IDS_R_SETTINGS, _T("ColumnDelta"), DefaultSettings.iColumnDelta);

		double *dPtr;
		UINT dSize;
		if(pApp->GetProfileBinary(IDS_R_SETTINGS, _T("CycleDelta"), (LPBYTE*)&dPtr, &dSize))
		{
			rendererSettings.fCycleDelta = *dPtr;
			delete [] dPtr;
		}

		if(pApp->GetProfileBinary(IDS_R_SETTINGS, _T("TargetSyncOffset"), (LPBYTE*)&dPtr, &dSize))
		{
			rendererSettings.fTargetSyncOffset = *dPtr;
			delete [] dPtr;
		}
		if(pApp->GetProfileBinary(IDS_R_SETTINGS, _T("ControlLimit"), (LPBYTE*)&dPtr, &dSize))
		{
			rendererSettings.fControlLimit = *dPtr;
			delete [] dPtr;
		}

		m_RenderersSettings.fResetDevice = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("ResetDevice"), TRUE);

		AudioRendererDisplayName = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIORENDERERTYPE, _T(""));
		fAutoloadAudio = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADAUDIO, TRUE);
		fAutoloadSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADSUBTITLES, !CMPlayerCApp::IsVSFilterInstalled() || (IsVistaOrAbove() && CMPlayerCApp::HasEVR()) );
		m_subtitlesLanguageOrder = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANGORDER, _T(""));
		m_audiosLanguageOrder = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOSLANGORDER, _T(""));
		fBlockVSFilter = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_BLOCKVSFILTER, TRUE);
		fEnableWorkerThreadForOpening = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWORKERTHREADFOROPENING, TRUE);
		fReportFailedPins = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REPORTFAILEDPINS, TRUE);
		fAllowMultipleInst = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MULTIINST, 0);
		iTitleBarTextStyle = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTSTYLE, 1);
		fTitleBarTextTitle = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTTITLE, FALSE);
		iOnTop = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ONTOP, 0);
		fTrayIcon = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TRAYICON, 0);
		fRememberZoomLevel = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOZOOM, 1);
		fShowBarsWhenFullScreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLS, 1);
		nShowBarsWhenFullScreenTimeOut = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLSTIMEOUT, 0);

		//Multi-monitor code
		f_hmonitor = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_FULLSCREENMONITOR, _T(""));
		// Prevent Minimize when in Fullscreen mode on non default monitor
		m_fPreventMinimize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_PREVENT_MINIMIZE, 0);
		m_fUseWin7TaskBar = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_WIN7TASKBAR, 1);
		m_fExitAfterPlayback = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_EXIT_AFTER_PB, 0);
		m_fNextInDirAfterPlayback = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NEXT_AFTER_PB, 0);
		m_fDontUseSearchInFolder   = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NO_SEARCH_IN_FOLDER, 0);
		nOSD_Size = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_OSD_SIZE, 20);
		m_OSD_Font= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_MPC_OSD_FONT, _T("Arial"));

		// Associated types with icon or not...
		fAssociatedWithIcons = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASSOCIATED_WITH_ICON, 1);
		// Last Open Dir
		f_lastOpenDir = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LAST_OPEN_DIR, _T("C:\\"));

		if ( pApp->GetProfileBinary(IDS_R_SETTINGS, IDS_RS_FULLSCREENRES, &ptr, &len) )
		{
			if ( len == sizeof(AChFR) )
				memcpy( &AutoChangeFullscrRes, ptr, sizeof(AChFR) );
			else
				AutoChangeFullscrRes.bEnabled = false;
			delete [] ptr;
		}
		else
		{
			AutoChangeFullscrRes.bEnabled = false;
		}

		fExitFullScreenAtTheEnd = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_EXITFULLSCREENATTHEEND, 1);
		fRestoreResAfterExit = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("RestoreResAfterExit"), 1);
		fRememberWindowPos = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWPOS, 0);
		fRememberWindowSize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWSIZE, 0);
		fSnapToDesktopEdges = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SNAPTODESKTOPEDGES, 0);
		AspectRatio.cx = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_X, 0);
		AspectRatio.cy = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_Y, 0);
		fKeepHistory = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPHISTORY, 1);
		if ( pApp->GetProfileBinary(IDS_R_SETTINGS, IDS_RS_LASTWINDOWRECT, &ptr, &len) )
		{
			if ( len == sizeof(CRect) )
				memcpy( &rcLastWindowPos, ptr, sizeof(CRect) );
			else
				fRememberWindowPos = false;
			delete [] ptr;
		}
		else
		{
			fRememberWindowPos = false;
		}
		lastWindowType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTWINDOWTYPE, SIZE_RESTORED);
		sDVDPath = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_DVDPATH, _T(""));
		fUseDVDPath = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USEDVDPATH, 0);
		idMenuLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MENULANG, 0);
		idAudioLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOLANG, 0);
		idSubtitlesLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANG, 0);
		fAutoSpeakerConf = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOSPEAKERCONF, 1);
		// TODO: rename subdefstyle -> defStyle, IDS_RS_SPLOGFONT -> IDS_RS_SPSTYLE
		{
			CString temp = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SPLOGFONT, _T(""));
			subdefstyle <<= temp;
			if(temp == _T(""))
				subdefstyle.relativeTo = 1; //default "Position subtitles relative to the video frame" option is checked
		}
		fOverridePlacement = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPOVERRIDEPLACEMENT, 0);
		nHorPos = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPHORPOS, 50);
		nVerPos = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPVERPOS, 90);
		m_RenderersSettings.nSPCSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPCSIZE, 3);
		m_RenderersSettings.nSPCMaxRes = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPCMAXRES, 2);
		nSubDelayInterval = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBDELAYINTERVAL, 500);
		m_RenderersSettings.fSPCPow2Tex = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_POW2TEX, TRUE);

		bool bAllowAnimationWhenBuffering = true;
		SYSTEM_INFO SysInfo;
		GetSystemInfo(&SysInfo);
		if (SysInfo.dwNumberOfProcessors < 3)
			bAllowAnimationWhenBuffering = false;


		m_RenderersSettings.fSPCAllowAnimationWhenBuffering = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SPCAllowAnimationWhenBuffering"), bAllowAnimationWhenBuffering);
		fEnableSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLESUBTITLES, TRUE);
		fPrioritizeExternalSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITIZEEXTERNALSUBTITLES, FALSE);
		fDisableInternalSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEINTERNALSUBTITLES, FALSE);
		szSubtitlePaths = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLEPATHS, _T(".;.\\subtitles;.\\subs"));
		fUseDefaultSubtitlesStyle = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USEDEFAULTSUBTITLESSTYLE, FALSE);
		fEnableAudioSwitcher = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOSWITCHER, TRUE);
		fAudioTimeShift = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOTIMESHIFT, 0);
		tAudioTimeShift = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOTIMESHIFT, 0);
		fDownSampleTo441 = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DOWNSAMPLETO441, 0);
		fCustomChannelMapping = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CUSTOMCHANNELMAPPING, 0);

		BOOL bResult = pApp->GetProfileBinary( IDS_R_SETTINGS, IDS_RS_SPEAKERTOCHANNELMAPPING, &ptr, &len );
		if ( bResult && len == sizeof(pSpeakerToChannelMap) )
		{
			memcpy( pSpeakerToChannelMap, ptr, sizeof(pSpeakerToChannelMap) );
		}
		else
		{
			memset(pSpeakerToChannelMap, 0, sizeof(pSpeakerToChannelMap));
			for(int j = 0; j < 18; j++)
				for(int i = 0; i <= j; i++)
					pSpeakerToChannelMap[j][i] = 1<<i;

			pSpeakerToChannelMap[0][0] = 1<<0;
			pSpeakerToChannelMap[0][1] = 1<<0;

			pSpeakerToChannelMap[3][0] = 1<<0;
			pSpeakerToChannelMap[3][1] = 1<<1;
			pSpeakerToChannelMap[3][2] = 0;
			pSpeakerToChannelMap[3][3] = 0;
			pSpeakerToChannelMap[3][4] = 1<<2;
			pSpeakerToChannelMap[3][5] = 1<<3;

			pSpeakerToChannelMap[4][0] = 1<<0;
			pSpeakerToChannelMap[4][1] = 1<<1;
			pSpeakerToChannelMap[4][2] = 1<<2;
			pSpeakerToChannelMap[4][3] = 0;
			pSpeakerToChannelMap[4][4] = 1<<3;
			pSpeakerToChannelMap[4][5] = 1<<4;
		}
		if ( bResult )
			delete [] ptr;

		fAudioNormalize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZE, FALSE);
		fAudioNormalizeRecover = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZERECOVER, TRUE);
		AudioBoost = (float)_tstof(pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, _T("1")));

		fnChannels = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPEAKERCHANNELS, 2);

		{
			for(int i = 0; ; i++)
			{
				CString key;
				key.Format(_T("%s\\%04d"), IDS_R_FILTERS, i);

				CAutoPtr<FilterOverride> f(DNew FilterOverride);

				f->fDisabled = !pApp->GetProfileInt(key, _T("Enabled"), 0);

				UINT j = pApp->GetProfileInt(key, _T("SourceType"), -1);
				if(j == 0)
				{
					f->type = FilterOverride::REGISTERED;
					f->dispname = CStringW(pApp->GetProfileString(key, _T("DisplayName"), _T("")));
					f->name = pApp->GetProfileString(key, _T("Name"), _T(""));
				}
				else if(j == 1)
				{
					f->type = FilterOverride::EXTERNAL;
					f->path = pApp->GetProfileString(key, _T("Path"), _T(""));
					f->name = pApp->GetProfileString(key, _T("Name"), _T(""));
					f->clsid = GUIDFromCString(pApp->GetProfileString(key, _T("CLSID"), _T("")));
				}
				else
				{
					pApp->WriteProfileString(key, NULL, 0);
					break;
				}

				f->backup.RemoveAll();
				for(int i = 0; ; i++)
				{
					CString val;
					val.Format(_T("org%04d"), i);
					CString guid = pApp->GetProfileString(key, val, _T(""));
					if(guid.IsEmpty()) break;
					f->backup.AddTail(GUIDFromCString(guid));
				}

				f->guids.RemoveAll();
				for(int i = 0; ; i++)
				{
					CString val;
					val.Format(_T("mod%04d"), i);
					CString guid = pApp->GetProfileString(key, val, _T(""));
					if(guid.IsEmpty()) break;
					f->guids.AddTail(GUIDFromCString(guid));
				}

				f->iLoadType = (int)pApp->GetProfileInt(key, _T("LoadType"), -1);
				if(f->iLoadType < 0) break;

				f->dwMerit = pApp->GetProfileInt(key, _T("Merit"), MERIT_DO_NOT_USE+1);

				filters.AddTail(f);
			}
		}

		fIntRealMedia = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_INTREALMEDIA, 0);
		//fRealMediaRenderless = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIARENDERLESS, 0);
		//iQuickTimeRenderer = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_QUICKTIMERENDERER, 2);
		RealMediaQuickTimeFPS = 25.0;
		*((DWORD*)&RealMediaQuickTimeFPS) = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIAFPS, *((DWORD*)&RealMediaQuickTimeFPS));

		m_pnspresets.RemoveAll();
		for(int i = 0; i < (ID_PANNSCAN_PRESETS_END - ID_PANNSCAN_PRESETS_START); i++)
		{
			CString str;
			str.Format(_T("Preset%d"), i);
			str = pApp->GetProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, str, _T(""));
			if(str.IsEmpty()) break;
			m_pnspresets.Add(str);
		}
		if(m_pnspresets.IsEmpty())
		{
			double _4p3 = 4.0/3.0;
			double _16p9 = 16.0/9.0;
			double _185p1 = 1.85/1.0;
			double _235p1 = 2.35/1.0;
			UNUSED_ALWAYS(_185p1);

			CString str;
			str.Format(ResStr(IDS_SCALE_16_9), 0.5, 0.5, _4p3/_4p3, _16p9/_4p3);
			m_pnspresets.Add(str);
			str.Format(ResStr(IDS_SCALE_WIDESCREEN), 0.5, 0.5, _16p9/_4p3, _16p9/_4p3);
			m_pnspresets.Add(str);
			str.Format(ResStr(IDS_SCALE_ULTRAWIDE), 0.5, 0.5, _235p1/_4p3, _235p1/_4p3);
			m_pnspresets.Add(str);
		}

		for(int i = 0; i < wmcmds.GetCount(); i++)
		{
			CString str;
			str.Format(_T("CommandMod%d"), i);
			str = pApp->GetProfileString(IDS_R_COMMANDS, str, _T(""));
			if(str.IsEmpty()) break;
			int cmd, fVirt, key, repcnt, mouse, appcmd;
			TCHAR buff[128];
			int n;
			if(5 > (n = _stscanf_s(str, _T("%d %x %x %s %d %d %d"), &cmd, &fVirt, &key, buff, countof(buff), &repcnt, &mouse, &appcmd)))
				break;
			if(POSITION pos = wmcmds.Find(cmd))
			{
				wmcmd& wc = wmcmds.GetAt(pos);
				wc.cmd = cmd;
				wc.fVirt = fVirt;
				wc.key = key;
				if(n >= 6) wc.mouse = (UINT)mouse;
				if(n >= 7) wc.appcmd = (UINT)appcmd;
				wc.rmcmd = CStringA(buff).Trim('\"');
				wc.rmrepcnt = repcnt;
			}
		}

		CAtlArray<ACCEL> pAccel;
		pAccel.SetCount(wmcmds.GetCount());
		POSITION pos = wmcmds.GetHeadPosition();
		for(int i = 0; pos; i++) pAccel[i] = wmcmds.GetNext(pos);
		hAccel = CreateAcceleratorTable(pAccel.GetData(), pAccel.GetCount());

		WinLircAddr = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WINLIRCADDR, _T("127.0.0.1:8765"));
		fWinLirc = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WINLIRC, 0);
		UIceAddr = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_UICEADDR, _T("127.0.0.1:1234"));
		fUIce = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_UICE, 0);
		fGlobalMedia = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_GLOBALMEDIA, 0);

		fDisableXPToolbars = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEXPTOOLBARS, 0);
		fUseWMASFReader = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USEWMASFREADER, FALSE);
		nJumpDistS = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTS, 1000);
		nJumpDistM = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTM, 5000);
		nJumpDistL = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTL, 20000);
		fLimitWindowProportions = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LIMITWINDOWPROPORTIONS, FALSE);
		fNotifyMSN = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_NOTIFYMSN, FALSE);
		fNotifyGTSdll = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_NOTIFYGTSDLL, FALSE);

		Formats.UpdateData(false);

		SrcFilters = pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_SRCFILTERS, ~0);
		TraFilters = pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_TRAFILTERS, ~0);
		DXVAFilters = pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_DXVAFILTERS, ~0);
		FFmpegFilters = pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_FFMPEGFILTERS, ~0);

		logofn = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LOGOFILE, _T(""));
		logoid = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOID, DEF_LOGO);
		logoext = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOEXT, 0);

		fHideCDROMsSubMenu = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECDROMSSUBMENU, 0);

		priority = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITY, NORMAL_PRIORITY_CLASS);
		::SetPriorityClass(::GetCurrentProcess(), priority);
		launchfullscreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LAUNCHFULLSCREEN, FALSE);

		fEnableWebServer = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWEBSERVER, FALSE);
		nWebServerPort = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPORT, 13579);
		fWebServerPrintDebugInfo = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPRINTDEBUGINFO, FALSE);
		fWebServerUseCompression = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERUSECOMPRESSION, TRUE);
		fWebServerLocalhostOnly = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERLOCALHOSTONLY, FALSE);
		WebRoot = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBROOT, _T("*./webroot"));
		WebDefIndex = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBDEFINDEX, _T("index.html;index.php"));
		WebServerCGI = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBSERVERCGI, _T(""));

		CString MyPictures;

		CRegKey key;
		// grrrrr
		// if(!SHGetSpecialFolderPath(NULL, MyPictures.GetBufferSetLength(_MAX_PATH), CSIDL_MYPICTURES, TRUE)) MyPictures.Empty();
		// else MyPictures.ReleaseBuffer();
		if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), KEY_READ))
		{
			ULONG len = _MAX_PATH;
			if(ERROR_SUCCESS == key.QueryStringValue(_T("My Pictures"), MyPictures.GetBuffer(_MAX_PATH), &len)) MyPictures.ReleaseBufferSetLength(len);
			else MyPictures.Empty();
		}
		SnapShotPath = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTPATH, MyPictures);
		SnapShotExt = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTEXT, _T(".jpg"));

		ThumbRows = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBROWS, 4);
		ThumbCols = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBCOLS, 4);
		ThumbWidth = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBWIDTH, 1024);

		ISDb = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_ISDB, _T("www.opensubtitles.org/isdb"));

		D3D9RenderDevice = pApp->GetProfileString(IDS_R_SETTINGS, IDS_D3D9RENDERDEVICE, _T(""));

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("LastUsedPage"), 0);

		//

		m_shaders.RemoveAll();

		CAtlStringMap<UINT> shaders;

		shaders[_T("16-235 -> 0-255  [SD][HD]")] = IDF_SHADER_LEVELS;
		shaders[_T("16-235 -> 0-255  [SD]")] = IDF_SHADER_LEVELS2;
		shaders[_T("0-255 -> 16-235")] = IDF_SHADER_LEVELS3;
		shaders[_T("BT.601 -> BT.709")] = IDF_SHADER_BT601_BT709;
		shaders[_T("contour")] = IDF_SHADER_CONTOUR;
		shaders[_T("deinterlace (blend)")] = IDF_SHADER_DEINTERLACE;
		shaders[_T("edge sharpen")] = IDF_SHADER_EDGE_SHARPEN;
		shaders[_T("emboss")] = IDF_SHADER_EMBOSS;
		shaders[_T("grayscale")] = IDF_SHADER_GRAYSCALE;
		shaders[_T("invert")] = IDF_SHADER_INVERT;
		shaders[_T("letterbox")] = IDF_SHADER_LETTERBOX;
		shaders[_T("nightvision")] = IDF_SHADER_NIGHTVISION;
		shaders[_T("procamp")] = IDF_SHADER_PROCAMP;
		shaders[_T("sharpen")] = IDF_SHADER_SHARPEN;
		shaders[_T("sharpen complex")] = IDF_SHADER_SHARPEN_COMPLEX;
		shaders[_T("sharpen complex 2")] = IDF_SHADER_SHARPEN_COMPLEX2;
		shaders[_T("sphere")] = IDF_SHADER_SPHERE;
		shaders[_T("spotlight")] = IDF_SHADER_SPOTLIGHT;
		shaders[_T("wave")] = IDF_SHADER_WAVE;
		shaders[_T("denoise")] = IDF_SHADER_DENOISE;
		shaders[_T("YV12 Chroma Upsampling")] = IDF_SHADER_YV12CHROMAUP;

		int iShader = 0;

		for(; ; iShader++)
		{
			CString str;
			str.Format(_T("%d"), iShader);
			str = pApp->GetProfileString(_T("Shaders"), str);

			CAtlList<CString> sl;
			CString label = Explode(str, sl, '|');
			if(label.IsEmpty()) break;
			if(sl.GetCount() < 3) continue;

			Shader s;
			s.label = sl.RemoveHead();
			s.target = sl.RemoveHead();
			s.srcdata = sl.RemoveHead();
			s.srcdata.Replace(_T("\\n"), _T("\n"));
			s.srcdata.Replace(_T("\\t"), _T("\t"));
			m_shaders.AddTail(s);

			shaders.RemoveKey(s.label);
		}

		pos = shaders.GetStartPosition();
		for(; pos; iShader++)
		{
			CAtlStringMap<UINT>::CPair* pPair = shaders.GetNext(pos);

			CStringA srcdata;
			if(LoadResource(pPair->m_value, srcdata, _T("FILE")))
			{
				Shader s;
				s.label = pPair->m_key;

				// Select minimum version for each shader!
				switch (pPair->m_value)
				{
				case IDF_SHADER_DENOISE :
					s.target = _T("ps_3_0");
					break;
				case IDF_SHADER_SHARPEN_COMPLEX2 :
					s.target = _T("ps_2_a");
					break;
				default :
					s.target = _T("ps_2_0");
					break;
				}
				s.srcdata = CString(srcdata);
				m_shaders.AddTail(s);
			}
		}

		// CASIMIR666 : new settings
		fD3DFullscreen			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_D3DFULLSCREEN, FALSE);
		fMonitorAutoRefreshRate	= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MONITOR_AUTOREFRESHRATE, FALSE);

		dBrightness		= (float)_tstof(pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_BRIGHTNESS,	_T("1")));
		dContrast		= (float)_tstof(pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_CONTRAST,		_T("1")));
		dHue			= (float)_tstof(pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_HUE,			_T("0")));
		dSaturation		= (float)_tstof(pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_SATURATION,	_T("1")));
		strShaderList	= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLIST, _T(""));
		strShaderListScreenSpace	= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLISTSCREENSPACE, _T(""));
		m_bToggleShader = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADER, 0);
		m_bToggleShaderScreenSpace = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADERSSCREENSPACE, 0);

		m_RenderersSettings.iEvrBuffers		= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_EVR_BUFFERS, 5);
		fShowOSD		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SHOWOSD, 1);
		fEnableEDLEditor= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEEDLEDITOR, FALSE);

		// Save analog capture settings
		iDefaultCaptureDevice = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULT_CAPTURE, 0);
		strAnalogVideo		= pApp->GetProfileString(IDS_RS_CAPTURE, IDS_RS_VIDEO_DISP_NAME, _T(""));
		strAnalogAudio		= pApp->GetProfileString(IDS_RS_CAPTURE, IDS_RS_AUDIO_DISP_NAME, _T(""));
		iAnalogCountry		= pApp->GetProfileInt(IDS_RS_CAPTURE, IDS_RS_COUNTRY, 1);

		BDANetworkProvider	= pApp->GetProfileString(IDS_RS_DVB, IDS_RS_BDA_NETWORKPROVIDER, _T(""));
		BDATuner			= pApp->GetProfileString(IDS_RS_DVB, IDS_RS_BDA_TUNER, _T(""));
		BDAReceiver			= pApp->GetProfileString(IDS_RS_DVB, IDS_RS_BDA_RECEIVER, _T(""));
		DVBLastChannel		= pApp->GetProfileInt(IDS_RS_DVB, IDS_RS_DVB_LAST_CHANNEL, 1);

		for(int iChannel = 0; ; iChannel++)
		{
			CString		strTemp;
			CString		strChannel;
			CDVBChannel	Channel;
			strTemp.Format(_T("%d"), iChannel);
			strChannel = pApp->GetProfileString(IDS_RS_DVB, strTemp, _T(""));
			if (strChannel.IsEmpty()) break;
			Channel.FromString(strChannel);
			DVBChannels.AddTail (Channel);
		}

		// playback positions for last played DVDs
		fRememberDVDPos		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DVDPOS, 0);
		nCurrentDvdPosition = -1;
		memset (DvdPosition, 0, sizeof(DvdPosition));
		for (int i=0; i<MAX_DVD_POSITION; i++)
		{
			CString		strDVDPos;
			CString		strValue;

			strDVDPos.Format (_T("DVD Position %d"), i);
			strValue = pApp->GetProfileString(IDS_R_SETTINGS, strDVDPos, _T(""));
			if (strValue.GetLength()/2 == sizeof(DVD_POSITION))
			{
				DeserializeHex(strValue, (BYTE*)&DvdPosition[i], sizeof(DVD_POSITION));
			}
		}

		// playback positions for last played files
		fRememberFilePos		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOS, 0);
		nCurrentFilePosition = -1;
		for (int i=0; i<MAX_FILE_POSITION; i++)
		{
			CString		strFilePos;
			CString		strValue;

			strFilePos.Format (_T("File Name %d"), i);
			FilePosition[i].strFile = pApp->GetProfileString(IDS_R_SETTINGS, strFilePos, _T(""));

			strFilePos.Format (_T("File Position %d"), i);
			strValue = pApp->GetProfileString(IDS_R_SETTINGS, strFilePos, _T(""));
			FilePosition[i].llPosition = _tstoi64 (strValue);
		}

		fLastFullScreen		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTFULLSCREEN, 0);

		// CASIMIR666 : end of new settings

		// TODO: sort shaders by label

		m_shadercombine = pApp->GetProfileString(_T("Shaders"), _T("Combine"), _T(""));
		m_shadercombineScreenSpace = pApp->GetProfileString(_T("Shaders"), _T("CombineScreenSpace"), _T(""));

		if(launchfullscreen) nCLSwitches |= CLSW_FULLSCREEN;

		fInitialized = true;
	}
}

void CAppSettings::SaveCurrentFilePosition( )
{
	CWinApp* pApp = AfxGetApp();
	CString		strFilePos;
	CString		strValue;
	int i = nCurrentFilePosition;

	strFilePos.Format (_T("File Name %d"), i);
	pApp->WriteProfileString(IDS_R_SETTINGS, strFilePos, FilePosition[i].strFile);
	strFilePos.Format (_T("File Position %d"), i);
	strValue.Format (_T("%I64d"), FilePosition[i].llPosition);
	pApp->WriteProfileString(IDS_R_SETTINGS, strFilePos, strValue);
}

void CAppSettings::SaveCurrentDVDPosition( )
{
	CWinApp* pApp = AfxGetApp();
	CString		strDVDPos;
	CString		strValue;
	int i = nCurrentDvdPosition;

	strDVDPos.Format (_T("DVD Position %d"), i);
	strValue = SerializeHex((BYTE*)&DvdPosition[i], sizeof(DVD_POSITION));
	pApp->WriteProfileString(IDS_R_SETTINGS, strDVDPos, strValue);
}

__int64 CAppSettings::ConvertTimeToMSec(CString& time)
{
	__int64 Sec = 0;
	__int64 mSec = 0;
	__int64 mult = 1;

	int pos = time.GetLength() - 1;
	if (pos < 3) return 0;

	while (pos >= 0)
	{
		TCHAR ch = time[pos];
		if (ch == '.')
		{
			mSec = Sec * 1000 / mult;
			Sec = 0;
			mult = 1;
		}
		else if (ch == ':')
		{
			mult = mult * 6 / 10;
		}
		else if (ch >= '0' && ch <= '9')
		{
			Sec += (ch - '0') * mult;
			mult *= 10;
		}
		else
		{
			mSec = Sec = 0;
			break;
		}
		pos--;
	}
	return Sec*1000 + mSec;
}

void CAppSettings::ExtractDVDStartPos(CString& strParam)
{
	int i = 0, j = 0;
	for(CString token = strParam.Tokenize(_T("#"), i);
			j < 3 && !token.IsEmpty();
			token = strParam.Tokenize(_T("#"), i), j++)
	{
		switch (j)
		{
		case 0 :
			lDVDTitle = token.IsEmpty() ? 0 : (ULONG)_wtol(token);
			break;
		case 1 :
			if (token.Find(':') >0)
			{
				_stscanf_s(token, _T("%02d:%02d:%02d.%03d"), &DVDPosition.bHours, &DVDPosition.bMinutes, &DVDPosition.bSeconds, &DVDPosition.bFrames);
				/* Hack by Ron.  If bFrames >= 30, PlayTime commands fail due to invalid arg */
				DVDPosition.bFrames = 0;
			}
			else
				lDVDChapter = token.IsEmpty() ? 0 : (ULONG)_wtol(token);
			break;
		}
	}
}

void CAppSettings::ParseCommandLine(CAtlList<CString>& cmdln)
{
	nCLSwitches = 0;
	slFiles.RemoveAll();
	slDubs.RemoveAll();
	slSubs.RemoveAll();
	slFilters.RemoveAll();
	rtStart = 0;
	rtShift = 0;
	lDVDTitle = 0;
	lDVDChapter = 0;
	memset (&DVDPosition, 0, sizeof(DVDPosition));
	iAdminOption=0;
	fixedWindowSize.SetSize(0, 0);
	iMonitor = 0;
	hMasterWnd = 0;
	sPnSPreset.Empty();

	POSITION pos = cmdln.GetHeadPosition();
	while(pos)
	{
		CString param = cmdln.GetNext(pos);
		if(param.IsEmpty()) continue;

		if((param[0] == '-' || param[0] == '/') && param.GetLength() > 1)
		{
			CString sw = param.Mid(1).MakeLower();
			if(sw == _T("open")) nCLSwitches |= CLSW_OPEN;
			else if(sw == _T("play")) nCLSwitches |= CLSW_PLAY;
			else if(sw == _T("fullscreen")) nCLSwitches |= CLSW_FULLSCREEN;
			else if(sw == _T("minimized")) nCLSwitches |= CLSW_MINIMIZED;
			else if(sw == _T("new")) nCLSwitches |= CLSW_NEW;
			else if(sw == _T("help") || sw == _T("h") || sw == _T("?")) nCLSwitches |= CLSW_HELP;
			else if(sw == _T("dub") && pos) slDubs.AddTail(cmdln.GetNext(pos));
			else if(sw == _T("dubdelay") && pos)
			{
				CString		strFile = cmdln.GetNext(pos);
				int			nPos  = strFile.Find (_T("DELAY"));
				if (nPos != -1)
					rtShift = 10000 * _tstol(strFile.Mid(nPos + 6));
				slDubs.AddTail(strFile);
			}
			else if(sw == _T("sub") && pos) slSubs.AddTail(cmdln.GetNext(pos));
			else if(sw == _T("filter") && pos) slFilters.AddTail(cmdln.GetNext(pos));
			else if(sw == _T("dvd")) nCLSwitches |= CLSW_DVD;
			else if(sw == _T("dvdpos")) ExtractDVDStartPos(cmdln.GetNext(pos));
			else if(sw == _T("cd")) nCLSwitches |= CLSW_CD;
			else if(sw == _T("add")) nCLSwitches |= CLSW_ADD;
			else if(sw == _T("regvid")) nCLSwitches |= CLSW_REGEXTVID;
			else if(sw == _T("regaud")) nCLSwitches |= CLSW_REGEXTAUD;
			else if(sw == _T("unregall")) nCLSwitches |= CLSW_UNREGEXT;
			else if(sw == _T("unregvid")) nCLSwitches |= CLSW_UNREGEXT; /* keep for compatibility with old versions */
			else if(sw == _T("unregaud")) nCLSwitches |= CLSW_UNREGEXT; /* keep for compatibility with old versions */
			else if(sw == _T("start") && pos)
			{
				rtStart = 10000i64*_tcstol(cmdln.GetNext(pos), NULL, 10);
				nCLSwitches |= CLSW_STARTVALID;
			}
			else if(sw == _T("startpos") && pos)
			{
				rtStart = 10000i64 * ConvertTimeToMSec(cmdln.GetNext(pos));
				nCLSwitches |= CLSW_STARTVALID;
			}
			else if(sw == _T("nofocus")) nCLSwitches |= CLSW_NOFOCUS;
			else if(sw == _T("close")) nCLSwitches |= CLSW_CLOSE;
			else if(sw == _T("standby")) nCLSwitches |= CLSW_STANDBY;
			else if(sw == _T("hibernate")) nCLSwitches |= CLSW_HIBERNATE;
			else if(sw == _T("shutdown")) nCLSwitches |= CLSW_SHUTDOWN;
			else if(sw == _T("logoff")) nCLSwitches |= CLSW_LOGOFF;
			else if(sw == _T("d3dfs")) nCLSwitches |= CLSW_D3DFULLSCREEN;
			else if(sw == _T("adminoption"))
			{
				nCLSwitches |= CLSW_ADMINOPTION;
				iAdminOption = _ttoi (cmdln.GetNext(pos));
			}
			else if(sw == _T("slave"))
			{
				nCLSwitches |= CLSW_SLAVE;
				hMasterWnd = (HWND)_ttol (cmdln.GetNext(pos));
			}
			else if(sw == _T("fixedsize") && pos)
			{
				CAtlList<CString> sl;
				Explode(cmdln.GetNext(pos), sl, ',', 2);
				if(sl.GetCount() == 2)
				{
					fixedWindowSize.SetSize(_ttol(sl.GetHead()), _ttol(sl.GetTail()));
					if(fixedWindowSize.cx > 0 && fixedWindowSize.cy > 0)
						nCLSwitches |= CLSW_FIXEDSIZE;
				}
			}
			else if(sw == _T("monitor") && pos)
			{
				iMonitor = _tcstol(cmdln.GetNext(pos), NULL, 10);
				nCLSwitches |= CLSW_MONITOR;
			}
			else if(sw == _T("minidump"))
			{
				CMiniDump::Enable();
			}
			else if(sw == _T("pns")) sPnSPreset = cmdln.GetNext(pos);
			else if(sw == _T("webport") && pos)
			{
				int tmpport = _tcstol(cmdln.GetNext(pos), NULL, 10);
				if ( tmpport >= 0 && tmpport <= 65535 )
					nCmdlnWebServerPort = tmpport;
			}
			else if(sw == _T("debug"))
			{
				ShowDebugInfo = true;
			}
			else if(sw == _T("audiorenderer") && pos)
			{
				SetAudioRenderer(_ttoi(cmdln.GetNext(pos)));
			}
			else nCLSwitches |= CLSW_HELP|CLSW_UNRECOGNIZEDSWITCH;
		}
		else
		{
			slFiles.AddTail(param);
		}
	}
}

void CAppSettings::GetFav(favtype ft, CAtlList<CString>& sl)
{
	sl.RemoveAll();

	CString root;

	switch(ft)
	{
	case FAV_FILE:
		root = IDS_R_FAVFILES;
		break;
	case FAV_DVD:
		root = IDS_R_FAVDVDS;
		break;
	case FAV_DEVICE:
		root = IDS_R_FAVDEVICES;
		break;
	default:
		return;
	}

	for(int i = 0; ; i++)
	{
		CString s;
		s.Format(_T("Name%d"), i);
		s = AfxGetApp()->GetProfileString(root, s, NULL);
		if(s.IsEmpty()) break;
		sl.AddTail(s);
	}
}

void CAppSettings::SetFav(favtype ft, CAtlList<CString>& sl)
{
	CString root;

	switch(ft)
	{
	case FAV_FILE:
		root = IDS_R_FAVFILES;
		break;
	case FAV_DVD:
		root = IDS_R_FAVDVDS;
		break;
	case FAV_DEVICE:
		root = IDS_R_FAVDEVICES;
		break;
	default:
		return;
	}

	AfxGetApp()->WriteProfileString(root, NULL, NULL);

	int i = 0;
	POSITION pos = sl.GetHeadPosition();
	while(pos)
	{
		CString s;
		s.Format(_T("Name%d"), i++);
		AfxGetApp()->WriteProfileString(root, s, sl.GetNext(pos));
	}
}

void CAppSettings::AddFav(favtype ft, CString s)
{
	CAtlList<CString> sl;
	GetFav(ft, sl);
	if(sl.Find(s)) return;
	sl.AddTail(s);
	SetFav(ft, sl);
}

CDVBChannel* CAppSettings::FindChannelByPref(int nPrefNumber)
{
	POSITION	pos = DVBChannels.GetHeadPosition();
	while (pos)
	{
		CDVBChannel&	Channel = DVBChannels.GetNext (pos);
		if (Channel.GetPrefNumber() == nPrefNumber)
		{
			return &Channel;
		}
	}

	return NULL;
}

// Settings::CRecentFileAndURLList

CAppSettings::CRecentFileAndURLList::CRecentFileAndURLList(UINT nStart, LPCTSTR lpszSection,
		LPCTSTR lpszEntryFormat, int nSize,
		int nMaxDispLen)
	: CRecentFileList(nStart, lpszSection, lpszEntryFormat, nSize, nMaxDispLen)
{
}

//#include <afximpl.h>
extern BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
extern BOOL AFXAPI AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2);

void CAppSettings::CRecentFileAndURLList::Add(LPCTSTR lpszPathName)
{
	ASSERT(m_arrNames != NULL);
	ASSERT(lpszPathName != NULL);
	ASSERT(AfxIsValidString(lpszPathName));

	if(CString(lpszPathName).MakeLower().Find(_T("@device:")) >= 0)
		return;

	bool fURL = (CString(lpszPathName).Find(_T("://")) >= 0);

	// fully qualify the path name
	TCHAR szTemp[1024];
	if(fURL) _tcscpy_s(szTemp, lpszPathName);
	else AfxFullPath(szTemp, lpszPathName);

	// update the MRU list, if an existing MRU string matches file name
	int iMRU;
	for (iMRU = 0; iMRU < m_nSize-1; iMRU++)
	{
		if((fURL && !_tcscmp(m_arrNames[iMRU], szTemp))
				|| AfxComparePath(m_arrNames[iMRU], szTemp))
			break;		// iMRU will point to matching entry
	}
	// move MRU strings before this one down
	for (; iMRU > 0; iMRU--)
	{
		ASSERT(iMRU > 0);
		ASSERT(iMRU < m_nSize);
		m_arrNames[iMRU] = m_arrNames[iMRU-1];
	}
	// place this one at the beginning
	m_arrNames[0] = szTemp;
}
