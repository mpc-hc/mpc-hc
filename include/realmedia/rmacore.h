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
 *  Client Core interfaces
 *
 */

#ifndef _RMACORE_H_
#define _RMACORE_H_


/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE	IUnknown			IUnknown;
typedef _INTERFACE	IRMAStream			IRMAStream;
typedef _INTERFACE	IRMAStreamSource		IRMAStreamSource;
typedef _INTERFACE	IRMAPlayer			IRMAPlayer;
typedef _INTERFACE	IRMAClientEngine		IRMAClientEngine;
typedef _INTERFACE	IRMAScheduler			IRMAScheduler;
typedef _INTERFACE	IRMAClientAdviseSink		IRMAClientAdviseSink;
typedef _INTERFACE	IRMAValues      		IRMAValues;
typedef _INTERFACE	IRMABuffer      		IRMABuffer;
typedef _INTERFACE	IRMAPacket			IRMAPacket;
typedef _INTERFACE	IRMARenderer			IRMARenderer;
typedef _INTERFACE	IRMAPlayer2			IRMAPlayer2;
typedef _INTERFACE	IRMARequest			IRMArequest;

typedef struct _PNxEvent PNxEvent;


#ifdef _MACINTOSH
#pragma export on
#endif

#if defined _UNIX && !(defined _VXWORKS)
/* Includes needed for select() stuff */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef _BEOS	// fd_set stuff
#include <net/socket.h>
#endif

/* Used in renderer and advise sink interface */
enum BUFFERING_REASON
{
    BUFFERING_START_UP	= 0,
    BUFFERING_SEEK,
    BUFFERING_CONGESTION,
    BUFFERING_LIVE_PAUSE
};

/****************************************************************************
 * 
 *  Function:
 * 
 *	CreateEngine()
 * 
 *  Purpose:
 * 
 *	Function implemented by the RMA core to return a pointer to the
 *	client engine.  This function would be run by top level clients.
 */
STDAPI CreateEngine
		(
		    IRMAClientEngine**  /*OUT*/ ppEngine
		);

/****************************************************************************
 * 
 *  Function:
 * 
 *	CloseEngine()
 * 
 *  Purpose:
 * 
 *	Function implemented by the RMA core to close the engine which
 *	was returned in CreateEngine().
 */
STDAPI CloseEngine
		(
		    IRMAClientEngine*  /*IN*/ pEngine
		);

#ifdef _MACINTOSH
#pragma export off
#endif

/*
 * Definitions of Function Pointers to CreateEngine() and Close Engine().
 * These types are provided as a convenince to authors of top level clients.
 */
typedef PN_RESULT (PNEXPORT_PTR FPRMCREATEENGINE)(IRMAClientEngine** ppEngine);
typedef PN_RESULT (PNEXPORT_PTR FPRMCLOSEENGINE) (IRMAClientEngine*  pEngine);
typedef PN_RESULT (PNEXPORT_PTR FPRMSETDLLACCESSPATH) (const char*);

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAStream
 *
 *  Purpose:
 *
 *	Interface provided by the client engine to the renderers. This
 *	interface allows access to stream related information and properties.
 *
 *  IID_IRMAStream:
 *
 *	{00000400-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAStream, 0x00000400, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAStream

DECLARE_INTERFACE_(IRMAStream, IUnknown)
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
     * IRMAStream methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAStream::GetSource
     *	Purpose:
     *	    Get the interface to the source object of which the stream is
     *	    a part of.
     *
     */
    STDMETHOD(GetSource)		(THIS_
					REF(IRMAStreamSource*)	pSource) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStream::GetStreamNumber
     *	Purpose:
     *	    Get the stream number for this stream relative to the source 
     *	    object of which the stream is a part of.
     *
     */
    STDMETHOD_(UINT16,GetStreamNumber)	    (THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStream::GetStreamType
     *	Purpose:
     *	    Get the MIME type for this stream. NOTE: The returned string is
     *	    assumed to be valid for the life of the IRMAStream from which it
     *	    was returned.
     *
     */
    STDMETHOD_(const char*,GetStreamType)   (THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStream::GetHeader
     *	Purpose:
     *      Get the header for this stream.
     *
     */
    STDMETHOD_(IRMAValues*,GetHeader)   (THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStream::ReportQualityOfService
     *	Purpose:
     *	    Call this method to report to the playback context that the 
     *	    quality of service for this stream has changed. The unQuality
     *	    should be on a scale of 0 to 100, where 100 is the best possible
     *	    quality for this stream. Although the transport engine can 
     *	    determine lost packets and report these through the user
     *	    interface, only the renderer of this stream can determine the 
     *	    "real" perceived damage associated with this loss.
     *
     *	    NOTE: The playback context may use this value to indicate loss
     *	    in quality to the user interface. When the effects of a lost
     *	    packet are eliminated the renderer should call this method with
     *	    a unQuality of 100.
     *
     */
    STDMETHOD(ReportQualityOfService)	    (THIS_
					    UINT8   unQuality) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStream::ReportRebufferStatus
     *	Purpose:
     *	    Call this method to report to the playback context that the
     *	    available data has dropped to a critically low level, and that
     *	    rebuffering should occur. The renderer should call back into this
     *	    interface as it receives additional data packets to indicate the
     *	    status of its rebuffering effort.
     *
     *	    NOTE: The values of unNeeded and unAvailable are used to indicate
     *	    the general status of the rebuffering effort. For example, if a
     *	    renderer has "run dry" and needs 5 data packets to play smoothly
     *	    again, it should call ReportRebufferStatus() with 5,0 then as
     *	    packet arrive it should call again with 5,1; 5,2... and eventually
     *	    5,5.
     *
     */
    STDMETHOD(ReportRebufferStatus)	    (THIS_
					    UINT8   unNeeded,
					    UINT8   unAvailable) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStream::SetGranularity
     *	Purpose:
     *	    Sets the desired Granularity for this stream. The actual 
     *	    granularity will be the lowest granularity of all streams.
     *	    Valid to call before stream actually begins. Best to call during
     *	    IRMARenderer::OnHeader().
     */
    STDMETHOD(SetGranularity)		    (THIS_
					    ULONG32 ulGranularity) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStream::GetRendererCount
     *	Purpose:
     *	    Returns the current number of renderer instances supported by
     *	    this stream instance.
     */
    STDMETHOD_(UINT16, GetRendererCount)(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStream::GetRenderer
     *	Purpose:
     *	    Returns the Nth renderer instance supported by this stream.
     */
    STDMETHOD(GetRenderer)	(THIS_
				UINT16		nIndex,
				REF(IUnknown*)	pUnknown) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAStreamSource
 *
 *  Purpose:
 *
 *	Interface provided by the client engine to the renderers. This
 *	interface allows access to source related information and properties.
 *
 *  IID_IRMAStreamSource:
 *
 *	{00000401-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAStreamSource, 0x00000401, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAStreamSource

DECLARE_INTERFACE_(IRMAStreamSource, IUnknown)
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
     * IRMAStreamSource methods
     */

    /************************************************************************
     *	Method:
     *		IRMAStreamSource::IsLive
     *	Purpose:
     *		Ask the source whether it is live
     *
     */
    STDMETHOD_ (BOOL,IsLive)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStreamSource::GetPlayer
     *	Purpose:
     *	    Get the interface to the player object of which the source is
     *	    a part of.
     *
     */
    STDMETHOD(GetPlayer)	    (THIS_
				    REF(IRMAPlayer*)	pPlayer) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStreamSource::GetURL
     *	Purpose:
     *	    Get the URL for this source. NOTE: The returned string is
     *	    assumed to be valid for the life of the IRMAStreamSource from which
     *	    it was returned.
     *
     */
    STDMETHOD_(const char*,GetURL)  (THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStreamSource::GetStreamCount
     *	Purpose:
     *	    Returns the current number of stream instances supported by
     *	    this source instance.
     */
    STDMETHOD_(UINT16, GetStreamCount)(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAStreamSource::GetStream
     *	Purpose:
     *	    Returns the Nth stream instance supported by this source.
     */
    STDMETHOD(GetStream)	(THIS_
				UINT16		nIndex,
				REF(IUnknown*)	pUnknown) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *  	IRMAPlayer
 *
 *  Purpose:
 *
 *	Interface provided by the client engine to the renderers. This
 *	interface allows access to player related information, properties,
 *	and operations.
 *
 *  IID_IRMAPlayer:
 *
 *	{00000402-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAPlayer, 0x00000402, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPlayer

DECLARE_INTERFACE_(IRMAPlayer, IUnknown)
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
     * IRMAPlayer methods
     */

    /************************************************************************
     *	Method:
     *		IRMAPlayer::GetClientEngine
     *	Purpose:
     *		Get the interface to the client engine object of which the
     *		player is a part of.
     *
     */
    STDMETHOD(GetClientEngine)	(THIS_
				REF(IRMAClientEngine*)	pEngine) PURE;

    /************************************************************************
     *	Method:
     *		IRMAPlayer::IsDone
     *	Purpose:
     *		Ask the player if it is done with the current presentation
     *
     */
    STDMETHOD_(BOOL,IsDone)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *		IRMAPlayer::IsLive
     *	Purpose:
     *		Ask the player whether it contains the live source
     *
     */
    STDMETHOD_(BOOL,IsLive)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *		IRMAPlayer::GetCurrentPlayTime
     *	Purpose:
     *		Get the current time on the Player timeline
     *
     */
    STDMETHOD_(ULONG32,GetCurrentPlayTime)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *		IRMAPlayer::OpenURL
     *	Purpose:
     *		Tell the player to begin playback of all its sources.
     *
     */
    STDMETHOD(OpenURL)		(THIS_
				    const char*	pURL) PURE;

    /************************************************************************
     *	Method:
     *		IRMAPlayer::Begin
     *	Purpose:
     *		Tell the player to begin playback of all its sources.
     *
     */
    STDMETHOD(Begin)		(THIS) PURE;

    /************************************************************************
     *	Method:
     *		IRMAPlayer::Stop
     *	Purpose:
     *		Tell the player to stop playback of all its sources.
     *
     */
    STDMETHOD(Stop)		(THIS) PURE;

    /************************************************************************
     *	Method:
     *		IRMAPlayer::Pause
     *	Purpose:
     *		Tell the player to pause playback of all its sources.
     *
     */
    STDMETHOD(Pause)		(THIS) PURE;

    /************************************************************************
     *	Method:
     *		IRMAPlayer::Seek
     *	Purpose:
     *		Tell the player to seek in the playback timeline of all its 
     *		sources.
     *
     */
    STDMETHOD(Seek)		(THIS_
				ULONG32			ulTime) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlayer::GetSourceCount
     *	Purpose:
     *	    Returns the current number of source instances supported by
     *	    this player instance.
     */
    STDMETHOD_(UINT16, GetSourceCount)(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlayer::GetSource
     *	Purpose:
     *	    Returns the Nth source instance supported by this player.
     */
    STDMETHOD(GetSource)	(THIS_
				UINT16		nIndex,
				REF(IUnknown*)	pUnknown) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlayer::SetClientContext
     *	Purpose:
     *	    Called by the client to install itself as the provider of client
     *	    services to the core. This is traditionally called by the top 
     *	    level client application.
     */
    STDMETHOD(SetClientContext)	(THIS_
				IUnknown* pUnknown) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlayer::GetClientContext
     *	Purpose:
     *	    Called to get the client context for this player. This is
     *	    set by the top level client application.
     */
    STDMETHOD(GetClientContext)	(THIS_
				REF(IUnknown*) pUnknown) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlayer::AddAdviseSink
     *	Purpose:
     *	    Call this method to add a client advise sink.
     *
     */
    STDMETHOD(AddAdviseSink)	(THIS_
				IRMAClientAdviseSink*	pAdviseSink) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPlayer::RemoveAdviseSink
     *	Purpose:
     *	    Call this method to remove a client advise sink.
     */
    STDMETHOD(RemoveAdviseSink)	(THIS_
				IRMAClientAdviseSink*	pAdviseSink) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAClientEngine
 *
 *  Purpose:
 *
 *	Interface to the basic client engine. Provided to the renderers and
 *	other client side components.
 *
 *  IID_IRMAClientEngine:
 *
 *	{00000403-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAClientEngine, 0x00000403, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAClientEngine

DECLARE_INTERFACE_(IRMAClientEngine, IUnknown)
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
     * IRMAClientEngine methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAClientEngine::CreatePlayer
     *	Purpose:
     *	    Creates a new IRMAPlayer instance.
     *
     */
    STDMETHOD(CreatePlayer)	(THIS_
				REF(IRMAPlayer*)    pPlayer) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientEngine::ClosePlayer
     *	Purpose:
     *	    Called by the client when it is done using the player...
     *
     */
    STDMETHOD(ClosePlayer)	(THIS_
				IRMAPlayer*    pPlayer) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientEngine::GetPlayerCount
     *	Purpose:
     *	    Returns the current number of IRMAPlayer instances supported by
     *	    this client engine instance.
     */
    STDMETHOD_(UINT16, GetPlayerCount)(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientEngine::GetPlayer
     *	Purpose:
     *	    Returns the Nth IRMAPlayer instances supported by this client 
     *	    engine instance.
     */
    STDMETHOD(GetPlayer)	(THIS_
				UINT16		nPlayerNumber,
				REF(IUnknown*)	pUnknown) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientEngine::EventOccurred
     *	Purpose:
     *	    Clients call this to pass OS events to all players. PNxEvent
     *	    defines a cross-platform event.
     */
    STDMETHOD(EventOccurred)	(THIS_
				PNxEvent* /*IN*/ pEvent) PURE;
};

#if defined _UNIX && !defined (_VXWORKS)
DEFINE_GUID(IID_IRMAClientEngineSelector, 0x00000404, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAClientEngineSelector

DECLARE_INTERFACE_(IRMAClientEngineSelector, IUnknown)
{
    /*
     * IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientEngine::Select
     *	Purpose:
     *      Top level clients under Unix should use this instead of
     *      select() to select for events.
     */
    STDMETHOD_(INT32, Select) (THIS_
			       INT32 n,
			       fd_set *readfds,
			       fd_set *writefds,
			       fd_set *exceptfds,
			       struct timeval* timeout) PURE;
};
#endif /* _UNIX */

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAClientEngineSetup
 *
 *  Purpose:
 *
 *	Interface to the basic client engine. Provided to the renderers and
 *	other client side components.
 *
 *  IID_IRMAClientEngineSetup:
 *
 *	{00000405-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAClientEngineSetup, 0x00000405, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAClientEngineSetup

DECLARE_INTERFACE_(IRMAClientEngineSetup, IUnknown)
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
     * IRMAClientEngineSetup methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAClientEngineSetup::Setup
     *	Purpose:
     *      Top level clients use this interface to over-ride certain basic 
     *	    interfaces implemented by the core. Current over-ridable 
     *	    interfaces are: IRMAPreferences, IRMAHyperNavigate
     */
    STDMETHOD(Setup)		(THIS_
				IUnknown* pContext) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAInfoLogger
 *
 *  Purpose:
 *
 *	Interface to send any logging information back to the server.
 *	This information will appear in the server's access log.
 *
 *  IID_IRMAInfoLogger:
 *
 *	{00000409-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAInfoLogger, 0x00000409, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAInfoLogger

DECLARE_INTERFACE_(IRMAInfoLogger, IUnknown)
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
     *	IRMAInfoLogger methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAInfoLogger::LogInformation
     *	Purpose:
     *	    Logs any user defined information in form of action and 
     *	    associated data.
     */
    STDMETHOD(LogInformation)		(THIS_				
					const char* /*IN*/ pAction,
					const char* /*IN*/ pData) PURE;
};



/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAPlayer2
 *
 *  Purpose:
 *
 *	Extra methods in addition to IRMAPlayer
 *
 *  IID_IRMAPlayer2:
 *
 *	{00000411-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAPlayer2, 0x00000411, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPlayer2

DECLARE_INTERFACE_(IRMAPlayer2, IUnknown)
{
    /*
     * IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IID_IRMAPlayer2::SetMinimumPreroll
     *	Purpose:
     *	    Call this method to set the minimum preroll of this clip
     */
    STDMETHOD(SetMinimumPreroll) (THIS_
				UINT32	ulMinPreroll) PURE;

    /************************************************************************
     *	Method:
     *	    IID_IRMAPlayer2::GetMinimumPreroll
     *	Purpose:
     *	    Call this method to get the minimum preroll of this clip
     */
    STDMETHOD(GetMinimumPreroll) (THIS_
				REF(UINT32) ulMinPreroll) PURE;

    /************************************************************************
     *	Method:
     *	    IID_IRMAPlayer2::OpenRequest
     *	Purpose:
     *	    Call this method to open the IRMARequest
     */
    STDMETHOD(OpenRequest) (THIS_
			    IRMARequest* pRequest) PURE;

    /************************************************************************
     *	Method:
     *	    IID_IRMAPlayer2::GetRequest
     *	Purpose:
     *	    Call this method to get the IRMARequest
     */
    STDMETHOD(GetRequest) (THIS_
			   REF(IRMARequest*) pRequest) PURE;
};

#endif /* _RMACORE_H_ */

