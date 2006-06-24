/*****************************************************************
|
|    AP4 - MP4 File Info
|
|    Copyright 2002-2205 Gilles Boccon-Gibod & Julien Boeuf
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "Ap4.h"
#include "Ap4FileByteStream.h"
#include "Ap4Atom.h"
#include "Ap4File.h"
#include "Ap4Sample.h"
#include "Ap4SampleDescription.h"
#include "Ap4IsmaCryp.h"
#include "Ap4IsfmAtom.h"
#include "Ap4IkmsAtom.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define BANNER "MP4 File Info - Version 0.1a\n"\
               "(c) 2002-2005 Gilles Boccon-Gibod & Julien Boeuf"
 
/*----------------------------------------------------------------------
|       PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit()
{
    fprintf(stderr, 
            BANNER 
            "\n\nusage: mp4info [options] <input>\n");
    exit(1);
}

/*----------------------------------------------------------------------
|       ShowIsmaSampleDescription
+---------------------------------------------------------------------*/
static void
ShowIsmaSampleDescription(AP4_IsmaCrypSampleDescription* description)
{
    if (description == NULL) return;
    AP4_Debug("    [ENCRYPTED]\n");
    AP4_UI32 st = description->GetSchemeType();
    AP4_Debug("      Scheme Type:    %c%c%c%c\n", 
        (char)((st>>24) & 0xFF),
        (char)((st>>16) & 0xFF),
        (char)((st>> 8) & 0xFF),
        (char)((st    ) & 0xFF));
    AP4_Debug("      Scheme Version: %d\n", description->GetSchemeVersion());
    AP4_Debug("      Scheme URI:     %s\n", description->GetSchemeUri().c_str());
    AP4_IsmaCrypSchemeInfo* scheme_info = description->GetSchemeInfo();
    if (scheme_info == NULL) return;
    if (description->GetSchemeType() != AP4_ISMACRYP_SCHEME_TYPE_IAEC) {
        return;
    }

    AP4_Debug("      iAEC Scheme Info:\n");
    AP4_IkmsAtom* ikms = (AP4_IkmsAtom*)scheme_info->GetSchiAtom().FindChild("iKMS");
    if (ikms) {
        AP4_Debug("        KMS URI:              %s\n", ikms->GetKmsUri().c_str());
    }
    AP4_IsfmAtom* isfm = (AP4_IsfmAtom*)scheme_info->GetSchiAtom().FindChild("iSFM");
    if (isfm) {
        AP4_Debug("        Selective Encryption: %s\n", isfm->GetSelectiveEncryption()?"yes":"no");
        AP4_Debug("        Key Indicator Length: %d\n", isfm->GetKeyIndicatorLength());
        AP4_Debug("        IV Length:            %d\n", isfm->GetIvLength());
    }
}

/*----------------------------------------------------------------------
|       ShowSampleDescription
+---------------------------------------------------------------------*/
static void
ShowSampleDescription(AP4_SampleDescription* desc)
{
    AP4_SampleDescription::Type type = desc->GetType();

    AP4_MpegSampleDescription* mpeg_desc = NULL;
    if (type == AP4_SampleDescription::TYPE_MPEG) {
        mpeg_desc = dynamic_cast<AP4_MpegSampleDescription*>(desc);
    } else if (type == AP4_SampleDescription::TYPE_ISMACRYP) {
        AP4_IsmaCrypSampleDescription* isma_desc = dynamic_cast<AP4_IsmaCrypSampleDescription*>(desc);
        ShowIsmaSampleDescription(isma_desc);
        mpeg_desc = isma_desc->GetOriginalSampleDescription();
    }
    if (mpeg_desc) {
        AP4_Debug("    Stream Type: %s\n", mpeg_desc->GetStreamTypeString(mpeg_desc->GetStreamType()));
        AP4_Debug("    Object Type: %s\n", mpeg_desc->GetObjectTypeString(mpeg_desc->GetObjectTypeId()));
        AP4_Debug("    Max Bitrate: %d\n", mpeg_desc->GetMaxBitrate());
        AP4_Debug("    Avg Bitrate: %d\n", mpeg_desc->GetAvgBitrate());
        AP4_Debug("    Buffer Size: %d\n", mpeg_desc->GetBufferSize());
        if (mpeg_desc->GetStreamType() == AP4_AUDIO_STREAM_TYPE) {
            AP4_MpegAudioSampleDescription* audio_desc = 
                dynamic_cast<AP4_MpegAudioSampleDescription*>(mpeg_desc);
            AP4_Debug("    Sample Rate: %d\n", audio_desc->GetSampleRate());
            AP4_Debug("    Sample Size: %d\n", audio_desc->GetSampleSize());
            AP4_Debug("    Channels:    %d\n", audio_desc->GetChannelCount());
        } else if (mpeg_desc->GetStreamType() == AP4_VISUAL_STREAM_TYPE) {
            AP4_MpegVideoSampleDescription* video_desc = 
                dynamic_cast<AP4_MpegVideoSampleDescription*>(mpeg_desc);
            AP4_Debug("    Width:  %d\n", video_desc->GetWidth());
            AP4_Debug("    Height: %d\n", video_desc->GetHeight());
            AP4_Debug("    Depth:  %d\n", video_desc->GetDepth());
        }
    }
}

/*----------------------------------------------------------------------
|       ShowTrackInfo
+---------------------------------------------------------------------*/
static void
ShowTrackInfo(AP4_Track* track)
{
    AP4_Debug("  type:         ");
    switch (track->GetType()) {
        case AP4_Track::TYPE_AUDIO: AP4_Debug("Audio\n"); break;
        case AP4_Track::TYPE_VIDEO: AP4_Debug("Video\n"); break;
        case AP4_Track::TYPE_HINT:  AP4_Debug("Hint\n");  break;
        default:                    AP4_Debug("System\n");break;
    }
    AP4_Debug("  duration:     %ld ms\n", track->GetDurationMs());
    AP4_Debug("  timescale:    %ld\n", track->GetMediaTimeScale());
    AP4_Debug("  sample count: %ld\n", track->GetSampleCount());
	AP4_Sample  sample;
	AP4_Ordinal index = 0;
    AP4_Ordinal desc_index = 0xFFFFFFFF;
    while (AP4_SUCCEEDED(track->GetSample(index, sample))) {
        if (sample.GetDescriptionIndex() != desc_index) {
            desc_index = sample.GetDescriptionIndex();
            AP4_Debug("  [%d]: Format %d\n", index, desc_index);
            
            // get the sample description
            AP4_SampleDescription* sample_desc = 
                track->GetSampleDescription(desc_index);
            if (sample_desc != NULL) {
                ShowSampleDescription(sample_desc);
            }
        }
        index++;
    }
}

/*----------------------------------------------------------------------
|       ShowMovieInfo
+---------------------------------------------------------------------*/
static void
ShowMovieInfo(AP4_Movie* movie)
{
    AP4_Debug("Movie:\n");
    AP4_Debug("  duration:   %ld ms\n", movie->GetDurationMs());
    AP4_Debug("  time scale: %ld\n", movie->GetTimeScale());
    AP4_Debug("\n");
}

/*----------------------------------------------------------------------
|       main
+---------------------------------------------------------------------*/
int
main(int argc, char** argv)
{
    if (argc < 2) {
        PrintUsageAndExit();
    }
    
    AP4_ByteStream* input;
    try {
        input = new AP4_FileByteStream(argv[1],
                               AP4_FileByteStream::STREAM_MODE_READ);
    } catch (AP4_Exception) {
        fprintf(stderr, "ERROR: cannot open input file (%s)\n", argv[1]);
        return 1;
    }

    AP4_File* file = new AP4_File(*input);
    
    AP4_Movie* movie = file->GetMovie();
    if (movie != NULL) {
        ShowMovieInfo(movie);

        AP4_List<AP4_Track>& tracks = movie->GetTracks();
        AP4_Debug("Found %d Tracks\n", tracks.ItemCount());

        AP4_List<AP4_Track>::Item* track_item = tracks.FirstItem();
        int index = 1;
        while (track_item) {
            AP4_Debug("Track %d:\n", index); 
            index++;
            ShowTrackInfo(track_item->GetData());
            track_item = track_item->GetNext();
        }
    } else {
        AP4_Debug("No movie found in the file\n");
    }

    delete file;
    input->Release();

    return 0;
}
