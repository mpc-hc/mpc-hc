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
#ifndef Reader_libmmsH
#define Reader_libmmsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Reader/Reader__Base.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
/// @brief Reader_libmms
//***************************************************************************

class Reader_libmms : public Reader__Base
{
public :
    //Constructor/Destructor
    virtual ~Reader_libmms() {}

    //Format testing
    size_t Format_Test(MediaInfo_Internal* MI, String File_Name);
};

} //NameSpace
#endif
