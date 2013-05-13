/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Give information about a lot of media files
// Dispatch the file to be tested by all containers
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef Reader_DirectoryH
#define Reader_DirectoryH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Reader/Reader__Base.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
/// @brief Reader_Directory
//***************************************************************************

class Reader_Directory : public Reader__Base
{
public :
    //Constructor/Destructor
    virtual ~Reader_Directory() {}

    //Format testing
    size_t Format_Test(MediaInfo_Internal* MI, String File_Name);

    //For the list
    void Directory_Cleanup(ZtringList &List);

private :
    //Bdmv
    int  Bdmv_Format_Test(MediaInfo_Internal* MI, const String &File_Name);
    void Bdmv_Directory_Cleanup(ZtringList &List);

    //P2
    int  P2_Format_Test(MediaInfo_Internal* MI, const String &File_Name);
    void P2_Directory_Cleanup(ZtringList &List);

    //XDCAM
    int  Xdcam_Format_Test(MediaInfo_Internal* MI, const String &File_Name);
    void Xdcam_Directory_Cleanup(ZtringList &List);
};

} //NameSpace
#endif
