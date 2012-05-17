/*****************************************************************
|
|    AP4 - Atom Based Sample Table
|
|    Copyright 2003 Gilles Boccon-Gibod & Julien Boeuf
|
|
|    This atom is part of AP4 (MP4 Audio Proatom Library).
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
|    along with Bento4|GPL; see the atom COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

#ifndef _AP4_ATOM_SAMPLE_TABLE_H_
#define _AP4_ATOM_SAMPLE_TABLE_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4SampleTable.h"

/*----------------------------------------------------------------------
|       forward declarations
+---------------------------------------------------------------------*/
class AP4_Atom;
class AP4_ByteStream;
class AP4_StscAtom;
class AP4_StcoAtom;
class AP4_Co64Atom;
class AP4_StszAtom;
class AP4_SttsAtom;
class AP4_CttsAtom;
class AP4_StssAtom;
class AP4_StsdAtom;

/*----------------------------------------------------------------------
|       AP4_AtomSampleTable
+---------------------------------------------------------------------*/
class AP4_AtomSampleTable : public AP4_SampleTable
{
 public:
    // methods
             AP4_AtomSampleTable(AP4_ContainerAtom* stbl_atom, 
                                 AP4_ByteStream&    sample_stream);
    virtual ~AP4_AtomSampleTable();

    // AP4_SampleTable methods
    virtual AP4_Result GetSample(AP4_Ordinal index, AP4_Sample& sample);
    virtual AP4_Cardinal GetSampleCount();
    virtual AP4_SampleDescription* GetSampleDescription(AP4_Ordinal index);
    virtual AP4_Cardinal GetSampleDescriptionCount();
    virtual AP4_Result GetChunkForSample(AP4_Ordinal   sample,
                                         AP4_Ordinal&  chunk,
                                         AP4_Ordinal&  skip,
                                         AP4_Ordinal&  sample_description);
    virtual AP4_Result GetChunkOffset(AP4_Ordinal chunk, AP4_Offset& offset);
    virtual AP4_Result SetChunkOffset(AP4_Ordinal chunk, AP4_Offset offset);
    virtual AP4_Result SetSampleSize(AP4_Ordinal sample, AP4_Size size);
    virtual AP4_Result GetSampleIndexForTimeStamp(AP4_TimeStamp ts,
                                                  AP4_Ordinal& index);

private:
    // members
    AP4_ByteStream& m_SampleStream;
    AP4_StscAtom*   m_StscAtom;
    AP4_StcoAtom*   m_StcoAtom;
    AP4_Co64Atom*   m_Co64Atom;
    AP4_StszAtom*   m_StszAtom;
    AP4_SttsAtom*   m_SttsAtom;
    AP4_CttsAtom*   m_CttsAtom;
    AP4_StsdAtom*   m_StsdAtom;
    AP4_StssAtom*   m_StssAtom;
};

#endif // _AP4_ATOM_SAMPLE_TABLE_H_
