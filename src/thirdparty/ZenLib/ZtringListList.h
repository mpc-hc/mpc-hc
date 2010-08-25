// ZenLib::ZtringListList - More methods for std::vector<std::vector<std::(w)string>>
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
// More methods for std::vector<std::vector<std::(w)string>>
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_ZtringListListH
#define ZenLib_ZtringListListH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/ZtringList.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
/// @brief Vector of vectors of strings manipulation (based on std::vector<std::vector<std::(w)string>>)
//***************************************************************************

class ZtringListList : public std::vector<ZtringList>
{
public :
    //Constructors/Destructor
    ZtringListList ();
    ZtringListList (const ZtringListList &Source);
    ZtringListList (const Ztring &Source);
    ZtringListList (const Char *Source);
    #ifdef _UNICODE
    ZtringListList (const char *Source); //convert a UTF-8 string into Unicode
    #endif

    //Operateurs
    bool            operator == (const ZtringListList &Source) const;
    bool            operator != (const ZtringListList &Source) const;
    ZtringListList &operator += (const ZtringListList &Source);
    ZtringListList &operator =  (const ZtringListList &Source);

    ZtringList     &operator () (size_type Pos0); ///< Same as [], but write a empty string if Pos doesn't exist yet
    Ztring         &operator () (size_type Pos0, size_type Pos1);
    /// @brief Return [Pos][Pos1], Pos=First occurency of Pos0 in [xxx][0]
    Ztring         &operator () (const Ztring &Pos0, size_type Pos1=1)             {return operator() (Pos0, 0 , Pos1);};
    /// @brief Return [Pos][Pos1], Pos=First occurency of Pos0 in [xxx][Pos0_1]
    Ztring         &operator () (const Ztring &Pos0, size_type Pos0_1, size_type Pos1);

    //In/Out
    /// @brief Read all
    Ztring Read () const;
    /// @brief Read a vector of string
    Ztring Read (size_type Pos0) const;
    /// @brief Read a string
    const Ztring &Read (size_type Pos0, size_type Pos1) const;
    /// @brief Return [Pos][Pos1], Pos=First occurency of Pos0 in [xxx][0]
    const Ztring &Read (const Ztring &Pos0, size_type Pos1=1) const;
    /// @brief Return [Pos][Pos1], Pos=First occurency of Pos0 in [xxx][Pos0_1]
    const Ztring &Read (const Ztring &Pos0, size_type Pos0_1, size_type Pos1) const;
    /// @brief Return [Pos][Pos1], Pos=First occurency of Pos0 in [xxx][0], with default value
    const Ztring &Read (const Ztring &Pos0, const Ztring &Default, size_type Pos1=1) const;
    /// @brief Return [Pos][Pos1], Pos=First occurency of Pos0 in [xxx][Pos0_1], with default value
    const Ztring &Read (const Ztring &Pos0, const Ztring &Default, size_type Pos0_1, size_type Pos1) const;
    /// @brief Read all strings at position Pos1
    Ztring Read1 (size_type Pos1) const;

    /// @brief Write all
    void Write (const Ztring &ToWrite);
    /// @brief Write a vector of string
    void Write (const ZtringList &ToWrite, size_type Pos0);
    /// @brief Write a vector of string
    void Write (const Ztring &ToWrite, size_type Pos0, size_type Pos1);
    /// @brief Add a vector of string
    void push_back (const ZtringList &ToAdd);
    /// @brief Add a vector of string (with separator is ZtringListList Separator)
    void push_back (const Ztring &ToAdd);
    /// @brief Add a vector of string (Char version)
    void push_back (const Char* ToAdd)                                          {push_back(Ztring(ToAdd));};
    /// @brief Insert a vector of string at position Pos0
    void Insert (const ZtringList &ToInsert, size_type Pos0)                       {insert(begin()+Pos0, ToInsert);};
    /// @brief Insert a string at all positions Pos1
    void Insert1 (const Ztring &ToInsert, size_type Pos1);
    /// @brief Delete a vector of string at position Pos0
    void Delete (size_type Pos0)                                                   {erase(begin()+Pos0);};
    /// @brief Delete all vectors of string, with [xxx][Pos1] == ToFind
    void Delete (const Ztring &ToFind, size_type Pos1=0, const Ztring &Comparator=_T("=="), ztring_t Options=Ztring_Nothing);
    /// @brief Delete a string at all positions Pos1
    void Delete1 (size_type Pos1);

    //Edition
    /// @brief Swap 2 positions
    void Swap (size_type Pos0_A, size_type Pos0_B);
    /// @brief Swap 2 columns for each line
    void Swap1 (size_type Pos1_A, size_type Pos1_B);
    /// @brief Sort
    void Sort (size_type Pos1, ztring_t Options=Ztring_Nothing);

    //Information
    /// @brief Find the first position of the string in the vector of vector, in a specific column
    size_type Find (const Ztring &ToFind, size_type Pos1=0, size_type Pos0Begin=0) const;
    /// @brief Find the first position of the string in the vector of vector, in a specific column, which is not empty
    size_type Find_Filled (size_type Pos1=0, size_type Pos0Begin=0) const;
    /// @brief Find the first position of the string in the vector of vector, in a specific column
    size_type Find (const Ztring &ToFind, size_type Pos1, size_type Pos0Begin, const Ztring &Comparator, ztring_t Options=Ztring_Nothing) const;
    /// @brief Return [xxx][Pos1Value] when founded the first position of the string in the vector of vector, in a specific column
    Ztring FindValue (const Ztring &ToFind, size_type Pos1Value=1, size_type Pos1=0, size_type Pos0Begin=0, const Ztring &Comparator=_T("=="), ztring_t Options=Ztring_Nothing) const;
    /// @brief Return a subsheet, with all lines with position of the string in the vector of vector, in a specific column
    ZtringListList SubSheet (const Ztring &ToFind, size_type Pos1=0, size_type Pos0Begin=0, const Ztring &Comparator=_T("=="), ztring_t Options=Ztring_Nothing) const;

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

