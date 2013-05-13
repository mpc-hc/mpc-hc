/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Lagarith files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_LagarithH
#define MediaInfo_File_LagarithH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Lagarith
//***************************************************************************

class File_Lagarith : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Lagarith();

private :
    //Streams management
    void Streams_Fill();

    //Buffer - Global
    void Read_Buffer_Continue ();
};

} //NameSpace

#endif
