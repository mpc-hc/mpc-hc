/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2018 see Authors.txt
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
#include "AppSettings.h"
#include "CrashReporter.h"
#include "FGFilter.h"
#include "FakeFilterMapper2.h"
#include "FileAssoc.h"
#include "ExceptionHandler.h"
#include "PathUtils.h"
#include "Translations.h"
#include "UpdateChecker.h"
#include "WinAPIUtils.h"
#include "moreuuids.h"
#include "mplayerc.h"
#include "../thirdparty/sanear/sanear/src/Factory.h"
#include <VersionHelpersInternal.h>
#include <mvrInterfaces.h>

#pragma warning(push)
#pragma warning(disable: 4351) // new behavior: elements of array 'array' will be default initialized
CAppSettings::CAppSettings()
    : bInitialized(false)
    , nCLSwitches(0)
    , rtShift(0)
    , rtStart(0)
    , lDVDTitle(0)
    , lDVDChapter(0)
    , iMonitor(0)
    , fShowDebugInfo(false)
    , iAdminOption(0)
    , fAllowMultipleInst(false)
    , fTrayIcon(false)
    , fShowOSD(true)
    , fLimitWindowProportions(false)
    , fSnapToDesktopEdges(false)
    , fHideCDROMsSubMenu(false)
    , dwPriority(NORMAL_PRIORITY_CLASS)
    , iTitleBarTextStyle(1)
    , fTitleBarTextTitle(false)
    , fKeepHistory(true)
    , iRecentFilesNumber(20)
    , MRU(0, _T("Recent File List"), _T("File%d"), iRecentFilesNumber)
    , MRUDub(0, _T("Recent Dub List"), _T("Dub%d"), iRecentFilesNumber)
    , filePositions(AfxGetApp(), IDS_R_SETTINGS, iRecentFilesNumber)
    , dvdPositions(AfxGetApp(), IDS_R_SETTINGS, iRecentFilesNumber)
    , fRememberDVDPos(false)
    , fRememberFilePos(false)
    , iRememberPosForLongerThan(0)
    , bRememberPosForAudioFiles(true)
    , bRememberPlaylistItems(true)
    , fRememberWindowPos(false)
    , fRememberWindowSize(false)
    , fSavePnSZoom(false)
    , dZoomX(1.0)
    , dZoomY(1.0)
    , fAssociatedWithIcons(true)
    , hAccel(nullptr)
    , fWinLirc(false)
    , fUIce(false)
    , fGlobalMedia(true)
    , nLogoId(-1)
    , fLogoExternal(false)
    , fEnableWebServer(false)
    , nWebServerPort(13579)
    , nCmdlnWebServerPort(-1)
    , fWebServerUseCompression(true)
    , fWebServerLocalhostOnly(false)
    , bWebUIEnablePreview(false)
    , fWebServerPrintDebugInfo(false)
    , nVolume(100)
    , fMute(false)
    , nBalance(0)
    , nLoops(1)
    , fLoopForever(false)
    , eLoopMode(LoopMode::PLAYLIST)
    , fRememberZoomLevel(true)
    , nAutoFitFactor(75)
    , iZoomLevel(1)
    , fEnableWorkerThreadForOpening(true)
    , fReportFailedPins(true)
    , fAutoloadAudio(true)
    , fBlockVSFilter(true)
    , nVolumeStep(5)
    , nSpeedStep(0)
    , nDefaultToolbarSize(24)
    , eAfterPlayback(AfterPlayback::DO_NOTHING)
    , fUseDVDPath(false)
    , idMenuLang(0)
    , idAudioLang(0)
    , idSubtitlesLang(0)
    , fClosedCaptions(false)
    , iDSVideoRendererType(VIDRNDT_DS_DEFAULT)
    , iRMVideoRendererType(VIDRNDT_RM_DEFAULT)
    , iQTVideoRendererType(VIDRNDT_QT_DEFAULT)
    , fD3DFullscreen(false)
    , fLaunchfullscreen(false)
    , bHideFullscreenControls(true)
    , eHideFullscreenControlsPolicy(HideFullscreenControlsPolicy::SHOW_WHEN_HOVERED)
    , uHideFullscreenControlsDelay(0)
    , bHideFullscreenDockedPanels(true)
    , fExitFullScreenAtTheEnd(true)
    , iDefaultCaptureDevice(0)
    , iAnalogCountry(1)
    , iBDAScanFreqStart(474000)
    , iBDAScanFreqEnd(858000)
    , iBDABandwidth(8)
    , fBDAUseOffset(false)
    , iBDAOffset(166)
    , fBDAIgnoreEncryptedChannels(false)
    , nDVBLastChannel(INT_ERROR)
    , nDVBRebuildFilterGraph(DVB_REBUILD_FG_WHEN_SWITCHING)
    , nDVBStopFilterGraph(DVB_STOP_FG_WHEN_SWITCHING)
    , SrcFilters()
    , TraFilters()
    , fEnableAudioSwitcher(true)
    , fAudioNormalize(false)
    , nAudioMaxNormFactor(400)
    , fAudioNormalizeRecover(true)
    , nAudioBoost(0)
    , fDownSampleTo441(false)
    , fAudioTimeShift(false)
    , iAudioTimeShift(0)
    , fCustomChannelMapping(false)
    , nSpeakerChannels(2)
    , pSpeakerToChannelMap()
    , fOverridePlacement(false)
    , nHorPos(50)
    , nVerPos(90)
    , bSubtitleARCompensation(true)
    , nSubDelayStep(500)
    , bPreferDefaultForcedSubtitles(true)
    , fPrioritizeExternalSubtitles(true)
    , fDisableInternalSubtitles(true)
    , bAllowOverridingExternalSplitterChoice(false)
    , bAutoDownloadSubtitles(false)
    , nAutoDownloadScoreMovies(0x16)
    , nAutoDownloadScoreSeries(0x18)
    , bAutoUploadSubtitles(false)
    , bPreferHearingImpairedSubtitles(false)
    , bMPCTheme(false)
    , bMPCThemeLoaded(false)
    , bWindows10DarkThemeActive(false)
    , bWindows10AccentColorsEnabled(false)
    , bModernSeekbar(true)
    , iModernSeekbarHeight(MIN_MODERN_SEEKBAR_HEIGHT)
    , nJumpDistS(DEFAULT_JUMPDISTANCE_1)
    , nJumpDistM(DEFAULT_JUMPDISTANCE_2)
    , nJumpDistL(DEFAULT_JUMPDISTANCE_3)
    , bFastSeek(true)
    , eFastSeekMethod(FASTSEEK_NEAREST_KEYFRAME)
    , fShowChapters(true)
    , bNotifySkype(false)
    , fPreventMinimize(false)
    , bUseEnhancedTaskBar(true)
    , fLCDSupport(false)
    , fUseSearchInFolder(false)
    , fUseTimeTooltip(true)
    , nTimeTooltipPosition(TIME_TOOLTIP_ABOVE_SEEKBAR)
    , nOSDSize(0)
    , bHideWindowedMousePointer(true)
    , iBrightness(0)
    , iContrast(0)
    , iHue(0)
    , iSaturation(0)
    , nUpdaterAutoCheck(-1)
    , nUpdaterDelay(7)
    , eCaptionMenuMode(MODE_SHOWCAPTIONMENU)
    , fHideNavigation(false)
    , nCS(CS_SEEKBAR | CS_TOOLBAR | CS_STATUSBAR)
    , language(LANGID(-1))
    , fEnableSubtitles(true)
    , fUseDefaultSubtitlesStyle(false)
    , iDefaultVideoSize(DVS_FROMINSIDE)
    , fKeepAspectRatio(true)
    , fCompMonDeskARDiff(false)
    , iOnTop(0)
    , bFavRememberPos(true)
    , bFavRelativeDrive(false)
    , iThumbRows(4)
    , iThumbCols(4)
    , iThumbWidth(1024)
    , bSubSaveExternalStyleFile(false)
    , bShufflePlaylistItems(false)
    , bHidePlaylistFullScreen(false)
    , nLastWindowType(SIZE_RESTORED)
    , nLastUsedPage(0)
    , fRemainingTime(false)
    , bHighPrecisionTimer(false)
    , fLastFullScreen(false)
    , fIntRealMedia(false)
    , fEnableEDLEditor(false)
    , hMasterWnd(nullptr)
    , bHideWindowedControls(false)
    , nJpegQuality(90)
    , bEnableCoverArt(true)
    , nCoverArtSizeLimit(600)
    , bEnableLogging(false)
    , bUseLegacyToolbar(false)
    , iLAVGPUDevice(DWORD_MAX)
    , nCmdVolume(0)
    , eSubtitleRenderer(SubtitleRenderer::INTERNAL)
    , bUseYDL(true)
    , iYDLMaxHeight(1440)
    , iYDLVideoFormat(0)
    , bYDLAudioOnly(false)
    , sYDLCommandLine(_T(""))
{
    // Internal source filter
#if INTERNAL_SOURCEFILTER_CDDA
    SrcFiltersKeys[SRC_CDDA] = FilterKey(_T("SRC_CDDA"), true);
#endif
#if INTERNAL_SOURCEFILTER_CDXA
    SrcFiltersKeys[SRC_CDXA] = FilterKey(_T("SRC_CDXA"), true);
#endif
#if INTERNAL_SOURCEFILTER_VTS
    SrcFiltersKeys[SRC_VTS] = FilterKey(_T("SRC_VTS"), true);
#endif
#if INTERNAL_SOURCEFILTER_FLIC
    SrcFiltersKeys[SRC_FLIC] = FilterKey(_T("SRC_FLIC"), true);
#endif
#if INTERNAL_SOURCEFILTER_AC3
    SrcFiltersKeys[SRC_AC3] = FilterKey(_T("SRC_AC3"), true);
#endif
#if INTERNAL_SOURCEFILTER_DTS
    SrcFiltersKeys[SRC_DTS] = FilterKey(_T("SRC_DTS"), true);
#endif
#if INTERNAL_SOURCEFILTER_MATROSKA
    SrcFiltersKeys[SRC_MATROSKA] = FilterKey(_T("SRC_MATROSKA"), true);
#endif
#if INTERNAL_SOURCEFILTER_HTTP
    SrcFiltersKeys[SRC_HTTP] = FilterKey(_T("SRC_HTTP"), true);
#endif
#if INTERNAL_SOURCEFILTER_RTSP
    SrcFiltersKeys[SRC_RTSP] = FilterKey(_T("SRC_RTSP"), true);
#endif
#if INTERNAL_SOURCEFILTER_RTSP
    SrcFiltersKeys[SRC_UDP] = FilterKey(_T("SRC_UDP"), true);
#endif
#if INTERNAL_SOURCEFILTER_RTP
    SrcFiltersKeys[SRC_RTP] = FilterKey(_T("SRC_RTP"), true);
#endif
#if INTERNAL_SOURCEFILTER_MMS
    SrcFiltersKeys[SRC_MMS] = FilterKey(_T("SRC_MMS"), true);
#endif
#if INTERNAL_SOURCEFILTER_RTMP
    SrcFiltersKeys[SRC_RTMP] = FilterKey(_T("SRC_RTMP"), true);
#endif
#if INTERNAL_SOURCEFILTER_REALMEDIA
    SrcFiltersKeys[SRC_REALMEDIA] = FilterKey(_T("SRC_REALMEDIA"), true);
#endif
#if INTERNAL_SOURCEFILTER_AVI
    SrcFiltersKeys[SRC_AVI] = FilterKey(_T("SRC_AVI"), true);
#endif
#if INTERNAL_SOURCEFILTER_AVS
    SrcFiltersKeys[SRC_AVS] = FilterKey(_T("SRC_AVS"), true);
#endif
#if INTERNAL_SOURCEFILTER_OGG
    SrcFiltersKeys[SRC_OGG] = FilterKey(_T("SRC_OGG"), true);
#endif
#if INTERNAL_SOURCEFILTER_MPEG
    SrcFiltersKeys[SRC_MPEG] = FilterKey(_T("SRC_MPEG"), true);
    SrcFiltersKeys[SRC_MPEGTS] = FilterKey(_T("SRC_MPEGTS"), true);
#endif
#if INTERNAL_SOURCEFILTER_MPEGAUDIO
    SrcFiltersKeys[SRC_MPA] = FilterKey(_T("SRC_MPA"), true);
#endif
#if INTERNAL_SOURCEFILTER_DSM
    SrcFiltersKeys[SRC_DSM] = FilterKey(_T("SRC_DSM"), true);
#endif
    SrcFiltersKeys[SRC_SUBS] = FilterKey(_T("SRC_SUBS"), true);
#if INTERNAL_SOURCEFILTER_MP4
    SrcFiltersKeys[SRC_MP4] = FilterKey(_T("SRC_MP4"), true);
#endif
#if INTERNAL_SOURCEFILTER_FLV
    SrcFiltersKeys[SRC_FLV] = FilterKey(_T("SRC_FLV"), true);
#endif
#if INTERNAL_SOURCEFILTER_GIF
    SrcFiltersKeys[SRC_GIF] = FilterKey(_T("SRC_GIF"), true);
#endif
#if INTERNAL_SOURCEFILTER_ASF
    SrcFiltersKeys[SRC_ASF] = FilterKey(_T("SRC_ASF"), false);
#endif
#if INTERNAL_SOURCEFILTER_WTV
    SrcFiltersKeys[SRC_WTV] = FilterKey(_T("SRC_WTV"), true);
#endif
#if INTERNAL_SOURCEFILTER_FLAC
    SrcFiltersKeys[SRC_FLAC] = FilterKey(_T("SRC_FLAC"), true);
#endif
#if INTERNAL_SOURCEFILTER_RFS
    SrcFiltersKeys[SRC_RFS] = FilterKey(_T("SRC_RFS"), true);
#endif

    // Internal decoders
#if INTERNAL_DECODER_MPEG1
    TraFiltersKeys[TRA_MPEG1] = FilterKey(_T("TRA_MPEG1"), true);
#endif
#if INTERNAL_DECODER_MPEG2
    TraFiltersKeys[TRA_MPEG2] = FilterKey(_T("TRA_MPEG2"), true);
#endif
#if INTERNAL_DECODER_REALVIDEO
    TraFiltersKeys[TRA_RV] = FilterKey(_T("TRA_RV"), true);
#endif
#if INTERNAL_DECODER_REALAUDIO
    TraFiltersKeys[TRA_RA] = FilterKey(_T("TRA_RA"), true);
#endif
#if INTERNAL_DECODER_MPEGAUDIO
    TraFiltersKeys[TRA_MPA] = FilterKey(_T("TRA_MPA"), true);
#endif
#if INTERNAL_DECODER_DTS
    TraFiltersKeys[TRA_DTS] = FilterKey(_T("TRA_DTS"), true);
#endif
#if INTERNAL_DECODER_LPCM
    TraFiltersKeys[TRA_LPCM] = FilterKey(_T("TRA_LPCM"), true);
#endif
#if INTERNAL_DECODER_AC3
    TraFiltersKeys[TRA_AC3] = FilterKey(_T("TRA_AC3"), true);
#endif
#if INTERNAL_DECODER_AAC
    TraFiltersKeys[TRA_AAC] = FilterKey(_T("TRA_AAC"), true);
#endif
#if INTERNAL_DECODER_ALAC
    TraFiltersKeys[TRA_ALAC] = FilterKey(_T("TRA_ALAC"), true);
#endif
#if INTERNAL_DECODER_ALS
    TraFiltersKeys[TRA_ALS] = FilterKey(_T("TRA_ALS"), true);
#endif
#if INTERNAL_DECODER_PS2AUDIO
    TraFiltersKeys[TRA_PS2AUD] = FilterKey(_T("TRA_PS2AUD"), true);
#endif
#if INTERNAL_DECODER_VORBIS
    TraFiltersKeys[TRA_VORBIS] = FilterKey(_T("TRA_VORBIS"), true);
#endif
#if INTERNAL_DECODER_FLAC
    TraFiltersKeys[TRA_FLAC] = FilterKey(_T("TRA_FLAC"), true);
#endif
#if INTERNAL_DECODER_NELLYMOSER
    TraFiltersKeys[TRA_NELLY] = FilterKey(_T("TRA_NELLY"), true);
#endif
#if INTERNAL_DECODER_AMR
    TraFiltersKeys[TRA_AMR] = FilterKey(_T("TRA_AMR"), true);
#endif
#if INTERNAL_DECODER_OPUS
    TraFiltersKeys[TRA_OPUS] = FilterKey(_T("TRA_OPUS"), true);
#endif
#if INTERNAL_DECODER_WMA
    TraFiltersKeys[TRA_WMA] = FilterKey(_T("TRA_WMA"), false);
#endif
#if INTERNAL_DECODER_WMAPRO
    TraFiltersKeys[TRA_WMAPRO] = FilterKey(_T("TRA_WMAPRO"), false);
#endif
#if INTERNAL_DECODER_WMALL
    TraFiltersKeys[TRA_WMALL] = FilterKey(_T("TRA_WMALL"), false);
#endif
#if INTERNAL_DECODER_PCM
    TraFiltersKeys[TRA_PCM] = FilterKey(_T("TRA_PCM"), true);
#endif
#if INTERNAL_DECODER_H264
    TraFiltersKeys[TRA_H264] = FilterKey(_T("TRA_H264"), true);
#endif
#if INTERNAL_DECODER_HEVC
    TraFiltersKeys[TRA_HEVC] = FilterKey(_T("TRA_HEVC"), true);
#endif
#if INTERNAL_DECODER_AV1
    TraFiltersKeys[TRA_AV1] = FilterKey(_T("TRA_AV1"), true);
#endif
#if INTERNAL_DECODER_VC1
    TraFiltersKeys[TRA_VC1] = FilterKey(_T("TRA_VC1"), true);
#endif
#if INTERNAL_DECODER_FLV
    TraFiltersKeys[TRA_FLV4] = FilterKey(_T("TRA_FLV4"), true);
#endif
#if INTERNAL_DECODER_VP356
    TraFiltersKeys[TRA_VP356] = FilterKey(_T("TRA_VP356"), true);
#endif
#if INTERNAL_DECODER_VP8
    TraFiltersKeys[TRA_VP8] = FilterKey(_T("TRA_VP8"), true);
#endif
#if INTERNAL_DECODER_VP9
    TraFiltersKeys[TRA_VP9] = FilterKey(_T("TRA_VP9"), true);
#endif
#if INTERNAL_DECODER_XVID
    TraFiltersKeys[TRA_XVID] = FilterKey(_T("TRA_XVID"), true);
#endif
#if INTERNAL_DECODER_DIVX
    TraFiltersKeys[TRA_DIVX] = FilterKey(_T("TRA_DIVX"), true);
#endif
#if INTERNAL_DECODER_MSMPEG4
    TraFiltersKeys[TRA_MSMPEG4] = FilterKey(_T("TRA_MSMPEG4"), true);
#endif
#if INTERNAL_DECODER_WMV
    TraFiltersKeys[TRA_WMV] = FilterKey(_T("TRA_WMV"), false);
#endif
#if INTERNAL_DECODER_SVQ
    TraFiltersKeys[TRA_SVQ3] = FilterKey(_T("TRA_SVQ3"), true);
#endif
#if INTERNAL_DECODER_H263
    TraFiltersKeys[TRA_H263] = FilterKey(_T("TRA_H263"), true);
#endif
#if INTERNAL_DECODER_THEORA
    TraFiltersKeys[TRA_THEORA] = FilterKey(_T("TRA_THEORA"), true);
#endif
#if INTERNAL_DECODER_AMVV
    TraFiltersKeys[TRA_AMVV] = FilterKey(_T("TRA_AMVV"), true);
#endif
#if INTERNAL_DECODER_MJPEG
    TraFiltersKeys[TRA_MJPEG] = FilterKey(_T("TRA_MJPEG"), true);
#endif
#if INTERNAL_DECODER_INDEO
    TraFiltersKeys[TRA_INDEO] = FilterKey(_T("TRA_INDEO"), true);
#endif
#if INTERNAL_DECODER_SCREEN
    TraFiltersKeys[TRA_SCREEN] = FilterKey(_T("TRA_SCREEN"), true);
#endif
#if INTERNAL_DECODER_FLIC
    TraFiltersKeys[TRA_FLIC] = FilterKey(_T("TRA_FLIC"), true);
#endif
#if INTERNAL_DECODER_MSVIDEO
    TraFiltersKeys[TRA_MSVIDEO] = FilterKey(_T("TRA_MSVIDEO"), true);
#endif
#if INTERNAL_DECODER_V210_V410
    TraFiltersKeys[TRA_V210_V410] = FilterKey(_T("TRA_V210_V410"), false);
#endif

    ZeroMemory(&DVDPosition, sizeof(DVDPosition));

    ENSURE(SUCCEEDED(SaneAudioRenderer::Factory::CreateSettings(&sanear)));
}
#pragma warning(pop)

static constexpr wmcmd_base default_wmcmds[] = {
    { ID_FILE_OPENQUICK,                  'Q', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_MPLAYERC_0 },
    { ID_FILE_OPENMEDIA,                  'O', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_OPEN_FILE },
    { ID_FILE_OPENDVDBD,                  'D', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_OPEN_DVD },
    { ID_FILE_OPENDEVICE,                 'V', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_OPEN_DEVICE },
    { ID_FILE_REOPEN,                     'E', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_REOPEN },
    { ID_FILE_RECYCLE,                      0, FVIRTKEY | FNOINVERT,                    IDS_FILE_RECYCLE },

    { ID_FILE_SAVE_COPY,                    0, FVIRTKEY | FNOINVERT,                    IDS_AG_SAVE_COPY },
    { ID_FILE_SAVE_IMAGE,                 'I', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_SAVE_IMAGE },
    { ID_FILE_SAVE_IMAGE_AUTO,          VK_F5, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_6 },
    { ID_FILE_SAVE_THUMBNAILS,              0, FVIRTKEY | FNOINVERT,                    IDS_FILE_SAVE_THUMBNAILS },

    { ID_FILE_SUBTITLES_LOAD,             'L', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_LOAD_SUBTITLES },
    { ID_FILE_SUBTITLES_SAVE,             'S', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_SAVE_SUBTITLES },
    { ID_FILE_SUBTITLES_DOWNLOAD,         'D', FVIRTKEY | FNOINVERT,                    IDS_SUBTITLES_DOWNLOAD },
    { ID_FILE_SUBTITLES_UPLOAD,           'U', FVIRTKEY | FNOINVERT,                    IDS_SUBTITLES_UPLOAD },
    { ID_FILE_CLOSE_AND_RESTORE,          'C', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_CLOSE },
    { ID_FILE_PROPERTIES,              VK_F10, FVIRTKEY | FSHIFT | FNOINVERT,           IDS_AG_PROPERTIES },
    { ID_FILE_EXIT,                       'X', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_EXIT },
    { ID_PLAY_PLAYPAUSE,             VK_SPACE, FVIRTKEY | FNOINVERT,                    IDS_AG_PLAYPAUSE,   APPCOMMAND_MEDIA_PLAY_PAUSE, wmcmd::LUP, wmcmd::LUP },
    { ID_PLAY_PLAY,                         0, FVIRTKEY | FNOINVERT,                    IDS_AG_PLAY,        APPCOMMAND_MEDIA_PLAY },
    { ID_PLAY_PAUSE,                        0, FVIRTKEY | FNOINVERT,                    IDS_AG_PAUSE,       APPCOMMAND_MEDIA_PAUSE },
    { ID_PLAY_STOP,             VK_OEM_PERIOD, FVIRTKEY | FNOINVERT,                    IDS_AG_STOP,        APPCOMMAND_MEDIA_STOP },
    { ID_PLAY_FRAMESTEP,             VK_RIGHT, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_FRAMESTEP },
    { ID_PLAY_FRAMESTEPCANCEL,        VK_LEFT, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_MPLAYERC_16 },
    { ID_NAVIGATE_GOTO,                   'G', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_GO_TO },
    { ID_PLAY_INCRATE,                  VK_UP, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_INCREASE_RATE },
    { ID_PLAY_DECRATE,                VK_DOWN, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_DECREASE_RATE },
    { ID_PLAY_RESETRATE,                  'R', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_RESET_RATE },
    { ID_PLAY_INCAUDDELAY,             VK_ADD, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_21 },
    { ID_PLAY_DECAUDDELAY,        VK_SUBTRACT, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_22 },
    { ID_PLAY_SEEKFORWARDSMALL,             0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_23 },
    { ID_PLAY_SEEKBACKWARDSMALL,            0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_24 },
    { ID_PLAY_SEEKFORWARDMED,        VK_RIGHT, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_25 },
    { ID_PLAY_SEEKBACKWARDMED,        VK_LEFT, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_26 },
    { ID_PLAY_SEEKFORWARDLARGE,             0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_27 },
    { ID_PLAY_SEEKBACKWARDLARGE,            0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_28 },
    { ID_PLAY_SEEKKEYFORWARD,        VK_RIGHT, FVIRTKEY | FSHIFT | FNOINVERT,           IDS_MPLAYERC_29 },
    { ID_PLAY_SEEKKEYBACKWARD,        VK_LEFT, FVIRTKEY | FSHIFT | FNOINVERT,           IDS_MPLAYERC_30 },
    { ID_PLAY_SEEKSET,                VK_HOME, FVIRTKEY | FNOINVERT,                    IDS_AG_SEEKSET },
    { ID_PLAY_REPEAT_FOREVER,               0, FVIRTKEY | FNOINVERT,                    IDS_PLAYLOOP_FOREVER },
    { ID_PLAY_REPEAT_ONEFILE,               0, FVIRTKEY | FNOINVERT,                    IDS_PLAYLOOPMODE_FILE },
    { ID_PLAY_REPEAT_WHOLEPLAYLIST,         0, FVIRTKEY | FNOINVERT,                    IDS_PLAYLOOPMODE_PLAYLIST },
    { ID_NAVIGATE_SKIPFORWARD,        VK_NEXT, FVIRTKEY | FNOINVERT,                    IDS_AG_NEXT,        APPCOMMAND_MEDIA_NEXTTRACK, wmcmd::X2DOWN, wmcmd::X2DOWN },
    { ID_NAVIGATE_SKIPBACK,          VK_PRIOR, FVIRTKEY | FNOINVERT,                    IDS_AG_PREVIOUS,    APPCOMMAND_MEDIA_PREVIOUSTRACK, wmcmd::X1DOWN, wmcmd::X1DOWN },
    { ID_NAVIGATE_SKIPFORWARDFILE,    VK_NEXT, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_NEXT_FILE },
    { ID_NAVIGATE_SKIPBACKFILE,      VK_PRIOR, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PREVIOUS_FILE },
    { ID_NAVIGATE_TUNERSCAN,              'T', FVIRTKEY | FSHIFT | FNOINVERT,           IDS_NAVIGATE_TUNERSCAN },
    { ID_FAVORITES_QUICKADDFAVORITE,      'Q', FVIRTKEY | FSHIFT | FNOINVERT,           IDS_FAVORITES_QUICKADDFAVORITE },
    { ID_VIEW_CAPTIONMENU,                '0', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_CAPTION },
    { ID_VIEW_SEEKER,                     '1', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_SEEKER },
    { ID_VIEW_CONTROLS,                   '2', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_CONTROLS },
    { ID_VIEW_INFORMATION,                '3', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_INFO },
    { ID_VIEW_STATISTICS,                 '4', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_STATS },
    { ID_VIEW_STATUS,                     '5', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_STATUS },
    { ID_VIEW_SUBRESYNC,                  '6', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_SUBRESYNC },
    { ID_VIEW_PLAYLIST,                   '7', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_PLAYLIST },
    { ID_VIEW_CAPTURE,                    '8', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_CAPTURE },
    { ID_VIEW_NAVIGATION,                 '9', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TOGGLE_NAVIGATION },
    { ID_VIEW_DEBUGSHADERS,                 0, FVIRTKEY | FNOINVERT,                    IDS_AG_TOGGLE_DEBUGSHADERS },
    { ID_VIEW_MPCTHEME,                     0, FVIRTKEY | FNOINVERT,                    IDS_AG_TOGGLE_MPCTHEME },
    { ID_VIEW_PRESETS_MINIMAL,            '1', FVIRTKEY | FNOINVERT,                    IDS_AG_VIEW_MINIMAL },
    { ID_VIEW_PRESETS_COMPACT,            '2', FVIRTKEY | FNOINVERT,                    IDS_AG_VIEW_COMPACT },
    { ID_VIEW_PRESETS_NORMAL,             '3', FVIRTKEY | FNOINVERT,                    IDS_AG_VIEW_NORMAL },
    { ID_VIEW_FULLSCREEN,           VK_RETURN, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_FULLSCREEN, 0, wmcmd::LDBLCLK, wmcmd::LDBLCLK },
    { ID_VIEW_FULLSCREEN_SECONDARY,    VK_F11, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_39 },
    { ID_VIEW_ZOOM_50,                    '1', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_ZOOM_50 },
    { ID_VIEW_ZOOM_100,                   '2', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_ZOOM_100 },
    { ID_VIEW_ZOOM_200,                   '3', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_ZOOM_200 },
    { ID_VIEW_ZOOM_AUTOFIT,               '4', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_ZOOM_AUTO_FIT },
    { ID_VIEW_ZOOM_AUTOFIT_LARGER,        '5', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_ZOOM_AUTO_FIT_LARGER },
    { ID_ASPECTRATIO_NEXT,                  0, FVIRTKEY | FNOINVERT,                    IDS_AG_NEXT_AR_PRESET },
    { ID_VIEW_VF_HALF,                      0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_HALF },
    { ID_VIEW_VF_NORMAL,                    0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_NORMAL },
    { ID_VIEW_VF_DOUBLE,                    0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_DOUBLE },
    { ID_VIEW_VF_STRETCH,                   0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_STRETCH },
    { ID_VIEW_VF_FROMINSIDE,                0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_INSIDE },
    { ID_VIEW_VF_ZOOM1,                     0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_ZOOM1 },
    { ID_VIEW_VF_ZOOM2,                     0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_ZOOM2 },
    { ID_VIEW_VF_FROMOUTSIDE,               0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_OUTSIDE },
    { ID_VIEW_VF_SWITCHZOOM,                0, FVIRTKEY | FNOINVERT,                    IDS_AG_VIDFRM_SWITCHZOOM },
    { ID_ONTOP_ALWAYS,                    'A', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_ALWAYS_ON_TOP },
    { ID_VIEW_RESET,               VK_NUMPAD5, FVIRTKEY | FNOINVERT,                    IDS_AG_PNS_RESET },
    { ID_VIEW_INCSIZE,             VK_NUMPAD9, FVIRTKEY | FNOINVERT,                    IDS_AG_PNS_INC_SIZE },
    { ID_VIEW_INCWIDTH,            VK_NUMPAD6, FVIRTKEY | FNOINVERT,                    IDS_AG_PNS_INC_WIDTH },
    { ID_VIEW_INCHEIGHT,           VK_NUMPAD8, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_47 },
    { ID_VIEW_DECSIZE,             VK_NUMPAD1, FVIRTKEY | FNOINVERT,                    IDS_AG_PNS_DEC_SIZE },
    { ID_VIEW_DECWIDTH,            VK_NUMPAD4, FVIRTKEY | FNOINVERT,                    IDS_AG_PNS_DEC_WIDTH },
    { ID_VIEW_DECHEIGHT,           VK_NUMPAD2, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_50 },
    { ID_PANSCAN_CENTER,           VK_NUMPAD5, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_CENTER },
    { ID_PANSCAN_MOVELEFT,         VK_NUMPAD4, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_LEFT },
    { ID_PANSCAN_MOVERIGHT,        VK_NUMPAD6, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_RIGHT },
    { ID_PANSCAN_MOVEUP,           VK_NUMPAD8, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_UP },
    { ID_PANSCAN_MOVEDOWN,         VK_NUMPAD2, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_DOWN },
    { ID_PANSCAN_MOVEUPLEFT,       VK_NUMPAD7, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_UPLEFT },
    { ID_PANSCAN_MOVEUPRIGHT,      VK_NUMPAD9, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_UPRIGHT },
    { ID_PANSCAN_MOVEDOWNLEFT,     VK_NUMPAD1, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_PNS_DOWNLEFT },
    { ID_PANSCAN_MOVEDOWNRIGHT,    VK_NUMPAD3, FVIRTKEY | FCONTROL | FNOINVERT,         IDS_MPLAYERC_59 },
    { ID_PANSCAN_ROTATEXP,         VK_NUMPAD8, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_PNS_ROTATEX_P },
    { ID_PANSCAN_ROTATEXM,         VK_NUMPAD2, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_PNS_ROTATEX_M },
    { ID_PANSCAN_ROTATEYP,         VK_NUMPAD4, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_PNS_ROTATEY_P },
    { ID_PANSCAN_ROTATEYM,         VK_NUMPAD6, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_PNS_ROTATEY_M },
    { ID_PANSCAN_ROTATEZP,         VK_NUMPAD1, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_PNS_ROTATEZ_P },
    { ID_PANSCAN_ROTATEZM,         VK_NUMPAD3, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_PNS_ROTATEZ_M },
    { ID_VOLUME_UP,                     VK_UP, FVIRTKEY | FNOINVERT,                    IDS_AG_VOLUME_UP,   0, wmcmd::WUP, wmcmd::WUP },
    { ID_VOLUME_DOWN,                 VK_DOWN, FVIRTKEY | FNOINVERT,                    IDS_AG_VOLUME_DOWN, 0, wmcmd::WDOWN, wmcmd::WDOWN },
    { ID_VOLUME_MUTE,                     'M', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_VOLUME_MUTE, 0 },
    { ID_VOLUME_BOOST_INC,                  0, FVIRTKEY | FNOINVERT,                    IDS_VOLUME_BOOST_INC },
    { ID_VOLUME_BOOST_DEC,                  0, FVIRTKEY | FNOINVERT,                    IDS_VOLUME_BOOST_DEC },
    { ID_VOLUME_BOOST_MIN,                  0, FVIRTKEY | FNOINVERT,                    IDS_VOLUME_BOOST_MIN },
    { ID_VOLUME_BOOST_MAX,                  0, FVIRTKEY | FNOINVERT,                    IDS_VOLUME_BOOST_MAX },
    { ID_CUSTOM_CHANNEL_MAPPING,            0, FVIRTKEY | FNOINVERT,                    IDS_CUSTOM_CHANNEL_MAPPING },
    { ID_NORMALIZE,                         0, FVIRTKEY | FNOINVERT,                    IDS_NORMALIZE },
    { ID_REGAIN_VOLUME,                     0, FVIRTKEY | FNOINVERT,                    IDS_REGAIN_VOLUME },
    { ID_COLOR_BRIGHTNESS_INC,              0, FVIRTKEY | FNOINVERT,                    IDS_BRIGHTNESS_INC },
    { ID_COLOR_BRIGHTNESS_DEC,              0, FVIRTKEY | FNOINVERT,                    IDS_BRIGHTNESS_DEC },
    { ID_COLOR_CONTRAST_INC,                0, FVIRTKEY | FNOINVERT,                    IDS_CONTRAST_INC },
    { ID_COLOR_CONTRAST_DEC,                0, FVIRTKEY | FNOINVERT,                    IDS_CONTRAST_DEC },
    { ID_COLOR_HUE_INC,                     0, FVIRTKEY | FNOINVERT,                    IDS_HUE_INC },
    { ID_COLOR_HUE_DEC,                     0, FVIRTKEY | FNOINVERT,                    IDS_HUE_DEC },
    { ID_COLOR_SATURATION_INC,              0, FVIRTKEY | FNOINVERT,                    IDS_SATURATION_INC },
    { ID_COLOR_SATURATION_DEC,              0, FVIRTKEY | FNOINVERT,                    IDS_SATURATION_DEC },
    { ID_COLOR_RESET,                       0, FVIRTKEY | FNOINVERT,                    IDS_RESET_COLOR },
    { ID_NAVIGATE_TITLEMENU,              'T', FVIRTKEY | FALT | FNOINVERT,             IDS_MPLAYERC_63 },
    { ID_NAVIGATE_ROOTMENU,               'R', FVIRTKEY | FALT | FNOINVERT,             IDS_AG_DVD_ROOT_MENU },
    { ID_NAVIGATE_SUBPICTUREMENU,           0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_65 },
    { ID_NAVIGATE_AUDIOMENU,                0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_66 },
    { ID_NAVIGATE_ANGLEMENU,                0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_67 },
    { ID_NAVIGATE_CHAPTERMENU,              0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_68 },
    { ID_NAVIGATE_MENU_LEFT,          VK_LEFT, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_DVD_MENU_LEFT },
    { ID_NAVIGATE_MENU_RIGHT,        VK_RIGHT, FVIRTKEY | FALT | FNOINVERT,             IDS_MPLAYERC_70 },
    { ID_NAVIGATE_MENU_UP,              VK_UP, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_DVD_MENU_UP },
    { ID_NAVIGATE_MENU_DOWN,          VK_DOWN, FVIRTKEY | FALT | FNOINVERT,             IDS_AG_DVD_MENU_DOWN },
    { ID_NAVIGATE_MENU_ACTIVATE,            0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_73 },
    { ID_NAVIGATE_MENU_BACK,                0, FVIRTKEY | FNOINVERT,                    IDS_AG_DVD_MENU_BACK },
    { ID_NAVIGATE_MENU_LEAVE,               0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_75 },
    { ID_BOSS,                            'B', FVIRTKEY | FNOINVERT,                    IDS_AG_BOSS_KEY },
    { ID_MENU_PLAYER_SHORT,           VK_APPS, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_77, 0, wmcmd::RUP, wmcmd::RUP },
    { ID_MENU_PLAYER_LONG,                  0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_78 },
    { ID_MENU_FILTERS,                      0, FVIRTKEY | FNOINVERT,                    IDS_AG_FILTERS_MENU },
    { ID_VIEW_OPTIONS,                    'O', FVIRTKEY | FNOINVERT,                    IDS_AG_OPTIONS },
    { ID_STREAM_AUDIO_NEXT,               'A', FVIRTKEY | FNOINVERT,                    IDS_AG_NEXT_AUDIO },
    { ID_STREAM_AUDIO_PREV,               'A', FVIRTKEY | FSHIFT | FNOINVERT,           IDS_AG_PREV_AUDIO },
    { ID_STREAM_SUB_NEXT,                 'S', FVIRTKEY | FNOINVERT,                    IDS_AG_NEXT_SUBTITLE },
    { ID_STREAM_SUB_PREV,                 'S', FVIRTKEY | FSHIFT | FNOINVERT,           IDS_AG_PREV_SUBTITLE },
    { ID_STREAM_SUB_ONOFF,                'W', FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_85 },
    { ID_SUBTITLES_SUBITEM_START + 2,       0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_86 },
    { ID_DVD_ANGLE_NEXT,                    0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_91 },
    { ID_DVD_ANGLE_PREV,                    0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_92 },
    { ID_DVD_AUDIO_NEXT,                    0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_93 },
    { ID_DVD_AUDIO_PREV,                    0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_94 },
    { ID_DVD_SUB_NEXT,                      0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_95 },
    { ID_DVD_SUB_PREV,                      0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_96 },
    { ID_DVD_SUB_ONOFF,                     0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_97 },
    { ID_VIEW_TEARING_TEST,               'T', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_AG_TEARING_TEST },
    { ID_VIEW_OSD_DISPLAY_TIME,           'I', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_OSD_DISPLAY_CURRENT_TIME },
    { ID_VIEW_OSD_SHOW_FILENAME,          'N', FVIRTKEY | FNOINVERT,                    IDS_OSD_SHOW_FILENAME },
    { ID_SHADERS_PRESET_NEXT,               0, FVIRTKEY | FNOINVERT,                    IDS_AG_SHADERS_PRESET_NEXT },
    { ID_SHADERS_PRESET_PREV,               0, FVIRTKEY | FNOINVERT,                    IDS_AG_SHADERS_PRESET_PREV },
    { ID_D3DFULLSCREEN_TOGGLE,              0, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_99 },
    { ID_GOTO_PREV_SUB,                   'Y', FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_100 },
    { ID_GOTO_NEXT_SUB,                   'U', FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_101 },
    { ID_SHIFT_SUB_DOWN,              VK_NEXT, FVIRTKEY | FALT | FNOINVERT,             IDS_MPLAYERC_102 },
    { ID_SHIFT_SUB_UP,               VK_PRIOR, FVIRTKEY | FALT | FNOINVERT,             IDS_MPLAYERC_103 },
    { ID_VIEW_DISPLAY_RENDERER_STATS,     'J', FVIRTKEY | FCONTROL | FNOINVERT,         IDS_OSD_DISPLAY_RENDERER_STATS },
    { ID_VIEW_RESET_RENDERER_STATS,       'R', FVIRTKEY | FCONTROL | FALT | FNOINVERT,  IDS_OSD_RESET_RENDERER_STATS },
    { ID_VIEW_VSYNC,                      'V', FVIRTKEY | FNOINVERT,                    IDS_AG_VSYNC },
    { ID_VIEW_ENABLEFRAMETIMECORRECTION,    0, FVIRTKEY | FNOINVERT,                    IDS_AG_ENABLEFRAMETIMECORRECTION },
    { ID_VIEW_VSYNCACCURATE,              'V', FVIRTKEY | FCONTROL | FALT | FNOINVERT,  IDS_AG_VSYNCACCURATE },
    { ID_VIEW_VSYNCOFFSET_DECREASE,     VK_UP, FVIRTKEY | FCONTROL | FALT | FNOINVERT,  IDS_AG_VSYNCOFFSET_DECREASE },
    { ID_VIEW_VSYNCOFFSET_INCREASE,   VK_DOWN, FVIRTKEY | FCONTROL | FALT | FNOINVERT,  IDS_AG_VSYNCOFFSET_INCREASE },
    { ID_SUB_DELAY_DOWN,                VK_F1, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_104 },
    { ID_SUB_DELAY_UP,                  VK_F2, FVIRTKEY | FNOINVERT,                    IDS_MPLAYERC_105 },

    { ID_AFTERPLAYBACK_DONOTHING,           0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_DONOTHING },
    { ID_AFTERPLAYBACK_PLAYNEXT,            0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_PLAYNEXT },
    { ID_AFTERPLAYBACK_MONITOROFF,          0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_MONITOROFF },
    { ID_AFTERPLAYBACK_EXIT,                0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_EXIT },
    { ID_AFTERPLAYBACK_STANDBY,             0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_STANDBY },
    { ID_AFTERPLAYBACK_HIBERNATE,           0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_HIBERNATE },
    { ID_AFTERPLAYBACK_SHUTDOWN,            0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_SHUTDOWN },
    { ID_AFTERPLAYBACK_LOGOFF,              0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_LOGOFF },
    { ID_AFTERPLAYBACK_LOCK,                0, FVIRTKEY | FNOINVERT,                    IDS_AFTERPLAYBACK_LOCK },

    { ID_VIEW_EDITLISTEDITOR,               0, FVIRTKEY | FNOINVERT,                    IDS_AG_TOGGLE_EDITLISTEDITOR },
    { ID_EDL_IN,                            0, FVIRTKEY | FNOINVERT,                    IDS_AG_EDL_IN },
    { ID_EDL_OUT,                           0, FVIRTKEY | FNOINVERT,                    IDS_AG_EDL_OUT },
    { ID_EDL_NEWCLIP,                       0, FVIRTKEY | FNOINVERT,                    IDS_AG_EDL_NEW_CLIP },
    { ID_EDL_SAVE,                          0, FVIRTKEY | FNOINVERT,                    IDS_AG_EDL_SAVE }
};

void CAppSettings::CreateCommands()
{
    for (const auto& wc : default_wmcmds) {
        wmcmds.AddTail(wmcmd(wc));
    }
}

CAppSettings::~CAppSettings()
{
    if (hAccel) {
        DestroyAcceleratorTable(hAccel);
    }
}

bool CAppSettings::IsD3DFullscreen() const
{
    if (iDSVideoRendererType == VIDRNDT_DS_VMR9RENDERLESS ||
            iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM ||
            iDSVideoRendererType == VIDRNDT_DS_SYNC) {
        return fD3DFullscreen || (nCLSwitches & CLSW_D3DFULLSCREEN);
    } else {
        return false;
    }
}

bool CAppSettings::IsISRAutoLoadEnabled() const
{
    return eSubtitleRenderer == SubtitleRenderer::INTERNAL &&
           IsSubtitleRendererSupported(eSubtitleRenderer, iDSVideoRendererType);
}

CAppSettings::SubtitleRenderer CAppSettings::GetSubtitleRenderer() const
{
    if (IsSubtitleRendererSupported(SubtitleRenderer::INTERNAL, iDSVideoRendererType) ||
            IsSubtitleRendererSupported(SubtitleRenderer::XY_SUB_FILTER, iDSVideoRendererType) ||
            IsSubtitleRendererSupported(SubtitleRenderer::ASS_FILTER, iDSVideoRendererType)) {
        return eSubtitleRenderer;
    }
    return SubtitleRenderer::VS_FILTER;
}

bool CAppSettings::IsSubtitleRendererRegistered(SubtitleRenderer eSubtitleRenderer)
{
    switch (eSubtitleRenderer) {
        case SubtitleRenderer::INTERNAL:
            return true;
        case SubtitleRenderer::VS_FILTER:
            return IsCLSIDRegistered(CLSID_VSFilter);
        case SubtitleRenderer::XY_SUB_FILTER:
            return IsCLSIDRegistered(CLSID_XySubFilter);
        case SubtitleRenderer::ASS_FILTER:
            return IsCLSIDRegistered(CLSID_AssFilter);
        default:
            ASSERT(FALSE);
            return false;
    }
}

bool CAppSettings::IsSubtitleRendererSupported(SubtitleRenderer eSubtitleRenderer, int videoRenderer)
{
    switch (eSubtitleRenderer) {
        case SubtitleRenderer::INTERNAL:
            switch (videoRenderer) {
                case VIDRNDT_DS_VMR9RENDERLESS:
                case VIDRNDT_DS_EVR_CUSTOM:
                case VIDRNDT_DS_DXR:
                case VIDRNDT_DS_SYNC:
                case VIDRNDT_DS_MADVR:
                case VIDRNDT_DS_MPCVR:
                    return true;
            }
            break;

        case SubtitleRenderer::VS_FILTER:
            return true;

        case SubtitleRenderer::XY_SUB_FILTER:
        case SubtitleRenderer::ASS_FILTER:
            switch (videoRenderer) {
                case VIDRNDT_DS_VMR9RENDERLESS:
                case VIDRNDT_DS_EVR_CUSTOM:
                case VIDRNDT_DS_SYNC:
                case VIDRNDT_DS_MADVR:
                    return true;
            }
            break;

        default:
            ASSERT(FALSE);
    }

    return false;
}

bool CAppSettings::IsVideoRendererAvailable(int iVideoRendererType)
{
    switch (iVideoRendererType) {
        case VIDRNDT_DS_DXR:
            return IsCLSIDRegistered(CLSID_DXR);
        case VIDRNDT_DS_EVR:
        case VIDRNDT_DS_EVR_CUSTOM:
        case VIDRNDT_DS_SYNC:
            return IsCLSIDRegistered(CLSID_EnhancedVideoRenderer);
        case VIDRNDT_DS_MADVR:
            return IsCLSIDRegistered(CLSID_madVR);
        case VIDRNDT_DS_MPCVR:
            return IsCLSIDRegistered(CLSID_MPCVR);
        default:
            return true;
    }
}

bool CAppSettings::IsInitialized() const
{
    return bInitialized;
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

    if (!bInitialized) {
        return;
    }

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECAPTIONMENU, eCaptionMenuMode);
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
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOOPMODE, static_cast<int>(eLoopMode));
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ZOOM, iZoomLevel);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MULTIINST, fAllowMultipleInst);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTSTYLE, iTitleBarTextStyle);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTTITLE, fTitleBarTextTitle);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ONTOP, iOnTop);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TRAYICON, fTrayIcon);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOZOOM, fRememberZoomLevel);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOFITFACTOR, nAutoFitFactor);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AFTER_PLAYBACK, static_cast<int>(eAfterPlayback));

    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDE_FULLSCREEN_CONTROLS, bHideFullscreenControls));
    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDE_FULLSCREEN_CONTROLS_POLICY,
                                 static_cast<int>(eHideFullscreenControlsPolicy)));
    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDE_FULLSCREEN_CONTROLS_DELAY, uHideFullscreenControlsDelay));
    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDE_FULLSCREEN_DOCKED_PANELS, bHideFullscreenDockedPanels));
    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDE_WINDOWED_CONTROLS, bHideWindowedControls));

    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDE_WINDOWED_MOUSE_POINTER, bHideWindowedMousePointer));

    // Auto-change fullscreen mode
    SaveSettingsAutoChangeFullScreenMode();

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_EXITFULLSCREENATTHEEND, fExitFullScreenAtTheEnd);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWPOS, fRememberWindowPos);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWSIZE, fRememberWindowSize);
    if (fRememberWindowSize || fRememberWindowPos) {
        pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_LASTWINDOWRECT, (BYTE*)&rcLastWindowPos, sizeof(rcLastWindowPos));
        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTWINDOWTYPE, nLastWindowType);
    }
    if (fSavePnSZoom) {
        CString str;
        str.Format(_T("%.3f,%.3f"), dZoomX, dZoomY);
        pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_PANSCANZOOM, str);
    } else {
        pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_PANSCANZOOM, nullptr);
    }
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SNAPTODESKTOPEDGES, fSnapToDesktopEdges);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_X, sizeAspectRatio.cx);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_Y, sizeAspectRatio.cy);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPHISTORY, fKeepHistory);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_RECENT_FILES_NUMBER, iRecentFilesNumber);
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
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CLOSEDCAPTIONS, fClosedCaptions);
    CString style;
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SPSTYLE, style <<= subtitlesDefStyle);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPOVERRIDEPLACEMENT, fOverridePlacement);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPHORPOS, nHorPos);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPVERPOS, nVerPos);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLEARCOMPENSATION, bSubtitleARCompensation);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBDELAYINTERVAL, nSubDelayStep);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLESUBTITLES, fEnableSubtitles);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PREFER_FORCED_DEFAULT_SUBTITLES, bPreferDefaultForcedSubtitles);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITIZEEXTERNALSUBTITLES, fPrioritizeExternalSubtitles);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEINTERNALSUBTITLES, fDisableInternalSubtitles);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ALLOW_OVERRIDING_EXT_SPLITTER, bAllowOverridingExternalSplitterChoice);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTODOWNLOADSUBTITLES, bAutoDownloadSubtitles);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTODOWNLOADSCOREMOVIES, nAutoDownloadScoreMovies);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTODOWNLOADSCORESERIES, nAutoDownloadScoreSeries);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_AUTODOWNLOADSUBTITLESEXCLUDE, strAutoDownloadSubtitlesExclude);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOUPLOADSUBTITLES, bAutoUploadSubtitles);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PREFERHEARINGIMPAIREDSUBTITLES, bPreferHearingImpairedSubtitles);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPCTHEME, bMPCTheme);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MODERNSEEKBAR, bModernSeekbar);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MODERNSEEKBARHEIGHT, iModernSeekbarHeight);

    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLESPROVIDERS, strSubtitlesProviders);

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
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PREVENT_MINIMIZE, fPreventMinimize);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENHANCED_TASKBAR, bUseEnhancedTaskBar);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SEARCH_IN_FOLDER, fUseSearchInFolder);
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

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_BRIGHTNESS, iBrightness);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_CONTRAST, iContrast);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_HUE, iHue);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_SATURATION, iSaturation);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SHOWOSD, fShowOSD);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEEDLEDITOR, fEnableEDLEditor);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LANGUAGE, language);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FASTSEEK, bFastSeek);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FASTSEEK_METHOD, eFastSeekMethod);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SHOW_CHAPTERS, fShowChapters);


    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LCD_SUPPORT, fLCDSupport);

    // Save analog capture settings
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULT_CAPTURE, iDefaultCaptureDevice);
    pApp->WriteProfileString(IDS_R_CAPTURE, IDS_RS_VIDEO_DISP_NAME, strAnalogVideo);
    pApp->WriteProfileString(IDS_R_CAPTURE, IDS_RS_AUDIO_DISP_NAME, strAnalogAudio);
    pApp->WriteProfileInt(IDS_R_CAPTURE, IDS_RS_COUNTRY, iAnalogCountry);

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
    pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_DVB_REBUILD_FG, nDVBRebuildFilterGraph);
    pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_DVB_STOP_FG, nDVBStopFilterGraph);

    for (size_t i = 0; i < m_DVBChannels.size(); i++) {
        CString numChannel;
        numChannel.Format(_T("%Iu"), i);
        pApp->WriteProfileString(IDS_R_DVB, numChannel, m_DVBChannels[i].ToString());
    }

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DVDPOS, fRememberDVDPos);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOS, fRememberFilePos);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOSLONGER, iRememberPosForLongerThan);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOSAUDIO, bRememberPosForAudioFiles);
    if (fKeepHistory) {
        if (fRememberFilePos) {
            filePositions.Save();
        }
        if (fRememberDVDPos) {
            dvdPositions.Save();
        }
    }

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTFULLSCREEN, fLastFullScreen);
    // CASIMIR666 : end of new settings

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_INTREALMEDIA, fIntRealMedia);

    pApp->WriteProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, nullptr, nullptr);
    for (INT_PTR i = 0, j = m_pnspresets.GetCount(); i < j; i++) {
        CString str;
        str.Format(_T("Preset%Id"), i);
        pApp->WriteProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, str, m_pnspresets[i]);
    }

    pApp->WriteProfileString(IDS_R_COMMANDS, nullptr, nullptr);
    POSITION pos = wmcmds.GetHeadPosition();
    for (int i = 0; pos;) {
        const wmcmd& wc = wmcmds.GetNext(pos);
        if (wc.IsModified()) {
            CString str;
            str.Format(_T("CommandMod%d"), i);
            CString str2;
            str2.Format(_T("%hu %hx %hx \"%S\" %d %hhu %u %hhu"),
                        wc.cmd, (WORD)wc.fVirt, wc.key, wc.rmcmd.GetString(),
                        wc.rmrepcnt, wc.mouse, wc.appcmd, wc.mouseFS);
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
        pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, SrcFiltersKeys[f].name, SrcFilters[f]);
    }
    for (int f = 0; f < TRA_LAST; f++) {
        pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, TraFiltersKeys[f].name, TraFilters[f]);
    }

    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_LOGOFILE, strLogoFileName);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOID, nLogoId);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOEXT, fLogoExternal);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECDROMSSUBMENU, fHideCDROMsSubMenu);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITY, dwPriority);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LAUNCHFULLSCREEN, fLaunchfullscreen);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWEBSERVER, fEnableWebServer);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPORT, nWebServerPort);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERUSECOMPRESSION, fWebServerUseCompression);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERLOCALHOSTONLY, fWebServerLocalhostOnly);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBUI_ENABLE_PREVIEW, bWebUIEnablePreview);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPRINTDEBUGINFO, fWebServerPrintDebugInfo);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBROOT, strWebRoot);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBDEFINDEX, strWebDefIndex);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBSERVERCGI, strWebServerCGI);

    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTPATH, strSnapshotPath);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTEXT, strSnapshotExt);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBROWS, iThumbRows);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBCOLS, iThumbCols);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBWIDTH, iThumbWidth);

    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBSAVEEXTERNALSTYLEFILE, bSubSaveExternalStyleFile));
    {
        // Save the list of extra (non-default) shader files
        VERIFY(pApp->WriteProfileString(IDS_R_SHADERS, IDS_RS_SHADERS_EXTRA, m_ShadersExtraList.ToString()));
        // Save shader selection
        CString strPre, strPost;
        m_Shaders.GetCurrentPreset().ToStrings(strPre, strPost);
        VERIFY(pApp->WriteProfileString(IDS_R_SHADERS, IDS_RS_SHADERS_PRERESIZE, strPre));
        VERIFY(pApp->WriteProfileString(IDS_R_SHADERS, IDS_RS_SHADERS_POSTRESIZE, strPost));
        // Save shader presets
        int i = 0;
        CString iStr;
        pApp->WriteProfileString(IDS_R_SHADER_PRESETS, nullptr, nullptr);
        for (const auto& pair : m_Shaders.GetPresets()) {
            iStr.Format(_T("%d"), i++);
            pair.second.ToStrings(strPre, strPost);
            VERIFY(pApp->WriteProfileString(IDS_R_SHADER_PRESETS, iStr, pair.first));
            VERIFY(pApp->WriteProfileString(IDS_R_SHADER_PRESETS, IDS_RS_SHADERS_PRERESIZE + iStr, strPre));
            VERIFY(pApp->WriteProfileString(IDS_R_SHADER_PRESETS, IDS_RS_SHADERS_POSTRESIZE + iStr, strPost));
        }
        // Save selected preset name
        CString name;
        m_Shaders.GetCurrentPresetName(name);
        VERIFY(pApp->WriteProfileString(IDS_R_SHADERS, IDS_RS_SHADERS_LASTPRESET, name));
    }

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMAINING_TIME, fRemainingTime);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIGH_PRECISION_TIMER, bHighPrecisionTimer);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_UPDATER_AUTO_CHECK, nUpdaterAutoCheck);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_UPDATER_DELAY, nUpdaterDelay);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_NOTIFY_SKYPE, bNotifySkype);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JPEG_QUALITY, nJpegQuality);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COVER_ART, bEnableCoverArt);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COVER_ART_SIZE_LIMIT, nCoverArtSizeLimit);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOGGING, bEnableLogging);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USE_LEGACY_TOOLBAR, bUseLegacyToolbar);

    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLE_RENDERER,
                                 static_cast<int>(eSubtitleRenderer)));

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULTTOOLBARSIZE, nDefaultToolbarSize);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SAVEIMAGE_POSITION, bSaveImagePosition);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SAVEIMAGE_CURRENTTIME, bSaveImageCurrentTime);

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ALLOW_INACCURATE_FASTSEEK, bAllowInaccurateFastseek);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOOP_FOLDER_NEXT_FILE, bLoopFolderOnPlayNextFile);

    {
        CComHeapPtr<WCHAR> pDeviceId;
        BOOL bExclusive;
        UINT32 uBufferDuration;
        if (SUCCEEDED(sanear->GetOuputDevice(&pDeviceId, &bExclusive, &uBufferDuration))) {
            pApp->WriteProfileString(IDS_R_SANEAR, IDS_RS_SANEAR_DEVICE_ID, pDeviceId);
            pApp->WriteProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_DEVICE_EXCLUSIVE, bExclusive);
            pApp->WriteProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_DEVICE_BUFFER, uBufferDuration);
        }

        BOOL bAllowBitstreaming = sanear->GetAllowBitstreaming();
        pApp->WriteProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_ALLOW_BITSTREAMING, bAllowBitstreaming);

        BOOL bCrossfeedEnabled = sanear->GetCrossfeedEnabled();
        pApp->WriteProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_CROSSFEED_ENABLED, bCrossfeedEnabled);

        UINT32 uCutoffFrequency, uCrossfeedLevel;
        sanear->GetCrossfeedSettings(&uCutoffFrequency, &uCrossfeedLevel);
        pApp->WriteProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_CROSSFEED_CUTOFF_FREQ, uCutoffFrequency);
        pApp->WriteProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_CROSSFEED_LEVEL, uCrossfeedLevel);

        BOOL bIgnoreSystemChannelMixer = sanear->GetIgnoreSystemChannelMixer();
        pApp->WriteProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_IGNORE_SYSTEM_MIXER, bIgnoreSystemChannelMixer);
    }

    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USE_YDL, bUseYDL);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_YDL_MAX_HEIGHT, iYDLMaxHeight);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_YDL_VIDEO_FORMAT, iYDLVideoFormat);
    pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_YDL_AUDIO_ONLY, bYDLAudioOnly);
    pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_YDL_COMMAND_LINE, sYDLCommandLine);

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
            f->dispname = CStringW(pApp->GetProfileString(key, _T("DisplayName")));
            f->name = pApp->GetProfileString(key, _T("Name"));
        } else if (j == 1) {
            f->type = FilterOverride::EXTERNAL;
            f->path = pApp->GetProfileString(key, _T("Path"));
            f->name = pApp->GetProfileString(key, _T("Name"));
            f->clsid = GUIDFromCString(pApp->GetProfileString(key, _T("CLSID")));
        } else {
            pApp->WriteProfileString(key, nullptr, 0);
            break;
        }

        f->backup.RemoveAll();
        for (unsigned int k = 0; ; k++) {
            CString val;
            val.Format(_T("org%04u"), k);
            CString guid = pApp->GetProfileString(key, val);
            if (guid.IsEmpty()) {
                break;
            }
            f->backup.AddTail(GUIDFromCString(guid));
        }

        f->guids.RemoveAll();
        for (unsigned int k = 0; ; k++) {
            CString val;
            val.Format(_T("mod%04u"), k);
            CString guid = pApp->GetProfileString(key, val);
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

void CAppSettings::SaveSettingsAutoChangeFullScreenMode()
{
    auto pApp = AfxGetMyApp();
    ASSERT(pApp);

    // Ensure the section is cleared before saving the new settings
    for (size_t i = 0;; i++) {
        CString section;
        section.Format(IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE, i);

        // WriteProfileString doesn't return false when INI is used and the section doesn't exist
        // so instead check for the a value inside that section
        if (!pApp->HasProfileEntry(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_CHECKED)) {
            break;
        } else {
            VERIFY(pApp->WriteProfileString(section, nullptr, nullptr));
        }
    }
    pApp->WriteProfileString(IDS_R_SETTINGS_FULLSCREEN_AUTOCHANGE_MODE, nullptr, nullptr);

    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS_FULLSCREEN_AUTOCHANGE_MODE, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_ENABLE, autoChangeFSMode.bEnabled));
    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS_FULLSCREEN_AUTOCHANGE_MODE, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_APPLYDEFMODEATFSEXIT, autoChangeFSMode.bApplyDefaultModeAtFSExit));
    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS_FULLSCREEN_AUTOCHANGE_MODE, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_RESTORERESAFTEREXIT, autoChangeFSMode.bRestoreResAfterProgExit));
    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS_FULLSCREEN_AUTOCHANGE_MODE, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_DELAY, (int)autoChangeFSMode.uDelay));

    for (size_t i = 0; i < autoChangeFSMode.modes.size(); i++) {
        const auto& mode = autoChangeFSMode.modes[i];
        CString section;
        section.Format(IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE, i);

        VERIFY(pApp->WriteProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_CHECKED, mode.bChecked));
        VERIFY(pApp->WriteProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_FRAMERATESTART, std::lround(mode.dFrameRateStart * 1000000)));
        VERIFY(pApp->WriteProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_FRAMERATESTOP, std::lround(mode.dFrameRateStop * 1000000)));
        VERIFY(pApp->WriteProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_AUDIODELAY, mode.msAudioDelay));
        VERIFY(pApp->WriteProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_BPP, mode.dm.bpp));
        VERIFY(pApp->WriteProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_FREQ, mode.dm.freq));
        VERIFY(pApp->WriteProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_SIZEX, mode.dm.size.cx));
        VERIFY(pApp->WriteProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_SIZEY, mode.dm.size.cy));
        VERIFY(pApp->WriteProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_FLAGS, (int)mode.dm.dwDisplayFlags));
    }
}

void CAppSettings::LoadSettings()
{
    CWinApp* pApp = AfxGetApp();
    ASSERT(pApp);

    UINT  len;
    BYTE* ptr = nullptr;

    if (bInitialized) {
        return;
    }

    // Set interface language first!
    language = (LANGID)pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LANGUAGE, -1);
    if (language == LANGID(-1)) {
        language = Translations::SetDefaultLanguage();
    } else if (language != 0) {
        if (language <= 23) {
            // We must be updating from a really old version, use the default language
            language = Translations::SetDefaultLanguage();
        } else if (!Translations::SetLanguage(language)) {
            // In case of error, reset the language to English
            language = 0;
        }
    }
#if USE_DRDUMP_CRASH_REPORTER
    if (language && CrashReporter::IsEnabled()) {
        CrashReporter::Enable(Translations::GetLanguageResourceByLocaleID(language).dllPath);
    }
#endif

    CreateCommands();

    eCaptionMenuMode = static_cast<MpcCaptionState>(pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECAPTIONMENU, MODE_SHOWCAPTIONMENU));
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
    eLoopMode = static_cast<LoopMode>(pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOOPMODE, static_cast<int>(LoopMode::PLAYLIST)));
    iZoomLevel = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ZOOM, 1);
    iDSVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE,
                                               IsVideoRendererAvailable(VIDRNDT_DS_EVR_CUSTOM) ? VIDRNDT_DS_EVR_CUSTOM : VIDRNDT_DS_VMR9RENDERLESS);
    iRMVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_RMVIDEORENDERERTYPE, VIDRNDT_RM_DEFAULT);
    iQTVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_QTVIDEORENDERERTYPE, VIDRNDT_QT_DEFAULT);
    nVolumeStep = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUMESTEP, 5);
    nSpeedStep = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPEEDSTEP, 0);

    UpdateRenderersData(false);

    strAudioRendererDisplayName = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIORENDERERTYPE);
    fAutoloadAudio = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADAUDIO, TRUE);
    strSubtitlesLanguageOrder = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANGORDER);
    strAudiosLanguageOrder = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOSLANGORDER);
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
    eAfterPlayback = static_cast<AfterPlayback>(pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AFTER_PLAYBACK, 0));

    bHideFullscreenControls = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDE_FULLSCREEN_CONTROLS, TRUE);
    eHideFullscreenControlsPolicy =
        static_cast<HideFullscreenControlsPolicy>(pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDE_FULLSCREEN_CONTROLS_POLICY, 1));
    uHideFullscreenControlsDelay = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDE_FULLSCREEN_CONTROLS_DELAY, 0);
    bHideFullscreenDockedPanels = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDE_FULLSCREEN_DOCKED_PANELS, TRUE);
    bHideWindowedControls = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDE_WINDOWED_CONTROLS, FALSE);

    bHideWindowedMousePointer = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDE_WINDOWED_MOUSE_POINTER, TRUE);

    // Multi-monitor code
    strFullScreenMonitor = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_FULLSCREENMONITOR);
    // Prevent Minimize when in fullscreen mode on non default monitor
    fPreventMinimize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PREVENT_MINIMIZE, FALSE);
    bUseEnhancedTaskBar = IsWindows7OrGreater() ? !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENHANCED_TASKBAR, TRUE) : FALSE;
    fUseSearchInFolder = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SEARCH_IN_FOLDER, TRUE);
    fUseTimeTooltip = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USE_TIME_TOOLTIP, TRUE);
    nTimeTooltipPosition = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TIME_TOOLTIP_POSITION, TIME_TOOLTIP_ABOVE_SEEKBAR);
    nOSDSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_OSD_SIZE, 18);
    LOGFONT lf;
    GetMessageFont(&lf);
    strOSDFont = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_MPC_OSD_FONT, lf.lfFaceName);

    // Associated types with icon or not...
    fAssociatedWithIcons = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASSOCIATED_WITH_ICON, TRUE);
    // Last Open Dir
    strLastOpenDir = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LAST_OPEN_DIR, _T("C:\\"));

    fAudioTimeShift = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOTIMESHIFT, FALSE);
    iAudioTimeShift = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOTIMESHIFT, 0);

    // Auto-change fullscreen mode
    autoChangeFSMode.bEnabled = !!pApp->GetProfileInt(IDS_R_SETTINGS_FULLSCREEN_AUTOCHANGE_MODE, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_ENABLE, FALSE);
    autoChangeFSMode.bApplyDefaultModeAtFSExit = !!pApp->GetProfileInt(IDS_R_SETTINGS_FULLSCREEN_AUTOCHANGE_MODE, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_APPLYDEFMODEATFSEXIT, TRUE);
    autoChangeFSMode.bRestoreResAfterProgExit  = !!pApp->GetProfileInt(IDS_R_SETTINGS_FULLSCREEN_AUTOCHANGE_MODE, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_RESTORERESAFTEREXIT, TRUE);
    autoChangeFSMode.uDelay = pApp->GetProfileInt(IDS_R_SETTINGS_FULLSCREEN_AUTOCHANGE_MODE, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_DELAY, 0);

    autoChangeFSMode.modes.clear();
    for (size_t i = 0;; i++) {
        CString section;
        section.Format(IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE, i);

        int iChecked = (int)pApp->GetProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_CHECKED, INT_ERROR);
        if (iChecked == INT_ERROR) {
            break;
        }

        double dFrameRateStart = pApp->GetProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_FRAMERATESTART, 0) / 1000000.0;
        double dFrameRateStop = pApp->GetProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_FRAMERATESTOP, 0) / 1000000.0;
        int msAudioDelay = pApp->GetProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_AUDIODELAY, fAudioTimeShift ? iAudioTimeShift  : 0);
        DisplayMode dm;
        dm.bValid = true;
        dm.bpp = (int)pApp->GetProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_BPP, 0);
        dm.freq = (int)pApp->GetProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_FREQ, 0);
        dm.size.cx = (LONG)pApp->GetProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_SIZEX, 0);
        dm.size.cy = (LONG)pApp->GetProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_SIZEY, 0);
        dm.dwDisplayFlags = pApp->GetProfileInt(section, IDS_RS_FULLSCREEN_AUTOCHANGE_MODE_MODE_DM_FLAGS, 0);

        autoChangeFSMode.modes.emplace_back(!!iChecked, dFrameRateStart, dFrameRateStop, msAudioDelay, std::move(dm));
    }

    fExitFullScreenAtTheEnd = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_EXITFULLSCREENATTHEEND, TRUE);

    fRememberWindowPos = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWPOS, FALSE);
    fRememberWindowSize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWSIZE, FALSE);
    CString str = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_PANSCANZOOM);
    if (_stscanf_s(str, _T("%lf,%lf"), &dZoomX, &dZoomY) == 2 &&
            dZoomX >= 0.196 && dZoomX <= 5.0 &&
            dZoomY >= 0.196 && dZoomY <= 5.0) {
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
    fileAssoc.SetNoRecentDocs(!fKeepHistory);
    iRecentFilesNumber = std::max(0, (int)pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_RECENT_FILES_NUMBER, 20));
    MRU.SetSize(iRecentFilesNumber);
    MRUDub.SetSize(iRecentFilesNumber);
    filePositions.SetMaxSize(iRecentFilesNumber);
    dvdPositions.SetMaxSize(iRecentFilesNumber);

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

    strDVDPath = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_DVDPATH);
    fUseDVDPath = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USEDVDPATH, FALSE);
    idMenuLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MENULANG, 0);
    idAudioLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOLANG, 0);
    idSubtitlesLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANG, 0);
    fClosedCaptions = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CLOSEDCAPTIONS, FALSE);
    {
        CString temp = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SPSTYLE);
        subtitlesDefStyle <<= temp;
        if (temp.IsEmpty()) { // Position the text subtitles relative to the video frame by default
            subtitlesDefStyle.relativeTo = STSStyle::AUTO;
        }
    }
    fOverridePlacement = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPOVERRIDEPLACEMENT, FALSE);
    nHorPos = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPHORPOS, 50);
    nVerPos = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPVERPOS, 90);
    bSubtitleARCompensation = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLEARCOMPENSATION, TRUE);
    nSubDelayStep = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBDELAYINTERVAL, 500);

    fEnableSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLESUBTITLES, TRUE);
    bPreferDefaultForcedSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PREFER_FORCED_DEFAULT_SUBTITLES, TRUE);
    fPrioritizeExternalSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITIZEEXTERNALSUBTITLES, TRUE);
    fDisableInternalSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEINTERNALSUBTITLES, FALSE);
    bAllowOverridingExternalSplitterChoice = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ALLOW_OVERRIDING_EXT_SPLITTER, FALSE);
    bAutoDownloadSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTODOWNLOADSUBTITLES, FALSE);
    nAutoDownloadScoreMovies = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTODOWNLOADSCOREMOVIES, 0x16);
    nAutoDownloadScoreSeries = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTODOWNLOADSCORESERIES, 0x18);
    strAutoDownloadSubtitlesExclude = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUTODOWNLOADSUBTITLESEXCLUDE);
    bAutoUploadSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOUPLOADSUBTITLES, FALSE);
    bPreferHearingImpairedSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PREFERHEARINGIMPAIREDSUBTITLES, FALSE);
    bMPCTheme = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPCTHEME, FALSE);
    bMPCThemeLoaded = bMPCTheme;
    if (IsWindows10OrGreater()) {
        CRegKey key;
        if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"), KEY_READ)) {
            DWORD useTheme = -1;
            if (ERROR_SUCCESS == key.QueryDWORDValue(_T("AppsUseLightTheme"), useTheme)) {
                if (0 == useTheme) bWindows10DarkThemeActive = true;
            }
        }
        if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\DWM"), KEY_READ)) {
            DWORD useColorPrevalence = -1;
            if (ERROR_SUCCESS == key.QueryDWORDValue(_T("ColorPrevalence"), useColorPrevalence)) {
                if (1 == useColorPrevalence) bWindows10AccentColorsEnabled = true;
            }
        }
    }
    bModernSeekbar = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MODERNSEEKBAR, TRUE);
    iModernSeekbarHeight= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MODERNSEEKBARHEIGHT, MIN_MODERN_SEEKBAR_HEIGHT);
    if (iModernSeekbarHeight < MIN_MODERN_SEEKBAR_HEIGHT || iModernSeekbarHeight > MAX_MODERN_SEEKBAR_HEIGHT) iModernSeekbarHeight = DEF_MODERN_SEEKBAR_HEIGHT;

    strSubtitlesProviders = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLESPROVIDERS, _T("<|OpenSubtitles|||1|1|>"));
    strSubtitlePaths = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLEPATHS, DEFAULT_SUBTITLE_PATHS);
    fUseDefaultSubtitlesStyle = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USEDEFAULTSUBTITLESSTYLE, FALSE);
    fEnableAudioSwitcher = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOSWITCHER, TRUE);
    fDownSampleTo441 = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DOWNSAMPLETO441, FALSE);
    fCustomChannelMapping = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CUSTOMCHANNELMAPPING, FALSE);

    BOOL bResult = pApp->GetProfileBinary(IDS_R_SETTINGS, IDS_RS_SPEAKERTOCHANNELMAPPING, &ptr, &len);
    if (bResult && len == sizeof(pSpeakerToChannelMap)) {
        memcpy(pSpeakerToChannelMap, ptr, sizeof(pSpeakerToChannelMap));
    } else {
        ZeroMemory(pSpeakerToChannelMap, sizeof(pSpeakerToChannelMap));
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
    nAudioBoost = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, 0);

    nSpeakerChannels = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPEAKERCHANNELS, 2);

    // External filters
    LoadExternalFilters(m_filters);

    fIntRealMedia = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_INTREALMEDIA, FALSE);
    m_pnspresets.RemoveAll();

    for (int i = 0; i < (ID_PANNSCAN_PRESETS_END - ID_PANNSCAN_PRESETS_START); i++) {
        CString str2;
        str2.Format(_T("Preset%d"), i);
        str2 = pApp->GetProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, str2);
        if (str2.IsEmpty()) {
            break;
        }
        m_pnspresets.Add(str2);
    }

    if (m_pnspresets.IsEmpty()) {
        const double _4p3 = 4.0 / 3.0;
        const double _16p9 = 16.0 / 9.0;
        const double _185p1 = 1.85 / 1.0;
        const double _235p1 = 2.35 / 1.0;
        UNREFERENCED_PARAMETER(_185p1);

        CString str2;
        str2.Format(IDS_SCALE_16_9, 0.5, 0.5, /*_4p3 / _4p3 =*/ 1.0, _16p9 / _4p3);
        m_pnspresets.Add(str2);
        str2.Format(IDS_SCALE_WIDESCREEN, 0.5, 0.5, _16p9 / _4p3, _16p9 / _4p3);
        m_pnspresets.Add(str2);
        str2.Format(IDS_SCALE_ULTRAWIDE, 0.5, 0.5, _235p1 / _4p3, _235p1 / _4p3);
        m_pnspresets.Add(str2);
    }

    for (int i = 0; i < wmcmds.GetCount(); i++) {
        CString str2;
        str2.Format(_T("CommandMod%d"), i);
        str2 = pApp->GetProfileString(IDS_R_COMMANDS, str2);
        if (str2.IsEmpty()) {
            break;
        }

        wmcmd tmp;
        int n;
        int fVirt = 0;
        if (5 > (n = _stscanf_s(str2, _T("%hu %x %hx %S %d %hhu %u %hhu"),
                                &tmp.cmd, &fVirt, &tmp.key, tmp.rmcmd.GetBuffer(128), 128,
                                &tmp.rmrepcnt, &tmp.mouse, &tmp.appcmd, &tmp.mouseFS))) {
            break;
        }
        tmp.rmcmd.ReleaseBuffer();
        if (n >= 2) {
            tmp.fVirt = (BYTE)fVirt;
        }
        if (POSITION pos = wmcmds.Find(tmp)) {
            wmcmd& wc = wmcmds.GetAt(pos);
            wc.cmd = tmp.cmd;
            wc.fVirt = tmp.fVirt;
            wc.key = tmp.key;
            if (n >= 6) {
                wc.mouse = tmp.mouse;
            }
            if (n >= 7) {
                wc.appcmd = tmp.appcmd;
            }
            // If there is no distinct bindings for windowed and
            // fullscreen modes we use the same for both.
            wc.mouseFS = (n >= 8) ? tmp.mouseFS : wc.mouse;
            wc.rmcmd = tmp.rmcmd.Trim('\"');
            wc.rmrepcnt = tmp.rmrepcnt;
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
    fGlobalMedia = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_GLOBALMEDIA, !IsWindows10OrGreater());

    nJumpDistS = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTS, DEFAULT_JUMPDISTANCE_1);
    nJumpDistM = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTM, DEFAULT_JUMPDISTANCE_2);
    nJumpDistL = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTL, DEFAULT_JUMPDISTANCE_3);
    fLimitWindowProportions = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LIMITWINDOWPROPORTIONS, FALSE);

    m_Formats.UpdateData(false);

    // Internal filters
    for (int f = 0; f < SRC_LAST; f++) {
        SrcFilters[f] = !!pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, SrcFiltersKeys[f].name, SrcFiltersKeys[f].bDefault);
    }
    for (int f = 0; f < TRA_LAST; f++) {
        TraFilters[f] = !!pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, TraFiltersKeys[f].name, TraFiltersKeys[f].bDefault);
    }

    strLogoFileName = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LOGOFILE);
    nLogoId = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOID, -1);
    fLogoExternal = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOEXT, FALSE);

    fHideCDROMsSubMenu = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECDROMSSUBMENU, FALSE);

    dwPriority = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITY, NORMAL_PRIORITY_CLASS);
    ::SetPriorityClass(::GetCurrentProcess(), dwPriority);
    fLaunchfullscreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LAUNCHFULLSCREEN, FALSE);

    fEnableWebServer = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWEBSERVER, FALSE);
    nWebServerPort = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPORT, 13579);
    fWebServerUseCompression = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERUSECOMPRESSION, TRUE);
    fWebServerLocalhostOnly = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERLOCALHOSTONLY, FALSE);
    bWebUIEnablePreview = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBUI_ENABLE_PREVIEW, FALSE);
    fWebServerPrintDebugInfo = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPRINTDEBUGINFO, FALSE);
    strWebRoot = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBROOT, _T("*./webroot"));
    strWebDefIndex = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBDEFINDEX, _T("index.html;index.php"));
    strWebServerCGI = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBSERVERCGI);

    CString MyPictures;

    CRegKey key;
    // grrrrr
    // if (!SHGetSpecialFolderPath(nullptr, MyPictures.GetBufferSetLength(MAX_PATH), CSIDL_MYPICTURES, TRUE)) MyPictures.Empty();
    // else MyPictures.ReleaseBuffer();
    if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), KEY_READ)) {
        ULONG lenValue = MAX_PATH;
        if (ERROR_SUCCESS == key.QueryStringValue(_T("My Pictures"), MyPictures.GetBuffer(MAX_PATH), &lenValue)) {
            MyPictures.ReleaseBufferSetLength(lenValue);
        } else {
            MyPictures.Empty();
        }
    }
    strSnapshotPath = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTPATH, MyPictures);
    strSnapshotExt = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTEXT, _T(".jpg"));

    iThumbRows = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBROWS, 4);
    iThumbCols = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBCOLS, 4);
    iThumbWidth = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBWIDTH, 1024);

    bSubSaveExternalStyleFile = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBSAVEEXTERNALSTYLEFILE, FALSE);
    nLastUsedPage = WORD(pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTUSEDPAGE, 0));

    {
        // Load the list of extra (non-default) shader files
        m_ShadersExtraList = ShaderList(pApp->GetProfileString(IDS_R_SHADERS, IDS_RS_SHADERS_EXTRA));
        // Load shader selection
        m_Shaders.SetCurrentPreset(ShaderPreset(pApp->GetProfileString(IDS_R_SHADERS, IDS_RS_SHADERS_PRERESIZE),
                                                pApp->GetProfileString(IDS_R_SHADERS, IDS_RS_SHADERS_POSTRESIZE)));
        // Load shader presets
        ShaderSelection::ShaderPresetMap presets;
        for (int i = 0;; i++) {
            CString iStr;
            iStr.Format(_T("%d"), i);
            CString name = pApp->GetProfileString(IDS_R_SHADER_PRESETS, iStr);
            if (name.IsEmpty()) {
                break;
            }
            presets.emplace(name, ShaderPreset(pApp->GetProfileString(IDS_R_SHADER_PRESETS, IDS_RS_SHADERS_PRERESIZE + iStr),
                                               pApp->GetProfileString(IDS_R_SHADER_PRESETS, IDS_RS_SHADERS_POSTRESIZE + iStr)));
        }
        m_Shaders.SetPresets(presets);
        // Load last shader preset name
        CString name = pApp->GetProfileString(IDS_R_SHADERS, IDS_RS_SHADERS_LASTPRESET);
        if (!name.IsEmpty()) {
            m_Shaders.SetCurrentPreset(name);
        }
    }

    // CASIMIR666 : new settings
    fD3DFullscreen        = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_D3DFULLSCREEN, FALSE);

    iBrightness           = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_BRIGHTNESS, 0);
    iContrast             = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_CONTRAST, 0);
    iHue                  = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_HUE, 0);
    iSaturation           = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_SATURATION, 0);

    fShowOSD              = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SHOWOSD, TRUE);
    fEnableEDLEditor      = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEEDLEDITOR, FALSE);
    bFastSeek             = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FASTSEEK, TRUE);
    eFastSeekMethod       = static_cast<decltype(eFastSeekMethod)>(
                                pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FASTSEEK_METHOD, FASTSEEK_NEAREST_KEYFRAME));
    fShowChapters         = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SHOW_CHAPTERS, TRUE);


    fLCDSupport = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LCD_SUPPORT, FALSE);

    // Save analog capture settings
    iDefaultCaptureDevice = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULT_CAPTURE, 0);
    strAnalogVideo        = pApp->GetProfileString(IDS_R_CAPTURE, IDS_RS_VIDEO_DISP_NAME, _T("dummy"));
    strAnalogAudio        = pApp->GetProfileString(IDS_R_CAPTURE, IDS_RS_AUDIO_DISP_NAME, _T("dummy"));
    iAnalogCountry        = pApp->GetProfileInt(IDS_R_CAPTURE, IDS_RS_COUNTRY, 1);

    strBDANetworkProvider = pApp->GetProfileString(IDS_R_DVB, IDS_RS_BDA_NETWORKPROVIDER);
    strBDATuner           = pApp->GetProfileString(IDS_R_DVB, IDS_RS_BDA_TUNER);
    strBDAReceiver        = pApp->GetProfileString(IDS_R_DVB, IDS_RS_BDA_RECEIVER);
    //sBDAStandard        = pApp->GetProfileString(IDS_R_DVB, IDS_RS_BDA_STANDARD);
    iBDAScanFreqStart     = pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_SCAN_FREQ_START, 474000);
    iBDAScanFreqEnd       = pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_SCAN_FREQ_END, 858000);
    iBDABandwidth         = pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_BANDWIDTH, 8);
    fBDAUseOffset         = !!pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_USE_OFFSET, FALSE);
    iBDAOffset            = pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_OFFSET, 166);
    fBDAIgnoreEncryptedChannels = !!pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_IGNORE_ENCRYPTED_CHANNELS, FALSE);
    nDVBLastChannel       = pApp->GetProfileInt(IDS_R_DVB, IDS_RS_DVB_LAST_CHANNEL, INT_ERROR);
    nDVBRebuildFilterGraph = (DVB_RebuildFilterGraph) pApp->GetProfileInt(IDS_R_DVB, IDS_RS_DVB_REBUILD_FG, DVB_STOP_FG_ALWAYS);
    nDVBStopFilterGraph = (DVB_StopFilterGraph) pApp->GetProfileInt(IDS_R_DVB, IDS_RS_DVB_STOP_FG, DVB_STOP_FG_ALWAYS);

    for (int iChannel = 0; ; iChannel++) {
        CString strTemp;
        strTemp.Format(_T("%d"), iChannel);
        CString strChannel = pApp->GetProfileString(IDS_R_DVB, strTemp);
        if (strChannel.IsEmpty()) {
            break;
        }
        try {
            m_DVBChannels.emplace_back(strChannel);
        } catch (CException* e) {
            // The tokenisation can fail if the input string was invalid
            TRACE(_T("Failed to parse a DVB channel from string \"%s\""), strChannel.GetString());
            ASSERT(FALSE);
            e->Delete();
        }
    }

    // playback positions for last played files
    fRememberFilePos = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOS, FALSE);
    iRememberPosForLongerThan = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOSLONGER, 0);
    bRememberPosForAudioFiles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOSAUDIO, TRUE);
    filePositions.Load();

    // playback positions for last played DVDs
    fRememberDVDPos = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DVDPOS, FALSE);
    dvdPositions.Load();

    fLastFullScreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTFULLSCREEN, FALSE);

    fRemainingTime = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMAINING_TIME, FALSE);
    bHighPrecisionTimer = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIGH_PRECISION_TIMER, FALSE);

    nUpdaterAutoCheck = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_UPDATER_AUTO_CHECK, AUTOUPDATE_UNKNOWN);
    nUpdaterDelay = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_UPDATER_DELAY, 7);
    if (nUpdaterDelay < 1) {
        nUpdaterDelay = 1;
    }

    bNotifySkype = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_NOTIFY_SKYPE, FALSE);

    nJpegQuality = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JPEG_QUALITY, 90);
    if (nJpegQuality < 0 || nJpegQuality > 100) {
        nJpegQuality = 90;
    }

    bEnableCoverArt = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COVER_ART, TRUE);
    nCoverArtSizeLimit = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COVER_ART_SIZE_LIMIT, 600);

    bEnableLogging = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOGGING, FALSE);
    bUseLegacyToolbar = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USE_LEGACY_TOOLBAR, FALSE);

    eSubtitleRenderer = static_cast<SubtitleRenderer>(pApp->GetProfileInt(IDS_R_SETTINGS,
                                                      IDS_RS_SUBTITLE_RENDERER, static_cast<int>(SubtitleRenderer::INTERNAL)));

    nDefaultToolbarSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULTTOOLBARSIZE, 24);

    bSaveImagePosition = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SAVEIMAGE_POSITION, TRUE);
    bSaveImageCurrentTime = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SAVEIMAGE_CURRENTTIME, FALSE);

    bAllowInaccurateFastseek = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ALLOW_INACCURATE_FASTSEEK, TRUE);
    bLoopFolderOnPlayNextFile = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOOP_FOLDER_NEXT_FILE, FALSE);    

    if (fLaunchfullscreen) {
        nCLSwitches |= CLSW_FULLSCREEN;
    }

    sanear->SetOuputDevice(pApp->GetProfileString(IDS_R_SANEAR, IDS_RS_SANEAR_DEVICE_ID),
                           pApp->GetProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_DEVICE_EXCLUSIVE, FALSE),
                           pApp->GetProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_DEVICE_BUFFER,
                                               SaneAudioRenderer::ISettings::OUTPUT_DEVICE_BUFFER_DEFAULT_MS));

    sanear->SetAllowBitstreaming(pApp->GetProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_ALLOW_BITSTREAMING, TRUE));

    sanear->SetCrossfeedEnabled(pApp->GetProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_CROSSFEED_ENABLED, FALSE));

    sanear->SetCrossfeedSettings(pApp->GetProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_CROSSFEED_CUTOFF_FREQ,
                                                     SaneAudioRenderer::ISettings::CROSSFEED_CUTOFF_FREQ_CMOY),
                                 pApp->GetProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_CROSSFEED_LEVEL,
                                                     SaneAudioRenderer::ISettings::CROSSFEED_LEVEL_CMOY));

    sanear->SetIgnoreSystemChannelMixer(pApp->GetProfileInt(IDS_R_SANEAR, IDS_RS_SANEAR_IGNORE_SYSTEM_MIXER, TRUE));

    bUseYDL       = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USE_YDL, TRUE);
    iYDLMaxHeight = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_YDL_MAX_HEIGHT, 1440);
    iYDLVideoFormat = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_YDL_VIDEO_FORMAT, 0);
    bYDLAudioOnly   = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_YDL_AUDIO_ONLY, FALSE);
    sYDLCommandLine = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_YDL_COMMAND_LINE, _T(""));

    bInitialized = true;
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

        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRAlternateVSync"), ars.bVMR9AlterativeVSync);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSyncOffset"), ars.iVMR9VSyncOffset);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSyncAccurate2"), ars.bVMR9VSyncAccurate);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFullscreenGUISupport"), ars.bVMR9FullscreenGUISupport);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSync"), ars.bVMR9VSync);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRDisableDesktopComposition"), ars.bVMRDisableDesktopComposition);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFullFloatingPointProcessing2"), ars.bVMR9FullFloatingPointProcessing);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRHalfFloatingPointProcessing"), ars.bVMR9HalfFloatingPointProcessing);

        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementEnable"), ars.bVMR9ColorManagementEnable);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementInput"), ars.iVMR9ColorManagementInput);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementAmbientLight"), ars.iVMR9ColorManagementAmbientLight);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementIntent"), ars.iVMR9ColorManagementIntent);

        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVROutputRange"), ars.iEVROutputRange);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVRHighColorRes"), ars.bEVRHighColorResolution);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVRForceInputHighColorRes"), ars.bEVRForceInputHighColorResolution);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVREnableFrameTimeCorrection"), ars.bEVREnableFrameTimeCorrection);

        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUBeforeVSync"), ars.bVMRFlushGPUBeforeVSync);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUAfterPresent"), ars.bVMRFlushGPUAfterPresent);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUWait"), ars.bVMRFlushGPUWait);

        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeClock"), ars.bSynchronizeVideo);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeDisplay"), ars.bSynchronizeDisplay);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeNearest"), ars.bSynchronizeNearest);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("LineDelta"), ars.iLineDelta);
        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ColumnDelta"), ars.iColumnDelta);

        pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("CycleDelta"), (LPBYTE) & (ars.fCycleDelta), sizeof(ars.fCycleDelta));
        pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("TargetSyncOffset"), (LPBYTE) & (ars.fTargetSyncOffset), sizeof(ars.fTargetSyncOffset));
        pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("ControlLimit"), (LPBYTE) & (ars.fControlLimit), sizeof(ars.fControlLimit));

        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CACHESHADERS, ars.bCacheShaders);

        pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ResetDevice"), r.fResetDevice);

        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPCSIZE, r.subPicQueueSettings.nSize);
        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPCMAXRES, r.subPicQueueSettings.nMaxRes);
        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLE_SUBTITLE_ANIMATION, r.subPicQueueSettings.bDisableSubtitleAnimation);
        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_RENDER_AT_WHEN_ANIM_DISABLED, r.subPicQueueSettings.nRenderAtWhenAnimationIsDisabled);
        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLE_ANIMATION_RATE, r.subPicQueueSettings.nAnimationRate);
        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ALLOW_DROPPING_SUBPIC, r.subPicQueueSettings.bAllowDroppingSubpic);

        pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_EVR_BUFFERS, r.iEvrBuffers);

        pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_D3D9RENDERDEVICE, r.D3D9RenderDevice);
    } else {
        r.iAPSurfaceUsage = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_APSURACEFUSAGE, VIDRNDT_AP_TEXTURE3D);
        r.iDX9Resizer = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DX9_RESIZER, 1);
        r.fVMR9MixerMode = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERMODE, TRUE);

        CRenderersSettings::CAdvRendererSettings DefaultSettings;
        ars.bVMR9AlterativeVSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRAlternateVSync"), DefaultSettings.bVMR9AlterativeVSync);
        ars.iVMR9VSyncOffset = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSyncOffset"), DefaultSettings.iVMR9VSyncOffset);
        ars.bVMR9VSyncAccurate = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSyncAccurate2"), DefaultSettings.bVMR9VSyncAccurate);
        ars.bVMR9FullscreenGUISupport = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFullscreenGUISupport"), DefaultSettings.bVMR9FullscreenGUISupport);
        ars.bEVRHighColorResolution = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVRHighColorRes"), DefaultSettings.bEVRHighColorResolution);
        ars.bEVRForceInputHighColorResolution = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVRForceInputHighColorRes"), DefaultSettings.bEVRForceInputHighColorResolution);
        ars.bEVREnableFrameTimeCorrection = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVREnableFrameTimeCorrection"), DefaultSettings.bEVREnableFrameTimeCorrection);
        ars.bVMR9VSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSync"), DefaultSettings.bVMR9VSync);
        ars.bVMRDisableDesktopComposition = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRDisableDesktopComposition"), DefaultSettings.bVMRDisableDesktopComposition);
        ars.bVMR9FullFloatingPointProcessing = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFullFloatingPointProcessing2"), DefaultSettings.bVMR9FullFloatingPointProcessing);
        ars.bVMR9HalfFloatingPointProcessing = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRHalfFloatingPointProcessing"), DefaultSettings.bVMR9HalfFloatingPointProcessing);

        ars.bVMR9ColorManagementEnable = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementEnable"), DefaultSettings.bVMR9ColorManagementEnable);
        ars.iVMR9ColorManagementInput = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementInput"), DefaultSettings.iVMR9ColorManagementInput);
        ars.iVMR9ColorManagementAmbientLight = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementAmbientLight"), DefaultSettings.iVMR9ColorManagementAmbientLight);
        ars.iVMR9ColorManagementIntent = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementIntent"), DefaultSettings.iVMR9ColorManagementIntent);

        ars.iEVROutputRange = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVROutputRange"), DefaultSettings.iEVROutputRange);

        ars.bVMRFlushGPUBeforeVSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUBeforeVSync"), DefaultSettings.bVMRFlushGPUBeforeVSync);
        ars.bVMRFlushGPUAfterPresent = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUAfterPresent"), DefaultSettings.bVMRFlushGPUAfterPresent);
        ars.bVMRFlushGPUWait = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUWait"), DefaultSettings.bVMRFlushGPUWait);

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

        ars.bCacheShaders = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CACHESHADERS, DefaultSettings.bCacheShaders);
        if (AfxGetMyApp()->GetAppSavePath(ars.sShaderCachePath)) {
            ars.sShaderCachePath = PathUtils::CombinePaths(ars.sShaderCachePath, IDS_R_SHADER_CACHE);
        }

        r.fResetDevice = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("ResetDevice"), FALSE);

        r.subPicQueueSettings.nSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPCSIZE, 10);
        r.subPicQueueSettings.nMaxRes = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPCMAXRES, 0);
        r.subPicQueueSettings.bDisableSubtitleAnimation = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLE_SUBTITLE_ANIMATION, FALSE);
        r.subPicQueueSettings.nRenderAtWhenAnimationIsDisabled = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_RENDER_AT_WHEN_ANIM_DISABLED, 50);
        r.subPicQueueSettings.nAnimationRate = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLE_ANIMATION_RATE, 100);
        r.subPicQueueSettings.bAllowDroppingSubpic = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ALLOW_DROPPING_SUBPIC, TRUE);

        r.iEvrBuffers = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_EVR_BUFFERS, 5);
        r.D3D9RenderDevice = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_D3D9RENDERDEVICE);
    }
}

__int64 CAppSettings::ConvertTimeToMSec(const CString& time) const
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

        if (!fullPathName.IsEmpty() && PathUtils::Exists(fullPathName)) {
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
    ZeroMemory(&DVDPosition, sizeof(DVDPosition));
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
                    rtShift = 10000i64 * _tstol(strFile.Mid(nPos + 6));
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
            } else if (sw == _T("device")) {
                nCLSwitches |= CLSW_DEVICE;
            } else if (sw == _T("add")) {
                nCLSwitches |= CLSW_ADD;
            } else if (sw == _T("randomize")) {
                nCLSwitches |= CLSW_RANDOMIZE;
            } else if (sw == _T("volume") && pos) {
                int setVolumeVal = _ttoi(cmdln.GetNext(pos));
                if (setVolumeVal >= 0 && setVolumeVal <= 100) {
                    nCmdVolume = setVolumeVal;
                    nCLSwitches |= CLSW_VOLUME;
                } else {
                    nCLSwitches |= CLSW_UNRECOGNIZEDSWITCH;
                }
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
                hMasterWnd = (HWND)IntToPtr(_ttoi(cmdln.GetNext(pos)));
            } else if (sw == _T("fixedsize") && pos) {
                CAtlList<CString> sl;
                Explode(cmdln.GetNext(pos), sl, ',', 2);
                if (sl.GetCount() == 2) {
                    sizeFixedWindow.SetSize(_ttol(sl.GetHead()), _ttol(sl.GetTail()));
                    if (sizeFixedWindow.cx > 0 && sizeFixedWindow.cy > 0) {
                        nCLSwitches |= CLSW_FIXEDSIZE;
                    }
                }
            } else if (sw == _T("viewpreset") && pos) {
                int viewPreset = _ttoi(cmdln.GetNext(pos));
                switch (viewPreset) {
                    case 1:
                        nCLSwitches |= CLSW_PRESET1;
                        break;
                    case 2:
                        nCLSwitches |= CLSW_PRESET2;
                        break;
                    case 3:
                        nCLSwitches |= CLSW_PRESET3;
                        break;
                    default:
                        nCLSwitches |= CLSW_UNRECOGNIZEDSWITCH;
                        break;
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
            } else if (sw == _T("nocrashreporter")) {
#if USE_DRDUMP_CRASH_REPORTER
                CrashReporter::Disable();
#endif
                MPCExceptionHandler::Enable();
            } else if (sw == _T("audiorenderer") && pos) {
                SetAudioRenderer(_ttoi(cmdln.GetNext(pos)));
            } else if (sw == _T("shaderpreset") && pos) {
                m_Shaders.SetCurrentPreset(cmdln.GetNext(pos));
            } else if (sw == _T("reset")) {
                nCLSwitches |= CLSW_RESET;
            } else if (sw == _T("mute")) {
                nCLSwitches |= CLSW_MUTE;
            } else if (sw == _T("monitoroff")) {
                nCLSwitches |= CLSW_MONITOROFF;
            } else if (sw == _T("playnext")) {
                nCLSwitches |= CLSW_PLAYNEXT;
            } else if (sw == _T("hwgpu") && pos) {
                iLAVGPUDevice = _tcstol(cmdln.GetNext(pos), nullptr, 10);
            } else if (sw == _T("configlavsplitter")) {
                nCLSwitches |= CLSW_CONFIGLAVSPLITTER;
            } else if (sw == _T("configlavaudio")) {
                nCLSwitches |= CLSW_CONFIGLAVAUDIO;
            } else if (sw == _T("configlavvideo")) {
                nCLSwitches |= CLSW_CONFIGLAVVIDEO;
            } else {
                nCLSwitches |= CLSW_HELP | CLSW_UNRECOGNIZEDSWITCH;
            }
        } else {
            if (param == _T("-")) { // Special case: standard input
                slFiles.AddTail(_T("pipe://stdin"));
            } else {
                const_cast<CString&>(param) = ParseFileName(param);
                slFiles.AddTail(param);
            }
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
        s = AfxGetApp()->GetProfileString(root, s);
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
    auto it = find_if(m_DVBChannels.begin(), m_DVBChannels.end(), [&](CDVBChannel const & channel) {
        return channel.GetPrefNumber() == nPrefNumber;
    });

    return it != m_DVBChannels.end() ? &(*it) : nullptr;
}

// Settings::CRecentFileAndURLList
CAppSettings::CRecentFileAndURLList::CRecentFileAndURLList(UINT nStart, LPCTSTR lpszSection,
                                                           LPCTSTR lpszEntryFormat, int nSize,
                                                           int nMaxDispLen)
    : CRecentFileList(nStart, lpszSection, lpszEntryFormat, nSize, nMaxDispLen)
{
}

extern BOOL AFXAPI AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2);

void CAppSettings::CRecentFileAndURLList::Add(LPCTSTR lpszPathName)
{
    ASSERT(m_arrNames != nullptr);
    ASSERT(lpszPathName != nullptr);
    ASSERT(AfxIsValidString(lpszPathName));

    if (m_nSize <= 0 || CString(lpszPathName).MakeLower().Find(_T("@device:")) >= 0) {
        return;
    }

    CString pathName = lpszPathName;

    bool fURL = (pathName.Find(_T("://")) >= 0);

    // fully qualify the path name
    if (!fURL) {
        pathName = MakeFullPath(pathName);
    }

    // update the MRU list, if an existing MRU string matches file name
    int iMRU;
    for (iMRU = 0; iMRU < m_nSize - 1; iMRU++) {
        if ((fURL && !_tcscmp(m_arrNames[iMRU], pathName))
                || AfxComparePath(m_arrNames[iMRU], pathName)) {
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
    m_arrNames[0] = pathName;
}

void CAppSettings::CRecentFileAndURLList::SetSize(int nSize)
{
    ENSURE_ARG(nSize >= 0);

    if (m_nSize != nSize) {
        CString* arrNames = new CString[nSize];
        int nSizeToCopy = std::min(m_nSize, nSize);
        for (int i = 0; i < nSizeToCopy; i++) {
            arrNames[i] = m_arrNames[i];
        }
        delete [] m_arrNames;
        m_arrNames = arrNames;
        m_nSize = nSize;
    }
}

bool CAppSettings::IsVSFilterInstalled()
{
    return IsCLSIDRegistered(CLSID_VSFilter);
}

void CAppSettings::UpdateSettings()
{
    auto pApp = AfxGetMyApp();
    ASSERT(pApp);

    UINT version = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_R_VERSION, 0);
    if (version >= APPSETTINGS_VERSION) {
        return; // Nothing to update
    }

    // Use lambda expressions to copy data entries
    auto copyInt = [pApp](LPCTSTR oldSection, LPCTSTR oldEntry, LPCTSTR newSection, LPCTSTR newEntry) {
        if (pApp->HasProfileEntry(oldSection, oldEntry)) {
            int old = pApp->GetProfileInt(oldSection, oldEntry, 0);
            VERIFY(pApp->WriteProfileInt(newSection, newEntry, old));
        }
    };
    auto copyStr = [pApp](LPCTSTR oldSection, LPCTSTR oldEntry, LPCTSTR newSection, LPCTSTR newEntry) {
        if (pApp->HasProfileEntry(oldSection, oldEntry)) {
            CString old = pApp->GetProfileString(oldSection, oldEntry);
            VERIFY(pApp->WriteProfileString(newSection, newEntry, old));
        }
    };
    auto copyBin = [pApp](LPCTSTR oldSection, LPCTSTR oldEntry, LPCTSTR newSection, LPCTSTR newEntry) {
        UINT len;
        BYTE* old;
        if (pApp->GetProfileBinary(oldSection, oldEntry, &old, &len)) {
            VERIFY(pApp->WriteProfileBinary(newSection, newEntry, old, len));
            delete [] old;
        }
    };


    // Migrate to the latest version, these cases should fall through
    // so that all incremental updates are applied.
    switch (version) {
        case 0: {
            UINT nAudioBoostTmp = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, -1);
            if (nAudioBoostTmp == UINT(-1)) {
                double dAudioBoost_dB = _tstof(pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, _T("0")));
                if (dAudioBoost_dB < 0 || dAudioBoost_dB > 10) {
                    dAudioBoost_dB = 0;
                }
                nAudioBoostTmp = UINT(100 * pow(10.0, dAudioBoost_dB / 20.0) + 0.5) - 100;
            }
            if (nAudioBoostTmp > 300) { // Max boost is 300%
                nAudioBoostTmp = 300;
            }
            pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, nAudioBoostTmp);

            ConvertOldExternalFiltersList();
        }
        {
            const CString section(_T("Settings"));
            copyInt(section, _T("Remember DVD Pos"), section, _T("RememberDVDPos"));
            copyInt(section, _T("Remember File Pos"), section, _T("RememberFilePos"));
            copyInt(section, _T("Show OSD"), section, _T("ShowOSD"));
            copyStr(section, _T("Shaders List"), section, _T("ShadersList"));
            copyInt(section, _T("OSD_Size"), section, _T("OSDSize"));
            copyStr(section, _T("OSD_Font"), section, _T("OSDFont"));
            copyInt(section, _T("gotoluf"), section, _T("GoToLastUsed"));
            copyInt(section, _T("fps"), section, _T("GoToFPS"));
        }
        {
            // Copy DVB section
            const CString oldSection(_T("DVB configuration"));
            const CString newSection(_T("DVBConfiguration"));
            copyStr(oldSection, _T("BDANetworkProvider"), newSection, _T("BDANetworkProvider"));
            copyStr(oldSection, _T("BDATuner"), newSection, _T("BDATuner"));
            copyStr(oldSection, _T("BDAReceiver"), newSection, _T("BDAReceiver"));
            copyInt(oldSection, _T("BDAScanFreqStart"), newSection, _T("BDAScanFreqStart"));
            copyInt(oldSection, _T("BDAScanFreqEnd"), newSection, _T("BDAScanFreqEnd"));
            copyInt(oldSection, _T("BDABandWidth"), newSection, _T("BDABandWidth"));
            copyInt(oldSection, _T("BDAUseOffset"), newSection, _T("BDAUseOffset"));
            copyInt(oldSection, _T("BDAOffset"), newSection, _T("BDAOffset"));
            copyInt(oldSection, _T("BDAIgnoreEncryptedChannels"), newSection, _T("BDAIgnoreEncryptedChannels"));
            copyInt(oldSection, _T("LastChannel"), newSection, _T("LastChannel"));
            copyInt(oldSection, _T("RebuildFilterGraph"), newSection, _T("RebuildFilterGraph"));
            copyInt(oldSection, _T("StopFilterGraph"), newSection, _T("StopFilterGraph"));
            for (int iChannel = 0; ; iChannel++) {
                CString strTemp, strChannel;
                strTemp.Format(_T("%d"), iChannel);
                if (!pApp->HasProfileEntry(oldSection, strTemp)) {
                    break;
                }
                strChannel = pApp->GetProfileString(oldSection, strTemp);
                if (strChannel.IsEmpty()) {
                    break;
                }
                VERIFY(pApp->WriteProfileString(newSection, strTemp, strChannel));
            }
        }
        // no break
        case 1: {
            // Internal decoding of WMV 1/2/3 is now disabled by default so we reinitialize its value
            pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, _T("TRA_WMV"), FALSE);
        }
        // no break
        case 2: {
            const CString section(_T("Settings"));
            if (pApp->HasProfileEntry(section, _T("FullScreenCtrls")) &&
                    pApp->HasProfileEntry(section, _T("FullScreenCtrlsTimeOut"))) {
                bool bHide = true;
                int nHidePolicy = 0;
                int nTimeout = -1;
                if (!pApp->GetProfileInt(section, _T("FullScreenCtrls"), 0)) {
                    // hide always
                } else {
                    nTimeout = pApp->GetProfileInt(section, _T("FullScreenCtrlsTimeOut"), 0);
                    if (nTimeout < 0) {
                        // show always
                        bHide = false;
                    } else if (nTimeout == 0) {
                        // show when hovered
                        nHidePolicy = 1;
                    } else {
                        // show when mouse moved
                        nHidePolicy = 2;
                    }
                }
                VERIFY(pApp->WriteProfileInt(section, _T("HideFullscreenControls"), bHide));
                if (nTimeout >= 0) {
                    VERIFY(pApp->WriteProfileInt(section, _T("HideFullscreenControlsPolicy"), nHidePolicy));
                    VERIFY(pApp->WriteProfileInt(section, _T("HideFullscreenControlsDelay"), nTimeout * 1000));
                }
            }
        }
        // no break
        case 3: {
#pragma pack(push, 1)
            struct dispmode {
                bool fValid;
                CSize size;
                int bpp, freq;
                DWORD dmDisplayFlags;
            };

            struct fpsmode {
                double vfr_from;
                double vfr_to;
                bool fChecked;
                dispmode dmFSRes;
                bool fIsData;
            };

            struct AChFR {
                bool bEnabled;
                fpsmode dmFullscreenRes[30];
                bool bApplyDefault;
            }; //AutoChangeFullscrRes
#pragma pack(pop)

            LPBYTE ptr;
            UINT len;
            bool bSetDefault = true;
            if (pApp->GetProfileBinary(IDS_R_SETTINGS, _T("FullscreenRes"), &ptr, &len)) {
                if (len == sizeof(AChFR)) {
                    AChFR autoChangeFullscrRes;
                    memcpy(&autoChangeFullscrRes, ptr, sizeof(AChFR));

                    autoChangeFSMode.bEnabled = autoChangeFullscrRes.bEnabled;
                    autoChangeFSMode.bApplyDefaultModeAtFSExit = autoChangeFullscrRes.bApplyDefault;

                    for (size_t i = 0; i < _countof(autoChangeFullscrRes.dmFullscreenRes); i++) {
                        const auto& modeOld = autoChangeFullscrRes.dmFullscreenRes[i];
                        // The old settings could be corrupted quite easily so be careful when converting them
                        if (modeOld.fIsData
                                && modeOld.vfr_from >= 0.0 && modeOld.vfr_from <= 126.0
                                && modeOld.vfr_to >= 0.0 && modeOld.vfr_to <= 126.0
                                && modeOld.dmFSRes.fValid
                                && modeOld.dmFSRes.bpp == 32
                                && modeOld.dmFSRes.size.cx >= 640 && modeOld.dmFSRes.size.cx < 10000
                                && modeOld.dmFSRes.size.cy >= 380 && modeOld.dmFSRes.size.cy < 10000
                                && modeOld.dmFSRes.freq > 0 && modeOld.dmFSRes.freq < 1000) {
                            DisplayMode dm;
                            dm.bValid = true;
                            dm.size = modeOld.dmFSRes.size;
                            dm.bpp = 32;
                            dm.freq = modeOld.dmFSRes.freq;
                            dm.dwDisplayFlags = modeOld.dmFSRes.dmDisplayFlags & DM_INTERLACED;

                            autoChangeFSMode.modes.emplace_back(modeOld.fChecked, modeOld.vfr_from, modeOld.vfr_to, 0, std::move(dm));
                        }
                    }

                    bSetDefault = autoChangeFSMode.modes.empty() || autoChangeFSMode.modes[0].dFrameRateStart != 0.0 || autoChangeFSMode.modes[0].dFrameRateStop != 0.0;
                }
                delete [] ptr;
            }

            if (bSetDefault) {
                autoChangeFSMode.bEnabled = false;
                autoChangeFSMode.bApplyDefaultModeAtFSExit = true;
                autoChangeFSMode.modes.clear();
            }
            autoChangeFSMode.bRestoreResAfterProgExit = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("RestoreResAfterExit"), TRUE);
            autoChangeFSMode.uDelay = pApp->GetProfileInt(IDS_R_SETTINGS, _T("FullscreenResDelay"), 0);

            SaveSettingsAutoChangeFullScreenMode();
        }
        // no break
        case 4: {
            bool bDisableSubtitleAnimation = !pApp->GetProfileInt(IDS_R_SETTINGS, _T("SPCAllowAnimationWhenBuffering"), TRUE);
            VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLE_SUBTITLE_ANIMATION, bDisableSubtitleAnimation));
        }
        // no break
        case 5:
            copyInt(IDS_R_INTERNAL_FILTERS, _T("SRC_DTSAC3"), IDS_R_INTERNAL_FILTERS, _T("SRC_DTS"));
            copyInt(IDS_R_INTERNAL_FILTERS, _T("SRC_DTSAC3"), IDS_R_INTERNAL_FILTERS, _T("SRC_AC3"));
        // no break
        case 6: {
            SubtitleRenderer subrenderer = SubtitleRenderer::INTERNAL;
            if (!pApp->GetProfileInt(IDS_R_SETTINGS, _T("AutoloadSubtitles"), TRUE)) {
                if (IsSubtitleRendererRegistered(SubtitleRenderer::VS_FILTER)) {
                    subrenderer = SubtitleRenderer::VS_FILTER;
                }
                if (IsSubtitleRendererRegistered(SubtitleRenderer::XY_SUB_FILTER)) {
                    int renderer = IsVideoRendererAvailable(VIDRNDT_DS_EVR_CUSTOM) ? VIDRNDT_DS_EVR_CUSTOM : VIDRNDT_DS_VMR9RENDERLESS;
                    renderer = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE, renderer);
                    if (IsSubtitleRendererSupported(SubtitleRenderer::XY_SUB_FILTER, renderer)) {
                        subrenderer = SubtitleRenderer::XY_SUB_FILTER;
                    }
                }
            }
            VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLE_RENDERER, static_cast<int>(subrenderer)));
        }
        // no break
        case 7:
            // Update the settings after the removal of DirectX 7 renderers
            switch (pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE, VIDRNDT_DS_DEFAULT)) {
                case 3: // VIDRNDT_DS_VMR7WINDOWED
                    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE, VIDRNDT_DS_VMR9WINDOWED));
                    break;
                case 5: // VIDRNDT_DS_VMR7RENDERLESS
                    VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE, VIDRNDT_DS_VMR9RENDERLESS));
                    break;
            }
            if (pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_RMVIDEORENDERERTYPE, VIDRNDT_RM_DEFAULT) == 1) { // VIDRNDT_RM_DX7
                VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_RMVIDEORENDERERTYPE, VIDRNDT_RM_DX9));
            }
            if (pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_QTVIDEORENDERERTYPE, VIDRNDT_QT_DEFAULT) == 1) { // VIDRNDT_QT_DX7
                VERIFY(pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_QTVIDEORENDERERTYPE, VIDRNDT_QT_DX9));
            }
        // no break
        default:
            pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_R_VERSION, APPSETTINGS_VERSION);
    }
}
