/*
 * (C) 2009-2014 see Authors.txt
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

#pragma once

#define IDS_R_SETTINGS                      _T("Settings")
#define IDS_R_SETTINGS_FULLSCREEN_AUTOCHANGE_MODE IDS_R_SETTINGS _T("\\FullscreenAutoChangeMode")
#define IDS_R_VERSION                       _T("SettingsVersion")
#define IDS_R_FILTERS                       _T("Filters")
#define IDS_R_EXTERNAL_FILTERS_x86          _T("Filters\\x86")
#define IDS_R_EXTERNAL_FILTERS_x64          _T("Filters\\x64")
#ifndef _WIN64
#define IDS_R_EXTERNAL_FILTERS              IDS_R_EXTERNAL_FILTERS_x86
#else
#define IDS_R_EXTERNAL_FILTERS              IDS_R_EXTERNAL_FILTERS_x64
#endif
#define IDS_R_INTERNAL_FILTERS              _T("Internal Filters")
#define IDS_R_FAVFILES                      _T("Favorites\\Files")
#define IDS_R_FAVDVDS                       _T("Favorites\\DVDs")
#define IDS_R_FAVDEVICES                    _T("Favorites\\Devices")
#define IDS_R_COMMANDS                      _T("Commands2")
#define IDS_R_LOGINS                        _T("Logins")
#define IDS_R_FAVORITES                     _T("Favorites")

#define IDS_RS_FAV_REMEMBERPOS              _T("RememberPosition")
#define IDS_RS_FAV_RELATIVEDRIVE            _T("RelativeDrive")
#define IDS_RS_DVDPOS                       _T("RememberDVDPos")
#define IDS_RS_FILEPOS                      _T("RememberFilePos")
#define IDS_RS_FILEPOSLONGER                _T("RememberPosForLongerThan")
#define IDS_RS_FILEPOSAUDIO                 _T("RememberPosForAudioFiles")
#define IDS_RS_LASTFULLSCREEN               _T("LastFullScreen")
#define IDS_RS_EVR_BUFFERS                  _T("EVRBuffers")
#define IDS_RS_SHOWOSD                      _T("ShowOSD")
#define IDS_RS_LANGUAGE                     _T("InterfaceLanguage")
#define IDS_RS_GLOBALMEDIA                  _T("UseGlobalMedia")
#define IDS_RS_DXVAFILTERS                  _T("DXVAFilters")
#define IDS_RS_FFMPEGFILTERS                _T("FFmpegFilters")
#define IDS_RS_TITLEBARTEXTSTYLE            _T("TitleBarTextStyle")
#define IDS_RS_CONTROLSTATE                 _T("ControlState")
#define IDS_RS_LOOP                         _T("Loop")
#define IDS_RS_LOOPNUM                      _T("LoopNum")
#define IDS_RS_SNAPTODESKTOPEDGES           _T("SnapToDesktopEdges")
#define IDS_RS_ENABLESUBTITLES              _T("EnableSubtitles")
#define IDS_RS_PREFER_FORCED_DEFAULT_SUBTITLES _T("PreferForcedDefaultSubtitles")
#define IDS_RS_PRIORITIZEEXTERNALSUBTITLES  _T("PrioritizeExternalSubtitles")
#define IDS_RS_DISABLEINTERNALSUBTITLES     _T("DisableInternalSubtitles")
#define IDS_RS_ALLOW_OVERRIDING_EXT_SPLITTER _T("AllowOverridingExternalSplitterSubtitleChoice")
#define IDS_RS_SUBTITLEPATHS                _T("SubtitlePaths")
#define IDS_RS_USEDEFAULTSUBTITLESSTYLE     _T("UseDefaultsubtitlesStyle")
#define IDS_RS_THUMBWIDTH                   _T("ThumbWidth")
#define IDS_RS_SUBSAVEEXTERNALSTYLEFILE     _T("SubSaveExternalStyleFile")
#define IDS_RS_D3DFULLSCREEN                _T("D3DFullScreen")
//#define IDS_RS_MONITOR_AUTOREFRESHRATE      _T("MonitorAutoRefreshRate")
#define IDS_RS_SPEEDSTEP                    _T("SpeedStep")

// Audio
#define IDS_RS_VOLUME                       _T("Volume")
#define IDS_RS_MUTE                         _T("Mute")
#define IDS_RS_BALANCE                      _T("Balance")
#define IDS_RS_VOLUMESTEP                   _T("VolumeStep")

// AudioSwitcher
#define IDS_RS_ENABLEAUDIOSWITCHER          _T("EnableAudioSwitcher")
#define IDS_RS_AUDIONORMALIZE               _T("AudioNormalize")
#define IDS_RS_AUDIOMAXNORMFACTOR           _T("AudioMaxNormFactor")
#define IDS_RS_AUDIONORMALIZERECOVER        _T("AudioNormalizeRecover")
#define IDS_RS_AUDIOBOOST                   _T("AudioBoost")
#define IDS_RS_DOWNSAMPLETO441              _T("DownSampleTo441")
#define IDS_RS_ENABLEAUDIOTIMESHIFT         _T("EnableAudioTimeShift")
#define IDS_RS_AUDIOTIMESHIFT               _T("AudioTimeShift")
#define IDS_RS_CUSTOMCHANNELMAPPING         _T("CustomChannelMapping")
#define IDS_RS_SPEAKERCHANNELS              _T("SpeakerChannels")
#define IDS_RS_SPEAKERTOCHANNELMAPPING      _T("SpeakerToChannelMapping")

// Video
#define IDS_RS_COLOR_BRIGHTNESS             _T("VideoBrightness")
#define IDS_RS_COLOR_CONTRAST               _T("VideoContrast")
#define IDS_RS_COLOR_HUE                    _T("VideoHue")
#define IDS_RS_COLOR_SATURATION             _T("VideoSaturation")

// DVD/OGM
#define IDS_RS_DVDPATH                      _T("DVDPath")
#define IDS_RS_USEDVDPATH                   _T("UseDVDPath")
#define IDS_RS_MENULANG                     _T("MenuLang")
#define IDS_RS_AUDIOLANG                    _T("AudioLang")
#define IDS_RS_SUBTITLESLANG                _T("SubtitlesLang")
#define IDS_RS_AUTOSPEAKERCONF              _T("AutoSpeakerConf")
#define IDS_RS_CLOSEDCAPTIONS               _T("ClosedCaptions")

#define IDS_RS_TITLEBARTEXTTITLE            _T("TitleBarTextTitle")
#define IDS_RS_VMR9MIXERYUV                 _T("VMRMixerYUV")
#define IDS_RS_REWIND                       _T("Rewind")
#define IDS_RS_ZOOM                         _T("Zoom")
#define IDS_RS_MULTIINST                    _T("AllowMultipleInstances")
#define IDS_RS_ALWAYSONTOP                  _T("AlwaysOnTop")
#define IDS_RS_AUTOZOOM                     _T("AutoZoom")
#define IDS_RS_AUTOFITFACTOR                _T("AutoFitFactor")
#define IDS_RS_SPSTYLE                      _T("SPDefaultStyle")
#define IDS_RS_SPOVERRIDEPLACEMENT          _T("SPOverridePlacement")
#define IDS_RS_SPHORPOS                     _T("SPHorPos")
#define IDS_RS_SPVERPOS                     _T("SPVerPos")
#define IDS_RS_SUBTITLEARCOMPENSATION       _T("SubtitleARCompensation")
#define IDS_RS_SPCSIZE                      _T("SPCSize")
#define IDS_RS_SPCMAXRES                    _T("SPCMaxRes")
#define IDS_RS_DISABLE_SUBTITLE_ANIMATION   _T("DisableSubtitleAnimation")
#define IDS_RS_RENDER_AT_WHEN_ANIM_DISABLED _T("RenderAtWhenSubtitleAnimationIsDisabled")
#define IDS_RS_SUBTITLE_ANIMATION_RATE      _T("SubtitleAnimationRate")
#define IDS_RS_ALLOW_DROPPING_SUBPIC        _T("AllowDroppingSubpic")
#define IDS_RS_INTREALMEDIA                 _T("IntRealMedia")
#define IDS_RS_EXITFULLSCREENATTHEEND       _T("ExitFullscreenAtTheEnd")
#define IDS_RS_REMEMBERWINDOWPOS            _T("RememberWindowPos")
#define IDS_RS_LASTWINDOWRECT               _T("LastWindowRect")
#define IDS_RS_AUDIORENDERERTYPE            _T("AudioRendererType")
#define IDS_RS_HIDECAPTIONMENU              _T("HideCaptionMenu")
#define IDS_RS_HIDENAVIGATION               _T("HideNavigation")
#define IDS_RS_DEFAULTVIDEOFRAME            _T("DefaultVideoFrame")
#define IDS_RS_REMEMBERWINDOWSIZE           _T("RememberWindowSize")
#define IDS_RS_PANSCANZOOM                  _T("PanScanZoom")
#define IDS_RS_REALMEDIARENDERLESS          _T("RealMediaRenderless")
#define IDS_RS_QUICKTIMERENDERER            _T("QuickTimeRenderer")
#define IDS_RS_REALMEDIAFPS                 _T("RealMediaFPS")
#define IDS_RS_SUBDELAYINTERVAL             _T("SubDelayInterval")
#define IDS_RS_LOGOFILE                     _T("LogoFile")
#define IDS_RS_ENABLEWORKERTHREADFOROPENING _T("EnableWorkerThreadForOpening")
#define IDS_RS_PNSPRESETS                   _T("PnSPresets")
#define IDS_RS_AUTOLOADAUDIO                _T("AutoloadAudio")
#define IDS_RS_AUTOLOADSUBTITLES            _T("AutoloadSubtitles")
#define IDS_RS_SUBTITLESLANGORDER           _T("SubtitlesLanguageOrder")
#define IDS_RS_AUDIOSLANGORDER              _T("AudiosLanguageOrder")
#define IDS_RS_BLOCKVSFILTER                _T("BlockVSFilter")
#define IDS_RS_ACCELTBL                     _T("AccelTbl")
#define IDS_RS_WINLIRCADDR                  _T("WinLircAddr")
#define IDS_RS_WINLIRC                      _T("UseWinLirc")
#define IDS_RS_TRAYICON                     _T("TrayIcon")
#define IDS_RS_KEEPASPECTRATIO              _T("KeepAspectRatio")
#define IDS_RS_UICEADDR                     _T("UICEAddr")
#define IDS_RS_UICE                         _T("UseUICE")
#define IDS_RS_JUMPDISTS                    _T("JumpDistS")
#define IDS_RS_JUMPDISTM                    _T("JumpDistM")
#define IDS_RS_JUMPDISTL                    _T("JumpDistL")
#define IDS_RS_REPORTFAILEDPINS             _T("ReportFailedPins")
#define IDS_RS_SRCFILTERS                   _T("SrcFilters")
#define IDS_RS_KEEPHISTORY                  _T("KeepHistory")
#define IDS_RS_RECENT_FILES_NUMBER          _T("RecentFilesNumber")
#define IDS_RS_LOGOID                       _T("LogoID2")
#define IDS_RS_LOGOEXT                      _T("LogoExt")
#define IDS_RS_TRAFILTERS                   _T("TraFilters")
#define IDS_RS_COMPMONDESKARDIFF            _T("CompMonDeskARDiff")
#define IDS_RS_HIDECDROMSSUBMENU            _T("HideCDROMsSubMenu")
#define IDS_RS_VMRTEXTURE                   _T("VMRTexture")
#define IDS_RS_VMR3D                        _T("VMR3D")
#define IDS_RS_DSVIDEORENDERERTYPE          _T("DSVidRen")
#define IDS_RS_RMVIDEORENDERERTYPE          _T("RMVidRen")
#define IDS_RS_QTVIDEORENDERERTYPE          _T("QTVidRen")
#define IDS_RS_SHUFFLEPLAYLISTITEMS         _T("ShufflePlaylistItems")
#define IDS_RS_REMEMBERPLAYLISTITEMS        _T("RememberPlaylistItems")
#define IDS_RS_HIDEPLAYLISTFULLSCREEN       _T("HidePlaylistFullScreen")
#define IDS_RS_APSURACEFUSAGE               _T("APSurfaceUsage")
#define IDS_RS_ENABLEWEBSERVER              _T("EnableWebServer")
#define IDS_RS_WEBSERVERPORT                _T("WebServerPort")
#define IDS_RS_LASTWINDOWTYPE               _T("LastWindowType")
#define IDS_RS_ONTOP                        _T("OnTop")
#define IDS_RS_WEBSERVERPRINTDEBUGINFO      _T("WebServerPrintDebugIfo")
#define IDS_RS_WEBSERVERUSECOMPRESSION      _T("WebServerUseCompression")
#define IDS_RS_SNAPSHOTPATH                 _T("SnapshotPath")
#define IDS_RS_PRIORITY                     _T("Priority")
#define IDS_RS_SNAPSHOTEXT                  _T("SnapshotExt")
#define IDS_RS_LAUNCHFULLSCREEN             _T("LaunchFullScreen")
#define IDS_RS_ISDB                         _T("ISDb")
#define IDS_RS_WEBROOT                      _T("WebRoot")
#define IDS_RS_WEBSERVERLOCALHOSTONLY       _T("WebServerLocalhostOnly")
#define IDS_RS_ASPECTRATIO_X                _T("AspectRatioX")
#define IDS_RS_ASPECTRATIO_Y                _T("AspectRatioY")
#define IDS_RS_DX9_RESIZER                  _T("DX9Resizer")
#define IDS_RS_WEBSERVERCGI                 _T("WebServerCGI")
#define IDS_RS_WEBDEFINDEX                  _T("WebDefIndex")
#define IDS_RS_LIMITWINDOWPROPORTIONS       _T("LimitWindowProportions")
#define IDS_RS_LASTUSEDPAGE                 _T("LastUsedPage")
#define IDS_RS_VMR9MIXERMODE                _T("VMR9MixerMode")
#define IDS_RS_THUMBROWS                    _T("ThumbRows")
#define IDS_RS_THUMBCOLS                    _T("ThumbCols")
#define IDS_RS_ENABLEEDLEDITOR              _T("EnableEDLEditor")
#define IDS_RS_FULLSCREENMONITOR            _T("FullScreenMonitor")
#define IDS_RS_PREVENT_MINIMIZE             _T("PreventMinimize")
#define IDS_RS_WIN7TASKBAR                  _T("UseWin7TaskBar")
#define IDS_RS_EXIT_AFTER_PB                _T("ExitAfterPlayBack")
#define IDS_RS_NEXT_AFTER_PB                _T("SearchInDirAfterPlayBack")
#define IDS_RS_SEARCH_IN_FOLDER             _T("UseSearchInFolder")
#define IDS_RS_USE_TIME_TOOLTIP             _T("UseTimeTooltip")
#define IDS_RS_TIME_TOOLTIP_POSITION        _T("TimeTooltipPosition")
#define IDS_RS_MPC_OSD_SIZE                 _T("OSDSize")
#define IDS_RS_MPC_OSD_FONT                 _T("OSDFont")
#define IDS_RS_LAST_OPEN_DIR                _T("LastOpenDir")
#define IDS_RS_ASSOCIATED_WITH_ICON         _T("AssociatedWithIcon")
#define IDS_RS_ICON_LIB_VERSION             _T("IconLibVersion")

#define IDS_RS_HIDE_FULLSCREEN_CONTROLS        _T("HideFullscreenControls")
#define IDS_RS_HIDE_FULLSCREEN_CONTROLS_POLICY _T("HideFullscreenControlsPolicy")
#define IDS_RS_HIDE_FULLSCREEN_CONTROLS_DELAY  _T("HideFullscreenControlsDelay")
#define IDS_RS_HIDE_FULLSCREEN_DOCKED_PANELS   _T("HideFullscreenDockedPanels")
#define IDS_RS_HIDE_WINDOWED_CONTROLS          _T("HideWindowedControls")

#define IDS_RS_HIDE_WINDOWED_MOUSE_POINTER  _T("HideWindowedMousePointer")

#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_ENABLE               _T("Enable")
#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_APPLYDEFMODEATFSEXIT _T("ApplyDefaultModeAtFSExit")
#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_RESTORERESAFTEREXIT  _T("RestoreResAfterExit")
#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_DELAY                _T("Delay")
#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE                 IDS_R_SETTINGS_FULLSCREEN_AUTOCHANGE_MODE _T("\\Mode%Id")
#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_CHECKED         _T("Checked")
#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_FRAMERATESTART  _T("FrameRateStart")
#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_FRAMERATESTOP   _T("FrameRateStop")
#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_BPP          _T("BPP")
#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_FREQ         _T("Freq")
#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_SIZEX        _T("SizeX")
#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_SIZEY        _T("SizeY")
#define IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_FLAGS        _T("Flags")

#define IDS_RS_DEFAULT_CAPTURE              _T("DefaultCapture")
#define IDS_R_CAPTURE                       _T("Capture")
#define IDS_RS_VIDEO_DISP_NAME              _T("VidDispName")
#define IDS_RS_AUDIO_DISP_NAME              _T("AudDispName")
#define IDS_RS_COUNTRY                      _T("Country")

#define IDS_R_DVB                           _T("DVBConfiguration")
#define IDS_RS_BDA_NETWORKPROVIDER          _T("BDANetworkProvider")
#define IDS_RS_BDA_TUNER                    _T("BDATuner")
#define IDS_RS_BDA_RECEIVER                 _T("BDAReceiver")
#define IDS_RS_BDA_STANDARD                 _T("BDAStandard")
#define IDS_RS_BDA_SCAN_FREQ_START          _T("BDAScanFreqStart")
#define IDS_RS_BDA_SCAN_FREQ_END            _T("BDAScanFreqEnd")
#define IDS_RS_BDA_BANDWIDTH                _T("BDABandWidth")
#define IDS_RS_BDA_USE_OFFSET               _T("BDAUseOffset")
#define IDS_RS_BDA_OFFSET                   _T("BDAOffset")
#define IDS_RS_BDA_IGNORE_ENCRYPTED_CHANNELS _T("BDAIgnoreEncryptedChannels")
#define IDS_RS_DVB_LAST_CHANNEL             _T("LastChannel")
#define IDS_RS_DVB_REBUILD_FG               _T("RebuildFilterGraph")
#define IDS_RS_DVB_STOP_FG                  _T("StopFilterGraph")

#define IDS_RS_D3D9RENDERDEVICE             _T("D3D9RenderDevice")

#define IDS_RS_FASTSEEK                     _T("FastSeek")
#define IDS_RS_FASTSEEK_METHOD              _T("FastSeekMethod")
#define IDS_RS_SHOW_CHAPTERS                _T("ShowChapters")

#define IDS_RS_LCD_SUPPORT                  _T("LcdSupport")

#define IDS_RS_REMAINING_TIME               _T("RemainingTime")

#define IDS_RS_UPDATER_AUTO_CHECK           _T("UpdaterAutoCheck")
#define IDS_RS_UPDATER_LAST_CHECK           _T("UpdaterLastCheck")
#define IDS_RS_UPDATER_DELAY                _T("UpdaterDelay")
#define IDS_RS_UPDATER_IGNORE_VERSION       _T("UpdaterIgnoreVersion")

#define IDS_RS_NOTIFY_SKYPE                 _T("NotifySkype")
#define IDS_RS_JPEG_QUALITY                 _T("JpegQuality")

#define IDS_RS_GOTO_LAST_USED               _T("GoToLastUsed")
#define IDS_RS_GOTO_FPS                     _T("GoToFPS")

#define IDS_R_DLG_SUBTITLEDL                _T("Dialogs\\SubtitleDl")
#define IDS_RS_DLG_SUBTITLEDL_COLWIDTH      _T("ColWidth")

#define IDS_R_DLG_ORGANIZE_FAV              _T("Dialogs\\OrganizeFavorites")

#define IDS_R_SHADERS                       _T("Shaders")
#define IDS_RS_SHADERS_EXTRA                _T("Extra")
#define IDS_RS_SHADERS_PRERESIZE            _T("PreResize")
#define IDS_RS_SHADERS_POSTRESIZE           _T("PostResize")
#define IDS_RS_SHADERS_LASTPRESET           _T("LastPreset")
#define IDS_R_SHADER_PRESETS                _T("Shaders\\Presets")
#define IDS_R_DEBUG_SHADERS                 _T("Dialogs\\DebugShaders")
#define IDS_RS_DEBUG_SHADERS_LASTVERSION    _T("LastVersion")
#define IDS_RS_DEBUG_SHADERS_LASTFILE       _T("LastFile")
#define IDS_RS_DEBUG_SHADERS_FIRSTRUN       _T("FirstRun")
