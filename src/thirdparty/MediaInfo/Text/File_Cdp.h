// File_Cdp - Info for CDP files
// Copyright (C) 2010-2010 MediaArea.net SARL, Info@MediaArea.net
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
using namespace std;
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
    float32 AspectRatio;

    //Constructor/Destructor
    File_Cdp();
    ~File_Cdp();

private :
    //Streams management
    void Streams_Fill();
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
        bool            IsFilled;

        stream()
        {
            Parser=NULL;
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
