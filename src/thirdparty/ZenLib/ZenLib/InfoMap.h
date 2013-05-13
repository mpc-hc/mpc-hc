/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef ZenLib_InfoMapH
#define ZenLib_InfoMapH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/ZtringList.h"
#include <map>
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
/// @brief Helper for InfoMap
//***************************************************************************

class InfoMap : public std::multimap<Ztring, ZtringList>
{
public :
    //Constructors/Destructor
    InfoMap ();
    InfoMap (const Ztring &Source);
    InfoMap (const Char *Source);
    #ifdef _UNICODE
    InfoMap (const char *Source); //convert a UTF-8 string into Unicode
    #endif

    //In/Out
    const Ztring   &Get (const Ztring &Value, size_t Pos) const;
    const Ztring   &Get (const Ztring &Value, size_t Pos, const Ztring &WithValue, size_t WithValue_Pos) const;
          void      Write (const Ztring &NewLanguage);

    //Configuration
    /// @brief Set the Separator character
    void Separator_Set (size_type Level, const Ztring &NewSeparator);
    /// @brief Set the Quote character
    /// During Read() or Write() method, if Separator is in the sequence, we must quote it
    void Quote_Set (const Ztring &NewQuote);
    /// @brief Set the Maximum number of element to read
    /// During Read() or Write() method, if there is more elements, merge them with the last element
    void Max_Set (size_type Level, size_type Max);

protected :
    Ztring Separator[2];
    Ztring Quote;
    size_type Max[2];
};

} //namespace
#endif
