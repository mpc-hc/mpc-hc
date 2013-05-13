/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

#ifndef MediaInfo_Events_InternalH
#define MediaInfo_Events_InternalH

#include "MediaInfo/MediaInfo_Events.h"
#include "MediaInfo/TimeCode.h"
#include <cstring>
using namespace MediaInfoLib;
using namespace std;

//---------------------------------------------------------------------------
// Generic
#define EVENT_BEGIN(_EventType, _EventName, _EventVersion) \
{ \
    struct MediaInfo_Event_##_EventType##_##_EventName##_##_EventVersion Event;\
    Event_Prepare((struct MediaInfo_Event_Generic*)&Event); \
    Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_##_EventType, MediaInfo_Event_##_EventType##_##_EventName, _EventVersion); \
    Event.EventSize=sizeof(struct MediaInfo_Event_##_EventType##_##_EventName##_##_EventVersion); \

#define EVENT_END() \
    Config->Event_Send(Status[IsAccepted]?NULL:this, (const int8u*)&Event, Event.EventSize, IsSub?File_Name_WithoutDemux:File_Name); \
} \

#define EVENT(_EventType, _EventName, _EventVersion) \
    EVENT_BEGIN(_EventType, _EventName, _EventVersion) \
    EVENT_END  ()

//---------------------------------------------------------------------------
// Helpers
namespace MediaInfoLib
{
    inline void Events_PCR(int64u PCR, int64u &Event_PCR, char* Event_PCR_HR)
    {
        Event_PCR=PCR;
        if (PCR!=(int64u)-1)
        {
            string PCR_HR=Ztring().Duration_From_Milliseconds(PCR/1000000).To_UTF8();
            if (PCR_HR.size()==12)
                strcpy(Event_PCR_HR, PCR_HR.c_str());
            else
                memset(Event_PCR_HR, 0x00, 13);
        }
        else
            memset(Event_PCR_HR, 0x00, 13);
    }

    inline void Events_PTS(int64u PTS, int64u &Event_PTS, char* Event_PTS_HR)
    {
        Event_PTS=PTS;
        if (PTS!=(int64u)-1)
        {
            string PTS_HR=Ztring().Duration_From_Milliseconds(PTS/1000000).To_UTF8();
            if (PTS_HR.size()==12)
                strcpy(Event_PTS_HR, PTS_HR.c_str());
            else
                memset(Event_PTS_HR, 0x00, 13);
        }
        else
            memset(Event_PTS_HR, 0x00, 13);
    }

    inline void Events_DTS(int64u DTS, int64u &Event_DTS, char* Event_DTS_HR)
    {
        Event_DTS=DTS;
        if (DTS!=(int64u)-1)
        {
            string DTS_HR=Ztring().Duration_From_Milliseconds(DTS/1000000).To_UTF8();
            if (DTS_HR.size()==12)
                strcpy(Event_DTS_HR, DTS_HR.c_str());
            else
                memset(Event_DTS_HR, 0x00, 13);
        }
        else
            memset(Event_DTS_HR, 0x00, 13);
    }

    inline void Events_TimeCode(const TimeCode &Tc, MediaInfo_time_code &Event_TimeCode, char* Event_TimeCode_HR)
    {
        if (Tc.IsValid())
        {
            Event_TimeCode.Hours=Tc.Hours;
            Event_TimeCode.Minutes=Tc.Minutes;
            Event_TimeCode.Seconds=Tc.Seconds;
            Event_TimeCode.Frames=Tc.Frames;
            Event_TimeCode.FramesPerSecond=Tc.FramesPerSecond;
            Event_TimeCode.DropFrame=Tc.DropFrame;
            Event_TimeCode_HR[ 0]='0'+Tc.Hours/10;
            Event_TimeCode_HR[ 1]='0'+Tc.Hours%10;
            Event_TimeCode_HR[ 2]=':';
            Event_TimeCode_HR[ 3]='0'+Tc.Minutes/10;
            Event_TimeCode_HR[ 4]='0'+Tc.Minutes%10;
            Event_TimeCode_HR[ 5]=':';
            Event_TimeCode_HR[ 6]='0'+Tc.Seconds/10;
            Event_TimeCode_HR[ 7]='0'+Tc.Seconds%10;
            Event_TimeCode_HR[ 8]=Tc.DropFrame?';':':';
            Event_TimeCode_HR[ 9]='0'+Tc.Frames/10;
            Event_TimeCode_HR[10]='0'+Tc.Frames%10;
            Event_TimeCode_HR[11]='\0';
            Event_TimeCode_HR[12]='\0';
        }
        else
        {
            Event_TimeCode.Hours=(MediaInfo_int8u)-1;
            Event_TimeCode.Minutes=(MediaInfo_int8u)-1;
            Event_TimeCode.Seconds=(MediaInfo_int8u)-1;
            Event_TimeCode.Frames=(MediaInfo_int8u)-1;
            Event_TimeCode.FramesPerSecond=(MediaInfo_int8u)-1;
            Event_TimeCode.DropFrame=(MediaInfo_int8u)-1;
            memset(Event_TimeCode_HR, 0x00, 13);
        }
    }
}

#endif //MediaInfo_EventsH
