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
#include "MediaInfo/Multiple/File_DcpCpl.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "MediaInfo/Multiple/File_DcpPkl.h"
#include "MediaInfo/Multiple/File_Mxf.h"
#include "ZenLib/Dir.h"
#include "ZenLib/FileName.h"
#include "tinyxml2.h"
#include <list>
using namespace tinyxml2;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

struct DcpCpl_info
{
    Ztring FileName;
    File__ReferenceFilesHelper::references::iterator Reference;
};

//---------------------------------------------------------------------------
extern void DcpCpl_MergeFromPkl(File__ReferenceFilesHelper* FromCpl, File__ReferenceFilesHelper* FromPkl)
{
    map<Ztring, DcpCpl_info> Map;
    list<File__ReferenceFilesHelper::references::iterator> List;
    ZtringList ExtraFiles_Name;
    for (File__ReferenceFilesHelper::references::iterator Reference=FromPkl->References.begin(); Reference!=FromPkl->References.end(); ++Reference)
    {
        map<string, Ztring>::iterator UniqueID=Reference->Infos.find("UniqueID");
        for (size_t Pos=0; Pos<Reference->FileNames.size(); Pos++)
            if (UniqueID!=Reference->Infos.end())
            {
                Map[UniqueID->second].FileName=Reference->FileNames[Pos];
                Map[UniqueID->second].Reference=Reference;
            }
        List.push_back(Reference);
    }

    for (File__ReferenceFilesHelper::references::iterator Reference=FromCpl->References.begin(); Reference!=FromCpl->References.end(); ++Reference)
        for (size_t Pos=0; Pos<Reference->FileNames.size(); Pos++)
        {
            map<Ztring, DcpCpl_info>::iterator Map_Item=Map.find(Reference->FileNames[Pos]);
            if (Map_Item!=Map.end())
            {
                Reference->FileNames[Pos]=Map_Item->second.FileName;
                for (list<File__ReferenceFilesHelper::references::iterator>::iterator Reference2=List.begin(); Reference2!=List.end(); ++Reference2)
                    if (*Reference2==Map_Item->second.Reference)
                    {
                        List.erase(Reference2);
                        break;
                    }
            }
        }

    for (list<File__ReferenceFilesHelper::references::iterator>::iterator Reference=List.begin(); Reference!=List.end(); ++Reference)
    {
        FromCpl->References.push_back(**Reference);
        FromCpl->References[FromCpl->References.size()-1].StreamID=FromCpl->References.size()-1;
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_DcpCpl::File_DcpCpl()
:File__Analyze()
{
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_DcpCpl;
        StreamIDs_Width[0]=sizeof(size_t)*2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_EventWasSent_Accept_Specific=true;
    #endif //MEDIAINFO_DEMUX

    //Temp
    ReferenceFiles=NULL;
}

//---------------------------------------------------------------------------
File_DcpCpl::~File_DcpCpl()
{
    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_DcpCpl::Streams_Finish()
{
    if (Config->File_IsReferenced_Get() || ReferenceFiles==NULL)
        return;

    ReferenceFiles->ParseReferences();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_DcpCpl::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    if (Config->File_IsReferenced_Get() || ReferenceFiles==NULL)
        return 0;

    return ReferenceFiles->Read_Buffer_Seek(Method, Value, ID);
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_DcpCpl::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    bool IsDcp=false, IsImf=false;

    XMLElement* Root=document.FirstChildElement("CompositionPlaylist");
    if (!Root)
    {
        Reject("DcpCpl");
        return false;
    }

    const char* Attribute=Root->Attribute("xmlns");
    if (!Attribute)
    {
        Reject("DcpCpl");
        return false;
    }

    if (!strcmp(Attribute, "http://www.digicine.com/PROTO-ASDCP-CPL-20040511#"))
        IsDcp=true;
    if (!strcmp(Attribute, "http://www.smpte-ra.org/schemas/2067-3/XXXX") //Some muxers use XXXX instead of year
     || !strcmp(Attribute, "http://www.smpte-ra.org/schemas/2067-3/2013"))
        IsImf=true;

    if (!IsDcp && !IsImf)
    {
        Reject("DcpCpl");
        return false;
    }

    Accept("DcpCpl");
    Fill(Stream_General, 0, General_Format, IsDcp?"DCP CPL":"IMF CPL");
    Config->File_ID_OnlyRoot_Set(false);

    ReferenceFiles=new File__ReferenceFilesHelper(this, Config);

    //Parsing main elements
    for (XMLElement* Root_Item=Root->FirstChildElement(); Root_Item; Root_Item=Root_Item->NextSiblingElement())
    {
        //ReelList / SegmentList
        if ((IsDcp && !strcmp(Root_Item->Value(), "ReelList"))
         || (IsImf && !strcmp(Root_Item->Value(), "SegmentList")))
        {
            for (XMLElement* ReelList_Item=Root_Item->FirstChildElement(); ReelList_Item; ReelList_Item=ReelList_Item->NextSiblingElement())
            {
                //Reel
                if ((IsDcp && !strcmp(ReelList_Item->Value(), "Reel"))
                 || (IsImf && !strcmp(ReelList_Item->Value(), "Segment")))
                {
                    for (XMLElement* Reel_Item=ReelList_Item->FirstChildElement(); Reel_Item; Reel_Item=Reel_Item->NextSiblingElement())
                    {
                        //AssetList
                        if ((IsDcp && !strcmp(Reel_Item->Value(), "AssetList"))
                         || (IsImf && !strcmp(Reel_Item->Value(), "SequenceList")))
                        {
                            for (XMLElement* AssetList_Item=Reel_Item->FirstChildElement(); AssetList_Item; AssetList_Item=AssetList_Item->NextSiblingElement())
                            {
                                //File
                                //if ((IsDcp && (!strcmp(AssetList_Item->Value(), "MainPicture") || !strcmp(AssetList_Item->Value(), "MainSound")))
                                // || (IsImf && (!strcmp(AssetList_Item->Value(), "cc:MainImageSequence") || !strcmp(AssetList_Item->Value(), "cc:MainImage"))))
                                {
                                    File__ReferenceFilesHelper::reference ReferenceFile;
                                    Ztring Id;

                                    if ((IsDcp && !strcmp(AssetList_Item->Value(), "MainPicture"))
                                     || (IsImf && !strcmp(AssetList_Item->Value(), "cc:MainImageSequence")))
                                        ReferenceFile.StreamKind=Stream_Video;
                                    if ((IsDcp && !strcmp(AssetList_Item->Value(), "MainSound"))
                                     || (IsImf && !strcmp(AssetList_Item->Value(), "cc:MainAudioSequence")))
                                        ReferenceFile.StreamKind=Stream_Audio;

                                    for (XMLElement* File_Item=AssetList_Item->FirstChildElement(); File_Item; File_Item=File_Item->NextSiblingElement())
                                    {
                                        //Id
                                        if (!strcmp(File_Item->Value(), "Id") && Id.empty())
                                            Id.From_UTF8(File_Item->GetText());

                                        //ResourceList
                                        if (IsImf && !strcmp(File_Item->Value(), "ResourceList"))
                                        {
                                            for (XMLElement* ResourceList_Item=File_Item->FirstChildElement(); ResourceList_Item; ResourceList_Item=ResourceList_Item->NextSiblingElement())
                                            {
                                                //Resource
                                                if (!strcmp(ResourceList_Item->Value(), "Resource"))
                                                {
                                                    for (XMLElement* Resource_Item=ResourceList_Item->FirstChildElement(); Resource_Item; Resource_Item=Resource_Item->NextSiblingElement())
                                                    {
                                                        //TrackFileId
                                                        if (!strcmp(Resource_Item->Value(), "TrackFileId"))
                                                            ReferenceFile.FileNames.push_back(Resource_Item->GetText());
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if (ReferenceFile.FileNames.empty())
                                        ReferenceFile.FileNames.push_back(Id);
                                    ReferenceFile.StreamID=ReferenceFiles->References.size()+1;
                                    ReferenceFiles->References.push_back(ReferenceFile);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Element_Offset=File_Size;

    //Getting files names
    if (!Config->File_IsReferenced_Get())
    {
        FileName Directory(File_Name);
        ZtringList List;
        if (IsImf)
            List=Dir::GetAllFileNames(Directory.Path_Get()+PathSeparator+__T("PKL_*.xml"), Dir::Include_Files);
        if (IsDcp || List.empty())
            List=Dir::GetAllFileNames(Directory.Path_Get()+PathSeparator+__T("*_pkl.xml"), Dir::Include_Files);
        for (size_t Pos=0; Pos<List.size(); Pos++)
        {
            MediaInfo_Internal MI;
            MI.Option(__T("File_KeepInfo"), __T("1"));
            Ztring ParseSpeed_Save=MI.Option(__T("ParseSpeed_Get"), __T(""));
            Ztring Demux_Save=MI.Option(__T("Demux_Get"), __T(""));
            MI.Option(__T("ParseSpeed"), __T("0"));
            MI.Option(__T("Demux"), Ztring());
            MI.Option(__T("File_IsReferenced"), __T("1"));
            size_t MiOpenResult=MI.Open(List[Pos]);
            MI.Option(__T("ParseSpeed"), ParseSpeed_Save); //This is a global value, need to reset it. TODO: local value
            MI.Option(__T("Demux"), Demux_Save); //This is a global value, need to reset it. TODO: local value
            if (MiOpenResult
             && ((IsDcp && MI.Get(Stream_General, 0, General_Format)==__T("DCP PKL"))
              || (IsImf && MI.Get(Stream_General, 0, General_Format)==__T("IMF PKL"))))
            {
                DcpCpl_MergeFromPkl(ReferenceFiles, ((File_DcpCpl*)MI.Info)->ReferenceFiles);
            }
        }
    }

    ReferenceFiles->FilesForStorage=true;

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_DCP_YES

