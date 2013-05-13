/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Speex files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_SpeexH
#define MediaInfo_File_SpeexH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Speex
//***************************************************************************

class File_Speex : public File__Analyze
{
public :
    File_Speex();

private :
    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Identification();
    void Comment();

    //Temp
    bool Identification_Done;
};

} //NameSpace

#endif
