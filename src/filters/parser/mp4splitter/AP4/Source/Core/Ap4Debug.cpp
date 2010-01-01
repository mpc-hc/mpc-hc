/*****************************************************************
|
|    AP4 - Debug Support
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
#include <stdarg.h>
#include <stdio.h>

#include "Ap4Config.h"
#include "Ap4Types.h"
#include "Ap4Utils.h"
#include "Ap4Debug.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const int AP4_DEBUG_MAX_BUFFER = 1024;

/*----------------------------------------------------------------------
|   AP4_Print
+---------------------------------------------------------------------*/
static void
AP4_Print(const char* message)
{
    printf("%s", message);
}

/*----------------------------------------------------------------------
|   AP4_Debug
+---------------------------------------------------------------------*/
void
AP4_Debug(const char* format, ...)
{
    va_list args;
    
    va_start(args, format);

    char buffer[AP4_DEBUG_MAX_BUFFER];
    AP4_FormatStringVN(buffer, sizeof(buffer), format, args);
    AP4_Print(buffer);

    va_end(args);
}
