/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about ARRI RAW files
//
// From http://www.fileformat.info/format/png/
//
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
#if defined(MEDIAINFO_ARRIRAW_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_ArriRaw.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_ArriRaw::File_ArriRaw()
{
    //Config
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    IsRawStream=true;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_ArriRaw::Streams_Accept()
{
    Fill(Stream_General, 0, General_Format, "Arri Raw");

    if (!IsSub)
    {
        TestContinuousFileNames();

        Stream_Prepare((Config->File_Names.size()>1 || Config->File_IsReferenced_Get())?Stream_Video:Stream_Image);
        Fill(StreamKind_Last, StreamPos_Last, "StreamSize", File_Size);
        if (StreamKind_Last==Stream_Video)
            Fill(Stream_Video, StreamPos_Last, Video_FrameCount, Config->File_Names.size());
    }
    else
        Stream_Prepare(StreamKind_Last);

    //Configuration
    Frame_Count_NotParsedIncluded=0;
}

//***************************************************************************
// Header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_ArriRaw::FileHeader_Begin()
{
    // Minimum buffer size
    if (Buffer_Size<8)
        return false; // Must wait for more data

    // Testing
    if (Buffer[0]!=0x41 // "ARRI.4Vx"
     || Buffer[1]!=0x52
     || Buffer[2]!=0x52
     || Buffer[3]!=0x49
     || Buffer[4]!=0x12
     || Buffer[5]!=0x34
     || Buffer[6]!=0x56
     || Buffer[7]!=0x78)
    {
        Reject("Arri Raw");
        return false;
    }

    Accept();

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_ArriRaw::Read_Buffer_Unsynched()
{
    Read_Buffer_Unsynched_OneFramePerFile();
}

//---------------------------------------------------------------------------
void File_ArriRaw::Read_Buffer_Continue()
{
    //Parsing
    Skip_C4(                                                    "Signature");
    Skip_C1(                                                    "Signature");
    Skip_C3(                                                    "Signature");
    Skip_XX(File_Size-8,                                        "Data");

    FILLING_BEGIN();
        Frame_Count++;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded++;
        if (!Status[IsFilled])
        {
            Fill();
            if (Config->ParseSpeed<1.0)
                Finish();
        }
    FILLING_END();
}

} //NameSpace

#endif
