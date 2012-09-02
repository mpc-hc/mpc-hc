// File_Mpeg4_TimeCode - Info for MPEG-4 TimeCode  files
// Copyright (C) 2009-2012 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_MPEG4_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mpeg4_TimeCode.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Mpeg4_TimeCode::File_Mpeg4_TimeCode()
:File__Analyze()
{
    //Out
    Pos=(int32u)-1;

    NumberOfFrames=0;
    DropFrame=false;
    NegativeTimes=false;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg4_TimeCode::Streams_Fill()
{
    if (Pos!=(int32u)-1 && NumberOfFrames)
    {
        int64s  Pos_Temp=Pos;
        float64 FrameRate_WithDF=NumberOfFrames;
        if (DropFrame)
        {
            float64 FramesPerHour_NDF=FrameRate_WithDF*60*60;
            FrameRate_WithDF*=(FramesPerHour_NDF-108)/FramesPerHour_NDF;
        }

        Fill(Stream_General, 0, "Delay", Pos_Temp*1000/FrameRate_WithDF, 0);
    }
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg4_TimeCode::Read_Buffer_Continue()
{
    //Parsing
    int32u Position=0;
    while (Element_Offset<Element_Size)
    {
        Get_B4 (Position,                                       "Position");
        if (Pos==(int32u)-1) //First time code
        {
            Pos=Position;
            if (NegativeTimes)
                Pos=(int32s)Position;
        }
    }

    FILLING_BEGIN();
        if (!Status[IsAccepted])
        {
            Accept("TimeCode");
            Fill("TimeCode");
        }
    FILLING_END();
}

}

#endif //MEDIAINFO_MPEG4_YES
