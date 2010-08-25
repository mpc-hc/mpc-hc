// ZenLib::Trace - To trace (in files)
// Copyright (C) 2006-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
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
// Trace
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
    Trace+=_T(" - "); \
    Trace+=Trace2; \
} \
Trace+=_T("\r\n"); \
Trace_F=std::fopen("Trace.txt", "a+t"); \
std::fwrite(Trace.To_Local().c_str(), Trace.size(), 1, Trace_F); \
std::fclose(Trace_F);
#endif

//***************************************************************************


} //NameSpace

#endif // ZenTraceH


