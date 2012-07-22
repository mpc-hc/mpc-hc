/****************************************************************************
 *
 *  Copyright (C) 1995,1996,1997 Progressive Networks.
 *  All rights reserved.
 *
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary
 *  information of Progressive Networks, Inc, and is licensed
 *  subject to restrictions on use and distribution.
 *
 *
 *  RealMedia Architecture Video Surface Interfaces.
 *
 */

#ifndef _RMAVSURF_H_
#define _RMAVSURF_H_

/****************************************************************************
 *
 *  Video Surface Data Structures and Constants
 */
typedef struct _RMABitmapInfoHeader
{
    UINT32  biSize;
    INT32   biWidth;
    INT32   biHeight;
    UINT16  biPlanes;
    UINT16  biBitCount;
    UINT32  biCompression;
    UINT32  biSizeImage;
    INT32   biXPelsPerMeter;
    INT32   biYPelsPerMeter;
    UINT32  biClrUsed;
    UINT32  biClrImportant;
    UINT32  rcolor;
    UINT32  gcolor;
    UINT32  bcolor;

} RMABitmapInfoHeader;

/*
 * RMABitmapInfo structure.
 */
typedef struct _RMABitmapInfo
{
    struct
    {
        UINT32  biSize;         /* use OFFSETOF(dwBitMask) here     */
        INT32   biWidth;        /* image width (in pixels)          */
        INT32   biHeight;       /* image height                     */
        UINT16  biPlanes;       /* # of bitplanes; always use 1     */
        UINT16  biBitCount;     /* average # bits/pixel             */
        UINT32  biCompression;  /* one of the RMA_... FOURCC codes  */
        UINT32  biSizeImage;    /* = width * height * bitCount / 8  */
        INT32   biXPelsPerMeter;/* always 0                         */
        INT32   biYPelsPerMeter;/* always 0                         */
        UINT32  biClrUsed;      /* !0, if 8-bit RGB; 0, otherwise   */
        UINT32  biClrImportant; /* !0, if 8-bit RGB; 0, otherwise   */

    } bmiHeader;

    union
    {
        UINT32  dwBitMask[3];   /* color masks (for BI_BITFIELDS)   */
        UINT32  dwPalette[256]; /* palette (for 8-bit RGB image)    */
    } un;

} RMABitmapInfo;

typedef UINT32  RMA_COMPRESSION_TYPE;

/*
 * Windows DIB formats & MKFOURCC() macro:
 */
#ifndef BI_RGB
#define BI_RGB          0L      /* RGB-8, 16, 24, or 32             */
#define BI_RLE8         1L      /* 8-bit RLE compressed image       */
#define BI_RLE4         2L      /* 4-bit RLE compressed image       */
#define BI_BITFIELDS    3L      /* RGB 555, 565, etc.               */
#endif
#ifndef MKFOURCC
#define MKFOURCC(c0,c1,c2,c3)   \
        ((UINT32)(BYTE)(c0) | ((UINT32)(BYTE)(c1) << 8) |   \
        ((UINT32)(BYTE)(c2) << 16) | ((UINT32)(BYTE)(c3) << 24))
#endif

/*
 * RMA image formats:
 */
#define RMA_RGB         BI_RGB  /* Windows-compatible RGB formats:  */
#define RMA_RLE8        BI_RLE8
#define RMA_RLE4        BI_RLE4
#define RMA_BITFIELDS   BI_BITFIELDS
#define RMA_I420        MKFOURCC('I','4','2','0') /* planar YCrCb   */
#define RMA_YV12        MKFOURCC('Y','V','1','2') /* planar YVU420  */
#define RMA_YUY2        MKFOURCC('Y','U','Y','2') /* packed YUV422  */
#define RMA_UYVY        MKFOURCC('U','Y','V','Y') /* packed YUV422  */
#define RMA_YVU9        MKFOURCC('Y','V','U','9') /* Intel YVU9     */

/*
 * Non-standard FOURCC formats (these are just few aliases to what can be
 * represented by the standard formats, and they are left for backward
 * compatibility only).
 */
#define RMA_RGB3_ID     MKFOURCC('3','B','G','R') /* RGB-32 ??      */
#define RMA_RGB24_ID    MKFOURCC('B','G','R',' ') /* top-down RGB-24*/
#define RMA_RGB565_ID   MKFOURCC('6','B','G','R') /* RGB-16 565     */
#define RMA_RGB555_ID   MKFOURCC('5','B','G','R') /* RGB-16 555     */
#define RMA_8BIT_ID     MKFOURCC('T','I','B','8') /* RGB-8 w. pal-e */
#define RMA_YUV420_ID   MKFOURCC('2','V','U','Y') /* planar YCrCb   */
#define RMA_YUV411_ID   MKFOURCC('1','V','U','Y') /* ???            */
#define RMA_YUVRAW_ID   MKFOURCC('R','V','U','Y') /* ???            */


/****************************************************************************
 *
 *  Interface:
 *
 *  IRMAVideoSurface
 *
 *  Purpose:
 *
 *  Interface for IRMAVideoSurface objects.
 *
 *  IID_IRMAVideoSurface:
 *
 *  {00002200-0901-11d1-8B06-00A024406D59}
 *
 */

DEFINE_GUID(IID_IRMAVideoSurface, 0x00002200, 0x901, 0x11d1, 0x8b,
    0x6, 0x0, 0xa0, 0x24, 0x40, 0x6d, 0x59);

#undef  INTERFACE
#define INTERFACE   IRMAVideoSurface

DECLARE_INTERFACE_(IRMAVideoSurface, IUnknown)
{
    /*
     * IUnknown methods
     */
    STDMETHOD(QueryInterface)   (THIS_
                REFIID riid,
                void** ppvObj) PURE;

    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;

    STDMETHOD_(ULONG,Release) (THIS) PURE;

    /*
     * IRMAVideoSurface methods usually called by renderers to
     * Draw on the surface
     */
    STDMETHOD(Blt)      (THIS_
                UCHAR*          /*IN*/  pImageBits,
                RMABitmapInfoHeader*    /*IN*/  pBitmapInfo,
                REF(PNxRect)        /*IN*/  rDestRect,
                REF(PNxRect)        /*IN*/  rSrcRect) PURE;

    /************************************************************************
     *  Method:
     *      IRMAVideoSurface::BeginOptimizedBlt
     *  Purpose:
     *      Called by renderer to commit to a bitmap format for all future
     *      OptimizedBlt calls.
     */
    STDMETHOD(BeginOptimizedBlt)(THIS_
                RMABitmapInfoHeader*    /*IN*/  pBitmapInfo) PURE;

    /************************************************************************
     *  Method:
     *      IRMAVideoSurface::OptimizedBlt
     *  Purpose:
     *      Called by renderer to draw to the video surface, in the format
     *      previously specified by calling BeginOptimizedBlt.
     */
    STDMETHOD(OptimizedBlt) (THIS_
                UCHAR*          /*IN*/  pImageBits,
                REF(PNxRect)        /*IN*/  rDestRect,
                REF(PNxRect)        /*IN*/  rSrcRect) PURE;

    /************************************************************************
     *  Method:
     *      IRMAVideoSurface::EndOptimizedBlt
     *  Purpose:
     *      Called by renderer allow the video surface to cleanup after all
     *      OptimizedBlt calls have been made.
     */
    STDMETHOD(EndOptimizedBlt)  (THIS) PURE;

    /************************************************************************
     *  Method:
     *      IRMAVideoSurface::GetOptimizedFormat
     *  Purpose:
     *      Called by the client to find out what compression type the
     *      renderer committed to when it called BeginOptimizedBlt.
     */
    STDMETHOD(GetOptimizedFormat)(THIS_
                REF(RMA_COMPRESSION_TYPE) /*OUT*/ ulType) PURE;

    /************************************************************************
     *  Method:
     *      IRMAVideoSurface::GetPreferredFormat
     *  Purpose:
     *      Called by renderer to find out what compression type the video
     *      surface would prefer to be given in BeginOptimizedBlt.
     */
    STDMETHOD(GetPreferredFormat)(THIS_
                REF(RMA_COMPRESSION_TYPE) /*OUT*/ ulType) PURE;
};

#endif /* _RMAVSURF_H_ */
