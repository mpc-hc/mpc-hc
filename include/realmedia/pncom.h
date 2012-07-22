/****************************************************************************
 * 
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved.
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary 
 *  information of Progressive Networks, Inc, and is licensed
 *  subject to restrictions on use and distribution.
 *
 *
 *  Component Object Model defines, and macros
 *
 *  This file defines items required for COM interfaces in RealMedia and
 *  Progressive Networks SDKs.
 *
 *
 */

#ifndef _PNCOM_H_
#define _PNCOM_H_

#include "pntypes.h"		/* Needed for most type definitions */
#include "string.h"

// have to use the double expansion to get the prescan level

#define STATCONCAT1(w,x,y,z) STATCONCAT2(w,x,y,z)
#define STATCONCAT2(w,x,y,z) w##x##y##z  

#ifdef _STATICALLY_LINKED
#ifndef _PLUGINNAME
#define ENTRYPOINT(func) STATCONCAT1(entrypoint_error_symbol_should_not_be_needed,_PLUGINNAME,_,func)
#else /* _PLUGINNAME */
#define ENTRYPOINT(func) STATCONCAT1(entrypoint_for_,_PLUGINNAME,_,func)
#endif
#else /* _STATICALLY_LINKED */
#define ENTRYPOINT(func) func
#endif

/*
 * We include objbase.h when building for windows so that pncom.h can 
 * easily be used in any windows code.
 */
#ifdef _WIN32
#include <objbase.h>
#endif	/* _WIN32 */

#include "pnresult.h"

/*
 *  REF:
 *	Use this for reference parameters, so that C users can 
 *	use the interface as well.
 */
#if defined(__cplusplus)
#define REF(type)   type&
#else
#define REF(type)	const type * const
#endif

/*
 *  CONSTMETHOD:
 *	Use this for constant methods in an interface 
 *	Compiles away under C
 */
#if !defined( CONSTMETHOD )

#if defined(__cplusplus)
#define CONSTMETHOD	const
#else
#define CONSTMETHOD
#endif

#endif
/*
 *  CALL:
 *
 *	Used by C users to easily call a function through an interface
 *
 *  EXAMPLE:
 *
 *	pIFooObject->CALL(IFoo,DoSomething)(bar);
 *
 */
#if !defined(__cplusplus) || defined(CINTERFACE)
#define CALL(iface, func) iface##Vtbl->func
#endif


#define _INTERFACE struct

/*
 * If useing windows.h or the windows implementation of COM
 * these defines are not needed.
 */
#if !defined( _OBJBASE_H_ ) 

#ifdef _WIN16
typedef unsigned int	MMRESULT;
#define FAR             _far
#else
#define FAR             
#endif /* WIN16 */
#define PASCAL          _pascal
#define CDECL           _cdecl

/*
 * EXTERN_C
 */
#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C    extern "C"
#else
#define EXTERN_C    extern
#endif
#endif

#ifdef OLDERRORCODES
#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev,fac,code) ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))))
#endif /*MAKE_HRESULT*/
#endif /* OLDERRORCODES */

/*
 *  STDMETHODCALLTYPE
 */
#ifndef STDMETHODCALLTYPE
#if defined(_WIN32) || defined(_MPPC_)
#ifdef _MPPC_
#define STDMETHODCALLTYPE       __cdecl
#else
#define STDMETHODCALLTYPE       __stdcall
#endif
#elif defined(_WIN16)
#define STDMETHODCALLTYPE       __export FAR CDECL
#else
#define STDMETHODCALLTYPE
#endif
#endif

/*
 *  STDMETHODVCALLTYPE
 */
#ifndef STDMETHODVCALLTYPE
#if defined(_WINDOWS) || defined(_MPPC_)
#define STDMETHODVCALLTYPE      __cdecl
#else
#define STDMETHODVCALLTYPE
#endif
#endif

/*
 *  STDAPICALLTYPE
 */
#ifndef STDAPICALLTYPE
#if defined(_WIN32) || defined(_MPPC_)
#define STDAPICALLTYPE          __stdcall
#elif defined(_WIN16)
#define STDAPICALLTYPE          __export FAR PASCAL
#else
#define STDAPICALLTYPE
#endif
#endif

/*
 *  STDAPIVCALLTYPE
 */
#ifndef STDAPIVCALLTYPE
#if defined(_WINDOWS) || defined(_MPPC_)
#define STDAPIVCALLTYPE         __cdecl
#else
#define STDAPIVCALLTYPE
#endif
#endif

/*
 *  Standard API defines:
 *
 *	NOTE: the 'V' versions allow Variable Argument lists.
 *
 *	STDAPI
 *	STDAPI_(type)
 *	STDAPIV
 *	STDAPIV_(type)
 */
#ifndef STDAPI
#define STDAPI                  EXTERN_C PN_RESULT STDAPICALLTYPE
#endif
#ifndef STDAPI_
#define STDAPI_(type)           EXTERN_C type STDAPICALLTYPE
#endif
#ifndef STDAPIV
#define STDAPIV                 EXTERN_C PN_RESULT STDAPIVCALLTYPE
#endif
#ifndef STDAPIV_
#define STDAPIV_(type)          EXTERN_C type STDAPIVCALLTYPE
#endif


/*
 *  Standard Interface Method defines:
 *
 *	NOTE: the 'V' versions allow Variable Argument lists.
 *
 *	STDMETHODIMP
 *	STDMETHODIMP_(type)
 *	STDMETHODIMPV
 *	STDMETHODIMPV_(type)
 */
#ifndef STDMETHODIMP
#define STDMETHODIMP            PN_RESULT STDMETHODCALLTYPE
#endif
#ifndef STDMETHODIMP_
#define STDMETHODIMP_(type)     type STDMETHODCALLTYPE
#endif
#ifndef STDMETHODIMPV
#define STDMETHODIMPV           PN_RESULT STDMETHODVCALLTYPE
#endif
#ifndef STDMETHODIMPV_
#define STDMETHODIMPV_(type)    type STDMETHODVCALLTYPE
#endif


/*
 *
 *  Interface Declaration
 *
 *	These are macros for declaring interfaces.  They exist so that
 *	a single definition of the interface is simulataneously a proper
 *	declaration of the interface structures (C++ abstract classes)
 *	for both C and C++.
 *		
 *	DECLARE_INTERFACE(iface) is used to declare an interface that does
 *	not derive from a base interface.
 *	DECLARE_INTERFACE_(iface, baseiface) is used to declare an interface
 *	that does derive from a base interface.
 *		
 *	By default if the source file has a .c extension the C version of
 *	the interface declaratations will be expanded; if it has a .cpp
 *	extension the C++ version will be expanded. if you want to force
 *	the C version expansion even though the source file has a .cpp
 *	extension, then define the macro "CINTERFACE".
 *	eg.     cl -DCINTERFACE file.cpp
 *		
 *	Example Interface declaration:
 *		
 *	    #undef  INTERFACE
 *	    #define INTERFACE   IClassFactory
 *	
 *	    DECLARE_INTERFACE_(IClassFactory, IUnknown)
 *	    {
 *		// *** IUnknown methods 
 *	        STDMETHOD(QueryInterface) (THIS_
 *	                                  REFIID riid,
 *	                                  LPVOID FAR* ppvObj) PURE;
 *	        STDMETHOD_(ULONG,AddRef) (THIS) PURE;
 *	        STDMETHOD_(ULONG,Release) (THIS) PURE;
 *		
 *		// *** IClassFactory methods ***
 *		STDMETHOD(CreateInstance) (THIS_
 *		                          LPUNKNOWN pUnkOuter,
 *		                          REFIID riid,
 *		                          LPVOID FAR* ppvObject) PURE;
 *	    };
 *		
 *	Example C++ expansion:
 *		
 *	    struct FAR IClassFactory : public IUnknown
 *	    {
 *	        virtual PN_RESULT STDMETHODCALLTYPE QueryInterface(
 *	                                            IID FAR& riid,
 *	                                            LPVOID FAR* ppvObj) = 0;
 *	        virtual PN_RESULT STDMETHODCALLTYPE AddRef(void) = 0;
 *	        virtual PN_RESULT STDMETHODCALLTYPE Release(void) = 0;
 *	        virtual PN_RESULT STDMETHODCALLTYPE CreateInstance(
 *	                                        LPUNKNOWN pUnkOuter,
 *	                                        IID FAR& riid,
 *	                                        LPVOID FAR* ppvObject) = 0;
 *	    };
 *	
 *	    NOTE: Our documentation says '#define interface class' but we use
 *	    'struct' instead of 'class' to keep a lot of 'public:' lines
 *	    out of the interfaces.  The 'FAR' forces the 'this' pointers to
 *	    be far, which is what we need.
 *		
 *	Example C expansion:
 *		
 *	    typedef struct IClassFactory
 *	    {
 *	        const struct IClassFactoryVtbl FAR* lpVtbl;
 *	    } IClassFactory;
 *		
 *	    typedef struct IClassFactoryVtbl IClassFactoryVtbl;
 *		
 *	    struct IClassFactoryVtbl
 *	    {
 *	        PN_RESULT (STDMETHODCALLTYPE * QueryInterface) (
 *	                                            IClassFactory FAR* This,
 *	                                            IID FAR* riid,
 *	                                            LPVOID FAR* ppvObj) ;
 *	        PN_RESULT (STDMETHODCALLTYPE * AddRef) (IClassFactory FAR* This) ;
 *	        PN_RESULT (STDMETHODCALLTYPE * Release) (IClassFactory FAR* This) ;
 *	        PN_RESULT (STDMETHODCALLTYPE * CreateInstance) (
 *	                                            IClassFactory FAR* This,
 *	                                            LPUNKNOWN pUnkOuter,
 *	                                            IID FAR* riid,
 *	                                            LPVOID FAR* ppvObject);
 *	        PN_RESULT (STDMETHODCALLTYPE * LockServer) (
 *	                                            IClassFactory FAR* This,
 *													BOOL fLock);
 *	    };
 *
 */

#if defined(__cplusplus) && !defined(CINTERFACE)
#define _INTERFACE struct
#define STDMETHOD(method)       virtual PN_RESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type,method) virtual type STDMETHODCALLTYPE method
#define PURE                    = 0
#define THIS_
#define THIS                    void

#if defined(_WINDOWS) && defined(EXPORT_CLASSES)
#define DECLARE_INTERFACE(iface)    _INTERFACE PNEXPORT_CLASS iface
#define DECLARE_INTERFACE_(iface, baseiface)    _INTERFACE PNEXPORT_CLASS iface : public baseiface
#else
#define DECLARE_INTERFACE(iface)    _INTERFACE iface
#define DECLARE_INTERFACE_(iface, baseiface)    _INTERFACE iface : public baseiface
#endif // defined(_WINDOWS) && defined(EXPORT_CLASSES)

#if !defined(BEGIN_INTERFACE)
#if defined(_MPPC_)  && \
    ( (defined(_MSC_VER) || defined(__SC__) || defined(__MWERKS__)) && \
    !defined(NO_NULL_VTABLE_ENTRY) )
   #define BEGIN_INTERFACE virtual void a() {}
   #define END_INTERFACE
#else
   #define BEGIN_INTERFACE
   #define END_INTERFACE
#endif
#endif


#else

#define _INTERFACE               struct

#define STDMETHOD(method)       PN_RESULT (STDMETHODCALLTYPE * method)
#define STDMETHOD_(type,method) type (STDMETHODCALLTYPE * method)

#if !defined(BEGIN_INTERFACE)
#if defined(_MPPC_)
    #define BEGIN_INTERFACE       void    *b;
    #define END_INTERFACE
#else
    #define BEGIN_INTERFACE
    #define END_INTERFACE
#endif
#endif


#define PURE
#define THIS_                   INTERFACE FAR* This,
#define THIS                    INTERFACE FAR* This
#ifdef CONST_VTABLE
#undef CONST_VTBL
#define CONST_VTBL const
#define DECLARE_INTERFACE(iface)    typedef _INTERFACE iface { \
                                    const struct iface##Vtbl FAR* lpVtbl; \
                                } iface; \
                                typedef const struct iface##Vtbl iface##Vtbl; \
                                const struct iface##Vtbl
#else
#undef CONST_VTBL
#define CONST_VTBL
#define DECLARE_INTERFACE(iface)    typedef _INTERFACE iface { \
                                    struct iface##Vtbl FAR* lpVtbl; \
                                } iface; \
                                typedef struct iface##Vtbl iface##Vtbl; \
                                struct iface##Vtbl
#endif
#define DECLARE_INTERFACE_(iface, baseiface)    DECLARE_INTERFACE(iface)

#endif

/*
 *  COMMON TYPES
 */

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct  _GUID
    {
    ULONG32		Data1;
    UINT16		Data2;
    UINT16		Data3;
    UCHAR		Data4[ 8 ];
    }	GUID;

#endif

#if !defined( __IID_DEFINED__ )
#define __IID_DEFINED__
typedef GUID IID;
#define IID_NULL            GUID_NULL
typedef GUID CLSID;
#define CLSID_NULL          GUID_NULL

#if defined(__cplusplus)
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID &
#endif

#ifndef _REFIID_DEFINED
#define _REFIID_DEFINED
#define REFIID              const IID &
#endif

#ifndef _REFCLSID_DEFINED
#define _REFCLSID_DEFINED
#define REFCLSID            const CLSID &
#endif

#else
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID * const
#endif
#ifndef _REFIID_DEFINED
#define _REFIID_DEFINED
#define REFIID              const IID * const
#endif
#ifndef _REFCLSID_DEFINED
#define _REFCLSID_DEFINED
#define REFCLSID            const CLSID * const
#endif
#endif
#endif


/*
 *
 * macros to define byte pattern for a GUID.
 *      Example: DEFINE_GUID(GUID_XXX, a, b, c, ...);
 *
 * Each dll/exe must initialize the GUIDs once.  This is done in one of
 * two ways.  If you are not using precompiled headers for the file(s) which
 * initializes the GUIDs, define INITGUID before including objbase.h.  This
 * is how OLE builds the initialized versions of the GUIDs which are included
 * in ole2.lib.  The GUIDs in ole2.lib are all defined in the same text
 * segment GUID_TEXT.
 *
 * The alternative (which some versions of the compiler don't handle properly;
 * they wind up with the initialized GUIDs in a data, not a text segment),
 * is to use a precompiled version of objbase.h and then include initguid.h
 * after objbase.h followed by one or more of the guid defintion files.
 *
 */

#if !defined (INITGUID) || (defined (_STATICALLY_LINKED) && !defined(NCIHACK))
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID FAR name
#else

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#endif

#ifndef _VXWORKS
#include <memory.h>		/* for memcmp */
#endif

#ifdef __cplusplus
inline BOOL IsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
    return !memcmp(&rguid1, &rguid2, sizeof(GUID));
}

inline void SetGUID(GUID& rguid1, REFGUID rguid2)
{
    memcpy(&rguid1, &rguid2, sizeof(GUID));
}
#else
#define IsEqualGUID(rguid1, rguid2) (!memcmp(rguid1, rguid2, sizeof(GUID)))
#define SetGUID(rguid1, rguid2) (memcpy(rguid1, rguid2, sizeof(GUID)))
#endif

#define IsEqualIID(riid1, riid2) IsEqualGUID(riid1, riid2)
#define IsEqualCLSID(rclsid1, rclsid2) IsEqualGUID(rclsid1, rclsid2)

#define SetIID(riid1, riid2)	    SetGUID(riid1, riid2)
#define SetCLSID(rclsid1, rclsid2)  SetGUID(rclsid1, rclsid2)

#ifdef __cplusplus

/*
 * Because GUID is defined elsewhere in WIN32 land, the operator == and !=
 * are moved outside the class to global scope.
 */

inline BOOL operator==(const GUID& guidOne, const GUID& guidOther)
{
    return !memcmp(&guidOne,&guidOther,sizeof(GUID));
}

inline BOOL operator!=(const GUID& guidOne, const GUID& guidOther)
{
    return !(guidOne == guidOther);
}

#endif


/****************************************************************************
 * 
 *  Interface:
 *
 *	IUnknown
 *
 *  Purpose:
 *
 *	Base class of all interfaces. Defines life time management and
 *	support for dynamic cast.
 *
 *  IID_IUnknown:
 *
 *	{00000000-0000-0000-C000000000000046}
 *
 */
DEFINE_GUID(IID_IUnknown, 0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

#undef  INTERFACE
#define INTERFACE   IUnknown

DECLARE_INTERFACE(IUnknown)
{
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IMalloc
 *
 *  Purpose:
 *
 *	Basic memory management interface.
 *
 *  IID_IMalloc:
 *
 *	{00000002-0000-0000-C000000000000046}
 *
 */
DEFINE_GUID(IID_IMalloc, 00000002, 0x0000, 0x0000, 0xC0, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

#undef  INTERFACE
#define INTERFACE   IMalloc

DECLARE_INTERFACE_(IMalloc, IUnknown)
{
    /*
     * IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     * IMalloc methods
     */
    STDMETHOD_(void*,Alloc)	    (THIS_
				    UINT32  /*IN*/ count) PURE;

    STDMETHOD_(void*,Realloc)	    (THIS_
				    void*   /*IN*/ pMem,
				    UINT32  /*IN*/ count) PURE;

    STDMETHOD_(void,Free)	    (THIS_
				    void*   /*IN*/ pMem) PURE;

    STDMETHOD_(UINT32,GetSize)	    (THIS_
				    void*   /*IN*/ pMem) PURE;

    STDMETHOD_(BOOL,DidAlloc)	    (THIS_
				    void*   /*IN*/ pMem) PURE;

    STDMETHOD_(void,HeapMinimize)   (THIS) PURE;
};

/*
 *
 *  Synchronization: NOTE: These should be made thread safe or use built
 *  in synchronization support in an OS that supports it.
 *
 */
#define InterlockedIncrement(plong) (++(*(plong)))
#define InterlockedDecrement(plong) (--(*(plong)))

#else /* else case of !defined( _OBJBASE_H_ ) && !defined( _COMPOBJ_H_ ) */


/* Even in windows we want these GUID's defined... */

#if !(defined(INITGUID) && defined(USE_IUNKNOWN_AND_IMALLOC_FROM_UUID_LIB))
DEFINE_GUID(IID_IUnknown, 0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

DEFINE_GUID(IID_IMalloc, 00000002, 0x0000, 0x0000, 0xC0, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
#endif

#include <memory.h>		/* for memcmp */

#ifdef __cplusplus
inline void SetGUID(REFGUID rguid1, REFGUID rguid2)
{
    memcpy((void*)&rguid1, (void*)&rguid2, sizeof(GUID));
}
#else
#define SetGUID(rguid1, rguid2) (memcpy((void*)rguid1, (void*)rguid2, sizeof(GUID)))
#endif
#define SetIID(riid1, riid2)	    SetGUID(riid1, riid2)
#define SetCLSID(rclsid1, rclsid2)  SetGUID(rclsid1, rclsid2)

#endif /* !defined( _OBJBASE_H_ ) && !defined( _COMPOBJ_H_ )*/

#ifdef IsEqualIID
#undef IsEqualIID
#endif

#ifdef IsEqualCLSID
#undef IsEqualCLSID
#endif

#define IsEqualIID(riid1, riid2) RNIsEqualGUID(riid1, riid2)
#define IsEqualCLSID(rclsid1, rclsid2) RNIsEqualGUID(rclsid1, rclsid2)

#ifdef __cplusplus
inline BOOL RNIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
   return (((UINT32*) &rguid1)[0] == ((UINT32*) &rguid2)[0]  &&
	    ((UINT32*) &rguid1)[1] == ((UINT32*) &rguid2)[1] &&
	    ((UINT32*) &rguid1)[2] == ((UINT32*) &rguid2)[2] &&
	    ((UINT32*) &rguid1)[3] == ((UINT32*) &rguid2)[3]);
}
#else
#define RNIsEqualGUID(rguid1, rguid2)		\
	(((UINT32*) &rguid1)[0] == ((UINT32*) &rguid2)[0]  &&   \
	    ((UINT32*) &rguid1)[1] == ((UINT32*) &rguid2)[1] && \
	    ((UINT32*) &rguid1)[2] == ((UINT32*) &rguid2)[2] && \
	    ((UINT32*) &rguid1)[3] == ((UINT32*) &rguid2)[3]);
#endif

/****************************************************************************
 *
 *  Putting the following macro in the definition of your class will overload
 *  new and delete for that object.  New will then take an IMalloc* from
 *  which to allocate memory from and store it in the begining of the
 *  memory which it will return.  Delete will grab this IMalloc* from
 *  the beginning of the mem and use this pointer to deallocate the mem.
 *
 *  Example useage:
 *  class A
 *  {
 *  public:
 *      A(int);
 *      ~A();
 *
 *      IMALLOC_MEM
 *  };
 *
 *  IMalloc* pMalloc;
 *  m_pContext->QueryInterface(IID_IMalloc, (void**)&pMalloc);
 *  A* p = new(pMalloc) A(0);
 *  pMalloc->Release();
 *  delete p;
 */

#define IMALLOC_MEM\
    void* operator new(size_t size, IMalloc* pMalloc)\
    {\
        void* pMem = pMalloc->Alloc(size + sizeof(IMalloc*));\
        *(IMalloc**)pMem = pMalloc;\
        pMalloc->AddRef();\
        return ((unsigned char*)pMem + sizeof(IMalloc*));\
    }\
\
    void operator delete(void* pMem)\
    {\
        pMem = (unsigned char*)pMem - sizeof(IMalloc*);\
        IMalloc* pMalloc = *(IMalloc**)pMem;\
        pMalloc->Free(pMem);\
        pMalloc->Release();\
    }\

#endif /* _PNCOM_H_ */
