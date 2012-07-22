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
 *  RealMedia Architecture error reporting interfaces.
 *
 */

#ifndef _RMAERROR_H_
#define _RMAERROR_H_

/*
 * Forward declarations of some interfaces defined here-in.
 */
typedef _INTERFACE       IRMABuffer	    	    IRMABuffer;
typedef _INTERFACE       IRMAErrorSinkControl	    IRMAErrorSinkControl;


/* Message Severity values */

enum {
     PNLOG_EMERG     = 0, /* A panic condition.  Server / Player will halt or
			     restart. */

     PNLOG_ALERT     = 1, /* A condition that should be corrected immediately.
			     Needs user intervention to prevent problems. */

     PNLOG_CRIT      = 2, /* Critical conditions. */

     PNLOG_ERR       = 3, /* Errors. */

     PNLOG_WARNING   = 4, /* Warning messages. */

     PNLOG_NOTICE    = 5, /* Conditions that are not error conditions, but
			     should possibly be handled specially. */

     PNLOG_INFO      = 6, /* Informational messages. */

     PNLOG_DEBUG     = 7  /* Messages that contain information normally of use
			     only when debugging a program. */
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAErrorMessages
 * 
 *  Purpose:
 * 
 *	Error, event, and status message reporting interface
 * 
 *  IID_IRMAErrorMessages:
 * 
 *	{00000800-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAErrorMessages, 0x00000800, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAErrorMessages

DECLARE_INTERFACE_(IRMAErrorMessages, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *  IRMAErrorMessages methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAErrorMessages::Report
     *	Purpose:
     *	    Call this method to report an error, event, or status message.
     *	Parameters:
     *
     *	    const UINT8	unSeverity
     *	    Type of report. This value will impact how the player, tool, or
     *	    server will react to the report. Possible values are described 
     *	    above. Depending on the error type, an error message with the 
     *	    RMA code, anda string translation of that code will be displayed. 
     *	    The error dialog includes a "more info" section that displays the
     *	    user code and string, and a link to the more info URL. In the 
     *	    server these messages are logged to the log file.
     *
     *	    const ULONG32   ulRMACode
     *	    Well known RMA error code. This will be translated to a text
     *	    representation for display in an error dialog box or log file.
     *
     *	    const ULONG32   ulUserCode
     *	    User specific error code. This will NOT be translated to a text
     *	    representation. This can be any value the caller wants, it will
     *	    be logged or displayed but not interpretted.
     *
     *	    const char*	    pUserString
     *	    User specific error string. This will NOT be translated or 
     *	    modified. This can be any value the caller wants, it will
     *	    be logged or displayed but not interpretted.
     *
     *	    const char*	    pMoreInfoURL
     *	    User specific more info URL string.
     *
     */
    STDMETHOD(Report)		(THIS_
				const UINT8	unSeverity,  
				PN_RESULT	ulRMACode,
				const ULONG32	ulUserCode,
				const char*	pUserString,
				const char*	pMoreInfoURL) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAErrorMessages::GetErrorText
     *	Purpose:
     *	    Call this method to get the text description of a RMA error code.
     *	Parameters:
     *	    PN_RESULT ulRMACode (A RMA error code)
     *  Return Value:
     *	    IRMABuffer* containing error text.
     */
    STDMETHOD_(IRMABuffer*, GetErrorText)	(THIS_
						PN_RESULT	ulRMACode) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAErrorSink
 * 
 *  Purpose:
 * 
 *	Error Sink Interface
 * 
 *  IID_IRMAErrorSink:
 * 
 *	{00000801-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAErrorSink, 0x00000801, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAErrorSink

DECLARE_INTERFACE_(IRMAErrorSink, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *  IRMAErrorSink methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAErrorSink::ErrorOccurred
     *	Purpose:
     *	    After you have registered your error sink with an
     *	    IRMAErrorSinkControl (either in the server or player core) this
     *	    method will be called to report an error, event, or status message.
     *
     *	    The meaning of the arguments is exactly as described in
     *	    rmaerror.h
     */
    STDMETHOD(ErrorOccurred)	(THIS_
				const UINT8	unSeverity,  
				const ULONG32	ulRMACode,
				const ULONG32	ulUserCode,
				const char*	pUserString,
				const char*	pMoreInfoURL
				) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAErrorSinkControl
 * 
 *  Purpose:
 * 
 *	Error Sink Control Interface
 * 
 *  IID_IRMAErrorSinkControl:
 * 
 *	{00000802-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAErrorSinkControl, 0x00000802, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAErrorSinkControl


DECLARE_INTERFACE_(IRMAErrorSinkControl, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *  IRMAErrorSinkControl methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAErrorSinkControl::AddErrorSink
     *	Purpose:
     *	    Call this method to tell the sink controller to handle an error
     *	    sink.
     *
     *	    This method also allows you to set a range of severity levels which
     *	    you will receive reports for.
     *
     *      Note: You should specify an invalid range (Low = 1, High = 0 for
     *            example) if you don't want to receive any errors.
     *
     *	    The default severity range is PNLOG_EMERG to PNLOG_INFO (0-6).
     */
    STDMETHOD(AddErrorSink)	(THIS_
				IRMAErrorSink*	pErrorSink,	
                                const UINT8     unLowSeverity,
                                const UINT8     unHighSeverity) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAErrorSinkControl::AddErrorSink
     *	Purpose:
     *	    Call this method to remove an error sink.
     */
    STDMETHOD(RemoveErrorSink)	(THIS_
				IRMAErrorSink*	pErrorSink) PURE;

};

#endif /* _RMAERROR_H_ */
