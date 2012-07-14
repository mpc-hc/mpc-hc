/*****************************************************************
|
|    AP4 - iSFM Atoms 
|
|    Copyright 2002-2005 Gilles Boccon-Gibod & Julien Boeuf
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

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4Utils.h"
#include "Ap4IsfmAtom.h"

/*----------------------------------------------------------------------
|       AP4_IsfmAtom::AP4_IsfmAtom
+---------------------------------------------------------------------*/
AP4_IsfmAtom::AP4_IsfmAtom(bool     selective_encryption,
                           AP4_UI08 key_length_indicator,
                           AP4_UI08 iv_length) :
    AP4_Atom(AP4_ATOM_TYPE_ISFM, AP4_FULL_ATOM_HEADER_SIZE+3, true),
    m_SelectiveEncryption(selective_encryption),
    m_KeyIndicatorLength(key_length_indicator),
    m_IvLength(iv_length)
{
}

/*----------------------------------------------------------------------
|       AP4_IsfmAtom::AP4_IsfmAtom
+---------------------------------------------------------------------*/
AP4_IsfmAtom::AP4_IsfmAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_ISFM, size, true, stream)
{
    AP4_UI08 s;
    stream.ReadUI08(s);
    m_SelectiveEncryption = ((s&1) != 0);
    stream.ReadUI08(m_KeyIndicatorLength);
    stream.ReadUI08(m_IvLength);
}

/*----------------------------------------------------------------------
|       AP4_IsfmAtom::Clone
+---------------------------------------------------------------------*/
AP4_Atom* 
AP4_IsfmAtom::Clone()
{
    return new AP4_IsfmAtom(m_SelectiveEncryption, 
                            m_KeyIndicatorLength, 
                            m_IvLength);
}

/*----------------------------------------------------------------------
|       AP4_IsfmAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IsfmAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // selective encryption
    result = stream.WriteUI08(m_SelectiveEncryption ? 1 : 0);
    if (AP4_FAILED(result)) return result;

    // key indicator length
    result = stream.WriteUI08(m_KeyIndicatorLength);
    if (AP4_FAILED(result)) return result;

    // IV length
    result = stream.WriteUI08(m_IvLength);
    if (AP4_FAILED(result)) return result;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_IsfmAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IsfmAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("selective_encryption", m_SelectiveEncryption);
    inspector.AddField("key_indicator_length", m_KeyIndicatorLength);
    inspector.AddField("IV_length", m_IvLength);

    return AP4_SUCCESS;
}
