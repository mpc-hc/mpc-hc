// File_Mpeg4_TimeCode - Info for MPEG-4 TimeCode  files
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

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_MPEG4_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mpeg4_TimeCode.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Format
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg4_TimeCode::FileHeader_Parse()
{
    if (Buffer_Size!=4)
    {
        Reject("TimeCode");
        return;
    }

    //Parsing
    int32u Position;
    Get_B4 (Position,                                           "Position");

    //Filling
    Accept("TimeCode");

    if (FrameRate)
    {
        int64s Pos=Position;
        if (NegativeTimes)
            Pos=(int32s)Position;
        if (StreamKind==Stream_General)
        {
            //No link with a track, we do all
            Stream_Prepare(Stream_Video);
            Fill(Stream_Video, 0, Video_Delay, Pos*1000/FrameRate, 0);
            Fill(Stream_Video, 0, Video_Delay_Source, "Container");

            Stream_Prepare(Stream_Audio);
            Fill(Stream_Audio, 0, Audio_Delay, Pos*1000/FrameRate, 0);
            Fill(Stream_Video, 0, Video_Delay_Source, "Container");
        }
        else
        {
            Stream_Prepare(StreamKind);
            Fill(StreamKind, 0, Fill_Parameter(StreamKind_Last, Generic_Delay), Pos*1000/FrameRate, 0);
            Fill(StreamKind, 0, Fill_Parameter(StreamKind_Last, Generic_Delay_Source), "Container");
        }
    }

    Finish("TimeCode");
}

}

#endif //MEDIAINFO_MPEG4_YES
