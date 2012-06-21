// ZenLib::Format::Http::Cookies - Cookies handling
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
// Cookies handling
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_Format_Http_CookiesH
#define ZenLib_Format_Http_CookiesH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <string>
#include <ctime>
#include <map>
#include <sstream>
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

struct Cookie
{
    std::string Value;
    std::time_t Expires;
    std::string Path;
    std::string Domain;
    bool Secure;

    Cookie()
    {
        Expires=0;
        Secure=false;
    }
};
 
extern std::string EmptyString; //Must not change

class Cookies : public std::map<std::string, Cookie>
{
public :
    //Constructor/Destructor
    Cookies();

    //Helpers
    size_t Set(const std::string &Name, const std::string &Value=EmptyString, std::time_t Expires=(std::time_t)-1, const std::string &Path=EmptyString, const std::string &Domain=EmptyString, bool Secure=false);
    Cookie &Get(const std::string &Name);
    void Create_Lines(std::ostream& Out);
};

} //Namespace

} //Namespace

} //Namespace

#endif




