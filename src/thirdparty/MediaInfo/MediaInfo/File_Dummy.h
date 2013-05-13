/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Fill with Name of tags
// Used to give an example to the GUI of what MediaInfo can do
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DummyH
#define MediaInfo_File_DummyH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Dummy
//***************************************************************************

class File_Dummy : public File__Analyze
{
public :
    Ztring KindOfDummy;

protected :
    //Buffer - File header
    void FileHeader_Parse ();

private :
    void Fill_Dummy_General();
    void Fill_Dummy_Video();
    void Fill_Dummy_Audio();
    void Fill_Dummy_Text();
    void Fill_Dummy_Chapters();
};

} //NameSpace

#endif
