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
 *  RealMedia Architecture player creation interfaces.
 *
 */

#ifndef _RMAPSINK_H_
#define _RMAPSINK_H_

/*
 * Forward declarations of some interfaces defined here-in.
 */
typedef _INTERFACE       IRMAPlayer		    IRMAPlayer;
typedef _INTERFACE       IRMAPlayerSinkControl	    IRMAPlayerSinkControl;
typedef _INTERFACE       IRMAPlayerSinkControl	    IRMAPlayerSinkControl;


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPlayerCreationSink
 * 
 *  Purpose:
 * 
 *	PlayerCreation Sink Interface
 * 
 *  IID_IRMAPlayerCreationSink:
 * 
 *	{00002100-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAPlayerCreationSink,	0x00002100, 0x901, 0x11d1, 0x8b, 
				0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#undef  INTERFACE
#define INTERFACE   IRMAPlayerCreationSink

DECLARE_INTERFACE_(IRMAPlayerCreationSink, IUnknown)
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
     * IRMAPlayerCreationSink Methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAPlayerCreationSink::PlayerCreated
     *	Purpose:
     *	    Notification when a new player is created
     *
     */
    STDMETHOD(PlayerCreated)		(THIS_
					IRMAPlayer* pPlayer) PURE;
    
    /************************************************************************
     *	Method:
     *	    IRMAPlayerCreationSink::PlayerClosed
     *	Purpose:
     *	    Notification when an exisitng player is closed
     *
     */
    STDMETHOD(PlayerClosed)		(THIS_
					IRMAPlayer* pPlayer) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPlayerSinkControl
 * 
 *  Purpose:
 * 
 *	Player SinkControl Interface
 * 
 *  IID_IRMAPlayerSinkControl:
 * 
 *	{00002101-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPlayerSinkControl, 0x00002101, 0x901, 0x11d1, 0x8b, 
				    0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);
#undef  INTERFACE
#define INTERFACE   IRMAPlayerSinkControl

DECLARE_INTERFACE_(IRMAPlayerSinkControl, IUnknown)
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
     * IRMAPlayerSinkControl Methods
     */
    
    /************************************************************************
     *	Method:
     *	    IRMAPlayerSinkControl::AddSink
     *	Purpose:
     *	    Add a new sink
     *
     */
    STDMETHOD(AddSink)			(THIS_
					IRMAPlayerCreationSink* pSink) PURE;
    
    /************************************************************************
     *	Method:
     *	    IRMAPlayerSinkControl::RemoveSink
     *	Purpose:
     *	    Remove an exisitng sink
     *
     */
    STDMETHOD(RemoveSink)		(THIS_
					IRMAPlayerCreationSink* pSink) PURE;
};

#endif /* _RMAPSINK_H_ */
