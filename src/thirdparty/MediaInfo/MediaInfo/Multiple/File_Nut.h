/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Nut files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_NutH
#define MediaInfo_File_NutH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Nut
//***************************************************************************

class File_Nut : public File__Analyze
{
private :
    //Buffer
    void Header_Parse();
    void FileHeader_Parse();
    void Data_Parse();

    //Elements
    void main();
    void stream();
    void syncpoint();
    void index();
    void info();
};

} //NameSpace

#endif

