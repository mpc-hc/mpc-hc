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
 *  RealMedia Architecture Callback, Networking, and Scheduling interfaces.
 *
 */

#ifndef _RMAENGIN_H_
#define _RMAENGIN_H_

/*
 * Forward declarations of some interfaces used here-in.
 */

typedef _INTERFACE	IRMABuffer			IRMABuffer;
typedef _INTERFACE	IRMACallback			IRMACallback;
typedef _INTERFACE	IRMAScheduler			IRMAScheduler;
typedef _INTERFACE	IRMATCPResponse			IRMATCPResponse;
typedef _INTERFACE	IRMALBoundTCPSocket		IRMALBoundTCPSocket;
typedef _INTERFACE	IRMATCPSocket			IRMATCPSocket;
typedef _INTERFACE	IRMAListenResponse		IRMAListenResponse;
typedef _INTERFACE	IRMAListenSocket		IRMAListenSocket;
typedef _INTERFACE	IRMANetworkServices		IRMANetworkServices;
typedef _INTERFACE	IRMANetworkServices2		IRMANetworkServices2;
typedef _INTERFACE	IRMAUDPResponse		    	IRMAUDPResponse;
typedef _INTERFACE	IRMAUDPSocket			IRMAUDPSocket;
typedef _INTERFACE	IRMAResolver			IRMAResolver;
typedef _INTERFACE	IRMAResolverResponse		IRMAResolverResponse;
typedef _INTERFACE	IRMAInterruptSafe		IRMAInterruptSafe;
typedef _INTERFACE	IRMAAsyncIOSelection		IRMAAsyncIOSelection;
typedef _INTERFACE	IRMAUDPMulticastInit		IRMAUDPMulticastInit;
typedef _INTERFACE	IRMAInterruptState		IRMAInterruptState;
typedef _INTERFACE	IRMAOptimizedScheduler		IRMAOptimizedScheduler;


/*
 * Address flags starting with PNR are depricated.
 */
#define PNR_INADDR_ANY	(UINT32)0x00000000  //THIS FLAG IS DEPRICATED
#define PN_INADDR_ANY	(UINT32)0x00000000

/*
 * 255.255.255.254
 *
 * Bind to all ports in IPBindings list from
 * server config.
 */
#define PNR_INADDR_IPBINDINGS (UINT32)0xfffffffe    //THIS FLAG IS DEPRICATED
#define PN_INADDR_IPBINDINGS (UINT32)0xfffffffe


/* Async IO Selection Type (Unix Only) */

#define PNAIO_READ 1
#define PNAIO_WRITE 2
#define PNAIO_EXCEPTION 4

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMACallback
 * 
 *  Purpose:
 * 
 *	This interface defines a simple callback which will be used in
 *	various interfaces such as IRMAScheduler.
 * 
 *  IID_IRMACallback:
 * 
 *	{00000100-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMACallback, 0x00000100, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMACallback

DECLARE_INTERFACE_(IRMACallback, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *  IRMACallback methods
     */

    /************************************************************************
     *	Method:
     *	    IRMACallback::Func
     *	Purpose:
     *	    This is the function that will be called when a callback is
     *	    to be executed.
     */
    STDMETHOD(Func)		(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAScheduler
 * 
 *  Purpose:
 * 
 *	This interface provides the user with a way of scheduling callbacks
 *	that will be executed at some time in the future.
 * 
 *  IID_IRMAScheduler:
 * 
 *	{00000101-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAScheduler, 0x00000101, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAScheduler

typedef ULONG32 CallbackHandle;

typedef struct _RMATimeval
{
    UINT32 tv_sec;
    UINT32 tv_usec;
} RMATimeval;

DECLARE_INTERFACE_(IRMAScheduler, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAScheduler methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAScheduler::RelativeEnter
     *	Purpose:
     *	    Schedule a callback to be executed "ms" milliseconds from now
     *	    This function is less percise then AbsoluteEnter and should only
     *	    be used when accurate timing is not critical.
     */
    STDMETHOD_(CallbackHandle,RelativeEnter)	(THIS_
						IRMACallback* pCallback,
						UINT32 ms) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAScheduler::AbsoluteEnter
     *	Purpose:
     *	    Schedule a callback to be executed at time "tVal".
     */
    STDMETHOD_(CallbackHandle,AbsoluteEnter)	(THIS_
						IRMACallback* pCallback,
						RMATimeval tVal) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAScheduler::Remove
     *	Purpose:
     *	    Remove a callback from the scheduler.
     */
    STDMETHOD(Remove)		(THIS_
			    	CallbackHandle Handle) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAScheduler::GetCurrentSchedulerTime
     *	Purpose:
     *	    Gives the current time (in the timeline of the scheduler).
     */
    STDMETHOD_(RMATimeval,GetCurrentSchedulerTime)	(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMATCPResponse
 * 
 *  Purpose:
 * 
 *	This is the response interface for the asynchronous TCP networking 
 *	interface.
 * 
 *  IID_IRMATCPResponse:
 * 
 *	{00000102-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMATCPResponse, 0x00000102, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMATCPResponse

DECLARE_INTERFACE_(IRMATCPResponse, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMATCPResponse methods
     */

    /************************************************************************
     *	Method:
     *	    IRMATCPResponse::ConnectDone
     *	Purpose:
     *	    A Connect operation has been completed or an error has occurred.
     */
    STDMETHOD(ConnectDone)	(THIS_
				PN_RESULT		status) PURE;

    /************************************************************************
     *	Method:
     *	    IRMATCPResponse::ReadDone
     *	Purpose:
     *	    A Read operation has been completed or an error has occurred.
     *	    The data is returned in the IRMABuffer.
     */
    STDMETHOD(ReadDone)		(THIS_
				PN_RESULT		status,
				IRMABuffer*		pBuffer) PURE;

    /************************************************************************
     *	Method:
     *	    IRMATCPResponse::WriteReady
     *	Purpose:
     *	    This is the response method for WantWrite.
     *	    If PN_RESULT is ok, then the TCP channel is ok to Write to.
     */
    STDMETHOD(WriteReady)	(THIS_
    				PN_RESULT		status) PURE;

    /************************************************************************
     *	Method:
     *	    IRMATCPResponse::Closed
     *	Purpose:
     *	    This method is called to inform you that the TCP channel has
     *	    been closed by the peer or closed due to error.
     */
    STDMETHOD(Closed)		(THIS_
				PN_RESULT		status) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMATCPSocket
 * 
 *  Purpose:
 * 
 *	Provides the user with an asynchronous TCP networking interface.
 * 
 *  IID_IRMATCPSocket:
 * 
 *	{00000103-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMATCPSocket, 0x00000103, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMATCPSocket

DECLARE_INTERFACE_(IRMATCPSocket, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMATCPSocket methods
     *
     *  Network addresses and ports are in native byte order
     *  
     */

    STDMETHOD(Init)		(THIS_
				IRMATCPResponse*    /*IN*/  pTCPResponse) PURE;

    STDMETHOD(SetResponse)	(THIS_
    				IRMATCPResponse*	    pTCPResponse) PURE;

    STDMETHOD(Bind)		(THIS_
				UINT32			    ulLocalAddr,
				UINT16 			    nPort) PURE;

    /*
     * pDestination is a string containing host name or dotted-ip notation
     */
    STDMETHOD(Connect)		(THIS_
				const char*		    pDestination,
				UINT16 			    nPort) PURE;

    STDMETHOD(Read)		(THIS_
				UINT16			    Size) PURE;

    STDMETHOD(Write)		(THIS_
				IRMABuffer*		    pBuffer) PURE;

    /************************************************************************
     *	Method:
     *	    IRMATCPSocket::WantWrite
     *	Purpose:
     *	    This method is called when you wish to write a large amount of
     *	    data.  If you are only writing small amounts of data, you can
     *	    just call Write (all data not ready to be transmitted will be
     *	    buffered on your behalf).  When the TCP channel is ready to be
     *	    written to, the response interfaces WriteReady method will be 
     *	    called.
     */
    STDMETHOD(WantWrite)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMATCPSocket::GetForeignAddress
     *	Purpose:
     *	    Returns the address of the other end of the TCP socket as a
     *	    ULONG32 in local host order
     */
    STDMETHOD(GetForeignAddress)	(THIS_
    					REF(ULONG32) lAddress) PURE;

    STDMETHOD(GetLocalAddress)		(THIS_
    					REF(ULONG32) lAddress) PURE;

    /************************************************************************
     *	Method:
     *	    IRMATCPSocket::GetForeignPort
     *	Purpose:
     *	    Returns the port of the other end of the TCP socket in local
     *      host order.
     */
    STDMETHOD(GetForeignPort)		(THIS_
    					REF(UINT16) port) PURE;

    STDMETHOD(GetLocalPort)		(THIS_
    					REF(UINT16) port) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAListenResponse
 * 
 *  Purpose:
 * 
 *	This is the response interface for the asynchronous TCP listening
 *	socket interface.
 * 
 *  IID_IRMAListenResponse:
 * 
 *	{00000104-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAListenResponse, 0x00000104, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAListenResponse

DECLARE_INTERFACE_(IRMAListenResponse, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAListenResponse methods
     */

    STDMETHOD(NewConnection)	(THIS_
				PN_RESULT		status,
				IRMATCPSocket*		pTCPSocket) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAListenSocket
 * 
 *  Purpose:
 * 
 *	This interfaces allows you to asynchronously listen on a port for
 *	TCP connections.
 * 
 *  IID_IRMAListenSocket:
 * 
 *	{00000105-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAListenSocket, 0x00000105, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAListenSocket

DECLARE_INTERFACE_(IRMAListenSocket, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAListenSocket methods
     */

    STDMETHOD(Init)		(THIS_
				UINT32				ulLocalAddr,
				UINT16				port,
				IRMAListenResponse*    /*IN*/	pListenResponse
				) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMANetworkServices
 * 
 *  Purpose:
 * 
 *	This is a factory interface for the various types of networking
 *	interfaces described above.
 * 
 *  IID_IRMANetworkServices:
 * 
 *	{00000106-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMANetworkServices, 0x00000106, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMANetworkServices

DECLARE_INTERFACE_(IRMANetworkServices, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMANetworkServices methods
     */

    /************************************************************************
     *	Method:
     *	    IRMANetworkServices::CreateTCPSocket
     *	Purpose:
     *	    Create a new TCP socket.
     */
    STDMETHOD(CreateTCPSocket)	(THIS_
				IRMATCPSocket**    /*OUT*/  ppTCPSocket) PURE;

    /************************************************************************
     *	Method:
     *	    IRMANetworkServices::CreateUDPSocket
     *	Purpose:
     *	    Create a new UDP socket.
     */
    STDMETHOD(CreateUDPSocket)	(THIS_
				IRMAUDPSocket**    /*OUT*/  ppUDPSocket) PURE;

    /************************************************************************
     *	Method:
     *	    IRMANetworkServices::CreateListenSocket
     *	Purpose:
     *	    Create a new TCP socket that will listen for connections on a
     *	    particular port.
     */
    STDMETHOD(CreateListenSocket)   (THIS_
				    IRMAListenSocket** /*OUT*/ ppListenSocket
				    ) PURE;

    /************************************************************************
     *	Method:
     *	    IRMANetworkServices::CreateResolver
     *	Purpose:
     *	    Create a new resolver that can lookup host names
     */
    STDMETHOD(CreateResolver)  	(THIS_
			    	IRMAResolver**    /*OUT*/     ppResolver) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMANetworkServices2
 * 
 *  Purpose:
 * 
 *	This is a factory interface for the various types of networking
 *	interfaces described above.
 * 
 *  IID_IRMANetworkServices:
 * 
 *	{17951551-5683-11d3-B6BA-00C0F031C237}
 * 
 */

// {17951551-5683-11d3-B6BA-00C0F031C237}
DEFINE_GUID(IID_IRMANetworkServices2, 0x17951551, 0x5683, 0x11d3, 0xb6, 0xba, 0x0, 0xc0, 0xf0, 0x31, 0xc2, 0x37);

#undef  INTERFACE
#define INTERFACE   IRMANetworkServices2

DECLARE_INTERFACE_(IRMANetworkServices2, IRMANetworkServices)
{
    /************************************************************************
     *	Method:
     *	    IRMANetworkServices2::CreateLBoundTCPSocket
     *	Purpose:
     *	    Create a new local bound TCP socket.
     */
    STDMETHOD(CreateLBoundTCPSocket)	(THIS_
				IRMATCPSocket**    /*OUT*/  ppTCPSocket) PURE;
};



/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAUDPResponse
 * 
 *  Purpose:
 * 
 *	This is the response interface for the asynchronous UDP networking 
 *	interface.
 * 
 *  IID_IRMAUDPResponse:
 * 
 *	{00000107-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAUDPResponse, 0x00000107, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAUDPResponse

DECLARE_INTERFACE_(IRMAUDPResponse, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAUDPResponse methods
     */

    STDMETHOD(ReadDone)		(THIS_
				PN_RESULT		status,
				IRMABuffer*		pBuffer,
				ULONG32			ulAddr,
				UINT16			nPort) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAUDPSocket
 * 
 *  Purpose:
 * 
 *	Provides the user with an asynchronous UDP networking interface.
 * 
 *  IID_IRMAUDPSocket:
 * 
 *	{00000108-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAUDPSocket, 0x00000108, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAUDPSocket

DECLARE_INTERFACE_(IRMAUDPSocket, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAUDPSocket methods
     *
     *  Network addresses and ports are in native byte order
     */

    STDMETHOD(Init)		(THIS_
				ULONG32			ulAddr,
				UINT16			nPort,
				IRMAUDPResponse*	pUDPResponse) PURE;

    STDMETHOD(Bind)		(THIS_
				UINT32			    ulLocalAddr,
				UINT16 			    nPort) PURE;

    STDMETHOD(Read)		(THIS_
				UINT16			Size) PURE;

    STDMETHOD(Write)		(THIS_
				IRMABuffer*		pBuffer) PURE;

    STDMETHOD(WriteTo)		(THIS_
    				ULONG32			ulAddr,
				UINT16			nPort,
				IRMABuffer*		pBuffer) PURE;

    STDMETHOD(GetLocalPort)	(THIS_
    				REF(UINT16)		port) PURE;

    STDMETHOD(JoinMulticastGroup)	(THIS_
    					ULONG32	    ulMulticastAddr,
    					ULONG32	    ulInterfaceAddr) PURE;
    
    STDMETHOD(LeaveMulticastGroup)	(THIS_
    					ULONG32	    ulMulticastAddr,
    					ULONG32	    ulInterfaceAddr) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAResolver
 * 
 *  Purpose:
 * 
 *	This interface allows you to asynchronously resolve hostnames.
 * 
 *  IID_IRMAResolver:
 * 
 *	{00000109-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAResolver, 0x00000109, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAResolver

DECLARE_INTERFACE_(IRMAResolver, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAResolver methods
     */

    STDMETHOD(Init)			(THIS_
					IRMAResolverResponse*  pResponse) PURE;

    STDMETHOD(GetHostByName)		(THIS_
					const char* pHostName) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAResolverResponse
 * 
 *  Purpose:
 * 
 *	This is the response interface for the asynchronous DNS hostname
 *	resolver.
 * 
 *  IID_IRMAResolverResponse:
 * 
 *	{0000010A-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAResolverResponse, 0x0000010A, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAResolverResponse

DECLARE_INTERFACE_(IRMAResolverResponse, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAResolverResponse methods
     */

    STDMETHOD(GetHostByNameDone)	(THIS_
					PN_RESULT status,
					ULONG32 ulAddr) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAInterruptSafe
 * 
 *  Purpose:
 * 
 *	This interface is used in Macintosh implementations of callback 
 *	functions, renderers, etc... to determine if interrupt time execution  
 *	is supported. If this interface is not implemented then it is assumed
 *	that interrupt time execution is NOT supported. There are restrictions 
 *	on what may be executed at interrupt time; please consult the Macintosh
 *	Deferred Task Manager tech notes from Apple.
 * 
 *  IID_IRMAInterruptSafe:
 * 
 *	{0000010B-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAInterruptSafe, 0x0000010B, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAInterruptSafe

DECLARE_INTERFACE_(IRMAInterruptSafe, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *  IRMAInterruptSafe methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAInterruptSafe::IsInterruptSafe
     *	Purpose:
     *	    This is the function that will be called to determine if
     *	    interrupt time execution is supported.
     */
    STDMETHOD_(BOOL,IsInterruptSafe)		(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAAsyncIOSelection
 * 
 *  Purpose:
 * 
 *      This interface is implemented by the server/player context on Unix
 *      platforms.  This interface allows your plugin to get callbacks based
 *      I/O events that are normally handled by select().  This interface
 *	allows you to setup callbacks which will be executed when a file
 *	descriptor is ready for reading, writing, or has an exception.
 * 
 *  IID_IRMAAsyncIOSelection:
 * 
 *	{0000010C-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAsyncIOSelection, 0x0000010C, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAsyncIOSelection

DECLARE_INTERFACE_(IRMAAsyncIOSelection, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAAsyncIOSelection methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAAsyncIOSelection::Add
     *	Purpose:
     *	    This function will allow you to receive a callback when the
     *	    given descriptor is ready for read, write, or has an
     *	    exception.  This function is only available on Unix, and is
     *	    intended to replace the functionality of select().
     */
    STDMETHOD(Add)			(THIS_
					IRMACallback*	pCallback,
					INT32		lFileDescriptor,
					UINT32		ulType) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAsyncIOSelection::Remove
     *	Purpose:
     *	    This function will allow you remove the callback associated
     *	    with the given descriptor from the event handler.
     *	    This function is only available on Unix, and is intended to
     *	    replace the functionality of select().
     */
    STDMETHOD(Remove)                   (THIS_
                                        INT32           lFileDescriptor,
					UINT32		ulType) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAUDPMulticastInit
 * 
 *  Purpose:
 * 
 *	Provides the user with a way to set the TTL for outgoing multicast
 *	UDP packets.  Usually shared with IRMAUDPSocket.
 * 
 *  IID_IRMAUDPMulticastInit:
 * 
 *	{0000010D-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAUDPMulticastInit, 0x0000010D, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAUDPMulticastInit

DECLARE_INTERFACE_(IRMAUDPMulticastInit, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAUDPMulticastInit methods
     *
     */

     /************************************************************************
     *	Method:
     *	    IRMAUDPMulticastInit::InitMulticast
     *	Purpose:
     *	    This function will set the TTL (time to live) for the UDP socket
     *      so it can be used as a multicast socket, sending packets across
     *      the number of routers specified in the ulTTL parameter.  
     */

    STDMETHOD(InitMulticast)		(THIS_
    					UINT8	    chTTL) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAInterruptState
 * 
 *  Purpose:
 * 
 *	This interface is used in Macintosh implementations to inform the
 *	the client engine when entering & leaving an interupt task. It is
 *	also used to determine if it is currently at interrupt time.
 *	Please consult the Macintosh Deferred Task Manager tech notes from Apple
 *	for information on interrupt tasks.
 * 
 *  IID_IRMAInterruptState:
 * 
 *	{0000010E-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAInterruptState, 0x0000010E, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAInterruptState

DECLARE_INTERFACE_(IRMAInterruptState, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *  IRMAInterruptState methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAInterruptState::AtInterruptTime
     *	Purpose:
     *	    This function is called to determine if we are currently at
     *	    interrupt task time.
     */
        STDMETHOD_(BOOL,AtInterruptTime)		(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAInterruptState::EnterInterruptState
     *	Purpose:
     *	    This function is called when starting a deferred/interrupt task
     */
    STDMETHOD(EnterInterruptState)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAInterruptState::LeaveInterruptState
     *	Purpose:
     *	    This function is called when leaving a deferred/interrupt task
     */
    STDMETHOD(LeaveInterruptState)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAInterruptState::EnableInterrupt
     *	Purpose:
     *	    This function can be called to enable/disable interrupt time 
     *	    processsing
     */
    STDMETHOD(EnableInterrupt)	(THIS_
				BOOL	bEnable) PURE;
    
    /************************************************************************
     *	Method:
     *	    IRMAInterruptState::IsInterruptEnabled
     *	Purpose:
     *	    This function can be called to find if the core is currently
     *	    interrupt enabled.
     */
    STDMETHOD_(BOOL, IsInterruptEnabled)   (THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAOptimizedScheduler
 * 
 *  Purpose:
 * 
 *	This interface provides the user with a way of scheduling callbacks
 *	that will be executed at some time in the future.
 *
 *	This interface should ONLY be used if you need accurately timed 
 *	callbacks. These callbacks should be efficient and should not consume 
 *	much time/CPU. This is not a thread safe interface. The user has to 
 *	take care of synchronization in their callbacks.
 * 
 *  IID_IRMAOptimizedScheduler:
 * 
 *	{0000010F-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAOptimizedScheduler, 0x0000010F, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAOptimizedScheduler

DECLARE_INTERFACE_(IRMAOptimizedScheduler, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAOptimizedScheduler methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAOptimizedScheduler::RelativeEnter
     *	Purpose:
     *	    Schedule a callback to be executed "ms" milliseconds from now
     *	    This function is less percise then AbsoluteEnter and should only
     *	    be used when accurate timing is not critical.
     */
    STDMETHOD_(CallbackHandle,RelativeEnter)	(THIS_
						IRMACallback* pCallback,
						UINT32 ms) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAOptimizedScheduler::AbsoluteEnter
     *	Purpose:
     *	    Schedule a callback to be executed at time "tVal".
     */
    STDMETHOD_(CallbackHandle,AbsoluteEnter)	(THIS_
						IRMACallback* pCallback,
						RMATimeval tVal) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAOptimizedScheduler::Remove
     *	Purpose:
     *	    Remove a callback from the scheduler.
     */
    STDMETHOD(Remove)		(THIS_
			    	CallbackHandle Handle) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAOptimizedScheduler::GetCurrentSchedulerTime
     *	Purpose:
     *	    Gives the current time (in the timeline of the scheduler).
     */
    STDMETHOD_(RMATimeval,GetCurrentSchedulerTime)	(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMALoadBalancedListen
 * 
 *  Purpose:
 * 
 *	This interface is queried off of IRMAListenSocket.  It allows
 *	a plugin to specify that it wants the server to load balance
 *	multiple instances of itself.  The server will instantiate multiple
 *	instances of the plugin as needed based on socket / descriptor limits.
 *	Each plugin instance should attempt to listen on the same port as
 *	other instances (they will share the port).
 * 
 *  IID_IRMALoadBalancedListen:
 * 
 *	{00000110-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMALoadBalancedListen, 0x00000110, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMALoadBalancedListen

DECLARE_INTERFACE_(IRMALoadBalancedListen, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMALoadBalancedListen methods
     */

    /************************************************************************
     *	Method:
     *	    IRMALoadBalancedListen::SetID
     *	Purpose:
     *	    This function set's the unique ID for this listen socket.  This
     *	    ID is used to determine whether or not different instances of
     *	    a plugin trying to listen on a single port are actually the
     *	    same plugin.  Without this function, it would be possible for
     *	    two completely different plugins to listen on the same port using
     *	    the load balanced listener.
     */
    STDMETHOD(SetID)		(THIS_
			    	REFIID ID) PURE;

    /************************************************************************
     *	Method:
     *	    IRMALoadBalancedListen::SetReserveLimit
     *	Purpose:
     *	    Sets the reserve limit for descriptors / sockets.  If less
     *	    than reserve limit descriptors / sockets are left then a new
     *	    instance of the plugin will be created.
     */
    STDMETHOD(SetReserveLimit)	(THIS_
			    	UINT32		ulDescriptors,
				UINT32		ulSockets) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAOverrideDefaultServices
 * 
 *  Purpose:
 * 
 *	This interface is queried off of the context.  It allows
 *	a plugin to override any default services provided by the G2 system.
 *	Currently, it is supported only on the client side. 
 *	You may currently override IRMANetworkServices using this interface
 *	You can use the same interface to later restore back the overriden services.
 *	This is done by calling the same OverrideServices() function with the 
 *	original service QIed before the initial override.
 * 
 *  IID_IRMAOverrideDefaultServices:
 * 
 *	{00000111-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAOverrideDefaultServices, 0x00000111, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAOverrideDefaultServices

DECLARE_INTERFACE_(IRMAOverrideDefaultServices, IUnknown)
{
    /*
     * IUnknown methods
     */
    STDMETHOD(QueryInterface)   (THIS_
                                REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

   /*
    * IRMAOverrideDefaultServices methods
    */

   /************************************************************************
    *  Method:
    *      IRMAOverrideDefaultServices::OverrideServices
    *  Purpose:
    *      Override default services provided by the G2 system.
    *
    */
    STDMETHOD(OverrideServices)         (THIS_
				IUnknown* pContext) PURE;
};

enum PN_SOCKET_OPTION
{
    PN_SOCKOPT_REUSE_ADDR,
    PN_SOCKOPT_REUSE_PORT,
    PN_SOCKOPT_BROADCAST,
    PN_SOCKOPT_SET_RECVBUF_SIZE,
    PN_SOCKOPT_SET_SENDBUF_SIZE
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMASetSocketOption
 * 
 *  Purpose:
 * 
 *	Set sockt option
 * 
 *  IID_IRMASetSocketOption:
 * 
 *	IID_IRMASetSocketOption:    {00000114-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMASetSocketOption,	
    0x00000114, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMASetSocketOption
DECLARE_INTERFACE_(IRMASetSocketOption, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     *	IRMAListenSocket methods
     */

    STDMETHOD(SetOption)		(THIS_ 
					 PN_SOCKET_OPTION option,
					 UINT32 ulValue) PURE;					 
};

#define RMA_THREADSAFE_METHOD_FF_GETPACKET		0x00000001
/*
 * FileFormat::GetPacket() only calls:
 *     CCF->CI(Buffer), CCF->CI(Packet), CCF->CI(Values), *Alloc, *Free, 
 *     FS->Read(), FS->Close(), FS->Seek(),
 *     FFR->PacketReady(), FFR->StreamDone()
 *     Context->Scheduler->*,
 *     CCF->CI(Mutex), Mutex->*
 *     Context->ErrorMessages
 *
 * XXXSMPNOW
 */

#define RMA_THREADSAFE_METHOD_FS_READ			0x00000002
/*
 * FileSystem::Read()/Seek()/Close() only calls:
 *     CCF->CI(Buffer), CCF->CI(Packet), CCF->CI(Values), *Alloc, *Free, 
 *     FS->Read(), FS->Close(), FS->Seek(),
 *     Context->Scheduler->*,
 *     CCF->CI(Mutex), Mutex->*
 *     Context->ErrorMessages
 *
 * XXXSMPNOW
 */
#define RMA_THREADSAFE_METHOD_FSR_READDONE		0x00000004
/*
 * FileFormat::ReadDone()/SeekDone()/CloseDone() only calls:
 *     CCF->CI(Buffer), CCF->CI(Packet), CCF->CI(Values), *Alloc, *Free, 
 *     FS->Read(), FS->Close(), FS->Seek(),
 *     FFR->PacketReady(), FFR->StreamDone()
 *     Context->Scheduler->*,
 *     CCF->CI(Mutex), Mutex->*
 *     Context->ErrorMessages
 *
 * XXXSMPNOW
 */
#define RMA_THREADSAFE_METHOD_CACHE_FILE		0x00000008
/*
 * FileSystem::Read()/Seek()/Close() only calls:
 *     CCF->CI(Buffer), CCF->CI(Packet), CCF->CI(Values), *Alloc, *Free, 
 *     FS->Read(), FS->Close(), FS->Seek(),
 *     IRMACacheFile->*, IRMACacheFileResponse->*,
 *     Context->Scheduler->*,
 *     CCF->CI(Mutex), Mutex->*
 *     Context->ErrorMessages
 *
 * XXXSMPNOW
 */
#define RMA_THREADSAFE_METHOD_CACHE_FILE_RESPONSE	0x00000010
/*
 * FileSystem::Read()/Seek()/Close() only calls:
 *     CCF->CI(Buffer), CCF->CI(Packet), CCF->CI(Values), *Alloc, *Free, 
 *     FS->Read(), FS->Close(), FS->Seek(),
 *     IRMACacheFile->*, IRMACacheFileResponse->*,
 *     Context->Scheduler->*,
 *     CCF->CI(Mutex), Mutex->*
 *     Context->ErrorMessages
 *
 * XXXSMPNOW
 */

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAThreadSafeMethods
 * 
 *  Purpose:
 * 
 *	XXXSMPNOW
 * 
 *  IID_IRMAThreadSafeMethods:
 * 
 *	{00000115-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAThreadSafeMethods, 0x00000115, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAThreadSafeMethods

DECLARE_INTERFACE_(IRMAThreadSafeMethods, IUnknown)
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
     *	IRMAThreadSafeMethods methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAThreadSafeMethods::IsThreadSafe
     *	Purpose:
     *	    XXXSMPNOW
     */
    STDMETHOD_(UINT32,IsThreadSafe)	    (THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAMutex
 * 
 *  Purpose:
 * 
 *	XXXSMPNOW
 * 
 *  IID_IRMAMutex:
 * 
 *	{00000116-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAMutex, 0x00000116, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAMutex

/*
 *  The IRMACommonClassFactory supports creating an instance
 *  of this object.
 */
#define CLSID_IRMAMutex IID_IRMAMutex

DECLARE_INTERFACE_(IRMAMutex, IUnknown)
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
     *	IRMAMutex methods
     */

     /* XXXSMPNOW Comments */
    STDMETHOD(Lock)	    (THIS) PURE;

    STDMETHOD(TryLock)	    (THIS) PURE;

    STDMETHOD(Unlock)	    (THIS) PURE;
};


#endif /* _RMAENGIN_H_ */
