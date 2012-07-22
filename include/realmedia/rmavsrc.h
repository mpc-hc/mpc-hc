#ifndef _RMAVSRC_H
#define _RMAVSRC_H
/****************************************************************************
 * 
 *  Copyright (C) 1999 RealNetworks, Inc. All rights reserved..
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary 
 *  information of RealNetworks, Inc, and is licensed
 *  subject to restrictions on use and distribution.
 *
 *  RealMedia Architecture FileViewSource Interfaces.
 *
 */


typedef _INTERFACE	IRMAStreamSource		IRMAStreamSource;
typedef _INTERFACE	IRMAFileObject			IRMAFileObject;

// Interfaces definded in this file
typedef _INTERFACE	IRMAFileViewSource		IRMAFileViewSource;
typedef _INTERFACE	IRMAFileViewSourceResponse	IRMAFileViewSourceResponse;
typedef _INTERFACE	IRMAViewSourceCommand		IRMAViewSourceCommand;
typedef _INTERFACE	IRMAViewSourceURLResponse	IRMAViewSourceURLResponse;




/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileViewSource
 * 
 *  IID_IRMAFileViewSource:
 * 
 *	{00003500-0901-11d1-8B06-00A024406D59}
 * 
 */

enum SOURCE_TYPE
{
    RAW_SOURCE,
    HTML_SOURCE
};

DEFINE_GUID(IID_IRMAFileViewSource, 0x00003500, 0x901, 0x11d1, 0x8b, 0x6,
	    0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileViewSource

DECLARE_INTERFACE_(IRMAFileViewSource, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_ REFIID riid, void** ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /************************************************************************
     *  IRMAFileViewSource
     */
    STDMETHOD(InitViewSource)		(THIS_
	IRMAFileObject*		    /*IN*/ pFileObject,
	IRMAFileViewSourceResponse* /*IN*/ pResp,
	SOURCE_TYPE		    /*IN*/ sourceType,
	IRMAValues*		    /*IN*/ pOptions) PURE;
    STDMETHOD(GetSource)    (THIS) PURE;
    STDMETHOD(Close)	    (THIS) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileViewSourceResponse
 * 
 *  IID_IRMAFileViewSourceResponse:
 * 
 *	{00003501-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAFileViewSourceResponse, 0x00003501, 0x901, 0x11d1, 0x8b,
	    0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileViewSourceResponse

DECLARE_INTERFACE_(IRMAFileViewSourceResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_ REFIID riid, void** ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;
    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /************************************************************************
     *  IRMAFileViewSourceResoponse
     */
    STDMETHOD(InitDone)		(THIS_ PN_RESULT status	) PURE;
    STDMETHOD(SourceReady)	(THIS_ PN_RESULT status,
	IRMABuffer* pSource ) PURE;
    STDMETHOD(CloseDone) (THIS_ PN_RESULT) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAViewSourceCommand
 * 
 *  IID_IRMAViewSourceCommand:
 * 
 *	{00003504-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAViewSourceCommand, 0x00003504, 0x901, 0x11d1, 0x8b, 0x6,
	    0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAViewSourceCommand

DECLARE_INTERFACE_(IRMAViewSourceCommand, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_ REFIID riid, void** ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /************************************************************************
     *  IRMAViewSourceCommand
     */
    STDMETHOD_(BOOL, CanViewSource)	(THIS_
					IRMAStreamSource*		pStream) PURE;
    STDMETHOD(DoViewSource)		(THIS_
					IRMAStreamSource*		pStream) PURE;
    STDMETHOD(GetViewSourceURL) (THIS_
					IRMAStreamSource*		pSource,
					IRMAViewSourceURLResponse*      pResp) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAViewSourceURLResponse
 * 
 *  IID_IRMAViewSourceURLResponse:
 * 
 *	{00003505-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAViewSourceURLResponse, 0x00003505, 0x901, 0x11d1, 0x8b, 0x6,
	    0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAViewSourceURLResponse

DECLARE_INTERFACE_(IRMAViewSourceURLResponse, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_ REFIID riid, void** ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /************************************************************************
     *  IRMAViewSourceURLResponse
     */
    STDMETHOD(ViewSourceURLReady)	(THIS_	
					const char* /*out*/ pUrl) PURE;
};


#endif
