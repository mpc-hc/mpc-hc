/*****************************************************************
|
|    AP4 - HMAC Algorithms
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

#ifndef _AP4_HMAC_H_
#define _AP4_HMAC_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Results.h"
#include "Ap4Types.h"
#include "Ap4DataBuffer.h"

/*----------------------------------------------------------------------
|   AP4_Hmac
+---------------------------------------------------------------------*/
class AP4_Hmac
{
public:
    // types
    typedef enum {
        SHA256
    } Algorithm;
    
    // class methods
    static AP4_Result Create(Algorithm       algorithm, 
                             const AP4_UI08* key,
                             AP4_Size        key_size,
                             AP4_Hmac*&      hmac);
    
    // methods
    virtual ~AP4_Hmac() {}
    virtual AP4_Result Update(const AP4_UI08* data, AP4_Size data_size) = 0;
    virtual AP4_Result Final(AP4_DataBuffer& mac) = 0;
};

#endif // _AP4_HMAC_H_
