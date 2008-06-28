/****************************************************************************
 * 
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
 *  RealMedia Architecture AutoConfiguration interfaces
 *
 */

#ifndef _RMAACONF_H_
#define _RMAACONF_H_

/*
 * Forward declarations of some interfaces defined here-in.
 */
typedef _INTERFACE	IRMAAutoConfig			IRMAAutoConfig;
typedef _INTERFACE  	IRMAAutoConfigResponse		IRMAAutoConfigResponse;

#define RMA_TRANSPORT_MULTICAST     0
#define RMA_TRANSPORT_UDP	    1
#define RMA_TRANSPORT_TCP	    2
#define RMA_TRANSPORT_HTTP	    3

/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAAutoConfig
 *
 *  Purpose:
 *	This interface allows the auto-configuration of the protocol used by
 *	the client core.
 *
 *  IID_IRMAAutoConfig:
 *
 *	{00002700-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAAutoConfig, 0x00002700, 0x901, 0x11d1, 0x8b, 0x6, 
				0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAutoConfig

DECLARE_INTERFACE_(IRMAAutoConfig, IUnknown)
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
     *	IRMAAutoConfig methods
     */
    /************************************************************************
     *	Method:
     *	    IRMAAutoConfig::Init
     *	Purpose:
     *      Shutdown the configuration process.
     *
     */
    STDMETHOD(Init)			(THIS_
					IRMAAutoConfigResponse* pResponse,
					const char* pPNAURL,
					const char* pRTSPURL) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAutoConfig::Close
     *	Purpose:
     *      Shutdown the configuration process.
     *
     */
    STDMETHOD(Close)			(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAutoConfig::Abort
     *	Purpose:
     *      Abort the configuration process.
     *
     */
    STDMETHOD(Abort)			(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAutoConfig::DoAutoConfig
     *	Purpose:
     *      Start the auto-configuration Process.
     *
     */
    STDMETHOD(DoAutoConfig)		(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAAutoConfigResponse
 *
 *  Purpose:
 *	Response interface for IRMAAutoConfig.
 *
 *  IID_IRMAAutoConfig:
 *
 *	{00002701-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAAutoConfigResponse, 0x00002701, 0x901, 0x11d1, 0x8b, 
				0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAAutoConfigResponse

DECLARE_INTERFACE_(IRMAAutoConfigResponse, IUnknown)
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
     *	IRMAAutoConfigResponse methods
     */
    /************************************************************************
     *	Method:
     *	    IRMAAutoConfigResponse::OnBegin
     *	Purpose:
     *      Notification for start of auto-configure process
     *
     */
    STDMETHOD(OnBegin)			(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAutoConfigResponse::OnProgress
     *	Purpose:
     *      Notification for progress of auto-configure process
     *
     */
    STDMETHOD(OnProgress)	    (THIS_
				    UINT32	ulProgress,
				    UINT32	ulProtocolID,
				    const char* pProtocolDescription) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAAutoConfigResponse::OnComplete
     *	Purpose:
     *      Notification for completion of auto-configure process
     *
     */
    STDMETHOD(OnComplete)		(THIS_
					PN_RESULT   PNAResult,
					UINT32	    ulPNAProtocolID,
					PN_RESULT   RTSPResult,
					UINT32	    ulRTSPProtocolID) PURE;
};


#endif /* _RMAACONF_H_ */
