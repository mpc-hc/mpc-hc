/****************************************************************************
 * 
 *  Copyright (C) 1995-2000 RealNetworks, Inc. All rights reserved..
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary 
 *  information of Progressive Networks, Inc, and is licensed
 *  subject to restrictions on use and distribution.
 *
 *
 *  RealMedia Architecture Stream Data Conversion Interfaces.
 *
 */


#ifndef _RMADTCVT_H
#define _RMADTCVT_H

typedef _INTERFACE	IRMADataConvertSystemObject
						IRMADataConvertSystemObject;
typedef _INTERFACE	IRMADataConvert               IRMADataConvert;
typedef _INTERFACE	IRMADataConvertResponse       IRMADataConvertResponse;
typedef _INTERFACE	IRMADataRevert                IRMADataRevert;
typedef _INTERFACE	IRMADataRevertResponse        IRMADataRevertResponse;

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMADataConvertSystemObject
 *
 *  Purpose:
 *
 *	Object that allows Controller to communicate with a specific
 *	Data Convert plugin session (similar to IRMAFileSystemObject)
 *
 *  Implemented by:
 *
 *	Server side plugin.
 * 
 *  IID_IMADataConvertSystemObject:
 * 
 *	{00003900-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMADataConvertSystemObject,
    0x00003900, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMADataConvertSystemObject

DECLARE_INTERFACE_(IRMADataConvertSystemObject, IUnknown)
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
     * IRMADataConvertSystemObject
     */
     
    /***********************************************************************
     * Method: GetDataConvertInfo
     *
     * Purpose:
     *	Returns information needed for proper instantiation of data
     *  convert plugin.  pShortName should be a short, human readable
     *  name in the form of "company-dcname".  For example:
     *  pShortName = "rn-dataconvert"
     */
    STDMETHOD(GetDataConvertInfo) (THIS_ REF(const char*) pShortName) PURE;
    
    /***********************************************************************
     * Method: InitDataConvertSystem
     *
     * Purpose:
     *	Pass in options from config file from under DataConvertMount
     *  for this specific plugin.
     */
    STDMETHOD(InitDataConvertSystem) (THIS_ IRMAValues* pOptions) PURE;
    
    /***********************************************************************
     * Method: CreateDataConvert
     *
     * Purpose:
     *	Purpose:
     *	 System is requesting an IRMADataConvert object for this mount
     *   point.
     */
    STDMETHOD(CreateDataConvert) (THIS_ IUnknown** /*OUT*/ ppConvObj) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMADataConvert
 *
 *  Purpose:
 *
 *	Per connection object to handle the actual data and header
 *	conversions.
 *
 *  Implemented by:
 *
 *	Server side plugin.
 * 
 *  IID_IMADataConvert:
 * 
 *	{00003901-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMADataConvert,
    0x00003901, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMADataConvert

DECLARE_INTERFACE_(IRMADataConvert, IUnknown)
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
     *  IRMADataConvert
     */
    
    /*
     * NOTE: for each of ConvertFileHeader, ConvertStreamHeader,
     *    ConvertData, you can call the appropriate done method on
     *    the response object with a result of PNR_OK and a NULL buffer
     *    and the system will use the original header/packet.  Do this
     *    if you aren't going to change things in the header/packet.
     */
    /************************************************************************
     * Method: DataConvertInit
     *
     *Purpose:
     *	Basic initialization, mainly just to pass in response object.
     */
    STDMETHOD(DataConvertInit) (THIS_ IRMADataConvertResponse* pResponse) PURE;

    /************************************************************************
     * Method: ConvertFileHeader
     *
     * Purpose:
     *	Pass in file headers for data conversion.
     */
    STDMETHOD(ConvertFileHeader) (THIS_ IRMAValues* pFileHeader) PURE;
    
    /************************************************************************
     * Method: ConvertStreamHeader
     *	
     * Purpose:
     *	Pass in stream headers for data conversion.
     */
    STDMETHOD(ConvertStreamHeader) (THIS_ IRMAValues* pStreamHeader) PURE;
    
    /************************************************************************
     * Method: GetConversionMimeType
     *
     * Purpose:
     *	Tell the server what converstion type you are using for the
     *	session.
     */
    STDMETHOD(GetConversionMimeType)
	(THIS_ REF(const char*) pConversionType) PURE;

    /************************************************************************
     * Method: ConvertData
     *
     * Purpose:
     *	Pass in data to be converted.
     */
    STDMETHOD(ConvertData) (THIS_ IRMAPacket* pPacket) PURE;
    
    /************************************************************************
     * Method: ControlBufferReady
     *
     * Purpose:
     *  Pass in a control channel buffer sent from the IRMADataRevert
     *  on the other side (player).
     */
    STDMETHOD(ControlBufferReady) (THIS_ IRMABuffer* pBuffer) PURE;
    
    /************************************************************************
     * Method: SetMulticastTransportConverter
     *
     * Purpose:
     *  In this case the IRMADataConvert is only handling the header
     *  conversions per player and this call is handing you the data
     *  converter which is doing the data.  This will a different
     *  instance of the same object.
     */
    STDMETHOD(SetMulticastTransportConverter) (THIS_
	    IRMADataConvert* pConverter) PURE;
    
    
    /************************************************************************
     * Method: AddMulticastControlConverter
     *
     * Purpose:
     *  In this case the IRMADataConvert is only handling the data 
     *  conversions for all of the players (but only once because it's
     *  multicast).  This call is handing you one of a possible many
     *  IRMADataConvert objects which will be handling the header
     *  conversions.
     */
    STDMETHOD(AddMulticastControlConverter) (THIS_
	    IRMADataConvert* pConverter) PURE;
    
    /************************************************************************
     * Method: Done
     *
     * Purpose:
     *  Let IRMADataConvert know that it is done. This is mainly to clear
     *  circular refs between multicast transport and controllers.
     */
    STDMETHOD(Done)			(THIS) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMADataConvertResponse
 *
 *  Purpose:
 *
 *	Response object for IRMADataConvert.
 *
 *  Implemented by:
 *
 *	Server Core.
 * 
 *  IID_IMADataConvertResponse:
 * 
 *	{00003902-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMADataConvertResponse,
    0x00003902, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMADataConvertResponse

DECLARE_INTERFACE_(IRMADataConvertResponse, IUnknown)
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
     *  IRMADataConvertResponse
     */

    /*
     * NOTE: for each of ConvertFileHeader, ConvertStreamHeader,
     *    ConvertData, you can call the appropriate done method on
     *    the response object with a result of PNR_OK and a NULL buffer
     *    and the system will use the original header/packet.  Do this
     *    if you aren't going to change things in the header/packet.
     */
    /************************************************************************
     * Method: DataConvertInitDone
     *
     * Purpose:
     * 	Async notification that the IRMADataConvert is done with
     *  intialization.
     */
    STDMETHOD(DataConvertInitDone) (THIS_ PN_RESULT status) PURE;
    
    /************************************************************************
     * Method: ConvertedFileHeaderReady
     *
     * Purpose:
     *	Async notification that the IRMADataCovert is done converting the
     *  file header.
     */
    STDMETHOD(ConvertedFileHeaderReady) (THIS_
			    PN_RESULT status, IRMAValues* pFileHeader) PURE;
    
    /************************************************************************
     * Method: ConvertedStreamHeaderReady
     *
     * Purpose:
     *	Async notification that the IRMADataConvert is done converting the
     *	stream header.
     */
    STDMETHOD(ConvertedStreamHeaderReady) (THIS_
			    PN_RESULT status, IRMAValues* pStreamHeader) PURE;

    /************************************************************************
     * Method: ConvertedDataReady
     *
     * Purpose:
     *	Async notification that the IRMADataConvert is done converting
     *  the stream data packet.
     */
    STDMETHOD(ConvertedDataReady) (THIS_ PN_RESULT status,
	    				IRMAPacket* pPacket) PURE;

    /************************************************************************
     * Method: SendControlBuffer
     *
     * Purpose:
     *	Provided to allow IRMADataConvert to send an arbitrary buffer
     *  to the IRMADataRevert on the other side (player).
     */
    STDMETHOD(SendControlBuffer) (THIS_ IRMABuffer* pBuffer) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMADataRevert
 * 
 *  Purpose:
 *
 *	Revert data coming from coresponding IRMADataConvert on server.
 *
 *  Implemented by:
 *
 *	Player side plugin.
 *
 *  IID_IMADataRevert:
 * 
 *	{00003903-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMADataRevert,
    0x00003903, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMADataRevert

DECLARE_INTERFACE_(IRMADataRevert, IUnknown)
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
     *  IRMADataRevert
     */

    /************************************************************************
     * Method: DataRevertInit
     *
     * Purpose:
     *  Basic inialization of IRMADataRevert.  Mainly just to pass in
     *  response object.
     */
    STDMETHOD(DataRevertInit) (THIS_ IRMADataRevertResponse* pResponse) PURE;

    /************************************************************************
     * Method: GetDataRevertInfo
     *
     * Purpose:
     *	Allow IRMADataRevert to notify player core about which data
     *  conversion mime types it is willing to handle.
     */
    STDMETHOD(GetDataRevertInfo) (THIS_ REF(const char**)
					ppConversionMimeTypes) PURE;
    
    /************************************************************************
     * Method: RevertFileHeader
     *
     * Purpose:
     *	Pass in converted FileHeader to allow IRMADataRevert to revert
     *  the header.
     */
    STDMETHOD(RevertFileHeader)	(THIS_ IRMAValues* pFileHeader) PURE;
    
    /************************************************************************
     * Method: RevertStreamHeader
     *
     * Purpose:
     * 	Pass in converted StreamHeader to allow IRMADataRevert to revert
     *  the header.
     */
    STDMETHOD(RevertStreamHeader)(THIS_ IRMAValues* pStreamHeader) PURE;
    
    /************************************************************************
     * Method: RevertData
     *
     * Purpose:
     *	Pass in converted stream data to allow IRMADataRevert to 
     *  revert the data.
     */
    STDMETHOD(RevertData) (THIS_ IRMAPacket* pPacket) PURE;
    
    /************************************************************************
     * Method: ControlBufferReady
     *
     * Purpose:
     *	Pass in control channel buffer received from corresponding
     *  IRMADataConvert on server side.
     */
    STDMETHOD(ControlBufferReady) (THIS_ IRMABuffer* pBuffer) PURE;

};

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMADataRevertResponse
 * 
 *  Purpose:
 *
 *	Response ojbect for IRMADataRevert.
 *
 *  Implemented by:
 *
 *	Player core.
 *
 *  IID_IMADataRevertResponse:
 * 
 *	{00003904-0901-11d1-8B06-00A024406D59}
 * 
 */
DEFINE_GUID(IID_IRMADataRevertResponse,
    0x00003904, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMADataRevertResponse

DECLARE_INTERFACE_(IRMADataRevertResponse, IUnknown)
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
     *  IRMADataRevertResponse
     */

    /************************************************************************
     * Method: DataRevertInitDone
     *
     * Purpose:
     *	Async notification that the IRMADataRevert is done intializing
     *  and can begin processing headers.
     */
    STDMETHOD(DataRevertInitDone) (THIS_ PN_RESULT status) PURE;
    
    /************************************************************************
     * Method: RevertedFileHeaderReady
     *
     * Purpose:
     *	Async notification that the IRMADataRevert is done reverting the
     *  file headers.
     */
    STDMETHOD(RevertedFileHeaderReady)	(THIS_ 
	    			PN_RESULT status, IRMAValues* pHeader) PURE;
    
    /************************************************************************
     * Method: RevertedStreamHeaderReady
     *
     * Purpose:
     * 	Async notification that the IRMADataRevert is done reverting the
     *  stream headers.
     */
    STDMETHOD(RevertedStreamHeaderReady) (THIS_
				PN_RESULT status, IRMAValues* pHeader) PURE;

    /************************************************************************
     * Method: RevertedDataReady
     *
     * Purpose:
     *	Async notification that the IRMADataRevert is done reverting the
     *  stream data.
     */
    STDMETHOD(RevertedDataReady) (THIS_ PN_RESULT status,
	    				IRMAPacket* pPacket) PURE;

    /************************************************************************
     * Method: SendControlBuffer
     *
     * Purpose:
     *	Provided to allow IRMADataRevert to send an arbitrary control
     *  buffer to the IRMADataConvert on the other side (server).
     */
    STDMETHOD(SendControlBuffer) (THIS_ IRMABuffer* pBuffer) PURE;

};


#endif
