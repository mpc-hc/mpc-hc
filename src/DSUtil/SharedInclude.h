#pragma once

// TODO: check 4731 and 4799
// Compile Bento4 as a separate project
//#pragma warning(disable:4244 4267)
#pragma warning(disable:4995)
#ifdef _DEBUG
// Remove this if you want to see all the "unsafe" functions used
// For Release builds _CRT_SECURE_NO_WARNINGS is defined
#pragma warning(disable:4996)
#endif

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
