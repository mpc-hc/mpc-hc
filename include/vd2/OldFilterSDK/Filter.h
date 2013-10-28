#ifndef f_FILTER_H
#define f_FILTER_H

#include <vd2/plugin/vdvideofilt.h>
#include <windows.h>

// This is really dumb, but necessary to support VTbls in C++.

typedef struct VDXFilterVTbls {
	void *pvtblVBitmap;
} FilterVTbls;

#ifdef VDEXT_MAIN
	VDXFilterVTbls g_vtbls;
#elif defined(VDEXT_NOTMAIN)
	extern VDXFilterVTbls g_vtbls;
#endif

#define INITIALIZE_VTBLS		ff->InitVTables(&g_vtbls)

#include "VBitmap.h"

typedef ::VDXFilterInitProc		FilterInitProc;
typedef ::VDXFilterDeinitProc		FilterDeinitProc;
typedef ::VDXFilterRunProc		FilterRunProc;
typedef ::VDXFilterParamProc		FilterParamProc;
typedef ::VDXFilterConfigProc		FilterConfigProc;
typedef ::VDXFilterStringProc		FilterStringProc;
typedef ::VDXFilterStartProc		FilterStartProc;
typedef ::VDXFilterEndProc		FilterEndProc;
typedef ::VDXFilterScriptStrProc	FilterScriptStrProc;
typedef ::VDXFilterStringProc2		FilterStringProc2;
typedef ::VDXFilterSerialize		FilterSerialize;
typedef ::VDXFilterDeserialize		FilterDeserialize;
typedef ::VDXFilterCopy			FilterCopy;

typedef ::VDXFilterModuleInitProc	FilterModuleInitProc;
typedef ::VDXFilterModuleDeinitProc	FilterModuleDeinitProc;

//////////

typedef ::VDXFilterPreviewButtonCallback	FilterPreviewButtonCallback;
typedef ::VDXFilterPreviewSampleCallback	FilterPreviewSampleCallback;

typedef ::IVDXFilterPreview			IFilterPreview;

//////////

typedef ::VDXFilterModule			FilterModule;
typedef ::VDXFilterDefinition		FilterDefinition;
typedef ::VDXFilterStateInfo		FilterStateInfo;
typedef ::VDXFBitmap			VFBitmap;

typedef ::VDXFilterActivation		FilterActivation;
typedef ::VDXFilterFunctions		FilterFunctions;

#endif
