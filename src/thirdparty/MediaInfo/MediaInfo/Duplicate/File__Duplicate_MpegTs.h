// File__Duplicate_MpegTs - Duplication of some formats
// Copyright (C) 2007-2011 MediaArea.net SARL, Info@MediaArea.net
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
#ifndef File__Duplicate_MpegTsH
#define File__Duplicate_MpegTsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Duplicate/File__Duplicate__Base.h"
#include "MediaInfo/Duplicate/File__Duplicate__Writer.h"
#include "ZenLib/ZtringListList.h"
#include <set>
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File__Duplicate_MpegTs
//***************************************************************************

class File__Duplicate_MpegTs : public File__Duplicate__Base
{
public :
    //Constructor/Destructor
    File__Duplicate_MpegTs(const Ztring &Target);

    //Set
    bool   Configure (const Ztring &Value, bool ToRemove);

    //Write
    bool   Write (int16u PID, const int8u* ToAdd=NULL, size_t ToAdd_Size=0);

    //Output buffer
    size_t Output_Buffer_Get (unsigned char** Output_Buffer=NULL);

//private :
    File__Duplicate__Writer Writer;

    //Configuration
    std::set<int16u> Wanted_program_numbers;
    std::set<int16u> Wanted_program_map_PIDs;
    std::set<int16u> Wanted_elementary_PIDs;
    std::set<int16u> Remove_program_numbers;
    std::set<int16u> Remove_program_map_PIDs;
    std::set<int16u> Remove_elementary_PIDs;

    //Current
    std::vector<int8u> program_map_PIDs;
    std::vector<int8u> elementary_PIDs;
    std::vector<int16u> elementary_PIDs_program_map_PIDs;

    struct buffer
    {
        int8u*          Buffer;
        size_t          Offset;
        size_t          Begin; //After pointer_field
        size_t          End;   //Before CRC
        size_t          Size;
        int8u           continuity_counter;
        int8u           version_number;
        int8u           FromTS_version_number_Last;
        bool            ConfigurationHasChanged;

        buffer()
        {
            Buffer=NULL;
            Offset=0;
            Begin=0;
            End=0;
            Size=0;
            continuity_counter=0xFF;
            version_number=0xFF;
            FromTS_version_number_Last=0xFF;
            ConfigurationHasChanged=true;
        }
        ~buffer()
        {
            delete[] Buffer; //Buffer=NULL;
        }
    };

    struct buffer_const
    {
        const int8u*    Buffer;
        size_t          Offset;
        size_t          Begin; //After pointer_field
        size_t          End;   //Before CRC
        size_t          Size;

        buffer_const()
        {
            Buffer=NULL;
            Offset=0;
            Begin=0;
            End=0;
            Size=0;
        }
    };

    struct buffer_big
    {
        int8u*          Buffer;
        size_t          Buffer_Size;
        size_t          Buffer_Size_Max;

        buffer_big()
        {
            Buffer=NULL;
            Buffer_Size=0;
            Buffer_Size_Max=0;
        }
        ~buffer_big()
        {
            delete[] Buffer; //Buffer=NULL;
        }
    };

    //Data
    bool Manage_PAT(const int8u* ToAdd, size_t ToAdd_Size);
    bool Manage_PMT(const int8u* ToAdd, size_t ToAdd_Size);

    //Buffers
    buffer_const                FromTS;
    std::map<int16u, buffer>    PAT;
    std::map<int16u, buffer>    PMT;
    std::map<int16u, buffer_big> BigBuffers; //key is pid

    //Helpers
    bool Parsing_Begin(const int8u* ToAdd, size_t ToAdd_Size, std::map<int16u, buffer> &ToModify);
    void Parsing_End(std::map<int16u, buffer> &ToModify);

    //Temp
    int16u StreamID;
};


} //NameSpace

#endif
