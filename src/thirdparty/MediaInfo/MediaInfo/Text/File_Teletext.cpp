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
#if defined(MEDIAINFO_TELETEXT_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_Teletext.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events_Internal.h"
#endif //MEDIAINFO_EVENTS
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Teletext::File_Teletext()
:File__Analyze()
{
    //Configuration
    ParserName=__T("Teletext");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Teletext;
        StreamIDs_Width[0]=2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    PTS_DTS_Needed=true;
    IsRawStream=true;
    MustSynchronize=true;

    //In
    #if defined(MEDIAINFO_MPEGPS_YES)
        FromMpegPs=false;
        Parser=NULL;
    #endif
    IsSubtitle=false;
}

//---------------------------------------------------------------------------
File_Teletext::~File_Teletext()
{
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Teletext::Streams_Fill()
{
}

//---------------------------------------------------------------------------
void File_Teletext::Streams_Finish()
{
    for (streams::iterator Stream=Streams.begin(); Stream!=Streams.end(); ++Stream)
    {
        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, StreamPos_Last, Text_Format, IsSubtitle?"Teletext Subtitle":"Teletext");
        Fill(Stream_Text, StreamPos_Last, Text_ID, Ztring::ToZtring(Stream->first, 16));
    }
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Teletext::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+3<=Buffer_Size)
    {
        while (Buffer_Offset+3<=Buffer_Size)
        {
            if (Buffer[Buffer_Offset  ]==0x55
             && Buffer[Buffer_Offset+1]==0x55
             && Buffer[Buffer_Offset+2]==0x27)
                break; //while()

            Buffer_Offset++;
        }

        if (Buffer_Offset+3<=Buffer_Size) //Testing if size is coherant
        {
            if (Buffer_Offset+45==Buffer_Size)
                break;

            if (Buffer_Offset+45+3>Buffer_Size)
                return false; //Wait for more data

            if (Buffer[Buffer_Offset  ]==0x55
             && Buffer[Buffer_Offset+1]==0x55
             && Buffer[Buffer_Offset+2]==0x27)
                break; //while()

            Buffer_Offset++;
        }
    }

    //Must have enough buffer for having header
    if (Buffer_Offset+3>=Buffer_Size)
        return false;

    //Synched is OK
    if (!Status[IsAccepted])
    {
        //For the moment, we accept only if the file is in sync, the test is not strict enough
        if (Buffer_Offset)
        {
            Reject();
            return false;
        }

        Accept();
    }
    return true;
}

//---------------------------------------------------------------------------
bool File_Teletext::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+3>Buffer_Size)
        return false;

    //Quick test of synchro
    if (Buffer[Buffer_Offset  ]!=0x55
     || Buffer[Buffer_Offset+1]!=0x55
     || Buffer[Buffer_Offset+2]!=0x27)
    {
        Synched=false;
        return true;
    }

    //We continue
    return true;
}

//---------------------------------------------------------------------------
void File_Teletext::Synched_Init()
{
    //Stream
    Stream_HasChanged=0;

    //Temp
    PageNumber=0xFF;
    SubCode=0x3F7F;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Teletext::Read_Buffer_Unsynched()
{
    for (streams::iterator Stream=Streams.begin(); Stream!=Streams.end(); ++Stream)
        {
            Stream_HasChanged=0;
            for (size_t PosY=0; PosY<26; ++PosY)
                for (size_t PosX=0; PosX<40; ++PosX)
                    if (Stream->second.CC_Displayed_Values[PosY][PosX]!=L' ')
                    {
                        Stream->second.CC_Displayed_Values[PosY][PosX]=L' ';
                        Stream_HasChanged=Stream->first;
                    }

            if (Stream_HasChanged)
            {
                HasChanged();
                Stream_HasChanged=0;
            }
        }
}

static inline int8u ReverseBits(int8u c)
{
    // Input: bit order is 76543210
    //Output: bit order is 01234567
    c = (c & 0x0F) << 4 | (c & 0xF0) >> 4;
    c = (c & 0x33) << 2 | (c & 0xCC) >> 2;
    c = (c & 0x55) << 1 | (c & 0xAA) >> 1;
    return c;
}

//---------------------------------------------------------------------------
void File_Teletext::Read_Buffer_Continue()
{
    #if defined(MEDIAINFO_MPEGPS_YES)
        if (FromMpegPs)
        {
            if (!Status[IsAccepted])
                Accept();

            Skip_B1(                                            "data_identifier");
            while (Element_Offset<Element_Size)
            {
                int8u data_unit_id, data_unit_length;
                Get_B1 (data_unit_id,                           "data_unit_id");
                Get_B1 (data_unit_length,                       "data_unit_length");
                Skip_B1(                                        "field/line");
                if (data_unit_id==0x03 && data_unit_length==0x2C)
                {
                    int8u Data[43];
                    for (int8u Pos=0; Pos<43; ++Pos)
                        Data[Pos]=ReverseBits(Buffer[Buffer_Offset+(size_t)Element_Offset+Pos]);

                    if (Parser==NULL)
                    {
                        Parser=new File_Teletext();
                        Parser->MustSynchronize=false;
                        Open_Buffer_Init(Parser);
                    }
                    Element_Code=data_unit_id;
                    int8u Temp[2];
                    Temp[0]=0x55;
                    Temp[1]=0x55;
                    Demux(Temp, 2, ContentType_MainStream);
                    Demux(Data, 43, ContentType_MainStream);
                    Open_Buffer_Continue(Parser, Data, 43);
                    Element_Offset+=43;
                }
                else
                    Skip_XX(data_unit_length-1,                 "Data");
            }
        }
    #endif
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Teletext::Header_Parse()
{
    //Parsing
    if (MustSynchronize)
        Skip_B2(                                                "Clock run-in");
    Skip_B1(                                                    "Framing code");

    //Magazine and Packet Number (for all packets)
    X=0, Y=0;
    bool P1, D1, P2, D2, P3, D3, P4, D4;
    bool B;
    BS_Begin_LE();
    Element_Begin1("Magazine (X or M)");
        Get_TB (P1,                                             "Hamming 8/4");
        Get_TB (D1,                                             "Magazine 0");
        if (D1)
            X|=1<<0;
        Get_TB (P2,                                             "Hamming 8/4");
        Get_TB (D2,                                             "Magazine 1");
        if (D2)
            X|=1<<1;
        Get_TB (P3,                                             "Hamming 8/4");
        Get_TB (D3,                                             "Magazine 2");
        if (D3)
            X|=1<<2;
    Element_Info1(X);
    Element_End0();
    Element_Begin1("Packet Number (Y)");
        Get_TB (P4,                                             "Hamming 8/4");
        Get_TB (D4,                                             "Packet Number 0");
        if (D4)
            Y|=1<<0;
        /*
        {
            //Hamming 8/4
            bool A=P1^D1^D3^D4;
            bool B=D1^P2^D2^D4;
            bool C=D1^D2^P3^D3;
            bool D=P1^D1^P2^D2^P3^D3^P4^D4;
            if (A && B && C && D)
            {
            }
            else
            {
            }
        }
        */
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "Packet Number 1");
        if (B)
            Y|=1<<1;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "Packet Number 2");
        if (B)
            Y|=1<<2;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "Packet Number 3");
        if (B)
            Y|=1<<3;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "Packet Number 4");
        if (B)
            Y|=1<<4;
        if (X==0)
            X=8; // A packet with a magazine value of 0 is referred to as belonging to magazine 8
    Element_Info1(Y);
    Element_End0();

    //Page header
    if (Y==0)
    {
        C.reset();

        Element_Begin1("Page header");
        int8u PU=0, PT=0;
        bool B;
        Element_Begin1("Page Units");
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "Page Units 0");
        if (B)
            PU|=1<<0;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "Page Units 1");
        if (B)
            PU|=1<<1;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "Page Units 2");
        if (B)
            PU|=1<<2;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "Page Units 3");
        if (B)
            PU|=1<<3;
        Element_Info1(PU);
        Element_End0();
        Element_Begin1("Page Tens");
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "Page Tens 0");
        if (B)
            PT|=1<<0;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "Page Tens 1");
        if (B)
            PT|=1<<1;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "Page Tens 2");
        if (B)
            PT|=1<<2;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "Page Tens 3");
        if (B)
            PT|=1<<3;
        Element_Info1(PT);
        Element_End0();
        PageNumber=(PT<<4)|PU;
        Element_Info1(Ztring::ToZtring(PageNumber, 16));

        int8u S1=0, S2=0, S3=0, S4=0;
        Element_Begin1("Page sub-code 1");
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S1 0");
        if (B)
            S1|=1<<0;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S1 1");
        if (B)
            S1|=1<<1;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S1 2");
        if (B)
            S1|=1<<2;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S1 3");
        if (B)
            S1|=1<<3;
        Element_Info1(S1);
        Element_End0();
        Element_Begin1("Page sub-code 2");
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S2 0");
        if (B)
            S2|=1<<0;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S2 1");
        if (B)
            S2|=1<<1;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S2 2");
        if (B)
            S2|=1<<2;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "C4 - Erase Page");
        if (B)
            C[4]=true;
        Element_Info1(S2);
        Element_End0();
        Element_Begin1("Page sub-code 3");
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S3 0");
        if (B)
            S3|=1<<0;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S3 1");
        if (B)
            S3|=1<<1;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S3 2");
        if (B)
            S3|=1<<2;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S3 3");
        if (B)
            S3|=1<<3;
        Element_Info1(S3);
        Element_End0();
        Element_Begin1("Page sub-code 4");
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S4 0");
        if (B)
            S4|=1<<0;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "S4 1");
        if (B)
            S4|=1<<1;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "C5 - Newsflash");
        if (B)
            C[5]=true;
        #if MEDIAINFO_TRACE
            if (B)
                Element_Info1("Newsflash");
        #endif //MEDIAINFO_TRACE
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "C6 - Subtitle");
        if (B)
            C[6]=true;
        Element_Info1(S4);
        Element_End0();
        Element_Begin1("Control bits");
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "C7 - Suppress Header");
        if (B)
            C[7]=true;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "C8 - Update Indicator");
        if (B)
            C[8]=true;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "C9 - Interrupted Sequence");
        if (B)
            C[9]=true;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "C10 - Inhibit Display");
        if (B)
            C[10]=true;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "C11 - Magazine Serial");
        if (B)
            C[11]=true;
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "C12 - Character Subset");
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "C13 - Character Subset");
        Skip_TB(                                                "Hamming 8/4");
        Get_TB (B,                                              "C14 - Character Subset");
        Element_End0();

        SubCode=(S4<<12)|(S3<<8)|(S2<<4)|S1;

        Element_End0();
    }
    BS_End_LE();

    #if MEDIAINFO_TRACE
        if (C[4])
            Element_Info1("Erase Page");
        if (C[5])
            Element_Info1("Newsflash");
        if (C[6])
            Element_Info1("Subtitle");
        if (C[7])
            Element_Info1("Suppress Header");
        if (C[8])
            Element_Info1("Update Indicator");
        if (C[9])
            Element_Info1("Interrupted Sequence");
        if (C[10])
            Element_Info1("Inhibit Display");
        if (C[11])
            Element_Info1("Magazine Serial");
        Element_Info1(Ztring::ToZtring((X<<8)|PageNumber, 16)+__T(':')+Ztring().From_CC2(SubCode));
        Element_Info1(Y);
    #endif // MEDIAINFO_TRACE

    Header_Fill_Size(45);

    if (Y==0)
    {
        if (Stream_HasChanged)
        {
            HasChanged();
            Stream_HasChanged=0;
        }

        if (C[4])
        {
            stream &Stream=Streams[(X<<8)|PageNumber];
            for (size_t PosY=0; PosY<26; ++PosY)
                for (size_t PosX=0; PosX<40; ++PosX)
                    if (Stream.CC_Displayed_Values[PosY][PosX]!=L' ')
                    {
                        Stream.CC_Displayed_Values[PosY][PosX]=L' ';
                        Stream_HasChanged=(X<<8)|PageNumber;
                    }
        }
    }
}

//---------------------------------------------------------------------------
void File_Teletext::Data_Parse()
{
    if (PageNumber==0xFF)
    {
        Skip_XX(Y?40:32,                                            "Junk");
    }
    else if (Y>=26)
    {
        Skip_XX(40,                                                 "Special commands");
    }
    else
    {
        Element_Begin1("Data bytes");
        stream &Stream=Streams[(X<<8)|PageNumber];
        size_t PosX=Y?0:8;
        for (; PosX<40; ++PosX)
        {
            int8u byte;
            Get_B1(byte,                                            "Byte");
            byte&=0x7F;
            if (byte<0x20)
                byte=0x20;
            Param_Info1(Ztring().From_Local((const char*)&byte, 1));
            if (byte!=Stream.CC_Displayed_Values[Y][PosX] && (!C[7] || Y)) // C[7] is "Suppress Header", to be tested when Y==0
            {
                Stream.CC_Displayed_Values[Y][PosX]=byte;
                Stream_HasChanged=(X<<8)|PageNumber;
            }
        }
        Element_End0();
    }

    #if MEDIAINFO_TRACE
        if (PageNumber==0xFF)
        {
            Element_Name("Skip");
        }
        else
        {
            Element_Name(Ztring::ToZtring((X<<8)|PageNumber, 16)+__T(':')+Ztring().From_CC2(SubCode));
            Element_Info1(Y);
            if (Y<26)
            {
                Element_Info1(Streams[(X<<8)|PageNumber].CC_Displayed_Values[Y].c_str());
                if (Y==0)
                {
                    if (C[4])
                        Element_Info1("Erase Page");
                    if (C[5])
                        Element_Info1("Newsflash");
                    if (C[6])
                        Element_Info1("Subtitle");
                    if (C[7])
                        Element_Info1("Suppress Header");
                    if (C[8])
                        Element_Info1("Update Indicator");
                    if (C[9])
                        Element_Info1("Interrupted Sequence");
                    if (C[10])
                        Element_Info1("Inhibit Display");
                    if (C[11])
                        Element_Info1("Magazine Serial");
                }
            }
        }
    #endif //MEDIAINFO_TRACE
}

//---------------------------------------------------------------------------
void File_Teletext::HasChanged()
{
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_TELETEXT_YES
