/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef Export_EbuCoreH
#define Export_EbuCoreH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
/// @brief Export_EbuCore
//***************************************************************************

class Export_EbuCore
{
public :
    //Constructeur/Destructeur
    Export_EbuCore ();
    ~Export_EbuCore ();

    //Input
    Ztring Transform(MediaInfo_Internal &MI);
};

} //NameSpace
#endif
