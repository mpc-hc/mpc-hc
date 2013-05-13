/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef MediaInfo_Ps2AudioH
#define MediaInfo_Ps2AudioH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Ps2Audio
//***************************************************************************

class File_Ps2Audio : public File__Analyze
{
private :
    //Buffer - Global
    void Read_Buffer_Continue();

    //Elements
    void SSbd();
    void SShd();

    //Temp
    int32u BitRate;
};

} //NameSpace

#endif
