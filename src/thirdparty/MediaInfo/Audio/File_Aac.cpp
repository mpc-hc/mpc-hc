// File_Aac - Info for AAC (Raw) files
// Copyright (C) 2008-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_AAC_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Aac.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Aac::File_Aac()
{
    //In
    #ifdef MEDIAINFO_MPEG4_YES
        DecSpecificInfoTag=NULL;
        SLConfig=NULL;
    #endif

    //libfaad specific
    #ifdef MEDIAINFO_FAAD_YES
        hAac=NULL;
    #endif
}

//---------------------------------------------------------------------------
File_Aac::~File_Aac()
{
    //libfaad specific
    #ifdef MEDIAINFO_FAAD_YES
        NeAACDecClose(hAac); //hAac=NULL;
    #endif
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::Read_Buffer_Continue()
{
    if (!Codec.empty())
        From_Codec(); //It is impossible to detect... Default is no detection, only filling
    else
        libfaad(); //Trying libfaad if available
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::From_Codec()
{
    //Filling
    Accept("AAC");

    Stream_Prepare(Stream_Audio);
    Fill(Stream_Audio, 0, Audio_Format, "AAC");
    Fill(Stream_Audio, 0, Audio_Codec, Codec);
    Ztring Profile;
    int8u Version=0, SBR=2, PS=2;
         if (Codec==_T("A_AAC/MPEG2/MAIN"))     {Version=2; Profile=_T("Main");}
    else if (Codec==_T("A_AAC/MPEG2/LC"))       {Version=2; Profile=_T("LC");   SBR=0;}
    else if (Codec==_T("A_AAC/MPEG2/LC/SBR"))   {Version=2; Profile=_T("LC");   SBR=1;}
    else if (Codec==_T("A_AAC/MPEG2/SSR"))      {Version=2; Profile=_T("SSR");}
    else if (Codec==_T("A_AAC/MPEG4/MAIN"))     {Version=4; Profile=_T("Main");}
    else if (Codec==_T("A_AAC/MPEG4/LC"))       {Version=4; Profile=_T("LC");   SBR=0;}
    else if (Codec==_T("A_AAC/MPEG4/LC/SBR"))   {Version=4; Profile=_T("LC");   SBR=1; PS=0;}
    else if (Codec==_T("A_AAC/MPEG4/LC/SBR/PS")){Version=4; Profile=_T("LC");   SBR=1; PS=1;}
    else if (Codec==_T("A_AAC/MPEG4/SSR"))      {Version=4; Profile=_T("SSR");}
    else if (Codec==_T("A_AAC/MPEG4/LTP"))      {Version=4; Profile=_T("LTP");}
    else if (Codec==_T("raac"))                 {           Profile=_T("LC");}
    else if (Codec==_T("racp"))                 {           Profile=_T("LC");   SBR=1; PS=0;}

    if (Version>0)
        Fill(Stream_Audio, 0, Audio_Format_Version, Version==2?"Version 2":"Version 4");
    Fill(Stream_Audio, 0, Audio_Format_Profile, Profile);
    if (SBR!=2)
    {
        if (SBR)
            Fill(Stream_Audio, 0, Audio_Format_Settings, "SBR");
        Fill(Stream_Audio, 0, Audio_Format_Settings_SBR, SBR?"Yes":"No");
    }
    if (PS!=2)
    {
        if (PS)
            Fill(Stream_Audio, 0, Audio_Format_Settings, "PS");
        Fill(Stream_Audio, 0, Audio_Format_Settings_PS, PS?"Yes":"No");
    }

    Finish("AAC");
}

//***************************************************************************
// libfaad specific
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::libfaad()
{
    #if defined(MEDIAINFO_FAAD_YES) && defined(MEDIAINFO_MPEG4_YES)
        unsigned long samplerate;
        unsigned char channels;

        //Open the library
        if (hAac==NULL)
        {
            hAac = NeAACDecOpen();
            // Initialise the library using one of the initialization functions
            char err = NeAACDecInit2(hAac, (unsigned char *)DecSpecificInfoTag->Buffer, DecSpecificInfoTag->Buffer_Size, &samplerate, &channels);
            if (err != 0)
            {
                //
                // Handle error
                //
            }
        }

        //Decode the frame in buffer
        void* samplebuffer;
        NeAACDecFrameInfo hInfo;
        samplebuffer = NeAACDecDecode(hAac, &hInfo, (unsigned char *)Buffer, Buffer_Size);
        if ((hInfo.error == 0) && (hInfo.samples > 0))
        {
            //
            // do what you need to do with the decoded samples
            //
        }
        else if (hInfo.error != 0)
        {
            //
            // Some error occurred while decoding this frame
            //
        }
    #else
        //Filling
        if (!Status[IsAccepted])
        {
            Accept("AAC");

            Stream_Prepare(Stream_Audio);
            Fill(Stream_Audio, 0, Audio_Format, "AAC");
            Fill(Stream_Audio, 0, Audio_Codec, "AAC");

            Finish("AAC");
        }
    #endif
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_AAC_YES
