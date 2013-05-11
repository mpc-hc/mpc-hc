/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include "mplayerc.h"
#include "AppSettings.h"
#include "FGFilter.h"
#include "FileAssoc.h"
#include "MiniDump.h"
#include "SysVersion.h"
#include "WinAPIUtils.h"
#include "UpdateChecker.h"


CAppSettings::CAppSettings()
    : fInitialized(false)
    , MRU(0, _T("Recent File List"), _T("File%d"), 20)
    , MRUDub(0, _T("Recent Dub List"), _T("Dub%d"), 20)
    , filePositions(AfxGetApp(), IDS_R_SETTINGS, MAX_FILE_POSITION)
    , dvdPositions(AfxGetApp(), IDS_R_SETTINGS, MAX_DVD_POSITION)
    , hAccel(nullptr)
    , nCmdlnWebServerPort(-1)
    , fShowDebugInfo(false)
    , hMasterWnd(nullptr)
    , nCLSwitches(0)
    , iMonitor(0)
    , fMute(0)
    , fPreventMinimize(0)
    , fUseWin7TaskBar(TRUE)
    , fExitAfterPlayback(0)
    , fNextInDirAfterPlayback(0)
    , fUseSearchInFolder(0)
    , fUseTimeTooltip(TRUE)
    , nTimeTooltipPosition(TIME_TOOLTIP_ABOVE_SEEKBAR)
    , nOSDSize(0)
    , nSpeakerChannels(2)
    , fRemainingTime(FALSE)
    , nUpdaterAutoCheck(-1)
    , nUpdaterDelay(7)
    , fShaderEditorWasOpened(false)
    , fBDAUseOffset(0)
    , iBDAOffset(166)
    , fBDAIgnoreEncryptedChannels(0)
    , nDVBLastChannel(1)
    , fEnableAudioSwitcher(TRUE)
    , fAudioNormalize(FALSE)
    , fAudioNormalizeRecover(TRUE)
    , fDownSampleTo441(0)
    , fAudioTimeShift(0)
    , iAudioTimeShift(0)
    , fCustomChannelMapping(0)
    , fOverridePlacement(0)
    , nHorPos(50)
    , nVerPos(90)
    , nSubDelayInterval(500)
    , fPrioritizeExternalSubtitles(TRUE)
    , fDisableInternalSubtitles(FALSE)
    , nJumpDistS(DEFAULT_JUMPDISTANCE_1)
    , nJumpDistM(DEFAULT_JUMPDISTANCE_2)
    , nJumpDistL(DEFAULT_JUMPDISTANCE_3)
    , fFastSeek(FALSE)
    , fShowChapters(TRUE)
    , fLCDSupport(FALSE)
    , iBrightness(0)
    , iContrast(0)
    , iHue(0)
    , iSaturation(0)
    , iCaptionMenuMode(MODE_SHOWCAPTIONMENU)
    , fHideNavigation(0)
    , nCS(CS_SEEKBAR | CS_TOOLBAR | CS_STATUSBAR)
    , language(-1)
    , fEnableSubtitles(TRUE)
    , fUseDefaultSubtitlesStyle(FALSE)
    , iDefaultVideoSize(DVS_FROMINSIDE)
    , fKeepAspectRatio(TRUE)
    , fCompMonDeskARDiff(FALSE)
    , iOnTop(0)
    , bFavRememberPos(TRUE)
    , bFavRelativeDrive(FALSE)
    , iThumbRows(4)
    , iThumbCols(4)
    , iThumbWidth(1024)
    , fToggleShader(0)
    , fToggleShaderScreenSpace(0)
    , bShufflePlaylistItems(FALSE)
    , bHidePlaylistFullScreen(FALSE)
    , nLastWindowType(SIZE_RESTORED)
    , nLastUsedPage(0)
    , fLastFullScreen(0)
    , fIntRealMedia(0)
    , fEnableEDLEditor(false)
    , bNotifySkype(FALSE)
{
    // Internal source filter
#if INTERNAL_SOURCEFILTER_CDDA
    SrcFiltersKeys[SRC_CDDA] = _T("SRC_CDDA");
#endif
#if INTERNAL_SOURCEFILTER_CDXA
    SrcFiltersKeys[SRC_CDXA] = _T("SRC_CDXA");
#endif
#if INTERNAL_SOURCEFILTER_VTS
    SrcFiltersKeys[SRC_VTS] = _T("SRC_VTS");
#endif
#if INTERNAL_SOURCEFILTER_FLIC
    SrcFiltersKeys[SRC_FLIC] = _T("SRC_FLIC");
#endif
#if INTERNAL_SOURCEFILTER_DVSOURCE
    SrcFiltersKeys[SRC_D2V] = _T("SRC_D2V");
#endif
#if INTERNAL_SOURCEFILTER_DTSAC3
    SrcFiltersKeys[SRC_DTSAC3] = _T("SRC_DTSAC3");
#endif
#if INTERNAL_SOURCEFILTER_MATROSKA
    SrcFiltersKeys[SRC_MATROSKA] = _T("SRC_MATROSKA");
#endif
#if INTERNAL_SOURCEFILTER_SHOUTCAST
    SrcFiltersKeys[SRC_SHOUTCAST] = _T("SRC_SHOUTCAST");
#endif
#if INTERNAL_SOURCEFILTER_REALMEDIA
    SrcFiltersKeys[SRC_REALMEDIA] = _T("SRC_REALMEDIA");
#endif
#if INTERNAL_SOURCEFILTER_AVI
    SrcFiltersKeys[SRC_AVI] = _T("SRC_AVI");
#endif
#if INTERNAL_SOURCEFILTER_OGG
    SrcFiltersKeys[SRC_OGG] = _T("SRC_OGG");
#endif
#if INTERNAL_SOURCEFILTER_MPEG
    SrcFiltersKeys[SRC_MPEG] = _T("SRC_MPEG");
#endif
#if INTERNAL_SOURCEFILTER_MPEGAUDIO
    SrcFiltersKeys[SRC_MPA] = _T("SRC_MPA");
#endif
#if INTERNAL_SOURCEFILTER_DSM
    SrcFiltersKeys[SRC_DSM] = _T("SRC_DSM");
#endif
    SrcFiltersKeys[SRC_SUBS] = _T("SRC_SUBS");
#if INTERNAL_SOURCEFILTER_MP4
    SrcFiltersKeys[SRC_MP4] = _T("SRC_MP4");
#endif
#if INTERNAL_SOURCEFILTER_FLV
    SrcFiltersKeys[SRC_FLV] = _T("SRC_FLV");
#endif
#if INTERNAL_SOURCEFILTER_FLAC
    SrcFiltersKeys[SRC_FLAC] = _T("SRC_FLAC");
#endif
#if INTERNAL_SOURCEFILTER_RFS
    SrcFiltersKeys[SRC_RFS] = _T("SRC_RFS");
#endif

    // Internal decoders
#if INTERNAL_DECODER_MPEG1
    TraFiltersKeys[TRA_MPEG1] = _T("TRA_MPEG1");
#endif
#if INTERNAL_DECODER_MPEG2
    TraFiltersKeys[TRA_MPEG2] = _T("TRA_MPEG2");
#endif
#if INTERNAL_DECODER_REALVIDEO
    TraFiltersKeys[TRA_RV] = _T("TRA_RV");
#endif
#if INTERNAL_DECODER_REALAUDIO
    TraFiltersKeys[TRA_RA] = _T("TRA_RA");
#endif
#if INTERNAL_DECODER_MPEGAUDIO
    TraFiltersKeys[TRA_MPA] = _T("TRA_MPA");
#endif
#if INTERNAL_DECODER_DTS
    TraFiltersKeys[TRA_DTS] = _T("TRA_DTS");
#endif
#if INTERNAL_DECODER_LPCM
    TraFiltersKeys[TRA_LPCM] = _T("TRA_LPCM");
#endif
#if INTERNAL_DECODER_AC3
    TraFiltersKeys[TRA_AC3] = _T("TRA_AC3");
#endif
#if INTERNAL_DECODER_AAC
    TraFiltersKeys[TRA_AAC] = _T("TRA_AAC");
#endif
#if INTERNAL_DECODER_ALAC
    TraFiltersKeys[TRA_ALAC] = _T("TRA_ALAC");
#endif
#if INTERNAL_DECODER_ALS
    TraFiltersKeys[TRA_ALS] = _T("TRA_ALS");
#endif
#if INTERNAL_DECODER_PS2AUDIO
    TraFiltersKeys[TRA_PS2AUD] = _T("TRA_PS2AUD");
#endif
#if INTERNAL_DECODER_VORBIS
    TraFiltersKeys[TRA_VORBIS] = _T("TRA_VORBIS");
#endif
#if INTERNAL_DECODER_FLAC
    TraFiltersKeys[TRA_FLAC] = _T("TRA_FLAC");
#endif
#if INTERNAL_DECODER_NELLYMOSER
    TraFiltersKeys[TRA_NELLY] = _T("TRA_NELLY");
#endif
#if INTERNAL_DECODER_AMR
    TraFiltersKeys[TRA_AMR] = _T("TRA_AMR");
#endif
#if INTERNAL_DECODER_PCM
    TraFiltersKeys[TRA_PCM] = _T("TRA_PCM");
#endif

    // Internal DXVA decoders
#if INTERNAL_DECODER_H264_DXVA
    DXVAFiltersKeys[TRA_DXVA_H264] = _T("TRA_DXVA_H264");
#endif
#if INTERNAL_DECODER_VC1_DXVA
    DXVAFiltersKeys[TRA_DXVA_VC1] = _T("TRA_DXVA_VC1");
#endif
#if INTERNAL_DECODER_WMV3_DXVA
    DXVAFiltersKeys[TRA_DXVA_WMV3] = _T("TRA_DXVA_WMV3");
#endif
#if INTERNAL_DECODER_MPEG2_DXVA
    DXVAFiltersKeys[TRA_DXVA_MPEG2] = _T("TRA_DXVA_MPEG2");
#endif

    // Internal FFmpeg decoders
#if INTERNAL_DECODER_H264
    FFMFiltersKeys[FFM_H264] = _T("FFM_H264");
#endif
#if INTERNAL_DECODER_VC1
    FFMFiltersKeys[FFM_VC1] = _T("FFM_VC1");
#endif
#if INTERNAL_DECODER_FLV
    FFMFiltersKeys[FFM_FLV4] = _T("FFM_FLV4");
#endif
#if INTERNAL_DECODER_VP356
    FFMFiltersKeys[FFM_VP356] = _T("FFM_VP356");
#endif
#if INTERNAL_DECODER_VP8
    FFMFiltersKeys[FFM_VP8] = _T("FFM_VP8");
#endif
#if INTERNAL_DECODER_XVID
    FFMFiltersKeys[FFM_XVID] = _T("FFM_XVID");
#endif
#if INTERNAL_DECODER_DIVX
    FFMFiltersKeys[FFM_DIVX] = _T("FFM_DIVX");
#endif
#if INTERNAL_DECODER_MSMPEG4
    FFMFiltersKeys[FFM_MSMPEG4] = _T("FFM_MSMPEG4");
#endif
#if INTERNAL_DECODER_WMV
    FFMFiltersKeys[FFM_WMV] = _T("FFM_WMV");
#endif
#if INTERNAL_DECODER_SVQ
    FFMFiltersKeys[FFM_SVQ3] = _T("FFM_SVQ3");
#endif
#if INTERNAL_DECODER_H263
    FFMFiltersKeys[FFM_H263] = _T("FFM_H263");
#endif
#if INTERNAL_DECODER_THEORA
    FFMFiltersKeys[FFM_THEORA] = _T("FFM_THEORA");
#endif
#if INTERNAL_DECODER_AMVV
    FFMFiltersKeys[FFM_AMVV] = _T("FFM_AMVV");
#endif
#if INTERNAL_DECODER_MJPEG
    FFMFiltersKeys[FFM_MJPEG] = _T("FFM_MJPEG");
#endif
#if INTERNAL_DECODER_INDEO
    FFMFiltersKeys[FFM_INDEO] = _T("FFM_INDEO");
#endif

}

void CAppSettings::CreateCommands()
{
#define ADDCMD(cmd) wmcmds.AddTail(wmcmd##cmd)
    ADDCMD((ID_FILE_OPENQUICK,                  'Q', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_MPLAYERC_0));
    ADDCMD((ID_FILE_OPENMEDIA,                  'O', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_OPEN_FILE));
    ADDCMD((ID_FILE_OPENDVDBD,                  'D', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_OPEN_DVD));
    ADDCMD((ID_FILE_OPENDEVICE,                 'V', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_OPEN_DEVICE));
    ADDCMD((ID_FILE_REOPEN,                     'E', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_REOPEN));

    ADDCMD((ID_FILE_SAVE_COPY,                    0, FVIRTKEY | FNOINVERT,                    IDS_AG_SAVE_AS));
    ADDCMD((ID_FILE_SAVE_IMAGE,                 'I', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_SAVE_IMAGE));
    ADDCMD((ID_FILE_SAVE_IMAGE_AUTO,          VK_F5, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_6));
    ADDCMD((ID_FILE_SAVE_THUMBNAILS,              0, FVIRTKEY | FNOINVERT,                    IDS_FILE_SAVE_THUMBNAILS));

    ADDCMD((ID_FILE_LOAD_SUBTITLE,              'L', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_LOAD_SUBTITLE));
    ADDCMD((ID_FILE_SAVE_SUBTITLE,              'S', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_SAVE_SUBTITLE));
    ADDCMD((ID_FILE_CLOSEPLAYLIST,              'C', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_CLOSE));
    ADDCMD((ID_FILE_PROPERTIES,              VK_F10, FVIRTKEY | FSHIFT | FNOINVERT,           IDS_AG_PROPERTIES));
    ADDCMD((ID_FILE_EXIT,                       'X', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_EXIT));
    ADDCMD((ID_PLAY_PLAYPAUSE,             VK_SPACE, FVIRTKEY | FNOINVERT,                    IDS_AG_PLAYPAUSE,   APPCOMMAND_MEDIA_PLAY_PAUSE, wmcmd::LDOWN, wmcmd::LDOWN));
    ADDCMD((ID_PLAY_PLAY,                         0, FVIRTKEY | FNOINVERT,                    IDS_AG_PLAY,        APPCOMMAND_MEDIA_PLAY));
    ADDCMD((ID_PLAY_PAUSE,                        0, FVIRTKEY | FNOINVERT,                    IDS_AG_PAUSE,       APPCOMMAND_MEDIA_PAUSE));
    ADDCMD((ID_PLAY_STOP,             VK_OEM_PERIOD, FVIRTKEY | FNOINVERT,                    IDS_AG_STOP,        APPCOMMAND_MEDIA_STOP));
    ADDCMD((ID_PLAY_FRAMESTEP,             VK_RIGHT, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_FRAMESTEP));
    ADDCMD((ID_PLAY_FRAMESTEPCANCEL,        VK_LEFT, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_MPLAYERC_16));
    ADDCMD((ID_PLAY_GOTO,                       'G', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_GO_TO));
    ADDCMD((ID_PLAY_INCRATE,                  VK_UP, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_INCREASE_RATE));
    ADDCMD((ID_PLAY_DECRATE,                VK_DOWN, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_DECREASE_RATE));
    ADDCMD((ID_PLAY_RESETRATE,                  'R', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_RESET_RATE));
    ADDCMD((ID_PLAY_INCAUDDELAY,             VK_ADD, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_21));
    ADDCMD((ID_PLAY_DECAUDDELAY,        VK_SUBTRACT, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_22));
    ADDCMD((ID_PLAY_SEEKFORWARDSMALL,             0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_23));
    ADDCMD((ID_PLAY_SEEKBACKWARDSMALL,            0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_24));
    ADDCMD((ID_PLAY_SEEKFORWARDMED,        VK_RIGHT, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_25));
    ADDCMD((ID_PLAY_SEEKBACKWARDMED,        VK_LEFT, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_26));
    ADDCMD((ID_PLAY_SEEKFORWARDLARGE,             0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_27));
    ADDCMD((ID_PLAY_SEEKBACKWARDLARGE,            0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_28));
    ADDCMD((ID_PLAY_SEEKKEYFORWARD,        VK_RIGHT, FVIRTKEY | FSHIFT | FNOINVERT,           IDS_MPLAYERC_29));
    ADDCMD((ID_PLAY_SEEKKEYBACKWARD,        VK_LEFT, FVIRTKEY | FSHIFT | FNOINVERT,           IDS_MPLAYERC_30));
    ADDCMD((ID_NAVIGATE_SKIPFORWARD,        VK_NEXT, FVIRTKEY | FNOINVERT,                    IDS_AG_NEXT,        APPCOMMAND_MEDIA_NEXTTRACK, wmcmd::X2DOWN, wmcmd::X2DOWN));
    ADDCMD((ID_NAVIGATE_SKIPBACK,          VK_PRIOR, FVIRTKEY | FNOINVERT,                    IDS_AG_PREVIOUS,    APPCOMMAND_MEDIA_PREVIOUSTRACK, wmcmd::X1DOWN, wmcmd::X1DOWN));
    ADDCMD((ID_NAVIGATE_SKIPFORWARDFILE,    VK_NEXT, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_NEXT_FILE));
    ADDCMD((ID_NAVIGATE_SKIPBACKFILE,      VK_PRIOR, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PREVIOUS_FILE));
    ADDCMD((ID_NAVIGATE_TUNERSCAN,              'T', FVIRTKEY | FSHIFT | FNOINVERT,           IDS_NAVIGATE_TUNERSCAN));
    ADDCMD((ID_FAVORITES_QUICKADDFAVORITE,      'Q', FVIRTKEY | FSHIFT | FNOINVERT,           IDS_FAVORITES_QUICKADDFAVORITE));
    ADDCMD((ID_VIEW_CAPTIONMENU,                '0', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_CAPTION));
    ADDCMD((ID_VIEW_SEEKER,                     '1', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_SEEKER));
    ADDCMD((ID_VIEW_CONTROLS,                   '2', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_CONTROLS));
    ADDCMD((ID_VIEW_INFORMATION,                '3', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_INFO));
    ADDCMD((ID_VIEW_STATISTICS,                 '4', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_STATS));
    ADDCMD((ID_VIEW_STATUS,                     '5', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_STATUS));
    ADDCMD((ID_VIEW_SUBRESYNC,                  '6', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_SUBRESYNC));
    ADDCMD((ID_VIEW_PLAYLIST,                   '7', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_PLAYLIST));
    ADDCMD((ID_VIEW_CAPTURE,                    '8', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_CAPTURE));
    ADDCMD((ID_VIEW_SHADEREDITOR,               '9', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_SHADER));
    ADDCMD((ID_VIEW_PRESETS_MINIMAL,            '1', FVIRTKEY | FNOINVERT,                    IDS_AG_VIEW_MINIMAL));
    ADDCMD((ID_VIEW_PRESETS_COMPACT,            '2', FVIRTKEY | FNOINVERT,                    IDS_AG_VIEW_COMPACT));
    ADDCMD((ID_VIEW_PRESETS_NORMAL,             '3', FVIRTKEY | FNOINVERT,                    IDS_AG_VIEW_NORMAL));
    ADDCMD((ID_VIEW_FULLSCREEN,           VK_RETURN, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_FULLSCREEN, 0, wmcmd::LDBLCLK, wmcmd::LDBLCLK));
    ADDCMD((ID_VIEW_FULLSCREEN_SECONDARY, VK_RETURN, FVIRTKEY | FALT | FNOINVERT,             IDS_MPLAYERC_39));
    ADDCMD((ID_VIEW_ZOOM_50,                    '1', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_ZOOM_50));
    ADDCMD((ID_VIEW_ZOOM_100,                   '2', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_ZOOM_100));
    ADDCMD((ID_VIEW_ZOOM_200,                   '3', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_ZOOM_200));
    ADDCMD((ID_VIEW_ZOOM_AUTOFIT,               '4', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_ZOOM_AUTO_FIT));
    ADDCMD((ID_VIEW_ZOOM_AUTOFIT_LARGER,        '5', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_ZOOM_AUTO_FIT_LARGER));
    ADDCMD((ID_ASPECTRATIO_NEXT,                  0, FVIRTKEY | FNOINVERT,                    IDS_AG_NEXT_AR_PRESET));
    ADDCMD((ID_VIEW_VF_HALF,                      0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_HALF));
    ADDCMD((ID_VIEW_VF_NORMAL,                    0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_NORMAL));
    ADDCMD((ID_VIEW_VF_DOUBLE,                    0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_DOUBLE));
    ADDCMD((ID_VIEW_VF_STRETCH,                   0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_STRETCH));
    ADDCMD((ID_VIEW_VF_FROMINSIDE,                0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_INSIDE));
    ADDCMD((ID_VIEW_VF_ZOOM1,                     0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_ZOOM1));
    ADDCMD((ID_VIEW_VF_ZOOM2,                     0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_ZOOM2));
    ADDCMD((ID_VIEW_VF_FROMOUTSIDE,               0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_OUTSIDE));
    ADDCMD((ID_VIEW_VF_SWITCHZOOM,              'P', FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_SWITCHZOOM));
    ADDCMD((ID_ONTOP_ALWAYS,                    'A', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_ALWAYS_ON_TOP));
    ADDCMD((ID_VIEW_RESET,               VK_NUMPAD5, FVIRTKEY | FNOINVERT,                    IDS_AG_PNS_RESET));
    ADDCMD((ID_VIEW_INCSIZE,             VK_NUMPAD9, FVIRTKEY | FNOINVERT,                    IDS_AG_PNS_INC_SIZE));
    ADDCMD((ID_VIEW_INCWIDTH,            VK_NUMPAD6, FVIRTKEY | FNOINVERT,                    IDS_AG_PNS_INC_WIDTH));
    ADDCMD((ID_VIEW_INCHEIGHT,           VK_NUMPAD8, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_47));
    ADDCMD((ID_VIEW_DECSIZE,             VK_NUMPAD1, FVIRTKEY | FNOINVERT,                    IDS_AG_PNS_DEC_SIZE));
    ADDCMD((ID_VIEW_DECWIDTH,            VK_NUMPAD4, FVIRTKEY | FNOINVERT,                    IDS_AG_PNS_DEC_WIDTH));
    ADDCMD((ID_VIEW_DECHEIGHT,           VK_NUMPAD2, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_50));
    ADDCMD((ID_PANSCAN_CENTER,           VK_NUMPAD5, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_CENTER));
    ADDCMD((ID_PANSCAN_MOVELEFT,         VK_NUMPAD4, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_LEFT));
    ADDCMD((ID_PANSCAN_MOVERIGHT,        VK_NUMPAD6, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_RIGHT));
    ADDCMD((ID_PANSCAN_MOVEUP,           VK_NUMPAD8, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_UP));
    ADDCMD((ID_PANSCAN_MOVEDOWN,         VK_NUMPAD2, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_DOWN));
    ADDCMD((ID_PANSCAN_MOVEUPLEFT,       VK_NUMPAD7, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_UPLEFT));
    ADDCMD((ID_PANSCAN_MOVEUPRIGHT,      VK_NUMPAD9, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_UPRIGHT));
    ADDCMD((ID_PANSCAN_MOVEDOWNLEFT,     VK_NUMPAD1, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_DOWNLEFT));
    ADDCMD((ID_PANSCAN_MOVEDOWNRIGHT,    VK_NUMPAD3, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_MPLAYERC_59));
    ADDCMD((ID_PANSCAN_ROTATEXP,         VK_NUMPAD8, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_PNS_ROTATEX_P));
    ADDCMD((ID_PANSCAN_ROTATEXM,         VK_NUMPAD2, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_PNS_ROTATEX_M));
    ADDCMD((ID_PANSCAN_ROTATEYP,         VK_NUMPAD4, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_PNS_ROTATEY_P));
    ADDCMD((ID_PANSCAN_ROTATEYM,         VK_NUMPAD6, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_PNS_ROTATEY_M));
    ADDCMD((ID_PANSCAN_ROTATEZP,         VK_NUMPAD1, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_PNS_ROTATEZ_P));
    ADDCMD((ID_PANSCAN_ROTATEZM,         VK_NUMPAD3, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_PNS_ROTATEZ_M));
    ADDCMD((ID_VOLUME_UP,                     VK_UP, FVIRTKEY | FNOINVERT,                    IDS_AG_VOLUME_UP,   0, wmcmd::WUP, wmcmd::WUP));
    ADDCMD((ID_VOLUME_DOWN,                 VK_DOWN, FVIRTKEY | FNOINVERT,                    IDS_AG_VOLUME_DOWN, 0, wmcmd::WDOWN, wmcmd::WDOWN));
    ADDCMD((ID_VOLUME_MUTE,                     'M', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_VOLUME_MUTE, 0));
    ADDCMD((ID_VOLUME_BOOST_INC,                  0, FVIRTKEY | FNOINVERT,                    IDS_VOLUME_BOOST_INC));
    ADDCMD((ID_VOLUME_BOOST_DEC,                  0, FVIRTKEY | FNOINVERT,                    IDS_VOLUME_BOOST_DEC));
    ADDCMD((ID_VOLUME_BOOST_MIN,                  0, FVIRTKEY | FNOINVERT,                    IDS_VOLUME_BOOST_MIN));
    ADDCMD((ID_VOLUME_BOOST_MAX,                  0, FVIRTKEY | FNOINVERT,                    IDS_VOLUME_BOOST_MAX));
    ADDCMD((ID_CUSTOM_CHANNEL_MAPPING,            0, FVIRTKEY | FNOINVERT,                    IDS_CUSTOM_CHANNEL_MAPPING));
    ADDCMD((ID_NORMALIZE,                         0, FVIRTKEY | FNOINVERT,                    IDS_NORMALIZE));
    ADDCMD((ID_REGAIN_VOLUME,                     0, FVIRTKEY | FNOINVERT,                    IDS_REGAIN_VOLUME));
    ADDCMD((ID_COLOR_BRIGHTNESS_INC,              0, FVIRTKEY | FNOINVERT,                    IDS_BRIGHTNESS_INC));
    ADDCMD((ID_COLOR_BRIGHTNESS_DEC,              0, FVIRTKEY | FNOINVERT,                    IDS_BRIGHTNESS_DEC));
    ADDCMD((ID_COLOR_CONTRAST_INC,                0, FVIRTKEY | FNOINVERT,                    IDS_CONTRAST_INC));
    ADDCMD((ID_COLOR_CONTRAST_DEC,                0, FVIRTKEY | FNOINVERT,                    IDS_CONTRAST_DEC));
    ADDCMD((ID_COLOR_SATURATION_INC,              0, FVIRTKEY | FNOINVERT,                    IDS_SATURATION_INC));
    ADDCMD((ID_COLOR_SATURATION_DEC,              0, FVIRTKEY | FNOINVERT,                    IDS_SATURATION_DEC));
    ADDCMD((ID_COLOR_RESET,                       0, FVIRTKEY | FNOINVERT,                    IDS_RESET_COLOR));
    ADDCMD((ID_NAVIGATE_TITLEMENU,              'T', FVIRTKEY | FALT | FNOINVERT,             IDS_MPLAYERC_63));
    ADDCMD((ID_NAVIGATE_ROOTMENU,               'R', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_DVD_ROOT_MENU));
    ADDCMD((ID_NAVIGATE_SUBPICTUREMENU,           0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_65));
    ADDCMD((ID_NAVIGATE_AUDIOMENU,                0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_66));
    ADDCMD((ID_NAVIGATE_ANGLEMENU,                0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_67));
    ADDCMD((ID_NAVIGATE_CHAPTERMENU,              0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_68));
    ADDCMD((ID_NAVIGATE_MENU_LEFT,          VK_LEFT, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_DVD_MENU_LEFT));
    ADDCMD((ID_NAVIGATE_MENU_RIGHT,        VK_RIGHT, FVIRTKEY | FALT | FNOINVERT,             IDS_MPLAYERC_70));
    ADDCMD((ID_NAVIGATE_MENU_UP,              VK_UP, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_DVD_MENU_UP));
    ADDCMD((ID_NAVIGATE_MENU_DOWN,          VK_DOWN, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_DVD_MENU_DOWN));
    ADDCMD((ID_NAVIGATE_MENU_ACTIVATE,            0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_73));
    ADDCMD((ID_NAVIGATE_MENU_BACK,                0, FVIRTKEY | FNOINVERT,                    IDS_AG_DVD_MENU_BACK));
    ADDCMD((ID_NAVIGATE_MENU_LEAVE,               0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_75));
    ADDCMD((ID_BOSS,                            'B', FVIRTKEY | FNOINVERT,                    IDS_AG_BOSS_KEY));
    ADDCMD((ID_MENU_PLAYER_SHORT,           VK_APPS, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_77, 0, wmcmd::RUP, wmcmd::RUP));
    ADDCMD((ID_MENU_PLAYER_LONG,                  0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_78));
    ADDCMD((ID_MENU_FILTERS,                      0, FVIRTKEY | FNOINVERT,                    IDS_AG_FILTERS_MENU));
    ADDCMD((ID_VIEW_OPTIONS,                    'O', FVIRTKEY | FNOINVERT,                    IDS_AG_OPTIONS));
    ADDCMD((ID_STREAM_AUDIO_NEXT,               'A', FVIRTKEY | FNOINVERT,                    IDS_AG_NEXT_AUDIO));
    ADDCMD((ID_STREAM_AUDIO_PREV,               'A', FVIRTKEY | FSHIFT | FNOINVERT,           IDS_AG_PREV_AUDIO));
    ADDCMD((ID_STREAM_SUB_NEXT,                 'S', FVIRTKEY | FNOINVERT,                    IDS_AG_NEXT_SUBTITLE));
    ADDCMD((ID_STREAM_SUB_PREV,                 'S', FVIRTKEY | FSHIFT | FNOINVERT,           IDS_AG_PREV_SUBTITLE));
    ADDCMD((ID_STREAM_SUB_ONOFF,                'W', FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_85));
    ADDCMD((ID_SUBTITLES_SUBITEM_START + 2,       0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_86));
    ADDCMD((ID_FILE_ISDB_DOWNLOAD,              'D', FVIRTKEY | FNOINVERT,                    IDS_DOWNLOAD_SUBS));
    ADDCMD((ID_OGM_AUDIO_NEXT,                    0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_87));
    ADDCMD((ID_OGM_AUDIO_PREV,                    0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_88));
    ADDCMD((ID_OGM_SUB_NEXT,                      0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_89));
    ADDCMD((ID_OGM_SUB_PREV,                      0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_90));
    ADDCMD((ID_DVD_ANGLE_NEXT,                    0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_91));
    ADDCMD((ID_DVD_ANGLE_PREV,                    0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_92));
    ADDCMD((ID_DVD_AUDIO_NEXT,                    0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_93));
    ADDCMD((ID_DVD_AUDIO_PREV,                    0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_94));
    ADDCMD((ID_DVD_SUB_NEXT,                      0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_95));
    ADDCMD((ID_DVD_SUB_PREV,                      0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_96));
    ADDCMD((ID_DVD_SUB_ONOFF,                     0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_97));
    ADDCMD((ID_VIEW_TEARING_TEST,               'T', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TEARING_TEST));
    ADDCMD((ID_VIEW_REMAINING_TIME,             'I', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_MPLAYERC_98));
    ADDCMD((ID_SHADERS_TOGGLE,                  'P', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AT_TOGGLE_SHADER));
    ADDCMD((ID_SHADERS_TOGGLE_SCREENSPACE,      'P', FVIRTKEY | FCONTROL | FALT | FNOINVERT,  IDS_AT_TOGGLE_SHADERSCREENSPACE));
    ADDCMD((ID_D3DFULLSCREEN_TOGGLE,            'F', FVIRTKEY | FCONTROL | FALT | FNOINVERT,  IDS_MPLAYERC_99));
    ADDCMD((ID_GOTO_PREV_SUB,                   'Y', FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_100,  APPCOMMAND_BROWSER_BACKWARD));
    ADDCMD((ID_GOTO_NEXT_SUB,                   'U', FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_101,  APPCOMMAND_BROWSER_FORWARD));
    ADDCMD((ID_SHIFT_SUB_DOWN,              VK_NEXT, FVIRTKEY | FALT | FNOINVERT,             IDS_MPLAYERC_102));
    ADDCMD((ID_SHIFT_SUB_UP,               VK_PRIOR, FVIRTKEY | FALT | FNOINVERT,             IDS_MPLAYERC_103));
    ADDCMD((ID_VIEW_DISPLAYSTATS,               'J', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_DISPLAY_STATS));
    ADDCMD((ID_VIEW_RESETSTATS,                 'R', FVIRTKEY | FCONTROL | FALT | FNOINVERT,  IDS_AG_RESET_STATS));
    ADDCMD((ID_VIEW_VSYNC,                      'V', FVIRTKEY | FNOINVERT,                    IDS_AG_VSYNC));
    ADDCMD((ID_VIEW_ENABLEFRAMETIMECORRECTION,    0, FVIRTKEY | FNOINVERT,                    IDS_AG_ENABLEFRAMETIMECORRECTION));
    ADDCMD((ID_VIEW_VSYNCACCURATE,              'V', FVIRTKEY | FCONTROL | FALT | FNOINVERT,  IDS_AG_VSYNCACCURATE));
    ADDCMD((ID_VIEW_VSYNCOFFSET_DECREASE,     VK_UP, FVIRTKEY | FCONTROL | FALT | FNOINVERT,  IDS_AG_VSYNCOFFSET_DECREASE));
    ADDCMD((ID_VIEW_VSYNCOFFSET_INCREASE,   VK_DOWN, FVIRTKEY | FCONTROL | FALT | FNOINVERT,  IDS_AG_VSYNCOFFSET_INCREASE));
    ADDCMD((ID_SUB_DELAY_DOWN,                VK_F1, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_104));
    ADDCMD((ID_SUB_DELAY_UP,                  VK_F2, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_105));

    ADDCMD((ID_AFTERPLAYBACK_CLOSE,               0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_CLOSE));
    ADDCMD((ID_AFTERPLAYBACK_STANDBY,             0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_STANDBY));
    ADDCMD((ID_AFTERPLAYBACK_HIBERNATE,           0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_HIBERNATE));
    ADDCMD((ID_AFTERPLAYBACK_SHUTDOWN,            0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_SHUTDOWN));
    ADDCMD((ID_AFTERPLAYBACK_LOGOFF,              0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_LOGOFF));
    ADDCMD((ID_AFTERPLAYBACK_LOCK,                0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_LOCK));
    ADDCMD((ID_AFTERPLAYBACK_EXIT,                0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_EXIT));
    ADDCMD((ID_AFTERPLAYBACK_DONOTHING,           0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_DONOTHING));
    ADDCMD((ID_AFTERPLAYBACK_NEXT,                0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_NEXT));

    ADDCMD((ID_VIEW_EDITLISTEDITOR,               0, FVIRTKEY | FNOINVERT,                    IDS_AG_TOGGLE_EDITLISTEDITOR));
    ADDCMD((ID_EDL_IN,                            0, FVIRTKEY | FNOINVERT,                    IDS_AG_EDL_IN));
    ADDCMD((ID_EDL_OUT,                           0, FVIRTKEY | FNOINVERT,                    IDS_AG_EDL_OUT));
    ADDCMD((ID_EDL_NEWCLIP,                       0, FVIRTKEY | FNOINVERT,                    IDS_AG_EDL_NEW_CLIP));
    ADDCMD((ID_EDL_SAVE,                          0, FVIRTKEY | FNOINVERT,                    IDS_AG_EDL_SAVE));

#undef ADDCMD
}

CAppSettings::~CAppSettings()
{
    if (hAccel) {
        DestroyAcceleratorTable(hAccel);
    }
}

bool CAppSettings::IsD3DFullscreen() const
{
    if (nCLSwitches & CLSW_D3DFULLSCREEN) {
        return true;
    } else if (iDSVideoRendererType == VIDRNDT_DS_VMR9RENDERLESS ||
               iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM ||
               iDSVideoRendererType == VIDRNDT_DS_MADVR ||
               iDSVideoRendererType == VIDRNDT_DS_SYNC) {
        return fD3DFullscreen;
    } else {
        return false;
    }
}

CString CAppSettings::SelectedAudioRenderer() const
{
    CString strResult;
    if (!AfxGetMyApp()->m_AudioRendererDisplayName_CL.IsEmpty()) {
        strResult = AfxGetMyApp()->m_AudioRendererDisplayName_CL;
    } else {
        strResult = AfxGetAppSettings().strAudioRendererDisplayName;
    }

    return strResult;
}

void CAppSettings::SaveSettings()
{
    CMPlayerCApp* pApp = AfxGetMyApp();
    ASSERT(pApp);

    if (!fInitialized) {
        return;
    }

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECAPTIONMENU, iCaptionMenuMode);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDENAVIGATION, fHideNavigation);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CONTROLSTATE, nCS);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULTVIDEOFRAME, iDefaultVideoSize);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPASPECTRATIO, fKeepAspectRatio);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COMPMONDESKARDIFF, fCompMonDeskARDiff);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUME, nVolume);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_BALANCE, nBalance);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUMESTEP, nVolumeStep);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPEEDSTEP, nSpeedStep);
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
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOFITFACTOR, nAutoFitFactor);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLS, fShowBarsWhenFullScreen);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLSTIMEOUT, nShowBarsWhenFullScreenTimeOut);
    pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_FULLSCREENRES, (BYTE*)&AutoChangeFullscrRes, sizeof(AutoChangeFullscrRes));
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_EXITFULLSCREENATTHEEND, fExitFullScreenAtTheEnd);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_RESTORERESAFTEREXIT, fRestoreResAfterExit);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWPOS, fRememberWindowPos);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWSIZE, fRememberWindowSize);
    if (fSavePnSZoom) {
        CString str;
        str.Format(_T("%.3f,%.3f"), dZoomX, dZoomY);
        pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_PANSCANZOOM, str);
    } else {
        pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_PANSCANZOOM, nullptr);
    }
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SNAPTODESKTOPEDGES, fSnapToDesktopEdges);
    pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_LASTWINDOWRECT, (BYTE*)&rcLastWindowPos, sizeof(rcLastWindowPos));
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTWINDOWTYPE, nLastWindowType);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_X, sizeAspectRatio.cx);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_Y, sizeAspectRatio.cy);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPHISTORY, fKeepHistory);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE, iDSVideoRendererType);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_RMVIDEORENDERERTYPE, iRMVideoRendererType);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_QTVIDEORENDERERTYPE, iQTVideoRendererType);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SHUFFLEPLAYLISTITEMS, bShufflePlaylistItems);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERPLAYLISTITEMS, bRememberPlaylistItems);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDEPLAYLISTFULLSCREEN, bHidePlaylistFullScreen);
    pApp->WriteProfileInt(IDS_R_FAVORITES, IDS_RS_FAV_REMEMBERPOS, bFavRememberPos);
    pApp->WriteProfileInt(IDS_R_FAVORITES, IDS_RS_FAV_RELATIVEDRIVE, bFavRelativeDrive);

    UpdateRenderersData(true);

    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_AUDIORENDERERTYPE, CString(strAudioRendererDisplayName));
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADAUDIO, fAutoloadAudio);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADSUBTITLES, fAutoloadSubtitles);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANGORDER, CString(strSubtitlesLanguageOrder));
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOSLANGORDER, CString(strAudiosLanguageOrder));
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_BLOCKVSFILTER, fBlockVSFilter);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWORKERTHREADFOROPENING, fEnableWorkerThreadForOpening);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REPORTFAILEDPINS, fReportFailedPins);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_DVDPATH, strDVDPath);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USEDVDPATH, fUseDVDPath);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MENULANG, idMenuLang);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOLANG, idAudioLang);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANG, idSubtitlesLang);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOSPEAKERCONF, fAutoSpeakerConf);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CLOSEDCAPTIONS, fClosedCaptions);
    CString style;
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SPLOGFONT, style <<= subdefstyle);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPOVERRIDEPLACEMENT, fOverridePlacement);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPHORPOS, nHorPos);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPVERPOS, nVerPos);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBDELAYINTERVAL, nSubDelayInterval);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLESUBTITLES, fEnableSubtitles);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PREFER_FORCED_DEFAULT_SUBTITLES, bPreferDefaultForcedSubtitles);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITIZEEXTERNALSUBTITLES, fPrioritizeExternalSubtitles);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEINTERNALSUBTITLES, fDisableInternalSubtitles);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ALLOW_OVERRIDING_EXT_SPLITTER, bAllowOverridingExternalSplitterChoice);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLEPATHS, strSubtitlePaths);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USEDEFAULTSUBTITLESSTYLE, fUseDefaultSubtitlesStyle);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOSWITCHER, fEnableAudioSwitcher);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOTIMESHIFT, fAudioTimeShift);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOTIMESHIFT, iAudioTimeShift);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DOWNSAMPLETO441, fDownSampleTo441);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CUSTOMCHANNELMAPPING, fCustomChannelMapping);
    pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_SPEAKERTOCHANNELMAPPING, (BYTE*)pSpeakerToChannelMap, sizeof(pSpeakerToChannelMap));
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZE, fAudioNormalize);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOMAXNORMFACTOR, nAudioMaxNormFactor);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZERECOVER, fAudioNormalizeRecover);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, nAudioBoost);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPEAKERCHANNELS, nSpeakerChannels);

    // Multi-monitor code
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_FULLSCREENMONITOR, CString(strFullScreenMonitor));
    // Prevent Minimize when in Fullscreen mode on non default monitor
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_PREVENT_MINIMIZE, fPreventMinimize);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_WIN7TASKBAR, fUseWin7TaskBar);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_EXIT_AFTER_PB, fExitAfterPlayback);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NEXT_AFTER_PB, fNextInDirAfterPlayback);
    // TODO: Change IDS_RS_MPC_NO_SEARCH_IN_FOLDER into IDS_RS_SEARCH_IN_FOLDER
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NO_SEARCH_IN_FOLDER, !fUseSearchInFolder);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USE_TIME_TOOLTIP, fUseTimeTooltip);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TIME_TOOLTIP_POSITION, nTimeTooltipPosition);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_OSD_SIZE, nOSDSize);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_MPC_OSD_FONT, strOSDFont);

    // Associated types with icon or not...
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASSOCIATED_WITH_ICON, fAssociatedWithIcons);
    // Last Open Dir
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_LAST_OPEN_DIR, strLastOpenDir);

    // CASIMIR666 : new settings
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_D3DFULLSCREEN, fD3DFullscreen);
    //pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MONITOR_AUTOREFRESHRATE, fMonitorAutoRefreshRate);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_BRIGHTNESS, iBrightness);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_CONTRAST, iContrast);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_HUE, iHue);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_SATURATION, iSaturation);

    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLIST, strShaderList);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLISTSCREENSPACE, strShaderListScreenSpace);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADER, (int)fToggleShader);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADERSSCREENSPACE, (int)fToggleShaderScreenSpace);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SHOWOSD, (int)fShowOSD);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEEDLEDITOR, (int)fEnableEDLEditor);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LANGUAGE, language);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FASTSEEK_KEYFRAME, (int)fFastSeek);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SHOW_CHAPTERS, (int)fShowChapters);


    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LCD_SUPPORT, (int)fLCDSupport);

    // Save analog capture settings
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULT_CAPTURE, iDefaultCaptureDevice);
    pApp->WriteProfileString(IDS_R_CAPTURE, IDS_RS_VIDEO_DISP_NAME, strAnalogVideo);
    pApp->WriteProfileString(IDS_R_CAPTURE, IDS_RS_AUDIO_DISP_NAME, strAnalogAudio);
    pApp->WriteProfileInt(IDS_R_CAPTURE, IDS_RS_COUNTRY,         iAnalogCountry);

    // Save digital capture settings (BDA)
    pApp->WriteProfileString(IDS_R_DVB, nullptr, nullptr); // Ensure the section is cleared before saving the new settings

    pApp->WriteProfileString(IDS_R_DVB, IDS_RS_BDA_NETWORKPROVIDER, strBDANetworkProvider);
    pApp->WriteProfileString(IDS_R_DVB, IDS_RS_BDA_TUNER, strBDATuner);
    pApp->WriteProfileString(IDS_R_DVB, IDS_RS_BDA_RECEIVER, strBDAReceiver);
    //pApp->WriteProfileString(IDS_R_DVB, IDS_RS_BDA_STANDARD, strBDAStandard);
    pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_BDA_SCAN_FREQ_START, iBDAScanFreqStart);
    pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_BDA_SCAN_FREQ_END, iBDAScanFreqEnd);
    pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_BDA_BANDWIDTH, iBDABandwidth);
    pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_BDA_USE_OFFSET, fBDAUseOffset);
    pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_BDA_OFFSET, iBDAOffset);
    pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_BDA_IGNORE_ENCRYPTED_CHANNELS, fBDAIgnoreEncryptedChannels);
    pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_DVB_LAST_CHANNEL, nDVBLastChannel);

    int iChannel = 0;
    POSITION pos = m_DVBChannels.GetHeadPosition();
    while (pos) {
        CString strTemp2;
        CString strChannel;
        CDVBChannel& Channel = m_DVBChannels.GetNext(pos);
        strTemp2.Format(_T("%d"), iChannel);
        pApp->WriteProfileString(IDS_R_DVB, strTemp2, Channel.ToString());
        iChannel++;
    }

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DVDPOS, (int)fRememberDVDPos);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOS, (int)fRememberFilePos);
    if (fKeepHistory) {
        if (fRememberFilePos) {
            filePositions.Save();
        }
        if (fRememberDVDPos) {
            dvdPositions.Save();
        }
    }

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTFULLSCREEN, (int)fLastFullScreen);
    // CASIMIR666 : end of new settings

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_INTREALMEDIA, fIntRealMedia);
    //pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIARENDERLESS, fRealMediaRenderless);
    //pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_QUICKTIMERENDERER, iQuickTimeRenderer);
    //pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIAFPS, *((DWORD*)&dRealMediaQuickTimeFPS));

    pApp->WriteProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, nullptr, nullptr);
    for (INT_PTR i = 0, j = m_pnspresets.GetCount(); i < j; i++) {
        CString str;
        str.Format(_T("Preset%Id"), i);
        pApp->WriteProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, str, m_pnspresets[i]);
    }

    pApp->WriteProfileString(IDS_R_COMMANDS, nullptr, nullptr);
    pos = wmcmds.GetHeadPosition();
    for (int i = 0; pos;) {
        wmcmd& wc = wmcmds.GetNext(pos);
        if (wc.IsModified()) {
            CString str;
            str.Format(_T("CommandMod%d"), i);
            CString str2;
            str2.Format(_T("%d %x %x %s %d %u %u %u"),
                        wc.cmd, wc.fVirt, wc.key,
                        _T("\"") + CString(wc.rmcmd) +  _T("\""), wc.rmrepcnt,
                        wc.mouse, wc.appcmd, wc.mouseFS);
            pApp->WriteProfileString(IDS_R_COMMANDS, str, str2);
            i++;
        }
    }

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WINLIRC, fWinLirc);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WINLIRCADDR, strWinLircAddr);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_UICE, fUIce);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_UICEADDR, strUIceAddr);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_GLOBALMEDIA, fGlobalMedia);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTS, nJumpDistS);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTM, nJumpDistM);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTL, nJumpDistL);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LIMITWINDOWPROPORTIONS, fLimitWindowProportions);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTUSEDPAGE, nLastUsedPage);

    m_Formats.UpdateData(true);

    // Internal filters
    for (int f = 0; f < SRC_LAST; f++) {
        pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, SrcFiltersKeys[f], SrcFilters[f]);
    }
    for (int f = 0; f < TRA_LAST; f++) {
        pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, TraFiltersKeys[f], TraFilters[f]);
    }
    for (int f = 0; f < TRA_DXVA_LAST; f++) {
        pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, DXVAFiltersKeys[f], DXVAFilters[f]);
    }
    for (int f = 0; f < FFM_LAST; f++) {
        pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, FFMFiltersKeys[f], FFmpegFilters[f]);
    }

    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_LOGOFILE, strLogoFileName);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOID, nLogoId);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOEXT, fLogoExternal);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECDROMSSUBMENU, fHideCDROMsSubMenu);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITY, dwPriority);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LAUNCHFULLSCREEN, fLaunchfullscreen);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWEBSERVER, fEnableWebServer);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPORT, nWebServerPort);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPRINTDEBUGINFO, fWebServerPrintDebugInfo);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERUSECOMPRESSION, fWebServerUseCompression);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERLOCALHOSTONLY, fWebServerLocalhostOnly);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBROOT, strWebRoot);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBDEFINDEX, strWebDefIndex);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBSERVERCGI, strWebServerCGI);

    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTPATH, strSnapShotPath);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTEXT, strSnapShotExt);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBROWS, iThumbRows);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBCOLS, iThumbCols);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBWIDTH, iThumbWidth);

    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_ISDB, strISDb);

    if (fShaderEditorWasOpened) { // This is a large data block. Save it only when really necessary.
        // Erase the currently saved shaders
        pApp->WriteProfileString(IDS_R_SHADERS, nullptr, nullptr);
        pApp->WriteProfileInt(IDS_R_SHADERS, IDS_RS_SHADERS_INITIALIZED, 1);

        pos = m_shaders.GetHeadPosition();
        for (int i = 0; pos; i++) {
            const Shader& s = m_shaders.GetNext(pos);

            if (!s.label.IsEmpty()) {
                CString index;
                index.Format(_T("%d"), i);
                CString srcdata = s.srcdata;
                srcdata.Replace(_T("\r"), _T(""));
                srcdata.Replace(_T("\n"), _T("\\n"));
                srcdata.Replace(_T("\t"), _T("\\t"));
                pApp->WriteProfileString(IDS_R_SHADERS, index, s.label + _T("|") + s.target + _T("|") + srcdata);
            }
        }

        fShaderEditorWasOpened = false;
    }

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMAINING_TIME, fRemainingTime);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_UPDATER_AUTO_CHECK, nUpdaterAutoCheck);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_UPDATER_DELAY, nUpdaterDelay);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_NOTIFY_SKYPE, bNotifySkype);

    pApp->FlushProfile();
}

void CAppSettings::LoadExternalFilters(CAutoPtrList<FilterOverride>& filters, LPCTSTR baseKey /*= IDS_R_EXTERNAL_FILTERS*/)
{
    CWinApp* pApp = AfxGetApp();
    ASSERT(pApp);

    for (unsigned int i = 0; ; i++) {
        CString key;
        key.Format(_T("%s\\%04u"), baseKey, i);

        CAutoPtr<FilterOverride> f(DEBUG_NEW FilterOverride);

        f->fDisabled = !pApp->GetProfileInt(key, _T("Enabled"), FALSE);

        UINT j = pApp->GetProfileInt(key, _T("SourceType"), -1);
        if (j == 0) {
            f->type = FilterOverride::REGISTERED;
            f->dispname = CStringW(pApp->GetProfileString(key, _T("DisplayName"), _T("")));
            f->name = pApp->GetProfileString(key, _T("Name"), _T(""));
        } else if (j == 1) {
            f->type = FilterOverride::EXTERNAL;
            f->path = pApp->GetProfileString(key, _T("Path"), _T(""));
            f->name = pApp->GetProfileString(key, _T("Name"), _T(""));
            f->clsid = GUIDFromCString(pApp->GetProfileString(key, _T("CLSID"), _T("")));
        } else {
            pApp->WriteProfileString(key, nullptr, 0);
            break;
        }

        f->backup.RemoveAll();
        for (unsigned int k = 0; ; k++) {
            CString val;
            val.Format(_T("org%04u"), k);
            CString guid = pApp->GetProfileString(key, val, _T(""));
            if (guid.IsEmpty()) {
                break;
            }
            f->backup.AddTail(GUIDFromCString(guid));
        }

        f->guids.RemoveAll();
        for (unsigned int k = 0; ; k++) {
            CString val;
            val.Format(_T("mod%04u"), k);
            CString guid = pApp->GetProfileString(key, val, _T(""));
            if (guid.IsEmpty()) {
                break;
            }
            f->guids.AddTail(GUIDFromCString(guid));
        }

        f->iLoadType = (int)pApp->GetProfileInt(key, _T("LoadType"), -1);
        if (f->iLoadType < 0) {
            break;
        }

        f->dwMerit = pApp->GetProfileInt(key, _T("Merit"), MERIT_DO_NOT_USE + 1);

        filters.AddTail(f);
    }
}

void CAppSettings::ConvertOldExternalFiltersList()
{
    CAutoPtrList<FilterOverride> filters, succeededFilters, failedFilters;
    // Load the old filters list
    LoadExternalFilters(filters, IDS_R_FILTERS);
    if (!filters.IsEmpty()) {
        POSITION pos = filters.GetHeadPosition();
        while (pos) {
            CAutoPtr<FilterOverride>& fo = filters.GetNext(pos);

            CAutoPtr<CFGFilter> pFGF;
            if (fo->type == FilterOverride::REGISTERED) {
                pFGF.Attach(DEBUG_NEW CFGFilterRegistry(fo->dispname));
            } else if (fo->type == FilterOverride::EXTERNAL) {
                pFGF.Attach(DEBUG_NEW CFGFilterFile(fo->clsid, fo->path, CStringW(fo->name)));
            }
            if (!pFGF) {
                continue;
            }

            CComPtr<IBaseFilter> pBF;
            CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
            if (SUCCEEDED(pFGF->Create(&pBF, pUnks))) {
                succeededFilters.AddTail(fo);
            } else {
                failedFilters.AddTail(fo);
            }
        }
        // Clear the old filters list
        filters.RemoveAll();
        SaveExternalFilters(filters, IDS_R_FILTERS);
        // Save the new filters lists
#ifndef _WIN64
        SaveExternalFilters(succeededFilters, IDS_R_EXTERNAL_FILTERS_x86);
        SaveExternalFilters(failedFilters, IDS_R_EXTERNAL_FILTERS_x64);
#else
        SaveExternalFilters(succeededFilters, IDS_R_EXTERNAL_FILTERS_x64);
        SaveExternalFilters(failedFilters, IDS_R_EXTERNAL_FILTERS_x86);
#endif
    }
}

void CAppSettings::SaveExternalFilters(CAutoPtrList<FilterOverride>& filters, LPCTSTR baseKey /*= IDS_R_EXTERNAL_FILTERS*/)
{
    // Saving External Filter settings takes a long time. Use only when really necessary.
    CWinApp* pApp = AfxGetApp();
    ASSERT(pApp);

    // Remove the old keys
    for (unsigned int i = 0; ; i++) {
        CString key;
        key.Format(_T("%s\\%04u"), baseKey, i);
        int j = pApp->GetProfileInt(key, _T("Enabled"), -1);
        pApp->WriteProfileString(key, nullptr, nullptr);
        if (j < 0) {
            break;
        }
    }

    unsigned int k = 0;
    POSITION pos = filters.GetHeadPosition();
    while (pos) {
        FilterOverride* f = filters.GetNext(pos);

        if (f->fTemporary) {
            continue;
        }

        CString key;
        key.Format(_T("%s\\%04u"), baseKey, k);

        pApp->WriteProfileInt(key, _T("SourceType"), (int)f->type);
        pApp->WriteProfileInt(key, _T("Enabled"), (int)!f->fDisabled);
        if (f->type == FilterOverride::REGISTERED) {
            pApp->WriteProfileString(key, _T("DisplayName"), CString(f->dispname));
            pApp->WriteProfileString(key, _T("Name"), f->name);
        } else if (f->type == FilterOverride::EXTERNAL) {
            pApp->WriteProfileString(key, _T("Path"), f->path);
            pApp->WriteProfileString(key, _T("Name"), f->name);
            pApp->WriteProfileString(key, _T("CLSID"), CStringFromGUID(f->clsid));
        }
        POSITION pos2 = f->backup.GetHeadPosition();
        for (unsigned int i = 0; pos2; i++) {
            CString val;
            val.Format(_T("org%04u"), i);
            pApp->WriteProfileString(key, val, CStringFromGUID(f->backup.GetNext(pos2)));
        }
        pos2 = f->guids.GetHeadPosition();
        for (unsigned int i = 0; pos2; i++) {
            CString val;
            val.Format(_T("mod%04u"), i);
            pApp->WriteProfileString(key, val, CStringFromGUID(f->guids.GetNext(pos2)));
        }
        pApp->WriteProfileInt(key, _T("LoadType"), f->iLoadType);
        pApp->WriteProfileInt(key, _T("Merit"), f->dwMerit);

        k++;
    }
}

void CAppSettings::LoadSettings()
{
    CWinApp* pApp = AfxGetApp();
    ASSERT(pApp);

    UINT  len;
    BYTE* ptr = nullptr;

    if (fInitialized) {
        return;
    }

    // Set interface language first!
    language = (LANGID)pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LANGUAGE, -1);
    if (language == (LANGID) - 1) {
        CMPlayerCApp::SetDefaultLanguage();
    } else if (language != 0) {
        if (language <= 23) { // Here for compatibility with old settings
            CMPlayerCApp::SetLanguage(CMPlayerCApp::GetLanguageResourceByResourceID(language + ID_LANGUAGE_ENGLISH));
        } else {
            CMPlayerCApp::SetLanguage(CMPlayerCApp::GetLanguageResourceByLocaleID(language));
        }
    }

    CreateCommands();

    iCaptionMenuMode = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECAPTIONMENU, MODE_SHOWCAPTIONMENU);
    fHideNavigation = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDENAVIGATION, FALSE);
    nCS = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CONTROLSTATE, CS_SEEKBAR | CS_TOOLBAR | CS_STATUSBAR);
    iDefaultVideoSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULTVIDEOFRAME, DVS_FROMINSIDE);
    fKeepAspectRatio = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPASPECTRATIO, TRUE);
    fCompMonDeskARDiff = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COMPMONDESKARDIFF, FALSE);
    nVolume = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUME, 100);
    nBalance = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_BALANCE, 0);
    fMute = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MUTE, FALSE);
    nLoops = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOOPNUM, 1);
    fLoopForever = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOOP, FALSE);
    fRewind = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REWIND, FALSE);
    iZoomLevel = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ZOOM, 1);
    iDSVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE, (SysVersion::IsVistaOrLater() ? (HasEVR() ? VIDRNDT_DS_EVR_CUSTOM : VIDRNDT_DS_DEFAULT) : VIDRNDT_DS_VMR7WINDOWED));
    iRMVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_RMVIDEORENDERERTYPE, VIDRNDT_RM_DEFAULT);
    iQTVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_QTVIDEORENDERERTYPE, VIDRNDT_QT_DEFAULT);
    nVolumeStep = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUMESTEP, 5);
    nSpeedStep = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPEEDSTEP, 0);

    UpdateRenderersData(false);

    strAudioRendererDisplayName = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIORENDERERTYPE, _T(""));
    fAutoloadAudio = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADAUDIO, TRUE);
    fAutoloadSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADSUBTITLES, !IsVSFilterInstalled() || (SysVersion::IsVistaOrLater() && HasEVR()));
    strSubtitlesLanguageOrder = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANGORDER, _T(""));
    strAudiosLanguageOrder = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOSLANGORDER, _T(""));
    fBlockVSFilter = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_BLOCKVSFILTER, TRUE);
    fEnableWorkerThreadForOpening = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWORKERTHREADFOROPENING, TRUE);
    fReportFailedPins = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REPORTFAILEDPINS, TRUE);
    fAllowMultipleInst = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MULTIINST, FALSE);
    iTitleBarTextStyle = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTSTYLE, 1);
    fTitleBarTextTitle = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTTITLE, FALSE);
    iOnTop = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ONTOP, 0);
    fTrayIcon = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TRAYICON, FALSE);
    fRememberZoomLevel = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOZOOM, TRUE);
    nAutoFitFactor = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOFITFACTOR, 75);
    fShowBarsWhenFullScreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLS, TRUE);
    nShowBarsWhenFullScreenTimeOut = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLSTIMEOUT, 0);

    //Multi-monitor code
    strFullScreenMonitor = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_FULLSCREENMONITOR, _T(""));
    // Prevent Minimize when in Fullscreen mode on non default monitor
    fPreventMinimize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_PREVENT_MINIMIZE, FALSE);
    fUseWin7TaskBar = SysVersion::Is7OrLater() ? !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_WIN7TASKBAR, TRUE) : FALSE;
    fExitAfterPlayback = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_EXIT_AFTER_PB, FALSE);
    fNextInDirAfterPlayback = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NEXT_AFTER_PB, FALSE);
    // TODO: Change IDS_RS_MPC_NO_SEARCH_IN_FOLDER into IDS_RS_SEARCH_IN_FOLDER
    fUseSearchInFolder = !pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NO_SEARCH_IN_FOLDER, FALSE);
    fUseTimeTooltip = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USE_TIME_TOOLTIP, TRUE);
    nTimeTooltipPosition = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TIME_TOOLTIP_POSITION, TIME_TOOLTIP_ABOVE_SEEKBAR);
    nOSDSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_OSD_SIZE, SysVersion::IsVistaOrLater() ? 18 : 20);
    if (SysVersion::IsVistaOrLater()) {
        LOGFONT lf;
        GetMessageFont(&lf);
        strOSDFont = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_MPC_OSD_FONT, lf.lfFaceName);
    } else {
        strOSDFont = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_MPC_OSD_FONT, _T("Arial"));
    }

    // Associated types with icon or not...
    fAssociatedWithIcons = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASSOCIATED_WITH_ICON, TRUE);
    // Last Open Dir
    strLastOpenDir = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LAST_OPEN_DIR, _T("C:\\"));

    if (pApp->GetProfileBinary(IDS_R_SETTINGS, IDS_RS_FULLSCREENRES, &ptr, &len)) {
        if (len == sizeof(AChFR)) {
            memcpy(&AutoChangeFullscrRes, ptr, sizeof(AChFR));
        } else {
            AutoChangeFullscrRes.bEnabled = false;
        }
        delete [] ptr;
    } else {
        AutoChangeFullscrRes.bEnabled = false;
    }

    fExitFullScreenAtTheEnd = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_EXITFULLSCREENATTHEEND, TRUE);
    fRestoreResAfterExit = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_RESTORERESAFTEREXIT, TRUE);
    fRememberWindowPos = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWPOS, FALSE);
    fRememberWindowSize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWSIZE, FALSE);
    CString str = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_PANSCANZOOM, _T(""));
    if (_stscanf_s(str, _T("%lf,%lf"), &dZoomX, &dZoomY) == 2 &&
            dZoomX >= 0.196 && dZoomX <= 3.06 && // 0.196 = 0.2 / 1.02
            dZoomY >= 0.196 && dZoomY <= 3.06) { // 3.06 = 3 * 1.02
        fSavePnSZoom = true;
    } else {
        fSavePnSZoom = false;
        dZoomX = 1.0;
        dZoomY = 1.0;
    }
    fSnapToDesktopEdges = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SNAPTODESKTOPEDGES, FALSE);
    sizeAspectRatio.cx = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_X, 0);
    sizeAspectRatio.cy = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_Y, 0);

    fKeepHistory = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPHISTORY, TRUE);
    CFileAssoc::SetNoRecentDocs(!fKeepHistory);

    if (pApp->GetProfileBinary(IDS_R_SETTINGS, IDS_RS_LASTWINDOWRECT, &ptr, &len)) {
        if (len == sizeof(CRect)) {
            memcpy(&rcLastWindowPos, ptr, sizeof(CRect));
        } else {
            fRememberWindowPos = false;
        }
        delete [] ptr;
    } else {
        fRememberWindowPos = false;
    }
    nLastWindowType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTWINDOWTYPE, SIZE_RESTORED);

    bShufflePlaylistItems = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SHUFFLEPLAYLISTITEMS, FALSE);
    bRememberPlaylistItems = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERPLAYLISTITEMS, TRUE);
    bHidePlaylistFullScreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDEPLAYLISTFULLSCREEN, FALSE);
    bFavRememberPos = !!pApp->GetProfileInt(IDS_R_FAVORITES, IDS_RS_FAV_REMEMBERPOS, TRUE);
    bFavRelativeDrive = !!pApp->GetProfileInt(IDS_R_FAVORITES, IDS_RS_FAV_RELATIVEDRIVE, FALSE);

    strDVDPath = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_DVDPATH, _T(""));
    fUseDVDPath = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USEDVDPATH, FALSE);
    idMenuLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MENULANG, 0);
    idAudioLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOLANG, 0);
    idSubtitlesLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANG, 0);
    fAutoSpeakerConf = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOSPEAKERCONF, TRUE);
    fClosedCaptions = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CLOSEDCAPTIONS, FALSE);
    // TODO: rename subdefstyle -> defStyle, IDS_RS_SPLOGFONT -> IDS_RS_SPSTYLE
    {
        CString temp = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SPLOGFONT, _T(""));
        subdefstyle <<= temp;
        if (temp.IsEmpty()) {
            subdefstyle.relativeTo = 1; // default "Position subtitles relative to the video frame" option is checked
        }
    }
    fOverridePlacement = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPOVERRIDEPLACEMENT, FALSE);
    nHorPos = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPHORPOS, 50);
    nVerPos = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPVERPOS, 90);
    nSubDelayInterval = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBDELAYINTERVAL, 500);

    fEnableSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLESUBTITLES, TRUE);
    bPreferDefaultForcedSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PREFER_FORCED_DEFAULT_SUBTITLES, TRUE);
    fPrioritizeExternalSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITIZEEXTERNALSUBTITLES, TRUE);
    fDisableInternalSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEINTERNALSUBTITLES, FALSE);
    bAllowOverridingExternalSplitterChoice = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ALLOW_OVERRIDING_EXT_SPLITTER, FALSE);
    strSubtitlePaths = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLEPATHS, DEFAULT_SUBTITLE_PATHS);
    fUseDefaultSubtitlesStyle = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USEDEFAULTSUBTITLESSTYLE, FALSE);
    fEnableAudioSwitcher = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOSWITCHER, TRUE);
    fAudioTimeShift = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOTIMESHIFT, FALSE);
    iAudioTimeShift = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOTIMESHIFT, 0);
    fDownSampleTo441 = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DOWNSAMPLETO441, FALSE);
    fCustomChannelMapping = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CUSTOMCHANNELMAPPING, FALSE);

    BOOL bResult = pApp->GetProfileBinary(IDS_R_SETTINGS, IDS_RS_SPEAKERTOCHANNELMAPPING, &ptr, &len);
    if (bResult && len == sizeof(pSpeakerToChannelMap)) {
        memcpy(pSpeakerToChannelMap, ptr, sizeof(pSpeakerToChannelMap));
    } else {
        memset(pSpeakerToChannelMap, 0, sizeof(pSpeakerToChannelMap));
        for (int j = 0; j < 18; j++) {
            for (int i = 0; i <= j; i++) {
                pSpeakerToChannelMap[j][i] = 1 << i;
            }
        }

        pSpeakerToChannelMap[0][0] = 1 << 0;
        pSpeakerToChannelMap[0][1] = 1 << 0;

        pSpeakerToChannelMap[3][0] = 1 << 0;
        pSpeakerToChannelMap[3][1] = 1 << 1;
        pSpeakerToChannelMap[3][2] = 0;
        pSpeakerToChannelMap[3][3] = 0;
        pSpeakerToChannelMap[3][4] = 1 << 2;
        pSpeakerToChannelMap[3][5] = 1 << 3;

        pSpeakerToChannelMap[4][0] = 1 << 0;
        pSpeakerToChannelMap[4][1] = 1 << 1;
        pSpeakerToChannelMap[4][2] = 1 << 2;
        pSpeakerToChannelMap[4][3] = 0;
        pSpeakerToChannelMap[4][4] = 1 << 3;
        pSpeakerToChannelMap[4][5] = 1 << 4;
    }
    if (bResult) {
        delete [] ptr;
    }

    fAudioNormalize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZE, FALSE);
    nAudioMaxNormFactor = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOMAXNORMFACTOR, 400);
    fAudioNormalizeRecover = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZERECOVER, TRUE);
    nAudioBoost = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, -1);
    if (nAudioBoost == UINT(-1)) { // Backward compatibility
        double dAudioBoost_dB = _tstof(pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, _T("0")));
        if (dAudioBoost_dB < 0 || dAudioBoost_dB > 10) {
            dAudioBoost_dB = 0;
        }
        nAudioBoost = UINT(100 * pow(10.0, dAudioBoost_dB / 20.0) + 0.5) - 100;
    }
    if (nAudioBoost > 300) { // Max boost is 300%
        nAudioBoost = 300;
    }

    nSpeakerChannels = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPEAKERCHANNELS, 2);

    // External filters
    ConvertOldExternalFiltersList(); // Here for backward compatibility
    LoadExternalFilters(m_filters);

    fIntRealMedia = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_INTREALMEDIA, FALSE);
    //fRealMediaRenderless = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIARENDERLESS, FALSE);
    //iQuickTimeRenderer = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_QUICKTIMERENDERER, 2);
    //dRealMediaQuickTimeFPS = 25.0;
    //*((DWORD*)&dRealMediaQuickTimeFPS) = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIAFPS, *((DWORD*)&dRealMediaQuickTimeFPS));

    m_pnspresets.RemoveAll();
    for (int i = 0; i < (ID_PANNSCAN_PRESETS_END - ID_PANNSCAN_PRESETS_START); i++) {
        CString str2;
        str2.Format(_T("Preset%d"), i);
        str2 = pApp->GetProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, str2, _T(""));
        if (str2.IsEmpty()) {
            break;
        }
        m_pnspresets.Add(str2);
    }

    if (m_pnspresets.IsEmpty()) {
        double _4p3 = 4.0 / 3.0;
        double _16p9 = 16.0 / 9.0;
        double _185p1 = 1.85 / 1.0;
        double _235p1 = 2.35 / 1.0;
        UNREFERENCED_PARAMETER(_185p1);

        CString str2;
        str2.Format(IDS_SCALE_16_9, 0.5, 0.5, _4p3 / _4p3, _16p9 / _4p3);
        m_pnspresets.Add(str2);
        str2.Format(IDS_SCALE_WIDESCREEN, 0.5, 0.5, _16p9 / _4p3, _16p9 / _4p3);
        m_pnspresets.Add(str2);
        str2.Format(IDS_SCALE_ULTRAWIDE, 0.5, 0.5, _235p1 / _4p3, _235p1 / _4p3);
        m_pnspresets.Add(str2);
    }

    for (int i = 0; i < wmcmds.GetCount(); i++) {
        CString str2;
        str2.Format(_T("CommandMod%d"), i);
        str2 = pApp->GetProfileString(IDS_R_COMMANDS, str2, _T(""));
        if (str2.IsEmpty()) {
            break;
        }
        int cmd, fVirt, key, repcnt;
        UINT mouse, mouseFS, appcmd;
        TCHAR buff[128];
        int n;
        if (5 > (n = _stscanf_s(str2, _T("%d %x %x %s %d %u %u %u"), &cmd, &fVirt, &key, buff, _countof(buff), &repcnt, &mouse, &appcmd, &mouseFS))) {
            break;
        }
        if (POSITION pos = wmcmds.Find(cmd)) {
            wmcmd& wc = wmcmds.GetAt(pos);
            wc.cmd = cmd;
            wc.fVirt = fVirt;
            wc.key = key;
            if (n >= 6) {
                wc.mouse = mouse;
            }
            if (n >= 7) {
                wc.appcmd = appcmd;
            }
            // If there is no distinct bindings for windowed and
            // fullscreen modes we use the same for both.
            wc.mouseFS = (n >= 8) ? mouseFS : wc.mouse;
            wc.rmcmd = CStringA(buff).Trim('\"');
            wc.rmrepcnt = repcnt;
        }
    }

    CAtlArray<ACCEL> pAccel;
    pAccel.SetCount(wmcmds.GetCount());
    POSITION pos = wmcmds.GetHeadPosition();
    for (int i = 0; pos; i++) {
        pAccel[i] = wmcmds.GetNext(pos);
    }
    hAccel = CreateAcceleratorTable(pAccel.GetData(), (int)pAccel.GetCount());

    strWinLircAddr = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WINLIRCADDR, _T("127.0.0.1:8765"));
    fWinLirc = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WINLIRC, FALSE);
    strUIceAddr = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_UICEADDR, _T("127.0.0.1:1234"));
    fUIce = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_UICE, FALSE);
    fGlobalMedia = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_GLOBALMEDIA, TRUE);

    nJumpDistS = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTS, DEFAULT_JUMPDISTANCE_1);
    nJumpDistM = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTM, DEFAULT_JUMPDISTANCE_2);
    nJumpDistL = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTL, DEFAULT_JUMPDISTANCE_3);
    fLimitWindowProportions = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LIMITWINDOWPROPORTIONS, FALSE);

    m_Formats.UpdateData(false);

    // Internal filters
    for (int f = 0; f < SRC_LAST; f++) {
        SrcFilters[f] = !!pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, SrcFiltersKeys[f], TRUE);
    }
    for (int f = 0; f < TRA_LAST; f++) {
        TraFilters[f] = !!pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, TraFiltersKeys[f], TRUE);
    }
    for (int f = 0; f < TRA_DXVA_LAST; f++) {
        DXVAFilters[f] = !!pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, DXVAFiltersKeys[f], TRUE);
    }
    for (int f = 0; f < FFM_LAST; f++) {
        FFmpegFilters[f] = !!pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, FFMFiltersKeys[f], TRUE);
    }
    if (!TRA_DXVA_LAST) {
        DXVAFilters[0] = FALSE;
    }
    if (!FFM_LAST) {
        FFmpegFilters[0] = FALSE;
    }

    strLogoFileName = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LOGOFILE, _T(""));
    nLogoId = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOID, DEF_LOGO);
    fLogoExternal = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOEXT, FALSE);

    fHideCDROMsSubMenu = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECDROMSSUBMENU, FALSE);

    dwPriority = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITY, NORMAL_PRIORITY_CLASS);
    ::SetPriorityClass(::GetCurrentProcess(), dwPriority);
    fLaunchfullscreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LAUNCHFULLSCREEN, FALSE);

    fEnableWebServer = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWEBSERVER, FALSE);
    nWebServerPort = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPORT, 13579);
    fWebServerPrintDebugInfo = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPRINTDEBUGINFO, FALSE);
    fWebServerUseCompression = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERUSECOMPRESSION, TRUE);
    fWebServerLocalhostOnly = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERLOCALHOSTONLY, FALSE);
    strWebRoot = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBROOT, _T("*./webroot"));
    strWebDefIndex = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBDEFINDEX, _T("index.html;index.php"));
    strWebServerCGI = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBSERVERCGI, _T(""));

    CString MyPictures;

    CRegKey key;
    // grrrrr
    // if (!SHGetSpecialFolderPath(nullptr, MyPictures.GetBufferSetLength(MAX_PATH), CSIDL_MYPICTURES, TRUE)) MyPictures.Empty();
    // else MyPictures.ReleaseBuffer();
    if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), KEY_READ)) {
        ULONG len = MAX_PATH;
        if (ERROR_SUCCESS == key.QueryStringValue(_T("My Pictures"), MyPictures.GetBuffer(MAX_PATH), &len)) {
            MyPictures.ReleaseBufferSetLength(len);
        } else {
            MyPictures.Empty();
        }
    }
    strSnapShotPath = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTPATH, MyPictures);
    strSnapShotExt = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTEXT, _T(".jpg"));

    iThumbRows = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBROWS, 4);
    iThumbCols = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBCOLS, 4);
    iThumbWidth = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBWIDTH, 1024);

    strISDb = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_ISDB, _T("www.opensubtitles.org/isdb"));

    nLastUsedPage = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTUSEDPAGE, 0);
    //

    m_shaders.RemoveAll();

    CAtlStringMap<UINT> shaders;

    shaders[_T("16-235 -> 0-255  [SD][HD]")] = IDF_SHADER_LEVELS;
    shaders[_T("16-235 -> 0-255  [SD]")]     = IDF_SHADER_LEVELS2;
    shaders[_T("0-255 -> 16-235")]           = IDF_SHADER_LEVELS3;
    shaders[_T("BT.601 -> BT.709")]          = IDF_SHADER_BT601_BT709;
    shaders[_T("YV12 Chroma Upsampling")]    = IDF_SHADER_YV12CHROMAUP;
    shaders[_T("grayscale")]                 = IDF_SHADER_GRAYSCALE;

    shaders[_T("sharpen")]                   = IDF_SHADER_SHARPEN;
    shaders[_T("sharpen complex")]           = IDF_SHADER_SHARPEN_COMPLEX;
    shaders[_T("sharpen complex 2")]         = IDF_SHADER_SHARPEN_COMPLEX2;
    shaders[_T("edge sharpen")]              = IDF_SHADER_EDGE_SHARPEN;
    shaders[_T("contour")]                   = IDF_SHADER_CONTOUR;

    shaders[_T("deinterlace (blend)")]       = IDF_SHADER_DEINTERLACE;
    shaders[_T("denoise")]                   = IDF_SHADER_DENOISE;
    shaders[_T("procamp")]                   = IDF_SHADER_PROCAMP;
    shaders[_T("invert")]                    = IDF_SHADER_INVERT;
    shaders[_T("letterbox")]                 = IDF_SHADER_LETTERBOX;
    shaders[_T("LCD angle correction")]      = IDF_SHADER_LCD;

    shaders[_T("nightvision")]               = IDF_SHADER_NIGHTVISION;
    shaders[_T("emboss")]                    = IDF_SHADER_EMBOSS;
    shaders[_T("sphere")]                    = IDF_SHADER_SPHERE;
    shaders[_T("spotlight")]                 = IDF_SHADER_SPOTLIGHT;
    shaders[_T("wave")]                      = IDF_SHADER_WAVE;

    for (int iShader = 0; ; iShader++) {
        CString str2;
        str2.Format(_T("%d"), iShader);
        str2 = pApp->GetProfileString(IDS_R_SHADERS, str2);

        CAtlList<CString> sl;
        CString label = Explode(str2, sl, '|');
        if (label.IsEmpty()) {
            break;
        }
        if (sl.GetCount() < 3) {
            continue;
        }

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
    while (pos) {
        CAtlStringMap<UINT>::CPair* pPair = shaders.GetNext(pos);

        CStringA srcdata;
        if (LoadResource(pPair->m_value, srcdata, _T("SHADER"))) {
            Shader s;
            s.label = pPair->m_key;

            // Select minimum version for each shader!
            switch (pPair->m_value) {
                case IDF_SHADER_DENOISE:
                    s.target = _T("ps_3_0");
                    break;
                case IDF_SHADER_SHARPEN_COMPLEX2:
                    s.target = _T("ps_2_a");
                    break;
                default:
                    s.target = _T("ps_2_0");
                    break;
            }
            s.srcdata = CString(srcdata);
            m_shaders.AddTail(s);
        }
    }

    // CASIMIR666 : new settings
    fD3DFullscreen          = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_D3DFULLSCREEN, FALSE);
    //fMonitorAutoRefreshRate = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MONITOR_AUTOREFRESHRATE, FALSE);

    iBrightness = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_BRIGHTNESS, 0);
    iContrast   = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_CONTRAST, 0);
    iHue        = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_HUE, 0);
    iSaturation = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_SATURATION, 0);

    strShaderList   = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLIST, _T(""));
    strShaderListScreenSpace = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLISTSCREENSPACE, _T(""));
    fToggleShader = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADER, FALSE);
    fToggleShaderScreenSpace = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADERSSCREENSPACE, FALSE);

    fShowOSD              = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SHOWOSD, TRUE);
    fEnableEDLEditor      = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEEDLEDITOR, FALSE);
    fFastSeek             = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FASTSEEK_KEYFRAME, FALSE);
    fShowChapters         = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SHOW_CHAPTERS, TRUE);


    fLCDSupport = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LCD_SUPPORT, FALSE);

    // Save analog capture settings
    iDefaultCaptureDevice = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULT_CAPTURE, 0);
    strAnalogVideo        = pApp->GetProfileString(IDS_R_CAPTURE, IDS_RS_VIDEO_DISP_NAME, _T("dummy"));
    strAnalogAudio        = pApp->GetProfileString(IDS_R_CAPTURE, IDS_RS_AUDIO_DISP_NAME, _T("dummy"));
    iAnalogCountry        = pApp->GetProfileInt(IDS_R_CAPTURE, IDS_RS_COUNTRY, 1);

    strBDANetworkProvider = pApp->GetProfileString(IDS_R_DVB, IDS_RS_BDA_NETWORKPROVIDER, _T(""));
    strBDATuner           = pApp->GetProfileString(IDS_R_DVB, IDS_RS_BDA_TUNER, _T(""));
    strBDAReceiver        = pApp->GetProfileString(IDS_R_DVB, IDS_RS_BDA_RECEIVER, _T(""));
    //sBDAStandard        = pApp->GetProfileString(IDS_R_DVB, IDS_RS_BDA_STANDARD, _T(""));
    iBDAScanFreqStart     = pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_SCAN_FREQ_START, 474000);
    iBDAScanFreqEnd       = pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_SCAN_FREQ_END, 858000);
    iBDABandwidth         = pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_BANDWIDTH, 8);
    fBDAUseOffset         = !!pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_USE_OFFSET, FALSE);
    iBDAOffset            = pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_OFFSET, 166);
    fBDAIgnoreEncryptedChannels = !!pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_IGNORE_ENCRYPTED_CHANNELS, FALSE);
    nDVBLastChannel       = pApp->GetProfileInt(IDS_R_DVB, IDS_RS_DVB_LAST_CHANNEL, 1);

    for (int iChannel = 0; ; iChannel++) {
        CString strTemp;
        CString strChannel;
        CDVBChannel Channel;
        strTemp.Format(_T("%d"), iChannel);
        strChannel = pApp->GetProfileString(IDS_R_DVB, strTemp, _T(""));
        if (strChannel.IsEmpty()) {
            break;
        }
        Channel.FromString(strChannel);
        m_DVBChannels.AddTail(Channel);
    }

    // playback positions for last played files
    fRememberFilePos = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOS, FALSE);
    filePositions.Load();

    // playback positions for last played DVDs
    fRememberDVDPos = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DVDPOS, FALSE);
    dvdPositions.Load();

    fLastFullScreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTFULLSCREEN, FALSE);

    // TODO: sort shaders by label

    fRemainingTime = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMAINING_TIME, FALSE);

    nUpdaterAutoCheck = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_UPDATER_AUTO_CHECK, AUTOUPDATE_UNKNOWN);
    nUpdaterDelay = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_UPDATER_DELAY, 7);
    if (nUpdaterDelay < 1) {
        nUpdaterDelay = 1;
    }

    bNotifySkype = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_NOTIFY_SKYPE, FALSE);

    if (fLaunchfullscreen) {
        nCLSwitches |= CLSW_FULLSCREEN;
    }

    fInitialized = true;
}

bool CAppSettings::GetAllowMultiInst() const
{
    return !!AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MULTIINST, FALSE);
}

void CAppSettings::UpdateRenderersData(bool fSave)
{
    CWinApp* pApp = AfxGetApp();
    CRenderersSettings& r = m_RenderersSettings;
    CRenderersSettings::CAdvRendererSettings& ars = r.m_AdvRendSets;

    if (fSave) {
        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_APSURACEFUSAGE, r.iAPSurfaceUsage);
        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DX9_RESIZER, r.iDX9Resizer);
        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERMODE, r.fVMR9MixerMode);
        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERYUV, r.fVMR9MixerYUV);

        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRAlternateVSync"), ars.fVMR9AlterativeVSync);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSyncOffset"), ars.iVMR9VSyncOffset);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSyncAccurate2"), ars.iVMR9VSyncAccurate);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFullscreenGUISupport"), ars.iVMR9FullscreenGUISupport);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSync"), ars.iVMR9VSync);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRDisableDesktopComposition"), ars.iVMRDisableDesktopComposition);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFullFloatingPointProcessing"), ars.iVMR9FullFloatingPointProcessing);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRHalfFloatingPointProcessing"), ars.iVMR9HalfFloatingPointProcessing);

        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementEnable"), ars.iVMR9ColorManagementEnable);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementInput"), ars.iVMR9ColorManagementInput);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementAmbientLight"), ars.iVMR9ColorManagementAmbientLight);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementIntent"), ars.iVMR9ColorManagementIntent);

        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVROutputRange"), ars.iEVROutputRange);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVRHighColorRes"), ars.iEVRHighColorResolution);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVRForceInputHighColorRes"), ars.iEVRForceInputHighColorResolution);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVREnableFrameTimeCorrection"), ars.iEVREnableFrameTimeCorrection);

        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUBeforeVSync"), ars.iVMRFlushGPUBeforeVSync);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUAfterPresent"), ars.iVMRFlushGPUAfterPresent);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUWait"), ars.iVMRFlushGPUWait);

        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeClock"), ars.bSynchronizeVideo);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeDisplay"), ars.bSynchronizeDisplay);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeNearest"), ars.bSynchronizeNearest);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("LineDelta"), ars.iLineDelta);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ColumnDelta"), ars.iColumnDelta);

        pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("CycleDelta"), (LPBYTE) & (ars.fCycleDelta), sizeof(ars.fCycleDelta));
        pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("TargetSyncOffset"), (LPBYTE) & (ars.fTargetSyncOffset), sizeof(ars.fTargetSyncOffset));
        pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("ControlLimit"), (LPBYTE) & (ars.fControlLimit), sizeof(ars.fControlLimit));

        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ResetDevice"), r.fResetDevice);

        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPCSIZE, r.nSPCSize);
        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPCMAXRES, r.nSPCMaxRes);
        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_POW2TEX, r.fSPCPow2Tex);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SPCAllowAnimationWhenBuffering"), r.fSPCAllowAnimationWhenBuffering);

        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_EVR_BUFFERS, r.iEvrBuffers);

        pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_D3D9RENDERDEVICE, r.D3D9RenderDevice);
    } else {
        r.iAPSurfaceUsage = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_APSURACEFUSAGE, (SysVersion::IsVistaOrLater() ? VIDRNDT_AP_TEXTURE3D : VIDRNDT_AP_TEXTURE2D));
        r.iDX9Resizer = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DX9_RESIZER, 1);
        r.fVMR9MixerMode = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERMODE, TRUE);
        r.fVMR9MixerYUV = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERYUV, FALSE);

        CRenderersSettings::CAdvRendererSettings DefaultSettings;
        ars.fVMR9AlterativeVSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRAlternateVSync"), DefaultSettings.fVMR9AlterativeVSync);
        ars.iVMR9VSyncOffset = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSyncOffset"), DefaultSettings.iVMR9VSyncOffset);
        ars.iVMR9VSyncAccurate = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSyncAccurate2"), DefaultSettings.iVMR9VSyncAccurate);
        ars.iVMR9FullscreenGUISupport = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFullscreenGUISupport"), DefaultSettings.iVMR9FullscreenGUISupport);
        ars.iEVRHighColorResolution = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVRHighColorRes"), DefaultSettings.iEVRHighColorResolution);
        ars.iEVRForceInputHighColorResolution = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVRForceInputHighColorRes"), DefaultSettings.iEVRForceInputHighColorResolution);
        ars.iEVREnableFrameTimeCorrection = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVREnableFrameTimeCorrection"), DefaultSettings.iEVREnableFrameTimeCorrection);
        ars.iVMR9VSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSync"), DefaultSettings.iVMR9VSync);
        ars.iVMRDisableDesktopComposition = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRDisableDesktopComposition"), DefaultSettings.iVMRDisableDesktopComposition);
        ars.iVMR9FullFloatingPointProcessing = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFullFloatingPointProcessing"), DefaultSettings.iVMR9FullFloatingPointProcessing);
        ars.iVMR9HalfFloatingPointProcessing = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRHalfFloatingPointProcessing"), DefaultSettings.iVMR9HalfFloatingPointProcessing);

        ars.iVMR9ColorManagementEnable = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementEnable"), DefaultSettings.iVMR9ColorManagementEnable);
        ars.iVMR9ColorManagementInput = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementInput"), DefaultSettings.iVMR9ColorManagementInput);
        ars.iVMR9ColorManagementAmbientLight = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementAmbientLight"), DefaultSettings.iVMR9ColorManagementAmbientLight);
        ars.iVMR9ColorManagementIntent = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementIntent"), DefaultSettings.iVMR9ColorManagementIntent);

        ars.iEVROutputRange = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVROutputRange"), DefaultSettings.iEVROutputRange);

        ars.iVMRFlushGPUBeforeVSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUBeforeVSync"), DefaultSettings.iVMRFlushGPUBeforeVSync);
        ars.iVMRFlushGPUAfterPresent = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUAfterPresent"), DefaultSettings.iVMRFlushGPUAfterPresent);
        ars.iVMRFlushGPUWait = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUWait"), DefaultSettings.iVMRFlushGPUWait);

        ars.bSynchronizeVideo = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SynchronizeClock"), DefaultSettings.bSynchronizeVideo);
        ars.bSynchronizeDisplay = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SynchronizeDisplay"), DefaultSettings.bSynchronizeDisplay);
        ars.bSynchronizeNearest = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SynchronizeNearest"), DefaultSettings.bSynchronizeNearest);
        ars.iLineDelta = pApp->GetProfileInt(IDS_R_SETTINGS, _T("LineDelta"), DefaultSettings.iLineDelta);
        ars.iColumnDelta = pApp->GetProfileInt(IDS_R_SETTINGS, _T("ColumnDelta"), DefaultSettings.iColumnDelta);

        double* dPtr;
        UINT dSize;
        if (pApp->GetProfileBinary(IDS_R_SETTINGS, _T("CycleDelta"), (LPBYTE*)&dPtr, &dSize)) {
            ars.fCycleDelta = *dPtr;
            delete [] dPtr;
        }

        if (pApp->GetProfileBinary(IDS_R_SETTINGS, _T("TargetSyncOffset"), (LPBYTE*)&dPtr, &dSize)) {
            ars.fTargetSyncOffset = *dPtr;
            delete [] dPtr;
        }
        if (pApp->GetProfileBinary(IDS_R_SETTINGS, _T("ControlLimit"), (LPBYTE*)&dPtr, &dSize)) {
            ars.fControlLimit = *dPtr;
            delete [] dPtr;
        }

        r.fResetDevice = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("ResetDevice"), TRUE);

        r.nSPCSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPCSIZE, 10);
        r.nSPCMaxRes = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPCMAXRES, 0);
        r.fSPCPow2Tex = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_POW2TEX, TRUE);
        r.fSPCAllowAnimationWhenBuffering = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SPCAllowAnimationWhenBuffering"), TRUE);

        r.iEvrBuffers = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_EVR_BUFFERS, 5);
        r.D3D9RenderDevice = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_D3D9RENDERDEVICE, _T(""));
    }
}

__int64 CAppSettings::ConvertTimeToMSec(CString& time) const
{
    __int64 Sec = 0;
    __int64 mSec = 0;
    __int64 mult = 1;

    int pos = time.GetLength() - 1;
    if (pos < 3) {
        return 0;
    }

    while (pos >= 0) {
        TCHAR ch = time[pos];
        if (ch == '.') {
            mSec = Sec * 1000 / mult;
            Sec = 0;
            mult = 1;
        } else if (ch == ':') {
            mult = mult * 6 / 10;
        } else if (ch >= '0' && ch <= '9') {
            Sec += (ch - '0') * mult;
            mult *= 10;
        } else {
            mSec = Sec = 0;
            break;
        }
        pos--;
    }
    return Sec * 1000 + mSec;
}

void CAppSettings::ExtractDVDStartPos(CString& strParam)
{
    int i = 0, j = 0;
    for (CString token = strParam.Tokenize(_T("#"), i);
            j < 3 && !token.IsEmpty();
            token = strParam.Tokenize(_T("#"), i), j++) {
        switch (j) {
            case 0:
                lDVDTitle = token.IsEmpty() ? 0 : (ULONG)_wtol(token);
                break;
            case 1:
                if (token.Find(':') > 0) {
                    UINT h = 0, m = 0, s = 0, f = 0;
                    int nRead = _stscanf_s(token, _T("%02u:%02u:%02u.%03u"), &h, &m, &s, &f);
                    if (nRead >= 3) {
                        DVDPosition.bHours   = (BYTE)h;
                        DVDPosition.bMinutes = (BYTE)m;
                        DVDPosition.bSeconds = (BYTE)s;
                        DVDPosition.bFrames  = (BYTE)f;
                    }
                } else {
                    lDVDChapter = token.IsEmpty() ? 0 : (ULONG)_wtol(token);
                }
                break;
        }
    }
}

CString CAppSettings::ParseFileName(CString const& param)
{
    CString fullPathName;

    // Try to transform relative pathname into full pathname
    if (param.Find(_T(":")) < 0) {
        fullPathName.ReleaseBuffer(GetFullPathName(param, MAX_PATH, fullPathName.GetBuffer(MAX_PATH), nullptr));

        if (!fullPathName.IsEmpty() && FileExists(fullPathName)) {
            return fullPathName;
        }
    }

    return param;
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
    memset(&DVDPosition, 0, sizeof(DVDPosition));
    iAdminOption = 0;
    sizeFixedWindow.SetSize(0, 0);
    iMonitor = 0;
    strPnSPreset.Empty();

    POSITION pos = cmdln.GetHeadPosition();
    while (pos) {
        const CString& param = cmdln.GetNext(pos);
        if (param.IsEmpty()) {
            continue;
        }

        if ((param[0] == '-' || param[0] == '/') && param.GetLength() > 1) {
            CString sw = param.Mid(1).MakeLower();
            if (sw == _T("open")) {
                nCLSwitches |= CLSW_OPEN;
            } else if (sw == _T("play")) {
                nCLSwitches |= CLSW_PLAY;
            } else if (sw == _T("fullscreen")) {
                nCLSwitches |= CLSW_FULLSCREEN;
            } else if (sw == _T("minimized")) {
                nCLSwitches |= CLSW_MINIMIZED;
            } else if (sw == _T("new")) {
                nCLSwitches |= CLSW_NEW;
            } else if (sw == _T("help") || sw == _T("h") || sw == _T("?")) {
                nCLSwitches |= CLSW_HELP;
            } else if (sw == _T("dub") && pos) {
                slDubs.AddTail(ParseFileName(cmdln.GetNext(pos)));
            } else if (sw == _T("dubdelay") && pos) {
                CString strFile = ParseFileName(cmdln.GetNext(pos));
                int nPos = strFile.Find(_T("DELAY"));
                if (nPos != -1) {
                    rtShift = 10000 * _tstol(strFile.Mid(nPos + 6));
                }
                slDubs.AddTail(strFile);
            } else if (sw == _T("sub") && pos) {
                slSubs.AddTail(ParseFileName(cmdln.GetNext(pos)));
            } else if (sw == _T("filter") && pos) {
                slFilters.AddTail(cmdln.GetNext(pos));
            } else if (sw == _T("dvd")) {
                nCLSwitches |= CLSW_DVD;
            } else if (sw == _T("dvdpos") && pos) {
                ExtractDVDStartPos(cmdln.GetNext(pos));
            } else if (sw == _T("cd")) {
                nCLSwitches |= CLSW_CD;
            } else if (sw == _T("add")) {
                nCLSwitches |= CLSW_ADD;
            } else if (sw == _T("regvid")) {
                nCLSwitches |= CLSW_REGEXTVID;
            } else if (sw == _T("regaud")) {
                nCLSwitches |= CLSW_REGEXTAUD;
            } else if (sw == _T("regpl")) {
                nCLSwitches |= CLSW_REGEXTPL;
            } else if (sw == _T("regall")) {
                nCLSwitches |= (CLSW_REGEXTVID | CLSW_REGEXTAUD | CLSW_REGEXTPL);
            } else if (sw == _T("unregall")) {
                nCLSwitches |= CLSW_UNREGEXT;
            } else if (sw == _T("unregvid")) {
                nCLSwitches |= CLSW_UNREGEXT;    /* keep for compatibility with old versions */
            } else if (sw == _T("unregaud")) {
                nCLSwitches |= CLSW_UNREGEXT;    /* keep for compatibility with old versions */
            } else if (sw == _T("iconsassoc")) {
                nCLSwitches |= CLSW_ICONSASSOC;
            } else if (sw == _T("start") && pos) {
                rtStart = 10000i64 * _tcstol(cmdln.GetNext(pos), nullptr, 10);
                nCLSwitches |= CLSW_STARTVALID;
            } else if (sw == _T("startpos") && pos) {
                rtStart = 10000i64 * ConvertTimeToMSec(cmdln.GetNext(pos));
                nCLSwitches |= CLSW_STARTVALID;
            } else if (sw == _T("nofocus")) {
                nCLSwitches |= CLSW_NOFOCUS;
            } else if (sw == _T("close")) {
                nCLSwitches |= CLSW_CLOSE;
            } else if (sw == _T("standby")) {
                nCLSwitches |= CLSW_STANDBY;
            } else if (sw == _T("hibernate")) {
                nCLSwitches |= CLSW_HIBERNATE;
            } else if (sw == _T("shutdown")) {
                nCLSwitches |= CLSW_SHUTDOWN;
            } else if (sw == _T("logoff")) {
                nCLSwitches |= CLSW_LOGOFF;
            } else if (sw == _T("lock")) {
                nCLSwitches |= CLSW_LOCK;
            } else if (sw == _T("d3dfs")) {
                nCLSwitches |= CLSW_D3DFULLSCREEN;
            } else if (sw == _T("adminoption") && pos) {
                nCLSwitches |= CLSW_ADMINOPTION;
                iAdminOption = _ttoi(cmdln.GetNext(pos));
            } else if (sw == _T("slave") && pos) {
                nCLSwitches |= CLSW_SLAVE;
                hMasterWnd = (HWND)_ttol(cmdln.GetNext(pos));
            } else if (sw == _T("fixedsize") && pos) {
                CAtlList<CString> sl;
                Explode(cmdln.GetNext(pos), sl, ',', 2);
                if (sl.GetCount() == 2) {
                    sizeFixedWindow.SetSize(_ttol(sl.GetHead()), _ttol(sl.GetTail()));
                    if (sizeFixedWindow.cx > 0 && sizeFixedWindow.cy > 0) {
                        nCLSwitches |= CLSW_FIXEDSIZE;
                    }
                }
            } else if (sw == _T("monitor") && pos) {
                iMonitor = _tcstol(cmdln.GetNext(pos), nullptr, 10);
                nCLSwitches |= CLSW_MONITOR;
            } else if (sw == _T("pns") && pos) {
                strPnSPreset = cmdln.GetNext(pos);
            } else if (sw == _T("webport") && pos) {
                int tmpport = _tcstol(cmdln.GetNext(pos), nullptr, 10);
                if (tmpport >= 0 && tmpport <= 65535) {
                    nCmdlnWebServerPort = tmpport;
                }
            } else if (sw == _T("debug")) {
                fShowDebugInfo = true;
            } else if (sw == _T("nominidump")) {
                CMiniDump::Disable();
            } else if (sw == _T("audiorenderer") && pos) {
                SetAudioRenderer(_ttoi(cmdln.GetNext(pos)));
            } else if (sw == _T("reset")) {
                nCLSwitches |= CLSW_RESET;
            } else {
                nCLSwitches |= CLSW_HELP | CLSW_UNRECOGNIZEDSWITCH;
            }
        } else {
            slFiles.AddTail(ParseFileName(param));
        }
    }
}

void CAppSettings::GetFav(favtype ft, CAtlList<CString>& sl) const
{
    sl.RemoveAll();

    CString root;

    switch (ft) {
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

    for (int i = 0; ; i++) {
        CString s;
        s.Format(_T("Name%d"), i);
        s = AfxGetApp()->GetProfileString(root, s, nullptr);
        if (s.IsEmpty()) {
            break;
        }
        sl.AddTail(s);
    }
}

void CAppSettings::SetFav(favtype ft, CAtlList<CString>& sl)
{
    CString root;

    switch (ft) {
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

    AfxGetApp()->WriteProfileString(root, nullptr, nullptr);

    int i = 0;
    POSITION pos = sl.GetHeadPosition();
    while (pos) {
        CString s;
        s.Format(_T("Name%d"), i++);
        AfxGetApp()->WriteProfileString(root, s, sl.GetNext(pos));
    }
}

void CAppSettings::AddFav(favtype ft, CString s)
{
    CAtlList<CString> sl;
    GetFav(ft, sl);
    if (sl.Find(s)) {
        return;
    }
    sl.AddTail(s);
    SetFav(ft, sl);
}

CDVBChannel* CAppSettings::FindChannelByPref(int nPrefNumber)
{
    POSITION pos = m_DVBChannels.GetHeadPosition();
    while (pos) {
        CDVBChannel& Channel = m_DVBChannels.GetNext(pos);
        if (Channel.GetPrefNumber() == nPrefNumber) {
            return &Channel;
        }
    }

    return nullptr;
}

// Settings::CRecentFileAndURLList
CAppSettings::CRecentFileAndURLList::CRecentFileAndURLList(UINT nStart, LPCTSTR lpszSection,
        LPCTSTR lpszEntryFormat, int nSize,
        int nMaxDispLen)
    : CRecentFileList(nStart, lpszSection, lpszEntryFormat, nSize, nMaxDispLen)
{
}

extern BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
extern BOOL AFXAPI AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2);

void CAppSettings::CRecentFileAndURLList::Add(LPCTSTR lpszPathName)
{
    ASSERT(m_arrNames != nullptr);
    ASSERT(lpszPathName != nullptr);
    ASSERT(AfxIsValidString(lpszPathName));

    if (CString(lpszPathName).MakeLower().Find(_T("@device:")) >= 0) {
        return;
    }

    bool fURL = (CString(lpszPathName).Find(_T("://")) >= 0);

    // fully qualify the path name
    TCHAR szTemp[1024];
    if (fURL) {
        _tcscpy_s(szTemp, lpszPathName);
    } else {
        AfxFullPath(szTemp, lpszPathName);
    }

    // update the MRU list, if an existing MRU string matches file name
    int iMRU;
    for (iMRU = 0; iMRU < m_nSize - 1; iMRU++) {
        if ((fURL && !_tcscmp(m_arrNames[iMRU], szTemp))
                || AfxComparePath(m_arrNames[iMRU], szTemp)) {
            break;    // iMRU will point to matching entry
        }
    }
    // move MRU strings before this one down
    for (; iMRU > 0; iMRU--) {
        ASSERT(iMRU > 0);
        ASSERT(iMRU < m_nSize);
        m_arrNames[iMRU] = m_arrNames[iMRU - 1];
    }
    // place this one at the beginning
    m_arrNames[0] = szTemp;
}

bool CAppSettings::IsVSFilterInstalled()
{
    return IsCLSIDRegistered(_T("{9852A670-F845-491B-9BE6-EBD841B8A613}"));
}

bool CAppSettings::HasEVR()
{
    return IsCLSIDRegistered(CLSID_EnhancedVideoRenderer);
}
