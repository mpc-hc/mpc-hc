/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "ZenLib/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf_Internal.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
// Platform differences
//***************************************************************************

//End of line
#ifdef WINDOWS
    const Char* EOL=__T("\r\n");
    const Char  PathSeparator=__T('\\');
#endif
#ifdef UNIX
    const Char* EOL=__T("\n");
    const Char  PathSeparator=__T('/');
#endif
#if defined (MACOS) || defined (MACOSX)
    const Char* EOL=__T("\n");
    const Char  PathSeparator=__T('/');
#endif

//***************************************************************************
//
//***************************************************************************

} //namespace
