// PreComp - PreComp file for ZenLib
// Copyright (C) 2006-2012 MediaArea.net SARL, Info@MediaArea.net
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Helpers for compilers (precompilation)
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_PreCompH
#define ZenLib_PreCompH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(_MSC_VER) || defined(__BORLANDC__)
    #include <cstring>
    #include <cstdio>
    #include <cstdlib>
    #include <ctime>
    #include <algorithm>
    #include <map>
    #include <sstream>
    #include <iomanip>
    #include <cmath>
    #include "ZenLib/Conf.h"
    #include "ZenLib/Conf_Internal.h"
#endif //defined(_MSC_VER) || defined(__BORLANDC__)
//---------------------------------------------------------------------------

#endif
