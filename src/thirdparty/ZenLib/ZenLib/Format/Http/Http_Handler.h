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
#ifndef ZenLib_Format_Http_RequestH
#define ZenLib_Format_Http_RequestH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Format/Http/Http_Cookies.h"
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

class Handler
{
public:
    //Constructor/Destructor
    Handler();

    //In
    std::string                         Path;                   //The path being requested by this request
    std::map<std::string, std::string>  Request_Headers;        //All the incoming HTTP headers from the client web browser.
    std::map<std::string, std::string>  Request_Cookies;        //The set of cookies that came from the client along with this request
    std::map<std::string, std::string>  Request_Queries;        //All the key/value pairs in the query string of this request
    std::string                         Foreign_IP;             //The foreign ip address for this request
    std::string                         Local_IP;               //The foreign port number for this request
    unsigned short                      Foreign_Port;           //The IP of the local interface this request is coming in on
    unsigned short                      Local_Port;             //The local port number this request is coming in on
    bool                                HeadersOnly;            //The request requests only the header

    //Out
    size_t                              Response_HTTP_Code;     //HTTP code to be sent
    std::map<std::string, std::string>  Response_Headers;       //Additional headers you wish to appear in the HTTP response to this request
    Cookies                             Response_Cookies;       //New cookies to pass back to the client along with the result of this request
    std::string                         Response_Body;          //To be displayed as the response to this request
};

} //Namespace

} //Namespace

} //Namespace

#endif
