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
#include "MediaInfo/Multiple/File_DcpAm.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "ZenLib/Dir.h"
#include "ZenLib/File.h"
#include "ZenLib/FileName.h"
#include "tinyxml2.h"
#include <list>
using namespace tinyxml2;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

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
    if (ReferenceFiles==NULL)
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
    if (ReferenceFiles==NULL)
        return 0;

    return ReferenceFiles->Seek(Method, Value, ID);
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

    if (!strcmp(Attribute, "http://www.digicine.com/PROTO-ASDCP-CPL-20040511#")
     ||!strcmp(Attribute, "http://www.smpte-ra.org/schemas/429-7/2006/CPL"))
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
    for (XMLElement* CompositionPlaylist_Item=Root->FirstChildElement(); CompositionPlaylist_Item; CompositionPlaylist_Item=CompositionPlaylist_Item->NextSiblingElement())
    {
        //CompositionTimecode
        if (IsImf && (!strcmp(CompositionPlaylist_Item->Value(), "CompositionTimecode") || !strcmp(CompositionPlaylist_Item->Value(), "cpl:CompositionTimecode")))
        {
            sequence* Sequence=new sequence;
            Sequence->StreamKind=Stream_Other;
            Sequence->Infos["Type"]=__T("Time code");
            Sequence->Infos["Format"]=__T("CPL TC");
            Sequence->Infos["TimeCode_Striped"]=__T("Yes");
            bool IsDropFrame=false;

            for (XMLElement* CompositionTimecode_Item=CompositionPlaylist_Item->FirstChildElement(); CompositionTimecode_Item; CompositionTimecode_Item=CompositionTimecode_Item->NextSiblingElement())
            {
                //TimecodeDropFrame
                if (!strcmp(CompositionTimecode_Item->Value(), "TimecodeDropFrame") || !strcmp(CompositionTimecode_Item->Value(), "cpl:TimecodeDropFrame"))
                {
                    if (strcmp(CompositionTimecode_Item->GetText(), "") && strcmp(CompositionTimecode_Item->GetText(), "0"))
                        IsDropFrame=true;
                }

                //TimecodeRate
                if (!strcmp(CompositionTimecode_Item->Value(), "TimecodeRate") || !strcmp(CompositionTimecode_Item->Value(), "cpl:TimecodeRate"))
                    Sequence->Infos["FrameRate"].From_UTF8(CompositionTimecode_Item->GetText());

                //TimecodeStartAddress
                if (!strcmp(CompositionTimecode_Item->Value(), "TimecodeStartAddress") || !strcmp(CompositionTimecode_Item->Value(), "cpl:TimecodeStartAddress"))
                    Sequence->Infos["TimeCode_FirstFrame"].From_UTF8(CompositionTimecode_Item->GetText());
            }

            //Adaptation
            if (IsDropFrame)
            {
                std::map<string, Ztring>::iterator Info=Sequence->Infos.find("TimeCode_FirstFrame");
                if (Info!=Sequence->Infos.end() && Info->second.size()>=11 && Info->second[8]!=__T(';'))
                    Info->second[8]=__T(';');
            }

            Sequence->StreamID=ReferenceFiles->Sequences_Size()+1;
            ReferenceFiles->AddSequence(Sequence);

            Stream_Prepare(Stream_Other);
            Fill(Stream_Other, StreamPos_Last, Other_ID, Sequence->StreamID);
            for (std::map<string, Ztring>::iterator Info=Sequence->Infos.begin(); Info!=Sequence->Infos.end(); ++Info)
                Fill(Stream_Other, StreamPos_Last, Info->first.c_str(), Info->second);
        }

        //ReelList / SegmentList
        if ((IsDcp && !strcmp(CompositionPlaylist_Item->Value(), "ReelList"))
         || (IsImf && !strcmp(CompositionPlaylist_Item->Value(), "SegmentList")))
        {
            for (XMLElement* ReelList_Item=CompositionPlaylist_Item->FirstChildElement(); ReelList_Item; ReelList_Item=ReelList_Item->NextSiblingElement())
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
                                    sequence* Sequence=new sequence;
                                    Ztring Asset_Id;

                                    if ((IsDcp && !strcmp(AssetList_Item->Value(), "MainPicture"))
                                     || (IsImf && !strcmp(AssetList_Item->Value(), "cc:MainImageSequence")))
                                        Sequence->StreamKind=Stream_Video;
                                    if ((IsDcp && !strcmp(AssetList_Item->Value(), "MainSound"))
                                     || (IsImf && !strcmp(AssetList_Item->Value(), "cc:MainAudioSequence")))
                                        Sequence->StreamKind=Stream_Audio;

                                    for (XMLElement* File_Item=AssetList_Item->FirstChildElement(); File_Item; File_Item=File_Item->NextSiblingElement())
                                    {
                                        //Id
                                        if (!strcmp(File_Item->Value(), "Id") && Asset_Id.empty())
                                            Asset_Id.From_UTF8(File_Item->GetText());

                                        //ResourceList
                                        if (IsImf && !strcmp(File_Item->Value(), "ResourceList"))
                                        {
                                            for (XMLElement* ResourceList_Item=File_Item->FirstChildElement(); ResourceList_Item; ResourceList_Item=ResourceList_Item->NextSiblingElement())
                                            {
                                                //Resource
                                                if (!strcmp(ResourceList_Item->Value(), "Resource"))
                                                {
                                                    Ztring Resource_Id;

                                                    resource* Resource=new resource;
                                                    for (XMLElement* Resource_Item=ResourceList_Item->FirstChildElement(); Resource_Item; Resource_Item=Resource_Item->NextSiblingElement())
                                                    {
                                                        //EditRate
                                                        if (!strcmp(Resource_Item->Value(), "EditRate"))
                                                        {
                                                            const char* EditRate=Resource_Item->GetText();
                                                            Resource->EditRate=atof(EditRate);
                                                            const char* EditRate2=strchr(EditRate, ' ');
                                                            if (EditRate2!=NULL)
                                                            {
                                                                float64 EditRate2f=atof(EditRate2);
                                                                if (EditRate2f)
                                                                    Resource->EditRate/=EditRate2f;
                                                            }
                                                        }

                                                        //EntryPoint
                                                        if (!strcmp(Resource_Item->Value(), "EntryPoint"))
                                                        {
                                                            Resource->IgnoreEditsBefore=atoi(Resource_Item->GetText());
                                                            if (Resource->IgnoreEditsAfter!=(int64u)-1)
                                                                Resource->IgnoreEditsAfter+=Resource->IgnoreEditsBefore;
                                                        }

                                                        //Id
                                                        if (!strcmp(File_Item->Value(), "Id") && Resource_Id.empty())
                                                            Resource_Id.From_UTF8(File_Item->GetText());

                                                        //SourceDuration
                                                        if (!strcmp(Resource_Item->Value(), "SourceDuration"))
                                                            Resource->IgnoreEditsAfter=Resource->IgnoreEditsBefore+atoi(Resource_Item->GetText());

                                                        //TrackFileId
                                                        if (!strcmp(Resource_Item->Value(), "TrackFileId"))
                                                            Resource->FileNames.push_back(Ztring().From_UTF8(Resource_Item->GetText()));
                                                    }

                                                    if (Resource->FileNames.empty())
                                                        Resource->FileNames.push_back(Resource_Id);
                                                    Sequence->AddResource(Resource);
                                                }
                                            }
                                        }
                                    }

                                    if (Sequence->Resources.empty())
                                    {
                                        resource* Resource=new resource;
                                        Resource->FileNames.push_back(Asset_Id);
                                        Sequence->AddResource(Resource);
                                    }
                                    Sequence->StreamID=ReferenceFiles->Sequences_Size()+1;
                                    ReferenceFiles->AddSequence(Sequence);
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
    FileName Directory(File_Name);
    Ztring Assetmap_FileName=Directory.Path_Get()+PathSeparator+__T("ASSETMAP.xml");
    bool IsOk=false;
    if (File::Exists(Assetmap_FileName))
        IsOk=true;
    else
    {
        Assetmap_FileName.resize(Assetmap_FileName.size()-4); //Old fashion, without ".xml"
        if (File::Exists(Assetmap_FileName))
            IsOk=true;
    }
    if (IsOk)
    {
        MediaInfo_Internal MI;
        MI.Option(__T("File_KeepInfo"), __T("1"));
        Ztring ParseSpeed_Save=MI.Option(__T("ParseSpeed_Get"), __T(""));
        Ztring Demux_Save=MI.Option(__T("Demux_Get"), __T(""));
        MI.Option(__T("ParseSpeed"), __T("0"));
        MI.Option(__T("Demux"), Ztring());
        MI.Option(__T("File_IsReferenced"), __T("1"));
        size_t MiOpenResult=MI.Open(Assetmap_FileName);
        MI.Option(__T("ParseSpeed"), ParseSpeed_Save); //This is a global value, need to reset it. TODO: local value
        MI.Option(__T("Demux"), Demux_Save); //This is a global value, need to reset it. TODO: local value
        if (MiOpenResult
            && (MI.Get(Stream_General, 0, General_Format)==__T("DCP AM")
            || MI.Get(Stream_General, 0, General_Format)==__T("IMF AM")))
        {
            MergeFromAm(((File_DcpAm*)MI.Info)->Streams);
        }
    }

    ReferenceFiles->FilesForStorage=true;

    //All should be OK...
    return true;
}

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
void File_DcpCpl::MergeFromAm (File_DcpPkl::streams &StreamsToMerge)
{
    for (File_DcpPkl::streams::iterator StreamToMerge=StreamsToMerge.begin(); StreamToMerge!=StreamsToMerge.end(); ++StreamToMerge)
        if (!StreamToMerge->ChunkList.empty()) // Note: ChunkLists with more than 1 file are not yet supported)
            ReferenceFiles->UpdateFileName(Ztring().From_UTF8(StreamToMerge->Id), Ztring().From_UTF8(StreamToMerge->ChunkList[0].Path));
}

} //NameSpace

#endif //MEDIAINFO_DCP_YES
