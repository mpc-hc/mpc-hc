#include "cpudetect.h"
#include "../libswscale/config.h"
#include <string.h>
#include "fastmemcpy.h"
#include "../compiler.h"

CpuCaps gCpuCaps;

void init_libvo()
{
 gCpuCaps.hasMMX=1;
 gCpuCaps.hasMMX2=1;
 gCpuCaps.has3DNow=1;
 gCpuCaps.has3DNowExt=1;
 gCpuCaps.hasSSE=1;
 gCpuCaps.hasSSE2=1;
 gCpuCaps.hasSSSE3=1;
 init_fast_memcpy();
 // Avoid using multithread if the CPU is Pentium4-HT
 // because it is not faster at all and uses more CPU.
 // (Swscaler depends much on MMX and P4HT have only one MMX unit.)
}
