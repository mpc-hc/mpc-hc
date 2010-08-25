// ZenLib::ZtringList - More methods for vector<std::(w)string>
// Copyright (C) 2002-2010 MediaArea.net SARL, Info@MediaArea.net
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
// More methods for std::vector<std::(w)string>
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_ZtringListH
#define ZenLib_ZtringListH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Ztring.h"
#include <vector>
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
/// @brief Vector of strings manipulation (based on std::vector<std::(w)string>)
//***************************************************************************

class ZtringList : public std::vector<Ztring>
{
public :
    //Constructors/destructor
    ZtringList ();
    ZtringList (const ZtringList &Source);
    ZtringList (const Ztring &Source);
    ZtringList (const Char *Source);
    #ifdef _UNICODE
    ZtringList (const char *Source); //convert a UTF-8 string into Unicode
    #endif

    //Operators
    bool        operator == (const ZtringList &Source) const;
    bool        operator != (const ZtringList &Source) const;
    ZtringList &operator += (const ZtringList &Source);
    ZtringList &operator =  (const ZtringList &Source);

    Ztring     &operator () (size_type Pos); ///< Same as [], but write a empty string if Pos doesn't exist yet

    //In/out
    Ztring Read () const; /// Read all
    const Ztring &Read (size_type Pos) const; /// Read a string
    void Write (const Ztring &ToWrite); /// Write all
    void Write (const Ztring &ToWrite, size_type Pos); /// Write a string
    /// @brief Insert a string at position Pos0
    void Insert (const Ztring &ToInsert, size_type Pos0)                           {insert(begin()+Pos0, ToInsert);};
    /// @brief Delete a string at position Pos0
    void Delete (size_type Pos0)                                                   {erase(begin()+Pos0);};

    //Edition
    /// @brief Swap 2 positions
    void Swap (size_type Pos0_A, size_type Pos0_B);
    /// @brief Sort
    void Sort (ztring_t Options=Ztring_Nothing);

    //Information
    /// @brief Find the position of the string in the vector
    size_type Find (const Ztring &ToFind, size_type PosBegin=0, const Ztring &Comparator=_T("=="), ztring_t Options=Ztring_Nothing) const;
    /// @brief Return the length of the longest string in the list.
    size_type MaxStringLength_Get ();

    //Configuration
    /// @brief Set the Separator character
    void Separator_Set (size_type Level, const Ztring &NewSeparator);
    /// @brief Set the Quote character
    /// During Read() or Write() method, if Separator is in the sequence, we must quote it
    void Quote_Set (const Ztring &NewQuote);
    /// @brief Set the Maximum number of element to read
    /// During Read() or Write() method, if there is more elements, merge them with the last element
    void Max_Set (size_type Level, size_type Max_New);

protected :
    Ztring Separator[1];
    Ztring Quote;
    size_type Max[1];
};

} //namespace
#endif

