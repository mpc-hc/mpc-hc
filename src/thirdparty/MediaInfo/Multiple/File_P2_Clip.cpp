// File_P2_Clip - Info for P2 Clip (XML) files
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
#if defined(MEDIAINFO_P2_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_P2_Clip.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "ZenLib/Dir.h"
#include "ZenLib/FileName.h"
#include "ZenLib/TinyXml/tinyxml.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_P2_Clip::FileHeader_Begin()
{
    //Element_Size
    if (File_Size>64*1024)
    {
        Reject("P2_Clip");
        return false; //P2_Clip XML files are not big
    }

    //Element_Size
    if (Buffer_Size<5)
        return false; //Must wait for more data

    //XML header
    if (Buffer[0]!='<'
     || Buffer[1]!='?'
     || Buffer[2]!='x'
     || Buffer[3]!='m'
     || Buffer[4]!='l')
    {
        Reject("P2_Clip");
        return false;
    }

    TiXmlDocument document(File_Name.To_Local());
    if (document.LoadFile())
    {
        TiXmlElement* Root=document.FirstChildElement("P2Main");
        if (Root)
        {
            Accept("P2_Clip");
            Fill(Stream_General, 0, General_Format, "P2 Clip");

            TiXmlElement* ClipContent=Root->FirstChildElement("ClipContent");
            if (ClipContent)
            {
                TiXmlElement* ChildElement;

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
                int64u EditUnit_Numerator=EditUnit.SubString(Ztring(), _T("/")).To_int64u();
                int64u EditUnit_Denominator=EditUnit.SubString(_T("/"), Ztring()).To_int64u();
                if (Duration_Frames && EditUnit_Numerator && EditUnit_Denominator)
                    Fill(Stream_General, 0, General_Duration, ((float32)Duration_Frames)*1000*EditUnit_Numerator/EditUnit_Denominator, 0);

                //EssenceList
                TiXmlElement* EssenceList=ClipContent->FirstChildElement("EssenceList");
                int64u File_Size_Total=File_Size;
                if (EssenceList)
                {
                    TiXmlElement* Track=EssenceList->FirstChildElement();
                    while (Track)
                    {
                        string Field=Track->ValueStr();
                        if (Field=="Video")
                        {
                            Stream_Prepare(Stream_Video);

                            //CreationDate
                            ChildElement=Track->FirstChildElement("StartTimecode");
                            if (ChildElement)
                            {
                                string Text=ChildElement->GetText();
                                if (Text.size()==11)
                                {
                                    int32u ToFill=(Text[0]-'0')*10*60*60*1000
                                                + (Text[1]-'0')   *60*60*1000
                                                + (Text[3]-'0')   *10*60*1000
                                                + (Text[4]-'0')      *60*1000
                                                + (Text[6]-'0')      *10*1000
                                                + (Text[7]-'0')         *1000;
                                    ChildElement=Track->FirstChildElement("FrameRate");
                                    if (ChildElement)
                                    {
                                        Ztring FrameRateS=Ztring(ChildElement->GetText());
                                        float32 FrameRate=FrameRateS.To_float32();
                                        if (FrameRateS.find('i')!=string::npos)
                                            FrameRate/=2;
                                        if (FrameRate)
                                            ToFill+=float32_int32s(((Text[9]-'0')*10+(Text[10]-'0'))*1000/FrameRate);
                                    }
                                    Fill(Stream_Video, 0, Video_Delay, ToFill);
                                }
                            }

                            #if defined(MEDIAINFO_MXF_YES)
                                if (File_Name.size()>10+1+4
                                 && File_Name[File_Name.size()-10-1]==PathSeparator
                                 && File_Name[File_Name.size()-10-2]==_T('P')
                                 && File_Name[File_Name.size()-10-3]==_T('I')
                                 && File_Name[File_Name.size()-10-4]==_T('L')
                                 && File_Name[File_Name.size()-10-5]==_T('C'))
                                {
                                    Ztring file=File_Name.substr(File_Name.size()-10, 6);
                                    Ztring MXF_File=File_Name;
                                    MXF_File.resize(MXF_File.size()-(10+1+4));
                                    MXF_File+=_T("VIDEO");
                                    MXF_File+=PathSeparator;
                                    MXF_File+=file;
                                    MXF_File+=_T(".MXF");

                                    //int8u ReadByHuman=Ztring(MediaInfo::Option_Static(_T("ReadByHuman_Get"))).To_int8u();
                                    //MediaInfo::Option_Static(_T("ReadByHuman"), _T("0"));
                                    MediaInfo_Internal MI;
                                    if (MI.Open(MXF_File))
                                    {
                                        //MediaInfo::Option_Static(_T("ReadByHuman"), ReadByHuman?_T("1"):_T("0"));
                                        Merge(MI, Stream_Video, 0, StreamPos_Last);
                                        Fill(Stream_Video, StreamPos_Last, "Source", MXF_File);
                                        File_Size_Total+=Ztring(MI.Get(Stream_General, 0, General_FileSize)).To_int64u();

                                        //Commercial names
                                        Fill(Stream_General, 0, General_Format_Commercial_IfAny, MI.Get(Stream_General, 0, General_Format_Commercial_IfAny), true);
                                        Fill(Stream_General, 0, General_Format_Commercial, _T("P2 Clip ")+MI.Get(Stream_General, 0, General_Format_Commercial_IfAny), true);
                                    }
                                    //else
                                    //    MediaInfo::Option_Static(_T("ReadByHuman"), ReadByHuman?_T("1"):_T("0"));
                                }
                            #endif //defined(MEDIAINFO_MXF_YES)
                        }
                        else if (Field=="Audio")
                        {
                            Stream_Prepare(Stream_Audio);

                            #if defined(MEDIAINFO_MXF_YES)
                                if (File_Name.size()>10+1+4
                                 && File_Name[File_Name.size()-10-1]==PathSeparator
                                 && File_Name[File_Name.size()-10-2]==_T('P')
                                 && File_Name[File_Name.size()-10-3]==_T('I')
                                 && File_Name[File_Name.size()-10-4]==_T('L')
                                 && File_Name[File_Name.size()-10-5]==_T('C'))
                                {
                                    Ztring file=File_Name.substr(File_Name.size()-10, 6);
                                    Ztring MXF_File=File_Name;
                                    MXF_File.resize(MXF_File.size()-(10+1+4));
                                    MXF_File+=_T("AUDIO");
                                    MXF_File+=PathSeparator;
                                    MXF_File+=file;
                                    Ztring Pos=Ztring::ToZtring(StreamPos_Last);
                                    if (Pos.size()<2)
                                        Pos.insert(0, 1, _T('0'));
                                    MXF_File+=Pos;
                                    MXF_File+=_T(".MXF");

                                    //int8u ReadByHuman=Ztring(MediaInfo::Option_Static(_T("ReadByHuman_Get"))).To_int8u();
                                    //MediaInfo::Option_Static(_T("ReadByHuman"), _T("0"));
                                    MediaInfo_Internal MI;
                                    if (MI.Open(MXF_File))
                                    {
                                        //MediaInfo::Option_Static(_T("ReadByHuman"), ReadByHuman?_T("1"):_T("0"));
                                        Merge(MI, Stream_Audio, 0, StreamPos_Last);
                                        Fill(Stream_Audio, StreamPos_Last, "Source", MXF_File);
                                        File_Size_Total+=Ztring(MI.Get(Stream_General, 0, General_FileSize)).To_int64u();
                                    }
                                    //else
                                    //    MediaInfo::Option_Static(_T("ReadByHuman"), ReadByHuman?_T("1"):_T("0"));
                                }
                            #endif //defined(MEDIAINFO_MXF_YES)
                        }

                        Track=Track->NextSiblingElement();
                    }

                    if (File_Size_Total!=File_Size)
                        Fill(Stream_General, 0, General_FileSize, File_Size_Total, 10, true);
                }

                //ClipMetadata
                TiXmlElement* ClipMetadata=ClipContent->FirstChildElement("ClipMetadata");
                if (ClipMetadata)
                {
                    TiXmlElement* Access=ClipMetadata->FirstChildElement("Access");
                    if (Access)
                    {
                        //CreationDate
                        ChildElement=Access->FirstChildElement("CreationDate");
                        if (ChildElement)
                        {
                            Ztring Content=ChildElement->GetText();
                            if (Content.size()>=11 && Content[10]==_T('T'))
                                Content[10]=_T(' ');
                            if (Content.find(_T("+00:00"))!=string::npos)
                            {
                                Content.resize(10+1+8);
                                Content.insert(0, _T("UTC "));
                            }
                            Fill(Stream_General, 0, General_Recorded_Date, Content);
                        }

                        //CreationDate
                        ChildElement=Access->FirstChildElement("LastUpdateDate");
                        if (ChildElement)
                        {
                            Ztring Content=ChildElement->GetText();
                            if (Content.size()>=11 && Content[10]==_T('T'))
                                Content[10]=_T(' ');
                            if (Content.find(_T("+00:00"))!=string::npos)
                            {
                                Content.resize(10+1+8);
                                Content.insert(0, _T("UTC "));
                            }
                            Fill(Stream_General, 0, General_Tagged_Date, Content);
                        }
                    }

                    TiXmlElement* Device=ClipMetadata->FirstChildElement("Device");
                    if (Device)
                    {
                        //Manufacturer+ModelName
                        TiXmlElement* Manufacturer=Device->FirstChildElement("Manufacturer");
                        TiXmlElement* ModelName=Device->FirstChildElement("ModelName");
                        if (Manufacturer && ModelName)
                            Fill(Stream_General, 0, General_Encoded_Application, string(Manufacturer->GetText())+" "+ModelName->GetText());
                    }

                    TiXmlElement* Shoot=ClipMetadata->FirstChildElement("Shoot");
                    if (Shoot)
                    {
                        //StartDate
                        ChildElement=Shoot->FirstChildElement("StartDate");
                        if (ChildElement)
                        {
                            Ztring Content=ChildElement->GetText();
                            if (Content.size()>=11 && Content[10]==_T('T'))
                                Content[10]=_T(' ');
                            if (Content.find(_T("+00:00"))!=string::npos)
                            {
                                Content.resize(10+1+8);
                                Content.insert(0, _T("UTC "));
                            }
                            Fill(Stream_General, 0, General_Duration_Start, Content);
                        }

                        //EndDate
                        ChildElement=Shoot->FirstChildElement("EndDate");
                        if (ChildElement)
                        {
                            Ztring Content=ChildElement->GetText();
                            if (Content.size()>=11 && Content[10]==_T('T'))
                                Content[10]=_T(' ');
                            if (Content.find(_T("+00:00"))!=string::npos)
                            {
                                Content.resize(10+1+8);
                                Content.insert(0, _T("UTC "));
                            }
                            Fill(Stream_General, 0, General_Duration_End, Content);
                        }

                        //Location
                        TiXmlElement* Location=Shoot->FirstChildElement("Location");
                        if (Location)
                        {
                            //Longitude+Latitude
                            TiXmlElement* Longitude=Location->FirstChildElement("Longitude");
                            TiXmlElement* Latitude=Location->FirstChildElement("Latitude");
                            if (Longitude && Latitude)
                                Fill(Stream_General, 0, General_Recorded_Location, string(Latitude->GetText())+", "+Longitude->GetText());
                        }
                    }

                    TiXmlElement* Scenario=ClipMetadata->FirstChildElement("Scenario");
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

                    TiXmlElement* News=ClipMetadata->FirstChildElement("News");
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
    else
    {
        Reject("P2_Clip");
        return false;
    }

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_P2_YES
