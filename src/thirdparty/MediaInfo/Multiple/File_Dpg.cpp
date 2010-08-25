// File_Dpg - Info for DPG files
// Copyright (C) 2009-2010 MediaArea.net SARL, Info@MediaArea.net
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
//
// Format:
// * File header
// * MPEG Audio stream
// * MPEG Video stream
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_DPG_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Dpg.h"
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Dpg::File_Dpg()
{
    //Data
    Parser=NULL;
}

//---------------------------------------------------------------------------
File_Dpg::~File_Dpg()
{
    delete Parser; //Parser=NULL;
}

//***************************************************************************
// Buffer - File offset
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Dpg::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<0x14)
        return false; //Must wait for more data

    if (                CC4(Buffer     )!=0x44504730    //"DPG0"
     || LittleEndian2int32u(Buffer+0x10)!=0)            //Zero
    {
        Reject("DPG");
        return false;
    }

    //All should be OK...
    return true;
}

//---------------------------------------------------------------------------
void File_Dpg::FileHeader_Parse()
{
    //Parsing
    int32u  FrameCount, FrameRate, SamplingRate;
    Skip_C4(                                                    "Signature");
    Get_L4 (FrameCount,                                         "Frame count");
    Get_L4 (FrameRate,                                          "Frame rate"); Param_Info(FrameRate/0x100, " fps");
    Get_L4 (SamplingRate,                                       "Sampling rate");
    Skip_L4(                                                    "0x00000000");
    Get_L4 (Audio_Offset,                                       "Audio Offset");
    Get_L4 (Audio_Size,                                         "Audio Size");
    Get_L4 (Video_Offset,                                       "Video Offset");
    Get_L4 (Video_Size,                                         "Video Size");

    FILLING_BEGIN();
        Accept("DPG");

        Fill(Stream_General, 0, General_Format, "DPG");

        Stream_Prepare(Stream_Video);
        Fill(Stream_Video, 0, Video_FrameRate, (float)(FrameRate/0x100), 3);
        Fill(Stream_Video, 0, Video_FrameCount, FrameCount);
        Fill(Stream_Video, 0, Video_StreamSize, Video_Size);

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_SamplingRate, SamplingRate);
        Fill(Stream_Audio, 0, Audio_StreamSize, Audio_Size);

        //Positionning
        #if defined(MEDIAINFO_MPEGA_YES)
            Parser=new File_Mpega();
            Open_Buffer_Init(Parser);
            GoTo(Audio_Offset, "DPG");
        #elif defined(MEDIAINFO_MPEGV_YES)
            Audio_Size=0;
            Parser=new File_Mpegv();
            Open_Buffer_Init(Parser);
            GoTo(Video_Offset, "DPG");
        #else
            Finish("DPG");
        #endif
    FILLING_END();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dpg::Read_Buffer_Continue()
{
    if (!Parser)
        return; //Not ready

    if (Audio_Size)
    {
        #if defined(MEDIAINFO_MPEGA_YES)
            Open_Buffer_Continue(Parser, (size_t)((File_Offset+Buffer_Size<Audio_Offset+Audio_Size)?Buffer_Size:(Audio_Offset+Audio_Size-File_Offset)));
            if (Parser->Status[IsAccepted])
            {
                Finish(Parser);
                Merge(*Parser, Stream_Audio, 0, 0);
                #if defined(MEDIAINFO_MPEGV_YES)
                    Audio_Size=0;
                    Data_GoTo(Video_Offset, "DPG");
                    delete Parser; Parser=new File_Mpegv();
                    Open_Buffer_Init(Parser);
                #else
                    Finish("DPG");
                #endif
            }
        #endif
    }
    else
    {
        #if defined(MEDIAINFO_MPEGV_YES)
            Open_Buffer_Continue(Parser, (size_t)((File_Offset+Buffer_Size<Video_Offset+Video_Size)?Buffer_Size:(Video_Offset+Video_Size-File_Offset)));
            if (Parser->Status[IsAccepted])
            {
                //Merging
                Finish(Parser);
                Merge(*Parser, Stream_Video, 0, 0);

                Finish("DPG");
            }
        #endif
    }

    //Positioning
    Buffer_Offset=Buffer_Size; //We have already parsed this data
}

} //NameSpace

#endif //MEDIAINFO_DPG_YES
