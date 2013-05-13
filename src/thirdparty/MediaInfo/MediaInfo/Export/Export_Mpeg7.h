/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef Export_Mpeg7H
#define Export_Mpeg7H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
/// @brief Export_Mpeg7
//***************************************************************************

class Export_Mpeg7
{
public :
    //Constructeur/Destructeur
    Export_Mpeg7 ();
    ~Export_Mpeg7 ();

    //Input
    Ztring Transform(MediaInfo_Internal &MI);
};

} //NameSpace
#endif
