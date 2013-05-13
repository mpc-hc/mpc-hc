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
#if defined(MEDIAINFO_GXF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Umf.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Umf::File_Umf()
:File__Analyze()
{
    //In
    #if MEDIAINFO_SEEK || MEDIAINFO_DEMUX
        GopSize=(int64u)-1;
    #endif //MEDIAINFO_SEEK || MEDIAINFO_DEMUX
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Umf::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<2)
        return false; //Must wait for more data

    int16u Length=LittleEndian2int16u(Buffer);
    if (Buffer_Size<Length)
        return false; //Must wait for more data

    //All should be OK...
    Accept("UMF");
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Umf::Read_Buffer_Continue()
{
    //Parsing
    int32u Tracks, Segments;
    Element_Begin1("Payload description");
    Skip_L4(                                                    "Total length of the UMF data");
    Skip_L4(                                                    "Version of this file");
    Get_L4 (Tracks,                                             "Number of tracks in the material");
    Skip_L4(                                                    "Offset to track description section");
    Skip_L4(                                                    "Size of the track description section");
    Get_L4 (Segments,                                           "Number of segments");
    Skip_L4(                                                    "Offset to media description section");
    Skip_L4(                                                    "Size of the media description section");
    Skip_L4(                                                    "Offset to the user data section");
    Skip_L4(                                                    "Size of the user data section");
    Skip_L4(                                                    "Reserved");
    Skip_L4(                                                    "Reserved");
    Element_End0();

    Element_Begin1("Material description");
    Skip_L4(                                                    "Attributes");
    Skip_L4(                                                    "Maximum length of the material in fields");
    Skip_L4(                                                    "Minimum length of the material in fields");
    Skip_L4(                                                    "Material mark in value in fields");
    Skip_L4(                                                    "Material mark out value in fields");
    Skip_L4(                                                    "Time code at mark in value");
    Skip_L4(                                                    "Time code at mark out value");
    Skip_L4(                                                    "last modified time (Most)");
    Skip_L4(                                                    "last modified time (Least)");
    Skip_L4(                                                    "creation time (Most)");
    Skip_L4(                                                    "creation time (Least)");
    Skip_L2(                                                    "Reserved");
    Skip_L2(                                                    "Reserved");
    Skip_L2(                                                    "Number of audio tracks");
    Skip_L2(                                                    "Number of time code tracks");
    Skip_L2(                                                    "Reserved");
    Skip_L2(                                                    "Number of MPEG-1, MPEG-2, and MPEG-2 HD video tracks");
    Element_End0();

    for (int32u Pos=0; Pos<Tracks; Pos++)
    {
        Element_Begin1("Track description");
        Skip_C1(                                                "Track information - Track type");
        Skip_C1(                                                "Track information - Track logical number");
        Skip_L2(                                                "Number of segments on this track");
        Element_End0();

        if (Element_Offset>=Element_Size)
            break;
    }

    for (int32u Pos=0; Pos<Segments; Pos++)
    {
        Element_Begin1("Media description");
        int32u Type;
        int16u Length;
        Get_L2 (Length,                                         "Length of this media description");
        int64u End=Element_Offset+Length-2;
        Skip_C1(                                                "Track information - Track type");
        Skip_C1(                                                "Track information - Track logical number");
        Skip_L2(                                                "Media Sequence number");
        Skip_L2(                                                "Reserved");
        Skip_L4(                                                "Number of fields in segment");
        Skip_L4(                                                "Reserved");
        Skip_L4(                                                "Mark in value for the media file in fields");
        Skip_L4(                                                "Mark out value for the media file in fields");
        Skip_Local(88,                                          "Source device media file name");
        Get_L4 (Type,                                           "Type of media track");
        Skip_L4(                                                "Sampling rates for this track");
        Skip_L4(                                                "Size of sample for audio and time codes");
        Skip_L4(                                                "Reserved");
        switch (Type)
        {
            case 0x00000004 :
            case 0x00000007 :
            case 0x00000009 : //MPEG-Video
                {
                #if MEDIAINFO_SEEK || MEDIAINFO_DEMUX
                    int32u P, B;
                #endif //MEDIAINFO_SEEK || MEDIAINFO_DEMUX
                Skip_L4(                                                "Color difference format");
                Skip_L4(                                                "GoP structure");
                Skip_L4(                                                "Frame/field structure");
                Skip_L4(                                                "Target I-pictures per GoP");
                #if MEDIAINFO_SEEK || MEDIAINFO_DEMUX
                    Get_L4 (P,                                          "Target P-pictures per I-picture");
                    Get_L4 (B,                                          "Target B-pictures per P-picture or I-picture");
                #else //MEDIAINFO_SEEK || MEDIAINFO_DEMUX
                    Skip_L4(                                            "Target P-pictures per I-picture");
                    Skip_L4(                                            "Target B-pictures per P-picture or I-picture");
                #endif //MEDIAINFO_SEEK || MEDIAINFO_DEMUX
                Skip_L4(                                                "MPEG video attributes");
                Skip_L4(                                                "Reserved");
                #if MEDIAINFO_SEEK || MEDIAINFO_DEMUX
                    GopSize=(1+P)*(1+B);
                #endif //MEDIAINFO_SEEK || MEDIAINFO_DEMUX
                }
                break;
            case 0x00000003 : //TimeCode
                Skip_L4(                                                "Time code attributes");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                break;
            case 0x00000002 : //Audio
                //Skip_LF8(                                                "Level at which to play this segment");
                //Skip_LF8(                                                "Level at which to terminate this segment");
                Skip_L8(                                                 "Level at which to play this segment");
                Skip_L8(                                                 "Level at which to terminate this segment");
                Skip_L4(                                                 "Number of fields over which to ramp up");
                Skip_L4(                                                 "Number of fields over which to ramp down");
                Skip_L4(                                                 "Reserved");
                Skip_L4(                                                 "Reserved");
                break;
            case 0x00000005 : //DV25
            case 0x00000006 : //DV50
                Skip_L4(                                                "Attributes"); //With Aspect ratio
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                #if MEDIAINFO_SEEK || MEDIAINFO_DEMUX
                    GopSize=1;
                #endif //MEDIAINFO_SEEK || MEDIAINFO_DEMUX
                break;
            default         :
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
                Skip_L4(                                                "Reserved");
        }
        if (Element_Offset<End)
            Skip_XX(End-Element_Offset,                                 "Unknown");
        Element_End0();

        if (Element_Offset>=Element_Size)
            break;
    }

    while (Element_Offset<Element_Size)
    {
        Element_Begin1("User data");
        int32u Length;
            Get_L4 (Length,                                     "The length of this user data record");
            Skip_L4(                                            "Position on the material time line");
            Skip_L2(                                            "Track associated with the user data record");
            Skip_L2(                                            "Media Sequence Numbe");
            Skip_L4(                                            "User-defined key");
            if (Length>18)
                Skip_XX(Length-18,                              "User data");
            else
                Skip_XX(Element_Size-Element_Offset-2,          "User data");
            Skip_L1(                                            "NULL byte");
            Skip_L1(                                            "Reserved byte");
        Element_End0();
    }
}

} //NameSpace

#endif //MEDIAINFO_UMF_YES
