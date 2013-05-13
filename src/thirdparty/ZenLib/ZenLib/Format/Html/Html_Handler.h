/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// A HTML Handler
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_Server_Html_HandlerH
#define ZenLib_Server_Html_HandlerH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <ctime>
#include <map>
#include <vector>
#include "ZenLib/Ztring.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

namespace Format
{

namespace Html
{

//***************************************************************************
/// @brief
//***************************************************************************

struct header
{
    //In
    Ztring  Title;
    Ztring  Language;

    //Init
    header()
    {
    }
};

class Handler
{
public:
    //Constructor/Destructor
    Handler();

    //Maintenance
    void CleanUp();

    //The data
    header  Header;
};

} //Namespace

} //Namespace

} //Namespace

#define ENDL "\r\n"
#define HTML_ENDL "<br />\r\n"

#endif
