/****************************************************************************
 *
 *  rmalvpix.h
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
 *  RealPix live encoder interfaces
 *
 */

#ifndef _RMALVPIX_H
#define _RMALVPIX_H

/*
 * PixInitInfo
 *
 * This struct contains initialization information
 * and is passed in in the IRMALiveRealPix::StartEncoder()
 * method. All of the members of this struct are input variables.
 */
typedef struct PNEXPORT_CLASS _PixInitInfo
{
    UINT32  m_ulStructLength;       /* length in bytes of this structure */
    char   *m_pServerAddress;       /* name or ip address of server */
    UINT32  m_ulServerPort;         /* pn-encoder port on server */
    char   *m_pUsername;            /* username for authentication on server */
    char   *m_pPassword;            /* password for authentication on server */
    char   *m_pFilename;            /* filename which clients should open */
    char   *m_pTitle;               /* title of live presentation */
    char   *m_pAuthor;              /* author of live presentation */
    char   *m_pCopyright;           /* copyright on live presentation */
    UINT32  m_ulBitrate;            /* bitrate of realpix stream */
    UINT32  m_ulMaxFps;             /* max frames per second of effects */
    UINT32  m_ulDisplayWidth;       /* display width in pixels */
    UINT32  m_ulDisplayHeight;      /* display height in pixels */
    BOOL    m_bPreserveAspect;      /* preserve aspect ratio by default? */
    char   *m_pDefaultURL;          /* default URL to send browser to when clicked */
    UINT32  m_ulNumImageCodecs;     /* number of image codecs used in this stream */
    char  **m_ppImageCodec;         /* string names of codecs */
    UINT32  m_ulNumEffectPackages;  /* number of external effect packages used */
    char  **m_ppEffectPackage;      /* string names of effect packages*/
}
PixInitInfo;

/*
 * PixImageInfo
 *
 * This struct contains all the information for images which
 * should be sent down the stream. This struct is used in the 
 * IRMALiveRealPix::InitImage() method.
 */
typedef struct PNEXPORT_CLASS _PixImageInfo
{
    UINT32  m_ulStructLength;      /* Input:  size of struct */
    BYTE   *m_pImageBuffer;        /* Input:  buffer holding image data */
    UINT32  m_ulImageBufferSize;   /* Input:  size of image buffer */
    char   *m_pImageCodec;         /* Input:  image codec to be used */
    UINT32  m_ulHandle;            /* Output: image handle */
    UINT32  m_ulNumPackets;        /* Output: number of packets image will be sent */
    UINT32  m_ulTimeToSend;        /* Output: milliseconds required to send image */
}
PixImageInfo;

#define EFFECT_FILL           0
#define EFFECT_FADEIN         1
#define EFFECT_FADEOUT        2
#define EFFECT_CROSSFADE      3
#define EFFECT_WIPE           4
#define EFFECT_VIEWCHANGE     5
#define EFFECT_EXTERNAL       6
#define EFFECT_ANIMATE        7

#define WIPE_DIRECTION_UP     0
#define WIPE_DIRECTION_DOWN   1
#define WIPE_DIRECTION_LEFT   2
#define WIPE_DIRECTION_RIGHT  3

#define WIPE_TYPE_NORMAL      0
#define WIPE_TYPE_PUSH        1

/*
 * PixEffectInfo
 *
 * This struct contains all the information about the effect which 
 * the RealPix renderer should perform. This struct is used in the
 * IRMALiveRealPix::SendEffect() method.
 */
typedef struct PNEXPORT_CLASS _PixEffectInfo
{
    UINT32  m_ulStructLength;      /* Input:  Length in bytes of this struct */
    BYTE    m_ucEffectType;        /* Input:  Effect Type: EFFECT_FILL, EFFECT_FADEIN, etc. */
    UINT32  m_ulStart;             /* Input:  Start time in milliseconds of effect */
    UINT32  m_ulDuration;          /* Input:  Duration in milliseconds of effect */
    UINT32  m_ulTarget;            /* Input:  Handle of image to perform effect on */
    UINT32  m_ulSrcX;              /* Input:  Horizontal offset of source rectangle */
    UINT32  m_ulSrcY;              /* Input:  Vertical offset of source rectangle */
    UINT32  m_ulSrcW;              /* Input:  Width of source rectangle */
    UINT32  m_ulSrcH;              /* Input:  Height of source rectangle */
    UINT32  m_ulDstX;              /* Input:  Horizontal offset of destination rectangle */
    UINT32  m_ulDstY;              /* Input:  Vertical offset of destination rectangle */
    UINT32  m_ulDstW;              /* Input:  Width of destination rectangle */
    UINT32  m_ulDstH;              /* Input:  Height of destination rectangle */
    UINT32  m_ulMaxFps;            /* Input:  Max frames per second for this effect */
    BOOL    m_bAspectFlag;         /* Input:  TRUE: preserve aspect on this effect; FALSE: don't preserve */
    BYTE    m_ucRed;               /* Input:  Red component of fill or fadeout color */
    BYTE    m_ucGreen;             /* Input:  Green component of fill or fadeout color */
    BYTE    m_ucBlue;              /* Input:  Blue component of fill or fadeout color */
    BYTE    m_ucWipeDirection;     /* Input:  WIPE_DIRECTION_xxx, where xxx is UP, DOWN, LEFT, or RIGHT */
    BYTE    m_ucWipeType;          /* Input:  WIPE_TYPE_NORMAL or WIPE_TYPE_PUSH */
    char   *m_pURL;                /* Input:  URL for this effect */
    char   *m_pExtPackage;         /* Input:  Name of external effect package */
    char   *m_pExtName;            /* Input:  Name of external effect within the package */
    char   *m_pExtData;            /* Input   Opaque string data for external effect */
    char   *m_pExtFile;            /* Input:  File contents of external effect data */
    BOOL    m_bDiscardImage;       /* Input:  TRUE: discard image after doing effect; FALSE: don't discard */
    BOOL    m_bDisplayImmediately; /* Input:  FALSE: obey start time; TRUE: do effect immediately */
	UINT32  m_ulHandle;            /* Output: Handle by which effect will be referred to */
}
PixEffectInfo;

/*
 * Forward declarations of some interfaces defined here.
 */
typedef _INTERFACE IRMALiveRealPix         IRMALiveRealPix;
typedef _INTERFACE IRMALiveRealPixResponse IRMALiveRealPixResponse;
typedef _INTERFACE IRMALiveRealPixResend   IRMALiveRealPixResend;

/*
 * Forward declarations of interfaces used here
 */
typedef _INTERFACE IRMAValues              IRMAValues;

/*
 * Declaration of the DLL entry point
 */
STDAPI CreateLiveRealPix(IRMALiveRealPix **);

typedef PN_RESULT (PNEXPORT_PTR FPRMCREATELIVEREALPIX)(IRMALiveRealPix **ppLiveRealPix);

/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMALivePix
 * 
 *  Purpose:
 * 
 *  Asynchronous interface that allows an application to encode live RealPix
 * 
 *  IID_IRMALiveRealPix:
 * 
 *  {E7ADF466-477D-11d2-AA0C-0060972D23A7}
 *
 */
DEFINE_GUID(IID_IRMALiveRealPix,  0xe7adf466, 0x477d, 0x11d2, 0xaa, 0xc, 0x0,
            0x60, 0x97, 0x2d, 0x23, 0xa7);

#undef  INTERFACE
#define INTERFACE   IRMALiveRealPix

DECLARE_INTERFACE_(IRMALiveRealPix, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)      (THIS_ REFIID riid, void **ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)     (THIS) PURE;

    STDMETHOD_(ULONG,Release)    (THIS) PURE;

    /*
     *  IRMALiveRealPix methods
     */

    /*
     * StartEncoder() must be called before anything else and a pointer to
     * an IRMALiveRealPixResponse interface must be passed in. The encoder will
     * respond to this call asynchronously with IRMALiveRealPixResponse::EncoderStarted().
     */
    STDMETHOD(StartEncoder)        (THIS_ PixInitInfo             *pInitInfo,
                                          IRMALiveRealPixResponse *pResponse) PURE;

    /*
     * InitImage() is called to prep an image for being sent. It returns several useful
     * pieces of information which the caller can make use of: the handle to refer to
     * the image in SendImage(), the number of packets this image will be broken up into,
     * and the time required to send this image in milliseconds. Note that InitImage()
     * simply breaks the image up into packets - nothing has been sent to the server yet.
     * This is not an asynchronous call - all processing has finished by the time this
     * call returns.
     */
    STDMETHOD(InitImage)           (THIS_ PixImageInfo *pImageInfo) PURE;

    /*
     * SendImage() tranfers the all the packets for the image referred to by ulImageHandle
     * into the packet send queue. Further calls to Process() will result in these
     * packets being sent to the RealServer. When all the packets for this image have
     * been sent, the encoder will respond with IRMALiveRealPixResponse::ImageSent().
     */
    STDMETHOD(SendImage)           (THIS_ UINT32 ulImageHandle) PURE;

    /*
     * SendEffect() creates an effect packet with the information contained in
     * the PixEffectInfo struct and immediately adds this packet to the packet
     * send queue. Further calls to Process() will result in this packet being 
     * sent to the server. A handle is returned in the PixEffectInfo struct by
     * which this effect can later be identified. When the effect has been
     * sent to the server, the encoder will respond with
     * IRMALiveRealPixResponse::EffectSent().
     */
    STDMETHOD(SendEffect)          (THIS_ PixEffectInfo *pEffectInfo) PURE;

    /*
     * StopEncoder() may be called at any time after calling StartEncoder().
     * This tells the encoder that no more images of effects are going to
     * be sent to the encoder. The encoder shuts down the connection to
     * the server and responds with IRMALiveRealPixResponse::EncoderStopped().
     */
    STDMETHOD(StopEncoder)         (THIS) PURE;

    /*
     * GetTime() returns the time in milliseconds since the encoder library was initialized.
     */
    STDMETHOD_(UINT32, GetTime)    (THIS) PURE;

    /*
     * Process() must be called in order to give the library time to send
     * packets to the server. It should be called very often in between SendImage()
     * and ImageSent(), as well as between SendEffect() and EffectSent(). Other
     * that these times, it should be called every 3-5 seconds.
     */
    STDMETHOD(Process)             (THIS) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMALiveRealPixResponse
 * 
 *  Purpose:
 * 
 *  Asynchronous response interface that allows an application to
 *  encode live RealPix
 * 
 *  IID_IRMALiveRealPixResponse:
 * 
 *  {E7ADF46C-477D-11d2-AA0C-0060972D23A7}
 *
 */
DEFINE_GUID(IID_IRMALiveRealPixResponse, 0xe7adf46c, 0x477d, 0x11d2, 0xaa, 0xc, 0x0,
            0x60, 0x97, 0x2d, 0x23, 0xa7);


#undef  INTERFACE
#define INTERFACE   IRMALiveRealPixResponse

DECLARE_INTERFACE_(IRMALiveRealPixResponse, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)      (THIS_ REFIID riid, void **ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)     (THIS) PURE;

    STDMETHOD_(ULONG,Release)    (THIS) PURE;

    /*
     *  IRMALiveRealPixResponse methods
     */

    /*
     * EncoderStarted() is the asynchronous response to StartEncoder(). The status
     * argument tells whether initializing the server was successful or not, and the pszHeaderString
     * argument returns the text string returned by the RealServer.
     */
    STDMETHOD(EncoderStarted)      (THIS_ PN_RESULT status, const char *pszHeaderString) PURE;

    /*
     * ImageSent() is the asynchronous response to SendImage(). The ulImageHandle argument
     * identifies which image has just completed sending to the server, and the status 
     * argument tells whether or not the send was successful or not.
     */
    STDMETHOD(ImageSent)           (THIS_ PN_RESULT status, UINT32 ulImageHandle) PURE;

    /*
     * EffectSent() is the asynchronous response to SendEffect(). The ulEffectHandle argument
     * identifies which effect has just completed sending to the server, and the status 
     * argument tells whether or not the send was successful or not.
     */
    STDMETHOD(EffectSent)          (THIS_ PN_RESULT status, UINT32 ulEffectHandle) PURE;

    /*
     * EncoderStopped() is the asynchronous response to StopEncoder(). The status
     * argument tells whether or not the stopping of the encoder was successful.
     * If the status is PNR_OK, then the application can then shut down or turn
     * around and call StartEncoder() again.
     */
    STDMETHOD(EncoderStopped)      (THIS_ PN_RESULT status) PURE;

    /*
     * ErrorOccurred() is called when the encoder receives an error from the RealServer.
     * Depending upon the severity of the error, the RealServer may then shut down
     * the encoder and an EncoderStopped() call would be made. Therefore, the
     * application should be ready to handle an EncoderStopped() call at any time.
     * If an error occurs, it's probably a good idea to call StopEncoder() and shut
     * down the encoder from the application side anyway.
     */
    STDMETHOD(ErrorOccurred)       (THIS_ const UINT8   unSeverity,
                                          const ULONG32 ulRMACode,
                                          const ULONG32 ulUserCode,
                                          const char   *pszUserString,
                                          const char   *pszMoreInfoURL) PURE;
};

/****************************************************************************
 * 
 *  Interface:
 * 
 *  IRMALiveRealPixResend
 * 
 *  Purpose:
 * 
 *  Allows re-sending and releasing of images with IRMALiveRealPix
 * 
 *  IID_IRMALiveRealPixResend:
 * 
 *  {D814DA11-8B02-11D3-8AF3-00C0F030B4E5}
 */
DEFINE_GUID(IID_IRMALiveRealPixResend, 0xd814da11, 0x8b02, 0x11d3, 0x8a, 0xfe, 0x0,
            0xc0, 0xf0, 0x30, 0xb4, 0xe5);


#undef  INTERFACE
#define INTERFACE   IRMALiveRealPixResend

DECLARE_INTERFACE_(IRMALiveRealPixResend, IUnknown)
{
    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)      (THIS_ REFIID riid, void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)     (THIS) PURE;

    STDMETHOD_(ULONG,Release)    (THIS) PURE;

    /*
     *  IRMALiveRealPixResend methods
     */

    /*
     * InitResend() informs the encoder that from now on, after an
     * image is sent with SendImage() it should not be discarded 
     * but held in the encoder until ReleaseImage() is called().
     */
    STDMETHOD(InitResend)   (THIS_ IRMAValues* pOptions) PURE;

    /*
     * ReleaseImage() informs the encoder that the application no longer
     * intends to call SendImage() on the image referenced by ulImageHandle
     * and that it can discard the image. Further calls to SendImage(x) will
     * return PNR_UNKNOWN_IMAGE after ReleaseImage(x) has been called.
     * ReleaseImage(x) will return PNR_UNKNOWN_IMAGE if x is an unknown handle.
     */
    STDMETHOD(ReleaseImage) (THIS_ UINT32 ulImageHandle) PURE;

    /*
     * DumpAllQueuedPackets() clears any packets currently waiting to be
     * sent to the server. These packets were put on the send queue by
     * either IRMALiveRealPix::SendImage() or IRMALiveRealPix::SendEffect().
     */
    STDMETHOD(DumpAllQueuedPackets) (THIS) PURE;
};

#endif
