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
 *  Client Advise Sink Interfaces
 *
 */

#ifndef _RMACLSNK_H_
#define _RMACLSNK_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE   IRMAClientAdviseSink       IRMAClientAdviseSink;

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAClientAdviseSink
 * 
 *  Purpose:
 * 
 *	Interface supplied by client to core to receive notifications of
 *	status changes.
 * 
 *  IID_IRMAClientAdviseSink:
 * 
 *	{00000B00-0901-11d1-8B06-00A024406D59}
 * 
 */

DEFINE_GUID(IID_IRMAClientAdviseSink, 0x00000B00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAClientAdviseSink

DECLARE_INTERFACE_(IRMAClientAdviseSink, IUnknown)
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
     *	IRMAClientAdviseSink methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnPosLength
     *	Purpose:
     *	    Called to advise the client that the position or length of the
     *	    current playback context has changed.
     */
    STDMETHOD(OnPosLength)		(THIS_
					UINT32	  ulPosition,
					UINT32	  ulLength) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnPresentationOpened
     *	Purpose:
     *	    Called to advise the client a presentation has been opened.
     */
    STDMETHOD(OnPresentationOpened)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnPresentationClosed
     *	Purpose:
     *	    Called to advise the client a presentation has been closed.
     */
    STDMETHOD(OnPresentationClosed)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnStatisticsChanged
     *	Purpose:
     *	    Called to advise the client that the presentation statistics
     *	    have changed. 
     */
    STDMETHOD(OnStatisticsChanged)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnPreSeek
     *	Purpose:
     *	    Called by client engine to inform the client that a seek is
     *	    about to occur. The render is informed the last time for the 
     *	    stream's time line before the seek, as well as the first new
     *	    time for the stream's time line after the seek will be completed.
     *
     */
    STDMETHOD(OnPreSeek)	(THIS_
				ULONG32		    ulOldTime,
				ULONG32		    ulNewTime) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnPostSeek
     *	Purpose:
     *	    Called by client engine to inform the client that a seek has
     *	    just occurred. The render is informed the last time for the 
     *	    stream's time line before the seek, as well as the first new
     *	    time for the stream's time line after the seek.
     *
     */
    STDMETHOD(OnPostSeek)	(THIS_
				ULONG32		    ulOldTime,
				ULONG32		    ulNewTime) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnStop
     *	Purpose:
     *	    Called by client engine to inform the client that a stop has
     *	    just occurred. 
     *
     */
    STDMETHOD(OnStop)		(THIS) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnPause
     *	Purpose:
     *	    Called by client engine to inform the client that a pause has
     *	    just occurred. The render is informed the last time for the 
     *	    stream's time line before the pause.
     *
     */
    STDMETHOD(OnPause)		(THIS_
				ULONG32		    ulTime) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnBegin
     *	Purpose:
     *	    Called by client engine to inform the client that a begin or
     *	    resume has just occurred. The render is informed the first time 
     *	    for the stream's time line after the resume.
     *
     */
    STDMETHOD(OnBegin)		(THIS_
				ULONG32		    ulTime) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnBuffering
     *	Purpose:
     *	    Called by client engine to inform the client that buffering
     *	    of data is occuring. The render is informed of the reason for
     *	    the buffering (start-up of stream, seek has occurred, network
     *	    congestion, etc.), as well as percentage complete of the 
     *	    buffering process.
     *
     */
    STDMETHOD(OnBuffering)	(THIS_
				ULONG32		    ulFlags,
				UINT16		    unPercentComplete) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnContacting
     *	Purpose:
     *	    Called by client engine to inform the client is contacting
     *	    hosts(s).
     *
     */
    STDMETHOD(OnContacting)	(THIS_
				 const char*	pHostName) PURE;
};

#endif /* _RMACLSNK_H_ */
