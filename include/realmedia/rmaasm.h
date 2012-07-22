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
 *  RealMedia Architecture Interfaces for Adaptive Stream Management and
 *  Backchannel Support.
 *
 */

#ifndef _RMAASM_H_
#define _RMAASM_H_

/*
 * Forward declarations of some interfaces defined here-in.
 */
typedef _INTERFACE	IRMAPacket			IRMAPacket;
typedef _INTERFACE	IRMABackChannel			IRMABackChannel;
typedef _INTERFACE	IRMAASMSource			IRMAASMSource;
typedef _INTERFACE	IRMAASMStreamSink		IRMAASMStreamSink;
typedef _INTERFACE	IRMAASMStream			IRMAASMStream;


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMABackChannel
 * 
 *  Purpose:
 * 
 *      Backchannel interface to be used by renderers and implemented by
 *	FileFormat Plugins
 * 
 *  IID_IRMABackChannel:
 *  
 *	{00001500-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMABackChannel, 0x00001500, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
				 0xa0, 0x24, 0x40, 0x6d, 0x59);
#undef  INTERFACE
#define INTERFACE   IRMABackChannel

DECLARE_INTERFACE_(IRMABackChannel, IUnknown)
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
     * IRMABackChannel method
     */

    /************************************************************************
     *	Method:
     *	    IRMABackChannel::PacketReady
     *	Purpose:
     *      A back channel packet sent from Renderer to FileFormat plugin.
     */
    STDMETHOD(PacketReady)	(THIS_
				IRMAPacket* pPacket) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAASMSource
 *
 *  Purpose:
 *
 *      This interface is implemented by file formats so that they can
 *	act on ASM Subscribe and Unsubscribe actions.
 *
 *  IID_IRMAASMSource:
 *
 *	{00001501-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAASMSource, 0x00001501, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
				 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAASMSource

DECLARE_INTERFACE_(IRMAASMSource, IUnknown)
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
     * IRMAASMSource methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAASMSource::Subscribe
     *	Purpose:
     *      Called to inform a file format that a subscription has occurred,
     *	    to rule number uRuleNumber, for stream uStreamNumber.
     */
    STDMETHOD(Subscribe)	(THIS_
				UINT16	uStreamNumber,
				UINT16	uRuleNumber) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAASMSource::Unsubscribe
     *	Purpose:
     *      Called to inform a file format that a unsubscription has occurred,
     *	    to rule number uRuleNumber, for stream uStreamNumber.
     */
    STDMETHOD(Unsubscribe)	(THIS_
				UINT16	uStreamNumber,
				UINT16	uRuleNumber) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAASMStream
 *
 *  Purpose:
 *	This interface is implemented by the client core.  Top level clients
 *	renderers, etc can query for this interface off of IRMAStream.  This
 *	interface allows you to subscribe and unsubscribe to certain rules,
 *	and it also allows you to add a advise sink for these events. 
 *
 *  IID_IRMAASMStream:
 *
 *	{00001502-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAASMStream, 0x00001502, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
				 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAASMStream

DECLARE_INTERFACE_(IRMAASMStream, IUnknown)
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
     * IRMAASMStream methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAASMStream::AddASMStreamSink
     *	Purpose:
     *	    Add an advise sink for getting subscribe and unsubscribe
     *	    notifications.
     */
    STDMETHOD(AddStreamSink)	(THIS_
				IRMAASMStreamSink*	pASMStreamSink) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAASMStream::RemoveStreamSink
     *	Purpose:
     *	    Remove an advise sink for getting subscribe and unsubscribe
     *	    notifications.
     */
    STDMETHOD(RemoveStreamSink)	(THIS_
				IRMAASMStreamSink*	pASMStreamSink) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAASMStream::Subscribe
     *	Purpose:
     *	    Called by renderers and possibly even top level clients to
     *	    inform the core to subscribe to a particular rule number for
     *	    this stream.
     */
    STDMETHOD(Subscribe)	(THIS_
				UINT16	uRuleNumber) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAASMStream::Unsubscribe
     *	Purpose:
     *	    Called by renderers and possibly even top level clients to
     *	    inform the core to unsubscribe to a particular rule number for
     *	    this stream.
     */
    STDMETHOD (Unsubscribe)	(THIS_
				UINT16	uRuleNumber) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAASMStreamSink
 *
 *  Purpose:
 *	This is a advise sink for getting notification about subscriptions
 *	and unsubscriptions for a stream.
 *
 *  IID_IRMAASMStream:
 *
 *	{00001503-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAASMStreamSink, 0x00001503, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
				 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAASMStreamSink

DECLARE_INTERFACE_(IRMAASMStreamSink, IUnknown)
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
     * IRMAASMStreamSink methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAASMStreamSink::OnSubscribe
     *	Purpose:
     *	    Called to inform you that a subscribe has occurred.
     */
    STDMETHOD (OnSubscribe)	(THIS_
				UINT16	uRuleNumber) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAASMStreamSink::OnUnsubscribe
     *	Purpose:
     *	    Called to inform you that a unsubscribe has occurred.
     */
    STDMETHOD (OnUnsubscribe)	(THIS_
				UINT16	uRuleNumber) PURE;
};

#endif /*_RMAASM_H_*/
