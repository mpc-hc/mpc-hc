

#ifdef _MSC_VER
	#if (_MSC_VER == 1500)
		char	FfmpegCompiler[] = "VS 2008";
	#elif (_MSC_VER == 1400)
		char	FfmpegCompiler[] = "VS 2005";
	#elif (_MSC_VER == 1310)
		char	FfmpegCompiler[] = "VS 2003";
	#elif (_MSC_VER == 1300)
		char	FfmpegCompiler[] = "VS 2002";
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

