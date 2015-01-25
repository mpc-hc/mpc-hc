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
#if defined(MEDIAINFO_MPEG4_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mpeg4_TimeCode.h"
#include "MediaInfo/TimeCode.h"
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

        TimeCode TC(Pos_Temp, NumberOfFrames, DropFrame);
        Stream_Prepare(Stream_Other);
        Fill(Stream_Other, StreamPos_Last, Other_Type, "Time code");
        Fill(Stream_Other, StreamPos_Last, Other_TimeCode_FirstFrame, TC.ToString().c_str());
        if (Frame_Count==1)
            Fill(Stream_Other, StreamPos_Last, Other_TimeCode_Striped, "Yes");
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
        Frame_Count+=Element_Size/4;

        if (!Status[IsAccepted])
        {
            Accept("TimeCode");
            Fill("TimeCode");
        }
    FILLING_END();
}

}

#endif //MEDIAINFO_MPEG4_YES
