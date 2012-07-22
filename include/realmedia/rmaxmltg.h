/****************************************************************************
 * 
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved..
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary 
 *  information of Progressive Networks, Inc, and is licensed
 *  subject to restrictions on use and distribution.
 *
 *
 *  RealMedia Architecture XMLTag Object Interfaces.
 *
 */


#ifndef _RMAXMLTG_H
#define _RMAXMLTG_H


typedef _INTERFACE	IRMAXMLTagObject		IRMAXMLTagObject;
typedef _INTERFACE	IRMAXMLTagObjectResponse IRMAXMLTagObjectResponse;

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAXMLTagObjectResponse
 * 
 *  IID_IRMAXMLTagObjectResponse:
 * 
 *	{00002C02-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAXMLTagObjectResponse, 0x00002C02, 0x901, 0x11d1, 0x8b, 0x6,
	    0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAXMLTagObjectResponse

DECLARE_INTERFACE_(IRMAXMLTagObjectResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /************************************************************************
     *  IRMAXMLTagObjectResponse
     */
    STDMETHOD(OnTagDone) (THIS_ UINT32 ulInstance, IRMABuffer* pTag) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAXMLTagHandler
 * 
 *  IID_IRMAXMLTagHandler:
 * 
 *	{00002C00-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAXMLTagHandler, 0x00002C03, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
	    0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAXMLTagHandler

DECLARE_INTERFACE_(IRMAXMLTagHandler, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /************************************************************************
     *  IRMAXMLTagHandler
     */
    STDMETHOD(InitTagHandler) (THIS_ IRMAValues* pOptions) PURE;
    STDMETHOD(CreateTagObject) (THIS_ IRMAXMLTagObject**  ppObj) PURE;
    STDMETHOD(GetTagHandlerInfo) (THIS_
			    REF(const char*) /*OUT*/ pShortName,
			    REF(const char**) /*OUT*/ pTag,
			    REF(BOOL)	    /*OUT*/ bCanHandleAsync) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAXMLTagObject
 * 
 *  IID_IRMAXMLTagObject:
 * 
 *	{00002C01-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAXMLTagObject, 0x00002C04, 0x901, 0x11d1, 0x8b, 0x6, 0x0,
	    0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAXMLTagObject

DECLARE_INTERFACE_(IRMAXMLTagObject, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;

    /************************************************************************
     *  IRMAXMLTagObject
     */
    STDMETHOD(InitTagObject) (IRMAXMLTagObjectResponse* pResp) PURE;
    STDMETHOD(OnTag) (THIS_ UINT32 ulInstance, IRMABuffer* pTag) PURE;
    STDMETHOD(Close) (THIS) PURE;

};



#endif
