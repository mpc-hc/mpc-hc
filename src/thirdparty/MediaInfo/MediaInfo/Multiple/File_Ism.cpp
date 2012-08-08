// File_Ism - Info for ISM files
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
#if defined(MEDIAINFO_ISM_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Ism.h"
#include <set>
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "ZenLib/Dir.h"
#include "ZenLib/FileName.h"
#include "ZenLib/Format/Http/Http_Utils.h"
#include "tinyxml2.h"
using namespace tinyxml2;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Ism::File_Ism()
:File__Analyze()
{
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Ism;
        StreamIDs_Width[0]=sizeof(size_t)*2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_EventWasSent_Accept_Specific=true;
    #endif //MEDIAINFO_DEMUX

    //Temp
    ReferenceFiles=NULL;
}

//---------------------------------------------------------------------------
File_Ism::~File_Ism()
{
    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ism::Streams_Finish()
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
size_t File_Ism::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
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
bool File_Ism::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    {
        XMLElement* Root=document.FirstChildElement("smil");
        if (Root)
        {
            Accept("ISM");
            Fill(Stream_General, 0, General_Format, "ISM");

            ReferenceFiles=new File__ReferenceFilesHelper(this, Config);

            std::set<Ztring> FileNames;

            XMLElement* Body=Root->FirstChildElement();
            while (Body)
            {
                if (string(Body->Value())=="body")
                {
                    XMLElement* Switch=Body->FirstChildElement();
                    while (Switch)
                    {
                        if (string(Switch->Value())=="switch")
                        {
                            XMLElement* Stream=Switch->FirstChildElement();
                            while (Stream)
                            {
                                if (string(Stream->Value())=="video" || string(Stream->Value())=="audio")
                                {
                                    File__ReferenceFilesHelper::reference ReferenceFile;

                                    if (string(Stream->Value())=="video")
                                        ReferenceFile.StreamKind=Stream_Video;
                                    if (string(Stream->Value())=="audio")
                                        ReferenceFile.StreamKind=Stream_Audio;

                                    const char* Attribute=Stream->Attribute("src");
                                    if (Attribute)
                                        ReferenceFile.FileNames.push_back(Ztring().From_UTF8(Attribute));

                                    XMLElement* Param=Stream->FirstChildElement();
                                    while (Param)
                                    {
                                        if (string(Param->Value())=="param")
                                        {
                                            Attribute=Param->Attribute("name");
                                            if (Attribute && Ztring().From_UTF8(Attribute)==__T("trackID"))
                                            {
                                                Attribute=Param->Attribute("value");
                                                if (Attribute)
                                                    ReferenceFile.StreamID=Ztring().From_UTF8(Attribute).To_int64u();
                                            }
                                        }
                                        Param=Param->NextSiblingElement();
                                    }

                                    if (!ReferenceFile.FileNames.empty() && !ReferenceFile.FileNames[0].empty() && FileNames.find(ReferenceFile.FileNames[0])==FileNames.end())
                                    {
                                        ReferenceFiles->References.push_back(ReferenceFile);
                                        FileNames.insert(ReferenceFile.FileNames[0]);
                                    }
                                }

                                Stream=Stream->NextSiblingElement();
                            }
                        }

                        Switch=Switch->NextSiblingElement();
                    }
                }

                Body=Body->NextSiblingElement();
            }
        }
        else
        {
            Reject("ISM");
            return false;
        }
    }

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_ISM_YES

