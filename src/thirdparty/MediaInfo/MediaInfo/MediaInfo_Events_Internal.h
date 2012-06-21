// MediaInfo_Events - 
// Copyright (C) 2010-2011 MediaArea.net SARL, Info@MediaArea.net
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
