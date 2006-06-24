/*****************************************************************
|
|    AP4 - Common Interfaces
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

#ifndef _AP4_INTERFACES_H_
#define _AP4_INTERFACES_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"

/*----------------------------------------------------------------------
|       macros
+---------------------------------------------------------------------*/
#define AP4_RELEASE(o) do { if (o) (o)->Release(); (o) = NULL; } while (0)
#define AP4_ADD_REFERENCE(o) do { if (o) (o)->AddReference(); } while (0)

/*----------------------------------------------------------------------
|       AP4_Exception
+---------------------------------------------------------------------*/
class AP4_Exception
{
public:
    // methods
    AP4_Exception(AP4_Result error) : m_Error(error) {}

    // members
    AP4_Result m_Error;
};

/*----------------------------------------------------------------------
|       AP4_Referenceable
+---------------------------------------------------------------------*/
class AP4_Referenceable
{
 public:
    // methods
    virtual ~AP4_Referenceable() {}
    virtual void AddReference() = 0;
    virtual void Release() = 0;
};

#endif // _AP4_INTERFACES_H_
