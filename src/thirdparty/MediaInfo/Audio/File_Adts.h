// File_Aac_Adts - Info for AAC (ADTS) Audio files
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
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
#ifndef MediaInfo_File_AdtsH
#define MediaInfo_File_AdtsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/Tag/File__Tags.h"
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Aac_Adts
//***************************************************************************

class File_Adts : public File__Analyze, public File__Tags_Helper
{
public :
    //In
    size_t Frame_Count_Valid;

    //Constructor/Destructor
    File_Adts();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish()                                                       {File__Tags_Helper::Streams_Finish();}

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Temp
    std::vector<int16u> aac_frame_lengths;
    int16u adts_buffer_fullness;
    int16u aac_frame_length;
    int8u  profile_ObjectType;
    int8u  sampling_frequency_index;
    int8u  channel_configuration;
    bool   id;
};

} //NameSpace

#endif
