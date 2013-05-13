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
#ifndef Reader__BaseH
#define Reader__BaseH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
/// @brief Reader__Base
//***************************************************************************

class Reader__Base
{
public :
    //Constructor/Destructor
    virtual ~Reader__Base() {}

    //Format testing
    virtual size_t Format_Test(MediaInfo_Internal* MI, String File_Name)=0;
    virtual size_t Format_Test_PerParser_Continue (MediaInfo_Internal* /*MI*/) {return 0;};
    #if MEDIAINFO_SEEK
    virtual size_t Format_Test_PerParser_Seek (MediaInfo_Internal*, size_t, int64u, int64u) {return 0;};
    #endif //MEDIAINFO_SEEK
};

} //NameSpace
#endif
