/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef Export_PBCore2H
#define Export_PBCore2H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
/// @brief Export_PBCore 2.0
//***************************************************************************

class Export_PBCore2
{
public :
    //Constructeur/Destructeur
    Export_PBCore2 ();
    ~Export_PBCore2 ();

    //Input
    Ztring Transform(MediaInfo_Internal &MI);
};

} //NameSpace
#endif
