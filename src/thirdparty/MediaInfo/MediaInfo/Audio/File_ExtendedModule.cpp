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
#if defined(MEDIAINFO_XM_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_ExtendedModule.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_ExtendedModule::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<38)
        return false; //Must wait for more data

    if (CC8(Buffer)!=0x457874656E646564LL || CC8(Buffer+8)!=0x204D6F64756C653ALL  //"Extended Module: "
     || CC1(Buffer+16)!=0x20 || CC1(Buffer+37)!=0x1A)
    {
        Reject("Extended Module");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_ExtendedModule::Read_Buffer_Continue()
{
    //Parsing
    Ztring ModuleName, TrackerName;
    int32u HeaderSize;
    int16u Length, Channels, Patterns, Instruments, Flags, Tempo, BPM;
    int8u  VersionMinor, VersionMajor;
    Skip_String(17,                                             "Signature");
    Get_Local(20, ModuleName,                                   "Module name");
    Skip_L1(                                                    "0x1A");
    Get_Local(20, TrackerName,                                  "Tracker name");
    Get_L1 (VersionMinor,                                       "Version (minor)");
    Get_L1 (VersionMajor,                                       "Version (major)");
    Get_L4 (HeaderSize,                                         "Header size");
    Get_L2 (Length,                                             "Song Length");
    Skip_L2(                                                    "Restart position");
    Get_L2 (Channels,                                           "Number of channels");
    Get_L2 (Patterns,                                           "Number of patterns");
    Get_L2 (Instruments,                                        "Number of instruments");
    Get_L2 (Flags,                                              "Flags");
    Get_L2 (Tempo,                                              "Tempo");
    Get_L2 (BPM,                                                "BPM");
    Skip_XX(256,                                                "Pattern order table");

    FILLING_BEGIN();
        Accept("Extended Module");

        Fill(Stream_General, 0, General_Format, "Extended Module");
        Fill(Stream_General, 0, General_Format_Version, Ztring::ToZtring(VersionMajor)+__T(".")+Ztring::ToZtring(VersionMinor/10)+Ztring::ToZtring(VersionMinor%10));
        Fill(Stream_General, 0, General_Track, ModuleName.Trim(__T(' ')));
        Fill(Stream_General, 0, General_Encoded_Application, TrackerName.Trim(__T(' ')));
        Fill(Stream_General, 0, "Tempo", Tempo);
        Fill(Stream_General, 0, "BPM", BPM);

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, "Sampler, Channels", Channels);
        Fill(Stream_Audio, 0, "Sampler, Patterns", Patterns);
        Fill(Stream_Audio, 0, "Sampler, Instruments", Instruments);

        //No more need data
        Finish("Extended Module");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_XM_YES
