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
 *  RealMedia Architecture Selective Record interface
 *
 */

#ifndef _RMAPHOOK_H_
#define _RMAPHOOK_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE	IRMAPacket			IRMAPacket;
typedef _INTERFACE	IRMAPacketHook			IRMAPacketHook;
typedef _INTERFACE	IRMAPacketHookManager		IRMAPacketHookManager;
typedef _INTERFACE	IRMAPacketHookHelper		IRMAPacketHookHelper;
typedef _INTERFACE	IRMAPacketHookHelperResponse    IRMAPacketHookHelperResponse;

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAPacketHook
 *
 *  Purpose:
 *
 *	Interface implemented by the top level client to support selective
 *	record
 *
 *  IID_IRMAPacketHook:
 *
 *	{00002000-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAPacketHook, 0x00002000, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

DECLARE_INTERFACE_(IRMAPacketHook, IUnknown)
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
     * IRMAPacketHook methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAPacketHook::OnStart
     *	Purpose:
     *	    Called by the core to notify the start of this packet hook session
     */
    STDMETHOD(OnStart)		(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPacketHook::OnEnd
     *	Purpose:
     *	    Called by the core to notify the end of this packet hook session
     */
    STDMETHOD(OnEnd)		(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPacketHook::OnFileHeader
     *	Purpose:
     *	    Called by the core to send file header information
     *
     */
    STDMETHOD(OnFileHeader)	(THIS_
				IRMAValues* pValues) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPacketHook::OnStreamHeader
     *	Purpose:
     *	    Called by the core to send stream header information	
     *
     */
    STDMETHOD(OnStreamHeader)	(THIS_
				IRMAValues* pValues) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPacketHook:OnPacket
     *	Purpose:
     *	    Called by the core to send packet information.
     *
     */
    STDMETHOD(OnPacket)		(THIS_
				IRMAPacket* pPacket) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAPacketHookManager
 *
 *  Purpose:
 *
 *	Interface to the selective record
 *
 *  IID_IRMAPacketHookManager
 *
 *	{00002001-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAPacketHookManager, 0x00002001, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPacketHookManager

DECLARE_INTERFACE_(IRMAPacketHookManager, IUnknown)
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
     * IRMAPacketHookManager methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAPacketHookManager::InitHook
     *	Purpose:
     *	    called by the top level client to pass the IRMAPacketHook object
     */
    STDMETHOD(InitHook)		(THIS_
				IRMAPacketHook* pPacketHook) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPacketHookManager::CloseHook
     *	Purpose:
     *	    called by the top level client to close the hook connection
     */
    STDMETHOD(CloseHook)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPacketHookManager::StartHook
     *	Purpose:
     *	    called by the top level client to start recording
     */
    STDMETHOD(StartHook)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPacketHookManager::StopHook
     *	Purpose:
     *	    called by the top level client to stop recording
     */
    STDMETHOD(StopHook)		(THIS) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPacketHookHelper
 * 
 *  Purpose:
 * 
 *	provide methods to prepare the packet for recording and send back the core
 * 
 *  IID_IRMAPacketHookHelper:
 * 
 *	{00002002-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAPacketHookHelper, 0x00002002, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
		0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPacketHookHelper

DECLARE_INTERFACE_(IRMAPacketHookHelper, IUnknown)
{
    /*
     * IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /*
     * IRMAPacketHookHelper methods
     */

    /******************************************************************
     * Method:
     *     IRMAPacketHookHelper::StartHook
     *
     * Purpose:
     *	   tell the renderer to start sending the record packets
     *
     */
    STDMETHOD(StartHook)		(THIS_
					ULONG32	ulStreamNumber,
					ULONG32	ulTimeOffset,
					IRMAPacketHookHelperResponse* pPacketHookHelperResponse) PURE;


    /******************************************************************
     * Method:
     *    IRMAPacketHookHelper::StopHook
     *
     * Purpose:
     *    tell the renderer to stop sending the record packets
     */
    STDMETHOD(StopHook)			(THIS) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAPacketHookHelperResponse
 *
 *  Purpose:
 *
 *	Response interface to the IRMAPacketHookHelper at renderer
 *
 *  IID_IRMAPacketHookHelperResponse
 *
 *	{00002003-0901-11d1-8B06-00A024406D59}
 */
DEFINE_GUID(IID_IRMAPacketHookHelperResponse, 0x00002003, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPacketHookHelperResponse

DECLARE_INTERFACE_(IRMAPacketHookHelperResponse, IUnknown)
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
     * IRMAPacketHookHelperResponse methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAPacketHookHelperResponse::OnPacket
     *	Purpose:
     *	    called by the renderer to pass the packet for recording
     */
    STDMETHOD(OnPacket)		(THIS_
				IRMAPacket* pPacket) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPacketHookManager::OnEndOfPackets
     *	Purpose:
     *	    called by the renderer to notify the end of this stream
     */
    STDMETHOD(OnEndOfPackets)	(THIS) PURE;
};

#endif /* _RMAPHOOK_H_ */
