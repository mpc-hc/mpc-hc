// File_Xdcam_Clip - Info for XDCAM XML files
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
#if defined(MEDIAINFO_XDCAM_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Xdcam_Clip.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "ZenLib/Dir.h"
#include "ZenLib/File.h"
#include "ZenLib/FileName.h"
#include "ZenLib/TinyXml/tinyxml.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Xdcam_Clip::FileHeader_Begin()
{
    //Element_Size
    if (File_Size>64*1024)
    {
        Reject("Xdcam_Clip");
        return false; //Xdcam_Clip XML files are not big
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
        Reject("Xdcam_Clip");
        return false;
    }

    TiXmlDocument document(File_Name.To_Local());
    if (document.LoadFile())
    {
        TiXmlElement* Root=document.FirstChildElement("NonRealTimeMeta");
        if (Root)
        {
            Accept("Xdcam_Clip");
            Fill(Stream_General, 0, General_Format, "XDCAM Clip");

            TiXmlElement* Element;

            //CreationDate
            Element=Root->FirstChildElement("CreationDate");
            string A=Element->Attribute("value");
            if (Element)
                Fill(Stream_General, 0, General_Recorded_Date, Element->Attribute("value"));

            //CreationDate
            Element=Root->FirstChildElement("LastUpdate");
            if (Element)
                Fill(Stream_General, 0, General_Tagged_Date, Element->Attribute("value"));

            //Duration
            Ztring Duration, EditUnit;
            Element=Root->FirstChildElement("Duration");
            if (Element)
                Duration=Element->Attribute("value");
            Element=Root->FirstChildElement("LtcChangeTable");
            if (Element)
                EditUnit=Element->Attribute("tcFps");
            int64u Duration_Frames=Duration.To_int64u();
            int64u EditUnit_Denominator=EditUnit.To_int64u();
            if (Duration_Frames && EditUnit_Denominator)
                Fill(Stream_General, 0, General_Duration, ((float32)Duration_Frames)*1000/EditUnit_Denominator, 0);

            int64u File_Size_Total=File_Size;

            #if defined(MEDIAINFO_MXF_YES)
                if (File_Name.size()>12
                 && File_Name[File_Name.size()-7]==_T('M')
                 && File_Name[File_Name.size()-6]==_T('0')
                 && File_Name[File_Name.size()-5]==_T('1')
                 && File_Name[File_Name.size()-4]==_T('.')
                 && File_Name[File_Name.size()-3]==_T('X')
                 && File_Name[File_Name.size()-2]==_T('M')
                 && File_Name[File_Name.size()-1]==_T('L'))
                {
                    Ztring file=File_Name.substr(File_Name.size()-12, 5);
                    Ztring MXF_File=File_Name;
                    MXF_File.resize(MXF_File.size()-12);
                    MXF_File+=file;
                    if (File::Exists(MXF_File+_T(".MXF")))
                        MXF_File+=_T(".MXF");
                    if (File::Exists(MXF_File+_T(".MP4")))
                        MXF_File+=_T(".MP4");

                    //int8u ReadByHuman=Ztring(MediaInfo::Option_Static(_T("ReadByHuman_Get"))).To_int8u();
                    //MediaInfo::Option_Static(_T("ReadByHuman"), _T("0"));
                    MediaInfo_Internal MI;
                    if (MI.Open(MXF_File))
                    {
                        //MediaInfo::Option_Static(_T("ReadByHuman"), ReadByHuman?_T("1"):_T("0"));
                        Merge(MI);
                        Fill(Stream_Video, StreamPos_Last, "Source", MXF_File);
                        File_Size_Total+=Ztring(MI.Get(Stream_General, 0, General_FileSize)).To_int64u();

                        //Commercial names
                        Fill(Stream_General, 0, General_Format_Commercial_IfAny, MI.Get(Stream_General, 0, General_Format_Commercial_IfAny));
                        Ztring Format_Commercial=MI.Get(Stream_General, 0, General_Format_Commercial_IfAny);
                        if (!Format_Commercial.empty())
                        {
                            Format_Commercial.FindAndReplace(_T("XDCAM "), Ztring());
                            Fill(Stream_General, 0, General_Format_Commercial, _T("XDCAM Clip ")+Format_Commercial, true);
                        }
                    }
                    //else
                    //    MediaInfo::Option_Static(_T("ReadByHuman"), ReadByHuman?_T("1"):_T("0"));
                }
            #endif //defined(MEDIAINFO_MXF_YES)

            //Device
            Element=Root->FirstChildElement("Device");
            if (Element)
                Fill(Stream_General, 0, General_Encoded_Application, string(Element->Attribute("manufacturer"))+" "+Element->Attribute("modelName"), true, true);

            if (File_Size_Total!=File_Size)
                Fill(Stream_General, 0, General_FileSize, File_Size_Total, 10, true);
        }
        else
        {
            Reject("Xdcam_Clip");
            return false;
        }
    }
    else
    {
        Reject("Xdcam_Clip");
        return false;
    }

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_XDCAM_YES
