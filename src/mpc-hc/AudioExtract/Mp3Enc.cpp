/*

*  (C) 2017 see Authors.txt
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
#include "../../thirdparty/lame/include/lame.h"
#include "MP3Enc.h"
#ifdef	__cplusplus
extern "C" {
#endif
#define         Min(A, B)       ((A) < (B) ? (A) : (B))
#define         Max(A, B)       ((A) > (B) ? (A) : (B))

#define _RELEASEDEBUG 0


const BYTE MAJORVERSION = 1;
const BYTE MINORVERSION = 32;

// Local variables
static DWORD				dwSampleBufferSize = 0;
static HMODULE				gs_hModule = NULL;
static BOOL					gs_bLogFile = FALSE;
static lame_global_flags*	gfp_save = NULL;

static void PresetOptions(lame_global_flags *gfp, LONG myPreset);

static void PresetOptions( lame_global_flags *gfp, LONG myPreset )
{
    switch (myPreset)
    {
        /*-1*/case LQP_NOPRESET:
            break;

        /*0*/case LQP_NORMAL_QUALITY:
            /*	lame_set_quality( gfp, 5 );*/
            break;

        /*1*/case LQP_LOW_QUALITY:
             lame_set_quality( gfp, 9 );
             break;

        /*2*/case LQP_HIGH_QUALITY:
             lame_set_quality( gfp, 2 );
             break;

        /*3*/case LQP_VOICE_QUALITY:				// --voice flag for experimental voice mode
             lame_set_mode( gfp, MONO );
             lame_set_preset( gfp, 56);
             break;

        /*4*/case LQP_R3MIX:					// --R3MIX
             lame_set_preset( gfp, R3MIX);
             break;

        /*5*/case LQP_VERYHIGH_QUALITY:
             lame_set_quality( gfp, 0 );
             break;

        /*6*/case LQP_STANDARD:				// --PRESET STANDARD
            lame_set_preset( gfp, STANDARD);
            break;

        /*7*/case LQP_FAST_STANDARD:				// --PRESET FAST STANDARD
            lame_set_preset( gfp, STANDARD_FAST);
            break;

        /*8*/case LQP_EXTREME:				// --PRESET EXTREME
            lame_set_preset( gfp, EXTREME);
            break;

        /*9*/case LQP_FAST_EXTREME:				// --PRESET FAST EXTREME:
            lame_set_preset( gfp, EXTREME_FAST);
            break;

        /*10*/case LQP_INSANE:				// --PRESET INSANE
            lame_set_preset( gfp, INSANE);
            break;

        /*11*/case LQP_ABR:					// --PRESET ABR
            // handled in beInitStream
            break;

        /*12*/case LQP_CBR:					// --PRESET CBR
            // handled in beInitStream
            break;

        /*13*/case LQP_MEDIUM:					// --PRESET MEDIUM
            lame_set_preset( gfp, MEDIUM);
            break;

        /*14*/case LQP_FAST_MEDIUM:					// --PRESET FAST MEDIUM
            lame_set_preset( gfp, MEDIUM_FAST);
            break;

        /*1000*/case LQP_PHONE:
            lame_set_mode( gfp, MONO );
            lame_set_preset( gfp, 16);
            break;

        /*2000*/case LQP_SW:
            lame_set_mode( gfp, MONO );
            lame_set_preset( gfp, 24);
            break;

        /*3000*/case LQP_AM:
            lame_set_mode( gfp, MONO );
            lame_set_preset( gfp, 40);
            break;

        /*4000*/case LQP_FM:
            lame_set_preset( gfp, 112);
            break;

        /*5000*/case LQP_VOICE:
            lame_set_mode( gfp, MONO );
            lame_set_preset( gfp, 56);
            break;

        /*6000*/case LQP_RADIO:
            lame_set_preset( gfp, 112);
            break;

        /*7000*/case LQP_TAPE:
            lame_set_preset( gfp, 112);
            break;

        /*8000*/case LQP_HIFI:
            lame_set_preset( gfp, 160);
            break;

        /*9000*/case LQP_CD:
            lame_set_preset( gfp, 192);
            break;

        /*10000*/case LQP_STUDIO:
            lame_set_preset( gfp, 256);
            break;

    }
}

BE_ERR	beInitStream(PBE_CONFIG pbeConfig, PDWORD dwSamples, PDWORD dwBufferSize, PHBE_STREAM phbeStream)
{
	int actual_bitrate;
 
	BE_CONFIG			lameConfig = { 0, };
	int					nInitReturn = 0;
	lame_global_flags*	gfp = NULL;

	// Init the global flags structure
	gfp = lame_init();
	*phbeStream = (HBE_STREAM)gfp;

	// clear out structure
	memset(&lameConfig, 0x00, CURRENT_STRUCT_SIZE);

	// Check if this is a regular BLADE_ENCODER header
	if (pbeConfig->dwConfig != BE_CONFIG_LAME)
	{
		int nCRC = pbeConfig->format.mp3.bCRC;
		int nVBR = (nCRC >> 12) & 0x0F;

		// Copy parameter from old Blade structure
		lameConfig.format.LHV1.dwSampleRate = pbeConfig->format.mp3.dwSampleRate;
		//for low bitrates, LAME will automatically downsample for better
		//sound quality.  Forcing output samplerate = input samplerate is not a good idea 
		//unless the user specifically requests it:
		//lameConfig.format.LHV1.dwReSampleRate=pbeConfig->format.mp3.dwSampleRate;
		lameConfig.format.LHV1.nMode = (pbeConfig->format.mp3.byMode & 0x0F);
		lameConfig.format.LHV1.dwBitrate = pbeConfig->format.mp3.wBitrate;
		lameConfig.format.LHV1.bPrivate = pbeConfig->format.mp3.bPrivate;
		lameConfig.format.LHV1.bOriginal = pbeConfig->format.mp3.bOriginal;
		lameConfig.format.LHV1.bCRC = nCRC & 0x01;
		lameConfig.format.LHV1.bCopyright = pbeConfig->format.mp3.bCopyright;

		// Fill out the unknowns
		lameConfig.format.LHV1.dwStructSize = CURRENT_STRUCT_SIZE;
		lameConfig.format.LHV1.dwStructVersion = CURRENT_STRUCT_VERSION;

		// Get VBR setting from fourth nibble
		if (nVBR>0)
		{
			lameConfig.format.LHV1.bWriteVBRHeader = TRUE;
			lameConfig.format.LHV1.bEnableVBR = TRUE;
			lameConfig.format.LHV1.nVBRQuality = nVBR - 1;
		}

		// Get Quality from third nibble
		lameConfig.format.LHV1.nPreset = ((nCRC >> 8) & 0x0F);

	}
	else
	{
		// Copy the parameters
		memcpy(&lameConfig, pbeConfig, pbeConfig->format.LHV1.dwStructSize);
	}

	// --------------- Set arguments to LAME encoder -------------------------

	// Set input sample frequency
	lame_set_in_samplerate(gfp, lameConfig.format.LHV1.dwSampleRate);

	// disable INFO/VBR tag by default.  
	// if this tag is used, the calling program must call beWriteVBRTag()
	// after encoding.  But the original DLL documentation does not 
	// require the 
	// app to call beWriteVBRTag() unless they have specifically
	// set LHV1.bWriteVBRHeader=TRUE.  Thus the default setting should
	// be disabled.  
	lame_set_bWriteVbrTag(gfp, 0);

	//2001-12-18 Dibrom's ABR preset stuff

	if (lameConfig.format.LHV1.nPreset == LQP_ABR)		// --ALT-PRESET ABR
	{
		actual_bitrate = lameConfig.format.LHV1.dwVbrAbr_bps / 1000;

		// limit range
		if (actual_bitrate > 320)
		{
			actual_bitrate = 320;
		}

		if (actual_bitrate < 8)
		{
			actual_bitrate = 8;
		}

		lame_set_preset(gfp, actual_bitrate);
	}

	// end Dibrom's ABR preset 2001-12-18 ****** START OF CBR

	if (lameConfig.format.LHV1.nPreset == LQP_CBR)		// --ALT-PRESET CBR
	{
		actual_bitrate = lameConfig.format.LHV1.dwBitrate;
		lame_set_preset(gfp, actual_bitrate);
		lame_set_VBR(gfp, vbr_off);
	}

	// end Dibrom's CBR preset 2001-12-18

	// The following settings only used when preset is not one of the LAME QUALITY Presets
	if ((int)lameConfig.format.LHV1.nPreset < (int)LQP_STANDARD)
	{
		switch (lameConfig.format.LHV1.nMode)
		{
		case BE_MP3_MODE_STEREO:
			lame_set_mode(gfp, STEREO);
			lame_set_num_channels(gfp, 2);
			break;
		case BE_MP3_MODE_JSTEREO:
			lame_set_mode(gfp, JOINT_STEREO);
			//lame_set_force_ms( gfp, bForceMS ); // no check box to force this?
			lame_set_num_channels(gfp, 2);
			break;
		case BE_MP3_MODE_MONO:
			lame_set_mode(gfp, MONO);
			lame_set_num_channels(gfp, 1);
			break;
		case BE_MP3_MODE_DUALCHANNEL:
			lame_set_mode(gfp, DUAL_CHANNEL);
			lame_set_num_channels(gfp, 2);
			break;
		default:
		{
			//DebugPrintf("Invalid lameConfig.format.LHV1.nMode, value is %d\n", lameConfig.format.LHV1.nMode);
			return BE_ERR_INVALID_FORMAT_PARAMETERS;
		}
		}

		if (lameConfig.format.LHV1.bEnableVBR)
		{
			/* set VBR quality */
			lame_set_VBR_q(gfp, lameConfig.format.LHV1.nVBRQuality);

			/* select proper VBR method */
			switch (lameConfig.format.LHV1.nVbrMethod)
			{
			case VBR_METHOD_NONE:
				lame_set_VBR(gfp, vbr_off);
				break;

			case VBR_METHOD_DEFAULT:
				lame_set_VBR(gfp, vbr_default);
				break;

			case VBR_METHOD_OLD:
				lame_set_VBR(gfp, vbr_rh);
				break;

			case VBR_METHOD_MTRH:
			case VBR_METHOD_NEW:
				/*
				* the --vbr-mtrh commandline switch is obsolete.
				* now --vbr-mtrh is known as --vbr-new
				*/
				lame_set_VBR(gfp, vbr_mtrh);
				break;

			case VBR_METHOD_ABR:
				lame_set_VBR(gfp, vbr_abr);
				break;

			default:
				/* unsupported VBR method */
				//assert(FALSE);
				break;
			}
		}
		else
		{
			/* use CBR encoding method, so turn off VBR */
			lame_set_VBR(gfp, vbr_off);
		}

		/* Set bitrate.  (CDex users always specify bitrate=Min bitrate when using VBR) */
		lame_set_brate(gfp, lameConfig.format.LHV1.dwBitrate);

		/* check if we have to use ABR, in order to backwards compatible, this
		* condition should still be checked indepedent of the nVbrMethod method
		*/
		if (lameConfig.format.LHV1.dwVbrAbr_bps > 0)
		{
			/* set VBR method to ABR */
			lame_set_VBR(gfp, vbr_abr);

			/* calculate to kbps, round to nearest kbps */
			lame_set_VBR_mean_bitrate_kbps(gfp, (lameConfig.format.LHV1.dwVbrAbr_bps + 500) / 1000);

			/* limit range */
			if (lame_get_VBR_mean_bitrate_kbps(gfp) > 320)
			{
				lame_set_VBR_mean_bitrate_kbps(gfp, 320);
			}

			if (lame_get_VBR_mean_bitrate_kbps(gfp) < 8)
			{
				lame_set_VBR_mean_bitrate_kbps(gfp, 8);
			}
		}

	}

	// First set all the preset options
	if (LQP_NOPRESET != lameConfig.format.LHV1.nPreset)
	{
		PresetOptions(gfp, lameConfig.format.LHV1.nPreset);
	}

	// Set frequency resampling rate, if specified
	if (lameConfig.format.LHV1.dwReSampleRate > 0)
	{
		lame_set_out_samplerate(gfp, lameConfig.format.LHV1.dwReSampleRate);
	}


	switch (lameConfig.format.LHV1.nMode)
	{
	case BE_MP3_MODE_MONO:
		lame_set_mode(gfp, MONO);
		lame_set_num_channels(gfp, 1);
		break;

	default:
		break;
	}


	// Use strict ISO encoding?
	lame_set_strict_ISO(gfp, (lameConfig.format.LHV1.bStrictIso) ? 1 : 0);

	// Set copyright flag?
	if (lameConfig.format.LHV1.bCopyright)
	{
		lame_set_copyright(gfp, 1);
	}

	// Do we have to tag  it as non original 
	if (!lameConfig.format.LHV1.bOriginal)
	{
		lame_set_original(gfp, 0);
	}
	else
	{
		lame_set_original(gfp, 1);
	}

	// Add CRC?
	if (lameConfig.format.LHV1.bCRC)
	{
		lame_set_error_protection(gfp, 1);
	}
	else
	{
		lame_set_error_protection(gfp, 0);
	}

	// Set private bit?
	if (lameConfig.format.LHV1.bPrivate)
	{
		lame_set_extension(gfp, 1);
	}
	else
	{
		lame_set_extension(gfp, 0);
	}


	// Set VBR min bitrate, if specified
	if (lameConfig.format.LHV1.dwBitrate > 0)
	{
		lame_set_VBR_min_bitrate_kbps(gfp, lameConfig.format.LHV1.dwBitrate);
	}

	// Set Maxbitrate, if specified
	if (lameConfig.format.LHV1.dwMaxBitrate > 0)
	{
		lame_set_VBR_max_bitrate_kbps(gfp, lameConfig.format.LHV1.dwMaxBitrate);
	}
	// Set bit resovoir option
	if (lameConfig.format.LHV1.bNoRes)
	{
		lame_set_disable_reservoir(gfp, 1);
	}

	// check if the VBR tag is required
	if (lameConfig.format.LHV1.bWriteVBRHeader)
	{
		lame_set_bWriteVbrTag(gfp, 1);
	}
	else
	{
		lame_set_bWriteVbrTag(gfp, 0);
	}

	// Override Quality setting, use HIGHBYTE = NOT LOWBYTE to be backwards compatible
	if ((lameConfig.format.LHV1.nQuality & 0xFF) ==
		((~(lameConfig.format.LHV1.nQuality >> 8)) & 0xFF))
	{
		lame_set_quality(gfp, lameConfig.format.LHV1.nQuality & 0xFF);
	}

	if (0 != (nInitReturn = lame_init_params(gfp)))
	{
		return nInitReturn;
	}

	//LAME encoding call will accept any number of samples.  
	if (0 == lame_get_version(gfp))
	{
		// For MPEG-II, only 576 samples per frame per channel
		*dwSamples = 576 * lame_get_num_channels(gfp);
	}
	else
	{
		// For MPEG-I, 1152 samples per frame per channel
		*dwSamples = 1152 * lame_get_num_channels(gfp);
	}

	// Set the input sample buffer size, so we know what we can expect
	dwSampleBufferSize = *dwSamples;

	// Set MP3 buffer size, conservative estimate
	*dwBufferSize = (DWORD)(1.25 * (*dwSamples / lame_get_num_channels(gfp)) + 7200);

	// Everything went OK, thus return SUCCESSFUL
	return BE_ERR_SUCCESSFUL;
}


BE_ERR	beEncodeChunk(HBE_STREAM hbeStream, DWORD nSamples,
	PSHORT pSamples, PBYTE pOutput, PDWORD pdwOutput)
{
	// Encode it
	int dwSamples;
	int	nOutputSamples = 0;
	lame_global_flags*	gfp = (lame_global_flags*)hbeStream;

	dwSamples = nSamples / lame_get_num_channels(gfp);

	// old versions of lame_enc.dll required exactly 1152 samples
	// and worked even if nSamples accidently set to 2304 
	// simulate this behavoir:
	if (1 == lame_get_num_channels(gfp) && nSamples == 2304)
	{
		dwSamples /= 2;
	}


	if (1 == lame_get_num_channels(gfp))
	{
		nOutputSamples = lame_encode_buffer(gfp, pSamples, pSamples, dwSamples, pOutput, 0);
	}
	else
	{
		nOutputSamples = lame_encode_buffer_interleaved(gfp, pSamples, dwSamples, pOutput, 0);
	}


	if (nOutputSamples < 0)
	{
		*pdwOutput = 0;
		return BE_ERR_BUFFER_TOO_SMALL;
	}
	else
	{
		*pdwOutput = (DWORD)nOutputSamples;
	}

	return BE_ERR_SUCCESSFUL;
}


BE_ERR	beDeinitStream(HBE_STREAM hbeStream, PBYTE pOutput, PDWORD pdwOutput)
{
	int nOutputSamples = 0;

	lame_global_flags*	gfp = (lame_global_flags*)hbeStream;

	nOutputSamples = lame_encode_flush(gfp, pOutput, 0);

	if (nOutputSamples < 0)
	{
		*pdwOutput = 0;
		return BE_ERR_BUFFER_TOO_SMALL;
	}
	else
	{
		*pdwOutput = nOutputSamples;
	}

	return BE_ERR_SUCCESSFUL;
}


BE_ERR	beCloseStream(HBE_STREAM hbeStream)
{
	lame_global_flags*	gfp = (lame_global_flags*)hbeStream;

	// lame will be close in VbrWriteTag function
	if (!lame_get_bWriteVbrTag(gfp))
	{
		// clean up of allocated memory
		lame_close(gfp);

		gfp_save = NULL;
	}
	else
	{
		gfp_save = (lame_global_flags*)hbeStream;
	}

	// DeInit encoder
	return BE_ERR_SUCCESSFUL;
}


#ifdef	__cplusplus
}
#endif
