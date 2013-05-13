/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Musepack files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_TwinVQH
#define MediaInfo_File_TwinVQH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_TwinVQ
//***************************************************************************

class File_TwinVQ : public File__Analyze
{
private :
    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void _c__() {_____char("Copyright");}
    void ALBM() {_____char("Album");}
    void AUTH() {_____char("Performer");}
    void COMM();
    void COMT() {_____char("Comment");}
    void DATA();
    void DSIZ();
    void FILE() {_____char();}
    void NAME() {_____char("Title");}

    //Helpers
    void _____char();
    void _____char(const char* Parameter);
};

} //NameSpace

#endif

