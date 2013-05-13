/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about APETAG tagged files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_ApeTagH
#define MediaInfo_File_ApeTagH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_ApeTag
//***************************************************************************

class File_ApeTag : public File__Analyze
{
private :
    //Buffer - File header
    void FileHeader_Parse();

    //Buffer - Per element
    bool Header_Begin();
    void Header_Parse();
    void Data_Parse();

    //Elements
    void HeaderFooter();

    //Temp
    std::string Key;
};

} //NameSpace

#endif
