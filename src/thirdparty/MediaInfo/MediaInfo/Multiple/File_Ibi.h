/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Ibi files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_IbiH
#define MediaInfo_File_IbiH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Classe File_Ibi
//***************************************************************************

class File_Ibi : public File__Analyze
{
public :
    File_Ibi();
    ~File_Ibi();

    //In
    #if MEDIAINFO_IBIUSAGE
        ibi* Ibi;
    #endif //MEDIAINFO_IBIUSAGE

private :
    #if MEDIAINFO_IBIUSAGE
        //Get information
        const Ztring &Get (stream_t StreamKind, size_t StreamNumber, const Ztring &Parameter, info_t KindOfInfo=Info_Text, info_t KindOfSearch=Info_Name);
    #endif //MEDIAINFO_IBIUSAGE

    //Streams management
    void Streams_Accept();
    #if MEDIAINFO_IBIUSAGE
    void Streams_Finish();
    #endif //MEDIAINFO_IBIUSAGE

    //Buffer - Element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Zero();
    void CRC32();
    void Void();
    void Ebml();
    void Ebml_Version();
    void Ebml_ReadVersion();
    void Ebml_MaxIDLength();
    void Ebml_MaxSizeLength();
    void Ebml_DocType();
    void Ebml_DocTypeVersion();
    void Ebml_DocTypeReadVersion();
    void Stream();
    void Stream_Header();
    void Stream_ByteOffset();
    void Stream_FrameNumber();
    void Stream_Dts();
    void CompressedIndex();
    void WritingApplication();
    void WritingApplication_Name();
    void WritingApplication_Version();
    void InformData();
    void SourceInfo();
    void SourceInfo_IndexCreationDate();
    void SourceInfo_SourceModificationDate();
    void SourceInfo_SourceSize();

    //Data
    int64u   UInteger_Get();
    int128u  UInteger16_Get();
    void     UInteger_Info();

    //Temp
    Ztring  Get_Temp;
    int64u  ID_Current;
    #if MEDIAINFO_IBIUSAGE
        bool    Ibi_MustDelete;
    #endif //MEDIAINFO_IBIUSAGE
};

} //NameSpace

#endif

