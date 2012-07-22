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
 *  Simple Hyper Navigation Interfaces
 *
 */

#ifndef _RMAHYPER_H_
#define _RMAHYPER_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE   IUnknown				IUnknown;
typedef _INTERFACE   IRMAValues				IRMAValues;
typedef _INTERFACE   IRMAHyperNavigate			IRMAHyperNavigate;


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAHyperNavigate
 * 
 *  Purpose:
 * 
 *	Allows you to perform simple "Go to URL" operations.
 * 
 *  IID_IRMAHyperNavigate:
 * 
 *	{00000900-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAHyperNavigate, 0x00000900, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAHyperNavigate

DECLARE_INTERFACE_(IRMAHyperNavigate, IUnknown)
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
     *	IRMAHyperNavigate methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAHyperNavigate::GoToURL
     *	Purpose:
     *	    Performs a simple Go To URL operation.
     *	Parameters:
     *      pURL: fully qualified URL such as http://www.real.com
     *	    pTarget: target frame.  To not use a frame, set this to NULL
     */
    STDMETHOD(GoToURL)	    (THIS_
			    const char* pURL,
			    const char* pTarget) PURE;

};



#endif /* _RMAHYPER_H_ */
