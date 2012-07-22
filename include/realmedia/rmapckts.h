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
 *  RealMedia Architecture Packet and Header Interface.
 *
 */

#ifndef _RMAPCKTS_H_
#define _RMAPCKTS_H_

// Define IRMAUtilities

/* ASMFlags in IRMAPacket */
#define RMA_ASM_SWITCH_ON	 0x01
#define RMA_ASM_SWITCH_OFF	 0x02


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMABuffer
 *
 *  Purpose:
 *
 *	Basic opaque data storage buffer. Used in interfaces where 
 *	object ownership is best managed through COM style reference 
 *	counting.
 *
 *  IID_IRMABuffer:
 *
 *	{00001300-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMABuffer, 0x00001300, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  The IRMACommonClassFactory supports creating an instance
 *  of this object.
 */
#define CLSID_IRMABuffer IID_IRMABuffer

#undef  INTERFACE
#define INTERFACE   IRMABuffer

DECLARE_INTERFACE_(IRMABuffer, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMABuffer methods
     */
    STDMETHOD(Get)		(THIS_
				REF(UCHAR*)	pData, 
				REF(ULONG32)	ulLength) PURE;

    STDMETHOD(Set)		(THIS_
				const UCHAR*	pData, 
				ULONG32		ulLength) PURE;

    STDMETHOD(SetSize)		(THIS_
				ULONG32		ulLength) PURE;

    STDMETHOD_(ULONG32,GetSize)	(THIS) PURE;

    STDMETHOD_(UCHAR*,GetBuffer)(THIS) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAPacket
 *
 *  Purpose:
 *
 *	Basic data packet in the RealMedia system.
 *
 *  IID_IRMAPacket:
 *
 *	{00001301-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAPacket, 0x00001301, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  The IRMACommonClassFactory supports creating an instance
 *  of this object.
 */
#define CLSID_IRMAPacket IID_IRMAPacket

#undef  INTERFACE
#define INTERFACE   IRMAPacket

DECLARE_INTERFACE_(IRMAPacket, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMAPacket methods
     */
    STDMETHOD(Get)			(THIS_
					REF(IRMABuffer*)    pBuffer, 
					REF(UINT32)	    ulTime,
					REF(UINT16)	    unStreamNumber,
					REF(UINT8)	    unASMFlags,
					REF(UINT16)	    unASMRuleNumber
					) PURE;

    STDMETHOD_(IRMABuffer*,GetBuffer)	(THIS) PURE;

    STDMETHOD_(ULONG32,GetTime)		(THIS) PURE;

    STDMETHOD_(UINT16,GetStreamNumber)	(THIS) PURE;

    STDMETHOD_(UINT8,GetASMFlags)	(THIS) PURE;

    STDMETHOD_(UINT16,GetASMRuleNumber)	(THIS) PURE;

    STDMETHOD_(BOOL,IsLost)		(THIS) PURE;

    STDMETHOD(SetAsLost)		(THIS) PURE;

    STDMETHOD(Set)			(THIS_
					IRMABuffer* 	    pBuffer,
					UINT32	    	    ulTime,
					UINT16	    	    uStreamNumber,
					UINT8	    	    unASMFlags,
					UINT16	    	    unASMRuleNumber
					) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAPacket
 *
 *  Purpose:
 *
 *	RTP data packet in the RealMedia system.
 *
 *  IID_IRMARTPPacket:
 *
 *	{0169A731-1ED0-11d4-952B-00902742C923}
 *
 */
DEFINE_GUID(IID_IRMARTPPacket, 0x169a731, 0x1ed0, 0x11d4, 0x95, 0x2b, 0x0, 
	    0x90, 0x27, 0x42, 0xc9, 0x23);

/*
 *  The IRMACommonClassFactory supports creating an instance
 *  of this object.
 */
#define CLSID_IRMARTPPacket IID_IRMARTPPacket

#undef  INTERFACE
#define INTERFACE   IRMARTPPacket

DECLARE_INTERFACE_(IRMARTPPacket, IRMAPacket)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMAPacket methods
     */
    STDMETHOD(Get)			(THIS_
					REF(IRMABuffer*)    pBuffer, 
					REF(UINT32)	    ulTime,
					REF(UINT16)	    unStreamNumber,
					REF(UINT8)	    unASMFlags,
					REF(UINT16)	    unASMRuleNumber
					) PURE;

    STDMETHOD_(IRMABuffer*,GetBuffer)	(THIS) PURE;

    STDMETHOD_(ULONG32,GetTime)		(THIS) PURE;

    STDMETHOD_(UINT16,GetStreamNumber)	(THIS) PURE;

    STDMETHOD_(UINT8,GetASMFlags)	(THIS) PURE;

    STDMETHOD_(UINT16,GetASMRuleNumber)	(THIS) PURE;

    STDMETHOD_(BOOL,IsLost)		(THIS) PURE;

    STDMETHOD(SetAsLost)		(THIS) PURE;

    STDMETHOD(Set)			(THIS_
					IRMABuffer* 	    pBuffer,
					UINT32	    	    ulTime,
					UINT16	    	    uStreamNumber,
					UINT8	    	    unASMFlags,
					UINT16	    	    unASMRuleNumber
					) PURE;

    /*
     *	IRMARTPPacket methods
     */
    STDMETHOD_(ULONG32,GetRTPTime)	(THIS) PURE;

    STDMETHOD(GetRTP)			(THIS_
					REF(IRMABuffer*)    pBuffer, 
					REF(UINT32)	    ulTime,
					REF(UINT32)	    ulRTPTime,
					REF(UINT16)	    unStreamNumber,
					REF(UINT8)	    unASMFlags,
					REF(UINT16)	    unASMRuleNumber
					) PURE;

    STDMETHOD(SetRTP)			(THIS_
					IRMABuffer* 	    pBuffer,
					UINT32	    	    ulTime,
					UINT32		    ulRTPTime,
					UINT16	    	    uStreamNumber,
					UINT8	    	    unASMFlags,
					UINT16	    	    unASMRuleNumber
					) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAValues
 *
 *  Purpose:
 *
 *  	This is an interface to a generic name-value pair facility.  This
 *	is used in various places (such as stream headers).
 *
 *  IID_IRMAValues:
 *
 *	{00001302-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAValues, 0x00001302, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  The IRMACommonClassFactory supports creating an instance
 *  of this object.
 */
#define CLSID_IRMAValues IID_IRMAValues

#undef  INTERFACE
#define INTERFACE   IRMAValues

DECLARE_INTERFACE_(IRMAValues, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     *	IRMAValues methods
     */

    /*
     * Note: That strings returned as references should be copied or
     * 	     used immediately because their lifetime is only as long as the
     * 	     IRMAValues's objects lifetime.
     *
     * Note: Your iterator will be reset once you give up control to the
     *	     RMA core (i.e. you exit whatever function gave you a time slice).
     */

    STDMETHOD(SetPropertyULONG32)	(THIS_
					const char*      pPropertyName,
					ULONG32          uPropertyValue) PURE;

    STDMETHOD(GetPropertyULONG32)	(THIS_
					const char*      pPropertyName,
					REF(ULONG32)     uPropertyName) PURE;

    STDMETHOD(GetFirstPropertyULONG32)	(THIS_
					REF(const char*) pPropertyName,
					REF(ULONG32)     uPropertyValue) PURE;

    STDMETHOD(GetNextPropertyULONG32)	(THIS_
					REF(const char*) pPropertyName,
					REF(ULONG32)     uPropertyValue) PURE;

    STDMETHOD(SetPropertyBuffer)	(THIS_
					const char*      pPropertyName,
					IRMABuffer*      pPropertyValue) PURE;

    STDMETHOD(GetPropertyBuffer)	(THIS_
					const char*      pPropertyName,
					REF(IRMABuffer*) pPropertyValue) PURE;
    
    STDMETHOD(GetFirstPropertyBuffer)	(THIS_
					REF(const char*) pPropertyName,
					REF(IRMABuffer*) pPropertyValue) PURE;

    STDMETHOD(GetNextPropertyBuffer)	(THIS_
					REF(const char*) pPropertyName,
					REF(IRMABuffer*) pPropertyValue) PURE;

    STDMETHOD(SetPropertyCString)	(THIS_
					const char*      pPropertyName,
					IRMABuffer*      pPropertyValue) PURE;

    STDMETHOD(GetPropertyCString)	(THIS_
					const char*      pPropertyName,
					REF(IRMABuffer*) pPropertyValue) PURE;

    STDMETHOD(GetFirstPropertyCString)	(THIS_
					REF(const char*) pPropertyName,
					REF(IRMABuffer*) pPropertyValue) PURE;

    STDMETHOD(GetNextPropertyCString)	(THIS_
					REF(const char*) pPropertyName,
					REF(IRMABuffer*) pPropertyValue) PURE;
};


/****************************************************************************
 * 
 *  Interface:
 *
 *	IRMAValuesRemove
 *
 *  Purpose:
 *
 *      This interface is to add Remove methods to a class that supports 
 *      IRMAValues.  All classes that support this interface will also 
 *      support IRMAValues.
 *   
 *  
 *
 *  IID_IRMAValuesRemove:
 *
 *	{00001303-0901-11d1-8B06-00A024406D59}
 *
 */
DEFINE_GUID(IID_IRMAValuesRemove, 0x00001303, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 
			0xa0, 0x24, 0x40, 0x6d, 0x59);

/*
 *  The IRMACommonClassFactory does not support creating an instance
 *  of this object.
 */

#undef  INTERFACE
#define INTERFACE   IRMAValuesRemove

DECLARE_INTERFACE_(IRMAValuesRemove, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)	(THIS_
				REFIID riid,
				void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)	(THIS) PURE;

    STDMETHOD_(ULONG,Release)	(THIS) PURE;

    /*
     * IRMAValuesRemove methods
     */

     /************************************************************************
     *	Method:
     *	    IRMAKeyValuesRemove::Remove
     *	Purpose:
     *      Remove all items matching pKey.  (If you know what datatype you saved
     *      the key as, use the specific method.)
     */
    STDMETHOD(Remove)	     (const char* pKey) PURE;
    
     /************************************************************************
     *	Method:
     *	    IRMAKeyValuesRemove::RemoveULONG32
     *	Purpose:
     *      Remove all ULONG32 items matching pKey. 
     */
    STDMETHOD(RemoveULONG32) (const char* pKey) PURE;
    
     /************************************************************************
     *	Method:
     *	    IRMAKeyValuesRemove::RemoveBuffer
     *	Purpose:
     *      Remove all Buffer items matching pKey. 
     */
    STDMETHOD(RemoveBuffer)  (const char* pKey) PURE;
    
     /************************************************************************
     *	Method:
     *	    IRMAKeyValuesRemove::RemoveCString
     *	Purpose:
     *      Remove all CString items matching pKey. 
     */
    STDMETHOD(RemoveCString) (const char* pKey) PURE;
};

#endif /* _RMAPCKTS_H_ */

