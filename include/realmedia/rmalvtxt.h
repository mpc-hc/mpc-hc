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
 *  RealText encoder class using the SuperEncoder library.
 *
 */

#ifndef _RMALVTXT_H_
#define _RMALVTXT_H_

/* These need to be visible to rtlive and to exlvtext: */
/* For live, duration is max allowed val, or max ulong:
 */
#if !defined(MAX_ULONG32)
# define MAX_ULONG32		    0xFFFFFFFF
#endif
#if !defined(MIN_LONG32)
# define MIN_LONG32                 0xFFFFFFFF
#endif

/* These, too, need to be visible to rtlive and to exlvtext: */
#define MAX_PACKET_SIZE		    500
#define USE_DEFAULT_TYPE	    NULL
#define USE_DEFAULT_HEIGHT          MAX_ULONG32
#define USE_DEFAULT_WIDTH	    MAX_ULONG32
#define USE_DEFAULT_SCROLLRATE	    MIN_LONG32
#define USE_DEFAULT_CRAWLRATE	    MIN_LONG32
#define USE_DEFAULT_BGCOLOR	    NULL
#define USE_DEFAULT_LINKCOLOR	    NULL
#define USE_DEFAULT_LINKUNDERLINING 0x2
#define USE_DEFAULT_WORDWRAP	    0x2
#define USE_DEFAULT_DOLOOPING	    0L	    

#define USE_DEFAULT_IGNOREEXTRASPACES	FALSE


typedef _INTERFACE IUnknown			IUnknown;
typedef _INTERFACE IRMAValues			IRMAValues;
typedef _INTERFACE IRMAPacket			IRMAPacket;
typedef _INTERFACE IRMALiveText			IRMALiveText;
typedef _INTERFACE IRMALiveText2		IRMALiveText2;

STDAPI
CreateLiveText(IRMALiveText**);

typedef PN_RESULT (PNEXPORT_PTR FPRMCREATELIVETEXT)(IRMALiveText** ppLiveText);


DEFINE_GUID(IID_IRMALiveText, 	0x00001b00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef INTERFACE
#define INTERFACE IRMALiveText

DECLARE_INTERFACE_(IRMALiveText, IUnknown)
{
    /***********************************************************************/
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					 REFIID riid,
					 void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;


    /***********************************************************************/
    /*
     * IRMALiveText methods
     */

    /*
     * InitLiveText() must be called before any of the other IRMALiveText
     * methods:
     */
    STDMETHOD(InitLiveText)		(THIS_
					/*
					 * This is the computer where the
					 * RMServer is running:
					 */
					char* pAddress,
					/*
					 * This is the port number defined in
					 * the config file where the FSMount
					 * option has the following entry:
					 * { /encoder/, pn-encoder, Port=#### }
					 * where #### is the port number that
					 * encoders will use:
					 */
					UINT16 port,
					char* pUsername,
					char* pPassword,
					/*
					 * This is the "file" name that
					 * RMPlayers can use to view the
					 * stream.  For example, if your
					 * server was called "srv1" and the
					 * rtsp port in the config file was
					 * 8080, and the filename from the
					 * encoder was "foo.rtx", then a
					 * player could view the stream by
					 * opening the following location:
					 * "rtsp://srv1:8080/encoder/foo.rtx"
					 */
					char* pFilename) PURE;


    /* ******************************************************************* */
    /* The following methods are to be called prior to sending any data
     * and exist as overrides to the default attributes of a RealText
     * presentation.  Please see the section on the <WINDOW> header tag
     * in the realtext.htm in the SDK's doc directory for more information.
     * Note: with live text, you do not need a <WINDOW> tag; you must use
     * the following methods to set the stream type (e.g., "tickertape"),
     * window width, height, background color, ...etc.
     */
    
    STDMETHOD(SetType)			(THIS_
					char* szType) PURE;

    STDMETHOD(SetWindowDimensions)	(THIS_
					ULONG32 width,
					ULONG32 height) PURE;

    STDMETHOD(SetTextMotion)		(THIS_
					LONG32 scrollrate,
					LONG32 crawlrate) PURE;

    STDMETHOD(SetBackgroundColor)	(THIS_
					char* szBackgroundColor) PURE;

    STDMETHOD(SetHyperlinkInfo)		(THIS_
					BOOL underlineHyperlinks,
					char* szLinkColor) PURE;

    STDMETHOD(UseWordwrap)		(THIS_
					BOOL useWordwrap) PURE;
    /*
     * This is for "TickerTape"-type windows only; it is ignored by all
     * other types:
     */
    STDMETHOD(SetDoLooping)		(THIS_
					BOOL bDoLooping) PURE;


    /* ******************************************************************* */

    /*
     * AddData() lets you add text to the stream.  The second
     * parameter, bSendImmediately, can be used to force the encoder to
     * send all text that's been added (and not yet sent) immediately.
     * Note: if bSendImmediately is FALSE, the encoder will decide when
     * to send the text it has buffered based on the length of the text in
     * the buffer and the time since the last packet was sent:
     */
    STDMETHOD(AddData)			(THIS_
					char* szMoreData,
					BOOL bSendImmediately) PURE;
    /*
     * AddTickerItem() lets you add "tickertape" text to the stream.
     * This method prepends "<TU>" to szTickerUpperData, and
     * "<TL>" to szTickerLowerData.  Calling:
     *		AddTickerItem("ABCD", "5 1/2", TRUE)
     * is the same as calling
     *		AddData("<TU>ABCD", FALSE);
     *		AddData("<TL>5 1/2", TRUE);
     * except that the former guarantees that the "upper" and "lower"
     * items will be sent in the same packet. 
     * (Please see the comments above for AddData() for details on use of
     * the second parameter, bSendImmediately.
     */
    STDMETHOD(AddTickerItem)		(THIS_
					char* szTickerUpperData,
					char* szTickerLowerData,
					BOOL bSendImmediately) PURE;


    /* ******************************************************************* */
    /*
     * flush() tells the encoder to send everything that's in the buffer and
     * clear it:
     */
    STDMETHOD(flush)			(THIS) PURE;


    /* ******************************************************************* */
    /*
     * GetTime returns the current system time in milliseconds.  The return
     * value is a UINT32 (32-bit unsigned int):
     */
    STDMETHOD_(UINT32,GetTime)		    (THIS) PURE;


    /* ******************************************************************* */
    /*
     * This must be called on a regular basis: 
     */
    STDMETHOD(Process)			    (THIS) PURE;


    /* ******************************************************************* */
    /*
     * These methods keep track of the encoder's state and must be called
     * as they are in main.cpp in the exlvtext directory:
     */
    STDMETHOD_(BOOL,EncoderIsInitialized)   (THIS) PURE;
    STDMETHOD_(BOOL,PacketsHaveStarted)	    (THIS) PURE;
    STDMETHOD_(BOOL,EncoderIsDone)	    (THIS) PURE;
    STDMETHOD(SetEncoderDone)		    (THIS) PURE;
};


DEFINE_GUID(IID_IRMALiveText2, 	0x00001b01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef INTERFACE
#define INTERFACE IRMALiveText2

DECLARE_INTERFACE_(IRMALiveText2, IUnknown)
{
    /***********************************************************************/
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					 REFIID riid,
					 void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG,Release)		(THIS) PURE;


    /***********************************************************************/
    /*
     * IRMALiveText2 methods
     */

    /* ******************************************************************* */
    /* The following methods are to be called prior to sending any data
     * and exist as overrides to the default attributes of a RealText
     * presentation.  Please see the section on the <window> header tag
     * in the realtext.htm in the SDK's doc directory for more information.
     * Note: these are additional to the IRMALiveText methods, listed
     * above.
     */
    
    /* This allows you to give an estimate of the bit rate needed by a live
     * text stream so that the server can determine whether or not a player
     * can view the stream based on its available bandwidth  This method
     * should be called before sending any data::
     */
    STDMETHOD(DeclareAverageBitrate)	(THIS_
					ULONG32 ulAvgBitsPerSecond) PURE;

    /* As above, for DeclareAverageBitrate, this method should be called
     * before sending any data::
     */
    STDMETHOD(DeclareMaximumBitrate)	(THIS_
					ULONG32 ulMaxBitsPerSecond) PURE;

    /*This determines how multiple contiguous space characters are presented;
     * Defaults to FALSE which means that multiple spaces in non-tag text
     * would all be renderered.  If this value is TRUE, multiple contiguous
     * spaces will be treated as a single space.  This method should be
     * called before sending any data.
     */
    STDMETHOD(IgnoreExtraSpaces)	(THIS_
					BOOL bIgnoreExtraSpaces) PURE;

    /* This is for internal testing use only and does not affect the
     * presentation of a live text stream:
     */
    STDMETHOD(SetFlags)	(THIS_
					ULONG32 ulFlags) PURE;



    /* ******************************************************************* */
    /* The following methods can be called at any time while the encoder is
     * running.
     */

    /* This method tells you when the last-sent text will end.  This
     * allows you to determine when it's ok to send more text.  For example,
     * if you're streaming a presentation that's scrolling at a rate of 20
     * pixels per second in a window that's 200 pixels high and you call
     * IRMALiveText's AddData() with 100 lines of 20-point text, it will take
     * 100 seconds for all that text to scroll up and out of the window.  The
     * following method would return the current time plus 100 if you called
     * it right after calling AddData():
     */
    STDMETHOD(GetLastSentTextEndTime)	(THIS_
					ULONG32& ulLastEndTime) PURE;

    /* This method allows you to adjust the time, since the last data was
     * sent, that the encoder waits before resending "heartbeat" packets that
     * let the server know the encoder still alive and to let newly-connected
     * players know what's currently visible in the window (in case they
     * connected during a "dry" period in the stream, after the last new
     * text was sent).  Time is in milliseconds.  NOTE: this defaults to 
     * 3000 milliseconds; any number higher than this will result in longer
     * connect times for players coming in during a "dry" spell in the
     * stream, and a sufficiently high value may cause the server to
     * disconnect since it assumes the encoder has stopped if it does not
     * periodically send data.  This value's minimum is 500 milliseconds.
     */
    STDMETHOD(SetTimeBetweenDryStreamResends)	(THIS_
				ULONG32 ulMillisecBetweenResends) PURE;
};


#endif /* _RMALVTXT_H_ */
