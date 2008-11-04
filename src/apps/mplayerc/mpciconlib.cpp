#include "mpciconlib.h"
#include <afx.h>

int main()
{
	return 0;
}

int get_icon_index(CString ext)
{
	int iconindex = -1;

	     if(ext.CompareNoCase(_T(".aac")) == 0)    iconindex = IDI_AAC_ICON;
	//else if(ext.CompareNoCase(_T(".aiff")) == 0)   iconindex = IDI_AIFF_ICON;
	else if(ext.CompareNoCase(_T(".alac")) == 0)   iconindex = IDI_ALAC_ICON;
	//else if(ext.CompareNoCase(_T(".au")) == 0)     iconindex = IDI_AU_ICON;
	else if(ext.CompareNoCase(_T(".avi")) == 0)    iconindex = IDI_AVI_ICON;
	//else if(ext.CompareNoCase(_T(".bik")) == 0)    iconindex = IDI_BIK_ICON;
	else if(ext.CompareNoCase(_T(".cda")) == 0)    iconindex = IDI_CDA_ICON;
	//else if(ext.CompareNoCase(_T(".d2v")) == 0)    iconindex = IDI_D2V_ICON;
	//else if(ext.CompareNoCase(_T(".drc")) == 0)    iconindex = IDI_DRC_ICON;
	//else if(ext.CompareNoCase(_T(".dsm")) == 0)    iconindex = IDI_DSM_ICON;
	//else if(ext.CompareNoCase(_T(".dvda")) == 0)   iconindex = IDI_DVDA_ICON;
	//else if(ext.CompareNoCase(_T(".dvdf")) == 0)   iconindex = IDI_DVDF_ICON;
	else if(ext.CompareNoCase(_T(".flac")) == 0)   iconindex = IDI_FLAC_ICON;
	//else if(ext.CompareNoCase(_T(".flic")) == 0)   iconindex = IDI_FLIC_ICON;
	else if(ext.CompareNoCase(_T(".flv")) == 0)    iconindex = IDI_FLV_ICON;
	//else if(ext.CompareNoCase(_T(".ivf")) == 0)    iconindex = IDI_IVF_ICON;
	//else if(ext.CompareNoCase(_T(".mid")) == 0)    iconindex = IDI_MID_ICON;
	else if(ext.CompareNoCase(_T(".mka")) == 0)    iconindex = IDI_MKA_ICON;
	else if(ext.CompareNoCase(_T(".mkv")) == 0)    iconindex = IDI_MKV_ICON;
	else if(ext.CompareNoCase(_T(".mov")) == 0)    iconindex = IDI_MOV_ICON;
	else if(ext.CompareNoCase(_T(".mp3")) == 0)    iconindex = IDI_MP3_ICON;
	else if(ext.CompareNoCase(_T(".mp4")) == 0)    iconindex = IDI_MP4_ICON;
	else if(ext.CompareNoCase(_T(".mpa")) == 0)    iconindex = IDI_MPA_ICON;
	else if(ext.CompareNoCase(_T(".mpc")) == 0)    iconindex = IDI_MPC_ICON;
	else if(ext.CompareNoCase(_T(".mpeg")) == 0)   iconindex = IDI_MPG_ICON;
	else if(ext.CompareNoCase(_T(".mpg")) == 0)    iconindex = IDI_MPG_ICON;
	else if(ext.CompareNoCase(_T(".ogg")) == 0)    iconindex = IDI_OGG_ICON;
	else if(ext.CompareNoCase(_T(".ogm")) == 0)    iconindex = IDI_OGM_ICON;
	else if(ext.CompareNoCase(_T(".pls")) == 0)    iconindex = IDI_PLS_ICON;
	else if(ext.CompareNoCase(_T(".ra")) == 0)     iconindex = IDI_RA_ICON;
	//else if(ext.CompareNoCase(_T(".ratdvd")) == 0) iconindex = IDI_RATDVD_ICON;
	else if(ext.CompareNoCase(_T(".rm")) == 0)     iconindex = IDI_RM_ICON;
	//else if(ext.CompareNoCase(_T(".roq")) == 0)    iconindex = IDI_ROQ_ICON;
	//else if(ext.CompareNoCase(_T(".rt")) == 0)     iconindex = IDI_RT_ICON;
	else if(ext.CompareNoCase(_T(".wav")) == 0)    iconindex = IDI_WAV_ICON;
	else if(ext.CompareNoCase(_T(".wma")) == 0)    iconindex = IDI_WMA_ICON;
	else if(ext.CompareNoCase(_T(".wmv")) == 0)    iconindex = IDI_WMV_ICON;

	return iconindex;
}
