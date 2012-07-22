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
 *  RealMedia Architecture Rendering Interfaces.
 *
 */

#ifndef _RMARENDR_H_
#define _RMARENDR_H_

/*
 * Forward declarations of some interfaces defined or used here-in.
 */
typedef _INTERFACE	IRMARenderer		    IRMARenderer;
typedef _INTERFACE	IRMAStream		    IRMAStream;
typedef _INTERFACE	IRMAStreamSource	    IRMAStreamSource;
typedef _INTERFACE	IRMAPlayer		    IRMAPlayer;
typedef _INTERFACE	IRMAClientEngine	    IRMAClientEngine;


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMARenderer
 *
 *  Purpose:
 *
 *	Interface implemented by all renderers. Parts of this interface are
 *	called by the client engine to provide data packets to the 
 *	individual renderers.
 *
 *  IID_IRMARenderer:
 *
 *	{00000300-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMARenderer, 0x00000300, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMARenderer

typedef ULONG32	RMA_DISPLAY_TYPE;

#define RMA_DISPLAY_NONE                    0x00000000
#define RMA_DISPLAY_WINDOW                  0x00000001
#define RMA_DISPLAY_SUPPORTS_RESIZE         0x00000002
#define RMA_DISPLAY_SUPPORTS_FULLSCREEN     0x00000004
#define RMA_DISPLAY_SUPPORTS_VIDCONTROLS    0x00000008


DECLARE_INTERFACE_(IRMARenderer, IUnknown)
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
     * IRMARenderer methods
     */

    /************************************************************************
     *	Method:
     *	    IRMARenderer::GetRendererInfo
     *	Purpose:
     *	    Returns information vital to the instantiation of rendering 
     *	    plugins.
     */
    STDMETHOD(GetRendererInfo)	(THIS_
				REF(const char**)/*OUT*/ pStreamMimeTypes,
				REF(UINT32)	 /*OUT*/ unInitialGranularity
				) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARenderer::StartStream
     *	Purpose:
     *	    Called by client engine to inform the renderer of the stream it
     *	    will be rendering. The stream interface can provide access to
     *	    its source or player. This method also provides access to the 
     *	    primary client controller interface.
     *
     */
    STDMETHOD(StartStream)	(THIS_
				IRMAStream*	    pStream,
				IRMAPlayer*	    pPlayer) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARenderer::EndStream
     *	Purpose:
     *	    Called by client engine to inform the renderer that the stream
     *	    is was rendering is closed.
     *
     */
    STDMETHOD(EndStream)	(THIS) PURE;

    /************************************************************************
     *	Method:
     *		IRMARenderer::OnHeader
     *	Purpose:
     *		Called by client engine when a header for this renderer is 
     *		available. The header will arrive before any packets.
     *
     */
    STDMETHOD(OnHeader)		(THIS_
				IRMAValues*	    pHeader) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARenderer::OnPacket
     *	Purpose:
     *	    Called by client engine when a packet for this renderer is 
     *	    due.
     *
     */
    STDMETHOD(OnPacket)		(THIS_
				IRMAPacket*	    pPacket,
				LONG32		    lTimeOffset) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARenderer::OnTimeSync
     *	Purpose:
     *	    Called by client engine to inform the renderer of the current
     *	    time relative to the streams synchronized time-line. The 
     *	    renderer should use this time value to update its display or
     *	    render it's stream data accordingly.
     *
     */
    STDMETHOD(OnTimeSync)	(THIS_
				ULONG32		    ulTime) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARenderer::OnPreSeek
     *	Purpose:
     *	    Called by client engine to inform the renderer that a seek is
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
     *	    IRMARenderer::OnPostSeek
     *	Purpose:
     *	    Called by client engine to inform the renderer that a seek has
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
     *	    IRMARenderer::OnPause
     *	Purpose:
     *	    Called by client engine to inform the renderer that a pause has
     *	    just occurred. The render is informed the last time for the 
     *	    stream's time line before the pause.
     *
     */
    STDMETHOD(OnPause)		(THIS_
				ULONG32		    ulTime) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARenderer::OnBegin
     *	Purpose:
     *	    Called by client engine to inform the renderer that a begin or
     *	    resume has just occurred. The render is informed the first time 
     *	    for the stream's time line after the resume.
     *
     */
    STDMETHOD(OnBegin)		(THIS_
				ULONG32		    ulTime) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARenderer::OnBuffering
     *	Purpose:
     *	    Called by client engine to inform the renderer that buffering
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
     *	    IRMARenderer::GetDisplayType
     *	Purpose:
     *	    Called by client engine to ask the renderer for it's preferred
     *	    display type. When layout information is not present, the 
     *	    renderer will be asked for it's prefered display type. Depending
     *	    on the display type a buffer of additional information may be 
     *	    needed. This buffer could contain information about preferred
     *	    window size.
     *
     */
    STDMETHOD(GetDisplayType)	(THIS_
				REF(RMA_DISPLAY_TYPE)	ulFlags,
				REF(IRMABuffer*)	pBuffer) PURE;

    /************************************************************************
     *	Method:
     *	    IRMARenderer::OnEndofPackets
     *	Purpose:
     *	    Called by client engine to inform the renderer that all the
     *	    packets have been delivered. However, if the user seeks before
     *	    EndStream() is called, renderer may start getting packets again
     *	    and the client engine will eventually call this function again.
     */
    STDMETHOD(OnEndofPackets)	(THIS) PURE;
};

#endif /* _RMARENDR_H_ */
