/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Trace
//
// Provide a direct to file trace
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenTraceH
#define ZenTraceH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <fstream>
#include "ZenLib/Ztring.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************

static std::FILE* Trace_F;
static ZenLib::Ztring Trace;
static ZenLib::Ztring Trace2;

#ifdef TRACE
    #undef TRACE
#endif //TRACE

#if 1
#define TRACE(_TOAPPEND)
#else
#define TRACE(_TOAPPEND) \
Trace.clear(); \
Trace2.clear(); \
_TOAPPEND; \
if (!Trace2.empty()) \
{ \
    Trace+=__T(" - "); \
    Trace+=Trace2; \
} \
Trace+=__T("\r\n"); \
Trace_F=std::fopen("Trace.txt", "a+t"); \
if(Trace_F) \
{ \
  std::fwrite(Trace.To_Local().c_str(), Trace.size(), 1, Trace_F); \
  std::fclose(Trace_F); \
}
#endif

//***************************************************************************


} //NameSpace

#endif // ZenTraceH
