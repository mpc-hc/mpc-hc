/*****************************************************************
|
|    AP4 - SLConfig Descriptor 
|
|    Copyright 2002 Gilles Boccon-Gibod & Julien Boeuf
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

#ifndef _AP4_SLCONFIG_DESCRIPTOR_H_
#define _AP4_SLCONFIG_DESCRIPTOR_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4ByteStream.h"
#include "Ap4List.h"
#include "Ap4DataBuffer.h"
#include "Ap4Descriptor.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
const AP4_Descriptor::Tag AP4_DESCRIPTOR_TAG_SL_CONFIG = 0x06;

/*----------------------------------------------------------------------
|       AP4_SLConfigDescriptor
+---------------------------------------------------------------------*/
class AP4_SLConfigDescriptor : public AP4_Descriptor 
{
public:
    // methods
    AP4_SLConfigDescriptor(AP4_Size header_size);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

private:
    // members
    AP4_UI08 m_Predefined; // = 2 (fixed for MP4 files)
};


#endif // _AP4_SLCONFIG_DESCRIPTOR_H_
