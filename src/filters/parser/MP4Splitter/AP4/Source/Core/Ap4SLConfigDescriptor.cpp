/*****************************************************************
|
|    AP4 - SLConfig Descriptor 
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

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4SLConfigDescriptor.h"
#include "Ap4ByteStream.h"

/*----------------------------------------------------------------------
|   AP4_SLConfigDescriptor::AP4_SLConfigDescriptor
+---------------------------------------------------------------------*/
AP4_SLConfigDescriptor::AP4_SLConfigDescriptor(AP4_Size header_size) :
    AP4_Descriptor(AP4_DESCRIPTOR_TAG_SL_CONFIG, header_size, 1),
    m_Predefined(2)
{
}

/*----------------------------------------------------------------------
|   AP4_SLConfigDescriptor::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_SLConfigDescriptor::WriteFields(AP4_ByteStream& stream)
{
    // write the payload
    stream.WriteUI08(m_Predefined);

    return AP4_SUCCESS;
}
