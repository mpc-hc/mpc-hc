// File_Xdcam_Clip - Info for XDCAM XML files
// Copyright (C) 2010-2012 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_XDCAM_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Xdcam_Clip.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "ZenLib/Dir.h"
#include "ZenLib/File.h"
#include "ZenLib/FileName.h"
#include "tinyxml2.h"
using namespace tinyxml2;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Xdcam_Clip::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    {
        XMLElement* Root=document.FirstChildElement("NonRealTimeMeta");
        if (Root)
        {
            Accept("Xdcam_Clip");
            Fill(Stream_General, 0, General_Format, "XDCAM Clip");

            XMLElement* Element;

            //CreationDate
            Element=Root->FirstChildElement("CreationDate");
            if (Element)
                Fill(Stream_General, 0, General_Recorded_Date, Element->Attribute("value"));

            //LastUpdate
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
                 && File_Name[File_Name.size()-7]==__T('M')
                 && File_Name[File_Name.size()-6]==__T('0')
                 && File_Name[File_Name.size()-5]==__T('1')
                 && File_Name[File_Name.size()-4]==__T('.')
                 && File_Name[File_Name.size()-3]==__T('X')
                 && File_Name[File_Name.size()-2]==__T('M')
                 && File_Name[File_Name.size()-1]==__T('L'))
                {
                    Ztring file=File_Name.substr(File_Name.size()-12, 5);
                    Ztring MXF_File=File_Name;
                    MXF_File.resize(MXF_File.size()-12);
                    MXF_File+=file;
                    if (File::Exists(MXF_File+__T(".MXF")))
                        MXF_File+=__T(".MXF");
                    if (File::Exists(MXF_File+__T(".MP4")))
                        MXF_File+=__T(".MP4");

                    //int8u ReadByHuman=Ztring(MediaInfo::Option_Static(__T("ReadByHuman_Get"))).To_int8u();
                    //MediaInfo::Option_Static(__T("ReadByHuman"), __T("0"));
                    MediaInfo_Internal MI;
                    if (MI.Open(MXF_File))
                    {
                        //MediaInfo::Option_Static(__T("ReadByHuman"), ReadByHuman?__T("1"):__T("0"));
                        Merge(MI);
                        Fill(Stream_Video, StreamPos_Last, "Source", MXF_File);
                        File_Size_Total+=Ztring(MI.Get(Stream_General, 0, General_FileSize)).To_int64u();

                        //Commercial names
                        Fill(Stream_General, 0, General_Format_Commercial_IfAny, MI.Get(Stream_General, 0, General_Format_Commercial_IfAny));
                        Ztring Format_Commercial=MI.Get(Stream_General, 0, General_Format_Commercial_IfAny);
                        if (!Format_Commercial.empty())
                        {
                            Format_Commercial.FindAndReplace(__T("XDCAM "), Ztring());
                            Fill(Stream_General, 0, General_Format_Commercial, __T("XDCAM Clip ")+Format_Commercial, true);
                        }
                    }
                    //else
                    //    MediaInfo::Option_Static(__T("ReadByHuman"), ReadByHuman?__T("1"):__T("0"));
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

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_XDCAM_YES
