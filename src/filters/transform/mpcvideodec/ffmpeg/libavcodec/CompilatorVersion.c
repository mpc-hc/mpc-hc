/*
 * This file is part of Media Player Classic HomeCinema.
 *
 * MPC-HC is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifdef _MSC_VER
	#if (_MSC_VER == 1500)
		#if (_MSC_FULL_VER >= 150030729)
			char	FfmpegCompiler[] = "MSVS 2008 SP1";	
		#else	
			char	FfmpegCompiler[] = "MSVS 2008";
		#endif
	#elif (_MSC_VER == 1400)
		#if (_MSC_FULL_VER >= 140050727)
			char	FfmpegCompiler[] = "MSVS 2005 SP1";
		#else
			char	FfmpegCompiler[] = "MSVS 2005";
		#endif
	#elif (_MSC_VER == 1310)
		char	FfmpegCompiler[] = "MSVS 2003";
	#elif (_MSC_VER == 1300)
		char	FfmpegCompiler[] = "MSVS 2002";
	#endif

	char* GetFfmpegCompiler()
	{
		return FfmpegCompiler;
	}
#else
	#include <stdio.h>
	static char	g_Gcc_Compiler[20];
	char* GetFfmpegCompiler()
	{
		sprintf (g_Gcc_Compiler, "GCC %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
		return g_Gcc_Compiler;
	}
#endif
