/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Duplication helper for some specific formats
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
#if defined(MEDIAINFO_MPEGTS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_MpegTs.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "ZenLib/ZtringList.h"
#include "ZenLib/File.h"
using namespace ZenLib;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Format
//***************************************************************************

#if MEDIAINFO_DUPLICATE
void File_MpegTs::File__Duplicate_Streams_Finish ()
{
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
        Complete_Stream->Duplicates_Speed_FromPID.clear();
}
#endif //MEDIAINFO_DUPLICATE

//***************************************************************************
// Options
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpegTs::Option_Manage()
{
    if (Complete_Stream && !Complete_Stream->Streams.empty())
    {
        #if MEDIAINFO_FILTER
            //File_Filter configuration
            if (Config->File_Filter_HasChanged())
            {
                bool Searching_Payload_Start=!Config->File_Filter_Get();
                for (int32u Pos=0x01; Pos<0x10; Pos++)
                    Complete_Stream->Streams[Pos]->Searching_Payload_Start_Set(Searching_Payload_Start); //base PID depends of File_Filter configuration
                Complete_Stream->Streams[0x0000]->Searching_Payload_Start_Set(true); //program_map
            }
        #endif //MEDIAINFO_FILTER

        #if MEDIAINFO_DUPLICATE
            //File__Duplicate configuration
            if (File__Duplicate_HasChanged())
            {
                for (size_t Pos=0x0000; Pos<0x2000; Pos++)
                    Complete_Stream->Streams[Pos]->ShouldDuplicate=false;
                Complete_Stream->Streams[0x0000]->ShouldDuplicate=true;

                //For each program
                for (complete_stream::transport_stream::programs::iterator Program=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs.begin(); Program!=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs.end(); ++Program)
                {
                    //Do we want this program?
                    bool Wanted=false;
                    for (std::map<const String, File__Duplicate_MpegTs*>::iterator Duplicate=Complete_Stream->Duplicates.begin(); Duplicate!=Complete_Stream->Duplicates.end(); ++Duplicate)
                    {
                        if (Duplicate->second->Wanted_program_numbers.find(Program->first)!=Duplicate->second->Wanted_program_numbers.end())
                            Wanted=true;
                        if (Duplicate->second->Wanted_program_map_PIDs.find(Program->second.pid)!=Duplicate->second->Wanted_program_map_PIDs.end())
                            Wanted=true;
                    }

                    //Enabling it if wanted
                    if (Wanted)
                    {
                        Complete_Stream->Streams[Program->second.pid]->ShouldDuplicate=true;
                        for (size_t Pos=0; Pos<Program->second.elementary_PIDs.size(); Pos++)
                            Complete_Stream->Streams[Program->second.elementary_PIDs[Pos]]->ShouldDuplicate=true;
                    }
                }
        }
        #endif //MEDIAINFO_DUPLICATE
    }
}

//***************************************************************************
// Set
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_DUPLICATE
bool File_MpegTs::File__Duplicate_Set (const Ztring &Value)
{
    //Form: "Code;Target"                           <--Generic
    //Form: "program_number" or                     <--clear it
    //Form: "program_number;file" or                <--the exported filename is filename.program_number
    //Form: "program_number;file://filename" or     <--the exported filename is specified by user
    //Form: "program_number;memory" or              <--This will be a MediaInfo memory block
    //Form: "program_number;memory://pointer:size"  <--Memory block is specified by user
    //WARNING: program_number & pointer must be in ***DECIMAL*** format.
    //Example: "451;memory://123456789:1316"
    ZtringList List(Value);

    //Backward compatibility
    bool Orders_ToRemove_Global=false;

    //Searching Target
    bool IsForUs=true; //True by default for backward compatibility
    std::vector<ZtringList::iterator> Targets_ToAdd;
    std::vector<ZtringList::iterator> Targets_ToRemove;
    std::vector<ZtringList::iterator> Orders_ToAdd;
    std::vector<ZtringList::iterator> Orders_ToRemove;
    for (ZtringList::iterator Current=List.begin(); Current<List.end(); ++Current)
    {
        //Detecting if we want to remove
        bool ToRemove=false;
        if (Current->find(__T('-'))==0)
        {
            ToRemove=true;
            Current->erase(Current->begin());
        }

        //Managing targets
        if (Current->find(__T("file:"))==0
         || Current->find(__T("memory:"))==0)
            (ToRemove?Targets_ToRemove:Targets_ToAdd).push_back(Current);
        //Parser name
        else if (Current->find(__T("parser="))==0)
        {
            if (*Current==__T("parser=MpegTs"))
                IsForUs=true;
            else
                IsForUs=false; //Backward compatibility with missing parser name
        }
        //Backward compatibility with "0"
        else if (*Current==__T("0"))
            Orders_ToRemove_Global=true;
        //Managing orders
        else
            (ToRemove?Orders_ToRemove:Orders_ToAdd).push_back(Current);
    }

    //For us?
    if (!IsForUs)
        return false;

    //Backward compatibility
    if (Orders_ToRemove_Global) //with "0"
    {
        for (std::vector<ZtringList::iterator>::iterator Order=Orders_ToAdd.begin(); Order<Orders_ToAdd.end(); ++Order)
            Orders_ToRemove.push_back(*Order);
        Orders_ToAdd.clear();
    }

    //For each target to add
    for (std::vector<ZtringList::iterator>::iterator Target=Targets_ToAdd.begin(); Target<Targets_ToAdd.end(); ++Target)
    {
        //Adding the target if it does not exist yet
        if (Complete_Stream->Duplicates.find(**Target)==Complete_Stream->Duplicates.end())
        {
            Complete_Stream->Duplicates[**Target]=new File__Duplicate_MpegTs(**Target);
            size_t Pos=Config->File__Duplicate_Memory_Indexes_Get(**Target);
            if (Pos!=Error)
            {
                if (Pos>=Complete_Stream->Duplicates_Speed.size())
                    Complete_Stream->Duplicates_Speed.resize(Pos+1);
                Complete_Stream->Duplicates_Speed[Pos]=Complete_Stream->Duplicates[**Target];
            }
        }

        //For each order to add
        for (std::vector<ZtringList::iterator>::iterator Order=Orders_ToAdd.begin(); Order<Orders_ToAdd.end(); ++Order)
            Complete_Stream->Duplicates[**Target]->Configure(**Order, false);

        //For each order to remove
        for (std::vector<ZtringList::iterator>::iterator Order=Orders_ToRemove.begin(); Order<Orders_ToRemove.end(); ++Order)
            Complete_Stream->Duplicates[**Target]->Configure(**Order, true);
    }

    //For each target to remove
    for (std::vector<ZtringList::iterator>::iterator Target=Targets_ToRemove.begin(); Target<Targets_ToRemove.end(); ++Target)
    {
        std::map<const String, File__Duplicate_MpegTs*>::iterator Pointer=Complete_Stream->Duplicates.find(**Target);
        if (Pointer!=Complete_Stream->Duplicates.end())
        {
            //Duplicates_Speed
            for (std::vector<File__Duplicate_MpegTs*>::iterator Duplicate=Complete_Stream->Duplicates_Speed.begin(); Duplicate<Complete_Stream->Duplicates_Speed.end(); ++Duplicate)
                if (*Duplicate==Pointer->second)
                    *Duplicate=NULL;

            //Duplicates_Speed_FromPID
            for (std::vector<std::vector<File__Duplicate_MpegTs*> >::iterator Duplicate_FromPID=Complete_Stream->Duplicates_Speed_FromPID.begin(); Duplicate_FromPID<Complete_Stream->Duplicates_Speed_FromPID.end(); ++Duplicate_FromPID)
                for (std::vector<File__Duplicate_MpegTs*>::iterator Duplicate=Duplicate_FromPID->begin(); Duplicate<Duplicate_FromPID->end(); ++Duplicate)
                    if (*Duplicate==Pointer->second)
                        *Duplicate=NULL;

            //Duplicate
            Complete_Stream->Duplicates.erase(**Target);
        }
    }

    //Informing the status has changed
    Complete_Stream->File__Duplicate_HasChanged_=true;
    if (Complete_Stream->Duplicates_Speed_FromPID.empty())
        Complete_Stream->Duplicates_Speed_FromPID.resize(0x2000);

    Complete_Stream->Duplicates_Speed_FromPID[0x00]=Complete_Stream->Duplicates_Speed;

    return true;
}
#endif //MEDIAINFO_DUPLICATE

//***************************************************************************
// Write
//***************************************************************************

#if MEDIAINFO_DUPLICATE
void File_MpegTs::File__Duplicate_Write ()
{
    const int8u* ToAdd=Buffer+Buffer_Offset-(size_t)Header_Size;
    size_t ToAdd_Size=(size_t)(Element_Size+Header_Size);

    std::vector<File__Duplicate_MpegTs*> &Dup_FromPID=Complete_Stream->Duplicates_Speed_FromPID[pid];
    size_t Duplicates_Speed_FromPID_Size=Complete_Stream->Duplicates_Speed_FromPID[pid].size();
    bool ToUpdate=false;
    for (size_t Pos=0; Pos<Duplicates_Speed_FromPID_Size; Pos++)
        if (Dup_FromPID[Pos] && Dup_FromPID[Pos]->Write(pid, ToAdd, ToAdd_Size))
            ToUpdate=true;
    if (ToUpdate)
    {
        Complete_Stream->Duplicates_Speed_FromPID.clear();
        Complete_Stream->Duplicates_Speed_FromPID.resize(0x2000);
        Complete_Stream->Duplicates_Speed_FromPID[0x0000]=Complete_Stream->Duplicates_Speed;
        size_t Duplicates_Speed_Size=Complete_Stream->Duplicates_Speed.size();
        for (size_t Pos=0; Pos<Duplicates_Speed_Size; Pos++)
        {
            File__Duplicate_MpegTs* Dup=Complete_Stream->Duplicates_Speed[Pos];
            size_t program_map_PIDs_Size=Complete_Stream->Duplicates_Speed[Pos]->program_map_PIDs.size();
            for (size_t program_map_PIDs_Pos=0; program_map_PIDs_Pos<program_map_PIDs_Size; program_map_PIDs_Pos++)
                if (Dup->program_map_PIDs[program_map_PIDs_Pos])
                {
                    bool AlreadyPresent=false;
                    for (size_t Duplicates_Speed_FromPID_Pos=0; Duplicates_Speed_FromPID_Pos<Complete_Stream->Duplicates_Speed_FromPID[program_map_PIDs_Pos].size(); Duplicates_Speed_FromPID_Pos++)
                        if (Complete_Stream->Duplicates_Speed_FromPID[program_map_PIDs_Pos][Duplicates_Speed_FromPID_Pos]==Dup)
                            AlreadyPresent=true;
                    if (!AlreadyPresent)
                        Complete_Stream->Duplicates_Speed_FromPID[program_map_PIDs_Pos].push_back(Dup);
                }
            size_t elementary_PIDs_Size=Complete_Stream->Duplicates_Speed[Pos]->program_map_PIDs.size();
            for (size_t elementary_PIDs_Pos=0; elementary_PIDs_Pos<elementary_PIDs_Size; elementary_PIDs_Pos++)
                if (Dup->elementary_PIDs[elementary_PIDs_Pos])
                {
                    bool AlreadyPresent=false;
                    for (size_t Duplicates_Speed_FromPID_Pos=0; Duplicates_Speed_FromPID_Pos<Complete_Stream->Duplicates_Speed_FromPID[elementary_PIDs_Pos].size(); Duplicates_Speed_FromPID_Pos++)
                        if (Complete_Stream->Duplicates_Speed_FromPID[elementary_PIDs_Pos][Duplicates_Speed_FromPID_Pos]==Dup)
                            AlreadyPresent=true;
                    if (!AlreadyPresent)
                        Complete_Stream->Duplicates_Speed_FromPID[elementary_PIDs_Pos].push_back(Dup);
                }
        }
    }
}
#endif //MEDIAINFO_DUPLICATE

//***************************************************************************
// Output_Buffer
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_DUPLICATE
size_t File_MpegTs::Output_Buffer_Get (const String &Code)
{
    if (Complete_Stream==NULL)
        return 0;
    std::map<const String, File__Duplicate_MpegTs*>::iterator Stream=Complete_Stream->Duplicates.find(Code);
    if (Stream==Complete_Stream->Duplicates.end())
        return 0;

    return Stream->second->Output_Buffer_Get();

    //Was used for test (AVC output), but is a lot too slow, must find something else
    /*
    if (size_t Size=Stream->second->Output_Buffer_Get())
        return Size;

    //Parsing Parsers
    for (size_t Stream_Pos=0; Stream_Pos<Streams.size(); Stream_Pos++)
        if (Streams[Stream_Pos].Parser)
            if (size_t Size=Streams[Stream_Pos].Parser->Output_Buffer_Get(Code))
                return Size;

    return 0;
    */
}
#endif //MEDIAINFO_DUPLICATE

//---------------------------------------------------------------------------
#if MEDIAINFO_DUPLICATE
size_t File_MpegTs::Output_Buffer_Get (size_t Pos)
{
    if (Complete_Stream!=NULL && Pos<Complete_Stream->Duplicates_Speed.size() && Complete_Stream->Duplicates_Speed[Pos]!=NULL)
        if (size_t Size=Complete_Stream->Duplicates_Speed[Pos]->Output_Buffer_Get())
            return Size;

    //Parsing Parsers
    /*
    for (size_t Stream_Pos=0; Stream_Pos<Complete_Stream->Streams.size(); Stream_Pos++)
        if (Complete_Stream->Streams[Stream_Pos].Parser)
            if (size_t Size=Complete_Stream->Streams[Stream_Pos].Parser->Output_Buffer_Get(Pos))
            {
                //Optimization
                //if (Output_Buffer_Get_Pos.size()<=Pos)
                //    Output_Buffer_Get_Pos.resize(Pos+1, (int16u)-1);
                //Output_Buffer_Get_Pos[Pos]=Stream_Pos;

                return Size;
            }
    */

    return 0;
}
#endif //MEDIAINFO_DUPLICATE

} //NameSpace

#endif //MEDIAINFO_MPEGTS_YES
