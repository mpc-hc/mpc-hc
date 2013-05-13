/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

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
