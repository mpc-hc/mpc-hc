// ZenLib::Translation - Helper for translation
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
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
//
//
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_TranslationH
#define ZenLib_TranslationH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Ztring.h"
#include <map>
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
/// @brief Helper for translation
//***************************************************************************

class Translation : public std::map<Ztring, Ztring>
{
public :
    //Constructors/Destructor
    Translation ();
    Translation (const Ztring &Source);
    Translation (const Char *Source);
    #ifdef _UNICODE
    Translation (const char *Source); //convert a UTF-8 string into Unicode
    #endif

    //In/Out
          Ztring    Get () const;
    const Ztring   &Get (const Ztring &Value);
          Ztring    Get (const Ztring &Count, const Ztring &Value);
          void      Write (const Ztring &NewLanguage); //All language in one
          void      Write (const Ztring &Value, const Ztring &NewLanguage); //Per item

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

