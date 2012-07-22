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
 *  RealMedia Architecture Encoder Interfaces.
 *
 */

#ifndef _RMAENCOD_H_
#define _RMAENCOD_H_

typedef _INTERFACE IUnknown			IUnknown;
typedef _INTERFACE IRMAValues			IRMAValues;
typedef _INTERFACE IRMAPacket			IRMAPacket;
typedef _INTERFACE IRMARequest			IRMARequest;
typedef _INTERFACE IRMAEncoder			IRMAEncoder;
typedef _INTERFACE IRMAEncoderResponse		IRMAEncoderResponse;
typedef _INTERFACE IRMAEncoderCompletion	IRMAEncoderCompletion;
typedef _INTERFACE IRMAEncoderResponseCompletion IRMAEncoderResponseCompletion;
typedef _INTERFACE IRMAConnectionlessControl	IRMAConnectionlessControl;
typedef _INTERFACE IRMATransportControl		IRMATransportControl;

#ifndef _MACINTOSH
STDAPI_(IUnknown*) CreateContext();
#else
#pragma export on
STDAPI_(IUnknown*) CreateContext();
#pragma export off
#endif


DEFINE_GUID(IID_IRMAEncoderResponse, 	0x00001600, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef INTERFACE
#define INTERFACE IRMAEncoderResponse

DECLARE_INTERFACE_(IRMAEncoderResponse, IUnknown)
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
     * IRMAEncoderResponse methods
     */
    STDMETHOD(InitEncoderResponse) (THIS_
				    const char* pHost,
				    UINT16 unPort, 
				    IRMARequest* pRequest,
				    const char* pUsername,
				    const char* pPassword,
				    IRMAEncoder* pEncoder) PURE;

    STDMETHOD(FileHeaderReady)     (THIS_ 
				    PN_RESULT   result, 
				    IRMAValues* pHeader) PURE;

    STDMETHOD(StreamHeaderReady)   (THIS_ 
				    PN_RESULT   result, 
				    IRMAValues* pHeader) PURE;

    STDMETHOD(PacketReady)         (THIS_ 
				    PN_RESULT   result,
				    IRMAPacket* pPacket) PURE;

    STDMETHOD(StreamDone)          (THIS_ 
				    UINT16 unStream) PURE;

    STDMETHOD(Process)             (THIS) PURE;

    STDMETHOD_(UINT32,GetTime)     (THIS) PURE;
};

DEFINE_GUID(IID_IRMAEncoder,		0x00001601, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef INTERFACE
#define INTERFACE IRMAEncoder

DECLARE_INTERFACE_(IRMAEncoder, IUnknown)
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
     * IRMAEncoder methods
     */
    STDMETHOD(InitEncoderResponseDone) (THIS_ 
					PN_RESULT result) PURE;

    STDMETHOD(GetFileHeader)           (THIS) PURE;

    STDMETHOD(GetStreamHeader)         (THIS_ 
					UINT16 unStream) PURE;

    STDMETHOD(StartPackets)            (THIS_ 
					UINT16 unStream) PURE;

    STDMETHOD(StopPackets)             (THIS_ 
					UINT16 unStream) PURE;
};

DEFINE_GUID(IID_IRMAEncoderCompletion,	0x00001602, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef INTERFACE
#define INTERFACE IRMAEncoderCompletion

DECLARE_INTERFACE_(IRMAEncoderCompletion, IUnknown)
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
     * IRMAEncoderCompletion methods
     */

    STDMETHOD(EncoderDone)		(THIS_ 
					 PN_RESULT result) PURE;
};

DEFINE_GUID(IID_IRMAConnectionlessControl,	0x00001603, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef INTERFACE
#define INTERFACE IRMAConnectionlessControl

DECLARE_INTERFACE_(IRMAConnectionlessControl, IUnknown)
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
     * IRMAConnectionlessControl methods
     */

    STDMETHOD(EnableConnectionlessControl)
					(THIS) PURE;

    STDMETHOD(ConnectionCheckFailed)	(THIS_
					PN_RESULT status) PURE;

    STDMETHOD(SetConnectionTimeout)	(THIS_
					UINT32 uSeconds) PURE;
};

DEFINE_GUID(IID_IRMAEncoderResponseCompletion,	0x00001604, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef INTERFACE
#define INTERFACE IRMAEncoderResponseCompletion

DECLARE_INTERFACE_(IRMAEncoderResponseCompletion, IUnknown)
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
     * IRMAEncoderResponseCompletion methods
     */

    STDMETHOD(EncoderResponseDone)	(THIS) PURE;
};

/*
 * The only 2 encoder transport types supported are:
 * "x-pn-tng/udp"
 * "x-pn-tng/tcp"
 */

DEFINE_GUID(IID_IRMATransportControl,	0x00001605, 0x901, 0x11d1, 0x8b, 0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef INTERFACE
#define INTERFACE IRMATransportControl

DECLARE_INTERFACE_(IRMATransportControl, IUnknown)
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
     * IRMATransportControl methods
     */

    STDMETHOD(SetTransportType)	(const char* pTransportType) PURE;
};

#endif /* _RMAENCOD_H_ */
