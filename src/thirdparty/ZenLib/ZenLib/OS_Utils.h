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
#ifdef WINDOWS
#ifndef ZENLIB_NO_WIN9X_SUPPORT
    #undef __TEXT
    #include "windows.h"
#endif //ZENLIB_NO_WIN9X_SUPPORT
#endif //WINDOWS
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
// OS Information
//***************************************************************************

//---------------------------------------------------------------------------
bool IsWin9X ();
#ifdef WINDOWS
#ifndef ZENLIB_NO_WIN9X_SUPPORT
inline bool IsWin9X_Fast ()
{
    return GetVersion()>=0x80000000;
}
#endif //ZENLIB_NO_WIN9X_SUPPORT
#endif //WINDOWS

//***************************************************************************
// Execute
//***************************************************************************

void Shell_Execute(const Ztring &ToExecute);

//***************************************************************************
// Directorues
//***************************************************************************

Ztring OpenFolder_Show(void* Handle, const Ztring &Title, const Ztring &Caption);

} //namespace ZenLib
#endif
