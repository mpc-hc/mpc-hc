/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

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
