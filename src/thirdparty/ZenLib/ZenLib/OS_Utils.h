/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef ZenOS_UtilsH
#define ZenOS_UtilsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Ztring.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
// OS Information
//***************************************************************************

//---------------------------------------------------------------------------
bool IsWin9X ();
// Execute
//***************************************************************************

void Shell_Execute(const Ztring &ToExecute);

//***************************************************************************
// Directorues
//***************************************************************************

Ztring OpenFolder_Show(void* Handle, const Ztring &Title, const Ztring &Caption);

} //namespace ZenLib
#endif
