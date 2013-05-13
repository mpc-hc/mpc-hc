/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Helpers for compilers (precompilation)
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_PreCompH
#define MediaInfo_PreCompH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(_MSC_VER) || defined(__BORLANDC__)
 #include "MediaInfo/Setup.h"
 #include "MediaInfo/File__Analyze.h"
#endif //_MSC_VER
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

#endif
