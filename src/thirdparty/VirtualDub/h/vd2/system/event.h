//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2006 Avery Lee, All Rights Reserved.
//
//	Beginning with 1.6.0, the VirtualDub system library is licensed
//	differently than the remainder of VirtualDub.  This particular file is
//	thus licensed as follows (the "zlib" license):
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

#ifndef f_VD2_SYSTEM_EVENT_H
#define f_VD2_SYSTEM_EVENT_H

// Because Visual C++ uses different pointer-to-member representations for
// different inheritance regimes, we have to include a whole lot of stupid
// logic to detect and switch code paths based on the inheritance used.
// We detect the inheritance by the size of the member function pointer.
//
// Some have managed to make faster and more compact delegates by hacking
// into the PMT representation and pre-folding the this pointer adjustment.
// I'm avoiding this for now because (a) it's even less portable than what
// we have here, and (b) that fails if the object undergoes a change in
// virtual table status while the delegate is alive (which is possible
// during construction/destruction).
//
// Note: We can't handle virtual inheritance here because on X64, MSVC uses
// 16 bytes for both multiple and virtual inheritance cases.

#ifdef _MSC_VER
	class __single_inheritance VDDelegateHolderS;
	class __multiple_inheritance VDDelegateHolderM;
#else
	class VDDelegateHolderS;
#endif

class VDDelegate;

struct VDDelegateNode {
	VDDelegateNode *mpNext, *mpPrev;

	void (*mpCallback)(void *src, const void *info, VDDelegateNode&);
	void *mpObj;

#ifdef _MSC_VER
	union {
		void (VDDelegateHolderS::*mpFnS)();
		void (VDDelegateHolderM::*mpFnM)();
	};
#else
	void (VDDelegateHolderS::*mpFnS)();
#endif
};

class VDEventBase {
protected:
	VDEventBase();
	~VDEventBase();

	void Add(VDDelegate&);
	void Remove(VDDelegate&);
	void Raise(void *src, const void *info);

	VDDelegateNode mAnchor;
};

template<class Source, class ArgType>
class VDDelegateBinding {
public:
	VDDelegate *mpBoundDelegate;
};

template<class T, class Source, class ArgType>
struct VDDelegateAdapterS {
	typedef void (T::*T_Fn)(Source *, const ArgType&);
	typedef void (T::*T_Fn2)(Source *, ArgType);

	static void Init(VDDelegateNode& dst, T_Fn fn) {
		dst.mpCallback = Fn;
		dst.mpFnS = reinterpret_cast<void(VDDelegateHolderS::*)()>(fn);
	}

	static void Init(VDDelegateNode& dst, T_Fn2 fn) {
		dst.mpCallback = Fn2;
		dst.mpFnS = reinterpret_cast<void(VDDelegateHolderS::*)()>(fn);
	}

	static void Fn(void *src, const void *info, VDDelegateNode& del) {
		return (((T *)del.mpObj)->*reinterpret_cast<T_Fn>(del.mpFnS))(static_cast<Source *>(src), *static_cast<const ArgType *>(info));
	}

	static void Fn2(void *src, const void *info, VDDelegateNode& del) {
		return (((T *)del.mpObj)->*reinterpret_cast<T_Fn2>(del.mpFnS))(static_cast<Source *>(src), *static_cast<const ArgType *>(info));
	}
};

template<int size>
class VDDelegateAdapter {
public:
	template<class T, class Source, class ArgType>
	struct AdapterLookup {
		typedef VDDelegateAdapterS<T, Source, ArgType> result;
	};
};

#ifdef _MSC_VER
template<class T, class Source, class ArgType>
struct VDDelegateAdapterM {
	typedef void (T::*T_Fn)(Source *, const ArgType&);
	typedef void (T::*T_Fn2)(Source *, ArgType);

	static void Init(VDDelegateNode& dst, T_Fn fn) {
		dst.mpCallback = Fn;
		dst.mpFnM = reinterpret_cast<void(VDDelegateHolderM::*)()>(fn);
	}

	static void Init(VDDelegateNode& dst, T_Fn2 fn) {
		dst.mpCallback = Fn2;
		dst.mpFnM = reinterpret_cast<void(VDDelegateHolderM::*)()>(fn);
	}

	static void Fn(void *src, const void *info, VDDelegateNode& del) {
		return (((T *)del.mpObj)->*reinterpret_cast<T_Fn>(del.mpFnM))(static_cast<Source *>(src), *static_cast<const ArgType *>(info));
	}

	static void Fn2(void *src, const void *info, VDDelegateNode& del) {
		return (((T *)del.mpObj)->*reinterpret_cast<T_Fn2>(del.mpFnM))(static_cast<Source *>(src), *static_cast<const ArgType *>(info));
	}
};


template<>
class VDDelegateAdapter<sizeof(void (VDDelegateHolderM::*)())> {
public:
	template<class T, class Source, class ArgType>
	struct AdapterLookup {
		typedef VDDelegateAdapterM<T, Source, ArgType> result;
	};
};
#endif

class VDDelegate : public VDDelegateNode {
	friend class VDEventBase;
public:
	VDDelegate();
	~VDDelegate();

	template<class T, class Source, class ArgType>
	VDDelegateBinding<Source, ArgType> operator()(T *obj, void (T::*fn)(Source *, const ArgType&)) {
		mpObj = obj;

		VDDelegateAdapter<sizeof fn>::template AdapterLookup<T, Source, ArgType>::result::Init(*this, fn);

		VDDelegateBinding<Source, ArgType> binding = {this};
		return binding;
	}

	template<class T, class Source, class ArgType>
	VDDelegateBinding<Source, ArgType> Bind(T *obj, void (T::*fn)(Source *, ArgType)) {
		mpObj = obj;

		VDDelegateAdapter<sizeof fn>::template AdapterLookup<T, Source, ArgType>::result::Init(*this, fn);

		VDDelegateBinding<Source, ArgType> binding = {this};
		return binding;
	}
};

template<class Source, class ArgType>
class VDEvent : public VDEventBase {
public:
	void operator+=(const VDDelegateBinding<Source, ArgType>& binding) {
		Add(*binding.mpBoundDelegate);
	}

	void operator-=(VDDelegate& del) {
		Remove(del);
	}

	void Raise(Source *src, const ArgType& args) {
		VDEventBase::Raise(src, &args);
	}
};

#endif
