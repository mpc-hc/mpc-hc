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

#ifndef f_VD2_VDXFRAME_VIDEOFILTER_H
#define f_VD2_VDXFRAME_VIDEOFILTER_H

#include <stdlib.h>
#include <stddef.h>
#include <new>

#include <vd2/plugin/vdvideofilt.h>

///////////////////////////////////////////////////////////////////////////
///	\class VDXVideoFilter
///
///	This class handles most of the grimy work of creating the interface
///	between your filter and VirtualDub.
///
class VDXVideoFilter {
public:
	VDXVideoFilter();
	virtual ~VDXVideoFilter();

	void SetHooks(VDXFilterActivation *fa, const VDXFilterFunctions *ff);

	// linkage routines

	virtual bool Init();
	virtual uint32 GetParams()=0;
	virtual void Start();
	virtual void Run() = 0;
	virtual void End();
	virtual bool Configure(VDXHWND hwnd);
	virtual void GetSettingString(char *buf, int maxlen);
	virtual void GetScriptString(char *buf, int maxlen);
	virtual int Serialize(char *buf, int maxbuf);
	virtual int Deserialize(const char *buf, int maxbuf);
	virtual sint64 Prefetch(sint64 frame);
	virtual bool Prefetch2(sint64 frame, IVDXVideoPrefetcher *prefetcher);

	virtual bool OnEvent(uint32 event, const void *eventData);
	virtual bool OnInvalidateCaches();

	static void __cdecl FilterDeinit   (VDXFilterActivation *fa, const VDXFilterFunctions *ff);
	static int  __cdecl FilterRun      (const VDXFilterActivation *fa, const VDXFilterFunctions *ff);
	static long __cdecl FilterParam    (VDXFilterActivation *fa, const VDXFilterFunctions *ff);
	static int  __cdecl FilterConfig   (VDXFilterActivation *fa, const VDXFilterFunctions *ff, VDXHWND hWnd);
	static int  __cdecl FilterStart    (VDXFilterActivation *fa, const VDXFilterFunctions *ff);
	static int  __cdecl FilterEnd      (VDXFilterActivation *fa, const VDXFilterFunctions *ff);
	static void __cdecl FilterString   (const VDXFilterActivation *fa, const VDXFilterFunctions *ff, char *buf);
	static bool __cdecl FilterScriptStr(VDXFilterActivation *fa, const VDXFilterFunctions *, char *, int);
	static void __cdecl FilterString2  (const VDXFilterActivation *fa, const VDXFilterFunctions *ff, char *buf, int maxlen);
	static int  __cdecl FilterSerialize    (VDXFilterActivation *fa, const VDXFilterFunctions *ff, char *buf, int maxbuf);
	static void __cdecl FilterDeserialize  (VDXFilterActivation *fa, const VDXFilterFunctions *ff, const char *buf, int maxbuf);
	static sint64 __cdecl FilterPrefetch(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, sint64 frame);
	static bool __cdecl FilterPrefetch2(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, sint64 frame, IVDXVideoPrefetcher *prefetcher);
	static bool __cdecl FilterEvent(const VDXFilterActivation *fa, const VDXFilterFunctions *ff, uint32 event, const void *eventData);

	// member variables
	VDXFilterActivation *fa;
	const VDXFilterFunctions *ff;

	static const VDXScriptFunctionDef sScriptMethods[];

protected:
	void SafePrintf(char *buf, int maxbuf, const char *format, ...);
};

///////////////////////////////////////////////////////////////////////////
// Script method support
//
// To declare a Config() script method, add
//
//	VDXVF_DECLARE_SCRIPT_METHODS()
//
// to the public portion of your class definition, and then add a method
// table at namespace scope:
//
//	VDXVF_BEGIN_SCRIPT_METHODS(YUVTransformFilter)
//	VDXVF_DEFINE_SCRIPT_METHOD(YUVTransformFilter, ScriptConfig, "iii")
//	VDXVF_END_SCRIPT_METHODS()
//
// Use VDXVF_DEFINE_SCRIPT_METHOD() for the first method, and then
// VDXVF_DEFINE_SCRIPT_METHOD2() for any overloads.

#define VDXVF_DECLARE_SCRIPT_METHODS()	static const VDXScriptFunctionDef sScriptMethods[]

#define VDXVF_BEGIN_SCRIPT_METHODS(klass) const VDXScriptFunctionDef klass::sScriptMethods[] = {
#define VDXVF_DEFINE_SCRIPT_METHOD(klass, method, args) { (VDXScriptFunctionPtr)VDXVideoFilterScriptAdapter<klass, &klass::method>::AdaptFn, "Config", "0" args },
#define VDXVF_DEFINE_SCRIPT_METHOD2(klass, method, args) { (VDXScriptFunctionPtr)VDXVideoFilterScriptAdapter<klass, &klass::method>::AdaptFn, NULL, "0" args },
#define VDXVF_END_SCRIPT_METHODS() { NULL } };

extern char VDXVideoFilterConfigureOverloadTest(bool (VDXVideoFilter::*)(VDXHWND));
extern double VDXVideoFilterConfigureOverloadTest(...);
extern char VDXVideoFilterPrefetchOverloadTest(sint64 (VDXVideoFilter::*)(sint64));
extern double VDXVideoFilterPrefetchOverloadTest(...);
extern char VDXVideoFilterPrefetch2OverloadTest(bool (VDXVideoFilter::*)(sint64, IVDXVideoPrefetcher *));
extern double VDXVideoFilterPrefetch2OverloadTest(...);

template<class T, void (T::*T_Method)(IVDXScriptInterpreter *, const VDXScriptValue *, int)>
class VDXVideoFilterScriptAdapter
{
public:
	static void AdaptFn(IVDXScriptInterpreter *isi, void *fa0, const VDXScriptValue *argv, int argc) {
		VDXFilterActivation *fa = (VDXFilterActivation *)fa0;
		VDXVideoFilter *base = *(VDXVideoFilter **)fa->filter_data;
		(static_cast<T *>(base)->*T_Method)(isi, argv, argc);
	}
};

template<class T>
class VDXVideoFilterScriptObjectAdapter {
public:
	static const VDXScriptObject sScriptObject;
};

template<class T>
const VDXScriptObject VDXVideoFilterScriptObjectAdapter<T>::sScriptObject = {
	NULL, (T::sScriptMethods == VDXVideoFilter::sScriptMethods) ? NULL : (VDXScriptFunctionDef *)static_cast<const VDXScriptFunctionDef *>(T::sScriptMethods), NULL
};

///////////////////////////////////////////////////////////////////////////
///	\class VDXVideoFilterDefinition
///
///	This template creates the FilterDefinition structure for you based on
///	your filter class.
///
template<class T>
class VDXVideoFilterDefinition : public VDXFilterDefinition {
public:
	VDXVideoFilterDefinition(const char *pszAuthor, const char *pszName, const char *pszDescription) {
		_next			= NULL;
		_prev			= NULL;
		_module			= NULL;

		name			= pszName;
		desc			= pszDescription;
		maker			= pszAuthor;
		private_data	= NULL;
		inst_data_size	= sizeof(T) + sizeof(VDXVideoFilter *);

		initProc		= FilterInit;
		deinitProc		= T::FilterDeinit;
		runProc			= T::FilterRun;
		paramProc		= T::FilterParam;
		configProc		= sizeof(VDXVideoFilterConfigureOverloadTest(&T::Configure)) > 1 ? T::FilterConfig : NULL;
		stringProc		= T::FilterString;
		startProc		= T::FilterStart;
		endProc			= T::FilterEnd;

		script_obj		= T::sScriptMethods ? const_cast<VDXScriptObject *>(&VDXVideoFilterScriptObjectAdapter<T>::sScriptObject) : 0;
		fssProc			= T::FilterScriptStr;

		stringProc2		= T::FilterString2;
		serializeProc	= T::FilterSerialize;
		deserializeProc	= T::FilterDeserialize;
		copyProc		= FilterCopy;
		copyProc2		= FilterCopy2;

		prefetchProc	= sizeof(VDXVideoFilterPrefetchOverloadTest(&T::Prefetch)) > 1 ? T::FilterPrefetch : NULL;
		prefetchProc2	= sizeof(VDXVideoFilterPrefetch2OverloadTest(&T::Prefetch2)) > 1 || sizeof(VDXVideoFilterPrefetchOverloadTest(&T::Prefetch)) > 1 ? T::FilterPrefetch2 : NULL;

		eventProc		= T::FilterEvent;
	}

private:
	static int  __cdecl FilterInit     (VDXFilterActivation *fa, const VDXFilterFunctions *ff) {
		T *pThis = new((char *)fa->filter_data + sizeof(VDXVideoFilter *)) T;
		*(VDXVideoFilter **)fa->filter_data = static_cast<VDXVideoFilter *>(pThis);

		pThis->SetHooks(fa, ff);

		try {
			if (!pThis->Init()) {
				pThis->~T();
				return 1;
			}

			return 0;
		} catch(...) {
			pThis->~T();
			throw;
		}
	}

	static void __cdecl FilterCopy         (VDXFilterActivation *fa, const VDXFilterFunctions *ff, void *dst) {
		T *p = new((char *)dst + sizeof(VDXVideoFilter *)) T(*static_cast<T *>(*reinterpret_cast<VDXVideoFilter **>(fa->filter_data)));
		p->ff = ff;
		*(VDXVideoFilter **)dst = p;
	}

	static void __cdecl FilterCopy2        (VDXFilterActivation *fa, const VDXFilterFunctions *ff, void *dst, VDXFilterActivation *fanew, const VDXFilterFunctions *ffnew) {
		T *p = new((char *)dst + sizeof(VDXVideoFilter *)) T(*static_cast<T *>(*reinterpret_cast<VDXVideoFilter **>(fa->filter_data)));
		p->ff = ffnew;
		p->fa = fanew;
		*(VDXVideoFilter **)dst = p;
	}
};

#endif
