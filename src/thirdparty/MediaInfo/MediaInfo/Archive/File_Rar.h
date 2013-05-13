/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about RAR files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_RarH
#define MediaInfo_File_RarH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Rar
//***************************************************************************

class File_Rar : public File__Analyze
{
public :
    File_Rar();

protected :
    int state;

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Per element
    bool Header_Begin();
    void Header_Parse();
    void Header_Parse_Flags();
    void Header_Parse_Flags_73();
    void Header_Parse_Flags_74();
    void Header_Parse_Flags_XX();
    void Header_Parse_Content();
    void Header_Parse_Content_73();
    void Header_Parse_Content_74();
    void Header_Parse_Content_XX();
    void Data_Parse();

    //Temp
    int8u  HEAD_TYPE;
    int32u PACK_SIZE;
    int32u HIGH_PACK_SIZE;
    int16u HEAD_FLAGS;
    bool   high_fields;
    bool   usual_or_utf8;
    bool   salt;
    bool   exttime;
    bool   add_size;
};

} //NameSpace

#endif
