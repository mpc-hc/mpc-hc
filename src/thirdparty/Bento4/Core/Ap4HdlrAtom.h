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

#ifndef _AP4_HDLR_ATOM_H_
#define _AP4_HDLR_ATOM_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4ByteStream.h"
#include "Ap4List.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_HANDLER_TYPE_SOUN = AP4_ATOM_TYPE('s','o','u','n');
const AP4_UI32 AP4_HANDLER_TYPE_VIDE = AP4_ATOM_TYPE('v','i','d','e');
const AP4_UI32 AP4_HANDLER_TYPE_TEXT = AP4_ATOM_TYPE('t','e','x','t');
const AP4_UI32 AP4_HANDLER_TYPE_TX3G = AP4_ATOM_TYPE('t','x','3','g');
const AP4_UI32 AP4_HANDLER_TYPE_SUBP = AP4_ATOM_TYPE('s','u','b','p');
const AP4_UI32 AP4_HANDLER_TYPE_HINT = AP4_ATOM_TYPE('h','i','n','t');
const AP4_UI32 AP4_HANDLER_TYPE_SBTL = AP4_ATOM_TYPE('s','b','t','l');

/*----------------------------------------------------------------------
|       AP4_HdlrAtom
+---------------------------------------------------------------------*/
class AP4_HdlrAtom : public AP4_Atom
{
public:
    // methods
    AP4_HdlrAtom(AP4_UI32 hdlr_type, const char* hdlr_name);
    AP4_HdlrAtom(AP4_Size size, AP4_ByteStream& stream);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    AP4_UI32 GetHandlerType() { return m_HandlerType; }
    AP4_String GetHandlerName() { return m_HandlerName; }

private:
    // members
    AP4_UI32   m_HandlerType;
    AP4_String m_HandlerName;
};

#endif // _AP4_HDLR_ATOM_H_
