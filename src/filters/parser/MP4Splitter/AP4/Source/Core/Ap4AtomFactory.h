/*****************************************************************
|
|    AP4 - Atom Factory
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

#ifndef _AP4_ATOM_FACTORY_H_
#define _AP4_ATOM_FACTORY_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Atom.h"
#include "Ap4Array.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ByteStream;

/*----------------------------------------------------------------------
|   AP4_AtomFactory
+---------------------------------------------------------------------*/
class AP4_AtomFactory
{
public:
    // types
    class TypeHandler
    {
    public:
        virtual ~TypeHandler() {};
        virtual AP4_Result CreateAtom(AP4_Atom::Type  type,
                                      AP4_UI32        size,
                                      AP4_ByteStream& stream,
                                      AP4_Atom::Type  context,
                                      AP4_Atom*&      atom) = 0;
    };

    // constructor
    AP4_AtomFactory() {}

    // destructor
    ~AP4_AtomFactory();

    // methods
    AP4_Result AddTypeHandler(TypeHandler* handler);
    AP4_Result RemoveTypeHandler(TypeHandler* handler);
    AP4_Result CreateAtomFromStream(AP4_ByteStream&  stream,
                                    AP4_LargeSize&   bytes_available,
                                    AP4_Atom*&       atom);
    AP4_Result CreateAtomFromStream(AP4_ByteStream&  stream,
                                    AP4_Atom*&       atom);
    AP4_Result CreateAtomsFromStream(AP4_ByteStream& stream,
                                     AP4_AtomParent& atoms);
    AP4_Result CreateAtomsFromStream(AP4_ByteStream& stream,
                                     AP4_LargeSize   bytes_available,
                                     AP4_AtomParent& atoms);

    // context
    void PushContext(AP4_Atom::Type context);
    void PopContext();
    AP4_Atom::Type GetContext(AP4_Ordinal depth = 0);

private:
    // members
    AP4_Array<AP4_Atom::Type> m_ContextStack;
    AP4_List<TypeHandler>     m_TypeHandlers;
};

/*----------------------------------------------------------------------
|   AP4_DefaultAtomFactory
+---------------------------------------------------------------------*/
class AP4_DefaultAtomFactory : public AP4_AtomFactory
{
public:
    // class members
    static AP4_DefaultAtomFactory Instance;

    // constructor
    AP4_DefaultAtomFactory();
};

#endif // _AP4_ATOM_FACTORY_H_
