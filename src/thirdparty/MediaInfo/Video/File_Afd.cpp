// File_Afd - Info for AFD files
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
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_AFD_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Afd.h"

//---------------------------------------------------------------------------
namespace MediaInfoLib
{
//---------------------------------------------------------------------------

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Afd_active_format[]=
{
    //1st value is for 4:3, 2nd is for 16:9
    "", //Undefined
    "Reserved",
    "Not recommended",
    "Not recommended",
    "Aspect ratio greater than 16:9", //Use GA94
    "Reserved",
    "Reserved",
    "Reserved",
    "4:3 full frame image / 16:9 full frame image",
    "4:3 full frame image / 4:3 pillarbox image",
    "16:9 letterbox image / 16:9 full frame image",
    "14:9 letterbox image / 14:9 pillarbox image",
    "Reserved",
    "4:3 full frame image, alternative 14:9 center / 4:3 pillarbox image, alternative 14:9 center",
    "16:9 letterbox image, alternative 14:9 center / 16:9 full frame image, alternative 14:9 center",
    "16:9 letterbox image, alternative 4:3 center / 16:9 full frame image, alternative 4:3 center",
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Afd::File_Afd()
:File__Analyze()
{
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Afd::Streams_Fill()
{
    //Filling
    Stream_Prepare(Stream_Video);
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Afd::Read_Buffer_Continue()
{
    if (!Status[IsAccepted])
        Accept("AFD");

    //Parsing
    Element_Begin("Active Format Description");
    bool  active_format_flag;
    int8u active_format;
    BS_Begin();
    Mark_0();
    Get_SB (active_format_flag,                                 "active_format_flag");
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Mark_1_NoTrustError();
    if (active_format_flag)
    {
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Get_S1 (4, active_format,                               "active_format"); Param_Info(Afd_active_format[active_format]);
    }
    else
        active_format=0;
    BS_End();
    Element_End();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_AFD_YES
