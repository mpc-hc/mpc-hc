//	VDXFrame - Helper library for VirtualDub plugins
//	Copyright (C) 2008 Avery Lee
//
//	The plugin headers in the VirtualDub plugin SDK are licensed differently
//	differently than VirtualDub and the Plugin SDK themselves.  This
//	particular file is thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#ifndef f_VD2_VDXFRAME_VIDEOFILTERENTRY_H
#define f_VD2_VDXFRAME_VIDEOFILTERENTRY_H

#include <vd2/plugin/vdvideofilt.h>

#ifdef _MSC_VER
	#pragma once
#endif

struct VDXFilterModule;
struct VDXFilterFunctions;
struct VDXFilterDefinition;

///////////////////////////////////////////////////////////////////////////////
///	Video filter module entry point declaration macros
///
/// To declare the module init and deinit functions:
///
///		VDX_DECLARE_VFMODULE()
///
///	By default this declares the module as requiring copy contructor support
///	(V9 / VirtualDub 1.4.12). If you need to declare a different minimum
///	version, use:
///
///		VDX_DECLARE_VFMODULE_APIVER(version)
///
#define VDX_DECLARE_VFMODULE() VDX_DECLARE_VFMODULE_APIVER(VIRTUALDUB_FILTERDEF_COMPATIBLE_COPYCTOR)
#define VDX_DECLARE_VFMODULE_APIVER(apiver)	\
	extern "C" __declspec(dllexport) int __cdecl VirtualdubFilterModuleInit2(struct VDXFilterModule *fm, const VDXFilterFunctions *ff, int& vdfd_ver, int& vdfd_compat) {	\
		return VDXVideoFilterModuleInit2(fm, ff, vdfd_ver, vdfd_compat, (apiver));	\
	}	\
	\
	extern "C" __declspec(dllexport) void __cdecl VirtualdubFilterModuleDeinit(struct VDXFilterModule *fm, const VDXFilterFunctions *ff) {	\
		VDXVideoFilterModuleDeinit(fm, ff);		\
	}

///////////////////////////////////////////////////////////////////////////////
/// Video filter declaration macros
///
/// To declare video filters, use the following pattern:
///
///	VDX_DECLARE_VIDEOFILTERS_BEGIN()
///		VDX_DECLARE_VIDEOFILTER(definitionSymbolName)
///	VDX_DECLARE_VIDEOFILTERS_END()
///
/// Each entry points to a variable of type VDXFilterDefinition. Note that these must
/// be declared as _non-const_ for compatibility with older versions of VirtualDub.
/// Video filters declared this way are automatically registered by the module init
/// routine.
///
#define VDX_DECLARE_VIDEOFILTERS_BEGIN()		VDXFilterDefinition *VDXGetVideoFilterDefinition(int index) {
#define VDX_DECLARE_VIDEOFILTER(defVarName)			if (!index--) { extern VDXFilterDefinition defVarName; return &defVarName; }
#define VDX_DECLARE_VIDEOFILTERS_END()				return NULL;	\
												}
int VDXVideoFilterModuleInit2(struct VDXFilterModule *fm, const VDXFilterFunctions *ff, int& vdfd_ver, int& vdfd_compat, int ver_compat_target);
void VDXVideoFilterModuleDeinit(struct VDXFilterModule *fm, const VDXFilterFunctions *ff);

int VDXGetVideoFilterAPIVersion();

#endif
