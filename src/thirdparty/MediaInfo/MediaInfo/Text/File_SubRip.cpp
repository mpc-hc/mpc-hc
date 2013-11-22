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
#if defined(MEDIAINFO_SUBRIP_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_SubRip.h"
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
    #include "MediaInfo/MediaInfo_Events_Internal.h"
#endif //MEDIAINFO_EVENTS
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Utils
//***************************************************************************

int64u SubRip_str2timecode(const char* Value)
{
    size_t Length=strlen(Value);
         if (Length>=8
     && Value[0]>='0' && Value[0]<='9'
     && Value[1]>='0' && Value[1]<='9'
     && Value[2]==':'
     && Value[3]>='0' && Value[3]<='9'
     && Value[4]>='0' && Value[4]<='9'
     && Value[5]==':'
     && Value[6]>='0' && Value[6]<='9'
     && Value[7]>='0' && Value[7]<='9')
    {
        int64u ToReturn=(int64u)(Value[0]-'0')*10*60*60*1000000000
                       +(int64u)(Value[1]-'0')   *60*60*1000000000
                       +(int64u)(Value[3]-'0')   *10*60*1000000000
                       +(int64u)(Value[4]-'0')      *60*1000000000
                       +(int64u)(Value[6]-'0')      *10*1000000000
                       +(int64u)(Value[7]-'0')         *1000000000;
        if (Length>=9 && (Value[8]=='.' || Value[8]==','))
        {
            if (Length>9+9)
                Length=9+9; //Nanoseconds max
            const char* Value_End=Value+Length;
            Value+=9;
            int64u Multiplier=100000000;
            while (Value<Value_End)
            {
                ToReturn+=(int64u)(*Value-'0')*Multiplier;
                Multiplier/=10;
                Value++;
            }
        }

        return ToReturn;
    }
    else if (Length>=2
     && Value[Length-1]=='s')
    {
        return (int64u)(atof(Value)*1000000000);
    }
    else
        return (int64u)-1;
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_SubRip::File_SubRip()
:File__Analyze()
{
    //Configuration
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_SubRip;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS

    //Init
    Frame_Count=0;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_SubRip::FileHeader_Begin()
{
    if (!IsSub && (Buffer_Size<File_Size && Buffer_Size<65536))
    {
        Element_WaitForMoreData();
        return false;
    }

    ZtringListList List;
    List.Separator_Set(0, __T("\n\n"));
    List.Separator_Set(1, __T("\n"));

    bool IsLocal=false;
    Ztring Temp;
    Temp.From_UTF8((const char*)Buffer, Buffer_Size>65536?65536:Buffer_Size);
    if (Temp.empty())
    {
        Temp.From_Local((const char*)Buffer, Buffer_Size>65536?65536:Buffer_Size); // Trying from local code page
        IsLocal=true;
    }
    Temp.FindAndReplace(__T("\r\n"), __T("\n"), 0, Ztring_Recursive);
    Temp.FindAndReplace(__T("\r"), __T("\n"), 0, Ztring_Recursive);
    List.Write(Temp);

    size_t IsOk=0;
    size_t IsNok=0;
    for (size_t Pos=0; Pos<List.size(); Pos++)
    {
        if (List(Pos, 0).To_int64u()==Pos+1)
            IsOk++;
        else
            IsNok++;

        if (List(Pos, 1).size()>22 && List(Pos, 1)[2]==__T(':') && List(Pos, 1)[5]==__T(':') && List(Pos, 1).find(__T(" --> "))!=string::npos)
            IsOk++;
        else
            IsNok++;
    }

    if (!IsOk || IsNok>IsOk/2)
    {
        Reject();
        return true;
    }

    if (!IsSub && File_Size!=(int64u)-1 && Buffer_Size!=File_Size)
    {
        Element_WaitForMoreData();
        return false;
    }

    if (!Status[IsAccepted])
    {
        Accept();
        Fill(Stream_General, 0, General_Format, "SubRip");
        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, 0, "Format", "SubRip");
        Fill(Stream_Text, 0, "Codec", "SubRip");
    }

    if (IsLocal)
        Temp.From_Local((const char*)Buffer, Buffer_Size);
    else
        Temp.From_UTF8((const char*)Buffer, Buffer_Size);
    Temp.FindAndReplace(__T("\r\n"), __T("\n"), 0, Ztring_Recursive);
    Temp.FindAndReplace(__T("\r"), __T("\n"), 0, Ztring_Recursive);
    List.Write(Temp);

    #if MEDIAINFO_DEMUX
        size_t Pos=0;
        for (;;)
        {
            if (Pos>=List.size())
                break;

            if (List[Pos].size()>=3)
            {
                Ztring PTS_Begin_String=List[Pos][1].SubString(Ztring(), __T(" --> "));
                Ztring PTS_End_String=List[Pos][1].SubString(__T(" --> "), Ztring());
                item Item;
                Item.PTS_Begin=SubRip_str2timecode(PTS_Begin_String.To_UTF8().c_str());
                Item.PTS_End=SubRip_str2timecode(PTS_End_String.To_UTF8().c_str());
                for (size_t Pos2=2; Pos2<List[Pos].size(); Pos2++)
                {
                    List[Pos][Pos2].Trim();
                    Item.Content+=List[Pos][Pos2];
                    if (Pos2+1<List[Pos].size())
                        Item.Content+=EOL;
                }
                Items.push_back(Item);
            }

            Pos++;
        }
    #endif //MEDIAINFO_DEMUX

    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_SubRip::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    Open_Buffer_Unsynch();
    return 1;
}
#endif //MEDIAINFO_SEEK

//---------------------------------------------------------------------------
void File_SubRip::Read_Buffer_Continue()
{
    Element_Offset=File_Size;
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_SUBRIP_YES
