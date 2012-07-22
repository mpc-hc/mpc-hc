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
 *  RealMedia Architecture Broadcast Format Plug-in Interfaces.
 *
 */

#ifndef _RMAFORMT_H_
#define _RMAFORMT_H_

#include "rmafiles.h"

/*
 * Forward declarations of some interfaces defined here-in.
 */
typedef _INTERFACE	IRMAFileFormatObject	    IRMAFileFormatObject;
typedef _INTERFACE      IRMABroadcastFormatObject   IRMABroadcastFormatObject;
typedef _INTERFACE	IRMAFormatResponse	    IRMAFormatResponse;
typedef _INTERFACE	IRMAFileObject		    IRMAFileObject;
typedef _INTERFACE      IRMANetworkServices	    IRMANetworkServices;
typedef _INTERFACE      IRMAPacket                  IRMAPacket;
typedef _INTERFACE      IRMAValues                  IRMAValues;
typedef _INTERFACE	IRMAPacketTimeOffsetHandler IRMAPacketTimeOffsetHandler;
typedef _INTERFACE	IRMAPacketTimeOffsetHandlerResponse
						    IRMAPacketTimeOffsetHandlerResponse;
typedef _INTERFACE	IRMALiveFileFormatInfo	    IRMALiveFileFormatInfo;


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFileFormatObject
 * 
 *  Purpose:
 * 
 *	Object that allows a Controller to communicate with a specific
 *	File Format plug-in session
 * 
 *  IID_IRMAFileFormatObject:
 * 
 *	{00000F00-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFileFormatObject, 0x00000F00, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFileFormatObject

DECLARE_INTERFACE_(IRMAFileFormatObject, IUnknown)
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
     *	IRMAFileFormatObject methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAFileFormatObject::GetFileFormatInfo
     *	Purpose:
     *	    Returns information vital to the instantiation of file format 
     *	    plugins.
     */
    STDMETHOD(GetFileFormatInfo)(THIS_
				REF(const char**) /*OUT*/ pFileMimeTypes,
				REF(const char**) /*OUT*/ pFileExtensions,
				REF(const char**) /*OUT*/ pFileOpenNames
				) PURE;

    STDMETHOD(InitFileFormat)	
			(THIS_
		        IRMARequest*		/*IN*/	pRequest, 
			IRMAFormatResponse*	/*IN*/	pFormatResponse,
			IRMAFileObject*		/*IN*/  pFileObject) PURE;

    STDMETHOD(GetFileHeader)	(THIS) PURE;

    STDMETHOD(GetStreamHeader)	(THIS_
				UINT16 unStreamNumber) PURE;

    STDMETHOD(GetPacket)	(THIS_
				UINT16 unStreamNumber) PURE;

    STDMETHOD(Seek)		(THIS_
				ULONG32 ulOffset) PURE;

    STDMETHOD(Close)		(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMABroadcastFormatObject
 * 
 *  Purpose:
 * 
 *	Object that allows a Controller to communicate with a specific
 *	Broadcast Format plug-in session
 * 
 *  IID_IRMABroadcastFormatObject:
 * 
 *	{00000F01-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMABroadcastFormatObject, 0x00000F01, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMABroadcastFormatObject

DECLARE_INTERFACE_(IRMABroadcastFormatObject, IUnknown)
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
     *	IRMABroadcastFormatObject methods
     */

    /************************************************************************
     *	Method:
     *	    IRMABroadcastFormatObject::GetBroadcastFormatInfo
     *	Purpose:
     *	    Returns information vital to the instantiation of broadcast format 
     *	    plugins.
     */
    STDMETHOD(GetBroadcastFormatInfo)(THIS_
				REF(const char*) /*OUT*/ pToken) PURE;

    STDMETHOD(InitBroadcastFormat) (THIS_
				 const char*		/*IN*/	pURL, 
				 IRMAFormatResponse*	/*IN*/	pFormatResponse
				) PURE;

    STDMETHOD(GetFileHeader)	(THIS) PURE;

    STDMETHOD(GetStreamHeader)	(THIS_
				UINT16 unStreamNumber) PURE;

    STDMETHOD(StartPackets)	(THIS_
				UINT16 unStreamNumber) PURE;

    STDMETHOD(StopPackets)	(THIS_
				UINT16 unStreamNumber) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAFormatResponse
 * 
 *  Purpose:
 * 
 *	Object that allows a specific File Format Object to communicate 
 *	with its user
 * 
 *  IID_IRMAFormatResponse:
 * 
 *	{00000F02-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAFormatResponse, 0x00000F02, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAFormatResponse

DECLARE_INTERFACE_(IRMAFormatResponse, IUnknown)
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
     *	IRMAFormatResponse methods
     */
    STDMETHOD(InitDone)			(THIS_
					PN_RESULT	status) PURE;

    STDMETHOD(PacketReady)		(THIS_
					PN_RESULT	status,
					IRMAPacket*	pPacket) PURE;

    STDMETHOD(SeekDone)			(THIS_
					PN_RESULT	status) PURE;

    STDMETHOD(FileHeaderReady)		(THIS_
					PN_RESULT	status,
					IRMAValues*	pHeader) PURE;

    STDMETHOD(StreamHeaderReady)	(THIS_
					PN_RESULT	status,
					IRMAValues*	pHeader) PURE;

    STDMETHOD(StreamDone)		(THIS_
					UINT16		unStreamNumber) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPacketFormat
 * 
 *  Purpose:
 * 
 *	Interface that modifies the behavior of an IRMAFileFormat by defining
 *	the packet format it will be creating.
 * 
 *  IID_IRMAPacketFormat:
 * 
 *	{00000F03-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAPacketFormat, 0x00000F03, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAPacketFormat

DECLARE_INTERFACE_(IRMAPacketFormat, IUnknown)
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
     *	IRMAPacketFormat methods
     */
    STDMETHOD(GetSupportedPacketFormats)			
    					(THIS_
					REF(const char**) pFormats) PURE;

    STDMETHOD(SetPacketFormat)		(THIS_
					const char*	pFormat) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPacketTimeOffsetHandler
 * 
 *  Purpose:
 * 
 *	Provides methods for handling the changing of a packets timestamp.
 * 
 *  IID_IRMAPacketTimeOffsetHandler:
 * 
 *	{00000F04-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAPacketTimeOffsetHandler, 0x00000F04, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);


DECLARE_INTERFACE_(IRMAPacketTimeOffsetHandler, IUnknown)
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
     * IRMAPacketTimeOffsetHandler methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAPacketTimeOffsetHandler::Init
     *	Purpose:
     *	    Initialize the IRMAPacketTimeOffsetHandler and set the response.
     *      Implementors should look up the MimeType.
     */
    STDMETHOD(Init)		(THIS_
				IRMAPacketTimeOffsetHandlerResponse* pResponse,
				IRMAValues* pHeader,
				IUnknown* pContext) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPacketTimeOffsetHandler::SetTimeOffset
     *	Purpose:
     *	    Called to set the time offset.  Uses a bool and a UINT32 instead
     *      of and INT32 so that the time offset wraps around after 47 days
     *      instead of 24.  bPlus says whether to add or subtract.
     */
    STDMETHOD(SetTimeOffset)	(THIS_
				UINT32 ulTimeOffset,
				BOOL bPlus) PURE;

    /************************************************************************
     *	Method:
     *	    IRMAPacketTimeOffsetHandler::HandlePacket
     *	Purpose:
     *	    give the IRMAPacketTimeOffsetHandler a packet to modify for the
     *      time offset.
     */
    STDMETHOD(HandlePacket)	(THIS_
				IRMAPacket* pPacket) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMAPacketTimeOffsetHandlerResponse
 * 
 *  Purpose:
 * 
 *	Provides methods for the IRMAPacketTimeOffsetHandler to respond to.
 * 
 *  IID_IRMAPacketTimeOffsetHandlerResponse:
 * 
 *	{00000F05-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMAPacketTimeOffsetHandlerResponse, 0x00000F05, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);


DECLARE_INTERFACE_(IRMAPacketTimeOffsetHandlerResponse, IUnknown)
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
     * IRMAPacketTimeOffsetHandler methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAPacketTimeOffsetHandler::PacketReady
     *	Purpose:
     *	    Called by IRMAPacketTimeOffsetHandler to pass back the packet 
     *      when it is done with it.
     */
    STDMETHOD(TimeOffsetPacketReady)	(THIS_
					IRMAPacket* pPacket) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMALiveFileFormatInfo
 * 
 *  Purpose:
 * 
 *	Provides miscellaneous information needed to transmit a live stream.
 *	Optionally implemented by the file format object.
 * 
 *  IID_IRMALiveFileFormatInfo:
 * 
 *	{00000F06-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMALiveFileFormatInfo, 0x00000F06, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMALiveFileFormatInfo

DECLARE_INTERFACE_(IRMALiveFileFormatInfo, IUnknown)
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
     *	IRMALiveFileFormatInfo methods
     */

    /************************************************************************
     *	Method:
     *	    IRMALiveFileFormatInfo::VerifyFileCompatibility
     *	Purpose:
     *	    Compares two file headers and returns PNR_OK if these two 
     *	    files can be transmitted sequentially in a single live 
     *	    presentation.
     */
    STDMETHOD(VerifyFileCompatibility)	    (THIS_
					    IRMAValues* pFileHeader1,
					    IRMAValues* pFileHeader2) PURE;

    /************************************************************************
     *	Method:
     *	    IRMALiveFileFormatInfo::VerifyStreamCompatibility
     *	Purpose:
     *	    Compares two stream headers and returns PNR_OK if these two  
     *	    streams can be transmitted sequentially in a single live 
     *	    presentation.
     */
    STDMETHOD(VerifyStreamCompatibility)    (THIS_
					    IRMAValues* pStreamHeader1,
					    IRMAValues* pStreamHeader2) PURE;

    /************************************************************************
     *	Method:
     *	    IRMALiveFileFormatInfo::IsLiveResendRequired
     *	Purpose:
     *	    Returns TRUE if this stream requires the latest packet to be
     *	    resent periodically in a live presentation.
     */
    STDMETHOD_(BOOL,IsLiveResendRequired)   (THIS_
					    UINT16 unStreamNumber) PURE;

    /************************************************************************
     *	Method:
     *	    IRMALiveFileFormatInfo::GetResendBitrate
     *	Purpose:
     *	    If periodic live resends are required for this stream, this
     *	    method returns the rate at which we should resend packets. The 
     *	    resend rate is measured in bits per second.
     */
    STDMETHOD(GetResendBitrate)		    (THIS_
					    UINT16 unStreamNumber,
					    REF(UINT32) ulBitrate) PURE;

    /************************************************************************
     *	Method:
     *	    IRMALiveFileFormatInfo::GetResendDuration
     *	Purpose:
     *	    If periodic live resends are required for this stream, this
     *	    method returns the number of milliseconds for which this packet 
     *	    should be resent.
     */
    STDMETHOD(GetResendDuration)	    (THIS_
					    IRMAPacket* pPacket,
					    REF(UINT32) ulDuration) PURE;

    /************************************************************************
     *	Method:
     *	    IRMALiveFileFormatInfo::FormResendPacket
     *	Purpose:
     *	    Forms a live resend packet based upon the original packet passed
     *	    as the first parameter. This allows the file format plugin to
     *	    make resend packets distinguishable from original packets.
     */
    STDMETHOD(FormResendPacket)		(THIS_
					IRMAPacket* pOriginalPacket,
					REF(IRMAPacket*) pResendPacket) PURE;
};




#endif  /* _RMAFORMT_H_ */
