/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Blu-ray Movie files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_BdmvH
#define MediaInfo_File_BdmvH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <set>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Bdmv
//***************************************************************************

class File_Bdmv : public File__Analyze
{
public :
    void BDMV(); //The BDMV directory

private :
    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Global
    void Read_Buffer_Continue ();

    //Elements
    void Clpi_ProgramInfo();
    void Clpi_ExtensionData();
    void Indx_AppInfoBDMV();
    void Indx_Indexes();
    void Indx_Indexes_Index(int8u object_type);
    void Indx_ExtensionData();
    void Indx_ExtensionData_IDEX();
    void Indx_ExtensionData_IDEX_UIAppInfoAVCHD();
    void Indx_ExtensionData_IDEX_TableOfPlayLists();
    void Indx_ExtensionData_IDEX_MakersPrivateData();
    void Mobj_MovieObjects();
    void Mobj_ExtensionData();
    void Mpls_AppInfoPlayList();
    void Mpls_PlayList();
    void Mpls_PlayList_PlayItem();
    void Mpls_PlayList_PlayItem_STN_table();
    void Mpls_PlayList_PlayItem_STN_table_Video();
    void Mpls_PlayList_PlayItem_STN_table_Audio();
    void Mpls_PlayList_PlayItem_STN_table_Text();
    void Mpls_PlayList_SubPlayItem(int8u SubPath_type, int16u Pos);
    void Mpls_PlayListMarks();
    void Mpls_ExtensionData();
    void Mpls_ExtensionData_SubPath_entries();
    void StreamCodingInfo_Video();
    void StreamCodingInfo_Audio();
    void StreamCodingInfo_Text();

    //Temp
    int8u stream_type;
    std::map<int32u, size_t> Types; //Key is the start address
    int64u Mpls_PlayList_Duration;
    int64u Mpls_PlayList_PlayItem_Duration;
    int16u Mpls_PlayList_number_of_SubPaths;
    bool   Mpls_PlayList_IsParsed;
    std::set<Ztring> Clip_Information_file_names;
};

} //NameSpace

#endif

