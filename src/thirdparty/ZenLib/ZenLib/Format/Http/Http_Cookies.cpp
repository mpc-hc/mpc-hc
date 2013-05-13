/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#include "ZenLib/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf_Internal.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Format/Http/Http_Cookies.h"
#include "ZenLib/Format/Http/Http_Utils.h"
using namespace std;
//---------------------------------------------------------------------------

namespace ZenLib
{

namespace Format
{

namespace Http
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
Cookies::Cookies()
{
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
size_t Cookies::Set(const std::string &Name, const std::string &Value, std::time_t Expires, const std::string &Path, const std::string &Domain, bool Secure)
{
    //Name must not be empty
    if (Name.empty())
        return 1;

    //Default values handling
    if (Value.empty())
        Expires=time(NULL)-365*24*60*60-1; //minus 1 year

    //Default value
    if (Expires==(time_t)-1)
        Expires=time(NULL)+1*365*24*60*60; //+1 year

    //Expires can be the count of seconds to handle instead of real time
    if (Expires>=0 && Expires<3*365*24*60*60) //Les than year 1973, this is not a date, this is a time
        Expires+=time(NULL);

    //Registering
    Cookie Data;
    Data.Value=Value.empty()?std::string("Deleted"):URL_Encoded_Encode(Value); //If no value, we force a default value for having the cookie understable
    Data.Expires=Expires;
    Data.Path=URL_Encoded_Encode(Path.empty()?string("/"):Path);
    Data.Domain=URL_Encoded_Encode(Domain);
    Data.Secure=Secure;
    (*this)[URL_Encoded_Encode(Name)]=Data;

    return 0;
}

//---------------------------------------------------------------------------
void Cookies::Create_Lines(std::ostream& Out)
{
    for (Cookies::iterator Cookie=begin(); Cookie!=end(); ++Cookie)
    {
        Out << "Set-Cookie: " << Cookie->first << "=" << Cookie->second.Value;
        if (Cookie->second.Expires!=(time_t)-1)
        {
            char Temp[200];
            if (strftime(Temp, 200, "%a, %d-%b-%Y %H:%M:%S GMT", gmtime(&Cookie->second.Expires)))
                Out << "; expires=" << Temp;
        }
        if (!Cookie->second.Path.empty())
        {
            Out << "; path=" << Cookie->second.Path;
        }
        Out << "\r\n";
    }
}

} //Namespace

} //Namespace

} //Namespace
