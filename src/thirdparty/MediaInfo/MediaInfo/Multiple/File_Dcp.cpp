// File_Dcp - Info for DCP (XML) files
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
#if defined(MEDIAINFO_DCP_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Dcp.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
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
File_Dcp::File_Dcp()
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
File_Dcp::~File_Dcp()
{
    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dcp::Streams_Finish()
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
size_t File_Dcp::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
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
bool File_Dcp::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    {
        std::string NameSpace;
        XMLElement* AssetMap=document.FirstChildElement("AssetMap");
        if (AssetMap==NULL)
        {
            NameSpace="am:";
            AssetMap=document.FirstChildElement((NameSpace+"AssetMap").c_str());
        }
        if (AssetMap)
        {
            Accept("Dcp");
            Fill(Stream_General, 0, General_Format, "DCP");
            Fill(Stream_General, 0, General_Format_Version, NameSpace=="am:"?"SMPTE":"Interop");

            ReferenceFiles=new File__ReferenceFilesHelper(this, Config);

            XMLElement* IssueDate=AssetMap->FirstChildElement((NameSpace+"IssueDate").c_str());
            if (IssueDate)
                Fill(Stream_General, 0, General_Encoded_Date, IssueDate->GetText());
            XMLElement* Issuer=AssetMap->FirstChildElement((NameSpace+"Issuer").c_str());
            if (Issuer)
                Fill(Stream_General, 0, General_EncodedBy, Issuer->GetText());
            XMLElement* Creator=AssetMap->FirstChildElement((NameSpace+"Creator").c_str());
            if (Creator)
                Fill(Stream_General, 0, General_Encoded_Library, Creator->GetText());

            XMLElement* AssetList=AssetMap->FirstChildElement((NameSpace+"AssetList").c_str());
            if (AssetList)
            {
                XMLElement* Asset=AssetList->FirstChildElement((NameSpace+"Asset").c_str());
                while (Asset)
                {
                    XMLElement* ChunkList=Asset->FirstChildElement((NameSpace+"ChunkList").c_str());
                    if (ChunkList)
                    {
                        XMLElement* Chunk=ChunkList->FirstChildElement((NameSpace+"Chunk").c_str());
                        if (Chunk)
                        {
                            XMLElement* Path=Chunk->FirstChildElement((NameSpace+"Path").c_str());
                            if (Path)
                            {
                                File__ReferenceFilesHelper::reference ReferenceFile;
                                ReferenceFile.FileNames.push_back(Path->GetText());
                                ReferenceFile.StreamID=ReferenceFiles->References.size()+1;
                                ReferenceFiles->References.push_back(ReferenceFile);
                            }
                        }
                    }

                    Asset=Asset->NextSiblingElement();
                }
            }
        }
        else
        {
            Reject("Dcp");
            return false;
        }
    }

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_P2_YES

