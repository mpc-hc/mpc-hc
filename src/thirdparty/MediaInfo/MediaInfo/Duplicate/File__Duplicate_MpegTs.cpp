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
#if defined(MEDIAINFO_MPEGPS_YES) || defined(MEDIAINFO_MPEGTS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Duplicate/File__Duplicate_MpegTs.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "ZenLib/ZtringList.h"
#include "ZenLib/File.h"
#include <cstring>
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
// CRC_32_Table
// A CRC is computed like this:
// Init: int32u CRC_32 = 0xFFFFFFFF;
// for each data byte do
//     CRC_32=(CRC_32<<8) ^ CRC_32_Table[(CRC_32>>24)^(data_byte)];
extern int32u Psi_CRC_32_Table[256];

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

File__Duplicate_MpegTs::File__Duplicate_MpegTs (const Ztring &Target)
: File__Duplicate__Base()
{
    Writer.Configure(Target);

    //Current
    program_map_PIDs.resize(0x2000, 0);
    elementary_PIDs.resize(0x2000, 0);
    elementary_PIDs_program_map_PIDs.resize(0x2000, 0);
}

//***************************************************************************
// Set
//***************************************************************************

//---------------------------------------------------------------------------
bool File__Duplicate_MpegTs::Configure (const Ztring &Value, bool ToRemove)
{
    //Form: "program_number"
    if (Value.find(__T("program_number="))==0)
    {
        int16u program_number=Ztring(Value.substr(15, std::string::npos)).To_int16u();
        if (ToRemove)
        {
            if (Wanted_program_numbers.find(program_number)!=Wanted_program_numbers.end())
                Wanted_program_numbers.erase(program_number);
            else if (Remove_program_numbers.find(program_number)==Remove_program_numbers.end())
                Remove_program_numbers.insert(program_number);
        }
        else
        {
            if (Remove_program_numbers.find(program_number)!=Remove_program_numbers.end())
                Remove_program_numbers.erase(program_number);
            if (Wanted_program_numbers.find(program_number)==Wanted_program_numbers.end())
                Wanted_program_numbers.insert(program_number);
        }
        if (!PAT.empty())
            PAT.begin()->second.ConfigurationHasChanged=true;
    }
    //Form: "program_map_PID"
    else if (Value.find(__T("program_map_PID="))==0)
    {
        int16u program_map_PID=Ztring(Value.substr(16, std::string::npos)).To_int16u();
        if (ToRemove)
        {
            if (Wanted_program_map_PIDs.find(program_map_PID)!=Wanted_program_map_PIDs.end())
                Wanted_program_map_PIDs.erase(program_map_PID);
            else if (Remove_program_map_PIDs.find(program_map_PID)==Remove_program_map_PIDs.end())
                Remove_program_map_PIDs.insert(program_map_PID);
        }
        else
        {
            if (Remove_program_map_PIDs.find(program_map_PID)!=Remove_program_map_PIDs.end())
                Remove_program_map_PIDs.erase(program_map_PID);
            if (Wanted_program_map_PIDs.find(program_map_PID)==Wanted_program_map_PIDs.end())
                Wanted_program_map_PIDs.insert(program_map_PID);
        }
        if (PMT.find(program_map_PID)!=PMT.end())
            PMT[program_map_PID].ConfigurationHasChanged=true;
    }
    //Form: "elementary_PID"
    else if (Value.find(__T("elementary_PID="))==0)
    {
        int16u elementary_PID=Ztring(Value.substr(15, std::string::npos)).To_int16u();
        if (ToRemove)
        {
            if (Wanted_elementary_PIDs.find(elementary_PID)!=Wanted_elementary_PIDs.end())
                Wanted_elementary_PIDs.erase(elementary_PID);
            else if (Remove_elementary_PIDs.find(elementary_PID)==Remove_elementary_PIDs.end())
                Remove_elementary_PIDs.insert(elementary_PID);
        }
        else
        {
            if (Remove_elementary_PIDs.find(elementary_PID)!=Remove_elementary_PIDs.end())
                Remove_elementary_PIDs.erase(elementary_PID);
            if (Wanted_elementary_PIDs.find(elementary_PID)==Wanted_elementary_PIDs.end())
                Wanted_elementary_PIDs.insert(elementary_PID);
        }
        if (PMT.find(elementary_PIDs_program_map_PIDs[elementary_PID])!=PMT.end())
            PMT[elementary_PIDs_program_map_PIDs[elementary_PID]].ConfigurationHasChanged=true;
    }
    //Old
    else
    {
        int16u program_number=Ztring(Value).To_int16u();
        if (ToRemove)
        {
            if (Wanted_program_numbers.find(program_number)!=Wanted_program_numbers.end())
                Wanted_program_numbers.erase(program_number);
            else if (Remove_program_numbers.find(program_number)==Remove_program_numbers.end())
                Remove_program_numbers.insert(program_number);
        }
        else
        {
            if (Remove_program_numbers.find(program_number)!=Remove_program_numbers.end())
                Remove_program_numbers.erase(program_number);
            if (Wanted_program_numbers.find(program_number)==Wanted_program_numbers.end())
                Wanted_program_numbers.insert(program_number);
        }
        if (!PAT.empty())
            PAT.begin()->second.ConfigurationHasChanged=true;
    }

    //Can be disabled?
    if (Wanted_program_numbers.empty()
     && Wanted_program_map_PIDs.empty()
     && Wanted_elementary_PIDs.empty()
     && Remove_program_numbers.empty()
     && Remove_program_map_PIDs.empty()
     && Remove_elementary_PIDs.empty())
        return true; //It can be erased
     else
        return false; //We always need it
}

//***************************************************************************
// Write
//***************************************************************************

bool File__Duplicate_MpegTs::Write (int16u PID, const int8u* ToAdd, size_t ToAdd_Size)
{
    if (elementary_PIDs[PID])
    {
        Writer.Write(ToAdd, ToAdd_Size);
        return false;
    }
    else if (program_map_PIDs[PID])
        return Manage_PMT(ToAdd, ToAdd_Size);
    else if (PID==0x0000)
        return Manage_PAT(ToAdd, ToAdd_Size);
    else
        return false;
}

bool File__Duplicate_MpegTs::Manage_PAT (const int8u* ToAdd, size_t ToAdd_Size)
{
    if (!Parsing_Begin(ToAdd, ToAdd_Size, PAT))
        return false;

    //Programs
    program_map_PIDs.clear();
    program_map_PIDs.resize(0x2000, 0);
    elementary_PIDs.clear();
    elementary_PIDs.resize(0x2000, 0);
    while (FromTS.Offset+4<=FromTS.End)
    {
        //For each program
        int16u program_number =CC2(FromTS.Buffer+FromTS.Offset+0);
        int16u program_map_PID=CC2(FromTS.Buffer+FromTS.Offset+2)&0x1FFF;
        if (Wanted_program_numbers.find(program_number)  !=Wanted_program_numbers.end()
         || Wanted_program_map_PIDs.find(program_map_PID)!=Wanted_program_map_PIDs.end())
        {
            //Integrating it
            program_map_PIDs[program_map_PID]=1;
            std::memcpy(PAT[StreamID].Buffer+PAT[StreamID].Offset, FromTS.Buffer+FromTS.Offset, 4);
            PAT[StreamID].Offset+=4;
            PMT[program_number].ConfigurationHasChanged=true;
        }
        FromTS.Offset+=4;
    }

    Parsing_End(PAT);

    //Reseting
    std::vector<int16u> StreamID_List;
    for (std::map<int16u, buffer>::iterator PAT_=PAT.begin(); PAT_!=PAT.end(); ++PAT_)
        if (PAT_->first!=StreamID)
            StreamID_List.push_back(PAT_->first);
    for (size_t Pos=0; Pos<StreamID_List.size(); Pos++)
        PAT[StreamID_List[Pos]].FromTS_version_number_Last=0xFF;

    return true;
}

bool File__Duplicate_MpegTs::Manage_PMT (const int8u* ToAdd, size_t ToAdd_Size)
{
    if (!Parsing_Begin(ToAdd, ToAdd_Size, PMT))
        return false;

    //Testing program_number
    if (Wanted_program_numbers.find(StreamID)==Wanted_program_numbers.end()
     && Wanted_program_map_PIDs.find(elementary_PIDs_program_map_PIDs[StreamID]) == Wanted_program_map_PIDs.end())
    {
        delete[] PMT[StreamID].Buffer; PMT[StreamID].Buffer=NULL;
        return false;
    }

    //program_info_length
    int16u program_info_length=CC2(FromTS.Buffer+FromTS.Offset+2)&0x0FFF;
    std::memcpy(PMT[StreamID].Buffer+PMT[StreamID].Offset, FromTS.Buffer+FromTS.Offset, 4+program_info_length);
    FromTS.Offset+=4+program_info_length;
    PMT[StreamID].Offset+=4+program_info_length;

    //elementary_PIDs
    while (FromTS.Offset+5<=FromTS.End)
    {
        //For each elementary_PID
        int16u elementary_PID=CC2(FromTS.Buffer+FromTS.Offset+1)&0x1FFF;
        int16u ES_info_length=CC2(FromTS.Buffer+FromTS.Offset+3)&0x0FFF;
        if (Wanted_elementary_PIDs.empty() || Wanted_elementary_PIDs.find(elementary_PID)!=Wanted_elementary_PIDs.end())
        {
            //Integrating it
            elementary_PIDs[elementary_PID]=1;
            elementary_PIDs_program_map_PIDs[elementary_PID]=StreamID;
            std::memcpy(PMT[StreamID].Buffer+PMT[StreamID].Offset, FromTS.Buffer+FromTS.Offset, 5+ES_info_length);
            PMT[StreamID].Offset+=5+ES_info_length;
        }
        else
            elementary_PIDs[elementary_PID]=0;
        FromTS.Offset+=5+ES_info_length;
    }

    Parsing_End(PMT);
    return true;
}

//***************************************************************************
// Helpers
//***************************************************************************

bool File__Duplicate_MpegTs::Parsing_Begin (const int8u* ToAdd, size_t ToAdd_Size, std::map<int16u, buffer> &ToModify_)
{
    //Managing big chunks
    int16u PID=((ToAdd[1]&0x1F)<<8)|ToAdd[2]; //BigEndian2int16u(ToAdd+1)&0x1FFF;
    if (ToAdd[1]&0x40) //payload_unit_start_indicator
    {
        FromTS.Buffer=ToAdd;
        FromTS.Size=ToAdd_Size;
        FromTS.Offset=0;
    }
    else
    {
        if (BigBuffers.find(PID)==BigBuffers.end())
            return false; //Start is missing
        if (ToAdd_Size<4 || BigBuffers[PID].Buffer_Size+ToAdd_Size-4>BigBuffers[PID].Buffer_Size_Max)
            return false; //Problem
        std::memcpy(BigBuffers[PID].Buffer+BigBuffers[PID].Buffer_Size, ToAdd+4, ToAdd_Size-4);
        BigBuffers[PID].Buffer_Size+=ToAdd_Size-4;

        FromTS.Buffer=BigBuffers[PID].Buffer;
        FromTS.Size=BigBuffers[PID].Buffer_Size;
        FromTS.Offset=0;
    }

    //adaptation_field_length
    int8u adaptation_field_length=0;
    if (CC1(FromTS.Buffer+3)&0x20) //adaptation_field_control (adaptation) == true
        adaptation_field_length=1+CC1(FromTS.Buffer+4);

    //pointer_field
    FromTS.Offset+=4+adaptation_field_length;
    int8u pointer_field=CC1(FromTS.Buffer+FromTS.Offset);

    //table_id
    FromTS.Offset+=1+pointer_field;
    int8u table_id=FromTS.Buffer[FromTS.Offset];
    if (table_id!=0x00 && table_id!=0x02) //Currently only PAT and PMT are handled
        return false;

    //section_length
    FromTS.Offset++;
    if (FromTS.Offset+2>FromTS.Size)
        return false;
    FromTS.Begin=FromTS.Offset-1;
    int16u section_length=CC2(FromTS.Buffer+FromTS.Offset)&0x0FFF;
    FromTS.End=4+adaptation_field_length+section_length;

    //Positionning just after section_length
    FromTS.Offset+=2;

    //Retrieving StreamID
    StreamID=CC2(FromTS.Buffer+FromTS.Offset);
    buffer &ToModify=ToModify_[StreamID];

    //version_number
    int8u FromTS_version_number=(CC1(FromTS.Buffer+FromTS.Offset+2)>>1)&0x1F;
    if (ToModify.version_number==0xFF && FromTS.End<=FromTS.Size-4) //Only if we have enough data
        ToModify.version_number=FromTS_version_number;
    if (ToModify.continuity_counter==0xFF && FromTS.End<=FromTS.Size-4) //Only if we have enough data
        ToModify.continuity_counter=FromTS.Buffer[3]&0xF;
    if (FromTS_version_number!=ToModify.FromTS_version_number_Last || ToModify.ConfigurationHasChanged)
    {
        if (FromTS.End<=FromTS.Size-4) //Only if we have enough data
        {
            ToModify.version_number++;
            if (ToModify.version_number>0x1F)
                ToModify.version_number=0;
            ToModify.FromTS_version_number_Last=FromTS_version_number;
            ToModify.ConfigurationHasChanged=false;
        }
    }
    else
    {
        if (ToModify.Buffer==NULL)
            return false;

        //This is the same as before --> Copying the last version, except continuity_counter (incremented)
        for (size_t Pos=0; Pos<ToModify.Size; Pos+=188)
        {
            ToModify.continuity_counter++;
            if (ToModify.continuity_counter>0x0F)
                ToModify.continuity_counter=0x00;
            ToModify.Buffer[Pos+3]&=0xF0;
            ToModify.Buffer[Pos+3]|=ToModify.continuity_counter;
        }

        //Managing big chunks
        if (BigBuffers.find(PID)!=BigBuffers.end())
            BigBuffers.erase(BigBuffers.find(PID));

        Writer.Write(ToModify.Buffer, ToModify.Size);
        return false;
    }

    //Test if we have enough data
    if (FromTS.End>FromTS.Size-4)
    {
        //Waiting for more data
        if (BigBuffers[PID].Buffer==NULL)
        {
            //Saving the data (not done at the beginning)
            BigBuffers[PID].Buffer_Size=FromTS.Size;
            BigBuffers[PID].Buffer_Size_Max=FromTS.End+188;
            BigBuffers[PID].Buffer=new int8u[BigBuffers[PID].Buffer_Size_Max];
            std::memcpy(BigBuffers[PID].Buffer, ToAdd, ToAdd_Size);
        }
        return false;
    }

    //Verifying CRC
    int32u CRC_32=0xFFFFFFFF;
    for (int32u CRC_32_Offset=(int32u)FromTS.Begin; CRC_32_Offset<FromTS.End+4; CRC_32_Offset++) //After syncword
        CRC_32=(CRC_32<<8) ^ Psi_CRC_32_Table[(CRC_32>>24)^(FromTS.Buffer[CRC_32_Offset])];
    if (CRC_32)
        return false; //Problem

    //Copying
    if (ToModify.Buffer!=NULL && ToModify.Size<FromTS.Size)
        {delete[] ToModify.Buffer; ToModify.Buffer=NULL;}
    if (ToModify.Buffer==NULL)
        ToModify.Buffer=new int8u[FromTS.Size+(FromTS.Size/188)*4];
    std::memcpy(ToModify.Buffer, FromTS.Buffer, FromTS.Begin+8); //Only up to last_section_number included
    ToModify.Offset=FromTS.Offset;
    ToModify.Begin=FromTS.Begin;
    ToModify.End=FromTS.End;
    ToModify.Size=FromTS.Size;

    //Changing version_number
    int8u ToReplace=FromTS.Buffer[FromTS.Offset+2];
    ToReplace&=0xC1; //11000001, for removing old version_number
    ToReplace|=ToModify.version_number<<1; //merging, with 1 bit offset
    ToModify.Buffer[ToModify.Offset+2]=ToReplace;

    //Positionning after last_section_number
    ToModify.Offset+=5;
    FromTS.Offset+=5;

    return true;
}

void File__Duplicate_MpegTs::Parsing_End (std::map<int16u, buffer> &ToModify_)
{
    buffer &ToModify=ToModify_[StreamID];

    ToModify.End=ToModify.Offset;
    if (ToModify.End+4>ToModify.Size)
        return; //There was an error somewhere!

    //section_length
    int8u ToReplace=CC1(ToModify.Buffer+ToModify.Begin+1)&0xF0; //before section_length
    int16u section_length=(int16u)(ToModify.End-ToModify.Begin+1); //+4 for CRC, -3 for header size
    ToReplace|=section_length>>8;
    ToModify.Buffer[ToModify.Begin+1+0]=ToReplace;
    ToModify.Buffer[ToModify.Begin+1+1]=(int8u)(section_length&0xFF);

    //CRC32
    int32u CRC_32=0xFFFFFFFF;
    for (size_t Buffer_CRC_Pos=ToModify.Begin; Buffer_CRC_Pos<ToModify.End; Buffer_CRC_Pos++)
        CRC_32=(CRC_32<<8) ^ Psi_CRC_32_Table[(CRC_32>>24)^(ToModify.Buffer[Buffer_CRC_Pos])];

    ToModify.Buffer[ToModify.Offset+0]=(CRC_32>>24)&0xFF;
    ToModify.Buffer[ToModify.Offset+1]=(CRC_32>>16)&0xFF;
    ToModify.Buffer[ToModify.Offset+2]=(CRC_32>> 8)&0xFF;
    ToModify.Buffer[ToModify.Offset+3]= CRC_32     &0xFF;

    //Managing big chunks
    for (size_t Pos=188; Pos<ToModify.Size; Pos+=188)
    {
        std::memmove(ToModify.Buffer+Pos+4, ToModify.Buffer+Pos, ToModify.Size-Pos);
        std::memcpy(ToModify.Buffer+Pos, ToModify.Buffer, 4);
        ToModify.Buffer[Pos+1]&=0xBF; //Removing payload_unit_start_indicator
        ToModify.Offset+=4;
        ToModify.Size+=4;

        ToModify.continuity_counter++;
        if (ToModify.continuity_counter>0x0F)
            ToModify.continuity_counter=0x00;
        ToModify.Buffer[Pos+3]&=0xF0;
        ToModify.Buffer[Pos+3]|=ToModify.continuity_counter;
    }
    while (ToModify.Size-(ToModify.Offset+4)>188)
        ToModify.Size-=188;

    //Padding
    for (size_t Buffer_CRC_Pos=ToModify.End+4; Buffer_CRC_Pos<ToModify.Size; Buffer_CRC_Pos++)
        ToModify.Buffer[Buffer_CRC_Pos]=0xFF;

    Writer.Write(ToModify.Buffer, ToModify.Size);

    //Managing big chunks
    int16u PID=((ToModify.Buffer[1]&0x1F)<<8)|ToModify.Buffer[2]; //BigEndian2int16u(ToAdd+1)&0x1FFF;
    if (BigBuffers.find(PID)!=BigBuffers.end())
        BigBuffers.erase(BigBuffers.find(PID));
}

//***************************************************************************
// Output_Buffer
//***************************************************************************

//---------------------------------------------------------------------------
size_t File__Duplicate_MpegTs::Output_Buffer_Get (unsigned char** Output_Buffer)
{
    return Writer.Output_Buffer_Get(Output_Buffer);
}

} //NameSpace

#endif

