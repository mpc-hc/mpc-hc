/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about TTML files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_TtmlH
#define MediaInfo_File_TtmlH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace tinyxml2
{
    class XMLDocument;
    class XMLElement;
}

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Sami
//***************************************************************************

class File_Ttml : public File__Analyze
{
public :
    File_Ttml();
    ~File_Ttml();

private :
    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Global
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK
    void Read_Buffer_Continue();

    //Temp
    tinyxml2::XMLDocument*      document;
    tinyxml2::XMLElement*       div;
    tinyxml2::XMLElement*       p;
};

} //NameSpace

#endif
