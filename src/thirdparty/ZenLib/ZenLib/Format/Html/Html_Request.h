/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// A HTML Request
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_Server_Html_RequestH
#define ZenLib_Server_Html_RequestH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Format/Html/Html_Handler.h"
#include <ctime>
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

class Request
{
public:
    //Constructor/Destructor
    Request();
    Request(const Request &Req);
    ~Request();

    //The data
    ZenLib::Format::Html::Handler   *Html;
    bool                            IsCopy;
};

} //Namespace

} //Namespace

} //Namespace

#endif
