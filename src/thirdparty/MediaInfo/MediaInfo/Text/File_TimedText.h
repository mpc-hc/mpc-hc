/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Timed Text (MPEG-4 Part 17)
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_TimedTextH
#define MediaInfo_File_TimedTextH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Sami
//***************************************************************************

class File_TimedText : public File__Analyze
{
public:
    //Constructor/Destructor
    File_TimedText();

    #ifdef MEDIAINFO_MPEG4_YES
        bool IsChapter;
    #endif //MEDIAINFO_MPEG4_YES

private :
    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();
};

} //NameSpace

#endif

