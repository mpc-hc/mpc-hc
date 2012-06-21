// ZenLib::Format::Http::Request - A HTTP request
// Copyright (C) 2008-2011 MediaArea.net SARL, Info@MediaArea.net
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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




