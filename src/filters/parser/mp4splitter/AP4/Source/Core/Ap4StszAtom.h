/*****************************************************************
|
|    AP4 - stsz Atoms 
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

#ifndef _AP4_STSZ_ATOM_H_
#define _AP4_STSZ_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Array.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|   AP4_StszAtom
+---------------------------------------------------------------------*/
class AP4_StszAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_StszAtom, AP4_Atom)

    // class methods
    static AP4_StszAtom* Create(AP4_Size size, AP4_ByteStream& stream);

    // methods
    AP4_StszAtom();
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_UI32   GetSampleCount();
    virtual AP4_Result GetSampleSize(AP4_Ordinal sample, 
                                     AP4_Size&   sample_size);
    /**
     * Set the sample size.
     * @param sample 1-based index of a sample.
     * @param sample_size Size of the sample.
     * If this table represents a global-size table (one where there are
     * no individual entries for each sample, but all sample have the same
     * size = m_SampleSize), then calling this method with sample=1 will update
     * the global size, and calling with sample>1 will check that
     * the value of sample_size is the same as the global size m_SampleSize.
     */
    virtual AP4_Result SetSampleSize(AP4_Ordinal sample, 
                                     AP4_Size    sample_size);
    virtual AP4_Result AddEntry(AP4_UI32 size);

private:
    // methods
    AP4_StszAtom(AP4_UI32        size, 
                 AP4_UI32        version,
                 AP4_UI32        flags,
                 AP4_ByteStream& stream);

    // members
    AP4_UI32            m_SampleSize;
    AP4_UI32            m_SampleCount;
    AP4_Array<AP4_UI32> m_Entries;
};

#endif // _AP4_STSZ_ATOM_H_
