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
#if MEDIAINFO_IBI
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Ibi.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include <zlib.h>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Ibi::File_Ibi()
:File__Analyze()
{
    //Configuration
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX
    DataMustAlwaysBeComplete=false;

    //In
    Ibi=NULL;

    //Temp
    Ibi_MustDelete=false;
}

//---------------------------------------------------------------------------
File_Ibi::~File_Ibi()
{
    if (Ibi_MustDelete)
        delete Ibi; //Ibi=NULL;
}

//***************************************************************************
// Get information
//***************************************************************************

//---------------------------------------------------------------------------
const Ztring &File_Ibi::Get (stream_t /*StreamKind*/, size_t /*StreamNumber*/, const Ztring &Parameter, info_t /*KindOfInfo*/, info_t /*KindOfSearch*/)
{
    ibi::streams::iterator IbiStream_Temp=Ibi->Streams.begin(); //TODO: management of multiple streams
    if (IbiStream_Temp!=Ibi->Streams.end() && !IbiStream_Temp->second->Infos.empty())
    {
        int64u FrameNumber=Parameter.To_int64u();

        for (size_t Pos=0; Pos<IbiStream_Temp->second->Infos.size()-1; Pos++)
            if (IbiStream_Temp->second->Infos[Pos].FrameNumber==FrameNumber || IbiStream_Temp->second->Infos[Pos+1].FrameNumber>FrameNumber)
            {
                Get_Temp=__T("StreamOffset=")+Ztring::ToZtring(IbiStream_Temp->second->Infos[Pos].StreamOffset)
                       + __T(" / FrameNumber=")+Ztring::ToZtring(IbiStream_Temp->second->Infos[Pos].FrameNumber)
                       + __T(" / Dts=")+Ztring::ToZtring(IbiStream_Temp->second->Infos[Pos].Dts);
                return Get_Temp;
            }
    }

    //Can not be found
    Get_Temp.clear();
    return Get_Temp;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ibi::Streams_Accept()
{
    Fill(Stream_General, 0, General_Format, "Ibi");

    if (Ibi==NULL)
    {
        Ibi=new ibi();
        Ibi_MustDelete=true;
    }
}

//---------------------------------------------------------------------------
void File_Ibi::Streams_Finish()
{
    Config->File_KeepInfo_Set(true); //In order to let Get() available

    if (Count_Get(Stream_Video)==0) //If not yet done by Inform part
        for (ibi::streams::iterator IbiStream_Temp=Ibi->Streams.begin(); IbiStream_Temp!=Ibi->Streams.end(); ++IbiStream_Temp)
        {
            Stream_Prepare(Stream_Video);
            Fill(Stream_Video, StreamPos_Last, General_ID, IbiStream_Temp->first);
        }
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ibi::Header_Parse()
{
    //Test of zero padding
    int8u Null;
    Peek_B1(Null);
    if (Null==0x00)
    {
        if (Buffer_Offset_Temp==0)
            Buffer_Offset_Temp=Buffer_Offset+1;

        while (Buffer_Offset_Temp<Buffer_Size)
        {
            if (Buffer[Buffer_Offset_Temp])
                break;
            Buffer_Offset_Temp++;
        }
        if (Buffer_Offset_Temp>=Buffer_Size)
        {
            Element_WaitForMoreData();
            return;
        }

        Header_Fill_Code((int32u)-1); //Should be (int64u)-1 but Borland C++ does not like this
        Header_Fill_Size(Buffer_Offset_Temp-Buffer_Offset);
        Buffer_Offset_Temp=0;

        return;
    }

    //Parsing
    int64u Name, Size;
    Get_EB (Name,                                               "Name");
    Get_EB (Size,                                               "Size");

    //Filling
    Header_Fill_Code(Name, Ztring().From_Number(Name, 16));
    Header_Fill_Size(Element_Offset+Size);
}

//---------------------------------------------------------------------------
namespace Elements
{
    //Common
    const int64u Zero=(int32u)-1; //Should be (int64u)-1 but Borland C++ does not like this
    const int64u CRC32=0x3F;
    const int64u Void=0x6C;

    //EBML
    const int64u Ebml=0xA45DFA3;
    const int64u Ebml_Version=0x286;
    const int64u Ebml_ReadVersion=0x2F7;
    const int64u Ebml_MaxIDLength=0x2F2;
    const int64u Ebml_MaxSizeLength=0x2F3;
    const int64u Ebml_DocType=0x282;
    const int64u Ebml_DocTypeVersion=0x287;
    const int64u Ebml_DocTypeReadVersion=0x285;

    //Segment
    const int64u Stream=1;
    const int64u Stream_Header=1;
    const int64u Stream_ByteOffset=2;
    const int64u Stream_FrameNumber=3;
    const int64u Stream_Dts=4;
    const int64u CompressedIndex=2;
    const int64u WritingApplication=3;
    const int64u WritingApplication_Name=1;
    const int64u WritingApplication_Version=2;
    const int64u InformData=4;
    const int64u SourceInfo=5;
    const int64u SourceInfo_IndexCreationDate=1;
    const int64u SourceInfo_SourceModificationDate=2;
    const int64u SourceInfo_SourceSize=3;
}

//---------------------------------------------------------------------------
void File_Ibi::Data_Parse()
{
    #define LIS2(_ATOM, _NAME) \
        case Elements::_ATOM : \
                if (Level==Element_Level) \
                { \
                    Element_Name(_NAME); \
                    _ATOM(); \
                    Element_ThisIsAList(); \
                } \

    #define ATO2(_ATOM, _NAME) \
                case Elements::_ATOM : \
                        if (Level==Element_Level) \
                        { \
                            if (Element_IsComplete_Get()) \
                            { \
                                Element_Name(_NAME); \
                                _ATOM(); \
                            } \
                            else \
                            { \
                                Element_WaitForMoreData(); \
                                return; \
                            } \
                        } \
                        break; \

    #define ATOM_END_MK \
        ATOM(Zero) \
        ATOM(CRC32) \
        ATOM(Void) \
        ATOM_END

    //Parsing
    DATA_BEGIN
    LIST(Ebml)
        ATOM_BEGIN
        ATOM(Ebml_Version)
        ATOM(Ebml_ReadVersion)
        ATOM(Ebml_MaxIDLength)
        ATOM(Ebml_MaxSizeLength)
        ATOM(Ebml_DocType)
        ATOM(Ebml_DocTypeVersion)
        ATOM(Ebml_DocTypeReadVersion)
        ATOM_END_MK
    LIST(Stream)
        ATOM_BEGIN
            ATOM(Stream_Header)
            ATOM(Stream_ByteOffset)
            ATOM(Stream_FrameNumber)
            ATOM(Stream_Dts)
        ATOM_END_MK
    ATOM(CompressedIndex)
    LIST(WritingApplication)
        ATOM_BEGIN
            ATOM(WritingApplication_Name)
            ATOM(WritingApplication_Version)
        ATOM_END_MK
    LIST(SourceInfo)
        ATOM_BEGIN
            ATOM(SourceInfo_IndexCreationDate)
            ATOM(SourceInfo_SourceModificationDate)
            ATOM(SourceInfo_SourceSize)
        ATOM_END_MK
    ATOM(InformData)
    DATA_END_DEFAULT
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ibi::Zero()
{
    Element_Name("ZeroPadding");
}

//---------------------------------------------------------------------------
void File_Ibi::CRC32()
{
    Element_Name("CRC32");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Ibi::Void()
{
    Element_Name("Void");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Ibi::Ebml()
{
    Element_Name("Ebml");
}

//---------------------------------------------------------------------------
void File_Ibi::Ebml_Version()
{
    Element_Name("Version");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Ibi::Ebml_ReadVersion()
{
    Element_Name("ReadVersion");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Ibi::Ebml_MaxIDLength()
{
    Element_Name("MaxIDLength");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Ibi::Ebml_MaxSizeLength()
{
    Element_Name("MaxSizeLength");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Ibi::Ebml_DocType()
{
    Element_Name("DocType");

    //Parsing
    Ztring Data;
    Get_Local(Element_Size, Data,                               "Data"); Element_Info1(Data);

    //Filling
    FILLING_BEGIN();

        if (Data==__T("MediaInfo Index"))
            Accept("Ibi");
        else
        {
            Reject("Ibi");
            return;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Ibi::Ebml_DocTypeVersion()
{
    Element_Name("DocTypeVersion");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Ibi::Ebml_DocTypeReadVersion()
{
    Element_Name("DocTypeReadVersion");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Ibi::Stream()
{
    Element_Name("Stream");

    ID_Current=(int64u)-1;
}

void File_Ibi::Stream_Header()
{
    Element_Name("Stream Header");
    Get_EB (ID_Current,                                         "ID");

    FILLING_BEGIN();
        if (Ibi)
        {
            //Filling information for ID after data
            ibi::streams::iterator IbiStream_Temp=Ibi->Streams.find((int64u)-1);
            if (IbiStream_Temp!=Ibi->Streams.end())
            {
                Ibi->Streams[ID_Current]=IbiStream_Temp->second;
                Ibi->Streams.erase(IbiStream_Temp);
            }
        }
    FILLING_END();
}

void File_Ibi::Stream_ByteOffset()
{
    Element_Name("Byte Offset");

    //Parsing
    size_t Pos=0;
    int64u Offset=0;
    while (Element_Offset<Element_Size)
    {
        int64u Item;
        Get_EB (Item,                                           "Item");
        Offset+=Item;
        Param_Info1(Pos);
        Param_Info1(Ztring::ToZtring(Offset)+__T(" (0x")+Ztring::ToZtring(Offset, 16)+__T(')'));

        FILLING_BEGIN();
            if (Ibi)
            {
                //Filling information for ID after data
                if (Ibi->Streams[ID_Current]==NULL)
                    Ibi->Streams[ID_Current]=new ibi::stream();
                if (Pos>=Ibi->Streams[ID_Current]->Infos.size())
                {
                    Ibi->Streams[ID_Current]->Infos.push_back(ibi::stream::info());
                    Ibi->Streams[ID_Current]->Infos[Pos].IsContinuous=true; //default
                }
                Ibi->Streams[ID_Current]->Infos[Pos].StreamOffset=Offset;
                Pos++;
            }
        FILLING_END();
    }
}

void File_Ibi::Stream_FrameNumber()
{
    Element_Name("Frame Number");

    //Parsing
    size_t Pos=0;
    int64u Offset=0;
    while (Element_Offset<Element_Size)
    {
        int64u Item;
        Get_EB (Item,                                           "Item");
        Offset+=Item;
        Param_Info1(Pos);
        Param_Info1(Ztring::ToZtring(Offset)+__T(" (0x")+Ztring::ToZtring(Offset, 16)+__T(')'));

        FILLING_BEGIN();
            if (Ibi)
            {
                //Filling information for ID after data
                if (Ibi->Streams[ID_Current]==NULL)
                    Ibi->Streams[ID_Current]=new ibi::stream();
                if (Pos>=Ibi->Streams[ID_Current]->Infos.size())
                {
                    Ibi->Streams[ID_Current]->Infos.push_back(ibi::stream::info());
                    Ibi->Streams[ID_Current]->Infos[Pos].IsContinuous=true; //default
                }
                Ibi->Streams[ID_Current]->Infos[Pos].FrameNumber=Offset;
                Pos++;
            }
        FILLING_END();
    }
}

void File_Ibi::Stream_Dts()
{
    Element_Name("DTS");

    //Parsing
    int64u Item;
    Get_EB (Item,                                               "DtsFrequencyNumerator");
    FILLING_BEGIN();
        if (Ibi)
            Ibi->Streams[ID_Current]->DtsFrequencyNumerator=Item;
    FILLING_END();
    Get_EB (Item,                                               "DtsFrequencyDenominator");
    FILLING_BEGIN();
        if (Ibi)
        {
            Ibi->Streams[ID_Current]->DtsFrequencyDenominator=Item;

            //Handling of previous inverted Numerator/Denominator
            if (Ibi->Streams[ID_Current]->DtsFrequencyNumerator<Ibi->Streams[ID_Current]->DtsFrequencyDenominator)
                std::swap(Ibi->Streams[ID_Current]->DtsFrequencyNumerator, Ibi->Streams[ID_Current]->DtsFrequencyDenominator);
        }
    FILLING_END();

    size_t Pos=0;
    int64u Offset=0;
    while (Element_Offset<Element_Size)
    {
        int64u Item;
        Get_EB (Item,                                           "Item");
        Offset+=Item;
        Param_Info1(Pos);
        Param_Info1(Ztring::ToZtring(Offset)+__T(" (0x")+Ztring::ToZtring(Offset, 16)+__T(')'));

        FILLING_BEGIN();
            if (Ibi)
            {
                //Filling information for ID after data
                if (Ibi->Streams[ID_Current]==NULL)
                    Ibi->Streams[ID_Current]=new ibi::stream();
                if (Pos>=Ibi->Streams[ID_Current]->Infos.size())
                {
                    Ibi->Streams[ID_Current]->Infos.push_back(ibi::stream::info());
                    Ibi->Streams[ID_Current]->Infos[Pos].IsContinuous=true; //default
                }
                Ibi->Streams[ID_Current]->Infos[Pos].Dts=Offset;
                Pos++;
            }
        FILLING_END();
    }
}

void File_Ibi::CompressedIndex()
{
    if (!Status[IsAccepted])
    {
        Reject("Ibi");
        return;
    }

    Element_Name("Compressed Index");
    int64u UncompressedSize;
    Get_EB (UncompressedSize,                                   "Uncompressed size");

    //Sizes
    unsigned long Source_Size=(unsigned long)(Element_Size-Element_Offset);
    unsigned long Dest_Size=(unsigned long)UncompressedSize;

    //Uncompressing
    int8u* Dest;
    try
    {
        Dest=new int8u[Dest_Size];
    }
    catch (...)
    {
        //Memory error
        Reject();
        return;
    }
    if (uncompress((Bytef*)Dest, &Dest_Size, (const Bytef*)Buffer+Buffer_Offset+(size_t)Element_Offset, Source_Size)<0)
    {
        Skip_XX(Element_Size-Element_Offset,                    "Problem during the decompression");
        delete[] Dest; //Dest=NULL;
        return;
    }

    //Exiting this element
    Skip_XX(Element_Size-Element_Offset,                        "Will be parsed");

    //Configuring buffer
    const int8u* Buffer_Sav=Buffer;
    size_t Buffer_Size_Sav=Buffer_Size;
    int8u* Buffer_Temp_Sav=Buffer_Temp;
    size_t Buffer_Temp_Size_Sav=Buffer_Temp_Size;
    size_t Buffer_Offset_Sav=Buffer_Offset;
    size_t Buffer_Offset_Temp_Sav=Buffer_Offset_Temp;
    Buffer=NULL;
    Buffer_Size=0;
    Buffer_Temp=NULL;
    Buffer_Temp_Size=0;
    Buffer_Offset=0;
    Buffer_Offset_Temp=0;

    //Configuring level
    std::vector<int64u> Element_Sizes_Sav;
    size_t Element_Level_Sav=Element_Level;
    while(Element_Level)
    {
        Element_Sizes_Sav.push_back(Element_TotalSize_Get());
        Element_End0();
    }

    //Configuring file size
    int64u File_Size_Sav=File_Size;
    if (File_Size<File_Offset+Buffer_Offset+Element_Offset+Dest_Size)
        File_Size=File_Offset+Buffer_Offset+Element_Offset+Dest_Size;
    Element_Level++;
    Header_Fill_Size(File_Size);
    Element_Level--;

    //Parsing
    Buffer=Dest;
    Buffer_Size=Dest_Size;
    while (Open_Buffer_Continue_Loop());
    delete[] Dest; //Dest=NULL;

    //Resetting file size
    File_Size=File_Size_Sav;
    while(Element_Level)
        Element_End0();
    Element_Level++;
    Header_Fill_Size(File_Size);
    Element_Level--;

    //Configuring level
    while(Element_Level<Element_Level_Sav)
    {
        Element_Begin0();
        Element_Begin0();
        Header_Fill_Size(Element_Sizes_Sav[0]);
        Element_End0();
    }

    //Resetting buffer
    Buffer=Buffer_Sav;
    Buffer_Size=Buffer_Size_Sav;
    Buffer_Temp=Buffer_Temp_Sav;
    Buffer_Temp_Size=Buffer_Temp_Size_Sav;
    Buffer_Offset=Buffer_Offset_Sav;
    Buffer_Offset_Temp=Buffer_Offset_Temp_Sav;
}

//---------------------------------------------------------------------------
void File_Ibi::WritingApplication()
{
    Element_Name("WritingApplication");
}

//---------------------------------------------------------------------------
void File_Ibi::WritingApplication_Name()
{
    Element_Name("Name");

    //Parsing
    Skip_UTF8(Element_Size,                                     "Data");
}

//---------------------------------------------------------------------------
void File_Ibi::WritingApplication_Version()
{
    Element_Name("Version");

    //Parsing
    Skip_UTF8(Element_Size,                                     "Data");
}

//---------------------------------------------------------------------------
void File_Ibi::SourceInfo()
{
    Element_Name("Source Information");
}

//---------------------------------------------------------------------------
void File_Ibi::SourceInfo_IndexCreationDate()
{
    Element_Name("Index Creation Date");

    //Parsing
    Skip_B8(                                                    "Data");
}

//---------------------------------------------------------------------------
void File_Ibi::SourceInfo_SourceModificationDate()
{
    Element_Name("Source Modification Date");

    //Parsing
    Skip_B8(                                                    "Data");
}

//---------------------------------------------------------------------------
void File_Ibi::SourceInfo_SourceSize()
{
    Element_Name("Source Size");

    //Parsing
    Skip_B8(                                                    "Data");
}

//---------------------------------------------------------------------------
void File_Ibi::InformData()
{
    Element_Name("InformData");

    //Parsing
    Ztring InformData_FromFile;
    Get_UTF8 (Element_Size, InformData_FromFile,                "Data");

    //Filling
    ZtringListList Fields(InformData_FromFile);
    if (Config->Ibi_UseIbiInfoIfAvailable_Get())
    {
        for (size_t Pos=0; Pos<Fields.size(); Pos++)
        {
            if (Pos==0 || Fields[Pos].size()<2)
            {
                if (Pos)
                    Pos++;
                if (Pos>Fields.size() || Fields[Pos].size()<1)
                    break; //End or problem

                if (Fields[Pos][0]==__T("General"))
                    ; //Nothing to do
                else if (Fields[Pos][0]==__T("Video"))
                    Stream_Prepare(Stream_Video);
                else if (Fields[Pos][0]==__T("Audio"))
                    Stream_Prepare(Stream_Audio);
                else if (Fields[Pos][0]==__T("Text"))
                    Stream_Prepare(Stream_Text);
                else if (Fields[Pos][0]==__T("Other"))
                    Stream_Prepare(Stream_Other);
                else if (Fields[Pos][0]==__T("Image"))
                    Stream_Prepare(Stream_Image);
                else if (Fields[Pos][0]==__T("Menu"))
                    Stream_Prepare(Stream_Menu);
                else
                    break; //Problem
                Pos++;
            }

            Fill(StreamKind_Last, StreamPos_Last, Fields[Pos][0].To_UTF8().c_str(), Fields[Pos][1], true);
            if (Info_Options<Fields[Pos].size())
                (*Stream_More)[StreamKind_Last][StreamPos_Last](Fields[Pos][0].To_UTF8().c_str(), Info_Options)=Fields[Pos][Info_Options];
        }
    }
}

//***************************************************************************
// Data
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ibi::UInteger_Info()
{
    switch (Element_Size)
    {
        case 1 :
                {
                    Info_B1(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 2 :
                {
                    Info_B2(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 3 :
                {
                    Info_B3(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 4 :
                {
                    Info_B4(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 5 :
                {
                    Info_B5(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 6 :
                {
                    Info_B6(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 7 :
                {
                    Info_B7(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 8 :
                {
                    Info_B8(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 16:
                {
                    Info_B16(Data,                              "Data"); Element_Info1(Data);
                    return;
                }
        default : Skip_XX(Element_Size,                         "Data");
    }
}

//---------------------------------------------------------------------------
int64u File_Ibi::UInteger_Get()
{
    switch (Element_Size)
    {
        case 1 :
                {
                    int8u Data;
                    Get_B1 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 2 :
                {
                    int16u Data;
                    Get_B2 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 3 :
                {
                    int32u Data;
                    Get_B3 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 4 :
                {
                    int32u Data;
                    Get_B4 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 5 :
                {
                    int64u Data;
                    Get_B5 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 6 :
                {
                    int64u Data;
                    Get_B6 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 7 :
                {
                    int64u Data;
                    Get_B7 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 8 :
                {
                    int64u Data;
                    Get_B8 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        default :   Skip_XX(Element_Size,                       "Data");
                    return 0;
    }
}

//---------------------------------------------------------------------------
int128u File_Ibi::UInteger16_Get()
{
    switch (Element_Size)
    {
        case 1 :
                {
                    int8u Data;
                    Get_B1 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 2 :
                {
                    int16u Data;
                    Get_B2 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 3 :
                {
                    int32u Data;
                    Get_B3 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 4 :
                {
                    int32u Data;
                    Get_B4 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 5 :
                {
                    int64u Data;
                    Get_B5 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 6 :
                {
                    int64u Data;
                    Get_B6 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 7 :
                {
                    int64u Data;
                    Get_B7 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 8 :
                {
                    int64u Data;
                    Get_B8 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 16:
                {
                    int128u Data;
                    Get_B16(Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        default :   Skip_XX(Element_Size,                       "Data");
                    return 0;
    }
}

} //NameSpace

#endif //MEDIAINFO_IBI_YES
