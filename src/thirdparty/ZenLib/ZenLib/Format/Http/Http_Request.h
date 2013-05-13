/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// A HTTP Request
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_Server_Http_RequestH
#define ZenLib_Server_Http_RequestH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Format/Http/Http_Handler.h"
#include <string>
#include <ctime>
#include <map>
#include <vector>
//---------------------------------------------------------------------------


namespace ZenLib
{

namespace Format
{

namespace Http
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
    ZenLib::Format::Http::Handler   *Http;
    bool                            IsCopy;

    //Helpers
    bool Http_Begin(std::istream &In, std::ostream &Out);
    void Http_End  (std::ostream &Out);
};

} //Namespace

} //Namespace

} //Namespace

#endif
