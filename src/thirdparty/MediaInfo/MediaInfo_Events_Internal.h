/*Helpers for MediaInfo events */

#ifndef MediaInfo_Events_InternalH
#define MediaInfo_Events_InternalH

#include <cstring>
using namespace std;

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
}

#endif //MediaInfo_EventsH
