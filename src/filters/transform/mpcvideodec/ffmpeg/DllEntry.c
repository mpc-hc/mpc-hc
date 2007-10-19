/*
 * DllEntry point functions for ffmpeg encoder and decoder
 * Copyright (c) 2004 Andrew Ivanov <jho@nata-info.ru>.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/**
 * @file dllentry.c
 */

#include <windows.h>
#include "libavcodec/avcodec.h"
#include "../compiler.h"

CRITICAL_SECTION g_csStaticDataLock;

// --- standard WIN32 entrypoints --------------------------------------
BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
 static int count=0;
 char pomS[40];
 switch (dwReason)
  {
   case DLL_PROCESS_ATTACH:
    count++;
    //snprintf(pomS,40,"libavcodec: %i %i\n",count,hInstance);OutputDebugString(pomS);
    DisableThreadLibraryCalls(hInstance);
    InitializeCriticalSection( &g_csStaticDataLock );
    break;
   case DLL_PROCESS_DETACH:
    count--;
    //snprintf(pomS,40,"libavcodec: %i %i\n",count,hInstance);OutputDebugString(pomS);
    if (count<=0)
     av_free_static();
    DeleteCriticalSection( &g_csStaticDataLock );
    break;
  }
 return TRUE;
}

static char av_datetime[]=__DATE__" "__TIME__;
void getVersion(char **version,char **build,char **datetime,const char* *license)
{
 if (version) *version=AV_STRINGIFY(LIBAVCODEC_VERSION)", "COMPILER COMPILER_X64;
 if (build) *build=AV_STRINGIFY(LIBAVCODEC_BUILD);
 if (datetime) *datetime=av_datetime;
 if (license) *license="";
}
