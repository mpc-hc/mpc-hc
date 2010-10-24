// File_Dpx - Info for DPX (SMPTE 268M) files
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

//---------------------------------------------------------------------------
//
// Examples:
// http://samples.mplayerhq.hu/FLV/
//
// Reverse engineering
// http://osflash.org/documentation/amf/astypes
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_DPX_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Dpx.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Dpx::File_Dpx()
:File__Analyze()
{
    //Configuration
    ParserName=_T("DPX");
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dpx::Streams_Finish()
{
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Dpx::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<4)
        return false; //Must wait for more data

    if (CC4(Buffer)!=0x53445058) //"SPDX"
    {
        Reject();
        return false;
    }

    //All should be OK...
    return true;
}

//---------------------------------------------------------------------------
void File_Dpx::Read_Buffer_Continue()
{
    //Parsing
    Element_Begin("File information");
    Skip_String(4,                                              "Magic number");
    Element_End();

    FILLING_BEGIN();
        //Filling
        Accept();

        Fill(Stream_General, 0, General_Format, "DPX");

        Finish();
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_DPX_YES
