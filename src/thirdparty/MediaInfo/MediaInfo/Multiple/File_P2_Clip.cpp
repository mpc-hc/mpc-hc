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
#if defined(MEDIAINFO_P2_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_P2_Clip.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "ZenLib/Dir.h"
#include "ZenLib/FileName.h"
#include "tinyxml2.h"
using namespace tinyxml2;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_P2_Clip::File_P2_Clip()
:File__Analyze()
{
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_None; //TODO
        StreamIDs_Width[0]=sizeof(size_t)*2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_EventWasSent_Accept_Specific=true;
    #endif //MEDIAINFO_DEMUX

    //Temp
    ReferenceFiles=NULL;
}

//---------------------------------------------------------------------------
File_P2_Clip::~File_P2_Clip()
{
    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_P2_Clip::Streams_Finish()
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
size_t File_P2_Clip::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    if (ReferenceFiles==NULL)
        return 0;

    return ReferenceFiles->Read_Buffer_Seek(Method, Value, ID);
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_P2_Clip::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    {
        XMLElement* Root=document.FirstChildElement("P2Main");
        if (Root)
        {
            Accept("P2_Clip");
            Fill(Stream_General, 0, General_Format, "P2 Clip");

            ReferenceFiles=new File__ReferenceFilesHelper(this, Config);

            XMLElement* ClipContent=Root->FirstChildElement("ClipContent");
            if (ClipContent)
            {
                XMLElement* ChildElement;

                //ID
                ChildElement=ClipContent->FirstChildElement("GlobalClipID");
                if (ChildElement)
                    Fill(Stream_General, 0, General_UniqueID, ChildElement->GetText());

                //Duration
                Ztring Duration, EditUnit;
                ChildElement=ClipContent->FirstChildElement("Duration");
                if (ChildElement)
                    Duration=ChildElement->GetText();
                ChildElement=ClipContent->FirstChildElement("EditUnit");
                if (ChildElement)
                    EditUnit=ChildElement->GetText();
                int64u Duration_Frames=Duration.To_int64u();
                int64u EditUnit_Numerator=EditUnit.SubString(Ztring(), __T("/")).To_int64u();
                int64u EditUnit_Denominator=EditUnit.SubString(__T("/"), Ztring()).To_int64u();
                if (Duration_Frames && EditUnit_Numerator && EditUnit_Denominator)
                    Fill(Stream_General, 0, General_Duration, ((float32)Duration_Frames)*1000*EditUnit_Numerator/EditUnit_Denominator, 0);

                //EssenceList
                XMLElement* EssenceList=ClipContent->FirstChildElement("EssenceList");
                if (EssenceList)
                {
                    XMLElement* Track=EssenceList->FirstChildElement();
                    #if defined(MEDIAINFO_MXF_YES)
                    size_t Audio_Count=0;
                    #endif
                    while (Track)
                    {
                        string Field=Track->Value();
                        if (Field=="Video")
                        {
                            File__ReferenceFilesHelper::reference ReferenceFile;

                            //FrameRate
                            ChildElement=Track->FirstChildElement("FrameRate");
                            if (ChildElement)
                            {
                                Ztring FrameRateS=Ztring(ChildElement->GetText());
                                        if (FrameRateS.find(__T("23.97"))==0)
                                    ReferenceFile.FrameRate=((float64)24)*1000/1001;
                                else if (FrameRateS.find(__T("29.97"))==0)
                                    ReferenceFile.FrameRate=((float64)30)*1000/1001;
                                else if (FrameRateS.find(__T("59.94"))==0)
                                    ReferenceFile.FrameRate=((float64)60)*1000/1001;
                                else
                                    ReferenceFile.FrameRate=FrameRateS.To_float64();
                                if (FrameRateS.find('i')!=string::npos)
                                    ReferenceFile.FrameRate/=2;
                            }

                            //CreationDate
                            ChildElement=Track->FirstChildElement("StartTimecode");
                            if (ChildElement)
                            {
                                string Text=ChildElement->GetText();
                                if (Text.size()==11)
                                {
                                    int64u ToFill=(Text[0]-'0')*10*60*60*1000
                                                + (Text[1]-'0')   *60*60*1000
                                                + (Text[3]-'0')   *10*60*1000
                                                + (Text[4]-'0')      *60*1000
                                                + (Text[6]-'0')      *10*1000
                                                + (Text[7]-'0')         *1000;
                                        if (ReferenceFile.FrameRate)
                                            ToFill+=float64_int64s(((Text[9]-'0')*10+(Text[10]-'0'))*1000/ReferenceFile.FrameRate);
                                    //Fill(Stream_Video, StreamPos_Last, Video_Delay, ToFill);
                                    //Fill(Stream_Video, StreamPos_Last, Video_Delay_Source, "P2 Clip");
                                }
                            }

                           #if defined(MEDIAINFO_MXF_YES)
                                if (File_Name.size()>10+1+4+1
                                 && File_Name[File_Name.size()-10-1]==PathSeparator
                                 && (File_Name[File_Name.size()-10-2]&(~0x20))==__T('P')
                                 && (File_Name[File_Name.size()-10-3]&(~0x20))==__T('I')
                                 && (File_Name[File_Name.size()-10-4]&(~0x20))==__T('L')
                                 && (File_Name[File_Name.size()-10-5]&(~0x20))==__T('C')
                                 && File_Name[File_Name.size()-10-6]==PathSeparator)
                                {
                                    Ztring file=File_Name.substr(File_Name.size()-10, 6);
                                    Ztring MXF_File=__T("..");
                                    MXF_File+=PathSeparator;

                                    Ztring Path=File_Name.substr(0, File_Name.size()-10-6);
                                    ZtringList Dirs=Dir::GetAllFileNames(Path, Dir::Include_Dirs);
                                    bool Exists=false;
                                    for (size_t Pos=0; Pos<Dirs.size(); Pos++)
                                        if (Dirs[Pos].size()>6
                                         && (Dirs[Pos][Dirs[Pos].size()-1]&(~0x20))==__T('O')
                                         && (Dirs[Pos][Dirs[Pos].size()-2]&(~0x20))==__T('E')
                                         && (Dirs[Pos][Dirs[Pos].size()-3]&(~0x20))==__T('D')
                                         && (Dirs[Pos][Dirs[Pos].size()-4]&(~0x20))==__T('I')
                                         && (Dirs[Pos][Dirs[Pos].size()-5]&(~0x20))==__T('V')
                                         && Dirs[Pos][Dirs[Pos].size()-6]==PathSeparator)
                                        {
                                            MXF_File+=Dirs[Pos].substr(Dirs[Pos].size()-5, 5);
                                            Exists=true;
                                            break;
                                        }
                                    if (!Exists)
                                        MXF_File+=__T("AUDIO");

                                    MXF_File+=PathSeparator;
                                    MXF_File+=file;

                                    Dirs=Dir::GetAllFileNames(File_Name.substr(0, File_Name.size()-10-1)+PathSeparator+MXF_File+__T(".MXF"), Dir::Include_Files);
                                    Exists=false;
                                    for (size_t Pos=0; Pos<Dirs.size(); Pos++)
                                        if (Dirs[Pos].size()>4
                                         && (Dirs[Pos][Dirs[Pos].size()-1]&(~0x20))==__T('F')
                                         && (Dirs[Pos][Dirs[Pos].size()-2]&(~0x20))==__T('X')
                                         && (Dirs[Pos][Dirs[Pos].size()-3]&(~0x20))==__T('M')
                                         && Dirs[Pos][Dirs[Pos].size()-4]==__T('.'))
                                        {
                                            MXF_File+=Dirs[Pos].substr(Dirs[Pos].size()-4, 4);
                                            Exists=true;
                                            break;
                                        }
                                    if (!Exists)
                                        MXF_File+=__T(".MXF");

                                    ReferenceFile.FileNames.push_back(MXF_File);
                                    ReferenceFile.StreamKind=Stream_Video;
                                    ReferenceFile.StreamID=ReferenceFiles->References.size()+1;
                                    ReferenceFiles->References.push_back(ReferenceFile);
                                }
                            #endif //defined(MEDIAINFO_MXF_YES)
                        }
                        else if (Field=="Audio")
                        {
                            #if defined(MEDIAINFO_MXF_YES)
                                if (File_Name.size()>10+1+4+1
                                 && File_Name[File_Name.size()-10-1]==PathSeparator
                                 && (File_Name[File_Name.size()-10-2]&(~0x20))==__T('P')
                                 && (File_Name[File_Name.size()-10-3]&(~0x20))==__T('I')
                                 && (File_Name[File_Name.size()-10-4]&(~0x20))==__T('L')
                                 && (File_Name[File_Name.size()-10-5]&(~0x20))==__T('C')
                                 && File_Name[File_Name.size()-10-6]==PathSeparator)
                                {
                                    Ztring file=File_Name.substr(File_Name.size()-10, 6);
                                    Ztring MXF_File=__T("..");
                                    MXF_File+=PathSeparator;

                                    Ztring Path=File_Name.substr(0, File_Name.size()-10-6);
                                    ZtringList Dirs=Dir::GetAllFileNames(Path, Dir::Include_Dirs);
                                    bool Exists=false;
                                    for (size_t Pos=0; Pos<Dirs.size(); Pos++)
                                        if (Dirs[Pos].size()>6
                                         && (Dirs[Pos][Dirs[Pos].size()-1]&(~0x20))==__T('O')
                                         && (Dirs[Pos][Dirs[Pos].size()-2]&(~0x20))==__T('I')
                                         && (Dirs[Pos][Dirs[Pos].size()-3]&(~0x20))==__T('D')
                                         && (Dirs[Pos][Dirs[Pos].size()-4]&(~0x20))==__T('U')
                                         && (Dirs[Pos][Dirs[Pos].size()-5]&(~0x20))==__T('A')
                                         && Dirs[Pos][Dirs[Pos].size()-6]==PathSeparator)
                                        {
                                            MXF_File+=Dirs[Pos].substr(Dirs[Pos].size()-5, 5);
                                            Exists=true;
                                            break;
                                        }
                                    if (!Exists)
                                        MXF_File+=__T("AUDIO");

                                    MXF_File+=PathSeparator;
                                    MXF_File+=file;
                                    Ztring Pos=Ztring::ToZtring(Audio_Count);
                                    if (Pos.size()<2)
                                        Pos.insert(0, 1, __T('0'));
                                    MXF_File+=Pos;

                                    Dirs=Dir::GetAllFileNames(File_Name.substr(0, File_Name.size()-10-1)+PathSeparator+MXF_File+__T(".MXF"), Dir::Include_Files);
                                    Exists=false;
                                    for (size_t Pos=0; Pos<Dirs.size(); Pos++)
                                        if (Dirs[Pos].size()>4
                                         && (Dirs[Pos][Dirs[Pos].size()-1]&(~0x20))==__T('F')
                                         && (Dirs[Pos][Dirs[Pos].size()-2]&(~0x20))==__T('X')
                                         && (Dirs[Pos][Dirs[Pos].size()-3]&(~0x20))==__T('M')
                                         && Dirs[Pos][Dirs[Pos].size()-4]==__T('.'))
                                        {
                                            MXF_File+=Dirs[Pos].substr(Dirs[Pos].size()-4, 4);
                                            Exists=true;
                                            break;
                                        }
                                    if (!Exists)
                                        MXF_File+=__T(".MXF");

                                    File__ReferenceFilesHelper::reference ReferenceFile;
                                    ReferenceFile.FileNames.push_back(MXF_File);
                                    ReferenceFile.StreamKind=Stream_Audio;
                                    ReferenceFile.StreamID=ReferenceFiles->References.size()+1;
                                    ReferenceFiles->References.push_back(ReferenceFile);

                                    Audio_Count++;
                                }
                            #endif //defined(MEDIAINFO_MXF_YES)
                        }

                        Track=Track->NextSiblingElement();
                    }
                }

                //ClipMetadata
                XMLElement* ClipMetadata=ClipContent->FirstChildElement("ClipMetadata");
                if (ClipMetadata)
                {
                    XMLElement* Access=ClipMetadata->FirstChildElement("Access");
                    if (Access)
                    {
                        //CreationDate
                        ChildElement=Access->FirstChildElement("CreationDate");
                        if (ChildElement)
                        {
                            Ztring Content=ChildElement->GetText();
                            if (Content.size()>=11 && Content[10]==__T('T'))
                                Content[10]=__T(' ');
                            if (Content.find(__T("+00:00"))!=string::npos)
                            {
                                Content.resize(10+1+8);
                                Content.insert(0, __T("UTC "));
                            }
                            Fill(Stream_General, 0, General_Recorded_Date, Content);
                        }

                        //CreationDate
                        ChildElement=Access->FirstChildElement("LastUpdateDate");
                        if (ChildElement)
                        {
                            Ztring Content=ChildElement->GetText();
                            if (Content.size()>=11 && Content[10]==__T('T'))
                                Content[10]=__T(' ');
                            if (Content.find(__T("+00:00"))!=string::npos)
                            {
                                Content.resize(10+1+8);
                                Content.insert(0, __T("UTC "));
                            }
                            Fill(Stream_General, 0, General_Tagged_Date, Content);
                        }
                    }

                    XMLElement* Device=ClipMetadata->FirstChildElement("Device");
                    if (Device)
                    {
                        //Manufacturer+ModelName
                        XMLElement* Manufacturer=Device->FirstChildElement("Manufacturer");
                        XMLElement* ModelName=Device->FirstChildElement("ModelName");
                        if (Manufacturer && ModelName)
                            Fill(Stream_General, 0, General_Encoded_Application, string(Manufacturer->GetText())+" "+ModelName->GetText());
                    }

                    XMLElement* Shoot=ClipMetadata->FirstChildElement("Shoot");
                    if (Shoot)
                    {
                        //StartDate
                        ChildElement=Shoot->FirstChildElement("StartDate");
                        if (ChildElement)
                        {
                            Ztring Content=ChildElement->GetText();
                            if (Content.size()>=11 && Content[10]==__T('T'))
                                Content[10]=__T(' ');
                            if (Content.find(__T("+00:00"))!=string::npos)
                            {
                                Content.resize(10+1+8);
                                Content.insert(0, __T("UTC "));
                            }
                            Fill(Stream_General, 0, General_Duration_Start, Content);
                        }

                        //EndDate
                        ChildElement=Shoot->FirstChildElement("EndDate");
                        if (ChildElement)
                        {
                            Ztring Content=ChildElement->GetText();
                            if (Content.size()>=11 && Content[10]==__T('T'))
                                Content[10]=__T(' ');
                            if (Content.find(__T("+00:00"))!=string::npos)
                            {
                                Content.resize(10+1+8);
                                Content.insert(0, __T("UTC "));
                            }
                            Fill(Stream_General, 0, General_Duration_End, Content);
                        }

                        //Location
                        XMLElement* Location=Shoot->FirstChildElement("Location");
                        if (Location)
                        {
                            //Longitude+Latitude
                            XMLElement* Longitude=Location->FirstChildElement("Longitude");
                            XMLElement* Latitude=Location->FirstChildElement("Latitude");
                            if (Longitude && Latitude)
                                Fill(Stream_General, 0, General_Recorded_Location, string(Latitude->GetText())+", "+Longitude->GetText());
                        }
                    }

                    XMLElement* Scenario=ClipMetadata->FirstChildElement("Scenario");
                    if (Scenario)
                    {
                        //ProgramName
                        ChildElement=Scenario->FirstChildElement("ProgramName");
                        if (ChildElement)
                            Fill(Stream_General, 0, General_Title, ChildElement->GetText());

                        //SceneNo.
                        ChildElement=Scenario->FirstChildElement("SceneNo.");
                        if (ChildElement)
                            Fill(Stream_General, 0, "Scene Number", ChildElement->GetText());

                        //TakeNo.
                        ChildElement=Scenario->FirstChildElement("TakeNo.");
                        if (ChildElement)
                            Fill(Stream_General, 0, "Take Number", ChildElement->GetText());
                    }

                    XMLElement* News=ClipMetadata->FirstChildElement("News");
                    if (News)
                    {
                        //Reporter
                        ChildElement=News->FirstChildElement("Reporter");
                        if (ChildElement)
                            Fill(Stream_General, 0, "Reporter", ChildElement->GetText());

                        //Purpose
                        ChildElement=News->FirstChildElement("Purpose");
                        if (ChildElement)
                            Fill(Stream_General, 0, "Purpose", ChildElement->GetText());

                        //Object
                        ChildElement=News->FirstChildElement("Object");
                        if (ChildElement)
                            Fill(Stream_General, 0, "Object", ChildElement->GetText());
                    }
                }
            }
        }
        else
        {
            Reject("P2_Clip");
            return false;
        }
    }

    Element_Offset=File_Size;

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_P2_YES
