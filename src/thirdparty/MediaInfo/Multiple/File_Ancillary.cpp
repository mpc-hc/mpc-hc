// File_AncillaryDataPacket - Info for SCTE 20 streams
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
#if defined(MEDIAINFO_ANCILLARY_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Ancillary.h"
#if defined(MEDIAINFO_EIA608_YES)
    #include "MediaInfo/Text/File_Eia608.h"
#endif
#if defined(MEDIAINFO_EIA708_YES)
    #include "MediaInfo/Text/File_Eia708.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#include <cstring>
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace MediaInfoLib
{
//---------------------------------------------------------------------------

//***************************************************************************
// Infos
//***************************************************************************

const char* Ancillary_DataID(int8u DataID, int8u SecondaryDataID)
{
         if (DataID==0x00)
        return "Undefined format";
    else if (DataID<=0x03)
        return "Reserved";
    else if (DataID<=0x0F)
        return "Reserved for 8-bit applications";
    else if (DataID<=0x3F)
        return "Reserved";
    else if (DataID==0x41)
    {
        //SMPTE 2016-3-2007
        switch (SecondaryDataID)
        {
            case 0x05 : return "Bar Data";
            default   : return "Internationally registered";
        }
    }
    else if (DataID==0x45)
    {
        //SMPTE 2020-1-2008
        switch (SecondaryDataID)
        {
            case 0x01 : return "Audio Metadata - No association";
            case 0x02 : return "Audio Metadata - Channels 1/2";
            case 0x03 : return "Audio Metadata - Channels 3/4";
            case 0x04 : return "Audio Metadata - Channels 5/6";
            case 0x05 : return "Audio Metadata - Channels 7/8";
            case 0x06 : return "Audio Metadata - Channels 9/10";
            case 0x07 : return "Audio Metadata - Channels 11/12";
            case 0x08 : return "Audio Metadata - Channels 13/14";
            case 0x09 : return "Audio Metadata - Channels 15/16";
            default   : return "SMPTE 2020-1-2008?";
        }
    }
    else if (DataID<=0x4F)
        return "Internationally registered";
    else if (DataID<=0x5F)
        return "Reserved";
    else if (DataID==0x60)
        return "Ancillary time code (Internationally registered)";
    else if (DataID==0x61)
    {
        switch (SecondaryDataID)
        {
            case 0x01 : return "CEA-708 (CDP)";
            case 0x02 : return "CEA-608";
            default   : return "S334-1-2007 Defined data services?";
        }
    }
    else if (DataID==0x62)
    {
        switch (SecondaryDataID)
        {
            case 0x01 : return "Program description";
            case 0x02 : return "Data broadcast";
            case 0x03 : return "VBI data";
            default   : return "S334-1-2007 Variable-format data services?";
        }
    }
    else if (DataID<=0x7F)
        return "Internationally registered";
    else if (DataID==0x80)
        return "Ancillary packet marked for deletion";
    else if (DataID<=0x83)
        return "Reserved";
    else if (DataID==0x84)
        return "Optional ancillary packet data end marker";
    else if (DataID<=0x87)
        return "Reserved";
    else if (DataID==0x88)
        return "Optional ancillary packet data start marker";
    else if (DataID<=0x9F)
        return "Reserved";
    else if (DataID<=0xBF)
        return "Internationally registered";
    else if (DataID<=0xCF)
        return "User application";
    else if (DataID<=0xDF)
        return "Internationally registered";
    else
        return "Internationally registered";
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Ancillary::File_Ancillary()
:File__Analyze()
{
    //Configuration
    ParserName=_T("Ancillary");

    //In
    WithTenBit=false;
    WithChecksum=false;
}

//---------------------------------------------------------------------------
File_Ancillary::~File_Ancillary()
{
    for (size_t Pos=0; Pos<Cdp_Data.size(); Pos++)
        delete Cdp_Data[Pos]; //Cdp_Data[Pos]=NULL;
    for (size_t Pos=0; Pos<AfdBarData_Data.size(); Pos++)
        delete AfdBarData_Data[Pos]; //AfdBarData_Data[Pos]=NULL;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ancillary::Read_Buffer_Continue()
{
    if (Element_Size==0)
    {
        //Clearing old data
        for (size_t Pos=0; Pos<Cdp_Data.size(); Pos++)
            delete Cdp_Data[Pos]; //Cdp_Data[0]=NULL;
        Cdp_Data.clear();
        for (size_t Pos=0; Pos<AfdBarData_Data.size(); Pos++)
            delete AfdBarData_Data[Pos]; //AfdBarData_Data[0]=NULL;
        AfdBarData_Data.clear();
        return;
    }

    //Parsing
    int8u DataID, SecondaryDataID, DataCount;
    Get_L1 (DataID,                                             "Data ID");
    if (WithTenBit)
        Skip_L1(                                                "Parity+Unused"); //even:1, odd:2
    Get_L1 (SecondaryDataID,                                    "Secondary Data ID"); Param_Info(Ancillary_DataID(DataID, SecondaryDataID));
    if (WithTenBit)
        Skip_L1(                                                "Parity+Unused"); //even:1, odd:2
    Get_L1 (DataCount,                                          "Data count");
    if (WithTenBit)
        Skip_L1(                                                "Parity+Unused"); //even:1, odd:2

    //Buffer
    int8u* Payload=new int8u[DataCount];
    for(int8u Pos=0; Pos<DataCount; Pos++)
    {
        Get_L1 (Payload[Pos],                                   "Data");
        if (WithTenBit)
            Skip_L1(                                            "Parity+Unused"); //even:1, odd:2
    }

    //Parsing
    if (WithChecksum)
        Skip_L1(                                                "Checksum");
    if (WithTenBit)
        Skip_L1(                                                "Parity+Unused"); //even:1, odd:2
    Element_End();

    FILLING_BEGIN();
        switch (DataID)
        {
            case 0x41 : // (from SMPTE 2016-3)
                        switch (SecondaryDataID)
                        {
                            case 0x05 : //Bar Data (from SMPTE 2016-3), saving data for future use
                                        #if defined(MEDIAINFO_AFDBARDATA_YES)
                                        {
                                            buffered_data* AfdBarData=new buffered_data;
                                            AfdBarData->Data=new int8u[(size_t)DataCount];
                                            std::memcpy(AfdBarData->Data, Payload, (size_t)DataCount);
                                            AfdBarData->Size=(size_t)DataCount;
                                            AfdBarData_Data.push_back(AfdBarData);
                                        }
                                        #endif //MEDIAINFO_AFDBARDATA_YES
                                        break;
                            default   : ;
                            ;
                        }
                        break;
            case 0x45 : // (from SMPTE 2020-1)
                        switch (SecondaryDataID)
                        {
                            case 0x01 : //No association
                            case 0x02 : //Channel pair 1/2
                            case 0x03 : //Channel pair 3/4
                            case 0x04 : //Channel pair 5/6
                            case 0x05 : //Channel pair 7/8
                            case 0x06 : //Channel pair 9/10
                            case 0x07 : //Channel pair 11/12
                            case 0x08 : //Channel pair 13/14
                            case 0x09 : //Channel pair 15/16
                                        break;
                            default   : ;
                            ;
                        }
                        break;
            case 0x61 : //Defined data services (from SMPTE 331-1)
                        switch (SecondaryDataID)
                        {
                            case 0x01 : //CDP (from SMPTE 331-1), saving data for future use
                                        #if defined(MEDIAINFO_CDP_YES)
                                        {
                                            buffered_data* Cdp=new buffered_data;
                                            Cdp->Data=new int8u[(size_t)DataCount];
                                            std::memcpy(Cdp->Data, Payload, (size_t)DataCount);
                                            Cdp->Size=(size_t)DataCount;
                                            Cdp_Data.push_back(Cdp);
                                        }
                                        #endif //MEDIAINFO_CDP_YES
                                        break;
                            case 0x02 : //CEA-608 (from SMPTE 331-1)
                                        #if defined(MEDIAINFO_EIA608_YES)
                                        if (DataCount==3) //This must be 3-byte data
                                        {
                                            //CEA-608 in video presentation order
                                        }
                                        #endif //MEDIAINFO_EIA608_YES
                                        break;
                            default   : ;
                            ;
                        }
                        break;
            case 0x62 : //Variable-format data services (from SMPTE 331-1)
                        switch (SecondaryDataID)
                        {
                            case 0x01 : //Program description (from SMPTE 331-1),
                                        break;
                            case 0x02 : //Data broadcast (from SMPTE 331-1)
                                        break;
                            case 0x03 : //VBI data (from SMPTE 331-1)
                                        break;
                            default   : ;
                            ;
                        }
                        break;
            default   : ;
        }
    FILLING_END();

    delete[] Payload; //Payload=NULL
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_ANCILLARY_YES

