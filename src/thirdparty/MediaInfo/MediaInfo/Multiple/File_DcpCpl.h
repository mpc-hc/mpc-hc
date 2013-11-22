/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about DCP/IMF Composition Playlist files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DcpCplH
#define MediaInfo_File_DcpCplH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class File__ReferenceFilesHelper;

//***************************************************************************
// Class File_DcpCpl
//***************************************************************************

class File_DcpCpl : public File__Analyze
{
public :
    //Constructor/Destructor
    File_DcpCpl();
    ~File_DcpCpl();

private :
    //Streams management
    void Streams_Finish ();

    //Buffer - Global
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK

    //Buffer - File header
    bool FileHeader_Begin();

    //Temp
    File__ReferenceFilesHelper*     ReferenceFiles;
    friend class File_DcpAm;    //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_DcpPkl;   //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
};

} //NameSpace

#endif

