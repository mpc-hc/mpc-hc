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
 *  Pending Status interfaces
 *
 */

#ifndef _RMAPENDS_H_
#define _RMAPENDS_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE	IUnknown			IUnknown;
typedef _INTERFACE	IRMAPendingStatus		IRMAPendingStatus;


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAPendingStatus
 *
 *  Purpose:
 *
 *	Interface to get the current pending status from an object
 *
 *  IRMAPendingStatus:
 *
 *	{00001100-0901-11d1-8B06-00A024406D59}
 */

DEFINE_GUID(IID_IRMAPendingStatus, 0x00001100, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPendingStatus

#define RMA_STATUS_INITIALIZING	0x01
#define RMA_STATUS_BUFFERING	0x02
#define RMA_STATUS_CONTACTING	0x03
#define RMA_STATUS_READY	0x04

DECLARE_INTERFACE_(IRMAPendingStatus, IUnknown)
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
     * IRMAPendingStatus methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAPendingStatus::GetStatus
     *	Purpose:
     *	    Called by the user to get the current pending status from an object
     */
    STDMETHOD(GetStatus)	(THIS_
				REF(UINT16) uStatusCode, 
				REF(IRMABuffer*) pStatusDesc, 
				REF(UINT16) ulPercentDone) PURE;
};

#endif /* _RMAPENDS_H_ */
