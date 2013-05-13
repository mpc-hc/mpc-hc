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
#if defined(MEDIAINFO_DIRECTORY_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Reader/Reader_Directory.h"
#include "ZenLib/Dir.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

size_t Reader_Directory::Format_Test(MediaInfo_Internal* MI, String File_Name)
{
    #ifdef MEDIAINFO_BDMV_YES
        if (Dir::Exists(File_Name) && File_Name.find(Ztring(1, PathSeparator)+__T("BDMV"))+5==File_Name.size())
            return Bdmv_Format_Test(MI, File_Name);
    #endif //MEDIAINFO_BDMV_YES

    #ifdef MEDIAINFO_P2_YES
        if (Dir::Exists(File_Name) && File_Name.rfind(Ztring(1, PathSeparator)+__T("CONTENT"))+8==File_Name.size())
            return P2_Format_Test(MI, File_Name);
    #endif //MEDIAINFO_P2_YES

    #ifdef MEDIAINFO_XDCAM_YES
        if (Dir::Exists(File_Name) && File_Name.rfind(Ztring(1, PathSeparator)+__T("XDCAM"))+5==File_Name.size())
            return Xdcam_Format_Test(MI, File_Name);
    #endif //MEDIAINFO_XDCAM_YES

    return 0;
}

//---------------------------------------------------------------------------
void Reader_Directory::Directory_Cleanup(ZtringList &List)
{
    #ifdef MEDIAINFO_BDMV_YES
        Bdmv_Directory_Cleanup(List);
    #endif //MEDIAINFO_BDMV_YES

    #ifdef MEDIAINFO_P2_YES
        P2_Directory_Cleanup(List);
    #endif //MEDIAINFO_P2_YES

    #ifdef MEDIAINFO_XDCAM_YES
        Xdcam_Directory_Cleanup(List);
    #endif //MEDIAINFO_XDCAM_YES
}

//***************************************************************************
// Blu-ray stuff
//***************************************************************************

//---------------------------------------------------------------------------
#ifdef MEDIAINFO_BDMV_YES
int Reader_Directory::Bdmv_Format_Test(MediaInfo_Internal* MI, const String &File_Name)
{
    if (!MI->SelectFromExtension(__T("Bdmv")))
        return 0;

    MI->Open_Buffer_Init(0, File_Name);

    MI->Open_Buffer_Continue(NULL, 0);

    MI->Open_Buffer_Finalize();

    return 1;
}
#endif //MEDIAINFO_BDMV_YES

//---------------------------------------------------------------------------
#ifdef MEDIAINFO_BDMV_YES
void Reader_Directory::Bdmv_Directory_Cleanup(ZtringList &List)
{
    //if there is a BDMV folder, this is blu-ray
    Ztring ToSearch=Ztring(1, PathSeparator)+__T("BDMV")+PathSeparator+__T("index.bdmv"); //"\BDMV\index.bdmv"
    for (size_t File_Pos=0; File_Pos<List.size(); File_Pos++)
    {
        size_t BDMV_Pos=List[File_Pos].find(ToSearch);
        if (BDMV_Pos!=string::npos && BDMV_Pos!=0 && BDMV_Pos+16==List[File_Pos].size())
        {
            //This is a BDMV index, parsing the directory only if index and movie objects are BOTH present
            ToSearch=List[File_Pos];
            ToSearch.resize(ToSearch.size()-10);
            ToSearch+=__T("MovieObject.bdmv");  //"%CompletePath%\BDMV\MovieObject.bdmv"
            if (List.Find(ToSearch)!=string::npos)
            {
                //We want the folder instead of the files
                List[File_Pos].resize(List[File_Pos].size()-11); //only %CompletePath%\BDMV
                ToSearch=List[File_Pos];

                for (size_t Pos=0; Pos<List.size(); Pos++)
                {
                    if (List[Pos].find(ToSearch)==0 && List[Pos]!=ToSearch) //Remove all subdirs of ToSearch but not ToSearch
                    {
                        //Removing the file in the blu-ray directory
                        List.erase(List.begin()+Pos);
                        Pos--;
                    }
                }
            }
        }
    }
}
#endif //MEDIAINFO_BDMV_YES

//***************************************************************************
// P2 stuff
//***************************************************************************

//---------------------------------------------------------------------------
#ifdef MEDIAINFO_P2_YES
int Reader_Directory::P2_Format_Test(MediaInfo_Internal* MI, const String &File_Name)
{
    if (!MI->SelectFromExtension(__T("P2_Clip")))
        return 0;

    MI->Open(File_Name+__T("CLIP")+PathSeparator+__T("0013MM.XML"));

    return 1;
}
#endif //MEDIAINFO_P2_YES

//---------------------------------------------------------------------------
#ifdef MEDIAINFO_P2_YES
void Reader_Directory::P2_Directory_Cleanup(ZtringList &List)
{
    //if there is a CONTENTS/CLIP folder, this is P2
    Ztring ToSearch=Ztring(1, PathSeparator)+__T("CONTENTS")+PathSeparator+__T("CLIP")+PathSeparator; //"/CONTENTS/CLIP/"
    for (size_t File_Pos=0; File_Pos<List.size(); File_Pos++)
    {
        size_t P2_Pos=List[File_Pos].find(ToSearch);
        if (P2_Pos!=string::npos && P2_Pos!=0 && P2_Pos+1+8+1+4+1+10==List[File_Pos].size())
        {
            //This is a P2 CLIP
            Ztring Path_Begin=List[File_Pos];
            Path_Begin.resize(Path_Begin.size()-(1+8+1+4+1+10));
            Path_Begin+=Ztring(1, PathSeparator);
            bool HasChanged=false;
            for (size_t Pos=0; Pos<List.size(); Pos++)
            {
                if (List[Pos].find(Path_Begin)==0 && List[Pos].find(Path_Begin+__T("CONTENTS")+PathSeparator+__T("CLIP")+PathSeparator)==string::npos) //Remove all subdirs of Path_Begin but not with CLIP
                {
                    //Removing the file in the P2 directory
                    List.erase(List.begin()+Pos);
                    Pos--;
                    HasChanged=true;
                }
            }
            if (HasChanged)
                File_Pos=0;
        }
    }
}
#endif //MEDIAINFO_P2_YES

//***************************************************************************
// P2 stuff
//***************************************************************************

//---------------------------------------------------------------------------
#ifdef MEDIAINFO_XDCAM_YES
int Reader_Directory::Xdcam_Format_Test(MediaInfo_Internal* MI, const String &File_Name)
{
    if (!MI->SelectFromExtension(__T("Xdcam_Clip")))
        return 0;

    MI->Open(File_Name+__T("CLIP")+PathSeparator+__T("0013MM.XML"));

    return 1;
}
#endif //MEDIAINFO_XDCAM_YES

//---------------------------------------------------------------------------
#ifdef MEDIAINFO_XDCAM_YES
void Reader_Directory::Xdcam_Directory_Cleanup(ZtringList &List)
{
    //if there is a XDCAM/Clip folder, this is Xdcam
    Ztring ToSearch=Ztring(1, PathSeparator)+__T("Clip")+PathSeparator; //"/Clip/"
    for (size_t File_Pos=0; File_Pos<List.size(); File_Pos++)
    {
        size_t Xdcam_Pos=List[File_Pos].find(ToSearch);
        if (Xdcam_Pos!=string::npos && Xdcam_Pos!=0 && Xdcam_Pos+1+4+1+12==List[File_Pos].size())
        {
            //This is a XDCAM CLIP
            Ztring Path_Begin=List[File_Pos];
            Path_Begin.resize(Path_Begin.size()-(1+4+1+12));
            Path_Begin+=Ztring(1, PathSeparator);
            if (Dir::Exists(Path_Begin+__T("Edit")) && Dir::Exists(Path_Begin+__T("General")) && Dir::Exists(Path_Begin+__T("Sub")))
            {
                bool HasChanged=false;
                for (size_t Pos=0; Pos<List.size(); Pos++)
                {
                    if (List[Pos].find(Path_Begin)==0 && (List[Pos].find(Path_Begin+__T("Clip")+PathSeparator)==string::npos || (List[Pos].find(__T(".XML"))!=List[Pos].size()-4))) //Remove all subdirs of Path_Begin but not with the right XML
                    {
                        //Removing the file in the XDCAM directory
                        List.erase(List.begin()+Pos);
                        Pos--;
                        HasChanged=true;
                    }
                }
                if (HasChanged)
                    File_Pos=0;
            }
        }
    }
}
#endif //MEDIAINFO_XDCAM_YES

} //NameSpace

#endif //MEDIAINFO_DIRECTORY_YES

