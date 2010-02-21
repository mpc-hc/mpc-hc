//	VirtualDub - Video processing and capture application
//	Plugin headers
//	Copyright (C) 1998-2007 Avery Lee, All Rights Reserved.
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

#ifndef f_VD2_PLUGIN_VDPLUGIN_H
#define f_VD2_PLUGIN_VDPLUGIN_H

#ifdef _MSC_VER
	#pragma once
	#pragma pack(push, 8)
#endif

#include <stddef.h>

// Copied from <vd2/system/vdtypes.h>.  Must be in sync.
#ifndef VD_STANDARD_TYPES_DECLARED
	#if defined(_MSC_VER)
		typedef signed __int64		sint64;
		typedef unsigned __int64	uint64;
	#elif defined(__GNUC__)
		typedef signed long long	sint64;
		typedef unsigned long long	uint64;
	#endif
	typedef signed int			sint32;
	typedef unsigned int		uint32;
	typedef signed short		sint16;
	typedef unsigned short		uint16;
	typedef signed char			sint8;
	typedef unsigned char		uint8;

	typedef sint64				int64;
	typedef sint32				int32;
	typedef sint16				int16;
	typedef sint8				int8;

	typedef ptrdiff_t			sintptr;
	typedef size_t				uintptr;
#endif

#ifndef VDXAPIENTRY
	#define VDXAPIENTRY __stdcall
#endif

#ifndef VDXAPIENTRYV
	#define VDXAPIENTRYV __cdecl
#endif

enum VDXCPUFeatureFlags {
	kVDXCPUF_CPUID		= 0x00000001,
	kVDXCPUF_MMX		= 0x00000004,
	kVDXCPUF_ISSE		= 0x00000008,
	kVDXCPUF_SSE		= 0x00000010,
	kVDXCPUF_SSE2		= 0x00000020,
	kVDXCPUF_3DNOW		= 0x00000040,
	kVDXCPUF_3DNOW_EXT	= 0x00000080,
	kVDXCPUF_SSE3		= 0x00000100,
	kVDXCPUF_SSSE3		= 0x00000200
};

enum {
	kVDXPlugin_APIVersion		= 10
};


enum {
	kVDXPluginType_Video,		// Updated video filter API is not yet complete.
	kVDXPluginType_Audio,
	kVDXPluginType_Input
};

struct VDXPluginInfo {
	uint32			mSize;				// size of this structure in bytes
	const wchar_t	*mpName;
	const wchar_t	*mpAuthor;
	const wchar_t	*mpDescription;
	uint32			mVersion;			// (major<<24) + (minor<<16) + build.  1.4.1000 would be 0x010403E8.
	uint32			mType;
	uint32			mFlags;
	uint32			mAPIVersionRequired;
	uint32			mAPIVersionUsed;
	uint32			mTypeAPIVersionRequired;
	uint32			mTypeAPIVersionUsed;
	const void *	mpTypeSpecificInfo;
};

typedef const VDXPluginInfo *const *(VDXAPIENTRY *tpVDXGetPluginInfo)();

typedef VDXPluginInfo VDPluginInfo;
typedef tpVDXGetPluginInfo tpVDPluginInfo;

class IVDXPluginCallbacks {
public:
	virtual void * VDXAPIENTRY GetExtendedAPI(const char *pExtendedAPIName) = 0;
	virtual void VDXAPIENTRYV SetError(const char *format, ...) = 0;
	virtual void VDXAPIENTRY SetErrorOutOfMemory() = 0;
	virtual uint32 VDXAPIENTRY GetCPUFeatureFlags() = 0;
};

typedef IVDXPluginCallbacks IVDPluginCallbacks;

struct VDXPluginConfigEntry {
	enum Type {
		kTypeInvalid	= 0,
		kTypeU32		= 1,
		kTypeS32,
		kTypeU64,
		kTypeS64,
		kTypeDouble,
		kTypeAStr,
		kTypeWStr,
		kTypeBlock
	};

	const VDXPluginConfigEntry *next;

	unsigned	idx;
	uint32		type;
	const wchar_t *name;
	const wchar_t *label;
	const wchar_t *desc;	
};

struct VDXPixmap {
	void			*data;
	const uint32	*palette;
	sint32			w;
	sint32			h;
	ptrdiff_t		pitch;
	sint32			format;

	// Auxiliary planes are always byte-per-pixel.
	void			*data2;		// Cb (U) for YCbCr
	ptrdiff_t		pitch2;
	void			*data3;		// Cr (V) for YCbCr
	ptrdiff_t		pitch3;
};

struct VDXPixmapLayout {
	ptrdiff_t		data;
	const uint32	*palette;
	sint32			w;
	sint32			h;
	ptrdiff_t		pitch;
	sint32			format;

	// Auxiliary planes are always byte-per-pixel.
	ptrdiff_t		data2;		// Cb (U) for YCbCr
	ptrdiff_t		pitch2;
	ptrdiff_t		data3;		// Cr (V) for YCbCr
	ptrdiff_t		pitch3;
};

namespace nsVDXPixmap {
	enum VDXPixmapFormat {
		kPixFormat_Null				= 0,
		kPixFormat_XRGB1555			= 5,
		kPixFormat_RGB565			= 6,
		kPixFormat_RGB888			= 7,
		kPixFormat_XRGB8888			= 8,
		kPixFormat_Y8				= 9,
		kPixFormat_YUV422_UYVY		= 10,
		kPixFormat_YUV422_YUYV		= 11,
		kPixFormat_YUV444_Planar	= 13,
		kPixFormat_YUV422_Planar	= 14,
		kPixFormat_YUV420_Planar	= 15,
		kPixFormat_YUV411_Planar	= 16,
		kPixFormat_YUV410_Planar	= 17
	};
};

#define VDXMAKEFOURCC(a, b, c, d) ((uint32)(uint8)(d) + ((uint32)(uint8)(c) << 8) + ((uint32)(uint8)(b) << 16) + ((uint32)(uint8)(a) << 24))

class IVDXUnknown {
public:
	enum { kIID = VDXMAKEFOURCC('X', 'u', 'n', 'k') };
	virtual int VDXAPIENTRY AddRef() = 0;
	virtual int VDXAPIENTRY Release() = 0;
	virtual void *VDXAPIENTRY AsInterface(uint32 iid) = 0;
};

#ifdef _MSC_VER
	#pragma pack(pop)
#endif

#endif
