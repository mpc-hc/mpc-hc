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
    return ReferenceFiles->Seek(Method, Value, ID);
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
    if (11>Buffer_Size)
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

    //Element_Size
    if (Buffer_Size<File_Size)
        return false; //Must wait for more data

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
    int32u Opaque2_Length, Audio_Count;
    int16u Opaque1_Length;
    Element_Begin1("Header");
        Skip_B1(                                                "Magic");
        Skip_Local(16,                                          "Magic");
        Skip_L2(                                                "0x0500");
        Skip_L1(                                                "Unknown [1]");
        Skip_L1(                                                "0x5A [1]");
        Skip_L2(                                                "0x0001");
        Skip_L2(                                                "0x0004");
        Skip_L2(                                                "0x0000 [1]");
        Skip_L4(                                                "Unknown [2]");
        Skip_L2(                                                "0x035A");
        Skip_L2(                                                "0x6400");
        Skip_L2(                                                "0x0000 [1]");
        Skip_L2(                                                "0x0300");
        Skip_L2(                                                "0x0000 [1]");
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
        Skip_L1(                                                "0x00 [1]");
        Get_L4 (Format_Length,                                  "Format length");
        Get_UTF8(Format_Length, Format,                         "Format");
        if (Format!=__T("Pro Tools Session File"))
        {
            Element_End();
            Reject();
            return;
        }
        Skip_L2(                                                "0x0006");
        Get_L4 (Platform_Length,                                "Platform length");
        Skip_UTF8(Platform_Length,                              "Platform");
        Skip_L4(                                                "0x00000000");
        Skip_L2(                                                "0x5A05");
        Get_L2 (Opaque1_Length,                                 "Info list, Opaque length"); //0x0006 (10.2-) or 0x0008 (10.3+)
        Skip_L4(                                                "Unknown [3]");
        Skip_L4(                                                "0x00002067");
        Skip_L2(                                                "0x0000 [1]");
        Skip_L2(                                                "0x0000 (once) or 0x002A");
        Skip_L2(                                                "0x0000 [1]");
        Skip_L2(                                                "Unknown [4]");
        Skip_L4(                                                "Unknown [5]");
        Skip_L4(                                                "Unknown [6]");
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
        Skip_L4(                                                "0x00000000 or 0x0000002A");
        Skip_L4(                                                "Unknown [7]");
        Skip_L4(                                                "Unknown [8]");
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
        Skip_L4(                                                "0x0000002A [1]");
        Skip_L4(                                                "Unknown [9]");
        Skip_L4(                                                "Unknown [10]");
        Skip_L4(                                                "0x00000000");
        Skip_L1(                                                "0x00 or 0x01 [2]");
        Skip_L1(                                                "0x01");
        Skip_L1(                                                "0x00 or 0x01 or 0x02");
        Skip_L1(                                                "0x00 [2]");
        Skip_L1(                                                "0x01");
        Skip_L1(                                                "0x00 (once) or 0x01");
        Skip_L1(                                                "0x00 or 0x01 (once)");
        Skip_L1(                                                "0x00 or 0x01 (once) or 0x5A");
        if (Opaque1_Length<6)
        {
            if (Opaque1_Length)
                Skip_XX(Opaque1_Length,                         "Opaque1");
        }
        else
        {
            Skip_L4(                                            "Opaque1 - Unknown [1]");
            Skip_L2(                                            "Opaque1 - Unknown [2]");
            if (Opaque1_Length<8)
            {
                if (Opaque1_Length-6)
                    Skip_XX(Opaque1_Length-6,                   "Opaque1 - Unknown [3]");
            }
            else
            {
                Skip_L2(                                        "Opaque1 - 0x0000");
                if (Opaque1_Length>8)
                    Skip_XX(Opaque1_Length-8,                   "Opaque1 - Unknown [3]");
            }
        }
        Skip_L2(                                                "0x2519");
        Skip_L2(                                                "0x0001");
        Skip_L4(                                                "0x00000000 or B5112287");
        Skip_L4(                                                "0x00000000 or 4037F9DC");
        Skip_L4(                                                "0x00000001 [1]");
        Skip_L2(                                                "0x0003");
    Element_End();
    Get_L4 (Audio_Count,                                        "Audio count");
    if (111*Audio_Count>Element_Size)
    {
        Reject();
        return;
    }
    Element_Begin1("Audio tracks list 1");
    for (int16u Pos=0; Pos<Audio_Count; Pos++)
    {
        Element_Begin1("Name");
        Skip_L2(                                                "0x0000 [New]");
        Get_L4 (Name_Length,                                    "(Same 1/2/3) Name length");
        Info_UTF8(Name_Length, Name,                            "(Same 1/2/3) Name");
        Skip_L2(                                                "(Same 1/2/3) 0x0000 ");
        Skip_L4(                                                "(Same 1/2/3) 0x00000000");
        Skip_L4(                                                "(Same 1/2/3) 0x0000002A");
        Skip_L4(                                                "(Same 1/2/3) Unknown");
        Skip_L4(                                                "(Same 1/2/3) Unknown");
        Info_L3(Number,                                         "(Same 1/2/3) Ordered number"); Element_Info1(Number);
        Element_Info1(Name);
        Element_End();

        if (Name==__T("Lf")) //Exception? Typo?
            Name=__T("Lfe");
        Name.MakeLowerCase();
        Names.push_back(Name);
    }
    Element_End();
    Element_Begin1("Audio tracks list 2");
    for (int16u Pos=0; Pos<Audio_Count; Pos++)
    {
        Element_Begin1("Name");
        int32u Size;
        Skip_L3(                                                "(Same   2/3) 0x00025A [1]");
        Get_L4 (Size,                                           "(Same   2/3) Size");
        Skip_L4(                                                "(Same   2/3) 0x0000251A");
        Get_L4 (Name_Length,                                    "(Same 1/2/3) Name length");
        Info_UTF8(Name_Length, Name,                            "(Same 1/2/3) Name");
        Skip_L2(                                                "(Same 1/2/3) 0x0000 ");
        Skip_L4(                                                "(Same 1/2/3) 0x00000000");
        Skip_L4(                                                "(Same 1/2/3) 0x0000002A");
        Skip_L4(                                                "(Same 1/2/3) Unknown");
        Skip_L4(                                                "(Same 1/2/3) Unknown");
        Info_L3(Number,                                         "(Same 1/2/3) Ordered number"); Element_Info1(Number);
        Skip_L2(                                                "(Same   2/3) 0x0000");
        Element_Info1(Name);
        if (Name_Length+31!=Size)
        {
            Element_End();
            Element_End();
            Reject();
            return;
        }
        Element_End();
    }
    Element_End();
    Get_L4 (Audio_Count,                                        "Audio count");
    if (4*Audio_Count>Element_Size)
    {
        Reject();
        return;
    }
    Element_Begin1("Audio tracks list 3");
    for (int16u Pos=0; Pos<Audio_Count; Pos++)
    {
        Element_Begin1("Name");
        int32u Size;
        Skip_L3(                                                "(Same   2/3) 0x00025A [2]");
        Get_L4 (Size,                                           "(Same   2/3) Size");
        if (Size>0x10000)
        {
            Element_End();
            Element_End();
            Reject();
            return;
        }
        Skip_L4(                                                "(Same   2/3) 0x0000251A");
        Get_L4 (Name_Length,                                    "(Same 1/2/3) Name length");
        Info_UTF8(Name_Length, Name,                            "(Same 1/2/3) Name");
        Skip_L2(                                                "(Same 1/2/3) 0x0000 ");
        Skip_L4(                                                "(Same 1/2/3) 0x00000000");
        Skip_L4(                                                "(Same 1/2/3) 0x0000002A");
        Skip_L4(                                                "(Same 1/2/3) Unknown");
        Skip_L4(                                                "(Same 1/2/3) Unknown");
        Info_L3(Number,                                         "(Same 1/2/3) Ordered number"); Element_Info1(Number);
        Skip_L2(                                                "(Same   2/3) 0x0000");
        Element_Info1(Name);
        if (Name_Length+31!=Size)
        {
            Element_End();
            Element_End();
            Reject();
            return;
        }
        Element_End();
    }
    Element_End();
    Skip_L2(                                                    "0x0000 [4]");
    Skip_L2(                                                    "0x0018");
    Skip_L4(                                                    "0x00000001 [2]");
    Skip_L2(                                                    "0x0018");
    Skip_L4(                                                    "0x00000001 [2]");
    Skip_L2(                                                    "0x0001 [3]");
    Skip_L3(                                                    "0x00095A");
    Get_L4 (Opaque2_Length,                                     "Opaque2 length");
    Skip_XX(Opaque2_Length,                                     "Opaque2");
    Skip_L1(                                                    "0x5A [2]");
    Skip_L2(                                                    "0x0003 (10.0) or 0x0004 (10.2+)");
    Get_L4 (Opaque2_Length,                                     "Opaque3 length"); //0x0012 (10.0) or 0x0016 (10.2+)
    if (Opaque2_Length<0x12)
        Skip_XX(Opaque2_Length,                                 "Opaque3");
    else
    {
        Skip_L4(                                                "Opaque3 - 0x06002026");
        Skip_L4(                                                "Opaque3 - 0x00000000 [1]");
        Skip_L2(                                                "Opaque3 - 0x0000");
        Skip_L4(                                                "Opaque3 - Unknown [1]");
        Skip_L4(                                                "Opaque3 - Unknown [2]");
        if (Opaque2_Length<0x16)
        {
            if (Opaque2_Length-0x12)
                Skip_XX(Opaque2_Length-0x12,                    "Opaque3 - Unknown [3]");
        }
        else
        {
            Skip_L4(                                            "Opaque3 - 0x00000000 [2]");
            if (Opaque2_Length>0x16)
                Skip_XX(Opaque2_Length-0x16,                    "Opaque3 - Unknown  [4]");
        }
    }
    Skip_L3(                                                    "0x00025A [3]");
    Get_L4 (Opaque2_Length,                                     "0x00000015 (Opaque4 length?) or something else");
    if (Opaque2_Length==0x00000015)
    {
        Skip_L4(                                                "0x075A2032");
        Skip_L4(                                                "0x00000C00");
        Skip_L4(                                                "0x01204200");
        Skip_L4(                                                "0x00000000 or 0x01000000");
        Skip_L4(                                                "Unknown [13]");
        Skip_L1(                                                "0x00 [3]");
        Skip_L3(                                                "0x00025A [4]");
        Skip_L4(                                                "Unknown [14]");
    }
    Skip_L4(                                                    "Unknown [15]");
    Skip_L4(                                                    "0x015A0000");
    Skip_L4(                                                    "Unknown [16]");
    Skip_L4(                                                    "Unknown [17]");
    Skip_L4(                                                    "0x01000000");
    Get_L4 (FileName_Count,                                     "File name count");
    if (13*FileName_Count>Element_Size)
    {
        Reject();
        return;
    }
    Get_L4 (Directory_Length,                                   "Directory length");
    Get_UTF8(Directory_Length, Directory,                       "Directory");
    Skip_L4(                                                    "0x00000000 [11]");
    Element_Begin1("File names");
    vector<int8u> Roles;
    vector<Ztring> FileNames;
    vector<Ztring> FileNamesLowerCase;
    vector<int32u> Purposes;
    for (int32u Pos=0; Pos<FileName_Count; Pos++)
    {
        Ztring FileName;
        int32u FileName_Length, Purpose;
        int8u  Role; //
        Element_Begin1("File names");
        Get_L1 (Role,                                           "role? (0x02 for WAV files)");
        Skip_L4(                                                "Ordered number except WAV files and -1");
        Get_L4 (FileName_Length,                                "File Name length");
        Get_UTF8(FileName_Length, FileName,                     "File Name"); Element_Name(FileName);
        Get_C4 (Purpose,                                        "Purpose (e.g. EVAW for .wav files)"); //Found 1 .wav file without "EWAV".
        Element_End();

        Roles.push_back(Role);
        FileNames.push_back(FileName);
        FileName.MakeLowerCase();
        FileNamesLowerCase.push_back(FileName);
        Purposes.push_back(Purpose);
    }
    Element_End();
    Skip_XX(Element_Size-Element_Offset,                        "Unknown");

    FILLING_BEGIN();
        Accept("Ptx"); //Could be Ptf (former format but not supported, so we don't care currently)
        Fill("Ptx");
        Fill(Stream_General, 0, General_Format, "Pro Tools Session");
        Fill(Stream_General, 0, General_Format_Version, "Version 10");
        Fill(Stream_General, 0, General_Encoded_Library_Name, LibraryName);
        Fill(Stream_General, 0, General_Encoded_Library_Version, LibraryVersion);

        // Role==2 + Purpose==EWAV + listed
        if (Names.size()>1 || FileNames.size()==1)
        {
            size_t Pos_Offset=0;
            for (int32u Pos=0; Pos<FileName_Count; Pos++)
            {
                if (Roles[Pos]==0x02
                 && Purposes[Pos]==0x45564157 //"EWAV"
                 && Pos-Pos_Offset<Names.size()
                 && FileNames[Pos]!=__T("1 kHz @ -20dB.wav") //Exception?
                 && FileNames[Pos]!=__T("1K@-20db.wav") //Exception?
                 && FileNames[Pos]!=__T("1K@0VU-20REF.wav") //Exception?
                 && FileNames[Pos]!=__T("1k@0vu -20.wav") //Exception?
                 && FileNames[Pos]!=__T("1Khz@-20dB.wav") //Exception?
                 && FileNames[Pos].find(__T(".1Khz.wav"))==string::npos //Exception?
                 && FileNames[Pos].find(__T("_1KTONE_"))==string::npos //Exception?
                 && FileNamesLowerCase[Pos].find(Names[Pos-Pos_Offset]+__T(".wav"))!=string::npos
                 && FileNamesLowerCase[Pos].find(Names[Pos-Pos_Offset]+__T(".wav"))+Names[Pos-Pos_Offset].size()+4==FileNames[Pos].size())
                {
                    sequence* Sequence=new sequence;
                    Sequence->StreamKind=Stream_Audio;
                    Sequence->AddFileName(Directory+PathSeparator+FileNames[Pos]);
                    ReferenceFiles->AddSequence(Sequence);
                }
                else if (!ReferenceFiles->Sequences_Size())
                    Pos_Offset++;
            }

            if (Names.size()!=ReferenceFiles->Sequences_Size())
                ReferenceFiles->Clear(); //Failed to detect correctly
        }

        // Role==2 + listed
        if (!ReferenceFiles->Sequences_Size() && (Names.size()>1 || FileNames.size()==1))
        {
            size_t Pos_Offset=0;
            for (int32u Pos=0; Pos<FileName_Count; Pos++)
            {
                if (Roles[Pos]==0x02
                    && Pos-Pos_Offset<Names.size()
                 && FileNames[Pos]!=__T("1 kHz @ -20dB.wav") //Exception?
                 && FileNames[Pos]!=__T("1K@-20db.wav") //Exception?
                 && FileNames[Pos]!=__T("1K@0VU-20REF.wav") //Exception?
                 && FileNames[Pos]!=__T("1k@0vu -20.wav") //Exception?
                 && FileNames[Pos]!=__T("1Khz@-20dB.wav") //Exception?
                 && FileNames[Pos].find(__T(".1Khz.wav"))==string::npos //Exception?
                 && FileNames[Pos].find(__T("_1KTONE_"))==string::npos) //Exception?
                {
                     Ztring FileName=FileNames[Pos];
                     Ztring Name=Names[Pos-Pos_Offset];
                     FileName.MakeLowerCase();
                     Name.MakeLowerCase();
                     if (FileName.find(Name)==0
                        || FileName.find(Name+__T(".wav"))+5==Name.size())
                    {
                        sequence* Sequence=new sequence;
                        Sequence->StreamKind=Stream_Audio;
                        Sequence->AddFileName(Directory+PathSeparator+FileNames[Pos]);
                        ReferenceFiles->AddSequence(Sequence);
                    }
                    else if (!ReferenceFiles->Sequences_Size())
                        Pos_Offset++;
                }
                else if (!ReferenceFiles->Sequences_Size())
                    Pos_Offset++;
            }

            if (Names.size()!=ReferenceFiles->Sequences_Size())
                ReferenceFiles->Clear(); //Failed to detect correctly
        }

        // Role==2 + Purpose==EWAV + listed, special case with specific file names
        if (!ReferenceFiles->Sequences_Size() && (Names.size()>1 || FileNames.size()==1))
        {
            for (int32u Pos=0; Pos<FileName_Count; Pos++)
            {
                if (Roles[Pos]==0x02
                 && Purposes[Pos]==0x45564157 //"EWAV"
                 && FileNames[Pos]!=__T("1 kHz @ -20dB.wav") //Exception?
                 && FileNames[Pos]!=__T("1K@-20db.wav") //Exception?
                 && FileNames[Pos]!=__T("1K@0VU-20REF.wav") //Exception?
                 && FileNames[Pos]!=__T("1k@0vu -20.wav") //Exception?
                 && FileNames[Pos]!=__T("1Khz@-20dB.wav") //Exception?
                 && FileNames[Pos].find(__T(".1Khz.wav"))==string::npos //Exception?
                 && FileNames[Pos].find(__T("_1KTONE_"))==string::npos) //Exception?
                {
                    for (int32u Pos2=0; Pos2<Names.size(); Pos2++)
                        if (FileNamesLowerCase[Pos].find(Names[Pos2])==0)
                        {
                            sequence* Sequence=new sequence;
                            Sequence->StreamKind=Stream_Audio;
                            Sequence->AddFileName(Directory+PathSeparator+FileNames[Pos]);
                            ReferenceFiles->AddSequence(Sequence);
                            break;
                        }
                }
            }
        }

        // Role==2 + Purpose==EWAV
        if (!ReferenceFiles->Sequences_Size())
        {
            for (int32u Pos=0; Pos<FileName_Count; Pos++)
            {
                if (Roles[Pos]==0x02
                 && Purposes[Pos]==0x45564157 //"EWAV"
                 && FileNames[Pos]!=__T("1 kHz @ -20dB.wav") //Exception?
                 && FileNames[Pos]!=__T("1K@-20db.wav") //Exception?
                 && FileNames[Pos]!=__T("1K@0VU-20REF.wav") //Exception?
                 && FileNames[Pos]!=__T("1k@0vu -20.wav") //Exception?
                 && FileNames[Pos]!=__T("1Khz@-20dB.wav") //Exception?
                 && FileNames[Pos].find(__T(".1Khz.wav"))==string::npos //Exception?
                 && FileNames[Pos].find(__T("_1KTONE_"))==string::npos) //Exception?
                {
                    sequence* Sequence=new sequence;
                    Sequence->StreamKind=Stream_Audio;
                    Sequence->AddFileName(Directory+PathSeparator+FileNames[Pos]);
                    ReferenceFiles->AddSequence(Sequence);
                }
            }
        }

        // Role==2
        if (!ReferenceFiles->Sequences_Size())
        {
            for (int32u Pos=0; Pos<FileName_Count; Pos++)
            {
                if (Roles[Pos]==0x02
                 && FileNames[Pos]!=__T("1 kHz @ -20dB.wav") //Exception?
                 && FileNames[Pos]!=__T("1K@-20db.wav") //Exception?
                 && FileNames[Pos]!=__T("1K@0VU-20REF.wav") //Exception?
                 && FileNames[Pos]!=__T("1k@0vu -20.wav") //Exception?
                 && FileNames[Pos]!=__T("1Khz@-20dB.wav") //Exception?
                 && FileNames[Pos].find(__T(".1Khz.wav"))==string::npos //Exception?
                 && FileNames[Pos].find(__T("_1KTONE_"))==string::npos) //Exception?
                {
                    sequence* Sequence=new sequence;
                    Sequence->StreamKind=Stream_Audio;
                    Sequence->AddFileName(Directory+PathSeparator+FileNames[Pos]);
                    ReferenceFiles->AddSequence(Sequence);
                }
            }
        }
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_PTX_YES
