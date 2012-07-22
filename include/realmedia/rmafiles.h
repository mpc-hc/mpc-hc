/****************************************************************************
 * 
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved..
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary 
 *  information of Progressive Networks, Inc, and is licensed
 *  subject to restrictions on use and distribution.
 *
 *
 *  RealMedia Architecture File Format and File System Plug-in Interfaces.
 *
 */

#ifndef _RMAFILES_H_
#define _RMAFILES_H_

/*
 * Forward declarations of some interfaces defined here-in.
 */
typedef _INTERFACE	IRMAFileObject			IRMAFileObject;
typedef _INTERFACE	IRMAFileResponse		IRMAFileResponse;
typedef _INTERFACE	IRMAFileSystemObject		IRMAFileSystemObject;
typedef _INTERFACE	IRMAFileStat			IRMAFileStat;
typedef _INTERFACE	IRMAFileStatResponse		IRMAFileStatResponse;

typedef _INTERFACE	IRMAFileSystemManager		IRMAFileSystemManager;
typedef _INTERFACE	IRMAFileSystemManagerResponse	IRMAFileSystemManagerResponse;
typedef _INTERFACE	IRMAFileExists			IRMAFileExists;
typedef _INTERFACE	IRMAFileExistsResponse		IRMAFileExistsResponse;
typedef _INTERFACE	IRMAFileMimeMapper		IRMAFileMimeMapper;
typedef _INTERFACE	IRMAFileMimeMapperResponse	IRMAFileMimeMapperResponse;
typedef _INTERFACE	IRMABroadcastMapper		IRMABroadcastMapper;
typedef _INTERFACE	IRMABroadcastMapperResponse	IRMABroadcastMapperResponse;
typedef _INTERFACE	IRMAGetFileFromSamePoolResponse IRMAGetFileFromSamePoolResponse;
typedef _INTERFACE	IRMABuffer			IRMABuffer;
typedef _INTERFACE	IRMAPacket			IRMAPacket;
typedef _INTERFACE	IRMAValues			IRMAValues;
typedef _INTERFACE	IRMAMetaCreation		IRMAMetaCreation;

typedef _INTERFACE	IRMAAuthenticator               IRMAAuthenticator;
typedef _INTERFACE	IRMARequest                     IRMARequest;
typedef _INTERFACE	IRMAFileRename                  IRMAFileRename;
typedef _INTERFACE	IRMADirHandler			IRMADirHandler;
typedef _INTERFACE	IRMADirHandlerResponse		IRMADirHandlerResponse;
typedef _INTERFACE	IRMAFileRemove                  IRMAFileRemove;



/****************************************************************************
 *  Defines:
 *	PN_FILE_XXXX
 *  Purpose:
 *	Flags for opening file objects
 */
#define PN_FILE_READ		1
#define PN_FILE_WRITE		2
#define PN_FILE_BINARY		4
#define PN_FILE_NOTRUNC		8


/****************************************************************************
 *  Defines:
 *	RMA_FILEADVISE_XXXX
 *  Purpose:
 *	Flags for file object Advise method
 */
#define RMA_FILEADVISE_RANDOMACCESS		1


#if defined(_UNIX) || defined(_WINDOWS)
#include <sys/stat.h>
/*
 * This is a subset of standard stat()/fstat() values that both Unix and
 * Windows support (or at least define).
 *
 * These flags are returned from IRMAFileStatResponse::StatDone() in the
 * ulMode argument.
 */
#define PN_S_IFMT   S_IFMT
#define PN_S_IFDIR  S_IFDIR
#define PN_S_IFCHR  S_IFCHR
#define PN_S_IFIFO  S_IFIFO
#define PN_S_IFREG  S_IFREG
#else
/* Macintosh */
#define PN_S_IFMT   0170000
#define PN_S_IFDIR  0040000
#define PN_S_IFCHR  0020000
#define PN_S_IFIFO  0010000
#define PN_S_IFREG  0100000
#endif


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileObject
 * 
 *  Purpose:
 * 
 *	Object that exports file control API
 * 
 *  IID_IRMAFileObject:
 * 
 *	{00000200-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileObject, 0x00000200, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileObject

DECLARE_INTERFACE_(IRMAFileObject, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMAFileObject methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAFileObject::Init
     *	Purpose:
     *	    Associates a file object with the file response object it should
     *	    notify of operation completness. This method should also check
     *	    for validity of the object (for example by opening it if it is
     *	    a local file).
     */
    STDMETHOD(Init)	(THIS_
			ULONG32		    /*IN*/  ulFlags,
			IRMAFileResponse*   /*IN*/  pFileResponse) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAFileObject::GetFilename
     *	Purpose:
     *	    Returns the filename (without any path information) associated
     *	    with a file object.
     *
     *	    Note: The returned pointer's lifetime expires as soon as the
     *	    caller returns from a function which was called from the RMA
     *	    core (i.e. when you return control to the RMA core)
     *
     */
    STDMETHOD(GetFilename)	(THIS_
				REF(const char*)    /*OUT*/  pFilename) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAFileObject::Close
     *	Purpose:
     *	    Closes the file resource and releases all resources associated
     *	    with the object.
     */
    STDMETHOD(Close)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAFileObject::Read
     *	Purpose:
     *	    Reads a buffer of data of the specified length from the file
     *	    and asynchronously returns it to the caller via the 
     *	    IRMAFileResponse interface passed in to Init.
     */
    STDMETHOD(Read)	(THIS_
			ULONG32 ulCount) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAFileObject::Write
     *	Purpose:
     *	    Writes a buffer of data to the file and asynchronously notifies
     *	    the caller via the IRMAFileResponse interface passed in to Init,
     *	    of the completeness of the operation.
     */
    STDMETHOD(Write)	(THIS_
			IRMABuffer* pBuffer) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAFileObject::Seek
     *	Purpose:
     *	    Seeks to an offset in the file and asynchronously notifies
     *	    the caller via the IRMAFileResponse interface passed in to Init,
     *	    of the completeness of the operation.
     *	    If the bRelative flag is TRUE, it is a relative seek; else
     *	    an absolute seek.
     */
    STDMETHOD(Seek)	(THIS_
			ULONG32 ulOffset,
			BOOL	bRelative) PURE;    

    /************************************************************************
     *	Method:
     *	    IRMAFileObject::Advise
     *	Purpose:
     *      To pass information to the File Object advising it about usage
     *	    heuristics.
     */
    STDMETHOD(Advise)	(THIS_
			ULONG32 ulInfo) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileResponse
 * 
 *  Purpose:
 * 
 *	Object that exports file response API
 * 
 *  IID_IRMAFileResponse:
 * 
 *	{00000201-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileResponse, 0x00000201, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileResponse

DECLARE_INTERFACE_(IRMAFileResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAFileResponse methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAFileResponse::InitDone
     *	Purpose:
     *	    Notification interface provided by users of the IRMAFileObject
     *	    interface. This method is called by the IRMAFileObject when the
     *	    initialization of the file is complete. If the file is not valid 
     *	    for the file system, the status PNR_FAILED should be 
     *	    returned.
     */
    STDMETHOD(InitDone)			(THIS_
					PN_RESULT	    status) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAFileResponse::CloseDone
     *	Purpose:
     *	    Notification interface provided by users of the IRMAFileObject
     *	    interface. This method is called by the IRMAFileObject when the
     *	    close of the file is complete.
     */
    STDMETHOD(CloseDone)		(THIS_
					PN_RESULT	    status) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAFileResponse::ReadDone
     *	Purpose:
     *	    Notification interface provided by users of the IRMAFileObject
     *	    interface. This method is called by the IRMAFileObject when the
     *	    last read from the file is complete and a buffer is available.
     */
    STDMETHOD(ReadDone)			(THIS_ 
					PN_RESULT	    status,
					IRMABuffer*	    pBuffer) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAFileResponse::WriteDone
     *	Purpose:
     *	    Notification interface provided by users of the IRMAFileObject
     *	    interface. This method is called by the IRMAFileObject when the
     *	    last write to the file is complete.
     */
    STDMETHOD(WriteDone)		(THIS_ 
					PN_RESULT	    status) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAFileResponse::SeekDone
     *	Purpose:
     *	    Notification interface provided by users of the IRMAFileObject
     *	    interface. This method is called by the IRMAFileObject when the
     *	    last seek in the file is complete.
     */
    STDMETHOD(SeekDone)			(THIS_ 
					PN_RESULT	    status) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileSystemObject
 * 
 *  Purpose:
 * 
 *	Object that allows a Controller to communicate with a specific
 *	File System plug-in session
 * 
 *  IID_IRMAFileSystemObject:
 * 
 *	{00000202-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileSystemObject, 0x00000202, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileSystemObject

DECLARE_INTERFACE_(IRMAFileSystemObject, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMAFileSystemObject methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAFileSystemObject::GetFileSystemInfo
     *	Purpose:
     *	    Returns information vital to the instantiation of file system
     *	    plugin.
     *
     *	    pShortName should be a short, human readable name in the form
     *      of "company-fsname".  For example: pShortName = "pn-local".
     */
    STDMETHOD(GetFileSystemInfo)    (THIS_
				    REF(const char*) /*OUT*/ pShortName,
				    REF(const char*) /*OUT*/ pProtocol) PURE;

    STDMETHOD(InitFileSystem)	(THIS_
				IRMAValues* pOptions) PURE;

    STDMETHOD(CreateFile)	(THIS_
				IUnknown**    /*OUT*/	ppFileObject) PURE;

    /*
     * The following method is deprecated and should return PNR_NOTIMPL
     */

    STDMETHOD(CreateDir)	(THIS_
				IUnknown**   /*OUT*/ ppDirObject) PURE;

};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileStat
 * 
 *  Purpose:
 * 
 *      Gets information about a specific File object
 * 
 *  IID_IRMAFileStat:
 *  
 *	{00000205-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileStat, 0x00000205, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileStat

DECLARE_INTERFACE_(IRMAFileStat, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
                                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /*
     *	IRMAFileStat methods
     */

    STDMETHOD(Stat)		(THIS_
				IRMAFileStatResponse* pFileStatResponse
				) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileStatResponse
 * 
 *  Purpose:
 * 
 *      Returns information about a specific File object
 * 
 *  IID_IRMAFileStatResponse:
 *  
 *	{00000206-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileStatResponse, 0x00000206, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileStatResponse

DECLARE_INTERFACE_(IRMAFileStatResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
                                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /*
     *	IRMAFileStat methods
     */

    STDMETHOD(StatDone)		(THIS_
				 PN_RESULT status,
				 UINT32 ulSize,
				 UINT32 ulCreationTime,
				 UINT32 ulAccessTime,
				 UINT32 ulModificationTime,
				 UINT32 ulMode) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileSystemManager
 * 
 *  Purpose:
 * 
 *      Gives out File Objects based on URLs
 * 
 *  IID_IRMAFileSystemManager:
 *  
 *	{00000207-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileSystemManager, 0x00000207, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileSystemManager

#define CLSID_IRMAFileSystemManager IID_IRMAFileSystemManager

DECLARE_INTERFACE_(IRMAFileSystemManager, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
                                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /*
     *	IRMAFileSystemManager methods
     */

    STDMETHOD(Init) (THIS_
		    IRMAFileSystemManagerResponse* /*IN*/  pFileManagerResponse
		    ) PURE;

    /* GetFileObject attempts to locate an existing file via the DoesExist
     * method in each file system's objects, and returns that object through
     * FSManagerResponse->FileObjectReady
     */
    STDMETHOD(GetFileObject)	(THIS_
				 IRMARequest* pRequest,
				 IRMAAuthenticator* pAuthenticator) PURE;

    /* GetNewFileObject is similar to GetFileObject except that no DoesExist
     * checks are done.  The first file system that matches the mount point
     * or protocol for the path in the request object creates the file
     * which is then returned through FileObjectReady.  This is especially
     * useful for those who wish to open a brand new file for writing.
     */
    STDMETHOD(GetNewFileObject) (THIS_
				 IRMARequest* pRequest,
				 IRMAAuthenticator* pAuthenticator) PURE;

    STDMETHOD(GetRelativeFileObject) (THIS_
				      IUnknown* pOriginalObject,
				      const char* pPath) PURE;

    /*
     * The following method is deprecated and should return PNR_NOTIMPL
     */

    STDMETHOD(GetDirObjectFromURL)	(THIS_
                                        const char* pURL) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileSystemManagerResponse
 * 
 *  Purpose:
 * 
 *      Gives out File System objects based on URLs
 * 
 *  IID_IRMAFileSystemManagerResponse:
 *  
 *	{00000208-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileSystemManagerResponse, 0x00000208, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileSystemManagerResponse

DECLARE_INTERFACE_(IRMAFileSystemManagerResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
                                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /*
     *	IRMAFileSystemManagerResponse methods
     */

    /************************************************************************
     *	Method:
     *	IRMAFileSystemManagerResponse::InitDone
     *	Purpose:
     */
    STDMETHOD(InitDone)	    (THIS_
			    PN_RESULT	    status) PURE;

    STDMETHOD(FileObjectReady)	(THIS_
				PN_RESULT status,
                                IUnknown* pObject) PURE;

    /*
     * The following method is deprecated and should return PNR_NOTIMPL
     */

    STDMETHOD(DirObjectReady)	(THIS_
				PN_RESULT status,
                                IUnknown* pDirObject) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileExists
 * 
 *  Purpose:
 * 
 *	Checks for the existense of a file.  Must be implemented.
 * 
 *  IID_IRMAFileExists:
 * 
 *	{00000209-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileExists, 0x00000209, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileExists

DECLARE_INTERFACE_(IRMAFileExists, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMAFileExists methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAFileExists::DoesExist
     *	Purpose:
     */
    STDMETHOD(DoesExist) (THIS_
			const char*		/*IN*/  pPath, 
			IRMAFileExistsResponse* /*IN*/  pFileResponse) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileExistsResponse
 * 
 *  Purpose:
 * 
 *	Response interface for IRMAFileExists.  Must be implemented.
 * 
 *  IID_IRMAFileExistsResponse:
 * 
 *	{0000020A-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileExistsResponse, 0x0000020a, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileExists

DECLARE_INTERFACE_(IRMAFileExistsResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMAFileExistsResponse methods
     */

    STDMETHOD(DoesExistDone) (THIS_
			      BOOL	bExist) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileMimeMapper
 * 
 *  Purpose:
 * 
 *	Allows you to specify a mime type for a specific file.
 *	Optional interface.
 * 
 *  IID_IRMAFileMimeMapper:
 * 
 *	{0000020B-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileMimeMapper, 0x0000020b, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileMimeMapper

DECLARE_INTERFACE_(IRMAFileMimeMapper, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMAFileMimeMapper methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAFileMimeMapper::FindMimeType
     *	Purpose:
     */
    STDMETHOD(FindMimeType) (THIS_
			    const char*		    /*IN*/  pURL, 
			    IRMAFileMimeMapperResponse* /*IN*/  pMimeMapperResponse
			    ) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileMimeMapperResponse
 * 
 *  Purpose:
 * 
 *	Response interface for IRMAFileMimeMapper.
 *	Optional interface.
 * 
 *  IID_IRMAFileMimeMapperResponse:
 * 
 *	{0000020C-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileMimeMapperResponse, 0x0000020c, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileMimeMapperResponse

DECLARE_INTERFACE_(IRMAFileMimeMapperResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMAFileMimeMapperResponse methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAFileMimeMapperResponse::MimeTypeFound
     *	Purpose:
     *	    Notification interface provided by users of the IRMAFileMimeMapper
     *	    interface. This method is called by the IRMAFileObject when the
     *	    initialization of the file is complete, and the Mime type is
     *	    available for the request file. If the file is not valid for the
     *	    file system, the status PNR_FAILED should be returned,
     *	    with a mime type of NULL. If the file is valid but the mime type
     *	    is unknown, then the status PNR_OK should be returned with
     *	    a mime type of NULL.
     *	    
     */
    STDMETHOD(MimeTypeFound) (THIS_
			      PN_RESULT	status,
			      const char* pMimeType) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMABroadcastMapper
 * 
 *  Purpose:
 * 
 *	Associates a file with a broadcast format plugin.
 *	Implementation only required by broadcast plugin file systems.
 * 
 *  IID_IRMABroadcastMapper:
 * 
 *	{0000020D-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMABroadcastMapper, 0x0000020d, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMABroadcastMapper

DECLARE_INTERFACE_(IRMABroadcastMapper, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMABroadcastMapper methods
     */

    /************************************************************************
     *	Method:
     *	    IRMABroadcastMapper::FindBroadcastType
     *	Purpose:
     */
    STDMETHOD(FindBroadcastType) (THIS_
				const char*		     /*IN*/  pURL, 
				IRMABroadcastMapperResponse* /*IN*/  pBroadcastMapperResponse) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMABroadcastMapperResponse
 * 
 *  Purpose:
 * 
 *	Response interface for IRMABroadcastMapper.
 *	Implementation only required by broadcast plugin file systems.
 * 
 *  IID_IRMABroadcastMapperResponse:
 * 
 *	{0000020E-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMABroadcastMapperResponse, 0x0000020e, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMABroadcastMapperResponse

DECLARE_INTERFACE_(IRMABroadcastMapperResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMABroadcastMapperResponse methods
     */

    /************************************************************************
     *	Method:
     *	    IRMABroadcastMapperResponse::BroadcastTypeFound
     *	Purpose:
     *	    Notification interface provided by users of the IRMABroadcastMapper
     *	    interface. This method is called by the File Object when the
     *	    initialization of the file is complete, and the broadcast type is
     *	    available for the request file. If the file is not valid for the
     *	    file system, the status PNR_FAILED should be returned,
     *	    with the broadcast type set to NULL.
     *	    
     */
    STDMETHOD(BroadcastTypeFound) (THIS_
				  PN_RESULT	status,
				  const char* pBroadcastType) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAGetFileFromSamePool
 * 
 *  Purpose:
 * 
 *      Gives out File Objects based on filenames and relative "paths"
 * 
 *  IID_IRMAGetFileFromSamePool:
 *  
 *	{0000020f-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAGetFileFromSamePool, 0x0000020f, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);
#undef  INTERFACE
#define INTERFACE   IRMAGetFileFromSamePool

#define CLSID_IRMAGetFileFromSamePool IID_IRMAGetFileFromSamePool

DECLARE_INTERFACE_(IRMAGetFileFromSamePool, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
                                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;


    /*
     * IRMAGetFileFromSamePool method
     */
    /************************************************************************
     *	Method:
     *	    IRMAGetFileFromSamePool::GetFileObjectFromPool
     *	Purpose:
     *      To get another FileObject from the same pool. 
     */
    STDMETHOD(GetFileObjectFromPool)	(THIS_
					 IRMAGetFileFromSamePoolResponse*) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAGetFileFromSamePoolResponse
 * 
 *  Purpose:
 * 
 *      Gives out File Objects based on filenames and relative "paths"
 * 
 *  IID_IRMAGetFileFromSamePoolResponse:
 *  
 *	{00000210-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAGetFileFromSamePoolResponse, 0x00000210, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);
#undef  INTERFACE
#define INTERFACE   IRMAGetFileFromSamePoolResponse

#define CLSID_IRMAGetFileFromSamePoolResponse IID_IRMAGetFileFromSamePoolResponse

DECLARE_INTERFACE_(IRMAGetFileFromSamePoolResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
                                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /*
     * IRMAGetFileFromSamePoolResponse method
     */
    /************************************************************************
     *	Method:
     *	    IRMAGetFileFromSamePoolResponse::FileObjectReady
     *	Purpose:
     *      To return another FileObject from the same pool. 
     */
    STDMETHOD(FileObjectReady) (THIS_
				PN_RESULT status,
				IUnknown* ppUnknown) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileAuthenticator
 * 
 *  Purpose:
 * 
 *      Set and Get a file object's authenticator object.
 * 
 *  IID_IRMAFileAuthenticator:
 *  
 *	{00000211-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileAuthenticator, 0x00000211, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);
#undef  INTERFACE
#define INTERFACE   IRMAFileAuthenticator

#define CLSID_IRMAFileAuthenticator IID_IRMAFileAuthenticator

DECLARE_INTERFACE_(IRMAFileAuthenticator, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
                                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /*
     * IRMAFileAuthenticator methods
     */
    STDMETHOD(SetAuthenticator) (THIS_
				 IRMAAuthenticator* pAuthenticator) PURE;
    
    STDMETHOD(GetAuthenticator) (THIS_
				 REF(IRMAAuthenticator*) pAuthenticator) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMARequestHandler
 * 
 *  Purpose:
 * 
 *      Object to manage IRMARequest objects
 * 
 *  IID_IRMARequestHandler:
 *  
 *	{00000212-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMARequestHandler, 0x00000212, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);
#undef  INTERFACE
#define INTERFACE   IRMARequestHandler

#define CLSID_IRMARequestHandler IID_IRMARequestHandler

DECLARE_INTERFACE_(IRMARequestHandler, IUnknown)
{
    /*
     *	IUnknown methods
     */

    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
                                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARequestHandler::SetRequest
     *	Purpose:
     *	    Associates an IRMARequest with an object
     */
    STDMETHOD(SetRequest)   (THIS_
			    IRMARequest*        /*IN*/  pRequest) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARequestHandler::GetRequest
     *	Purpose:
     *	    Gets the IRMARequest object associated with an object
     */
    STDMETHOD(GetRequest)   (THIS_
			    REF(IRMARequest*)        /*OUT*/  pRequest) PURE;

};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMARequestContext
 * 
 *  Purpose:
 * 
 *      Object to manage the context of the Request
 * 
 *  IID_IRMARequestContext:
 *  
 *	{00000217-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMARequestContext, 0x00000217, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);
#undef  INTERFACE
#define INTERFACE   IRMARequestContext

#define CLSID_IRMARequestContext IID_IRMARequestContext

DECLARE_INTERFACE_(IRMARequestContext, IUnknown)
{
    /*
     *	IUnknown methods
     */

    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
                                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /*
     * IRMARequestContext methods
     */

    /************************************************************************
     *	Method:
     *	    IRMARequestContext::SetUserContext
     *	Purpose:
     *	    Sets the Authenticated users Context.
     */
    STDMETHOD(SetUserContext)
    (
	THIS_
	IUnknown* pIUnknownNewContext
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARequestContext::GetUserContext
     *	Purpose:
     *	    Gets the Authenticated users Context.
     */
    STDMETHOD(GetUserContext)
    (
	THIS_
	REF(IUnknown*) pIUnknownCurrentContext
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARequestContext::SetRequester
     *	Purpose:
     *	    Sets the Object that made the request.
     */
    STDMETHOD(SetRequester)
    (
	THIS_
	IUnknown* pIUnknownNewRequester
    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARequestContext::GetRequester
     *	Purpose:
     *	    Gets the Object that made the request.
     */
    STDMETHOD(GetRequester)
    (
	THIS_
	REF(IUnknown*) pIUnknownCurrentRequester
    ) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMARequest
 * 
 *  Purpose:
 * 
 *      Object to manage the RFC822 headers sent by the client
 * 
 *  IID_IRMARequest:
 *  
 *	{00000213-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMARequest, 0x00000213, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);
#undef  INTERFACE
#define INTERFACE   IRMARequest

#define CLSID_IRMARequest IID_IRMARequest

DECLARE_INTERFACE_(IRMARequest, IUnknown)
{
    /*
     *	IUnknown methods
     */

    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
                                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /*
     * IRMARequest methods
     */

    /************************************************************************
     *	Method:
     *	    IRMARequest::SetRequestHeaders
     *	Purpose:
     *	    Sets the headers that will be sent in the RFC822 header section
     *      of the request message
     */
    STDMETHOD(SetRequestHeaders)	(THIS_
					IRMAValues* pRequestHeaders) PURE;
    
    /************************************************************************
     *	Method:
     *	    IRMARequest::GetRequestHeaders
     *	Purpose:
     *	    Gets the headers that were sent in the RFC822 header section
     *	    of the request message
     */
    STDMETHOD(GetRequestHeaders)	(THIS_
					REF(IRMAValues*) pRequestHeaders) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARequest::SetResponseHeaders
     *	Purpose:
     *	    Sets the headers that will be returned in the RFC822 header
     *	    section of the response message
     */
    STDMETHOD(SetResponseHeaders)	(THIS_
					IRMAValues* pResponseHeaders) PURE;
    
    /************************************************************************
     *	Method:
     *	    IRMARequest::GetResponseHeaders
     *	Purpose:
     *	    Gets the headers that were returned in the RFC822 header section
     *	    of the response message
     */
    STDMETHOD(GetResponseHeaders)	(THIS_
					REF(IRMAValues*) pResponseHeaders) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARequest::SetURL
     *	Purpose:
     *	    Sets the fully qualified path associated with a file object.
     *	    Note: On the server, this path does not include the file system
     *	    	mount point.
     */
    STDMETHOD(SetURL)			(THIS_
					const char* pURL) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARequest::GetURL
     *	Purpose:
     *	    Returns the fully qualified path associated with a file object.
     *	    Note: On the server, this path does not include the file system
     *	    	mount point.
     *
     *	    Note: The returned pointer's lifetime expires as soon as the
     *	    caller returns from a function which was called from the RMA
     *	    core (i.e. when you return control to the RMA core)
     */
    STDMETHOD(GetURL)			(THIS_
					REF(const char*) pURL) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileRename
 * 
 *  Purpose:
 * 
 *      Interface to allow renaming of files.  Query off of the File Object.
 *	Not all filesystem plugins implement this feature.
 * 
 *  IID_IRMAFileRename:
 *  
 *	{00000214-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileRename, 0x00000214, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);
#undef  INTERFACE
#define INTERFACE   IRMAFileRename

DECLARE_INTERFACE_(IRMAFileRename, IUnknown)
{
    /*
     *	IUnknown methods
     */

    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
                                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /*
     * IRMAFileRename methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAFileRename::Rename
     *	Purpose:
     *	    Renames a file to a new name.
     */
    STDMETHOD(Rename)			(THIS_
					const char* pNewFileName) PURE;
};
    
/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMADirHandler
 * 
 *  Purpose:
 * 
 *	Object that exports directory handler API
 * 
 *  IID_IRMADirHandler:
 * 
 *	{00000215-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMADirHandler, 0x00000215, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMADirHandler

DECLARE_INTERFACE_(IRMADirHandler, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMADirHandler methods
     */

    /************************************************************************
     *	Method:
     *	    IRMADirHandler::InitDirHandler
     *	Purpose:
     *	    Associates a directory handler with the directory handler
     *	    response, it should notify of operation completness.
     */
    STDMETHOD(InitDirHandler)	(THIS_
				IRMADirHandlerResponse*    /*IN*/  pDirResponse) PURE;

    /************************************************************************
     *	Method:
     *	    IRMADirHandler::CloseDirHandler
     *	Purpose:
     *	    Closes the directory handler resource and releases all resources
     *	    associated with the object.
     */
    STDMETHOD(CloseDirHandler)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMADirHandler::MakeDir
     *	Purpose:
     *	    Create the directory
     */
    STDMETHOD(MakeDir)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMADirHandler::ReadDir
     *	Purpose:
     *	    Get a dump of the directory
     */
    STDMETHOD(ReadDir)	(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMADirHandlerResponse
 * 
 *  Purpose:
 * 
 *	Object that exports the directory handler response API
 * 
 *  IID_IRMADirHandlerResponse:
 * 
 *	{00000216-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMADirHandlerResponse, 0x00000216, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMADirHandlerResponse

DECLARE_INTERFACE_(IRMADirHandlerResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMADirHandlerResponse methods
     */

    /************************************************************************
     *	Method:
     *	    IRMADirHandlerResponse::InitDirHandlerDone
     *	Purpose:
     *	    Notification interface provided by users of the IRMADirHandler
     *	    interface. This method is called by the IRMADirHandler when the
     *	    initialization of the object is complete.
     */
    STDMETHOD(InitDirHandlerDone)	(THIS_
					PN_RESULT	    status) PURE;

    /************************************************************************
     *	Method:
     *	    IRMADirHandlerResponse::CloseDirHandlerDone
     *	Purpose:
     *	    Notification interface provided by users of the IRMADirHandler
     *	    interface. This method is called by the IRMADirHandler when the
     *	    close of the directory is complete.
     */
    STDMETHOD(CloseDirHandlerDone)	(THIS_
					PN_RESULT	    status) PURE;

    /************************************************************************
     *	Method:
     *	    IRMADirHandler::MakeDirDone
     *	Purpose:
     *	    Notification interface provided by users of the IRMADirHandler
     *	    interface. This method is called by the IRMADirHandler when the
     *	    attempt to create the directory is complete.
     */
    STDMETHOD(MakeDirDone)		(THIS_ 
					PN_RESULT	    status) PURE;

    /************************************************************************
     *	Method:
     *	    IRMADirHandler::ReadDirDone
     *	Purpose:
     *	    Notification interface provided by users of the IRMADirHandler
     *	    interface. This method is called by the IRMADirHandler when the
     *	    read from the directory is complete and a buffer is available.
     */
    STDMETHOD(ReadDirDone)		(THIS_ 
					PN_RESULT	    status,
					IRMABuffer*	    pBuffer) PURE;
};



/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileRemove
 * 
 *  Purpose:
 * 
 *      Interface to allow removing of files.  Query off of the File Object.
 *	Not all filesystem plugins implement this feature.
 * 
 *  IID_IRMAFileRemove:
 *  
 *	{0000021A-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileRemove, 0x0000021A, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);
#undef  INTERFACE
#define INTERFACE   IRMAFileRemove

DECLARE_INTERFACE_(IRMAFileRemove, IUnknown)
{
    /*
     *	IUnknown methods
     */

    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
                                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /*
     * IRMAFileRemove methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAFileRemove::Remove
     *	Purpose:
     *	    Removes a file from the file system.
     */
    STDMETHOD(Remove)			(THIS) PURE;
};





#endif  /* _RMAFILES_H_ */
