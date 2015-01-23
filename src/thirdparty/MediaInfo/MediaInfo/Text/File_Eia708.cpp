/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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
#if defined(MEDIAINFO_EIA708_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_Eia708.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Eia708::File_Eia708()
:File__Analyze()
{
    //Config
    PTS_DTS_Needed=true;
    MustSynchronize=true;

    //In
    cc_type=(int8u)-1;
    AspectRatio=((float32)4)/3; //Default to 4:3
    ParserName=__T("EIA-708");

    //Stream
    service_number=(int8u)-1;

    //Temp
    StandAloneCommand=false;
    HasContent=false;
    DataDetected=0x0000000000000000LL;
}

//---------------------------------------------------------------------------
File_Eia708::~File_Eia708()
{
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        delete Streams[Pos]; //Streams[Pos]=NULL
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Eia708::Streams_Fill()
{
    if (Config->File_Eia708_DisplayEmptyStream_Get() && Streams.size()<2)
        Streams.resize(2);

    if (ServiceDescriptors)
    {
        for (servicedescriptors708::iterator ServiceDescriptor=ServiceDescriptors->ServiceDescriptors708.begin(); ServiceDescriptor!=ServiceDescriptors->ServiceDescriptors708.end(); ++ServiceDescriptor)
        {
            service_number=ServiceDescriptor->first;
            block_size=0;
            Service();
        }
    }

    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        if (Streams[Pos] || (Pos && Pos<2 && Config->File_Eia708_DisplayEmptyStream_Get()))
        {
            Stream_Prepare(Stream_Text);
            Fill(Stream_Text, StreamPos_Last, Text_ID, Pos);
            Fill(Stream_Text, StreamPos_Last, "CaptionServiceName", Pos);
            (*Stream_More)[StreamKind_Last][StreamPos_Last](Ztring().From_Local("CaptionServiceName"), Info_Options)=__T("N NT");
            Fill(Stream_Text, StreamPos_Last, Text_Format, "EIA-708");
            Fill(Stream_Text, StreamPos_Last, Text_StreamSize, 0);
            Fill(Stream_Text, StreamPos_Last, Text_BitRate_Mode, "CBR");
            if (Config->ParseSpeed>=1.0)
            {
                Fill(Stream_Text, StreamPos_Last, "CaptionServiceContent_IsPresent", (DataDetected&((int64u)1)<<Pos)?"Yes":"No", Unlimited, true, true); //1 bit per service
                (*Stream_More)[Stream_Text][StreamPos_Last](Ztring().From_Local("CaptionServiceContent_IsPresent"), Info_Options)=__T("N NT");
            }
            if (ServiceDescriptors)
            {
                servicedescriptors708::iterator ServiceDescriptor=ServiceDescriptors->ServiceDescriptors708.find((int8u)Pos);
                if (ServiceDescriptor!=ServiceDescriptors->ServiceDescriptors708.end())
                {
                    Fill(Stream_Text, StreamPos_Last, Text_Language, ServiceDescriptor->second.language, true);
                    Fill(Stream_Text, StreamPos_Last, "CaptionServiceDescriptor_IsPresent", "Yes", Unlimited, true, true);
                    (*Stream_More)[Stream_Text][StreamPos_Last](Ztring().From_Local("CaptionServiceDescriptor_IsPresent"), Info_Options)=__T("N NT");
                }
                else //ServiceDescriptors pointer is for the support by the transport layer of the info
                {
                    Fill(Stream_Text, StreamPos_Last, "CaptionServiceDescriptor_IsPresent", "No", Unlimited, true, true);
                    (*Stream_More)[Stream_Text][StreamPos_Last](Ztring().From_Local("CaptionServiceDescriptor_IsPresent"), Info_Options)=__T("N NT");
                }
            }
        }
}

//---------------------------------------------------------------------------
void File_Eia708::Streams_Finish()
{
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Eia708::Synchronize()
{
    if (cc_type!=3)
        return false; //Waiting for sync from underlying layer

    if (!Status[IsAccepted])
        Accept("EIA-708");

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Eia708::Synched_Test()
{
    if (cc_type==4) //Magic value saying that the buffer must be kept (this is only a point of synchro from the undelying layer)
        Buffer_Offset=Buffer_Size; //Sync point

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Eia708::Read_Buffer_Init()
{
}

//---------------------------------------------------------------------------
void File_Eia708::Read_Buffer_Continue()
{
    FrameInfo.PTS=FrameInfo.DTS;
}

//---------------------------------------------------------------------------
void File_Eia708::Read_Buffer_Unsynched()
{
    for (service_number=1; service_number<Streams.size(); service_number++)
        if (Streams[service_number])
        {
            //Per window
            for (size_t WindowID=0; WindowID<Streams[service_number]->Windows.size(); WindowID++)
            {
                window* Window=Streams[service_number]->Windows[WindowID];
                if (Window)
                    for (size_t Pos_Y=0; Pos_Y<Window->Minimal.CC.size(); Pos_Y++)
                    {
                        for (size_t Pos_X=0; Pos_X<Window->Minimal.CC[Pos_Y].size(); Pos_X++)
                        {
                            Window->Minimal.CC[Pos_Y][Pos_X].Value=L' ';
                            Window->Minimal.CC[Pos_Y][Pos_X].Attribute=0;
                        }
                    }
            }

            //Global display
            for (size_t Pos_Y=0; Pos_Y<Streams[service_number]->Minimal.CC.size(); Pos_Y++)
            {
                for (size_t Pos_X=0; Pos_X<Streams[service_number]->Minimal.CC[Pos_Y].size(); Pos_X++)
                {
                    Streams[service_number]->Minimal.CC[Pos_Y][Pos_X].Value=L' ';
                    Streams[service_number]->Minimal.CC[Pos_Y][Pos_X].Attribute=0;
                }
            }
        }

    #if MEDIAINFO_EVENTS
        for (service_number=1; service_number<Streams.size(); service_number++)
            if (Streams[service_number])
                HasChanged();
    #endif //MEDIAINFO_EVENTS
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Eia708::Header_Parse()
{
    //Parsing
    int8u packet_size, sequence_number;
    BS_Begin();
    Get_S1(2, sequence_number,                                  "sequence_number");
    Get_S1(6, packet_size,                                      "packet_size_code");
    BS_End();

    Header_Fill_Code(0, __T("DTVCC packet"));
    Header_Fill_Size(packet_size==0?128:(packet_size*2));
}

//---------------------------------------------------------------------------
void File_Eia708::Data_Parse()
{
    //Parsing
    while (Element_Offset<Element_Size)
    {
        BS_Begin();
        Get_S1(3, service_number,                                   "service_number");
        Get_S1(5, block_size,                                       "block_size");
        if (service_number==7)
        {
            Mark_0();
            Mark_0();
            Get_S1(6, service_number,                               "extended_service_number");
        }
        BS_End();

        if (service_number)
        {
            Element_Begin1("Service Block Packet");
            Service();
            Element_End0();
        }
    }
}

//---------------------------------------------------------------------------
void File_Eia708::Service()
{
    if (service_number>=Streams.size())
        Streams.resize(service_number+1);
    if (Streams[service_number]==NULL)
    {
        Streams[service_number]=new stream;
        Streams[service_number]->Minimal.CC.resize(15);
        for (int8u Pos_Y=0; Pos_Y<15; Pos_Y++)
            Streams[service_number]->Minimal.CC[Pos_Y].resize((size_t)(24*AspectRatio));
        Streams[service_number]->Windows.resize(8);
    }

    for (int8u Pos=0; Pos<block_size; Pos++)
    {
        int8u cc_data_1;
        Get_B1(cc_data_1,                                   "cc_data");
        switch (cc_data_1)
        {
            //CEA-708-D, Section 7.1.4 (C0)
            case 0x00 : NUL(); break; //NUL
            case 0x03 : ETX(); break; //End Of Text
            case 0x08 : BS(); break; //BackSpace
            case 0x0C : FF(); break; //Form Feed
            case 0x0D : CR(); break; //Carriage Return
            case 0x0E : HCR(); break; //Horizontal Carriage Return
            case 0x01 :
            case 0x02 :
            case 0x04 :
            case 0x05 :
            case 0x06 :
            case 0x07 :
            case 0x09 :
            case 0x0A :
            case 0x0B :
            case 0x0F : break; //1-byte long undefined code
            case 0x10 : //EXT1
                        {
                            int8u cc_data_2;
                            Get_B1(cc_data_2,               "cc_data");
                            Pos++;
                            switch (cc_data_2)
                            {
                                //CEA-708-D, Section 7.1.8 (G2)
                                case 0x20 : Character_Fill(L' '     ); break;
                                case 0x21 : Character_Fill(L' '     ); break;
                                case 0x25 : Character_Fill(L'.'     ); Character_Fill(L'.'); Character_Fill(L'.'); break;
                                case 0x2A : Character_Fill(L'S'     ); break;
                                case 0x2C : Character_Fill(L'O'     ); Character_Fill(L'E'); break;
                                case 0x30 : Character_Fill(L' '     ); break; //(Solid Block)
                                case 0x31 : Character_Fill(L'\''    ); break;
                                case 0x32 : Character_Fill(L'\''    ); break;
                                case 0x33 : Character_Fill(L'\''    ); break;
                                case 0x34 : Character_Fill(L'\''    ); break;
                                case 0x35 : Character_Fill(L'.'     ); break;
                                case 0x39 : Character_Fill(L'_'     ); break; //TM
                                case 0x3A : Character_Fill(L's'     ); break;
                                case 0x3C : Character_Fill(L'_'     ); break; //oe
                                case 0x3D : Character_Fill(L'_'     ); break; //SM
                                case 0x3F : Character_Fill(L'Y'     ); break;
                                case 0x76 : Character_Fill(L'_'     ); break; //1/8
                                case 0x77 : Character_Fill(L'_'     ); break; //3/8
                                case 0x78 : Character_Fill(L'_'     ); break; //5/8
                                case 0x79 : Character_Fill(L'_'     ); break; //7/8
                                case 0x7A : Character_Fill(L'+'     ); break;
                                case 0x7B : Character_Fill(L'+'     ); break;
                                case 0x7C : Character_Fill(L'+'     ); break;
                                case 0x7D : Character_Fill(L'+'     ); break;
                                case 0x7E : Character_Fill(L'+'     ); break;
                                case 0x7F : Character_Fill(L'+'     ); break;
                                //CEA-708-D, Section 7.1.9 (G3)
                                case 0x80 : Character_Fill(L'_'     ); break; //(CC)
                                default   :
                                            //CEA-708-D, Section 7.1.10 (C2)
                                            if (cc_data_2>=0x08 && cc_data_2<0x10)
                                            {
                                                Skip_B1(                    "EXT1 1 byte");
                                                Pos++;
                                            }
                                            else if (cc_data_2>=0x10 && cc_data_2<0x18)
                                            {
                                                Skip_B2(                    "EXT1 2 byte");
                                                Pos+=2;
                                            }
                                            else if (cc_data_2>=0x18 && cc_data_2<0x20)
                                            {
                                                Skip_B3(                    "EXT1 3 byte");
                                                Pos+=3;
                                            }
                                            //CEA-708-D, Section 7.1.11 (C3)
                                            else if (cc_data_2>=0x80 && cc_data_2<0x88)
                                            {
                                                Skip_B4(                    "EXT1 4 byte");
                                                Pos+=4;
                                            }
                                            else if (cc_data_2>=0x88 && cc_data_2<0x90)
                                            {
                                                Skip_B5(                    "EXT1 5 byte");
                                                Pos+=5;
                                            }
                                            else if (cc_data_2>=0x90 && cc_data_2<0xA0)
                                            {
                                                //CEA-708-D, Section 7.1.11.2 (Variable Length Codes)
                                                int8u Length;
                                                BS_Begin();
                                                Skip_S1(2,                  "Type");
                                                Mark_0();
                                                Get_S1 (5, Length,          "Length");
                                                Skip_XX(Length,             "Data");
                                                Pos+=1+Length;
                                            }
                                            //CEA-708-D, Section 7.1.9 (G3)
                                            else
                                                Character_Fill(L'_'     );
                            }
                        }
                        break;
            case 0x11 :
            case 0x12 :
            case 0x13 :
            case 0x14 :
            case 0x15 :
            case 0x16 :
            case 0x17 : //2-byte long undefined
                        {
                            Skip_B1(                        "Undefined");
                            Pos++;
                        }
                        break;
            case 0x18 : //P16
                        {
                            Skip_B2(                        "P16");
                            Pos+=2;
                        }
                        break;
            case 0x19 :
            case 0x1A :
            case 0x1B :
            case 0x1C :
            case 0x1D :
            case 0x1E :
            case 0x1F : //3-byte long undefined
                        {
                            Skip_B2(                        "Undefined");
                            Pos+=2;
                        }
                        break;
            //CEA-708-D, Section 7.1.6 (G0)
            case 0x20 : Character_Fill(L' '     ); break;
            case 0x21 : Character_Fill(L'!'     ); break;
            case 0x22 : Character_Fill(L'"'     ); break;
            case 0x23 : Character_Fill(L'#'     ); break;
            case 0x24 : Character_Fill(L'$'     ); break;
            case 0x25 : Character_Fill(L'%'     ); break;
            case 0x26 : Character_Fill(L'&'     ); break;
            case 0x27 : Character_Fill(L'\''    ); break;
            case 0x28 : Character_Fill(L'('     ); break;
            case 0x29 : Character_Fill(L')'     ); break;
            case 0x2A : Character_Fill(L'*'     ); break;
            case 0x2B : Character_Fill(L'+'     ); break;
            case 0x2C : Character_Fill(L','     ); break;
            case 0x2D : Character_Fill(L'-'     ); break;
            case 0x2E : Character_Fill(L'.'     ); break;
            case 0x2F : Character_Fill(L'/'     ); break;
            case 0x30 : Character_Fill(L'0'     ); break;
            case 0x31 : Character_Fill(L'1'     ); break;
            case 0x32 : Character_Fill(L'2'     ); break;
            case 0x33 : Character_Fill(L'3'     ); break;
            case 0x34 : Character_Fill(L'4'     ); break;
            case 0x35 : Character_Fill(L'5'     ); break;
            case 0x36 : Character_Fill(L'6'     ); break;
            case 0x37 : Character_Fill(L'7'     ); break;
            case 0x38 : Character_Fill(L'8'     ); break;
            case 0x39 : Character_Fill(L'9'     ); break;
            case 0x3A : Character_Fill(L':'     ); break;
            case 0x3B : Character_Fill(L';'     ); break;
            case 0x3C : Character_Fill(L'<'     ); break;
            case 0x3E : Character_Fill(L'>'     ); break;
            case 0x3F : Character_Fill(L'?'     ); break;
            case 0x40 : Character_Fill(L'@'     ); break;
            case 0x41 : Character_Fill(L'A'     ); break;
            case 0x42 : Character_Fill(L'B'     ); break;
            case 0x43 : Character_Fill(L'C'     ); break;
            case 0x44 : Character_Fill(L'D'     ); break;
            case 0x45 : Character_Fill(L'E'     ); break;
            case 0x46 : Character_Fill(L'F'     ); break;
            case 0x47 : Character_Fill(L'G'     ); break;
            case 0x48 : Character_Fill(L'H'     ); break;
            case 0x49 : Character_Fill(L'I'     ); break;
            case 0x4A : Character_Fill(L'J'     ); break;
            case 0x4B : Character_Fill(L'K'     ); break;
            case 0x4C : Character_Fill(L'L'     ); break;
            case 0x4D : Character_Fill(L'M'     ); break;
            case 0x4E : Character_Fill(L'N'     ); break;
            case 0x4F : Character_Fill(L'O'     ); break;
            case 0x50 : Character_Fill(L'P'     ); break;
            case 0x51 : Character_Fill(L'Q'     ); break;
            case 0x52 : Character_Fill(L'R'     ); break;
            case 0x53 : Character_Fill(L'S'     ); break;
            case 0x54 : Character_Fill(L'T'     ); break;
            case 0x55 : Character_Fill(L'U'     ); break;
            case 0x56 : Character_Fill(L'V'     ); break;
            case 0x57 : Character_Fill(L'W'     ); break;
            case 0x58 : Character_Fill(L'X'     ); break;
            case 0x59 : Character_Fill(L'Y'     ); break;
            case 0x5A : Character_Fill(L'Z'     ); break;
            case 0x5B : Character_Fill(L'['     ); break;
            case 0x5C : Character_Fill(L'\\'    ); break;
            case 0x5D : Character_Fill(L']'     ); break;
            case 0x5E : Character_Fill(L'^'     ); break;
            case 0x5F : Character_Fill(L'_'     ); break;
            case 0x60 : Character_Fill(L'`'     ); break;
            case 0x61 : Character_Fill(L'a'     ); break;
            case 0x62 : Character_Fill(L'b'     ); break;
            case 0x63 : Character_Fill(L'c'     ); break;
            case 0x64 : Character_Fill(L'd'     ); break;
            case 0x65 : Character_Fill(L'e'     ); break;
            case 0x66 : Character_Fill(L'f'     ); break;
            case 0x67 : Character_Fill(L'g'     ); break;
            case 0x68 : Character_Fill(L'h'     ); break;
            case 0x69 : Character_Fill(L'i'     ); break;
            case 0x6A : Character_Fill(L'j'     ); break;
            case 0x6B : Character_Fill(L'k'     ); break;
            case 0x6C : Character_Fill(L'l'     ); break;
            case 0x6D : Character_Fill(L'm'     ); break;
            case 0x6E : Character_Fill(L'n'     ); break;
            case 0x6F : Character_Fill(L'o'     ); break;
            case 0x70 : Character_Fill(L'p'     ); break;
            case 0x71 : Character_Fill(L'q'     ); break;
            case 0x72 : Character_Fill(L'r'     ); break;
            case 0x73 : Character_Fill(L's'     ); break;
            case 0x74 : Character_Fill(L't'     ); break;
            case 0x75 : Character_Fill(L'u'     ); break;
            case 0x76 : Character_Fill(L'v'     ); break;
            case 0x77 : Character_Fill(L'w'     ); break;
            case 0x78 : Character_Fill(L'x'     ); break;
            case 0x79 : Character_Fill(L'y'     ); break;
            case 0x7A : Character_Fill(L'z'     ); break;
            case 0x7B : Character_Fill(L'{'     ); break;
            case 0x7C : Character_Fill(L'|'     ); break;
            case 0x7D : Character_Fill(L'}'     ); break;
            case 0x7E : Character_Fill(L'~'     ); break;
            case 0x7F : Character_Fill(L'\x266A'); break;
            //CEA-708-D, Section 7.1.5 (C1)
            case 0x80 : //CW0
            case 0x81 : //CW1
            case 0x82 : //CW2
            case 0x83 : //CW3
            case 0x84 : //CW4
            case 0x85 : //CW5
            case 0x86 : //CW6
            case 0x87 : //CW7
                        CWx(cc_data_1-0x80); break; //SetCurrentWindow
            case 0x88 : CLW(); Pos+=1; break; //ClearWindows
            case 0x89 : DSW(); Pos+=1; break; //DisplayWindows
            case 0x8A : HDW(); Pos+=1; break; //HideWindows
            case 0x8B : TGW(); Pos+=1; break; //ToggleWindows
            case 0x8C : DLW(); Pos+=1; break; //DeleteWindows
            case 0x8D : DLY(); Pos+=1; break; //Delay
            case 0x8E : DLC(); break; //Delay Cancel
            case 0x8F : RST(); break; //Reset
            case 0x90 : SPA(); Pos+=2; break; //SetPenAttributes
            case 0x91 : SPC(); Pos+=3; break; //SetPenColor
            case 0x92 : SPL(); Pos+=2; break; //SetPenLocation
            case 0x97 : SWA(); Pos+=5; break; //SetWindowAttributes
            case 0x98 : //DF0
            case 0x99 : //DF1
            case 0x9A : //DF2
            case 0x9B : //DF3
            case 0x9C : //DF4
            case 0x9D : //DF5
            case 0x9E : //DF6
            case 0x9F : //DF7
                        DFx(cc_data_1-0x98); Pos+=6; break; //DefineWindow
            //CEA-708-D, Section 7.1.6 (G1)
            case 0xA0 : Character_Fill(L'\xA0'  ); break;
            case 0xA1 : Character_Fill(L'\xA1'  ); break;
            case 0xA2 : Character_Fill(L'\xA2'  ); break;
            case 0xA3 : Character_Fill(L'\xA3'  ); break;
            case 0xA4 : Character_Fill(L'\xA4'  ); break;
            case 0xA5 : Character_Fill(L'\xA5'  ); break;
            case 0xA6 : Character_Fill(L'\xA6'  ); break;
            case 0xA7 : Character_Fill(L'\xA7'  ); break;
            case 0xA8 : Character_Fill(L'\xA8'  ); break;
            case 0xA9 : Character_Fill(L'\xA9'  ); break;
            case 0xAA : Character_Fill(L'\xAA'  ); break;
            case 0xAB : Character_Fill(L'\xAB'  ); break;
            case 0xAC : Character_Fill(L'\xAC'  ); break;
            case 0xAD : Character_Fill(L'\xAD'  ); break;
            case 0xAE : Character_Fill(L'\xAE'  ); break;
            case 0xAF : Character_Fill(L'\xAF'  ); break;
            case 0xB0 : Character_Fill(L'\xB0'  ); break;
            case 0xB1 : Character_Fill(L'\xB1'  ); break;
            case 0xB2 : Character_Fill(L'\xB2'  ); break;
            case 0xB3 : Character_Fill(L'\xB3'  ); break;
            case 0xB4 : Character_Fill(L'\xB4'  ); break;
            case 0xB5 : Character_Fill(L'\xB5'  ); break;
            case 0xB6 : Character_Fill(L'\xB6'  ); break;
            case 0xB7 : Character_Fill(L'\xB7'  ); break;
            case 0xB8 : Character_Fill(L'\xB8'  ); break;
            case 0xB9 : Character_Fill(L'\xB9'  ); break;
            case 0xBA : Character_Fill(L'\xBA'  ); break;
            case 0xBB : Character_Fill(L'\xBB'  ); break;
            case 0xBC : Character_Fill(L'\xBC'  ); break;
            case 0xBD : Character_Fill(L'\xBD'  ); break;
            case 0xBE : Character_Fill(L'\xBE'  ); break;
            case 0xBF : Character_Fill(L'\xBF'  ); break;
            case 0xC0 : Character_Fill(L'\xC0'  ); break;
            case 0xC1 : Character_Fill(L'\xC1'  ); break;
            case 0xC2 : Character_Fill(L'\xC2'  ); break;
            case 0xC3 : Character_Fill(L'\xC3'  ); break;
            case 0xC4 : Character_Fill(L'\xC4'  ); break;
            case 0xC5 : Character_Fill(L'\xC5'  ); break;
            case 0xC6 : Character_Fill(L'\xC6'  ); break;
            case 0xC7 : Character_Fill(L'\xC7'  ); break;
            case 0xC8 : Character_Fill(L'\xC8'  ); break;
            case 0xC9 : Character_Fill(L'\xC9'  ); break;
            case 0xCA : Character_Fill(L'\xCA'  ); break;
            case 0xCB : Character_Fill(L'\xCB'  ); break;
            case 0xCC : Character_Fill(L'\xCC'  ); break;
            case 0xCD : Character_Fill(L'\xCD'  ); break;
            case 0xCE : Character_Fill(L'\xCE'  ); break;
            case 0xCF : Character_Fill(L'\xCF'  ); break;
            case 0xD0 : Character_Fill(L'\xD0'  ); break;
            case 0xD1 : Character_Fill(L'\xD1'  ); break;
            case 0xD2 : Character_Fill(L'\xD2'  ); break;
            case 0xD3 : Character_Fill(L'\xD3'  ); break;
            case 0xD4 : Character_Fill(L'\xD4'  ); break;
            case 0xD5 : Character_Fill(L'\xD5'  ); break;
            case 0xD6 : Character_Fill(L'\xD6'  ); break;
            case 0xD7 : Character_Fill(L'\xD7'  ); break;
            case 0xD8 : Character_Fill(L'\xD8'  ); break;
            case 0xD9 : Character_Fill(L'\xD9'  ); break;
            case 0xDA : Character_Fill(L'\xDA'  ); break;
            case 0xDB : Character_Fill(L'\xDB'  ); break;
            case 0xDC : Character_Fill(L'\xDC'  ); break;
            case 0xDD : Character_Fill(L'\xDD'  ); break;
            case 0xDE : Character_Fill(L'\xDE'  ); break;
            case 0xDF : Character_Fill(L'\xDF'  ); break;
            case 0xE0 : Character_Fill(L'\xE0'  ); break;
            case 0xE1 : Character_Fill(L'\xE1'  ); break;
            case 0xE2 : Character_Fill(L'\xE2'  ); break;
            case 0xE3 : Character_Fill(L'\xE3'  ); break;
            case 0xE4 : Character_Fill(L'\xE4'  ); break;
            case 0xE5 : Character_Fill(L'\xE5'  ); break;
            case 0xE6 : Character_Fill(L'\xE6'  ); break;
            case 0xE7 : Character_Fill(L'\xE7'  ); break;
            case 0xE8 : Character_Fill(L'\xE8'  ); break;
            case 0xE9 : Character_Fill(L'\xE9'  ); break;
            case 0xEA : Character_Fill(L'\xEA'  ); break;
            case 0xEB : Character_Fill(L'\xEB'  ); break;
            case 0xEC : Character_Fill(L'\xEC'  ); break;
            case 0xED : Character_Fill(L'\xED'  ); break;
            case 0xEE : Character_Fill(L'\xEE'  ); break;
            case 0xEF : Character_Fill(L'\xEF'  ); break;
            case 0xF0 : Character_Fill(L'\xF0'  ); break;
            case 0xF1 : Character_Fill(L'\xF1'  ); break;
            case 0xF2 : Character_Fill(L'\xF2'  ); break;
            case 0xF3 : Character_Fill(L'\xF3'  ); break;
            case 0xF4 : Character_Fill(L'\xF4'  ); break;
            case 0xF5 : Character_Fill(L'\xF5'  ); break;
            case 0xF6 : Character_Fill(L'\xF6'  ); break;
            case 0xF7 : Character_Fill(L'\xF7'  ); break;
            case 0xF8 : Character_Fill(L'\xF8'  ); break;
            case 0xF9 : Character_Fill(L'\xF9'  ); break;
            case 0xFA : Character_Fill(L'\xFA'  ); break;
            case 0xFB : Character_Fill(L'\xFB'  ); break;
            case 0xFC : Character_Fill(L'\xFC'  ); break;
            case 0xFD : Character_Fill(L'\xFD'  ); break;
            case 0xFE : Character_Fill(L'\xFE'  ); break;
            case 0xFF : Character_Fill(L'\xFF'  ); break;
            default   : Illegal(1, cc_data_1);
        }
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
//NUL
void File_Eia708::NUL()
{
}

//---------------------------------------------------------------------------
//End Of Text
void File_Eia708::ETX()
{
}

//---------------------------------------------------------------------------
//Backspace
void File_Eia708::BS()
{
    Param_Info1("Backspace");

    int8u WindowID=Streams[service_number]->WindowID;
    if (WindowID==(int8u)-1)
        return; //Must wait for the corresponding CWx
    window* Window=Streams[service_number]->Windows[WindowID];
    if (Window==NULL)
        return; //Must wait for the corresponding DFx

    if (Window->Minimal.x)
    {
        //Clearing window
        Window->Minimal.x--;
        Window->Minimal.CC[Window->Minimal.y][Window->Minimal.x]=character();

        if (Window->visible)
        {
            //Clearing global area
            if (Window->Minimal.Window_y+Window->Minimal.y<(int8u)Streams[service_number]->Minimal.CC.size() && Window->Minimal.Window_x+Window->Minimal.x<(int8u)Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Window->Minimal.y].size())
                Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Window->Minimal.y][Window->Minimal.Window_x+Window->Minimal.x]=character();

            //Has changed
            Window_HasChanged();
            HasChanged();
        }
    }
}

//---------------------------------------------------------------------------
//Form Feed
void File_Eia708::FF()
{
    Param_Info1("Form Feed");

    int8u WindowID=Streams[service_number]->WindowID;
    if (WindowID==(int8u)-1)
        return; //Must wait for the corresponding CWx
    window* Window=Streams[service_number]->Windows[WindowID];
    if (Window==NULL)
        return; //Must wait for the corresponding DFx

    for (size_t Pos_Y=0; Pos_Y<Window->row_count; Pos_Y++)
        for (size_t Pos_X=0; Pos_X<Window->column_count; Pos_X++)
        {
            //Clearing window
            Streams[service_number]->Windows[Streams[service_number]->WindowID]->Minimal.CC[Pos_Y][Pos_X]=character();

            if (Window->visible)
            {
                //Clearing global area
                if (Window->Minimal.Window_y+Pos_Y<Streams[service_number]->Minimal.CC.size() && Window->Minimal.Window_x+Pos_X<Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Pos_Y].size())
                    Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Pos_Y][Window->Minimal.Window_x+Pos_X]=character();
            }
        }

    if (Window->visible)
    {
        //Has changed
        Window_HasChanged();
        HasChanged();
    }

    //SetPenLocation
    Window->Minimal.x=0;
    Window->Minimal.y=0;
}

//---------------------------------------------------------------------------
//Carriage return
void File_Eia708::CR()
{
    Param_Info1("Carriage return");

    int8u WindowID=Streams[service_number]->WindowID;
    if (WindowID==(int8u)-1)
        return; //Must wait for the corresponding CWx
    window* Window=Streams[service_number]->Windows[WindowID];
    if (Window==NULL)
        return; //Must wait for the corresponding DFx
    int8u y=Window->Minimal.y;

    y++;
    if (y>=Window->row_count)
    {
        //Rolling up window
        for (int8u Pos_Y=0; Pos_Y<Window->row_count-1; Pos_Y++)
            Window->Minimal.CC[Pos_Y]=Window->Minimal.CC[Pos_Y+1];

        //SetPenLocation
        y=Window->row_count-1;

        //Clearing last line of window
        for (int8u Pos_X=0; Pos_X<Window->column_count; Pos_X++)
            Window->Minimal.CC[y][Pos_X]=character();

        if (Window->visible)
        {
            //Updating global area
            for (int8u Pos_Y=0; Pos_Y<Window->row_count; Pos_Y++)
                for (int8u Pos_X=0; Pos_X<Window->column_count; Pos_X++)
                    if (Window->Minimal.Window_y+Pos_Y<(int8u)Streams[service_number]->Minimal.CC.size() && Window->Minimal.Window_x+Pos_X<(int8u)Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Pos_Y].size())
                        Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Pos_Y][Window->Minimal.Window_x+Pos_X]=Window->Minimal.CC[Pos_Y][Pos_X];

            //Has changed
            Window_HasChanged();
            HasChanged();
        }
    }

    //SetPenLocation
    Window->Minimal.x=0;
    Window->Minimal.y=y;
}

//---------------------------------------------------------------------------
//Horizontal Carriage Return
void File_Eia708::HCR()
{
    Param_Info1("Horizontal Carriage Return");

    int8u WindowID=Streams[service_number]->WindowID;
    if (WindowID==(int8u)-1)
        return; //Must wait for the corresponding CWx
    window* Window=Streams[service_number]->Windows[WindowID];
    if (Window==NULL)
        return; //Must wait for the corresponding DFx
    int8u y=Window->Minimal.y;

    for (int8u Pos_X=0; Pos_X<Window->column_count; Pos_X++)
    {
        //Clearing window
        Streams[service_number]->Windows[Streams[service_number]->WindowID]->Minimal.CC[y][Pos_X]=character();

        if (Window->visible)
        {
            //Clearing global area
            if (Window->Minimal.Window_y+y<(int8u)Streams[service_number]->Minimal.CC.size() && Window->Minimal.Window_x+Pos_X<(int8u)Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Window->Minimal.y].size())
                Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+y][Window->Minimal.Window_x+Pos_X]=character();

            //Has changed
            Window_HasChanged();
            HasChanged();
        }
    }

    //SetPenLocation
    Window->Minimal.x=0;
}

//---------------------------------------------------------------------------
//SetCurrentWindow
void File_Eia708::CWx(int8u WindowID)
{
    Param_Info1("SetCurrentWindow"); Param_Info1(WindowID);

    Streams[service_number]->WindowID=WindowID;
}

//---------------------------------------------------------------------------
//ClearWindows
void File_Eia708::CLW()
{
    Param_Info1("ClearWindows");

    int8u Save_WindowID=Streams[service_number]->WindowID;
    bool  Save_StandAloneCommand=StandAloneCommand;
    StandAloneCommand=false;

    Element_Begin1("ClearWindows");
    BS_Begin();
    int8u WindowID=8;
    bool HasChanged_=false;
    do
    {
        WindowID--;
        bool IsSet;
        Get_SB (   IsSet,                                       Ztring(__T("window ")+Ztring::ToZtring(WindowID)).To_Local().c_str());

        if (IsSet)
        {
            Streams[service_number]->WindowID=WindowID;
            window* Window=Streams[service_number]->Windows[WindowID];

            //ClearWindow is like Form Feed
            FF();

            if (Window && Window->visible)
            {
                //Has changed
                HasChanged_=true;
                Window_HasChanged();
            }
        }
    }
    while (WindowID>0);
    BS_End();
    Element_End0();

    Streams[service_number]->WindowID=Save_WindowID;
    StandAloneCommand=Save_StandAloneCommand;

    if (HasChanged_)
    {
        //Has changed
        HasChanged();
    }
}

//---------------------------------------------------------------------------
//DisplayWindows
void File_Eia708::DSW()
{
    Param_Info1("DisplayWindows");

    int8u Save_WindowID=Streams[service_number]->WindowID;
    bool  Save_StandAloneCommand=StandAloneCommand;
    StandAloneCommand=false;

    Element_Begin1("DisplayWindows");
    BS_Begin();
    int8u WindowID=8;
    bool HasChanged_=false;
    do
    {
        WindowID--;
        bool IsSet;
        Get_SB (   IsSet,                                       Ztring(__T("window ")+Ztring::ToZtring(WindowID)).To_Local().c_str());

        if (IsSet)
        {
            window* Window=Streams[service_number]->Windows[WindowID];

            if (Window && !Window->visible)
            {
                Window->visible=true;

                //Filling global area
                for (size_t Pos_Y=0; Pos_Y<Window->row_count; Pos_Y++)
                    for (size_t Pos_X=0; Pos_X<Window->column_count; Pos_X++)
                    {
                        if (Window->Minimal.Window_y+Pos_Y<Streams[service_number]->Minimal.CC.size() && Window->Minimal.Window_x+Pos_X<Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Pos_Y].size())
                            Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Pos_Y][Window->Minimal.Window_x+Pos_X]=Window->Minimal.CC[Pos_Y][Pos_X];
                    }

                //Has changed
                HasChanged_=true;
                Window_HasChanged();
            }
        }
    }
    while (WindowID>0);
    BS_End();
    Element_End0();

    Streams[service_number]->WindowID=Save_WindowID;
    StandAloneCommand=Save_StandAloneCommand;

    if (HasChanged_)
    {
        //Has changed
        HasChanged();
    }
}

//---------------------------------------------------------------------------
//HideWindows
void File_Eia708::HDW()
{
    Param_Info1("HideWindows");

    int8u Save_WindowID=Streams[service_number]->WindowID;
    bool  Save_StandAloneCommand=StandAloneCommand;
    StandAloneCommand=false;

    Element_Begin1("HideWindows");
    BS_Begin();
    int8u WindowID=8;
    bool HasChanged_=false;
    do
    {
        WindowID--;

        bool IsSet;
        Get_SB (   IsSet,                                       Ztring(__T("window ")+Ztring::ToZtring(WindowID)).To_Local().c_str());

        if (IsSet)
        {
            window* Window=Streams[service_number]->Windows[WindowID];

            if (Window && Window->visible)
            {
                Window->visible=false;

                for (size_t Pos_Y=0; Pos_Y<Window->row_count; Pos_Y++)
                    for (size_t Pos_X=0; Pos_X<Window->column_count; Pos_X++)
                    {
                        //Clearing window
                        Window->Minimal.CC[Pos_Y][Pos_X]=character();

                        //Filling global area
                        if (Window->Minimal.Window_y+Pos_Y<Streams[service_number]->Minimal.CC.size() && Window->Minimal.Window_x+Pos_X<Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Pos_Y].size())
                            Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Pos_Y][Window->Minimal.Window_x+Pos_X]=character();
                    }

                //Has changed
                HasChanged_=true;
                Window_HasChanged();
            }
        }
    }
    while (WindowID>0);
    BS_End();
    Element_End0();

    Streams[service_number]->WindowID=Save_WindowID;
    StandAloneCommand=Save_StandAloneCommand;

    if (HasChanged_)
    {
        //Has changed
        HasChanged();
    }
}

//---------------------------------------------------------------------------
//ToggleWindows
void File_Eia708::TGW()
{
    Param_Info1("ToggleWindows");

    int8u Save_WindowID=Streams[service_number]->WindowID;
    bool  Save_StandAloneCommand=StandAloneCommand;
    StandAloneCommand=false;

    Element_Begin1("ToggleWindows");
    BS_Begin();
    int8u WindowID=8;
    bool HasChanged_=false;
    do
    {
        WindowID--;
        bool IsSet;
        Get_SB (   IsSet,                                       Ztring(__T("window ")+Ztring::ToZtring(WindowID)).To_Local().c_str());

        if (IsSet)
        {
            window* Window=Streams[service_number]->Windows[WindowID];

            if (Window)
            {
                Window->visible=!Window->visible;

                //Filling global area
                for (size_t Pos_Y=0; Pos_Y<Window->row_count; Pos_Y++)
                    for (size_t Pos_X=0; Pos_X<Window->column_count; Pos_X++)
                    {
                        if (Window->Minimal.Window_y+Pos_Y<Streams[service_number]->Minimal.CC.size() && Window->Minimal.Window_x+Pos_X<Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Pos_Y].size())
                            Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Pos_Y][Window->Minimal.Window_x+Pos_X]=Window->visible?Window->Minimal.CC[Pos_Y][Pos_X]:character();
                    }

                //Has changed
                HasChanged_=true;
                Window_HasChanged();
            }
        }
    }
    while (WindowID>0);
    BS_End();
    Element_End0();

    Streams[service_number]->WindowID=Save_WindowID;
    StandAloneCommand=Save_StandAloneCommand;

    if (HasChanged_)
    {
        //Has changed
        HasChanged();
    }
}

//---------------------------------------------------------------------------
//DeleteWindows
void File_Eia708::DLW()
{
    Param_Info1("DeleteWindows");

    int8u Save_WindowID=Streams[service_number]->WindowID;
    bool  Save_StandAloneCommand=StandAloneCommand;
    StandAloneCommand=false;

    //Bug in some files
    bool Bug_WindowOffset=false;

    Element_Begin1("DeleteWindows");
    BS_Begin();
    int8u WindowID=8;
    bool HasChanged_=false;
    do
    {
        WindowID--;
        bool IsSet;
        Get_SB (   IsSet,                                       Ztring(__T("window ")+Ztring::ToZtring(WindowID)).To_Local().c_str());

        //Bug in some files
        if (IsSet && WindowID==1 && Streams[service_number]->Windows[0]!=NULL && Streams[service_number]->Windows[1]==NULL) //Mix between Windows 0 and 1
            Bug_WindowOffset=true;
        if (!IsSet && WindowID==0 && Bug_WindowOffset)
            IsSet=true;

        if (IsSet)
        {
            window* Window=Streams[service_number]->Windows[WindowID];

            if (Window && Window->visible)
            {
                //Filling global area
                for (size_t Pos_Y=0; Pos_Y<Window->row_count; Pos_Y++)
                    for (size_t Pos_X=0; Pos_X<Window->column_count; Pos_X++)
                    {
                        //Clearing window
                        Window->Minimal.CC[Pos_Y][Pos_X]=character();

                        //Filling global area
                        if (Window->Minimal.Window_y+Pos_Y<Streams[service_number]->Minimal.CC.size() && Window->Minimal.Window_x+Pos_X<Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Pos_Y].size())
                            Streams[service_number]->Minimal.CC[Window->Minimal.Window_y+Pos_Y][Window->Minimal.Window_x+Pos_X]=character();
                    }

                //Has changed
                HasChanged_=true;
                Window_HasChanged();
            }

            //Removing the window
            delete Streams[service_number]->Windows[WindowID]; Streams[service_number]->Windows[WindowID]=NULL;
            if (WindowID==Save_WindowID)
                Save_WindowID=(int8u)-1;
        }
    }
    while (WindowID>0);
    BS_End();
    Element_End0();

    Streams[service_number]->WindowID=Save_WindowID;
    StandAloneCommand=Save_StandAloneCommand;

    if (HasChanged_)
    {
        //Has changed
        HasChanged();
    }
}

//---------------------------------------------------------------------------
//Delay
void File_Eia708::DLY()
{
    Param_Info1("Delay");
    Element_Begin1("Delay");
    Skip_B1(                        "tenths of seconds");
    Element_End0();
}

//---------------------------------------------------------------------------
//Delay Cancel
void File_Eia708::DLC()
{
    Param_Info1("Delay Cancel");
}

//---------------------------------------------------------------------------
//Reset
void File_Eia708::RST()
{
    //TODO: Should clear all buffers
    Param_Info1("Reset");
}

//---------------------------------------------------------------------------
//Set Pen Attributes
void File_Eia708::SPA()
{
    Param_Info1("Set Pen Attributes");
    Element_Begin1("Set Pen Attributes");
    BS_Begin();
    Skip_S1(4,                                                  "text tag");
    Skip_S1(2,                                                  "offset");
    Skip_S1(2,                                                  "pen size");
    Skip_SB(                                                    "italics");
    Skip_SB(                                                    "underline");
    Skip_S1(3,                                                  "edge type");
    Skip_S1(3,                                                  "font style");
    BS_End();
    Element_End0();
}

//---------------------------------------------------------------------------
//Set Pen Color
void File_Eia708::SPC()
{
    Param_Info1("Set Pen Color");
    Element_Begin1("Set Pen Color");
    BS_Begin();
    Skip_S1(2,                                                  "foreground opacity");
    Skip_S1(2,                                                  "foreground red");
    Skip_S1(2,                                                  "foreground green");
    Skip_S1(2,                                                  "foreground blue");
    Skip_S1(2,                                                  "background opacity");
    Skip_S1(2,                                                  "background red");
    Skip_S1(2,                                                  "background green");
    Skip_S1(2,                                                  "background blue");
    Mark_0();
    Mark_0();
    Skip_S1(2,                                                  "edge red");
    Skip_S1(2,                                                  "edge green");
    Skip_S1(2,                                                  "edge blue");
    BS_End();
    Element_End0();
}

//---------------------------------------------------------------------------
//SetPenLocation
void File_Eia708::SPL()
{
    Param_Info1("SetPenLocation");
    Element_Begin1("SetPenLocation");
    int8u row, column;
    BS_Begin();
    Mark_0();
    Mark_0();
    Mark_0();
    Mark_0();
    Get_S1 (4, row,                                             "row");
    Mark_0();
    Mark_0();
    Get_S1 (6, column,                                          "column");
    BS_End();
    Element_End0();

    int8u WindowID=Streams[service_number]->WindowID;
    if (WindowID==(int8u)-1)
        return; //Must wait for the corresponding CWx
    window* Window=Streams[service_number]->Windows[WindowID];
    if (Window==NULL)
        return; //Must wait for the corresponding DFx

    if (row<Window->Minimal.CC.size() && column<Window->Minimal.CC[Window->Minimal.y].size())
    {
        Window->Minimal.x=column;
        Window->Minimal.y=row;
    }
    else
    {
        // There is a problem, resetting pen location
        Window->Minimal.x=0;
        Window->Minimal.y=0;
    }
}

//---------------------------------------------------------------------------
//SetWindowAttributes
void File_Eia708::SWA()
{
    Param_Info1("SetWindowAttributes");
    Element_Begin1("SetWindowAttributes");
    BS_Begin();
    Skip_S1(2,                                                  "fill opacity");
    Skip_S1(2,                                                  "fill red");
    Skip_S1(2,                                                  "fill green");
    Skip_S1(2,                                                  "fill blue");
    Skip_S1(2,                                                  "border type (low)");
    Skip_S1(2,                                                  "border red");
    Skip_S1(2,                                                  "border green");
    Skip_S1(2,                                                  "border blue");
    Skip_SB(                                                    "border type (high)");
    Skip_SB(                                                    "wordwrap");
    Skip_S1(2,                                                  "print direction");
    Skip_S1(2,                                                  "scroll direction");
    Skip_S1(2,                                                  "justify");
    Skip_S1(4,                                                  "effect speed");
    Skip_S1(2,                                                  "effect direction");
    Skip_S1(2,                                                  "display effect");
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Skip_S1(2,                                                  "edge red");
    Skip_S1(2,                                                  "edge green");
    Skip_S1(2,                                                  "edge blue");
    BS_End();
    Element_End0();
}

//---------------------------------------------------------------------------
//DefineWindow
void File_Eia708::DFx(int8u WindowID)
{
    Param_Info1("DefineWindow"); Param_Info1(WindowID);
    Element_Begin1("DefineWindow");
    int8u anchor_vertical, anchor_horizontal, anchor_point, row_count, column_count;
    bool visible, relative_positioning;
    BS_Begin();
    Mark_0();
    Mark_0();
    Get_SB (   visible,                                         "visible");
    Skip_SB(                                                    "row lock");
    Skip_SB(                                                    "column lock");
    Skip_S1(3,                                                  "priority");
    Get_SB (   relative_positioning,                            "relative positioning");
    Get_S1 (7, anchor_vertical,                                 "anchor vertical"); //Top left
    Get_S1 (8, anchor_horizontal,                               "anchor horizontal"); //Top left
    Get_S1 (4, anchor_point,                                    "anchor point");
    Get_S1 (4, row_count,                                       "row count"); //Maximum=14
    Mark_0();
    Mark_0();
    Get_S1 (6, column_count,                                    "column count"); //Maximum=31
    Mark_0();
    Mark_0();
    Skip_S1(4,                                                  "window style");
    Skip_S1(2,                                                  "pen style ID");
    BS_End();
    Element_End0();

    Streams[service_number]->WindowID=WindowID;
    if (Streams[service_number]->Windows[WindowID]==NULL)
        Streams[service_number]->Windows[WindowID]=new window;
    window* Window=Streams[service_number]->Windows[WindowID];
    Window->visible=visible;
    Window->relative_positioning=relative_positioning;
    Window->anchor_vertical=anchor_vertical;
    if (relative_positioning)
        Window->Minimal.Window_y=(int8u)(((float)15)*anchor_vertical/100);
    else
        Window->Minimal.Window_y=anchor_vertical/5;
    Window->anchor_horizontal=anchor_horizontal;
    int8u offset_y;
    switch (anchor_point)
    {
        case 0 :
        case 1 :
        case 2 :
                offset_y=0;
                break;
        case 3 :
        case 4 :
        case 5 :
                offset_y=(row_count+1)/2;
                break;
        case 6 :
        case 7 :
        case 8 :
                offset_y=(row_count+1);
                break;
        default: offset_y=0; //Not valid
    }
    if (offset_y<Window->Minimal.Window_y)
        Window->Minimal.Window_y-=offset_y;
    if (relative_positioning)
        Window->Minimal.Window_x=(int8u)(24*AspectRatio*anchor_horizontal/100);
    else
        Window->Minimal.Window_x=anchor_horizontal/5;
    int8u offset_x;
    switch (anchor_point)
    {
        case 0 :
        case 3 :
        case 6 :
                offset_x=0;
                break;
        case 1 :
        case 4 :
        case 7 :
                offset_x=(column_count+1)/2;
                break;
        case 2 :
        case 5 :
        case 8 :
                offset_x=(column_count+1);
                break;
        default: offset_x=0; //Not valid
    }
    if (offset_x<Window->Minimal.Window_x)
        Window->Minimal.Window_x-=offset_x;
    Window->row_count=row_count+1;
    Window->column_count=column_count+1;
    Window->Minimal.x=0;
    Window->Minimal.y=0;

    if (Window->row_count>15)
    {
        Window->row_count=15; //Limitation of specifications
    }
    if (AspectRatio && Window->column_count>(int8u)(24*AspectRatio))
    {
        Window->column_count=(int8u)(24*AspectRatio); //Limitation of specifications
    }
    Window->Minimal.CC.resize(Window->row_count);
    for (int8u Pos_Y=0; Pos_Y<Window->row_count; Pos_Y++)
        Window->Minimal.CC[Pos_Y].resize(Window->column_count);
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Eia708::Character_Fill(wchar_t Character)
{
    Param_Info1(Ztring().From_Unicode(&Character, 1)); //(Character) after new ZenLib release

    int8u WindowID=Streams[service_number]->WindowID;
    if (WindowID==(int8u)-1)
        return; //Must wait for the corresponding CWx
    window* Window=Streams[service_number]->Windows[WindowID];
    if (Window==NULL)
        return; //Must wait for the corresponding DFx

    int8u x=Window->Minimal.x;
    int8u y=Window->Minimal.y;
    int8u Window_x=Window->Minimal.Window_x;
    int8u Window_y=Window->Minimal.Window_y;

    if (x<Window->column_count && y<Window->row_count)
    {
        //Filling window
        Window->Minimal.CC[y][x].Value=Character;

        if (Window->visible)
        {
            //Filling global area
            if (Window_y+y<(int8u)Streams[service_number]->Minimal.CC.size() && Window_x+x<(int8u)Streams[service_number]->Minimal.CC[Window_y+y].size())
                Streams[service_number]->Minimal.CC[Window_y+y][Window_x+x].Value=Character;

            //Has changed
            Window_HasChanged();
            HasChanged();
        }

        x++;
        Window->Minimal.x=x;
    }

    if (!HasContent)
        HasContent=true;
    DataDetected|=((int64u)1)<<service_number; //1 bit per service
}

//---------------------------------------------------------------------------
void File_Eia708::Window_HasChanged()
{
}

//---------------------------------------------------------------------------
void File_Eia708::HasChanged()
{
}

//---------------------------------------------------------------------------
void File_Eia708::Illegal(int8u Size, int8u cc_data_1, int8u cc_data_2, int8u cc_data_3, int8u cc_data_4, int8u cc_data_5, int8u cc_data_6)
{
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_EIA708_YES
