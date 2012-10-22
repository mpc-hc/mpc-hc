// File_Cdp - Info for CDP files
// Copyright (C) 2010-2012 MediaArea.net SARL, Info@MediaArea.net
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
//
// Information about Caption Distribution Packet files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_CdpH
#define MediaInfo_File_CdpH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Cdp
//***************************************************************************

class File_Cdp : public File__Analyze
{
public :
    //In
    bool    WithAppleHeader;
    float64 AspectRatio;

    //Constructor/Destructor
    File_Cdp();
    ~File_Cdp();

private :
    //Streams management
    void Streams_Accept();
    void Streams_Update();
    void Streams_Update_PerStream(size_t Pos);
    void Streams_Finish();

    //Synchro
    void Read_Buffer_Unsynched();

    //Buffer - Global
    void Read_Buffer_Continue();

    //Elements
    void cdp_header();
    void time_code_section();
    void ccdata_section();
    void ccsvcinfo_section();
    void cdp_footer();
    void future_section();

    //Stream
    struct stream
    {
        File__Analyze*  Parser;
        size_t          StreamPos;
        bool            IsFilled;

        stream()
        {
            Parser=NULL;
            StreamPos=(size_t)-1;
            IsFilled=false;
        }

        ~stream()
        {
            delete Parser; //Parser=NULL;
        }
    };
    std::vector<stream*> Streams;
    size_t               Streams_Count;
};

} //NameSpace

#endif
