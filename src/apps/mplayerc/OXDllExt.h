// ==========================================================================
// 			Specification : header file for MFC DLL extension builds
// ==========================================================================

// Copyright © Dundas Software Ltd. 1997 - 1998, All Rights Reserved
                          
// //////////////////////////////////////////////////////////////////////////

#if !defined(OX_CLASS_DECL) || !defined(OX_API_DECL) && !defined(OX_DATA_DECL)

#ifdef _BUILD_UTB_INTO_EXTDLL
#if !defined(WIN32) || !defined(_AFXEXT)
#pragma error("Wrong settings for UTB Extension DLL build")
#endif
#endif

#ifdef _LINK_TO_UTB_IN_EXTDLL
#if !defined(WIN32) || !defined(_AFXDLL)
#pragma error("Wrong settings for project that uses UTB Extension DLL")
#endif
#endif

#if defined(_LINK_TO_UTB_IN_EXTDLL) && defined(_BUILD_UTB_INTO_EXTDLL)
#pragma error("Error: both _BUILD_UTB_INTO_EXTDLL and _LINK_TO_UTB_IN_EXTDLL options has been specified")
#endif

// When including UTB classes into a MFC Extension DLL
#ifdef _BUILD_UTB_INTO_EXTDLL
	#ifndef OX_CLASS_DECL
		#define OX_CLASS_DECL		AFX_CLASS_EXPORT
	#endif
	#ifndef OX_API_DECL
		#define OX_API_DECL			AFX_API_EXPORT
	#endif
	#ifndef OX_DATA_DECL
		#define OX_DATA_DECL		AFX_DATA_EXPORT
	#endif
#elif defined(_LINK_TO_UTB_IN_EXTDLL)
// When linking to extension DLL that includes UTB classes 
	#ifndef OX_CLASS_DECL
		#define OX_CLASS_DECL		AFX_CLASS_IMPORT
	#endif
	#ifndef OX_API_DECL
		#define OX_API_DECL			AFX_API_IMPORT
	#endif
	#ifndef OX_DATA_DECL
		#define OX_DATA_DECL		AFX_DATA_IMPORT
	#endif
#else
	#ifndef OX_CLASS_DECL
		#define OX_CLASS_DECL	
	#endif
	#ifndef OX_API_DECL
		#define OX_API_DECL    
	#endif
	#ifndef OX_DATA_DECL
		#define OX_DATA_DECL    
	#endif
#endif

#endif	//	!defined(OX_CLASS_DECL) || !defined(OX_API_DECL) && !defined(OX_DATA_DECL)
