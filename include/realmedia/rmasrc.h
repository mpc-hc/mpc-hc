/****************************************************************************
 * 
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved.
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary information of RealNetworks, Inc, 
 *  and is licensed subject to restrictions on use and distribution.
 *
 *
 *  RealMedia Architecture Packet Source Interfaces.
 *
 */

#ifndef _RMASRC_H_
#define _RMASRC_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE	IRMARawSourceObject		IRMARawSourceObject;
typedef _INTERFACE	IRMARawSinkObject		IRMARawSinkObject;
typedef _INTERFACE	IRMASourceFinderObject		IRMASourceFinderObject;
typedef _INTERFACE	IRMASourceFinderResponse	IRMASourceFinderResponse;
typedef _INTERFACE	IRMARequest			IRMARequest;

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMARawSourceObject
 * 
 *  Purpose:
 * 
 *	Object that serves packets to sinks
 * 
 *  IID_IRMARawSourceObject:
 * 
 *	{00001000-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMARawSourceObject, 0x00001000, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMARawSourceObject

DECLARE_INTERFACE_(IRMARawSourceObject, IUnknown)
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
	 *	IRMARawSourceObject methods
	 */

    /************************************************************************
     *	Method:
     *	    IRMARawSourceObject::Init
     *	Purpose:
     *	    Initializes the connection between the source and the sink
     */
    STDMETHOD(Init)		(THIS_
				IUnknown* pUnknown) PURE;

    STDMETHOD(Done)		(THIS) PURE;

    STDMETHOD(GetFileHeader)	(THIS) PURE;

    STDMETHOD(GetStreamHeader)	(THIS_
				UINT16 unStreamNumber) PURE;

    STDMETHOD(StartPackets)	(THIS_
				UINT16 unStreamNumber) PURE;

    STDMETHOD(StopPackets)	(THIS_
				UINT16 unStreamNumber) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMARawSinkObject
 * 
 *  Purpose:
 * 
 *	Object that receives raw packets from a source
 * 
 *  IID_IRMARawSinkObject:
 * 
 *	{00001001-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMARawSinkObject, 0x00001001, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMARawSinkObject

DECLARE_INTERFACE_(IRMARawSinkObject, IUnknown)
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
	 *	IRMARawSinkObject methods
	 */

    /************************************************************************
     *	Method:
     *	    IRMARawSinkObject::InitDone
     *	Purpose:
     *	    Callback after source object has initialized the connection
     */
    STDMETHOD(InitDone)			(THIS_
					PN_RESULT	status) PURE;

    STDMETHOD(FileHeaderReady)		(THIS_
					PN_RESULT	status,
					IRMAValues*	pHeader) PURE;

    STDMETHOD(StreamHeaderReady)	(THIS_
					PN_RESULT	status,
					IRMAValues*	pHeader) PURE;

    STDMETHOD(PacketReady)		(THIS_
					PN_RESULT	status,
					IRMAPacket*	pPacket) PURE;

    STDMETHOD(StreamDone)		(THIS_
					UINT16		unStreamNumber) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMASourceFinderObject
 * 
 *  Purpose:
 * 
 *	Object that allows a sink to search for a raw packet source
 * 
 *  IID_IRMASourceFinderObject:
 * 
 *	{00001002-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMASourceFinderObject, 0x00001002, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#define CLSID_IRMASourceFinderObject	IID_IRMASourceFinderObject

#undef  INTERFACE
#define INTERFACE   IRMASourceFinderObject

#define CLSID_IRMASourceFinderObject IID_IRMASourceFinderObject

DECLARE_INTERFACE_(IRMASourceFinderObject, IUnknown)
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
	 *	IRMASourceFinderObject methods
	 */

    STDMETHOD(Init)		(THIS_ 
				IUnknown*	pUnknown) PURE;

    STDMETHOD(Find)		(THIS_
				IRMARequest*	pRequest) PURE;

    STDMETHOD(Done)		(THIS) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMASourceFinderResponse
 * 
 *  Purpose:
 * 
 *	Object that returns a raw packet source to a sink
 * 
 *  IID_IRMASourceFinderResponse:
 * 
 *	{00001003-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMASourceFinderResponse, 0x00001003, 0x901, 0x11d1,
            0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);


#undef  INTERFACE
#define INTERFACE   IRMASourceFinderResponse

DECLARE_INTERFACE_(IRMASourceFinderResponse, IUnknown)
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
	 *	IRMASourceFinderResponse methods
	 */

    STDMETHOD(InitDone)			(THIS_
					PN_RESULT	status) PURE;

    STDMETHOD(FindDone)			(THIS_
					PN_RESULT	status,
					IUnknown*	pUnknown) PURE;

};

#endif /* _RMASRC_H_ */
