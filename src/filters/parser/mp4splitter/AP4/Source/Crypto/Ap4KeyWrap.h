/*****************************************************************
|
|    AP4 - Key Wrap Algorithms
|
|    Copyright 2002-2009 Axiomatic Systems, LLC
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

#ifndef _AP4_KEY_WRAP_H_
#define _AP4_KEY_WRAP_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Results.h"
#include "Ap4Types.h"
#include "Ap4DataBuffer.h"

/*----------------------------------------------------------------------
|   functions
+---------------------------------------------------------------------*/
AP4_Result AP4_AesKeyWrap(const AP4_UI08* kek, 
                          const AP4_UI08* cleartext_key, 
                          AP4_Size        cleartext_key_size,
                          AP4_DataBuffer& wrapped_key);
                          
AP4_Result AP4_AesKeyUnwrap(const AP4_UI08* kek,
                            const AP4_UI08* wrapped_key,
                            AP4_Size        wrapped_key_size,
                            AP4_DataBuffer& cleartext_key);

#endif // _AP4_KEY_WRAP_H_
