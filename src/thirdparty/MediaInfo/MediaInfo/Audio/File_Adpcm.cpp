/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_ADPCM_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Adpcm.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Format
//***************************************************************************

//---------------------------------------------------------------------------
void File_Adpcm::Read_Buffer_Continue()
{
    //It is impossible to detect... Default is no detection, only filling

    //Filling
    Accept("ADPCM");

    Stream_Prepare(Stream_Audio);
    Fill(Stream_General, 0, Audio_Format, "ADPCM");
    Fill(Stream_General, 0, Audio_Codec, "ADPCM");
    Ztring Profile, Firm;
         if (Codec==__T("alaw"))             {Profile=__T("A-Law");}
    else if (Codec==__T("ulaw"))             {Profile=__T("U-Law");}
    else if (Codec==__T("ima4"))             {                     Firm=__T("IMA");}
    else if (Codec==__T("6"))                {Profile=__T("A-Law");}
    else if (Codec==__T("7"))                {Profile=__T("U-Law");}
    else if (Codec==__T("102"))              {Profile=__T("A-Law");}
    else if (Codec==__T("171"))              {Profile=__T("U-Law"); Firm=__T("Unisys");}
    else if (Codec==__T("172"))              {Profile=__T("A-Law"); Firm=__T("Unisys");}

    if (!Profile.empty())
        Fill(Stream_Audio, 0, Audio_Format_Profile, Profile);
    if (!Firm.empty())
    {
        Fill(Stream_Audio, 0, Audio_Format_Settings, Firm);
        Fill(Stream_Audio, 0, Audio_Format_Settings_Firm, Firm);
        Fill(Stream_Audio, 0, Audio_Codec_Settings, Firm);
        Fill(Stream_Audio, 0, Audio_Codec_Settings_Firm, Firm);
    }
    Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");

    //No more need data
    Finish("ADPCM");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_ADPCM_YES
