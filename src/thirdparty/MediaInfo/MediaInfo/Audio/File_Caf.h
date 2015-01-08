/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Core Audio Format files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_CafH
#define MediaInfo_File_CafH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Au
//***************************************************************************

class File_Caf : public File__Analyze
{
public :
    File_Caf();

protected :
    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse ();

    //Buffer
    void Header_Parse();
    void Data_Parse();

    //Elements
    void data();
    void desc();
    void free();
    void info();
    void kuki();
    void pakt();
    void uuid();
};

} //NameSpace

#endif
