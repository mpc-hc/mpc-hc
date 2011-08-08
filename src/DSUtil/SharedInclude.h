#pragma once

#pragma warning(disable:4018)
#pragma warning(disable:4244)
#pragma warning(disable:4355)
#pragma warning(disable:4267)
#pragma warning(disable:4995)

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC // include Microsoft memory leak detection procedures

#if 0
#include <crtdbg.h>
#define DNew new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DNew new(__FILE__, __LINE__)
#endif

#else

#define DNew new

#endif
