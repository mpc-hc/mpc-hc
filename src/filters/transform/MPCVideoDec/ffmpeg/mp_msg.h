
#ifndef _MP_MSG_H
#define _MP_MSG_H

// verbosity elevel:

// stuff from level MSGL_FATAL-MSGL_HINT should be translated.

#define MSGL_FATAL 0  // will exit/abort
#define MSGL_ERR 1    // continues
#define MSGL_WARN 2   // only warning
#define MSGL_HINT 3   // short help message
#define MSGL_INFO 4   // -quiet
#define MSGL_STATUS 5 // v=0
#define MSGL_V 6      // v=1
#define MSGL_DBG2 7   // v=2
#define MSGL_DBG3 8   // v=3
#define MSGL_DBG4 9   // v=4

// code/module:

#define MSGT_GLOBAL 0        // common player stuff errors
#define MSGT_CPLAYER 1       // console player (mplayer.c)
#define MSGT_GPLAYER 2       // gui player

#define MSGT_VO 3          // libvo
#define MSGT_AO 4          // libao

#define MSGT_DEMUXER 5    // demuxer.c (general stuff)
#define MSGT_DS 6         // demux stream (add/read packet etc)
#define MSGT_DEMUX 7      // fileformat-specific stuff (demux_*.c)
#define MSGT_HEADER 8     // fileformat-specific header (*header.c)

#define MSGT_AVSYNC 9     // mplayer.c timer stuff
#define MSGT_AUTOQ 10     // mplayer.c auto-quality stuff

#define MSGT_CFGPARSER 11 // cfgparser.c

#define MSGT_DECAUDIO 12  // av decoder
#define MSGT_DECVIDEO 13

#define MSGT_SEEK 14    // seeking code
#define MSGT_WIN32 15   // win32 dll stuff
#define MSGT_OPEN 16    // open.c (stream opening)
#define MSGT_DVD 17 // open.c (DVD init/read/seek)

#define MSGT_PARSEES 18 // parse_es.c (mpeg stream parser)
#define MSGT_LIRC 19    // lirc_mp.c and input lirc driver

#define MSGT_STREAM 20  // stream.c
#define MSGT_CACHE 21   // cache2.c

#define MSGT_MENCODER 22

#define MSGT_XACODEC 23 // XAnim codecs

#define MSGT_TV 24  // TV input subsystem

#define MSGT_OSDEP 25   // OS Dependant parts (linux/ for now)

#define MSGT_SPUDEC 26  // spudec.c

#define MSGT_PLAYTREE 27    // Playtree handeling (playtree.c, playtreeparser.c)

#define MSGT_INPUT 28

#define MSGT_VFILTER 29

#define MSGT_OSD 30

#define MSGT_NETWORK 31

#define MSGT_CPUDETECT 32

#define MSGT_CODECCFG 33

#define MSGT_SWS 34

#define MSGT_VOBSUB 35
#define MSGT_SUBREADER 36

#define MSGT_AFILTER 37  // Audio filter messages

#define MSGT_NETST 38 // Netstream

#define MSGT_MAX 64

#if 1
#ifdef __GNUC__
#define mp_msg(mod,lev, args... )
#else
#define mp_msg(x)
#endif
#else
#include "ffImgfmt.h"
static inline const char *vo_format_name(int format)
{
    switch(format)
    {
    case IMGFMT_RGB1:
        return("RGB 1-bit");
    case IMGFMT_RGB4:
        return("RGB 4-bit");
    case IMGFMT_RG4B:
        return("RGB 4-bit per byte");
    case IMGFMT_RGB8:
        return("RGB 8-bit");
    case IMGFMT_RGB15:
        return("RGB 15-bit");
    case IMGFMT_RGB16:
        return("RGB 16-bit");
    case IMGFMT_RGB24:
        return("RGB 24-bit");
    case IMGFMT_RGB32:
        return("RGB 32-bit");
    case IMGFMT_BGR1:
        return("BGR 1-bit");
    case IMGFMT_BGR4:
        return("BGR 4-bit");
    case IMGFMT_BG4B:
        return("BGR 4-bit per byte");
    case IMGFMT_BGR8:
        return("BGR 8-bit");
    case IMGFMT_BGR15:
        return("BGR 15-bit");
    case IMGFMT_BGR16:
        return("BGR 16-bit");
    case IMGFMT_BGR24:
        return("BGR 24-bit");
    case IMGFMT_BGR32:
        return("BGR 32-bit");
    case IMGFMT_YVU9:
        return("Planar YVU9");
    case IMGFMT_IF09:
        return("Planar IF09");
    case IMGFMT_YV12:
        return("Planar YV12");
    case IMGFMT_I420:
        return("Planar I420");
    case IMGFMT_IYUV:
        return("Planar IYUV");
    case IMGFMT_CLPL:
        return("Planar CLPL");
    case IMGFMT_Y800:
        return("Planar Y800");
    case IMGFMT_Y8:
        return("Planar Y8");
    case IMGFMT_444P:
        return("Planar 444P");
    case IMGFMT_422P:
        return("Planar 422P");
    case IMGFMT_411P:
        return("Planar 411P");
    case IMGFMT_NV12:
        return("Planar NV12");
        //case IMGFMT_NV21: return("Planar NV21");
        //case IMGFMT_HM12: return("Planar NV12 Macroblock");
    case IMGFMT_IUYV:
        return("Packed IUYV");
    case IMGFMT_IY41:
        return("Packed IY41");
    case IMGFMT_IYU1:
        return("Packed IYU1");
    case IMGFMT_IYU2:
        return("Packed IYU2");
    case IMGFMT_UYVY:
        return("Packed UYVY");
    case IMGFMT_UYNV:
        return("Packed UYNV");
    case IMGFMT_cyuv:
        return("Packed CYUV");
    case IMGFMT_Y422:
        return("Packed Y422");
    case IMGFMT_YUY2:
        return("Packed YUY2");
    case IMGFMT_YUNV:
        return("Packed YUNV");
    case IMGFMT_YVYU:
        return("Packed YVYU");
    case IMGFMT_Y41P:
        return("Packed Y41P");
    case IMGFMT_Y211:
        return("Packed Y211");
    case IMGFMT_Y41T:
        return("Packed Y41T");
    case IMGFMT_Y42T:
        return("Packed Y42T");
    case IMGFMT_V422:
        return("Packed V422");
    case IMGFMT_V655:
        return("Packed V655");
    case IMGFMT_CLJR:
        return("Packed CLJR");
    case IMGFMT_YUVP:
        return("Packed YUVP");
    case IMGFMT_UYVP:
        return("Packed UYVP");
        //case IMGFMT_MPEGPES: return("Mpeg PES");
        //case IMGFMT_ZRMJPEGNI: return("Zoran MJPEG non-interlaced");
        //case IMGFMT_ZRMJPEGIT: return("Zoran MJPEG top field first");
        //case IMGFMT_ZRMJPEGIB: return("Zoran MJPEG bottom field first");
        //case IMGFMT_XVMC_MOCO_MPEG2: return("MPEG1/2 Motion Compensation");
        //case IMGFMT_XVMC_IDCT_MPEG2: return("MPEG1/2 Motion Compensation and IDCT");
    }
    return("Unknown");
}
#include <stdarg.h>
#define WINAPI __stdcall
void WINAPI OutputDebugStringA(const char*); //rather than including windows.h
static inline void mp_msg(int mod, int len, const char* fmt, ...)
{
    va_list args;
    char buf[1024];
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    OutputDebugStringA(buf);
};
#endif

#endif
