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
#if defined(MEDIAINFO_AAF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Aaf.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "ZenLib/FileName.h"
#include "ZenLib/File.h"
#include "tinyxml2.h"
using namespace ZenLib;
using namespace tinyxml2;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Info
//***************************************************************************

const char* AAf_tagSTGTY (int8u tagSTGTY)
{
    switch (tagSTGTY)
    {
        case 0 : return "unknown";
        case 1 : return "storage";
        case 2 : return "stream";
        case 3 : return "ILockBytes";
        case 4 : return "IPropertyStorage";
        case 5 : return "root";
        default: return "";
    }
}

const char* AAf_tagDECOLOR (int8u tagDECOLOR)
{
    switch (tagDECOLOR)
    {
        case 0 : return "red";
        case 1 : return "black";
        default: return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Aaf::File_Aaf()
:File__Analyze()
{
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Aaf;
        StreamIDs_Width[0]=16;
    #endif //MEDIAINFO_EVENTS

    //Temp
    ReferenceFiles=NULL;
}

//---------------------------------------------------------------------------
File_Aaf::~File_Aaf()
{
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        delete Streams[Pos];

    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aaf::Streams_Finish()
{
    if (ReferenceFiles==NULL)
        return;

    ReferenceFiles->ParseReferences();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_Aaf::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    if (ReferenceFiles==NULL)
        return 0;

    return ReferenceFiles->Seek(Method, Value, ID);
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Aaf::FileHeader_Begin()
{
    if (File_Size<0x100)
    {
        Reject("Aaf");
        return false;
    }

    //Element_Size
    if (Buffer_Size<0x18)
        return false; //Must wait for more data

    if (Buffer[ 0x0]!=0xD0
     || Buffer[ 0x1]!=0xCF
     || Buffer[ 0x2]!=0x11
     || Buffer[ 0x3]!=0xE0
     || Buffer[ 0x4]!=0xA1
     || Buffer[ 0x5]!=0xB1
     || Buffer[ 0x6]!=0x1A
     || Buffer[ 0x7]!=0xE1
     || Buffer[ 0x8]!=0x41
     || Buffer[ 0x9]!=0x41
     || Buffer[ 0xA]!=0x46
     || Buffer[ 0xB]!=0x42
     || Buffer[ 0xC]!=0x0D
     || Buffer[ 0xD]!=0x00
     || Buffer[ 0xE]!=0x4F
     || Buffer[ 0xF]!=0x4D
     || Buffer[0x10]!=0x06
     || Buffer[0x11]!=0x0E
     || Buffer[0x12]!=0x2B
     || Buffer[0x13]!=0x34
     || Buffer[0x14]!=0x01
     || Buffer[0x15]!=0x01
     || Buffer[0x16]!=0x01
     || Buffer[0x17]!=0xFF)
    {
        Reject("Aaf");
        return false;
    }

    //Element_Size
    if (Buffer_Size<File_Size)
        return false; //Must wait for more data

    //Accept the file
    Accept("Aaf");
    Fill(Stream_General, 0, General_Format, "AAF");

    Step=Step_None;
    ReferenceFiles=new File__ReferenceFilesHelper(this, Config);

    //All should be OK...
    return true;
}

//---------------------------------------------------------------------------
void File_Aaf::Read_Buffer_Continue()
{
    if (File_Offset || Buffer_Offset)
        return;

    //Parsing
    Element_Begin1("Header");
        int32u csectFat;
        int16u DllVersion, ByteOrder;
        Skip_B8(                                                "abSig");
        Skip_B16(                                               "clsid");
        Skip_L2(                                                "MinorVersion");
        Get_L2 (DllVersion,                                     "DllVersion");
        Get_L2 (ByteOrder,                                      "ByteOrder");
        Get_L2 (SectorShift,                                    "SectorShift");
        Get_L2 (MiniSectorShift,                                "MiniSectorShift");
        Skip_L2(                                                "Reserved");
        Skip_L4(                                                "Reserved");
        Skip_L4(                                                "csectDir");
        Get_L4 (csectFat,                                       "csectFat");
        Get_L4 (sectDirStart,                                   "sectDirStart");
        Skip_L4(                                                "signature");
        Get_L4 (MiniSectorCutoff,                               "MiniSectorCutoff");
        Get_L4 (sectMiniFatStart,                               "sectMiniFatStart");
        Skip_L4(                                                "csectMiniFat");
        Skip_L4(                                                "sectDifStart");
        Skip_L4(                                                "sectDif");
        Element_Begin1("sectFat");
            for (int16u Pos=0; Pos<(csectFat>109?109:csectFat); Pos++)
            {
                int32u sectFat;
                Get_L4 (sectFat,                                "sectFat");
                sectsFat.push_back(sectFat);
            }
            if (csectFat<109)
                Skip_XX((109-csectFat)*4,                       "unused sectsFat");
        Element_End();
    Element_End();

    FILLING_BEGIN();
        Fill("Aaf");
        Step=Step_Fat;
        sectsFat_Pos=0;
        if (sectsFat.empty())
        {
            Finish();
        }
        else
            GoTo((1+sectsFat[0])*(1<<SectorShift));
    FILLING_END();
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aaf::Header_Parse()
{
    switch (Step)
    {
        case Step_Fat           : Header_Fill_Code(0, "FAT");
                                  Header_Fill_Size(((int64u)1) << SectorShift);
                                  break;
        case Step_MiniFat       : Header_Fill_Code(0, "MiniFAT");
                                  Header_Fill_Size(((int64u)1) << SectorShift);
                                  break;
        case Step_Directory     : Header_Fill_Code(0, "Directory");
                                  Header_Fill_Size(((int64u)1) << SectorShift);
                                  break;
        case Step_Stream        : Header_Fill_Code(0, "Stream");
                                  Header_Fill_Size(((int64u)1) << (Streams[0]->Size<MiniSectorCutoff ? MiniSectorShift : SectorShift));
                                  break;
        default                 : ;
    }
}

//---------------------------------------------------------------------------
void File_Aaf::Data_Parse()
{
    //Parsing
    switch (Step)
    {
        case Step_Fat           : Fat(); return;
        case Step_MiniFat       : MiniFat(); break;
        case Step_Directory     : Directory(); break;
        case Step_Stream        : StreamElement(); return;
        default                 : Skip_XX(Element_Size,         "Unknown");
    }

    size_t Pointers_Pos=(size_t)((File_Offset+Buffer_Offset)>>SectorShift)-1;
    if (Pointers_Pos<Pointers.size())
    {
        if (Pointers[Pointers_Pos]<0xFFFFFFF0)
            GoTo((1+Pointers[Pointers_Pos])*(1<<SectorShift));
        else if (Step==Step_MiniFat)
        {
            Step=Step_Directory;
            Directory_Pos=0;
            GoTo((1+sectDirStart)*(1<<SectorShift));
        }
        else if (Step==Step_Directory)
        {
            Step=Step_Stream;
            if (Streams.empty())
                Finish();
            else
            {
                Streams_Pos=0;
                Streams_Pos2=0;
                GoTo(Streams[0]->StreamOffsets[0]);
            }
        }
        else
            Finish();
    }
    else
        Finish();
}

//---------------------------------------------------------------------------
void File_Aaf::Fat()
{
    //Parsing
    while (Element_Offset<Element_Size)
    {
        int32u Pointer;
        Get_L4 (Pointer,                                        "Pointer"); Param_Info1(Ztring::ToZtring(Pointers.size()));
        //Pointer=LittleEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
        //Element_Offset+=4;
        Pointers.push_back(Pointer);
    }

    //Next FAT sector or next step
    sectsFat_Pos++;
    if (sectsFat_Pos<sectsFat.size())
        GoTo((1+sectsFat[sectsFat_Pos])*(1<<SectorShift));
    else
    {
        Step=Step_MiniFat;
        GoTo((1+sectMiniFatStart)*(1<<SectorShift));
    }
}

//---------------------------------------------------------------------------
void File_Aaf::MiniFat()
{
    //Parsing
    while (Element_Offset<Element_Size)
    {
        int32u Pointer;
        Get_L4 (Pointer,                                        "Pointer"); Param_Info1(Ztring::ToZtring(MiniPointers.size()));
        //Pointer=LittleEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
        //Element_Offset+=4;
        MiniPointers.push_back(Pointer);
    }
}

//---------------------------------------------------------------------------
void File_Aaf::Directory()
{
    //Parsing
    while (Element_Offset<Element_Size)
        Directory_Entry();
}

//---------------------------------------------------------------------------
void File_Aaf::Directory_Entry()
{
    //Parsing
    Element_Begin1("Directory entry");
    Element_Info1(Directory_Pos);
    Ztring ab;
    int64u Size;
    int32u SectStart;
    int8u  mse;
    Get_UTF16L(64, ab,                                          "ab"); Element_Info1(ab);
    Skip_L2(                                                    "cb");
    Get_L1 (mse,                                                "mse"); Element_Info1(AAf_tagSTGTY(mse));
    Info_L1(flags,                                              "flags"); Element_Info1(AAf_tagDECOLOR(flags));
    Skip_L4(                                                    "LeftSib SID");
    Skip_L4(                                                    "RightSib SID");
    Skip_L4(                                                    mse==2?"0":"Child SID"); //Zero if stream
    Skip_L16(                                                   mse==2?"0":"clsId");//Zero if stream
    Skip_L4(                                                    "UserFlags");
    Info_L8(CreationTime,                                       mse==2?"0":"Create time"); if (mse!=2) {Param_Info1(CreationTime?Ztring().Date_From_Milliseconds_1601(CreationTime/10000):Ztring());} //Zero if stream
    Info_L8(ModificationTime,                                   mse==2?"0":"Modify time"); if (mse!=2) {Param_Info1(CreationTime?Ztring().Date_From_Milliseconds_1601(ModificationTime/10000):Ztring());} //Zero if stream
    Get_L4 (SectStart,                                          mse==1?"0":"SectStart"); //Zero if storage
    if (SectorShift<=9)
    {
        int32u Size32;
        Get_L4 (Size32,                                         mse==1?"0":"Size"); //Zero if storage
        Skip_L4(                                                "PropType");
        Size=Size32;

    }
    else
    {
        Get_L8 (Size,                                           mse==1?"0":"Size"); //Zero if storage
    }

    if (mse==5 && Size) //If root
    {
        //Building sectMiniFats_FatPointers
        int32u Pointers_Pos=SectStart;
        while (Pointers_Pos<Pointers.size())
        {
            Param_Info1(Ztring::ToZtring(Pointers_Pos<<SectorShift));
            sectsMiniStream.push_back(Pointers_Pos);
            Pointers_Pos=Pointers[Pointers_Pos];
        }
    }
    else if (mse==2 && Size) //If stream
    {
        Param_Info1("StreamOffset");
        stream* Stream = new stream(
                                    ab,
                                    Directory_Pos,
                                    Size
                                   );
        if (Size<MiniSectorCutoff) //MiniFAT
        {
            int32u Pointers_Pos=SectStart;
            while (Pointers_Pos<MiniPointers.size())
            {
                int32u SectPos=Pointers_Pos>>(SectorShift-MiniSectorShift);
                int32u MiniSectPos=Pointers_Pos&((((size_t)1)<<(SectorShift-MiniSectorShift))-1);
                Stream->StreamOffsets.push_back(((1+sectsMiniStream[SectPos])<<SectorShift)+(MiniSectPos<<MiniSectorShift));
                Param_Info1(Ztring::ToZtring(((1+sectsMiniStream[SectPos])<<SectorShift)+(MiniSectPos<<MiniSectorShift)));
                Pointers_Pos=MiniPointers[Pointers_Pos];
            }
        }
        else //FAT
        {
            int32u Pointers_Pos=SectStart;
            while (Pointers_Pos<Pointers.size())
            {
                Stream->StreamOffsets.push_back((1+Pointers_Pos)<<SectorShift);
                Param_Info1(Ztring::ToZtring((1+Pointers_Pos)<<SectorShift));
                Pointers_Pos=Pointers[Pointers_Pos];
            }
        }
        Streams.push_back(Stream);
    }
    Element_End0();

    Directory_Pos++;
}

//---------------------------------------------------------------------------
void File_Aaf::StreamElement()
{
    if (Streams_Pos>=Streams.size() || Streams[Streams_Pos]->Size>=0x1000000) //TODO: more serious test about size
        return; //Incoherancy

    //Saving data
    if (Streams[Streams_Pos]->StreamOffsets.size()!=1)
    {
        Skip_XX(Element_Size,                                    "Stream data");
        int16u Shift=(Streams[Streams_Pos]->Size<MiniSectorCutoff?MiniSectorShift:SectorShift);
        if (Streams[Streams_Pos]->Buffer==NULL)
            Streams[Streams_Pos]->Buffer=new int8u[(size_t)((1+(Streams[Streams_Pos]->Size>>Shift))<<Shift)];
        memcpy(Streams[Streams_Pos]->Buffer+Streams_Pos2*(((int64u)1)<<Shift), Buffer+Buffer_Offset, (size_t)Element_Size);
    }

    //Next Element
    Streams_Pos2++;
    if (Streams_Pos2>=Streams[Streams_Pos]->StreamOffsets.size())
    {
        Element_Offset=0;
        StreamElement_Parse();
        Streams_Pos++;
        Streams_Pos2=0;
    }
    if (Streams_Pos<Streams.size())
        GoTo(Streams[Streams_Pos]->StreamOffsets[Streams_Pos2]);
    else
        Finish();
}

//---------------------------------------------------------------------------
void File_Aaf::StreamElement_Parse()
{
    //Searching emulation_prevention_three_byte
    const int8u* Save_Buffer=Buffer;
    int64u Save_File_Offset=File_Offset;
    size_t Save_Buffer_Offset=Buffer_Offset;
    int64u Save_Element_Size=Element_Size;

    if (Streams[Streams_Pos]->Buffer)
    {
        //We must change the buffer for keeping out
        Element_Size=Streams[Streams_Pos]->Size;
        File_Offset=Streams[Streams_Pos]->StreamOffsets[0];
        Buffer_Offset=0;
        Buffer=Streams[Streams_Pos]->Buffer;
    }

    //Parsing
    Element_Info1(Streams[Streams_Pos]->Directory_Pos);
    Element_Info1(Streams[Streams_Pos]->Name);
    int16u Count;
    Skip_L2(                                                    "0x204C?");
    Get_L2 (Count,                                              "Count");
    vector<int16u> Sizes;
    vector<int16u> Keys;
    for (int16u Pos=0; Pos<Count; Pos++)
    {
        int16u Key, Size;
        Get_L2 (Key,                                            "Key");
        Skip_L2(                                                "Flags?");
        Get_L2 (Size,                                           "Size");
        Sizes.push_back(Size);
        Keys.push_back(Key);
    }

    #define ELEMENT(_ELEMENT, _NAME) \

    for (int16u Pos=0; Pos<Count; Pos++)
    {
        Element_Begin0();
        xxxSize=Sizes[Pos];
        switch (Keys[Pos])
        {
            case 0x0001 : Element_Name("MetaDictionary"); MetaDictionary(); break;
            case 0x0002 : Element_Name("Header"); Header(); break;
            case 0x0003 : Element_Name("ClassDefinitions"); ClassDefinitions(); break;
            case 0x0004 : Element_Name("TypeDefinitions"); TypeDefinitions(); break;
            case 0x0005 : Element_Name("Identification"); Identification(); break;
            case 0x0006 : Element_Name("Name"); Name(); break;
            case 0x0007 : Element_Name("MetaDefinition"); MetaDefinition(); break;
            case 0x0008 : Element_Name("ParentClass"); ParentClass(); break;
            case 0x0009 : Element_Name("Properties"); Properties(); break;
            case 0x000A : Element_Name("IsConcrete"); IsConcrete(); break;
            case 0x000B : Element_Name("Type"); Type(); break;
            case 0x000C : Element_Name("IsOptional"); IsOptional(); break;
            case 0x000D : Element_Name("LocalIdentification"); LocalIdentification(); break;
            case 0x000E : Element_Name("IsUniqueIdentifier"); IsUniqueIdentifier(); break;
            case 0x000F : Element_Name("Size"); Size(); break;
            case 0x3D02 : Element_Name("Locked"); Locked(); break;
            case 0x4001 : Element_Name("NetworkLocator"); NetworkLocator(); break;
            default     : Skip_XX(xxxSize,                      "Unknown");
        }
        Element_End0();
    }

    //
    if (Streams[Streams_Pos]->Buffer)
    {
        //We must change the buffer for keeping out
        Element_Size=Save_Element_Size;
        File_Offset=Save_File_Offset;
        Buffer_Offset=Save_Buffer_Offset;
        delete[] Buffer; Buffer=Save_Buffer;
        Element_Offset=Element_Size;
    }
}

//---------------------------------------------------------------------------
void File_Aaf::MetaDictionary()
{
    Skip_UTF16L(xxxSize,                                        "Data");
}

//---------------------------------------------------------------------------
void File_Aaf::Header()
{
    Skip_UTF16L(xxxSize,                                        "Data");
}

//---------------------------------------------------------------------------
void File_Aaf::ClassDefinitions()
{
    Skip_UTF16L(xxxSize,                                        "Data");
}

//---------------------------------------------------------------------------
void File_Aaf::TypeDefinitions()
{
    Skip_UTF16L(xxxSize,                                        "Data");
}

//---------------------------------------------------------------------------
void File_Aaf::Identification()
{
    Skip_B8(                                                    "Part2");
    Skip_B8(                                                    "Part1");
}

//---------------------------------------------------------------------------
void File_Aaf::Name()
{
    Skip_UTF16L(xxxSize,                                        "Data");
}

//---------------------------------------------------------------------------
void File_Aaf::MetaDefinition()
{
    Skip_UTF16L(xxxSize,                                        "Data");
}

//---------------------------------------------------------------------------
void File_Aaf::ParentClass()
{
    Skip_B5(                                                    "WeakReference");
    Skip_B8(                                                    "Part2");
    Skip_B8(                                                    "Part1");
}

//---------------------------------------------------------------------------
void File_Aaf::Properties()
{
    Skip_UTF16L(xxxSize,                                        "Data");
}

//---------------------------------------------------------------------------
void File_Aaf::IsConcrete()
{
    Skip_L1(                                                    "Data");
}

//---------------------------------------------------------------------------
void File_Aaf::Type()
{
    Skip_B8(                                                    "Part2");
    Skip_B8(                                                    "Part1");
}

//---------------------------------------------------------------------------
void File_Aaf::IsOptional()
{
    Skip_L1(                                                    "Data");
}

//---------------------------------------------------------------------------
void File_Aaf::LocalIdentification()
{
    Skip_L2(                                                    "Data");
}

//---------------------------------------------------------------------------
void File_Aaf::IsUniqueIdentifier()
{
    Skip_L1(                                                    "Data");
}

//---------------------------------------------------------------------------
void File_Aaf::Size()
{
    Skip_L1(                                                    "Data");
}

//---------------------------------------------------------------------------
void File_Aaf::Locked()
{
    Skip_L1(                                                    "Data");

    //Descriptors[Streams[Streams_Pos]->Directory_Pos].StreamKind=Stream_Audio;
}

//---------------------------------------------------------------------------
void File_Aaf::NetworkLocator()
{
    Ztring Data;
    Get_UTF16L(xxxSize, Data,                                   "Data");

    sequence* Sequence=new sequence;
    Sequence->AddFileName(Data);
    ReferenceFiles->AddSequence(Sequence);

    //Locators[Streams[Streams_Pos]->Directory_Pos].EssenceLocator=Data;
}

} //NameSpace

#endif //MEDIAINFO_AAF_YES
