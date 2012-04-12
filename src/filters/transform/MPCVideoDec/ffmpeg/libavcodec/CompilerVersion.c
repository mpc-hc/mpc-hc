/*
 * (C) 2008-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>

static char g_Gcc_Compiler[31];

#if defined(DEBUG)
	#define COMPILER " Debug"
#else
	#define COMPILER ""
#endif

#if defined(__AVX2__)
	#define COMPILER_SSE " (AVX2)"
#elif defined(__AVX__)
	#define COMPILER_SSE " (AVX)"
#elif defined(__SSE4_2__)
	#define COMPILER_SSE " (SSE4.2)"
#elif defined(__SSE4_1__)
	#define COMPILER_SSE " (SSE4.1)"
#elif defined(__SSE4__)
	#define COMPILER_SSE " (SSE4)"
#elif defined(__SSSE3__)
	#define COMPILER_SSE " (SSSE3)"
#elif defined(__SSE3__)
	#define COMPILER_SSE " (SSE3)"
#elif !defined(ARCH_X86_64)
	#if defined(__SSE2__)
		#define COMPILER_SSE " (SSE2)"
	#elif defined(__SSE__)
		#define COMPILER_SSE " (SSE)"
	#elif defined(__MMX__)
		#define COMPILER_SSE " (MMX)"
	#endif
#else
	#define COMPILER_SSE ""
#endif

char* GetFFmpegCompiler()
{
	sprintf (g_Gcc_Compiler, "MinGW GCC %d.%d.%d%s%s", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, COMPILER, COMPILER_SSE);
	return g_Gcc_Compiler;
}
