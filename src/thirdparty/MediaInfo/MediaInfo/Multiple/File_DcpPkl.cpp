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
#include "MediaInfo/Multiple/File_DcpPkl.h"
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
File_DcpPkl::File_DcpPkl()
:File__Analyze()
{
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_DcpPkl;
        StreamIDs_Width[0]=sizeof(size_t)*2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_EventWasSent_Accept_Specific=true;
    #endif //MEDIAINFO_DEMUX

    //Temp
    ReferenceFiles=NULL;
    HasCpl=false;
}

//---------------------------------------------------------------------------
File_DcpPkl::~File_DcpPkl()
{
    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_DcpPkl::Streams_Finish()
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
size_t File_DcpPkl::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
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
bool File_DcpPkl::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    bool IsDcp=false, IsImf=false;

    XMLElement* PackingList=document.FirstChildElement("PackingList");
    if (!PackingList)
    {
        Reject("DcpPkl");
        return false;
    }

    const char* Attribute=PackingList->Attribute("xmlns");
    if (!Attribute)
    {
        Reject("DcpPkl");
        return false;
    }

    if (!strcmp(Attribute, "http://www.digicine.com/PROTO-ASDCP-PKL-20040311#"))
        IsDcp=true;
    if (!strcmp(Attribute, "http://www.smpte-ra.org/schemas/429-8/2007/PKL"))
        IsImf=true;

    if (!IsDcp && !IsImf)
    {
        Reject("DcpPkl");
        return false;
    }

    Accept("DcpPkl");
    Fill(Stream_General, 0, General_Format, IsDcp?"DCP PKL":"IMF PKL");
    Config->File_ID_OnlyRoot_Set(false);

    ReferenceFiles=new File__ReferenceFilesHelper(this, Config);
    Ztring CPL_FileName;

    //Parsing main elements
    for (XMLElement* PackingList_Item=PackingList->FirstChildElement(); PackingList_Item; PackingList_Item=PackingList_Item->NextSiblingElement())
    {
        //AssetList
        if (!strcmp(PackingList_Item->Value(), "AssetList"))
        {
            for (XMLElement* AssetList_Item=PackingList_Item->FirstChildElement(); AssetList_Item; AssetList_Item=AssetList_Item->NextSiblingElement())
            {
                //Asset
                if (!strcmp(AssetList_Item->Value(), "Asset"))
                {
                    File__ReferenceFilesHelper::reference ReferenceFile;
                    bool IsCPL=false;
                    bool PreviousFileNameIsAnnotationText=false;

                    for (XMLElement* File_Item=AssetList_Item->FirstChildElement(); File_Item; File_Item=File_Item->NextSiblingElement())
                    {
                        //Id
                        if (!strcmp(File_Item->Value(), "Id"))
                            ReferenceFile.Infos["UniqueID"].From_UTF8(File_Item->GetText());

                        //Type
                        if (!strcmp(File_Item->Value(), "Type"))
                        {
                                    if (!strcmp(File_Item->GetText(), "application/x-smpte-mxf;asdcpKind=Picture"))
                                ReferenceFile.StreamKind=Stream_Video;
                            else if (!strcmp(File_Item->GetText(), "application/x-smpte-mxf;asdcpKind=Sound"))
                                ReferenceFile.StreamKind=Stream_Audio;
                            else if (!strcmp(File_Item->GetText(), "text/xml;asdcpKind=CPL"))
                            {
                                HasCpl=IsCPL=true;
                            }
                            else
                                ReferenceFile.StreamKind=Stream_Other;
                        }

                        //Id
                        if (!strcmp(File_Item->Value(), "OriginalFileName")
                         || (ReferenceFile.FileNames.empty() && !strcmp(File_Item->Value(), "AnnotationText"))) // Annotation contains file name (buggy IMF file)
                        {
                            if (PreviousFileNameIsAnnotationText)
                                ReferenceFile.FileNames.clear(); // Annotation is something else, no need of it
                            if (!strcmp(File_Item->Value(), "AnnotationText"))
                                PreviousFileNameIsAnnotationText=true;
                            ReferenceFile.FileNames.push_back(Ztring().From_UTF8(File_Item->GetText()));
                            string Text=File_Item->GetText();
                            if (Text.size()>=8
                             && (Text.find("_cpl.xml")==Text.size()-8)
                              || Text.find("CPL_")==0 && Text.find(".xml")==Text.size()-4)
                            {
                                HasCpl=IsCPL=true;
                                ReferenceFile.StreamKind=Stream_Max;
                            }
                        }
                    }

                    if (IsCPL && CPL_FileName.empty())
                        for (size_t Pos=0; Pos<ReferenceFile.FileNames.size(); Pos++)
                        {
                            CPL_FileName=ReferenceFile.FileNames[Pos]; //Using only the first CPL file meet
                            break;
                        }

                    ReferenceFile.StreamID=ReferenceFiles->References.size()+1;
                    ReferenceFiles->References.push_back(ReferenceFile);
                }
            }
        }
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
            && ((IsDcp && MI.Get(Stream_General, 0, General_Format)==__T("DCP CPL"))
            || (IsImf && MI.Get(Stream_General, 0, General_Format)==__T("IMF CPL"))))
        {
            DcpCpl_MergeFromPkl(((File_DcpCpl*)MI.Info)->ReferenceFiles, ReferenceFiles);
            ReferenceFiles->References=((File_DcpCpl*)MI.Info)->ReferenceFiles->References;
        }
    }

    ReferenceFiles->FilesForStorage=true;

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_DCP_YES

