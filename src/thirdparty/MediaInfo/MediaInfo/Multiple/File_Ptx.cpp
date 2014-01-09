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
#if defined(MEDIAINFO_PTX_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Ptx.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
using namespace tinyxml2;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Ptx::File_Ptx()
:File__Analyze()
{
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Ptx;
        StreamIDs_Width[0]=sizeof(size_t);
    #endif //MEDIAINFO_EVENTS

    //Temp
    ReferenceFiles=NULL;
}

//---------------------------------------------------------------------------
File_Ptx::~File_Ptx()
{
    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ptx::Streams_Finish()
{
    ReferenceFiles->ParseReferences();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_Ptx::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    return ReferenceFiles->Read_Buffer_Seek(Method, Value, ID);
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Ptx::FileHeader_Begin()
{
    if (File_Size<0x100)
    {
        Reject("Ptx");
        return false;
    }

    //Element_Size
    if (Buffer_Size<File_Size)
        return false; //Must wait for more data

    if (Buffer[ 0x0]!=0x03
     || Buffer[ 0x1]!=0x30
     || Buffer[ 0x2]!=0x30
     || Buffer[ 0x3]!=0x31
     || Buffer[ 0x4]!=0x30
     || Buffer[ 0x5]!=0x31
     || Buffer[ 0x6]!=0x31
     || Buffer[ 0x7]!=0x31
     || Buffer[ 0x8]!=0x31
     || Buffer[ 0x9]!=0x30
     || Buffer[ 0xA]!=0x30
     || Buffer[ 0xB]!=0x31
     || Buffer[ 0xC]!=0x30
     || Buffer[ 0xD]!=0x31
     || Buffer[ 0xE]!=0x30
     || Buffer[ 0xF]!=0x31
     || Buffer[0x10]!=0x31)
    {
        Reject("Ptx");
        return false;
    }

    ReferenceFiles=new File__ReferenceFilesHelper(this, Config);

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ptx::Read_Buffer_Continue()
{
    if (File_Offset || Buffer_Offset)
    {
        if (Buffer_Size)
            Reject(); //Problem
        return;
    }

    //Parsing
    ZtringList Names;
    Ztring LibraryName, LibraryVersion, Format, Directory;
    int32u LibraryName_Length, LibraryVersion_Length, LibraryRelease_Length, Format_Length, Platform_Length, Info_Count, Names_Count, Info_Length, Name_Length, FileName_Count, Directory_Length;
    int32u Unknown_Length;
    int16u Audio_Count;
    Element_Begin1("Header");
        Skip_B1(                                                "Magic");
        Skip_Local(16,                                          "Magic");
        Skip_L2(                                                "0x0500");
        Skip_L1(                                                "Unknown");
        Skip_L1(                                                "0x5A");
        Skip_L2(                                                "0x0001");
        Skip_L2(                                                "0x0004");
        Skip_L2(                                                "0x0000");
        Skip_L4(                                                "Unknown");
        Skip_L2(                                                "0x035A");
        Skip_L2(                                                "0x6400");
        Skip_L2(                                                "0x0000");
        Skip_L2(                                                "0x0300");
        Skip_L2(                                                "0x0000");
        Get_L4 (LibraryName_Length,                             "WritingLibrary name length");
        Get_UTF8(LibraryName_Length, LibraryName,               "Library name");
        Skip_L4(                                                "0x00000003");
        Skip_L4(                                                "Library version, major");
        Skip_L4(                                                "Library version, minor");
        Skip_L4(                                                "Library version, revision");
        Get_L4 (LibraryVersion_Length,                          "Library version length");
        Get_UTF8(LibraryVersion_Length, LibraryVersion,         "Library version");
        Skip_L1(                                                "0x01");
        Get_L4 (LibraryRelease_Length,                          "Library release length");
        Skip_UTF8(LibraryRelease_Length,                        "Library release");
        Skip_L1(                                                "0x00");
        Get_L4 (Format_Length,                                  "Format length");
        Get_UTF8(Format_Length, Format,                         "Format");
        if (Format!=__T("Pro Tools Session File"))
        {
            Element_End();
            Reject("Ptx");
            return;
        }
        Skip_L2(                                                "0x0006");
        Get_L4 (Platform_Length,                                "Platform length");
        Skip_UTF8(Platform_Length,                              "Platform");
        Skip_L4(                                                "0x00000000");
        Skip_L4(                                                "0x00085A05");
        Skip_L4(                                                "Unknown");
        Skip_L4(                                                "0x00002067");
        Skip_L4(                                                "0x002A0000");
        Skip_L4(                                                "Unknown");
        Skip_L4(                                                "Unknown");
        Skip_L4(                                                "Unknown");
    Element_End();
    Element_Begin1("Info list");
        Get_L4 (Info_Count,                                     "Info count");
        if (4*Info_Count>Element_Size)
        {
            Element_End();
            Reject();
            return;
        }
        for (int32u Pos=0; Pos<Info_Count; Pos++)
        {
            Element_Begin1("Info");
            Get_L4 (Info_Length,                                "Info length");
            if (Info_Length)
            {
                Info_UTF8(Info_Length, Info,                    "Name"); Element_Info1(Info);
            }
            Element_End();
        }
    Element_End();
    Element_Begin1("Unknown");
        Skip_L4(                                                "0x00000000");
        Element_Begin1("Names list 1");
        Get_L4 (Names_Count,                                    "Names count minus 1");
        if (4*Names_Count>Element_Size)
        {
            Element_End();
            Reject();
            return;
        }
        for (int16u Pos=0; Pos<1+Names_Count; Pos++)
        {
            Element_Begin1("Name");
            Get_L4 (Name_Length,                                "Name length");
            if (Name_Length)
            {
                Info_UTF8(Name_Length, Name,                    "Name"); Element_Name(Name);
            }
            Element_End();
        }
        Element_End();
        Skip_L4(                                                "0x00000000");
        Skip_L4(                                                "0x0000002A");
        Skip_L4(                                                "Unknown");
        Skip_L4(                                                "Unknown");
        Element_Begin1("Names list 2");
        Get_L4 (Names_Count,                                    "Names count");
        if (4*Names_Count>Element_Size)
        {
            Element_End();
            Reject();
            return;
        }
        for (int16u Pos=0; Pos<Names_Count; Pos++)
        {
            Element_Begin1("Name");
            Get_L4 (Name_Length,                                "Name length");
            if (Name_Length)
            {
                Info_UTF8(Name_Length, Name,                    "Name"); Element_Name(Name);
            }
            Element_End();
        }
        Element_End();
        Skip_L4(                                                "0x00000000");
        Skip_L4(                                                "0x0000002A");
        Skip_L4(                                                "Unknown");
        Skip_L4(                                                "Unknown");
        Skip_L4(                                                "0x00000000");
        Skip_L4(                                                "Unknown");
        Skip_L4(                                                "0x00000101");
        Skip_L4(                                                "0x00055A00");
        Skip_L4(                                                "Unknown");
        Skip_L4(                                                "0x00012519");
        Skip_L4(                                                "0x00000000");
        Skip_L4(                                                "0x00000000");
        Skip_L4(                                                "0x00000001");
        Skip_L2(                                                "0x0003");
    Element_End();
    Get_L2 (Audio_Count,                                        "Audio count");
    if (111*Audio_Count>Element_Size)
    {
        Reject();
        return;
    }
    Element_Begin1("Audio tracks list 1");
    for (int16u Pos=0; Pos<Audio_Count; Pos++)
    {
        Element_Begin1("Name");
        Skip_L4(                                                "0x00000000");
        Get_L4 (Unknown_Length,                                 "Name length");
        Info_UTF8(Unknown_Length, Name,                         "Name");
        Skip_L2(                                                "0x0000");
        Skip_L4(                                                "0x00000000");
        Skip_L4(                                                "0x0000002A");
        Skip_L4(                                                "Unknown (same 1/2/3)");
        Skip_L4(                                                "Unknown (same 1/2/3)");
        Info_L1(Number,                                         "Ordered number"); Element_Info1(Number);
        Element_Info1(Name);
        Element_End();

        Names.push_back(Name);
    }
    Element_End();
    Skip_L2(                                                    "0x0000");
    Element_Begin1("Audio tracks list 2");
    for (int16u Pos=0; Pos<Audio_Count; Pos++)
    {
        Element_Begin1("Name");
        int32u Size;
        Skip_L3(                                                "0x00025A");
        Get_L4 (Size,                                           "Size");
        Skip_L4(                                                "0x0000251A");
        Get_L4 (Unknown_Length,                                 "Name length");
        Info_UTF8(Unknown_Length, Name,                         "Name");
        Skip_L2(                                                "0x0000");
        Skip_L4(                                                "0x00000000");
        Skip_L4(                                                "0x0000002A");
        Skip_L4(                                                "Unknown (same 1/2/3)");
        Skip_L4(                                                "Unknown (same 1/2/3)");
        Info_L1(Number,                                         "Ordered number"); Element_Info1(Number);
        Skip_L4(                                                "0x00000000");
        Element_Info1(Name);
        if (Unknown_Length+31!=Size)
        {
            Reject();
            return;
        }
        Element_End();
    }
    Element_End();
    Get_L2 (Audio_Count,                                        "Audio count");
    if (4*Audio_Count>Element_Size)
    {
        Reject();
        return;
    }
    Skip_L2(                                                    "0x00");
    Element_Begin1("Audio tracks list 3");
    for (int16u Pos=0; Pos<Audio_Count; Pos++)
    {
        Element_Begin1("Name");
        int32u Size;
        Skip_L3(                                                "0x00025A");
        Get_L4 (Size,                                           "Size");
        Skip_L4(                                                "0x0000251A");
        Get_L4 (Unknown_Length,                                 "Name length");
        Info_UTF8(Unknown_Length, Name,                         "Name");
        Skip_L2(                                                "0x0000");
        Skip_L4(                                                "0x00000000");
        Skip_L4(                                                "0x0000002A");
        Skip_L4(                                                "Unknown (same 1/2/3)");
        Skip_L4(                                                "Unknown (same 1/2/3)");
        Info_L1(Number,                                         "Ordered number"); Element_Info1(Number);
        Skip_L4(                                                "0x00000000");
        Element_Info1(Name);
        if (Unknown_Length+31!=Size)
        {
            Reject();
            return;
        }
        Element_End();
    }
    Element_End();
    Skip_L2(                                                    "0x0000");
    Skip_L2(                                                    "0x0018");
    Skip_L4(                                                    "0x00000001");
    Skip_L2(                                                    "0x0018");
    Skip_L4(                                                    "0x00000001");
    Skip_L2(                                                    "0x0001");
    Skip_L3(                                                    "0x00095A");
    Get_L4 (Unknown_Length,                                     "Opaque length");
    Skip_XX(Unknown_Length,                                     "Opaque");
    Skip_L3(                                                    "0x00045A");
    Skip_L4(                                                    "0x00000016");
    Skip_L4(                                                    "0x06002026");
    Skip_L4(                                                    "0x00000000");
    Skip_L2(                                                    "0x0000");
    Skip_L4(                                                    "Unknown");
    Skip_L4(                                                    "Unknown");
    Skip_L4(                                                    "0x00000000");
    Skip_L3(                                                    "0x00025A");
    Skip_L4(                                                    "0x00000015");
    Skip_L4(                                                    "0x075A2032");
    Skip_L4(                                                    "0x00000C00");
    Skip_L4(                                                    "0x01204200");
    Skip_L4(                                                    "0x01000000");
    Skip_L4(                                                    "Unknown");
    Skip_L4(                                                    "0x00025A00");
    Skip_L4(                                                    "Unknown");
    Skip_L4(                                                    "Unknown");
    Skip_L4(                                                    "0x015A0000");
    Skip_L4(                                                    "Unknown");
    Skip_L4(                                                    "Unknown");
    Skip_L4(                                                    "0x01000000");
    Get_L4 (FileName_Count,                                     "File name count");
    if (13*FileName_Count>Element_Size)
    {
        Reject();
        return;
    }
    Get_L4 (Directory_Length,                                   "Directory length");
    Get_UTF8(Directory_Length, Directory,                       "Directory");
    Skip_L4(                                                    "0x00000000");
    Element_Begin1("File names");
    size_t Pos_Offset=0;
    for (int32u Pos=0; Pos<FileName_Count; Pos++)
    {
        Ztring Name;
        int32u Name_Length, Purpose;
        Element_Begin1("File names");
        Skip_L1(                                                "0x0002");
        Skip_L4(                                                "Ordered number except WAV files and -1");
        Get_L4 (Name_Length,                                    "Name length");
        Get_UTF8(Name_Length, Name,                             "Name"); Element_Name(Name);
        Get_C4 (Purpose,                                        "Purpose (e.g. EVAW for .wav files)");
        Element_End();

        switch (Purpose)
        {
            case 0x45564157:
                            if (Pos-Pos_Offset<Names.size()
                             && (Name.find(Names[Pos-Pos_Offset])==0
                              || Name.find(Names[Pos-Pos_Offset]+__T(".wav"))+5==Name.size()))
                            {
                                File__ReferenceFilesHelper::reference ReferenceFile;
                                ReferenceFile.StreamKind=Stream_Audio;
                                ReferenceFile.FileNames.push_back(Directory+PathSeparator+Name);
                                ReferenceFiles->References.push_back(ReferenceFile);
                            }
                            else if (ReferenceFiles->References.empty())
                                Pos_Offset++;
            default:        ;
        }

    }
    Element_End();
    Skip_XX(Element_Size-Element_Offset,                        "Unknown");

    FILLING_BEGIN();
        Accept("Ptx"); //Could be Ptf (former formatn but not supported, so we don't care currently
        Fill("Ptx");
        Fill(Stream_General, 0, General_Format, "Pro Tools Session");
        Fill(Stream_General, 0, General_Format_Version, "Version 10");
        Fill(Stream_General, 0, General_Encoded_Library_Name, LibraryName);
        Fill(Stream_General, 0, General_Encoded_Library_Version, LibraryVersion);
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_PTX_YES
