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
 *  RealMedia Architecture Common Utility interfaces
 *
 */

#ifndef _RMACOMM_H_
#define _RMACOMM_H_

#include "rmaengin.h" // For RMATimeval

/*
 * Forward declarations of some interfaces defined here-in.
 */

typedef _INTERFACE	IRMACommonClassFactory		IRMACommonClassFactory;
typedef _INTERFACE	IRMAStatistics			IRMAStatistics;
typedef _INTERFACE	IRMARegistryID			IRMARegistryID;
typedef _INTERFACE	IRMAServerFork			IRMAServerFork;
typedef _INTERFACE	IRMAServerControl		IRMAServerControl;
typedef _INTERFACE	IRMAReconfigServerResponse	IRMAReconfigServerResponse;
typedef _INTERFACE	IRMABuffer			IRMABuffer;
typedef _INTERFACE	IRMAWantServerReconfigNotification
						IRMAWantServerReconfigNotification;  
typedef _INTERFACE	IRMAFastAlloc			IRMAFastAlloc;



/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMACommonClassFactory
 * 
 *  Purpose:
 * 
 *	RMA interface that manages the creation of common RMA classes.
 * 
 *  IID_IRMACommonClassFactory:
 * 
 *	{00000000-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMACommonClassFactory, 0x00000000, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMACommonClassFactory

DECLARE_INTERFACE_(IRMACommonClassFactory, IUnknown)
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
     *	IRMACommonClassFactory methods
     */

    /************************************************************************
     *	Method:
     *	    IRMACommonClassFactory::CreateInstance
     *	Purpose:
     *	    Creates instances of common objects supported by the system,
     *	    like IRMABuffer, IRMAPacket, IRMAValues, etc.
     *
     *	    This method is similar to Window's CoCreateInstance() in its 
     *	    purpose, except that it only creates objects of a well known
     *	    types.
     *
     *	    NOTE: Aggregation is never used. Therefore and outer unknown is
     *	    not passed to this function, and you do not need to code for this
     *	    situation.
     */
    STDMETHOD(CreateInstance)		(THIS_
					REFCLSID    /*IN*/  rclsid,
					void**	    /*OUT*/ ppUnknown) PURE;

    /************************************************************************
     *  Method:
     *	    IRMAController::CreateInstanceAggregatable
     *  Purpose:
     *	    Creates instances of common objects that can be aggregated
     *	    supported by the system, like IRMASiteWindowed
     *
     *	    This method is similar to Window's CoCreateInstance() in its 
     *	    purpose, except that it only creates objects of a well known
     *	    types.
     *
     *	    NOTE 1: Unlike CreateInstance, this method will create internal
     *		    objects that support Aggregation.
     *
     *	    NOTE 2: The output interface is always the non-delegating 
     *		    IUnknown.
     */
    STDMETHOD(CreateInstanceAggregatable)
				    (THIS_
				    REFCLSID	    /*IN*/  rclsid,
				    REF(IUnknown*)  /*OUT*/ ppUnknown,
				    IUnknown*	    /*IN*/  pUnkOuter) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAStatistics
 * 
 *  Purpose:
 * 
 *	This interface allows update of the client statistics.
 * 
 *  IID_IRMAStatistics:
 * 
 *	{00000001-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAStatistics, 0x00000001, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
	    0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAStatistics

DECLARE_INTERFACE_(IRMAStatistics, IUnknown)
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
     *	IRMAStatistics methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAStatistics::Init
     *	Purpose:
     *	    Pass registry ID to the caller
     *
     */
    STDMETHOD(InitializeStatistics)	(THIS_
					UINT32	/*IN*/  ulRegistryID) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStatistics::Update
     *	Purpose:
     *	    Notify the client to update its statistics stored in the registry
     *
     */
    STDMETHOD(UpdateStatistics)		(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMARegistryID
 * 
 *  Purpose:
 * 
 *	This interface is implemented by IRMAPlayer, IRMAStreamSource,
 *	and IRMAStream.  It allows the user to get the registry Base ID,
 *	for an object that you have a pointer to.
 * 
 *  IID_IRMARegistryID:
 * 
 *	{00000002-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMARegistryID, 0x00000002, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
	    0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMARegistryID

DECLARE_INTERFACE_(IRMARegistryID, IUnknown)
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
     *	IRMARegistryID methods
     */

    /************************************************************************
     *	Method:
     *	    IRMARegistryID::GetID
     *	Purpose:
     *	    Get the registry ID of the object.
     *
     */
    STDMETHOD(GetID)	(THIS_
			REF(UINT32)	/*OUT*/  ulRegistryID) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAServerFork
 * 
 *  Purpose:
 * 
 *	This interface is implemented by the server context on Unix
 *	platforms.  This interface allows your plugin to fork off a
 *	process.  Note that the process that is forked off cannot use
 *	any RMA APIs.  The fork() system call is prohibited from within
 *	a RMA plugin.
 *
 *  IID_IRMAServerFork:
 * 
 *	{00000003-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAServerFork, 0x00000003, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
	    0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAServerFork

DECLARE_INTERFACE_(IRMAServerFork, IUnknown)
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
     *	IRMAServerFork methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAServerFork::Fork
     *	Purpose:
     *	    Fork off a child process.  The child process cannot use any RMA
     *	    APIs.  Upon successful completion, Fork returns 0 to the child
     *	    process and the PID of the child to the parent.  A return value
     *	    of -1 indicates an error.
     *
     *	    Note:  The child process should *NOT* Release any interfaces.
     *		The cleanup of the IRMAServerFork() interface and other
     *		RMA interfaces is done by the parent.
     *
     */
    STDMETHOD_(INT32, Fork)	(THIS) PURE;
};

/*
 * 
 *  Interface:
 *
 *	IRMAServerControl
 *
 *  Purpose:
 *
 *	This inteface provides access to the RealMedia server's controls
 *      for shutting down (for now).
 *
 *	Note:  This registry is not related to the Windows system registry.
 *
 *  IID_IRMAServerControl:
 *
 *	{00000004-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAServerControl, 0x00000004, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
            0xa0, 0x24, 0x40, 0x6d, 0x59);

#define CLSID_IRMAServerControl	IID_IRMAServerControl

#undef  INTERFACE
#define INTERFACE   IRMAServerControl

DECLARE_INTERFACE_(IRMAServerControl, IUnknown)
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
     *	IRMAServerControl methods
     */

    /************************************************************************
     *  Method:
     *      IRMAServerControl::ShutdownServer
     *  Purpose:
     *      Shutdown the server.
     */
    STDMETHOD(ShutdownServer)		(THIS_
					UINT32 status) PURE;
};

/*
 * 
 *  Interface:
 *
 *	IRMAServerControl2
 *
 *  Purpose:
 *
 *	Interface for extended server control methods.
 *
 *
 *  IID_IRMAServerControl2:
 *
 *	{00000005-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAServerControl2, 0x00000005, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
            0xa0, 0x24, 0x40, 0x6d, 0x59);


#undef  INTERFACE
#define INTERFACE   IRMAServerControl2

DECLARE_INTERFACE_(IRMAServerControl2, IUnknown)
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
     *	IRMAServerControl2 methods
     */

    /************************************************************************
     * IRMAServerControl2::RestartServer
     *
     * Purpose:
     *
     *	    Completely shutdown the server, then restart.  Mainly used to
     * cause not hot setting config var changes to take effect.
     */
    STDMETHOD(RestartServer) (THIS) PURE;

    /************************************************************************
     * IRMAServerControl2::ReconfigServer
     *
     * Purpose:
     *
     *	    Used to cause the server to re-read in config from file or registry
     * (however it was started) and attempt to use the values.
     */
    STDMETHOD(ReconfigServer)	(THIS_ IRMAReconfigServerResponse* pResp) PURE;

};

/*
 * 
 *  Interface:
 *
 *	IRMAReconfigServerResponse
 *
 *  Purpose:
 *
 *	Response interface for IRMAServerControl2::ReconfigServer
 *
 *
 *  IID_IRMAReconfigServerResponse:
 *
 *	{00000006-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAReconfigServerResponse, 0x00000006, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
            0xa0, 0x24, 0x40, 0x6d, 0x59);


#undef  INTERFACE
#define INTERFACE   IRMAReconfigServerResponse

DECLARE_INTERFACE_(IRMAReconfigServerResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
     * IRMAReconfigServerResponse::ReconfigServerDone
     *
     * Purpose:
     *
     *	    Notification that reconfiguring the server is done.
     */
    STDMETHOD(ReconfigServerDone)   (THIS_
				    PN_RESULT res,
				    IRMABuffer** pInfo,
				    UINT32 ulNumInfo) PURE;
};

/*
 * 
 *  Interface:
 *
 *	IRMAServerReconfigNotification
 *
 *  Purpose:
 *
 *	Register with the server that you want notification when a reconfig
 *  request comes in and want/need to take part in the reconfiguration.  This
 *  is used when you have configuration info outside the server config file
 *  which needs to be re-initialized.
 *
 *
 *  IID_IRMAServerReconfigNotification:
 *
 *	{00000007-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAServerReconfigNotification, 0x00000007, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
            0xa0, 0x24, 0x40, 0x6d, 0x59);


#undef  INTERFACE
#define INTERFACE   IRMAServerReconfigNotification

DECLARE_INTERFACE_(IRMAServerReconfigNotification, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
     * IRMAServerReconfigNotification::WantReconfigNotification
     *
     * Purpose:
     *
     *	    Tell the server that you want reconfig notification.
     */
    STDMETHOD(WantReconfigNotification)	(THIS_
		IRMAWantServerReconfigNotification* pResponse) PURE;
    
    /************************************************************************
     * IRMAServerReconfigNotification::CancelReconfigNotification
     *
     * Purpose:
     *
     *	    Tell the server that you no longer want reconfig notification.
     */
    STDMETHOD(CancelReconfigNotification)   (THIS_
		IRMAWantServerReconfigNotification* pResponse) PURE;

};

/*
 * 
 *  Interface:
 *
 *	IRMAWantServerReconfigNotification
 *
 *  Purpose:
 *
 *	Tell user that the server got a reconfig request and it is time to
 *  do your reconfiguration.  NOTE: You should not need this if all of your
 *  configuration is stored in the config file; that is taken care of through
 *  IRMAActiveRegistry.
 *
 *  IID_IRMAWantServerReconfigNotification:
 *
 *	{00000008-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAWantServerReconfigNotification, 0x00000008, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
            0xa0, 0x24, 0x40, 0x6d, 0x59);


#undef  INTERFACE
#define INTERFACE   IRMAWantServerReconfigNotification

DECLARE_INTERFACE_(IRMAWantServerReconfigNotification, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
     * IRMAWantServerReconfigNotification::ServerReconfig
     *
     * Purpose:
     *
     *	    Notify user that a server reconfig request had come in and it
     * is now your turn to do external (not server config) reconfiguration.*
     */
    STDMETHOD(ServerReconfig)	(THIS_
	IRMAReconfigServerResponse* pResponse) PURE;

};


#endif /*_RMACOMM_H_*/
