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
 *  Interfaces related to allowance plugins.
 *
 */

#ifndef _RMAALLOW_H_
#define _RMAALLOW_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE  IRMAValues			    IRMAValues;
typedef _INTERFACE  IRMABuffer			    IRMABuffer;
typedef _INTERFACE  IRMARequest			    IRMARequest;
typedef _INTERFACE  IRMAPlayerConnectionAdviseSinkManager  IRMAPlayerConnectionAdviseSinkManager;
typedef _INTERFACE  IRMAPlayerConnectionAdviseSink  IRMAPlayerConnectionAdviseSink;
typedef _INTERFACE  IRMAPlayerConnectionResponse    IRMAPlayerConnectionResponse;
typedef _INTERFACE  IRMAPlayerController            IRMAPlayerController;
typedef _INTERFACE  IRMAPlayerControllerProxyRedirect IRMAPlayerControllerProxyRedirect;

/****************************************************************************
 * 
 *  Interface:
 * 
 *      IRMAPlayerConnectionAdviseSink
 * 
 *  Purpose:
 *
 *      Advise Sink which receives notification whenever a new player 
 *      connects to the server.
 * 
 *  IID_IRMAPlayerConnectionAdviseSink:
 * 
 *      {00002600-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPlayerConnectionAdviseSink, 0x00002600, 0x901, 0x11d1, 
	    0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE IRMAPlayerConnectionAdviseSink

DECLARE_INTERFACE_(IRMAPlayerConnectionAdviseSink, IUnknown)
{
    STDMETHOD(QueryInterface)           (THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)          (THIS) PURE;

    STDMETHOD_(ULONG,Release)         (THIS) PURE;

    /* OnConnection is called when a new player has connected to the
     * server.  If the result is PNR_OK, then the plugin will be notified
     * when certain events occur in the player's life cycle.
     */
    STDMETHOD(OnConnection) (THIS_ 
				IRMAPlayerConnectionResponse* pResponse) PURE;

    /* SetPlayerController is called by the server core to provide us with
     * an interface which can stop, alert, redirect or otherwise control
     * the player we are receiving notifications about.
     */
    STDMETHOD(SetPlayerController)  (THIS_
				IRMAPlayerController* pPlayerController) PURE;

    /* SetRegistryID is called by the server core to provide us with the
     * ID for this Player in the server registry. The plugin can use this
     * registry ID to find out various information about the connected player.
     */
    STDMETHOD(SetRegistryID)        (THIS_ UINT32 ulPlayerRegistryID) PURE;

    STDMETHOD(OnURL)                (THIS_ IRMARequest* pRequest) PURE;
    STDMETHOD(OnBegin)              (THIS) PURE;
    STDMETHOD(OnStop)               (THIS) PURE;
    STDMETHOD(OnPause)              (THIS) PURE;
    STDMETHOD(OnDone)               (THIS) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *      IRMAPlayerConnectionResponse
 * 
 *  Purpose:
 *
 *      Response object for the PlayerConnectionAdviseSink.
 * 
 *  IID_IRMAPlayerConnectionResponse:
 * 
 *      {00002601-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPlayerConnectionResponse, 0x00002601, 0x901, 0x11d1, 
	    0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE IRMAPlayerConnectionResponse

DECLARE_INTERFACE_(IRMAPlayerConnectionResponse, IUnknown)
{
    STDMETHOD(QueryInterface)           (THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)          (THIS) PURE;

    STDMETHOD_(ULONG,Release)         (THIS) PURE;

    STDMETHOD(OnConnectionDone)		(THIS_ PN_RESULT status) PURE;
    STDMETHOD(OnURLDone)		(THIS_ PN_RESULT status) PURE;
    STDMETHOD(OnBeginDone)		(THIS_ PN_RESULT status) PURE;
    STDMETHOD(OnStopDone)		(THIS_ PN_RESULT status) PURE;
    STDMETHOD(OnPauseDone)		(THIS_ PN_RESULT status) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *      IRMAPlayerController
 * 
 *  Purpose:
 *
 *      Object created by the server core and given to the 
 *      IRMAPlayerConnectionResponse object so that the response object
 *      can control the connected player.
 * 
 *  IID_IRMAPlayerController:
 * 
 *      {00002602-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPlayerController, 0x00002602, 0x901, 0x11d1, 
	    0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE IRMAPlayerController

DECLARE_INTERFACE_(IRMAPlayerController, IUnknown)
{
    STDMETHOD(QueryInterface)           (THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)          (THIS) PURE;

    STDMETHOD_(ULONG,Release)         (THIS) PURE;

    STDMETHOD(Pause)		        (THIS) PURE;
    STDMETHOD(Resume)                   (THIS) PURE;
    STDMETHOD(Disconnect)               (THIS) PURE;
    STDMETHOD(AlertAndDisconnect)       (THIS_ IRMABuffer* pAlert) PURE;

    /* HostRedirect is called by a PlayerConnectionAdviseSink to redirect
     * this player to another host and/or port, for the same URL. This
     * method works with both RTSP and PNA protocols.
     */
    STDMETHOD(HostRedirect)             (THIS_ IRMABuffer* pHost, 
					UINT16 nPort) PURE;

    /* NetworkRedirect is called by a PlayerConnectionAdviseSink to redirect
     * this player to another URL. Note: This method is only available for
     * redirecting an RTSP player connection to another RTSP URL.
     */
    STDMETHOD(NetworkRedirect)          (THIS_ IRMABuffer* pURL,
					UINT32 ulSecsFromNow) PURE;

    /* Redirect is called by a PlayerConnectionAdviseSink to redirect
     * this player to another URL on the same server. For example, if 
     * pPartialURL were set to "welcome.rm", the player would be redirected
     * to "current_protocol://current_host:current_port/welcome.rm". This
     * method works with both RTSP and PNA protocols.
     */
    STDMETHOD(Redirect)			(THIS_ IRMABuffer* pPartialURL) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *      IRMAPlayerConnectionAdviseSinkManager
 * 
 *  Purpose:
 *
 *      Manages the creation of IRMAPlayerConnectionAdviseSink objects
 * 
 *  IID_IRMAPlayerConnectionAdviseSinkManager:
 * 
 *      {00002603-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPlayerConnectionAdviseSinkManager, 0x00002603, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE IRMAPlayerConnectionAdviseSinkManager

DECLARE_INTERFACE_(IRMAPlayerConnectionAdviseSinkManager, IUnknown)
{
    STDMETHOD(QueryInterface)           (THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)          (THIS) PURE;

    STDMETHOD_(ULONG,Release)         (THIS) PURE;

    STDMETHOD(CreatePlayerConnectionAdviseSink)
		(THIS_
		REF(IRMAPlayerConnectionAdviseSink*) pPCAdviseSink) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *      IRMAPlayerControllerProxyRedirect
 * 
 *  Purpose:
 *
 *      QueryInterfaced from IRMAPlayerController.  Allows 305 proxy redirect
 *      to be issued (as per RTSP spec).
 * 
 *  IID_IRMAPlayerControllerProxyRedirect:
 * 
 *      {00002607-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPlayerControllerProxyRedirect, 0x00002607, 0x901, 0x11d1, 
	    0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE IRMAPlayerControllerProxyRedirect

DECLARE_INTERFACE_(IRMAPlayerControllerProxyRedirect, IUnknown)
{
    STDMETHOD(QueryInterface)           (THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)          (THIS) PURE;

    STDMETHOD_(ULONG,Release)         (THIS) PURE;

    /*
     * This URL is just a hostname / port.  It must be formatted like this:
     * "rtsp://audio.real.com:554/".
     *
     * NOTE:  You can *only* call this method between OnURL() and OnURLDone().
     * NOTE:  This method only works on RTSP connections.
     */
    STDMETHOD(NetworkProxyRedirect)          (THIS_ IRMABuffer* pURL) PURE;
};

#endif /* _RMAALLOW_H_ */
