/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

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
    size_type Find (const Ztring &ToFind, size_type PosBegin=0, const Ztring &Comparator=__T("=="), ztring_t Options=Ztring_Nothing) const;
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
