/*
 * (C) 2008-2013, 2015 see Authors.txt
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

#include <Windows.h>
#include <tchar.h>

#include "mpciconlib.h"

int main()
{
    return 0;
}

extern "C" __declspec(dllexport) UINT GetIconLibVersion()
{
    return ICON_LIB_VERSION;
}

extern "C" __declspec(dllexport) int GetIconIndex(LPCTSTR ext)
{
    int iconIndex = IDI_NONE;

    if (_tcsicmp(ext, _T(".3g2")) == 0) {
        iconIndex = IDI_MOV_ICON;
    } else if (_tcsicmp(ext, _T(".3ga")) == 0) {
        iconIndex = IDI_MP4_ICON;
    } else if (_tcsicmp(ext, _T(".3gp")) == 0) {
        iconIndex = IDI_MP4_ICON;
    } else if (_tcsicmp(ext, _T(".3gp2")) == 0) {
        iconIndex = IDI_MOV_ICON;
    } else if (_tcsicmp(ext, _T(".3gpp")) == 0) {
        iconIndex = IDI_MP4_ICON;
    } else if (_tcsicmp(ext, _T(".aac")) == 0) {
        iconIndex = IDI_AAC_ICON;
    } else if (_tcsicmp(ext, _T(".ac3")) == 0) {
        iconIndex = IDI_AC3_ICON;
    } else if (_tcsicmp(ext, _T(".aif")) == 0) {
        iconIndex = IDI_AIFF_ICON;
    } else if (_tcsicmp(ext, _T(".aifc")) == 0) {
        iconIndex = IDI_AIFF_ICON;
    } else if (_tcsicmp(ext, _T(".aiff")) == 0) {
        iconIndex = IDI_AIFF_ICON;
    } else if (_tcsicmp(ext, _T(".alac")) == 0) {
        iconIndex = IDI_ALAC_ICON;
    } else if (_tcsicmp(ext, _T(".amr")) == 0) {
        iconIndex = IDI_AMR_ICON;
    } else if (_tcsicmp(ext, _T(".amv")) == 0) {
        iconIndex = IDI_OTHER_ICON;
    } else if (_tcsicmp(ext, _T(".aob")) == 0) {
        iconIndex = IDI_OTHER_ICON;
    } else if (_tcsicmp(ext, _T(".ape")) == 0) {
        iconIndex = IDI_APE_ICON;
    } else if (_tcsicmp(ext, _T(".apl")) == 0) {
        iconIndex = IDI_APE_ICON;
    } else if (_tcsicmp(ext, _T(".asf")) == 0) {
        iconIndex = IDI_WMV_ICON;
    } else if (_tcsicmp(ext, _T(".asx")) == 0) {
        iconIndex = IDI_PLAYLIST_ICON;
    } else if (_tcsicmp(ext, _T(".au")) == 0) {
        iconIndex = IDI_AU_ICON;
    } else if (_tcsicmp(ext, _T(".avi")) == 0) {
        iconIndex = IDI_AVI_ICON;
    } else if (_tcsicmp(ext, _T(".bdmv")) == 0) {
        iconIndex = IDI_PLAYLIST_ICON;
    } else if (_tcsicmp(ext, _T(".bik")) == 0) {
        iconIndex = IDI_BINK_ICON;
    } else if (_tcsicmp(ext, _T(".cda")) == 0) {
        iconIndex = IDI_CDA_ICON;
    } else if (_tcsicmp(ext, _T(".divx")) == 0) {
        iconIndex = IDI_OTHER_ICON;
    } else if (_tcsicmp(ext, _T(".dsa")) == 0) {
        iconIndex = IDI_DSM_ICON;
    } else if (_tcsicmp(ext, _T(".dsm")) == 0) {
        iconIndex = IDI_DSM_ICON;
    } else if (_tcsicmp(ext, _T(".dss")) == 0) {
        iconIndex = IDI_DSM_ICON;
    } else if (_tcsicmp(ext, _T(".dsv")) == 0) {
        iconIndex = IDI_DSM_ICON;
    } else if (_tcsicmp(ext, _T(".dts")) == 0) {
        iconIndex = IDI_DTS_ICON;
    } else if (_tcsicmp(ext, _T(".dtshd")) == 0) {
        iconIndex = IDI_DTS_ICON;
    } else if (_tcsicmp(ext, _T(".dtsma")) == 0) {
        iconIndex = IDI_DTS_ICON;
    } else if (_tcsicmp(ext, _T(".evo")) == 0) {
        iconIndex = IDI_MPEG_ICON;
    } else if (_tcsicmp(ext, _T(".f4v")) == 0) {
        iconIndex = IDI_FLV_ICON;
    } else if (_tcsicmp(ext, _T(".flac")) == 0) {
        iconIndex = IDI_FLAC_ICON;
    } else if (_tcsicmp(ext, _T(".flc")) == 0) {
        iconIndex = IDI_FLIC_ICON;
    } else if (_tcsicmp(ext, _T(".fli")) == 0) {
        iconIndex = IDI_FLIC_ICON;
    } else if (_tcsicmp(ext, _T(".flic")) == 0) {
        iconIndex = IDI_FLIC_ICON;
    } else if (_tcsicmp(ext, _T(".flv")) == 0) {
        iconIndex = IDI_FLV_ICON;
    } else if (_tcsicmp(ext, _T(".hdmov")) == 0) {
        iconIndex = IDI_MP4_ICON;
    } else if (_tcsicmp(ext, _T(".iflv")) == 0) {
        iconIndex = IDI_FLV_ICON;
    } else if (_tcsicmp(ext, _T(".ifo")) == 0) {
        iconIndex = IDI_IFO_ICON;
    } else if (_tcsicmp(ext, _T(".ivf")) == 0) {
        iconIndex = IDI_IVF_ICON;
    } else if (_tcsicmp(ext, _T(".m1a")) == 0) {
        iconIndex = IDI_MPA_ICON;
    } else if (_tcsicmp(ext, _T(".m1v")) == 0) {
        iconIndex = IDI_MPEG_ICON;
    } else if (_tcsicmp(ext, _T(".m2a")) == 0) {
        iconIndex = IDI_MPA_ICON;
    } else if (_tcsicmp(ext, _T(".m2p")) == 0) {
        iconIndex = IDI_MPEG_ICON;
    } else if (_tcsicmp(ext, _T(".m2t")) == 0) {
        iconIndex = IDI_TS_ICON;
    } else if (_tcsicmp(ext, _T(".m2ts")) == 0) {
        iconIndex = IDI_TS_ICON;
    } else if (_tcsicmp(ext, _T(".m2v")) == 0) {
        iconIndex = IDI_MPEG_ICON;
    } else if (_tcsicmp(ext, _T(".m3u")) == 0) {
        iconIndex = IDI_PLAYLIST_ICON;
    } else if (_tcsicmp(ext, _T(".m3u8")) == 0) {
        iconIndex = IDI_PLAYLIST_ICON;
    } else if (_tcsicmp(ext, _T(".m4a")) == 0) {
        iconIndex = IDI_AAC_ICON;
    } else if (_tcsicmp(ext, _T(".m4b")) == 0) {
        iconIndex = IDI_AAC_ICON;
    } else if (_tcsicmp(ext, _T(".m4r")) == 0) {
        iconIndex = IDI_AAC_ICON;
    } else if (_tcsicmp(ext, _T(".m4v")) == 0) {
        iconIndex = IDI_MP4_ICON;
    } else if (_tcsicmp(ext, _T(".mid")) == 0) {
        iconIndex = IDI_MIDI_ICON;
    } else if (_tcsicmp(ext, _T(".midi")) == 0) {
        iconIndex = IDI_MIDI_ICON;
    } else if (_tcsicmp(ext, _T(".mka")) == 0) {
        iconIndex = IDI_MKA_ICON;
    } else if (_tcsicmp(ext, _T(".mkv")) == 0) {
        iconIndex = IDI_MKV_ICON;
    } else if (_tcsicmp(ext, _T(".mlp")) == 0) {
        iconIndex = IDI_OTHER_ICON;
    } else if (_tcsicmp(ext, _T(".mov")) == 0) {
        iconIndex = IDI_MOV_ICON;
    } else if (_tcsicmp(ext, _T(".mp2")) == 0) {
        iconIndex = IDI_MPA_ICON;
    } else if (_tcsicmp(ext, _T(".mp2v")) == 0) {
        iconIndex = IDI_MPEG_ICON;
    } else if (_tcsicmp(ext, _T(".mp3")) == 0) {
        iconIndex = IDI_MP3_ICON;
    } else if (_tcsicmp(ext, _T(".mp4")) == 0) {
        iconIndex = IDI_MP4_ICON;
    } else if (_tcsicmp(ext, _T(".mp4v")) == 0) {
        iconIndex = IDI_MP4_ICON;
    } else if (_tcsicmp(ext, _T(".mpa")) == 0) {
        iconIndex = IDI_MPA_ICON;
    } else if (_tcsicmp(ext, _T(".mpc")) == 0) {
        iconIndex = IDI_MPC_ICON;
    } else if (_tcsicmp(ext, _T(".mpcpl")) == 0) {
        iconIndex = IDI_PLAYLIST_ICON;
    } else if (_tcsicmp(ext, _T(".mpe")) == 0) {
        iconIndex = IDI_MPEG_ICON;
    } else if (_tcsicmp(ext, _T(".mpeg")) == 0) {
        iconIndex = IDI_MPEG_ICON;
    } else if (_tcsicmp(ext, _T(".mpg")) == 0) {
        iconIndex = IDI_MPEG_ICON;
    } else if (_tcsicmp(ext, _T(".mpls")) == 0) {
        iconIndex = IDI_PLAYLIST_ICON;
    } else if (_tcsicmp(ext, _T(".mpv2")) == 0) {
        iconIndex = IDI_MPEG_ICON;
    } else if (_tcsicmp(ext, _T(".mpv4")) == 0) {
        iconIndex = IDI_MP4_ICON;
    } else if (_tcsicmp(ext, _T(".mts")) == 0) {
        iconIndex = IDI_TS_ICON;
    } else if (_tcsicmp(ext, _T(".ofr")) == 0) {
        iconIndex = IDI_OFR_ICON;
    } else if (_tcsicmp(ext, _T(".ofs")) == 0) {
        iconIndex = IDI_OFR_ICON;
    } else if (_tcsicmp(ext, _T(".oga")) == 0) {
        iconIndex = IDI_OGG_ICON;
    } else if (_tcsicmp(ext, _T(".ogg")) == 0) {
        iconIndex = IDI_OGG_ICON;
    } else if (_tcsicmp(ext, _T(".ogm")) == 0) {
        iconIndex = IDI_OGM_ICON;
    } else if (_tcsicmp(ext, _T(".ogv")) == 0) {
        iconIndex = IDI_OGM_ICON;
    } else if (_tcsicmp(ext, _T(".opus")) == 0) {
        iconIndex = IDI_OTHER_ICON;
    } else if (_tcsicmp(ext, _T(".pls")) == 0) {
        iconIndex = IDI_PLAYLIST_ICON;
    } else if (_tcsicmp(ext, _T(".pva")) == 0) {
        iconIndex = IDI_MPEG_ICON;
    } else if (_tcsicmp(ext, _T(".ra")) == 0) {
        iconIndex = IDI_RA_ICON;
    } else if (_tcsicmp(ext, _T(".ram")) == 0) {
        iconIndex = IDI_RM_ICON;
    } else if (_tcsicmp(ext, _T(".rm")) == 0) {
        iconIndex = IDI_RM_ICON;
    } else if (_tcsicmp(ext, _T(".rmi")) == 0) {
        iconIndex = IDI_MIDI_ICON;
    } else if (_tcsicmp(ext, _T(".rmm")) == 0) {
        iconIndex = IDI_RM_ICON;
    } else if (_tcsicmp(ext, _T(".rmvb")) == 0) {
        iconIndex = IDI_OTHER_ICON;
    } else if (_tcsicmp(ext, _T(".rp")) == 0) {
        iconIndex = IDI_RT_ICON;
    } else if (_tcsicmp(ext, _T(".rt")) == 0) {
        iconIndex = IDI_RT_ICON;
    } else if (_tcsicmp(ext, _T(".smil")) == 0) {
        iconIndex = IDI_RT_ICON;
    } else if (_tcsicmp(ext, _T(".smk")) == 0) {
        iconIndex = IDI_SMK_ICON;
    } else if (_tcsicmp(ext, _T(".snd")) == 0) {
        iconIndex = IDI_AU_ICON;
    } else if (_tcsicmp(ext, _T(".swf")) == 0) {
        iconIndex = IDI_SWF_ICON;
    } else if (_tcsicmp(ext, _T(".tp")) == 0) {
        iconIndex = IDI_TS_ICON;
    } else if (_tcsicmp(ext, _T(".trp")) == 0) {
        iconIndex = IDI_TS_ICON;
    } else if (_tcsicmp(ext, _T(".ts")) == 0) {
        iconIndex = IDI_TS_ICON;
    } else if (_tcsicmp(ext, _T(".rec")) == 0) {
        iconIndex = IDI_TS_ICON;
    } else if (_tcsicmp(ext, _T(".tak")) == 0) {
        iconIndex = IDI_OTHER_ICON;
    } else if (_tcsicmp(ext, _T(".tta")) == 0) {
        iconIndex = IDI_TTA_ICON;
    } else if (_tcsicmp(ext, _T(".vob")) == 0) {
        iconIndex = IDI_VOB_ICON;
    } else if (_tcsicmp(ext, _T(".wav")) == 0) {
        iconIndex = IDI_WAV_ICON;
    } else if (_tcsicmp(ext, _T(".wax")) == 0) {
        iconIndex = IDI_PLAYLIST_ICON;
    } else if (_tcsicmp(ext, _T(".webm")) == 0) {
        iconIndex = IDI_WEBM_ICON;
    } else if (_tcsicmp(ext, _T(".wm")) == 0) {
        iconIndex = IDI_WMV_ICON;
    } else if (_tcsicmp(ext, _T(".wma")) == 0) {
        iconIndex = IDI_WMA_ICON;
    } else if (_tcsicmp(ext, _T(".wmp")) == 0) {
        iconIndex = IDI_WMV_ICON;
    } else if (_tcsicmp(ext, _T(".wmv")) == 0) {
        iconIndex = IDI_WMV_ICON;
    } else if (_tcsicmp(ext, _T(".wmx")) == 0) {
        iconIndex = IDI_PLAYLIST_ICON;
    } else if (_tcsicmp(ext, _T(".wv")) == 0) {
        iconIndex = IDI_WV_ICON;
    } else if (_tcsicmp(ext, _T(".wvx")) == 0) {
        iconIndex = IDI_PLAYLIST_ICON;
    }

    return iconIndex;
}
