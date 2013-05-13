/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about SCC (Scenarist Closed Captioning) files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_SccH
#define MediaInfo_File_SccH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Scc
//***************************************************************************

class File_Scc : public File__Analyze
{
public :
    File_Scc();
    ~File_Scc();

private :
    //Streams management
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Temp
    File__Analyze* Parser;
};

} //NameSpace

#endif
