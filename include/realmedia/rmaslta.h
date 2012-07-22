/****************************************************************************
 * 
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved.
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary nformation of RealNetworks, Inc, 
 *  and is licensed subject to restrictions on use and distribution.
 *
 *
 *  RealMedia Architecture Interfaces for Simulated Live Transfer Agent.
 *
 */

#ifndef _RMASLTA_H
#define _RMASLTA_H

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMASLTA
 * 
 *  Purpose:
 * 
 *	Slta that works with RMA.  Simulates a live stream from a file.
 * 
 *  IID_IRMASLTA
 * 
 *	{00000D00-b4c8-11d0-9995-00a0248da5f0}
 * 
 */
DEFINE_GUID(IID_IRMASLTA,   0x00000D00, 0xb4c8, 0x11d0, 0x99, 
			    0x95, 0x0, 0xa0, 0x24, 0x8d, 0xa5, 0xf0);

DECLARE_INTERFACE_(IRMASLTA, IUnknown)
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
     *	IRMASLTA methods
     */

    /************************************************************************
     *	Method:
     *	    IRMASLTA::Connect
     *	Purpose:
     *	    Connects the slta to a server.
     */
    STDMETHOD(Connect)	(THIS_
			    const char* host,
			    UINT16 uPort,
			    const char* username,
			    const char* passwd,
			    const char* livefile
			) PURE;

    /************************************************************************
    * Method:
    *	    IRMASLTA::SetTAC
    * Purpose:
    *	    Set the TAC info for the stream. This method MUST be called
    *	before Encode to have any effect.
    */
    STDMETHOD(SetTAC)	(THIS_
			    const char* Title,
			    const char* Author,
			    const char* Copyright) PURE;

    /************************************************************************
    *	Method:
    *	    IRMASLTA:Encode
    *	Purpose:
    *	    Start encoding the file to the server.
    */
    STDMETHOD(Encode)	
			(THIS_
			    const char* filename
			) PURE;

    /************************************************************************
    *	Method:
    *	    IRMASLTA:Disconnect
    *	Purpose:
    *	    Disconnect the slta from the server.
    */
    STDMETHOD(Disconnect)   (THIS) PURE;


    /************************************************************************
    *	Method:
    *	    IRMASLTA::SetTargetBandwidth
    *	Purpose:
    *	    Sets the target bw for rule subscription.
    */
    STDMETHOD(SetTargetBandwidth)   (THIS_
					UINT32 ulTargetBW)  PURE;


};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMASltaEvent
 * 
 *  Purpose:
 * 
 *	Allows events to be sent through an SLTA stream
 * 
 *  IID_IRMASltaEvent
 * 
 *	{00000D01-b4c8-11d0-9995-00a0248da5f0}
 * 
 */
DEFINE_GUID(IID_IRMASltaEvent,   0x00000D01, 0xb4c8, 0x11d0, 0x99, 
			    0x95, 0x0, 0xa0, 0x24, 0x8d, 0xa5, 0xf0);

/* 
 * Valid RMA event IDs.
 */

#define RMA_EVENT_TITLE             0x0000
#define RMA_EVENT_AUTHOR            0x0001
#define RMA_EVENT_COPYRIGHT         0x0002
#define RMA_EVENT_SERVER_ALERT      0x0003
#define RMA_EVENT_PROGRESS_MESSAGE  0x0004
#define RMA_EVENT_TEXT_SIZE         0x0010
#define RMA_EVENT_TEXT              0x0011
#define RMA_EVENT_TEXT_ANCHOR       0x0012
#define RMA_EVENT_BROWSER_OPEN_URL  0x0020
#define RMA_EVENT_TOPIC             0x0030
#define RMA_EVENT_EMPTY             0x0200
#define RMA_EVENT_CUSTOM_BEGIN      0x0400

DECLARE_INTERFACE_(IRMASltaEvent, IUnknown)
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
     *	IRMASltaEvent methods
     */

   /************************************************************************
    * Method:
    *	    IRMASltaEvent::SetEvent
    * Purpose:
    *	    Sends an event to the slta stream. 
    *       nEventID must be one of the valid event IDs defined above.
    */
    STDMETHOD(SetEvent)             (THIS_
                                     UINT16 nEventID,
                                     const char* szEventText) PURE;

   /************************************************************************
    * Method:
    *	    IRMASltaEvent::SetRepeatedEvent
    * Purpose:
    *	    Sets an event to be repeated every ulFrequency milliseconds.
    *       nEventID must be one of the valid event IDs defined above.
    */

    STDMETHOD(SetRepeatedEvent)     (THIS_
                                     UINT16 nEventID,
                                     const char* szEventText,
                                     UINT32 ulFrequency) PURE;
};

#endif
