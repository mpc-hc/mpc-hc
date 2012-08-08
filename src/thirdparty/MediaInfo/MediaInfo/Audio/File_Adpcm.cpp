// File_Adpcm - Info for ADPCM files
// Copyright (C) 2008-2012 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
