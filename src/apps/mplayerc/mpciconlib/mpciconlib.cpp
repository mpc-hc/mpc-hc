#include "mpciconlib.h"
#include <afx.h>

int main()
{
	return 0;
}

int get_icon_index(CString ext)
{
	int iconindex = -1;

	if(ext.CompareNoCase(_T(".3g2")) == 0) {
		iconindex = IDI_MP4_ICON;
	} else if(ext.CompareNoCase(_T(".3gp")) == 0) {
		iconindex = IDI_MP4_ICON;
	} else if(ext.CompareNoCase(_T(".3gp2")) == 0) {
		iconindex = IDI_MP4_ICON;
	} else if(ext.CompareNoCase(_T(".3gpp")) == 0) {
		iconindex = IDI_MP4_ICON;
	} else if(ext.CompareNoCase(_T(".aac")) == 0) {
		iconindex = IDI_AAC_ICON;
	} else if(ext.CompareNoCase(_T(".ac3")) == 0) {
		iconindex = IDI_DVDA_ICON;
	} else if(ext.CompareNoCase(_T(".aif")) == 0) {
		iconindex = IDI_AIFF_ICON;
	} else if(ext.CompareNoCase(_T(".aifc")) == 0) {
		iconindex = IDI_AIFF_ICON;
	} else if(ext.CompareNoCase(_T(".aiff")) == 0) {
		iconindex = IDI_AIFF_ICON;
	} else if(ext.CompareNoCase(_T(".alac")) == 0) {
		iconindex = IDI_ALAC_ICON;
	} else if(ext.CompareNoCase(_T(".amr")) == 0) {
		iconindex = IDI_OTHER_ICON;
	} else if(ext.CompareNoCase(_T(".amv")) == 0) {
		iconindex = IDI_OTHER_ICON;
	} else if(ext.CompareNoCase(_T(".ape")) == 0) {
		iconindex = IDI_NONE;
	} else if(ext.CompareNoCase(_T(".asf")) == 0) {
		iconindex = IDI_WMV_ICON;
	} else if(ext.CompareNoCase(_T(".asx")) == 0) {
		iconindex = IDI_PLC_ICON;
	} else if(ext.CompareNoCase(_T(".au")) == 0) {
		iconindex = IDI_AU_ICON;
	} else if(ext.CompareNoCase(_T(".avi")) == 0) {
		iconindex = IDI_AVI_ICON;
	} else if(ext.CompareNoCase(_T(".bik")) == 0) {
		iconindex = IDI_BIK_ICON;
	} else if(ext.CompareNoCase(_T(".cda")) == 0) {
		iconindex = IDI_CDA_ICON;
	} else if(ext.CompareNoCase(_T(".d2v")) == 0) {
		iconindex = IDI_D2V_ICON;
	} else if(ext.CompareNoCase(_T(".divx")) == 0) {
		iconindex = IDI_AVI_ICON;
	} else if(ext.CompareNoCase(_T(".dsa")) == 0) {
		iconindex = IDI_DSM_ICON;
	} else if(ext.CompareNoCase(_T(".dsm")) == 0) {
		iconindex = IDI_DSM_ICON;
	} else if(ext.CompareNoCase(_T(".dss")) == 0) {
		iconindex = IDI_DSM_ICON;
	} else if(ext.CompareNoCase(_T(".dsv")) == 0) {
		iconindex = IDI_DSM_ICON;
	} else if(ext.CompareNoCase(_T(".dts")) == 0) {
		iconindex = IDI_DVDA_ICON;
	} else if(ext.CompareNoCase(_T(".evo")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".flac")) == 0) {
		iconindex = IDI_FLAC_ICON;
	} else if(ext.CompareNoCase(_T(".flic")) == 0) {
		iconindex = IDI_FLIC_ICON;
	} else if(ext.CompareNoCase(_T(".flv")) == 0) {
		iconindex = IDI_FLV_ICON;
	} else if(ext.CompareNoCase(_T(".iflv")) == 0) {
		iconindex = IDI_FLV_ICON;
	} else if(ext.CompareNoCase(_T(".f4v")) == 0) {
		iconindex = IDI_FLV_ICON;
	} else if(ext.CompareNoCase(_T(".hdmov")) == 0) {
		iconindex = IDI_MP4_ICON;
	} else if(ext.CompareNoCase(_T(".ifo")) == 0) {
		iconindex = IDI_DVDF_ICON;
	} else if(ext.CompareNoCase(_T(".ivf")) == 0) {
		iconindex = IDI_IVF_ICON;
	} else if(ext.CompareNoCase(_T(".m1a")) == 0) {
		iconindex = IDI_MPA_ICON;
	} else if(ext.CompareNoCase(_T(".m1v")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".m2a")) == 0) {
		iconindex = IDI_MPA_ICON;
	} else if(ext.CompareNoCase(_T(".m2t")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".m2ts")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".m2v")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".m3u")) == 0) {
		iconindex = IDI_PLC_ICON;
	} else if(ext.CompareNoCase(_T(".bdmv")) == 0) {
		iconindex = IDI_PLC_ICON;
	} else if(ext.CompareNoCase(_T(".m4a")) == 0) {
		iconindex = IDI_AAC_ICON;
	} else if(ext.CompareNoCase(_T(".m4b")) == 0) {
		iconindex = IDI_AAC_ICON;
	} else if(ext.CompareNoCase(_T(".m4v")) == 0) {
		iconindex = IDI_MP4_ICON;
	} else if(ext.CompareNoCase(_T(".mid")) == 0) {
		iconindex = IDI_MID_ICON;
	} else if(ext.CompareNoCase(_T(".midi")) == 0) {
		iconindex = IDI_MID_ICON;
	} else if(ext.CompareNoCase(_T(".mka")) == 0) {
		iconindex = IDI_MKA_ICON;
	} else if(ext.CompareNoCase(_T(".mkv")) == 0) {
		iconindex = IDI_MKV_ICON;
	} else if(ext.CompareNoCase(_T(".mov")) == 0) {
		iconindex = IDI_MOV_ICON;
	} else if(ext.CompareNoCase(_T(".mp2")) == 0) {
		iconindex = IDI_MPC_ICON;
	} else if(ext.CompareNoCase(_T(".mp2v")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".mp3")) == 0) {
		iconindex = IDI_MP3_ICON;
	} else if(ext.CompareNoCase(_T(".mp4")) == 0) {
		iconindex = IDI_MP4_ICON;
	} else if(ext.CompareNoCase(_T(".mpa")) == 0) {
		iconindex = IDI_MPA_ICON;
	} else if(ext.CompareNoCase(_T(".mpc")) == 0) {
		iconindex = IDI_MPC_ICON;
	} else if(ext.CompareNoCase(_T(".mpcpl")) == 0) {
		iconindex = IDI_PLC_ICON;
	} else if(ext.CompareNoCase(_T(".mpe")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".mpeg")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".mpg")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".mpv2")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".mts")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".oga")) == 0) {
		iconindex = IDI_OGG_ICON;
	} else if(ext.CompareNoCase(_T(".ogg")) == 0) {
		iconindex = IDI_OGG_ICON;
	} else if(ext.CompareNoCase(_T(".ogm")) == 0) {
		iconindex = IDI_OGM_ICON;
	} else if(ext.CompareNoCase(_T(".ogv")) == 0) {
		iconindex = IDI_OGM_ICON;
	} else if(ext.CompareNoCase(_T(".pls")) == 0) {
		iconindex = IDI_PLC_ICON;
	} else if(ext.CompareNoCase(_T(".pva")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".pss")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".qt")) == 0) {
		iconindex = IDI_MOV_ICON;
	} else if(ext.CompareNoCase(_T(".ra")) == 0) {
		iconindex = IDI_RA_ICON;
	} else if(ext.CompareNoCase(_T(".ram")) == 0) {
		iconindex = IDI_RM_ICON;
	} else if(ext.CompareNoCase(_T(".ratdvd")) == 0) {
		iconindex = IDI_RATDVD_ICON;
	} else if(ext.CompareNoCase(_T(".rm")) == 0) {
		iconindex = IDI_RM_ICON;
	} else if(ext.CompareNoCase(_T(".rmi")) == 0) {
		iconindex = IDI_MID_ICON;
	} else if(ext.CompareNoCase(_T(".rmm")) == 0) {
		iconindex = IDI_RM_ICON;
	} else if(ext.CompareNoCase(_T(".rmvb")) == 0) {
		iconindex = IDI_RM_ICON;
	} else if(ext.CompareNoCase(_T(".rp")) == 0) {
		iconindex = IDI_RT_ICON;
	} else if(ext.CompareNoCase(_T(".rpm")) == 0) {
		iconindex = IDI_RM_ICON;
	} else if(ext.CompareNoCase(_T(".rt")) == 0) {
		iconindex = IDI_RT_ICON;
	} else if(ext.CompareNoCase(_T(".smi")) == 0) {
		iconindex = IDI_RM_ICON;
	} else if(ext.CompareNoCase(_T(".smil")) == 0) {
		iconindex = IDI_RM_ICON;
	} else if(ext.CompareNoCase(_T(".smk")) == 0) {
		iconindex = IDI_OTHER_ICON;
	} else if(ext.CompareNoCase(_T(".snd")) == 0) {
		iconindex = IDI_AU_ICON;
	} else if(ext.CompareNoCase(_T(".tp")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".tpr")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".ts")) == 0) {
		iconindex = IDI_MPG_ICON;
	} else if(ext.CompareNoCase(_T(".vob")) == 0) {
		iconindex = IDI_DVDF_ICON;
	} else if(ext.CompareNoCase(_T(".vp6")) == 0) {
		iconindex = IDI_OTHER_ICON;
	} else if(ext.CompareNoCase(_T(".wav")) == 0) {
		iconindex = IDI_WAV_ICON;
	} else if(ext.CompareNoCase(_T(".wax")) == 0) {
		iconindex = IDI_PLC_ICON;
	} else if(ext.CompareNoCase(_T(".webm")) == 0) {
		iconindex = IDI_OTHER_ICON;
	} else if(ext.CompareNoCase(_T(".wm")) == 0) {
		iconindex = IDI_WMV_ICON;
	} else if(ext.CompareNoCase(_T(".wma")) == 0) {
		iconindex = IDI_WMA_ICON;
	} else if(ext.CompareNoCase(_T(".wmp")) == 0) {
		iconindex = IDI_WMV_ICON;
	} else if(ext.CompareNoCase(_T(".wmv")) == 0) {
		iconindex = IDI_WMV_ICON;
	} else if(ext.CompareNoCase(_T(".wmx")) == 0) {
		iconindex = IDI_PLC_ICON;
	} else if(ext.CompareNoCase(_T(".wv")) == 0) {
		iconindex = IDI_NONE;
	} else if(ext.CompareNoCase(_T(".wvx")) == 0) {
		iconindex = IDI_PLC_ICON;
	}

	return iconindex;
}
