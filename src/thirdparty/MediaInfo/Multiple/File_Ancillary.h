// File_Ancillary - Info for SCTE 20 streams
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
// Information about Ancillary data (SMPTE 291...)
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_AncillaryH
#define MediaInfo_AncillaryH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Ancillary
//***************************************************************************

class File_Ancillary : public File__Analyze
{
public :
    //In
    bool    WithTenBit;
    bool    WithChecksum;

    //In/Out
    struct buffered_data
    {
        size_t Size;
        int8u* Data;

        buffered_data()
        {
            Size=0;
            Data=NULL;
        }

        ~buffered_data()
        {
            delete[] Data; //Data=NULL;
        }
    };
    #if defined(MEDIAINFO_CDP_YES)
        std::vector<buffered_data*> Cdp_Data;
    #endif //defined(MEDIAINFO_CDP_YES)
    #if defined(MEDIAINFO_AFDBARDATA_YES)
        std::vector<buffered_data*> AfdBarData_Data;
    #endif //defined(MEDIAINFO_AFDBARDATA_YES)

    //Constructor/Destructor
    File_Ancillary();
    ~File_Ancillary();

private :
    //Buffer - Global
    void Read_Buffer_Continue();
};

} //NameSpace

#endif

