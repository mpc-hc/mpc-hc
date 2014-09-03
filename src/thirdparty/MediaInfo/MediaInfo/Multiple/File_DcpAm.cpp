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
#if defined(MEDIAINFO_DCP_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_DcpAm.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "MediaInfo/Multiple/File_DcpCpl.h"
#include "ZenLib/Dir.h"
#include "ZenLib/FileName.h"
#include "tinyxml2.h"
using namespace tinyxml2;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
extern void DcpCpl_MergeFromPkl(File__ReferenceFilesHelper* FromCpl, File__ReferenceFilesHelper* FromPkl);

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_DcpAm::File_DcpAm()
:File__Analyze()
{
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_DcpAm;
        StreamIDs_Width[0]=sizeof(size_t)*2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_EventWasSent_Accept_Specific=true;
    #endif //MEDIAINFO_DEMUX

    //Temp
    ReferenceFiles=NULL;
}

//---------------------------------------------------------------------------
File_DcpAm::~File_DcpAm()
{
    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_DcpAm::Streams_Finish()
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
size_t File_DcpAm::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
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
bool File_DcpAm::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    std::string NameSpace;
    XMLElement* AssetMap=document.FirstChildElement("AssetMap");
    if (AssetMap==NULL)
    {
        NameSpace="am:";
        AssetMap=document.FirstChildElement((NameSpace+"AssetMap").c_str());
    }
    if (!AssetMap)
    {
        Reject("DcpAm");
        return false;
    }

    const char* Attribute=AssetMap->Attribute(NameSpace.empty()?"xmlns":"xmlns:am");
    if (!Attribute)
    {
        Reject("DcpAm");
        return false;
    }

    if (strcmp(Attribute, "http://www.digicine.com/PROTO-ASDCP-AM-20040311#")
     && strcmp(Attribute, "http://www.smpte-ra.org/schemas/429-9/2007/AM"))
    {
        Reject("DcpAm");
        return false;
    }

    Accept("DcpAm");
    Fill(Stream_General, 0, General_Format, "DCP AM");
    Fill(Stream_General, 0, General_Format_Version, NameSpace=="am:"?"SMPTE":"Interop");
    Config->File_ID_OnlyRoot_Set(false);

    ReferenceFiles=new File__ReferenceFilesHelper(this, Config);
    Ztring CPL_FileName;

    for (XMLElement* AssetMap_Item=AssetMap->FirstChildElement(); AssetMap_Item; AssetMap_Item=AssetMap_Item->NextSiblingElement())
    {
        //AssetList
        if (!strcmp(AssetMap_Item->Value(), (NameSpace+"AssetList").c_str()))
        {
            for (XMLElement* AssetList_Item=AssetMap_Item->FirstChildElement(); AssetList_Item; AssetList_Item=AssetList_Item->NextSiblingElement())
            {
                //Asset
                if (!strcmp(AssetList_Item->Value(), (NameSpace+"Asset").c_str()))
                {
                    File__ReferenceFilesHelper::reference ReferenceFile;
                    bool IsPKL=false;
                    bool IsCPL=false;

                    for (XMLElement* Asset_Item=AssetList_Item->FirstChildElement(); Asset_Item; Asset_Item=Asset_Item->NextSiblingElement())
                    {
                        //Id
                        if (!strcmp(Asset_Item->Value(), (NameSpace+"Id").c_str()))
                            ReferenceFile.Infos["UniqueID"].From_UTF8(Asset_Item->GetText());

                        //ChunkList
                        if (!strcmp(Asset_Item->Value(), (NameSpace+"ChunkList").c_str()))
                        {
                            for (XMLElement* ChunkList_Item=Asset_Item->FirstChildElement(); ChunkList_Item; ChunkList_Item=ChunkList_Item->NextSiblingElement())
                            {
                                //Chunk
                                if (!strcmp(ChunkList_Item->Value(), (NameSpace+"Chunk").c_str()))
                                {
                                    for (XMLElement* Chunk_Item=ChunkList_Item->FirstChildElement(); Chunk_Item; Chunk_Item=Chunk_Item->NextSiblingElement())
                                    {
                                        //Path
                                        if (!strcmp(Chunk_Item->Value(), (NameSpace+"Path").c_str()))
                                        {
                                            ReferenceFile.FileNames.push_back(Ztring().From_UTF8(Chunk_Item->GetText()));
                                            string Text=Chunk_Item->GetText();
                                            if (Text.size()>=8
                                             && (Text.find("_pkl.xml")==Text.size()-8)
                                              || (Text.find("PKL_")==0 && Text.find(".xml")==Text.size()-4))
                                                IsPKL=true;
                                            if (Text.size()>=8
                                             && (Text.find("_cpl.xml")==Text.size()-8)
                                              || (Text.find("CPL_")==0 && Text.find(".xml")==Text.size()-4))
                                                IsCPL=true;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (IsCPL)
                    {
                        if (CPL_FileName.empty() && !ReferenceFile.FileNames.empty())
                            CPL_FileName=ReferenceFile.FileNames[0]; //Using only the first CPL file meet
                    }
                    else if (!IsPKL)
                    {
                        ReferenceFile.StreamID=ReferenceFiles->References.size()+1;
                        ReferenceFiles->References.push_back(ReferenceFile);
                    }
                }
            }
        }

        //Creator
        if (!strcmp(AssetMap_Item->Value(), (NameSpace+"Creator").c_str()))
            Fill(Stream_General, 0, General_Encoded_Library, AssetMap_Item->GetText());

        //IssueDate
        if (!strcmp(AssetMap_Item->Value(), (NameSpace+"IssueDate").c_str()))
            Fill(Stream_General, 0, General_Encoded_Date, AssetMap_Item->GetText());

        //Issuer
        if (!strcmp(AssetMap_Item->Value(), (NameSpace+"Issuer").c_str()))
            Fill(Stream_General, 0, General_EncodedBy, AssetMap_Item->GetText());
    }

    Element_Offset=File_Size;

    //Getting links between files
    if (!CPL_FileName.empty() && !Config->File_IsReferenced_Get())
    {
        FileName Directory(File_Name);
        if (CPL_FileName.find(__T("file://"))==0 && CPL_FileName.find(__T("file:///"))==string::npos)
            CPL_FileName.erase(0, 7); //TODO: better handling of relative and absolute file naes
        MediaInfo_Internal MI;
        MI.Option(__T("File_KeepInfo"), __T("1"));
        Ztring ParseSpeed_Save=MI.Option(__T("ParseSpeed_Get"), __T(""));
        Ztring Demux_Save=MI.Option(__T("Demux_Get"), __T(""));
        MI.Option(__T("ParseSpeed"), __T("0"));
        MI.Option(__T("Demux"), Ztring());
        MI.Option(__T("File_IsReferenced"), __T("1"));
        size_t MiOpenResult=MI.Open(Directory.Path_Get()+PathSeparator+CPL_FileName);
        MI.Option(__T("ParseSpeed"), ParseSpeed_Save); //This is a global value, need to reset it. TODO: local value
        MI.Option(__T("Demux"), Demux_Save); //This is a global value, need to reset it. TODO: local value
        if (MiOpenResult
            && (MI.Get(Stream_General, 0, General_Format)==__T("DCP CPL")
            ||  MI.Get(Stream_General, 0, General_Format)==__T("IMF CPL")))
        {
            DcpCpl_MergeFromPkl(((File_DcpCpl*)MI.Info)->ReferenceFiles, ReferenceFiles);
            ReferenceFiles->References=((File_DcpCpl*)MI.Info)->ReferenceFiles->References;
            if (MI.Get(Stream_General, 0, General_Format)==__T("IMF CPL"))
            {
                Fill(Stream_General, 0, General_Format, "IMF AM", Unlimited, true, true);
                Clear(Stream_General, 0, General_Format_Version);
            }

            for (size_t Pos=0; Pos<MI.Count_Get(Stream_Other); ++Pos)
            {
                Stream_Prepare(Stream_Other);
                Merge(*MI.Info, Stream_Other, Pos, StreamPos_Last);
            }
        }
    }

    ReferenceFiles->FilesForStorage=true;

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_P2_YES

