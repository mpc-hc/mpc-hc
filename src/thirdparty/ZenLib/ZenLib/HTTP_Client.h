/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Basic HTTP client
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_HTTPClientH
#define ZenLib_HTTPClientH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Ztring.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
/// @brief Basic HTTP client
//***************************************************************************

class HTTP_Client
{
public :
    //Constructor/Destructor
    HTTP_Client  ();
    ~HTTP_Client ();

    //Open/Close
    int  Open  (Ztring URL);
    void Close ();

    //Read
    Ztring Read();

private :
    int32u Handle;
};

} //NameSpace

#endif
