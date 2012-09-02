// File_Ibi_Creation - Creation of Ibi files
// Copyright (C) 2011-2012 MediaArea.net SARL, Info@MediaArea.net
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
#if MEDIAINFO_IBI
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Ibi_Creation.h"
#include <cstring>
#include <zlib.h>
#include "base64.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Ibi structure
//***************************************************************************

//---------------------------------------------------------------------------
ibi::ibi()
{
}

//---------------------------------------------------------------------------
ibi::~ibi()
{
    for (streams::iterator Stream=Streams.begin(); Stream!=Streams.end(); ++Stream)
        delete Stream->second; //Stream->second=NULL;
}

//---------------------------------------------------------------------------
void ibi::stream::Add (const info &Info)
{
    if (!IsSynchronized)
    {
        //Searching the right insertion point
        for (Infos_Pos=0; Infos_Pos<Infos.size(); Infos_Pos++)
            if (Info.StreamOffset<Infos[Infos_Pos].StreamOffset)
                break;
    }

    //Testing if new data is same as current insertion point
    if (Infos_Pos && Infos[Infos_Pos-1].FrameNumber==Info.FrameNumber && Info.FrameNumber!=(int64u)-1)
    {
        IsSynchronized=true;
        return;
    }
    if (Infos_Pos && Info.Dts==Infos[Infos_Pos-1].Dts && Info.Dts!=(int64u)-1)
    {
        //Duplicate, updating it (in case of new frame count)
        if (Infos_Pos && Info.FrameNumber!=(int64u)-1 && Info.FrameNumber<Infos[Infos_Pos-1].FrameNumber)
        {
            Infos[Infos_Pos-1].FrameNumber=Info.FrameNumber;
            Infos[Infos_Pos-1].Dts=Info.Dts;
        }

        IsSynchronized=true;
        return;
    }
    if (Infos_Pos && Info.StreamOffset==Infos[Infos_Pos-1].StreamOffset)
    {
        //Duplicate, updating it (in case of new frame count)
        if (Infos_Pos && Info.FrameNumber!=(int64u)-1 && Info.FrameNumber<Infos[Infos_Pos-1].FrameNumber)
        {
            Infos[Infos_Pos-1].FrameNumber=Info.FrameNumber;
            Infos[Infos_Pos-1].Dts=Info.Dts;
        }

        IsSynchronized=true;
        return;
    }

    //Previous item
    if (IsSynchronized && Infos_Pos)
    {
        Infos[Infos_Pos-1].IsContinuous=true;
        IsContinuous=true;

        IsModified=true;
    }
    IsSynchronized=true;

    //Testing if new data is same as next insertion point
    if (Infos_Pos<Infos.size() && Infos[Infos_Pos].FrameNumber==Info.FrameNumber && Info.FrameNumber!=(int64u)-1)
    {
        Infos_Pos++;

        IsSynchronized=true;
        return;
    }
    if (Infos_Pos<Infos.size() && Info.Dts==Infos[Infos_Pos].Dts && Info.Dts!=(int64u)-1)
    {
        Infos_Pos++;

        IsSynchronized=true;
        return;
    }
    if (Infos_Pos<Infos.size() && Info.StreamOffset==Infos[Infos_Pos].StreamOffset)
    {
        //Duplicate, updating it (in case of new frame count)
        if (IsSynchronized && Info.FrameNumber!=(int64u)-1)
            Infos[Infos_Pos].FrameNumber=Info.FrameNumber;

        Infos_Pos++;

        IsSynchronized=true;
        return;
    }

    Infos.insert(Infos.begin()+Infos_Pos, Info);
    Infos_Pos++;

    IsModified=true;
}

//---------------------------------------------------------------------------
void ibi::stream::Unsynch ()
{
    Infos_Pos=0;
    IsModified=false;
    IsContinuous=false;
    IsSynchronized=false;
}

//***************************************************************************
// Utils
//***************************************************************************

size_t int64u2Ebml(int8u* List, int64u Value)
{
    //1 byte
    if (Value<=126) //2^7-2
    {
        if (List)
        {
            List[0]=0x80|((int8u)Value);
        }
        return 1;
    }

    //2 bytes
    if (Value<=16382) //2^14-2
    {
        if (List)
        {
            List[0]=0x40|((int8u)(Value>>8));
            List[1]=(int8u)Value;
        }
        return 2;
    }

    //3 bytes
    if (Value<=2097150) //2^21-2
    {
        if (List)
        {
            List[0]=0x20|((int8u)(Value>>16));
            List[1]=(int8u)(Value>>8);
            List[2]=(int8u)Value;
        }
        return 3;
    }

    //4 bytes
    if (Value<=268435454) //2^28-2
    {
        if (List)
        {
            List[0]=0x10|((int8u)(Value>>24));
            List[1]=(int8u)(Value>>16);
            List[2]=(int8u)(Value>>8);
            List[3]=(int8u)Value;
        }
        return 4;
    }

    //5 bytes
    if (Value<=34359738366LL) //2^35-2
    {
        if (List)
        {
            List[0]=0x08|((int8u)(Value>>32));
            List[1]=(int8u)(Value>>24);
            List[2]=(int8u)(Value>>16);
            List[3]=(int8u)(Value>>8);
            List[4]=(int8u)Value;
        }
        return 5;
    }

    //6 bytes
    if (Value<=4398046511102LL) //2^42-2
    {
        if (List)
        {
            List[0]=0x04|((int8u)(Value>>40));
            List[1]=(int8u)(Value>>32);
            List[2]=(int8u)(Value>>24);
            List[3]=(int8u)(Value>>16);
            List[4]=(int8u)(Value>>8);
            List[5]=(int8u)Value;
        }
        return 6;
    }

    //7 bytes
    if (Value<=562949953421310LL) //2^49-2
    {
        if (List)
        {
            List[0]=0x02|((int8u)(Value>>48));
            List[1]=(int8u)(Value>>40);
            List[2]=(int8u)(Value>>32);
            List[3]=(int8u)(Value>>24);
            List[4]=(int8u)(Value>>16);
            List[5]=(int8u)(Value>>8);
            List[6]=(int8u)Value;
        }
        return 7;
    }

    //8 bytes
    if (Value<=72057594037927934LL) //2^56-2
    {
        if (List)
        {
            List[0]=0x01;
            List[1]=(int8u)(Value>>56);
            List[2]=(int8u)(Value>>40);
            List[3]=(int8u)(Value>>32);
            List[4]=(int8u)(Value>>24);
            List[5]=(int8u)(Value>>16);
            List[6]=(int8u)(Value>>8);
            List[7]=(int8u)Value;
        }
        return 8;
    }

    if (List)
        List[0]=0xFF;
    return 1;
}

size_t EbmlBlock(int8u* List, size_t List_MaxSize, int64u Code, int8u* Content, size_t Content_Size)
{
    if (Content_Size==0)
        return 0;

    size_t Code_EbmlSize=int64u2Ebml(NULL, Code);
    size_t Content_EbmlSize=int64u2Ebml(NULL, Content_Size);

    if (List && Code_EbmlSize+Content_EbmlSize+Content_Size>List_MaxSize)
        return 0;

    if (List)
    {
        List+=int64u2Ebml(List, Code);
        List+=int64u2Ebml(List, Content_Size);
        std::memcpy(List, Content, Content_Size); List+=Content_Size;//Content
    }

    return Code_EbmlSize+Content_EbmlSize+Content_Size;
}


//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Ibi_Creation::File_Ibi_Creation()
{
}

//---------------------------------------------------------------------------
File_Ibi_Creation::~File_Ibi_Creation()
{
    for (size_t Pos=0; Pos<Buffers.size(); Pos++)
        delete Buffers[Pos];
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ibi_Creation::Set(const ibi &Ibi)
{
    for (ibi::streams::const_iterator IbiStream_Temp=Ibi.Streams.begin(); IbiStream_Temp!=Ibi.Streams.end(); ++IbiStream_Temp)
        Add(IbiStream_Temp->first, *IbiStream_Temp->second);
}

//---------------------------------------------------------------------------
void File_Ibi_Creation::Add(int64u ID, const ibi::stream &Stream)
{
    //Useful?
    if (Stream.Infos.empty())
        return;

    //Header
    int8u* IbiHeader;
    size_t IbiHeader_Offset=0;
    if (ID!=(int64u)-1)
    {
        IbiHeader=new int8u[int64u2Ebml(NULL, ID)];
        IbiHeader_Offset+=int64u2Ebml(IbiHeader, ID);
    }
    else
        IbiHeader=NULL;

    //Init - Byte Offset
    int8u* IbiByteOffset=new int8u[8*Stream.Infos.size()];
    size_t IbiByteOffset_Offset=0;

    //Init - Frame Number
    int8u* IbiFrameNumber=new int8u[8*Stream.Infos.size()];
    size_t IbiFrameNumber_Offset=0;

    //Init - DTS
    int8u* IbiDts=new int8u[8*Stream.Infos.size()];
    size_t IbiDts_Offset=0;
    IbiDts_Offset+=int64u2Ebml(IbiDts+IbiDts_Offset, Stream.DtsFrequencyNumerator);
    IbiDts_Offset+=int64u2Ebml(IbiDts+IbiDts_Offset, Stream.DtsFrequencyDenominator);

    //Per item
    for (size_t Pos=0; Pos<Stream.Infos.size(); Pos++)
    {
        IbiByteOffset_Offset+=int64u2Ebml(IbiByteOffset+IbiByteOffset_Offset, Stream.Infos[Pos].StreamOffset-(Pos?Stream.Infos[Pos-1].StreamOffset:0));
        IbiFrameNumber_Offset+=int64u2Ebml(IbiFrameNumber+IbiFrameNumber_Offset, Stream.Infos[Pos].FrameNumber-(Pos?Stream.Infos[Pos-1].FrameNumber:0));
        IbiDts_Offset+=int64u2Ebml(IbiDts+IbiDts_Offset, Stream.Infos[Pos].Dts-(Pos?Stream.Infos[Pos-1].Dts:0));
    }

    //Sizes
    size_t IbiHeader_EbmlSize=EbmlBlock(NULL, 0, 0x01, NULL, IbiHeader_Offset);
    size_t IbiByteOffset_EbmlSize=EbmlBlock(NULL, 0, 0x02, NULL, IbiByteOffset_Offset);
    size_t IbiFrameNumber_EbmlSize=EbmlBlock(NULL, 0, 0x03, NULL, IbiFrameNumber_Offset);
    size_t IbiDts_EbmlSize=EbmlBlock(NULL, 0, 0x04, NULL, IbiDts_Offset);

    //Buffer
    buffer* Buffer=new buffer;
    Buffer->Content=new int8u[1+int64u2Ebml(NULL, IbiHeader_EbmlSize+IbiByteOffset_EbmlSize+IbiFrameNumber_EbmlSize+IbiDts_EbmlSize)+IbiHeader_EbmlSize+IbiByteOffset_EbmlSize+IbiFrameNumber_EbmlSize+IbiDts_EbmlSize];
    Buffer->Size+=int64u2Ebml(Buffer->Content+Buffer->Size, 0x01);              //Stream
    Buffer->Size+=int64u2Ebml(Buffer->Content+Buffer->Size, IbiHeader_EbmlSize+IbiByteOffset_EbmlSize+IbiFrameNumber_EbmlSize+IbiDts_EbmlSize);     //Size
    Buffer->Size+=EbmlBlock(Buffer->Content+Buffer->Size, IbiHeader_EbmlSize, 0x01, IbiHeader, IbiHeader_Offset); //IbiHeader
    Buffer->Size+=EbmlBlock(Buffer->Content+Buffer->Size, IbiByteOffset_EbmlSize, 0x02, IbiByteOffset, IbiByteOffset_Offset); //ByteOffset
    Buffer->Size+=EbmlBlock(Buffer->Content+Buffer->Size, IbiFrameNumber_EbmlSize, 0x03, IbiFrameNumber, IbiFrameNumber_Offset); //FrameNumber
    Buffer->Size+=EbmlBlock(Buffer->Content+Buffer->Size, IbiDts_EbmlSize, 0x04, IbiDts, IbiDts_Offset); //Dts

    Buffers.push_back(Buffer);
}

Ztring File_Ibi_Creation::Finish()
{
    //Test
    if (Buffers.empty())
        return Ztring();

    //Size computing
    size_t Size=4+1+2+1+15+2+1+1; //Header size
    for (size_t Pos=0; Pos<Buffers.size(); Pos++)
        Size+=Buffers[Pos]->Size;

    //Buffer creation
    int8u* Main=new int8u[Size];
    size_t Main_Offset=0;

    //Header
    Main_Offset+=int64u2Ebml(Main+Main_Offset, 0x0A45DFA3);                 //EBML
    Main_Offset+=int64u2Ebml(Main+Main_Offset, 2+1+15+2+1+1);               //Size (Code + Size + Content, twice)
    Main_Offset+=int64u2Ebml(Main+Main_Offset, 0x0282);                     //DocType
    Main_Offset+=int64u2Ebml(Main+Main_Offset, 15);                         //Size
    std::memcpy(Main+Main_Offset, "MediaInfo Index", 15); Main_Offset+=15;  //Content
    Main_Offset+=int64u2Ebml(Main+Main_Offset, 0x0285);                     //DocTypeReadVersion
    Main_Offset+=int64u2Ebml(Main+Main_Offset, 1);                          //Size
    Main[Main_Offset]=0x01; Main_Offset+=1;                                 //Content

    //for each stream
    for (size_t Pos=0; Pos<Buffers.size(); Pos++)
    {
        std::memcpy(Main+Main_Offset, Buffers[Pos]->Content, Buffers[Pos]->Size);
        Main_Offset+=Buffers[Pos]->Size;
    }



    //Compressed
    buffer Buffer;
    int8u* Compressed=new int8u[Main_Offset];
    unsigned long Compressed_OffsetWihtoutHeader=(unsigned long)Main_Offset;
    if (compress2(Compressed, &Compressed_OffsetWihtoutHeader, Main, (unsigned long)Main_Offset, Z_BEST_COMPRESSION)==Z_OK && Compressed_OffsetWihtoutHeader+100<Main_Offset)
    {
        size_t Header_Offset=4+1+2+1+15+2+1+1; //Same header
        Header_Offset+=int64u2Ebml(Main+Header_Offset, 0x02);                   //Compressed index
        Header_Offset+=int64u2Ebml(Main+Header_Offset, int64u2Ebml(NULL, Main_Offset)+Compressed_OffsetWihtoutHeader); //Size
        Header_Offset+=int64u2Ebml(Main+Header_Offset, Main_Offset);            //Uncompressed size

        //Filling
        Buffer.Size=Header_Offset+Compressed_OffsetWihtoutHeader;
        Buffer.Content=new int8u[Buffer.Size];
        std::memcpy(Buffer.Content, Main, Header_Offset); //Same header + Compressed index header
        std::memcpy(Buffer.Content+Header_Offset, Compressed, Compressed_OffsetWihtoutHeader); //Same header
    }
    else
    {
        //Filling
        Buffer.Size=Main_Offset;
        Buffer.Content=new int8u[Buffer.Size];
        std::memcpy(Buffer.Content, Main, Main_Offset);
    }

    std::string Data_Raw((const char*)Buffer.Content, Buffer.Size);
    std::string Data_Base64(Base64::encode(Data_Raw));

    return Ztring().From_UTF8(Data_Base64);
}

} //NameSpace

#endif //MEDIAINFO_IBI_YES

