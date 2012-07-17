/*****************************************************************
|
|    AP4 - hdlr Atoms 
|
|    Copyright 2002 Gilles Boccon-Gibod
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
#include "Ap4HdlrAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_HdlrAtom::AP4_HdlrAtom
+---------------------------------------------------------------------*/
AP4_HdlrAtom::AP4_HdlrAtom(AP4_Atom::Type hdlr_type, const char* hdlr_name) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, true),
    m_HandlerType(hdlr_type),
    m_HandlerName(hdlr_name)
{
    m_Size += 20+m_HandlerName.length()+1;
}

/*----------------------------------------------------------------------
|       AP4_HdlrAtom::AP4_HdlrAtom
+---------------------------------------------------------------------*/
AP4_HdlrAtom::AP4_HdlrAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, size, true, stream)
{
    unsigned char reserved[12];
    stream.Read(reserved, 4, NULL);
    stream.ReadUI32(m_HandlerType);
    stream.Read(reserved, 12, NULL);
    
    // read the name unless it is empty
    int name_size = size-(AP4_FULL_ATOM_HEADER_SIZE+20);
    if (name_size > 0) {
        char* name = new char[name_size+1];
        stream.Read(name, name_size);
        name[name_size] = '\0'; // force a null termination
        m_HandlerName = name;
        delete[] name;
    }
}

/*----------------------------------------------------------------------
|       AP4_HdlrAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_HdlrAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // write the data
    unsigned char reserved[12];
    memset(reserved, 0, sizeof(reserved));    
    result = stream.Write(reserved, 4);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI32(m_HandlerType);
    if (AP4_FAILED(result)) return result;
    result = stream.Write(reserved, 12);
    if (AP4_FAILED(result)) return result;
    result = stream.Write(m_HandlerName.c_str(), 
                          m_HandlerName.length()+1);
    if (AP4_FAILED(result)) return result;

    // pad with zeros if necessary
    AP4_Size padding = m_Size-(AP4_FULL_ATOM_HEADER_SIZE+20+m_HandlerName.length()+1);
    while (padding--) stream.WriteUI08(0);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_HdlrAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_HdlrAtom::InspectFields(AP4_AtomInspector& inspector)
{
    char type[5];
    AP4_FormatFourChars(type, m_HandlerType);
    inspector.AddField("handler_type", type);
    inspector.AddField("handler_name", m_HandlerName.c_str());

    return AP4_SUCCESS;
}
