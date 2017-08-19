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

#ifndef f_VD2_PLUGIN_VDVIDEOFILT_H
#define f_VD2_PLUGIN_VDVIDEOFILT_H

#ifdef _MSC_VER
	#pragma once
	#pragma pack(push, 8)
#endif

#include <stddef.h>

#include "vdplugin.h"

typedef struct VDXHINSTANCEStruct *VDXHINSTANCE;
typedef struct VDXHDCStruct *VDXHDC;
typedef struct VDXHWNDStruct *VDXHWND;

//////////////////

struct VDXScriptObject;
struct VDXFilterVTbls;

//////////////////

enum {
	/// Request distinct source and destination buffers. Otherwise, the source and destination buffers
	/// alias (in-place mode).
	FILTERPARAM_SWAP_BUFFERS		= 0x00000001L,

	/// Request an extra buffer for the previous source frame.
	FILTERPARAM_NEEDS_LAST			= 0x00000002L,

	/// Filter supports image formats other than RGB32. Filters that support format negotiation must
	/// set this flag for all calls to paramProc.
	FILTERPARAM_SUPPORTS_ALTFORMATS	= 0x00000004L,

	/// Filter requests 16 byte alignment for source and destination buffers. This guarantees that:
	///
	///		- data and pitch fields are multiples of 16 bytes (aligned)
	///		- an integral number of 16 byte vectors may be read, even if the last vector includes
	///		  some bytes beyond the end of the scanline (their values are undefined)
	///		- an integral number of 16 byte vectors may be written, even if the last vector includes
	///		  some bytes beyong the end of the scanline (their values are ignored)
	///
	FILTERPARAM_ALIGN_SCANLINES		= 0x00000008L,

	/// Filter's output is purely a function of configuration parameters and source image data, and not
	/// source or output frame numbers. In other words, two output frames produced by a filter instance
	/// can be assumed to be identical images if:
	///
	///		- the same number of source frames are prefetched
	///		- the same type of prefetches are performed (direct vs. non-direct)
	///		- the frame numbers for the two prefetch lists, taken in order, correspond to identical
	///		  source frames
	///		- the prefetch cookies match
	///
	/// Enabling this flag improves the ability of the host to identify identical frames and drop them
	/// in preview or in the output file.
	///
	FILTERPARAM_PURE_TRANSFORM		= 0x00000010L,

	/// Filter cannot support the requested source format. Note that this sets all bits, so the meaning
	/// of other bits is ignored. The one exception is that FILTERPARAM_SUPPORTS_ALTFORMATS is assumed
	/// to be implicitly set.
	FILTERPARAM_NOT_SUPPORTED		= (long)0xFFFFFFFF
};

/// The filter has a delay from source to output. For instance, a lag of 3 indicates that the
/// filter internally buffers three frames, so when it is fed frames in sequence, frame 0 emerges
/// after frame 3 has been processed. The host attempts to correct timestamps in order to compensate.
///
/// VirtualDub 1.9.1 or later: Setting this flag can have a performance penalty, as it causes the host
/// to request additional frames to try to produce the correct requested output frames.
///
#define FILTERPARAM_HAS_LAG(frames) ((int)(frames) << 16)

///////////////////

class VDXFBitmap;
class VDXFilterActivation;
struct VDXFilterFunctions;
struct VDXFilterModule;
class IVDXVideoPrefetcher;

enum {
	kVDXVFEvent_None				= 0,
	kVDXVFEvent_InvalidateCaches	= 1
};

typedef int  (__cdecl *VDXFilterInitProc     )(VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef void (__cdecl *VDXFilterDeinitProc   )(VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef int  (__cdecl *VDXFilterRunProc      )(const VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef long (__cdecl *VDXFilterParamProc    )(VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef int  (__cdecl *VDXFilterConfigProc   )(VDXFilterActivation *fa, const VDXFilterFunctions *ff, VDXHWND hWnd);
typedef void (__cdecl *VDXFilterStringProc   )(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, char *buf);
typedef int  (__cdecl *VDXFilterStartProc    )(VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef int  (__cdecl *VDXFilterEndProc      )(VDXFilterActivation *fa, const VDXFilterFunctions *ff);
typedef bool (__cdecl *VDXFilterScriptStrProc)(VDXFilterActivation *fa, const VDXFilterFunctions *, char *, int);
typedef void (__cdecl *VDXFilterStringProc2  )(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, char *buf, int maxlen);
typedef int  (__cdecl *VDXFilterSerialize    )(VDXFilterActivation *fa, const VDXFilterFunctions *ff, char *buf, int maxbuf);
typedef void (__cdecl *VDXFilterDeserialize  )(VDXFilterActivation *fa, const VDXFilterFunctions *ff, const char *buf, int maxbuf);
typedef void (__cdecl *VDXFilterCopy         )(VDXFilterActivation *fa, const VDXFilterFunctions *ff, void *dst);
typedef sint64 (__cdecl *VDXFilterPrefetch   )(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, sint64 frame);
typedef void (__cdecl *VDXFilterCopy2Proc    )(VDXFilterActivation *fa, const VDXFilterFunctions *ff, void *dst, VDXFilterActivation *fa2, const VDXFilterFunctions *ff2);
typedef bool (__cdecl *VDXFilterPrefetch2Proc)(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, sint64 frame, IVDXVideoPrefetcher *prefetcher);
typedef bool (__cdecl *VDXFilterEventProc	 )(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, uint32 event, const void *eventData);

typedef int (__cdecl *VDXFilterModuleInitProc)(VDXFilterModule *fm, const VDXFilterFunctions *ff, int& vdfd_ver, int& vdfd_compat);
typedef void (__cdecl *VDXFilterModuleDeinitProc)(VDXFilterModule *fm, const VDXFilterFunctions *ff);

//////////

typedef void (__cdecl *VDXFilterPreviewButtonCallback)(bool fNewState, void *pData);
typedef void (__cdecl *VDXFilterPreviewSampleCallback)(VDXFBitmap *, long lFrame, long lCount, void *pData);

class IVDXFilterPreview {
public:
	virtual void SetButtonCallback(VDXFilterPreviewButtonCallback, void *)=0;
	virtual void SetSampleCallback(VDXFilterPreviewSampleCallback, void *)=0;

	virtual bool isPreviewEnabled()=0;
	virtual void Toggle(VDXHWND)=0;
	virtual void Display(VDXHWND, bool)=0;
	virtual void RedoFrame()=0;
	virtual void RedoSystem()=0;
	virtual void UndoSystem()=0;
	virtual void InitButton(VDXHWND)=0;
	virtual void Close()=0;
	virtual bool SampleCurrentFrame()=0;
	virtual long SampleFrames()=0;
};

class IVDXFilterPreview2 : public IVDXFilterPreview {
public:
	virtual bool IsPreviewDisplayed() = 0;
};

class IVDXVideoPrefetcher : public IVDXUnknown {
public:
	enum { kIID = VDXMAKEFOURCC('X', 'v', 'p', 'f') };

	/// Request a video frame fetch from an upstream source.
	virtual void VDXAPIENTRY PrefetchFrame(sint32 srcIndex, sint64 frame, uint64 cookie) = 0;

	/// Request a video frame fetch from an upstream source in direct mode.
	/// This specifies that the output frame is the same as the input frame.
	/// There cannot be more than one direct fetch and there must be no standard
	/// fetches at the same time. There can, however, be symbolic fetches.
	virtual void VDXAPIENTRY PrefetchFrameDirect(sint32 srcIndex, sint64 frame) = 0;

	/// Request a symbolic fetch from a source. This does not actually fetch
	/// any frames, but marks an association from source to output. This is
	/// useful for indicating the approximate center of where an output derives
	/// in a source, even if those frames aren't fetched (perhaps due to caching).
	/// There may be either zero or one symbolic fetch per source.
	///
	/// If no symbolic fetches are performed, the symbolic frame is assumed to
	/// be the rounded mean of the fetched source frames.
	virtual void VDXAPIENTRY PrefetchFrameSymbolic(sint32 srcIndex, sint64 frame) = 0;
};

//////////

enum {
	// This is the highest API version supported by this header file.
	VIRTUALDUB_FILTERDEF_VERSION		= 14,

	// This is the absolute lowest API version supported by this header file.
	// Note that V4 is rather old, corresponding to VirtualDub 1.2.
	// Chances are you will need to declare a higher version.
	VIRTUALDUB_FILTERDEF_COMPATIBLE		= 4,

	// API V9 is a slightly saner baseline, since it is the first API
	// version that has copy constructor support. You may still need to
	// declare a higher vdfd_compat version in your module init if you
	// need features beyond V9 (VirtualDub 1.4.12).
	VIRTUALDUB_FILTERDEF_COMPATIBLE_COPYCTOR = 9

};

// v3: added lCurrentSourceFrame to FrameStateInfo
// v4 (1.2): lots of additions (VirtualDub 1.2)
// v5 (1.3d): lots of bugfixes - stretchblt bilinear, and non-zero startproc
// v6 (1.4): added error handling functions
// v7 (1.4d): added frame lag, exception handling
// v8 (1.4.11): added string2 proc
// v9 (1.4.12): added (working) copy constructor
// v10 (1.5.10): added preview flag
// v11 (1.7.0): guaranteed src structure setup before configProc; added IVDFilterPreview2
// v12 (1.8.0): support for frame alteration
// v13 (1.8.2): added mOutputFrame field to VDXFilterStateInfo
// v14 (1.9.1): added copyProc2, prefetchProc2, input/output frame arrays

struct VDXFilterDefinition {
	void *_next;		// deprecated - set to NULL
	void *_prev;		// deprecated - set to NULL
	void *_module;		// deprecated - set to NULL

	const char *		name;
	const char *		desc;
	const char *		maker;
	void *				private_data;
	int					inst_data_size;

	VDXFilterInitProc		initProc;
	VDXFilterDeinitProc		deinitProc;
	VDXFilterRunProc		runProc;
	VDXFilterParamProc		paramProc;
	VDXFilterConfigProc		configProc;
	VDXFilterStringProc		stringProc;
	VDXFilterStartProc		startProc;
	VDXFilterEndProc		endProc;

	VDXScriptObject			*script_obj;

	VDXFilterScriptStrProc	fssProc;

	// NEW - 1.4.11
	VDXFilterStringProc2	stringProc2;
	VDXFilterSerialize		serializeProc;
	VDXFilterDeserialize	deserializeProc;
	VDXFilterCopy			copyProc;

	VDXFilterPrefetch		prefetchProc;		// (V12/V1.7.4+)

	// NEW - V14 / 1.9.1
	VDXFilterCopy2Proc		copyProc2;
	VDXFilterPrefetch2Proc	prefetchProc2;
	VDXFilterEventProc		eventProc;
};

//////////

// FilterStateInfo: contains dynamic info about file being processed

class VDXFilterStateInfo {
public:
	sint32	lCurrentFrame;				// current sequence frame (previously called output frame)
	sint32	lMicrosecsPerFrame;			// microseconds per sequence frame
	sint32	lCurrentSourceFrame;		// current source frame
	sint32	lMicrosecsPerSrcFrame;		// microseconds per source frame
	sint32	lSourceFrameMS;				// source frame timestamp
	sint32	lDestFrameMS;				// output frame timestamp

	enum {
		kStateNone		= 0x00000000,
		kStatePreview	= 0x00000001,	// (V1.5.10+) Job output is not being saved to disk.
		kStateRealTime	= 0x00000002,	// (V1.5.10+) Operation is running in real-time (capture, playback).
		kStateMax		= 0xFFFFFFFF
	};

	uint32	flags;						// (V10 / 1.5.10+ only)

	sint32	mOutputFrame;				// (V13/V1.8.2+) current output frame
};

// VDXFBitmap: VBitmap extended to hold filter-specific information

class VDXBitmap {
public:
	void *			_vtable;	// Reserved - do not use.
	uint32 *		data;		// Pointer to start of _bottom-most_ scanline of plane 0.
	uint32 *		palette;	// Pointer to palette (reserved - set to NULL).
	sint32			depth;		// Bit depth, in bits. Set to zero if mpPixmap/mpPixmapLayout are active.
	sint32			w;			// Width of bitmap, in pixels.
	sint32			h;			// Height of bitmap, in pixels.
	ptrdiff_t		pitch;		// Distance, in bytes, from the start of one scanline in plane 0 to the next.
	ptrdiff_t		modulo;		// Distance, in bytes, from the end of one scanline in plane 0 to the start of the next.
	ptrdiff_t		size;		// Size of plane 0, including padding.
	ptrdiff_t		offset;		// Offset from beginning of buffer to beginning of plane 0.

	uint32 *Address32(int x, int y) const {
		return Address32i(x, h-y-1);
	}

	uint32 *Address32i(int x, int y) const {
		return (uint32 *)((char *)data + y*pitch + x*4);
	}

	void AlignTo4() {
		pitch = w << 2;
	}

	void AlignTo8() {
		pitch = ((w+1)&~1) << 2;
	}
};

class VDXFBitmap : public VDXBitmap {
public:
	enum {
		/// Set in paramProc if the filter requires a Win32 GDI display context
		/// for a bitmap. (Deprecated as of API V12 - do not use)
		NEEDS_HDC		= 0x00000001L,
	};

	uint32		dwFlags;
	VDXHDC		hdc;

	uint32	mFrameRateHi;		// Frame rate numerator (V1.7.4+)
	uint32	mFrameRateLo;		// Frame rate denominator (V1.7.4+)
	sint64	mFrameCount;		// Frame count; -1 if unlimited or indeterminate (V1.7.4+)

	VDXPixmapLayout	*mpPixmapLayout;
	const VDXPixmap	*mpPixmap;

	uint32	mAspectRatioHi;				///< Pixel aspect ratio fraction (numerator).	0/0 = unknown
	uint32	mAspectRatioLo;				///< Pixel aspect ratio fraction (denominator).

	sint64	mFrameNumber;				///< Current frame number (zero based).
	sint64	mFrameTimestampStart;		///< Starting timestamp of frame, in 100ns units.
	sint64	mFrameTimestampEnd;			///< Ending timestamp of frame, in 100ns units.
	sint64	mCookie;					///< Cookie supplied when frame was requested.
};

// VDXFilterActivation: This is what is actually passed to filters at runtime.

class VDXFilterActivation {
public:
	const VDXFilterDefinition *filter;		// 
	void *filter_data;
	VDXFBitmap&	dst;
	VDXFBitmap&	src;
	VDXFBitmap	*_reserved0;
	VDXFBitmap	*const last;
	uint32		x1;
	uint32		y1;
	uint32		x2;
	uint32		y2;

	VDXFilterStateInfo	*pfsi;
	IVDXFilterPreview	*ifp;
	IVDXFilterPreview2	*ifp2;			// (V11+)

	uint32		mSourceFrameCount;		// (V14+)
	VDXFBitmap *const *mpSourceFrames;	// (V14+)
	VDXFBitmap *const *mpOutputFrames;	// (V14+)
};

// These flags must match those in cpuaccel.h!

#ifndef f_VIRTUALDUB_CPUACCEL_H
#define CPUF_SUPPORTS_CPUID			(0x00000001L)
#define CPUF_SUPPORTS_FPU			(0x00000002L)
#define CPUF_SUPPORTS_MMX			(0x00000004L)
#define CPUF_SUPPORTS_INTEGER_SSE	(0x00000008L)
#define CPUF_SUPPORTS_SSE			(0x00000010L)
#define CPUF_SUPPORTS_SSE2			(0x00000020L)
#define CPUF_SUPPORTS_3DNOW			(0x00000040L)
#define CPUF_SUPPORTS_3DNOW_EXT		(0x00000080L)
#endif

struct VDXFilterFunctions {
	VDXFilterDefinition *(__cdecl *addFilter)(VDXFilterModule *, VDXFilterDefinition *, int fd_len);
	void (__cdecl *removeFilter)(VDXFilterDefinition *);
	bool (__cdecl *isFPUEnabled)();
	bool (__cdecl *isMMXEnabled)();
	void (__cdecl *InitVTables)(VDXFilterVTbls *);

	// These functions permit you to throw MyError exceptions from a filter.
	// YOU MUST ONLY CALL THESE IN runProc, initProc, and startProc.

	void (__cdecl *ExceptOutOfMemory)();						// ADDED: V6 (VirtualDub 1.4)
	void (__cdecl *Except)(const char *format, ...);			// ADDED: V6 (VirtualDub 1.4)

	// These functions are callable at any time.

	long (__cdecl *getCPUFlags)();								// ADDED: V6 (VirtualDub 1.4)
	long (__cdecl *getHostVersionInfo)(char *buffer, int len);	// ADDED: V7 (VirtualDub 1.4d)
};





///////////////////////////////////////////////////////////////////////////

class VDXScriptValue;
class VDXScriptError;
struct VDXScriptObject;

class VDXScriptError {
public:
	enum {
		PARSE_ERROR=1,
		SEMICOLON_EXPECTED,
		IDENTIFIER_EXPECTED,

		TYPE_INT_REQUIRED,
		TYPE_ARRAY_REQUIRED,
		TYPE_FUNCTION_REQUIRED,
		TYPE_OBJECT_REQUIRED,

		OBJECT_MEMBER_NAME_REQUIRED,
		FUNCCALLEND_EXPECTED,
		TOO_MANY_PARAMS,
		DIVIDE_BY_ZERO,
		VAR_NOT_FOUND,
		MEMBER_NOT_FOUND,
		OVERLOADED_FUNCTION_NOT_FOUND,
		IDENT_TOO_LONG,
		OPERATOR_EXPECTED,
		CLOSEPARENS_EXPECTED,
		CLOSEBRACKET_EXPECTED,

		VAR_UNDEFINED,

		OUT_OF_STRING_SPACE,
		OUT_OF_MEMORY,
		INTERNAL_ERROR,
		EXTERNAL_ERROR,

		FCALL_OUT_OF_RANGE,
		FCALL_INVALID_PTYPE,
		FCALL_UNKNOWN_STR,

		ARRAY_INDEX_OUT_OF_BOUNDS,

		NUMERIC_OVERFLOW,
		STRING_NOT_AN_INTEGER_VALUE,
		STRING_NOT_A_REAL_VALUE,

		ASSERTION_FAILED,
		AMBIGUOUS_CALL,
		CANNOT_CAST
	};
};

class IVDXScriptInterpreter {
public:
	virtual	void _placeholder1() {}
	virtual void _placeholder2(void *, void *) {}
	virtual void _placeholder3(char *s) {}

	virtual void ScriptError(int e)=0;
	virtual void _placeholder4(VDXScriptError& cse) {}
	virtual char** AllocTempString(long l)=0;

	virtual void _placeholder5() {}
};

#define EXT_SCRIPT_ERROR(x)	(isi->ScriptError((VDXScriptError::x)))

typedef VDXScriptValue (*VDXScriptFunctionPtr)(IVDXScriptInterpreter *, void *, const VDXScriptValue *, int);
typedef void (*VDXScriptVoidFunctionPtr)(IVDXScriptInterpreter *, void *, const VDXScriptValue *, int);
typedef int (*VDXScriptIntFunctionPtr)(IVDXScriptInterpreter *, void *, const VDXScriptValue *, int);

struct VDXScriptFunctionDef {
	VDXScriptFunctionPtr func_ptr;
	const char *name;
	const char *arg_list;
};

struct VDXScriptObject {
	void *_lookup;							// reserved - set to NULL
	VDXScriptFunctionDef	*func_list;
	void *_obj_list;						// reserved - set to NULL
};

class VDXScriptValue {
public:
	enum { T_VOID, T_INT, T_PINT, T_STR, T_ARRAY, T_OBJECT, T_FNAME, T_FUNCTION, T_VARLV, T_LONG, T_DOUBLE } type;
	VDXScriptObject *thisPtr;
	union {
		int i;
		char **s;
		sint64 l;
		double d;
	} u;

	VDXScriptValue()					{ type = T_VOID; }
	VDXScriptValue(int i)				{ type = T_INT;			u.i = i; }
	VDXScriptValue(sint64 l)			{ type = T_LONG;		u.l = l; }
	VDXScriptValue(double d)			{ type = T_DOUBLE;		u.d = d; }
	VDXScriptValue(char **s)			{ type = T_STR;			u.s = s; }

	bool isVoid() const					{ return type == T_VOID; }
	bool isInt() const					{ return type == T_INT; }
	bool isString() const				{ return type == T_STR; }
	bool isLong() const					{ return type == T_LONG; }
	bool isDouble() const				{ return type == T_DOUBLE; }

	int		asInt() const				{ return u.i; }
	sint64	asLong() const				{ return u.l; }
	double	asDouble() const			{ return u.d; }
	char **	asString() const 			{ return u.s; }
};

#ifdef _MSC_VER
	#pragma pack(pop)
#endif

#endif
