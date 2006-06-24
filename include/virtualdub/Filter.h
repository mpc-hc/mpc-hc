#ifndef f_FILTER_H
#define f_FILTER_H

#include <windows.h>

// This is really dumb, but necessary to support VTbls in C++.

struct FilterVTbls {
	void *pvtblVBitmap;
};

#ifdef VDEXT_MAIN
struct FilterVTbls g_vtbls;
#elif defined(VDEXT_NOTMAIN)
extern struct FilterVTbls g_vtbls;
#endif

#define INITIALIZE_VTBLS		ff->InitVTables(&g_vtbls)

#include "VBitmap.h"

//////////////////

struct CScriptObject;

//////////////////

enum {
	FILTERPARAM_SWAP_BUFFERS	= 0x00000001L,
	FILTERPARAM_NEEDS_LAST		= 0x00000002L,
};

///////////////////

class VFBitmap;
class FilterActivation;
struct FilterFunctions;

typedef int  (*FilterInitProc     )(FilterActivation *fa, const FilterFunctions *ff);
typedef void (*FilterDeinitProc   )(FilterActivation *fa, const FilterFunctions *ff);
typedef int  (*FilterRunProc      )(const FilterActivation *fa, const FilterFunctions *ff);
typedef long (*FilterParamProc    )(FilterActivation *fa, const FilterFunctions *ff);
typedef int  (*FilterConfigProc   )(FilterActivation *fa, const FilterFunctions *ff, HWND hWnd);
typedef int  (*FilterConfig2Proc  )(FilterActivation *fa, const FilterFunctions *ff, HWND hWnd);
typedef void (*FilterStringProc   )(const FilterActivation *fa, const FilterFunctions *ff, char *buf);
typedef int  (*FilterStartProc    )(FilterActivation *fa, const FilterFunctions *ff);
typedef int  (*FilterEndProc      )(FilterActivation *fa, const FilterFunctions *ff);
typedef bool (*FilterScriptStrProc)(FilterActivation *fa, const FilterFunctions *, char *, int);

typedef int (__cdecl *FilterModuleInitProc)(struct FilterModule *fm, const FilterFunctions *ff, int& vdfd_ver, int& vdfd_compat);
typedef void (__cdecl *FilterModuleDeinitProc)(struct FilterModule *fm, const FilterFunctions *ff);

//////////

typedef void (__cdecl *FilterPreviewButtonCallback)(bool fNewState, void *pData);
typedef void (__cdecl *FilterPreviewSampleCallback)(VFBitmap *, long lFrame, long lCount, void *pData);

class IFilterPreview {
public:
	virtual void SetButtonCallback(FilterPreviewButtonCallback, void *)=0;
	virtual void SetSampleCallback(FilterPreviewSampleCallback, void *)=0;

	virtual bool isPreviewEnabled()=0;
	virtual void Toggle(HWND)=0;
	virtual void Display(HWND, bool)=0;
	virtual void RedoFrame()=0;
	virtual void RedoSystem()=0;
	virtual void UndoSystem()=0;
	virtual void InitButton(HWND)=0;
	virtual void Close()=0;
	virtual bool SampleCurrentFrame()=0;
	virtual long SampleFrames()=0;
};

//////////

#define VIRTUALDUB_FILTERDEF_VERSION		(4)
#define	VIRTUALDUB_FILTERDEF_COMPATIBLE		(4)

// v3: added lCurrentSourceFrame to FrameStateInfo
// v4: lots of additions (VirtualDub 1.2)

typedef struct FilterModule {
	struct FilterModule *next, *prev;
	HINSTANCE				hInstModule;
	FilterModuleInitProc	initProc;
	FilterModuleDeinitProc	deinitProc;
} FilterModule;

typedef struct FilterDefinition {

	struct FilterDefinition *next, *prev;
	FilterModule *module;

	char *				name;
	char *				desc;
	char *				maker;
	void *				private_data;
	int					inst_data_size;

	FilterInitProc		initProc;
	FilterDeinitProc	deinitProc;
	FilterRunProc		runProc;
	FilterParamProc		paramProc;
	FilterConfigProc	configProc;
	FilterStringProc	stringProc;
	FilterStartProc		startProc;
	FilterEndProc		endProc;

	CScriptObject	*script_obj;

	FilterScriptStrProc	fssProc;

} FilterDefinition;

//////////

// FilterStateInfo: contains dynamic info about file being processed

class FilterStateInfo {
public:
	long	lCurrentFrame;				// current output frame
	long	lMicrosecsPerFrame;			// microseconds per output frame
	long	lCurrentSourceFrame;		// current source frame
	long	lMicrosecsPerSrcFrame;		// microseconds per source frame
	long	lSourceFrameMS;				// source frame timestamp
	long	lDestFrameMS;				// output frame timestamp
};

// VFBitmap: VBitmap extended to hold filter-specific information

class VFBitmap : public VBitmap {
public:
	enum {
		NEEDS_HDC		= 0x00000001L,
	};

	DWORD	dwFlags;
	HDC		hdc;
};

// FilterActivation: This is what is actually passed to filters at runtime.

class FilterActivation {
public:
	FilterDefinition *filter;
	void *filter_data;
	VFBitmap &dst, &src;
	VFBitmap *rsrc, *last;
	unsigned long x1, y1, x2, y2;

	FilterStateInfo *pfsi;
	IFilterPreview *ifp;

	FilterActivation(VFBitmap& _dst, VFBitmap& _src) : dst(_dst), src(_src) {}
	FilterActivation(const FilterActivation& fa, VFBitmap& _dst, VFBitmap& _src);
};

struct FilterFunctions {
	FilterDefinition *(*addFilter)(FilterModule *, FilterDefinition *, int fd_len);
	void (*removeFilter)(FilterDefinition *);
	bool (*isFPUEnabled)();
	bool (*isMMXEnabled)();
	void (*InitVTables)(struct FilterVTbls *);
};

#endif
