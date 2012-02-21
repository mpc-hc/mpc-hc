/*
 * Copyright (c) 2003-2006 Milan Cutka
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "stdafx.h"
#include <vector>
#include "PODtypes.h"
#include "avcodec.h"
#include <uuids.h>
#include <moreuuids.h>
#include "char_t.h"
#include "ffImgfmt.h"

const TcspInfo cspInfos[]= {
    {
        FF_CSP_420P,_l("YV12"),
        1,12, //Bpp
        3, //numplanes
        {0,1,1,0}, //shiftX
        {0,1,1,0}, //shiftY
        {0,128,128,0},  //black,
        FOURCC_YV12, FOURCC_YV12, &MEDIASUBTYPE_YV12
    },
    {
        FF_CSP_422P,_l("422P"),
        1,16, //Bpp
        3, //numplanes
        {0,1,1,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,128,128,0},  //black
        FOURCC_422P, FOURCC_422P, &MEDIASUBTYPE_422P
    },
    {
        FF_CSP_444P,_l("444P"),
        1,24, //Bpp
        3, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,128,128,0},  //black
        FOURCC_444P, FOURCC_444P, &MEDIASUBTYPE_444P
    },
    {
        FF_CSP_411P,_l("411P"),
        1,17, //Bpp
        3, //numplanes
        {0,2,2,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,128,128,0},  //black
        FOURCC_411P, FOURCC_411P, &MEDIASUBTYPE_411P
    },
    {
        FF_CSP_410P,_l("410P"),
        1,10, //Bpp
        3, //numplanes
        {0,2,2,0}, //shiftX
        {0,2,2,0}, //shiftY
        {0,128,128,0}, //black
        FOURCC_410P, FOURCC_410P, &MEDIASUBTYPE_410P
    },

    {
        FF_CSP_YUY2,_l("YUY2"),
        2,16, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0x8000,0,0,0},  //black
        FOURCC_YUY2, FOURCC_YUY2, &MEDIASUBTYPE_YUY2,
        0,1 //packedLumaOffset,packedChromaOffset
    },
    {
        FF_CSP_UYVY,_l("UYVY"),
        2,16, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0x0080,0,0,0},  //black
        FOURCC_UYVY, FOURCC_UYVY, &MEDIASUBTYPE_UYVY,
        1,0 //packedLumaOffset,packedChromaOffset
    },
    {
        FF_CSP_YVYU,_l("YVYU"),
        2,16, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0},  //shiftY
        {0x8000,0,0,0},  //black
        FOURCC_YVYU, FOURCC_YVYU, &MEDIASUBTYPE_YVYU,
        0,1 //packedLumaOffset,packedChromaOffset
    },
    {
        FF_CSP_VYUY,_l("VYUY"),
        2,16, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0},  //shiftY
        {0x0080,0,0,0},  //black
        FOURCC_VYUY, FOURCC_VYUY, &MEDIASUBTYPE_VYUY,
        1,0 //packedLumaOffset,packedChromaOffset
    },

    {
        FF_CSP_ABGR,_l("ABGR"),
        4,32, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        BI_RGB, 0, &MEDIASUBTYPE_RGB32
    },
    {
        FF_CSP_RGBA,_l("RGBA"),
        4,32, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        BI_RGB, 0, &MEDIASUBTYPE_RGB32
    },
    {
        FF_CSP_BGR32,_l("BGR32"),
        4,32, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        BI_RGB, 0, &MEDIASUBTYPE_RGB32
    },
    {
        FF_CSP_BGR24,_l("BGR24"),
        3,24, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        BI_RGB, 0, &MEDIASUBTYPE_RGB24
    },
    {
        FF_CSP_BGR15,_l("BGR15"),
        2,16, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        BI_RGB, 0, &MEDIASUBTYPE_RGB555
    },
    {
        FF_CSP_BGR16,_l("BGR16"),
        2,16, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        BI_RGB, 0, &MEDIASUBTYPE_RGB565
    },
    {
        FF_CSP_RGB32,_l("RGB32"),
        4,32, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        BI_RGB, FOURCC_RGB3, &MEDIASUBTYPE_RGB32
    },
    {
        FF_CSP_RGB24,_l("RGB24"),
        3,24, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        BI_RGB, FOURCC_RGB2, &MEDIASUBTYPE_RGB24
    },
    {
        FF_CSP_RGB15,_l("RGB15"),
        2,16, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        BI_RGB, FOURCC_RGB5, &MEDIASUBTYPE_RGB555
    },
    {
        FF_CSP_RGB16,_l("RGB16"),
        2,16, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        BI_RGB, FOURCC_RGB6, &MEDIASUBTYPE_RGB565
    },
    {
        FF_CSP_CLJR,_l("cljr"),
        1,16, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        FOURCC_CLJR, FOURCC_CLJR, &MEDIASUBTYPE_CLJR
    },
    {
        FF_CSP_Y800,_l("gray"),
        1,8, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        FOURCC_Y800, FOURCC_Y800, &MEDIASUBTYPE_Y800
    },
    {
        FF_CSP_NV12,_l("NV12"),
        1,12, //Bpp
        2, //numplanes
        {0,0,0,0}, //shiftX
        {0,1,1,0}, //shiftY
        {0,128,128,0}, //black
        FOURCC_NV12, FOURCC_NV12, &MEDIASUBTYPE_NV12
    },
    {
        FF_CSP_420P10,_l("420P10"),
        2,24, //Bpp
        3, //numplanes
        {0,1,1,0}, //shiftX
        {0,1,1,0}, //shiftY
        {0,512,512,0},  //black
        FOURCC_420R, FOURCC_420R, &MEDIASUBTYPE_420R
    },
    {
        FF_CSP_444P10,_l("444P10"),
        2,48, //Bpp
        3, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,512,512,0},  //black
        FOURCC_444R, FOURCC_444R, &MEDIASUBTYPE_444R
    },
    {
        FF_CSP_P016,_l("P016"),
        2,24, //Bpp
        2, //numplanes
        {0,0,0,0}, //shiftX
        {0,1,1,0}, //shiftY
        {0,32768,32768,0},  //black,
        FOURCC_P016, FOURCC_P016, &MEDIASUBTYPE_P016
    },
    {
        FF_CSP_P010,_l("P010"),
        2,24, //Bpp
        2, //numplanes
        {0,0,0,0}, //shiftX
        {0,1,1,0}, //shiftY
        {0,32768,32768,0},  //black,
        FOURCC_P010, FOURCC_P010, &MEDIASUBTYPE_P010
    },
    {
        FF_CSP_422P10,_l("422P10"),
        2,32, //Bpp
        3, //numplanes
        {0,1,1,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,512,512,0},  //black
        FOURCC_422R, FOURCC_422R, &MEDIASUBTYPE_422R
    },
    {
        FF_CSP_P210,_l("P210"),
        2,32, //Bpp
        2, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,32768,32768,0},  //black,
        FOURCC_P210, FOURCC_P210, &MEDIASUBTYPE_P210
    },
    {
        FF_CSP_P216,_l("P216"),
        2,32, //Bpp
        2, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,32768,32768,0},  //black,
        FOURCC_P216, FOURCC_P216, &MEDIASUBTYPE_P216
    },
    {
        FF_CSP_AYUV,_l("AYUV"),
        4,32, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,128,128,0},  //black
        FOURCC_AYUV, FOURCC_AYUV, &MEDIASUBTYPE_AYUV
    },
    {
        FF_CSP_Y416,_l("Y416"),
        8,64, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,32768,32768,0},  //black
        FOURCC_Y416, FOURCC_Y416, &MEDIASUBTYPE_Y416
    },
    {
        FF_CSP_PAL8,_l("pal8"),
        1,8, //Bpp
        1, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        BI_RGB, 0, &MEDIASUBTYPE_RGB8
    },
    {
        FF_CSP_GBRP,_l("GBRP"),
        1,24, //Bpp
        3, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        0, 0, NULL
    },
    {
        FF_CSP_GBRP9,_l("GBRP9"),
        2,48, //Bpp
        3, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        0, 0, NULL
    },
    {
        FF_CSP_GBRP10,_l("GBRP10"),
        2,48, //Bpp
        3, //numplanes
        {0,0,0,0}, //shiftX
        {0,0,0,0}, //shiftY
        {0,0,0,0}, //black
        0, 0, NULL
    },
    0
};

TcspInfo cspInfoIYUV= {
    FF_CSP_420P,_l("YV12"),
    1,12, //Bpp
    3, //numplanes
    {0,1,1,0}, //shiftX
    {0,1,1,0}, //shiftY
    {0,128,128,0},  //black,
    FOURCC_IYUV, FOURCC_IYUV, &MEDIASUBTYPE_IYUV
};

const TcspFcc cspFccs[]= {
    _l("YV12")     ,FOURCC_YV12,FF_CSP_420P|FF_CSP_FLAGS_YUV_ADJ,false,true,
    _l("I420/IYUV"),FOURCC_I420,FF_CSP_420P|FF_CSP_FLAGS_YUV_ADJ|FF_CSP_FLAGS_YUV_ORDER,false,true,
    _l("YUY2")     ,FOURCC_YUY2,FF_CSP_YUY2,false,true,
    _l("YVYU")     ,FOURCC_YVYU,FF_CSP_YVYU,false,true,
    _l("UYVY")     ,FOURCC_UYVY,FF_CSP_UYVY,false,true,
    _l("VYUY")     ,FOURCC_VYUY,FF_CSP_VYUY,false,true,
    _l("RGB32")    ,FOURCC_RGB3,FF_CSP_RGB32,true,true,
    _l("RGB24")    ,FOURCC_RGB2,FF_CSP_RGB24,true,true,
    _l("RGB555")   ,FOURCC_RGB5,FF_CSP_RGB15,true,true,
    _l("RGB565")   ,FOURCC_RGB6,FF_CSP_RGB16,true,true,
    _l("CLJR")     ,FOURCC_CLJR,FF_CSP_CLJR,false,false,
    _l("Y800")     ,FOURCC_Y800,FF_CSP_Y800,false,true,
    _l("NV12")     ,FOURCC_NV12,FF_CSP_NV12,false,false,
    _l("P016")     ,FOURCC_P016,FF_CSP_P016,false,false,
    _l("P010")     ,FOURCC_P010,FF_CSP_P010,false,false,
    _l("P210")     ,FOURCC_P210,FF_CSP_P210,false,false,
    _l("P216")     ,FOURCC_P216,FF_CSP_P216,false,false,
    _l("AYUV")     ,FOURCC_AYUV,FF_CSP_AYUV,false,false,
    _l("Y416")     ,FOURCC_Y416,FF_CSP_Y416,false,false,
    NULL,0
};

char_t* csp_getName(uint64_t csp,char_t *buf,size_t len)
{
    return csp_getName2(csp_getInfo(csp),csp,buf,len);
}
char_t* csp_getName2(const TcspInfo *cspInfo,uint64_t csp,char_t *buf,size_t len)
{
    const char_t *colorspaceName=cspInfo?cspInfo->name:_l("unknown");
    _sntprintf_s(buf,
                 len,
                 _TRUNCATE,
                 _l("%s%s%s%s%s%s"),
                 colorspaceName,
                 csp & FF_CSP_FLAGS_VFLIP ? _l(", flipped vertically") : _l(""),
                 csp & FF_CSP_FLAGS_INTERLACED ? _l(", interlaced") : _l(""),
                /*csp & FF_CSP_FLAGS_YUV_ADJ ? _l(", adj") : */_l(""), // These 3 flags are irrelevant to the end user, can raise questions about them
                /*csp & FF_CSP_FLAGS_YUV_ORDER ? _l(", VU") : */_l(""),
                /*csp & FF_CSP_FLAGS_YUV_JPEG ? _l(", JPEG") : */_l(""));
    return buf;
}

const TcspInfo* csp_getInfoFcc(FOURCC fcccsp)
{
    if (fcccsp==FOURCC_IYUV || fcccsp==FOURCC_I420) {
        return &cspInfoIYUV;
    } else {
        for (int i=0; i<FF_CSPS_NUM; i++)
            if (cspInfos[i].fcccsp==fcccsp) {
                return cspInfos+i;
            }
        return NULL;
    }
}

uint64_t csp_bestMatch(uint64_t inCSP, uint64_t wantedCSPS, int *rank, uint64_t outPrimaryCSP)
{
    if (outPrimaryCSP & wantedCSPS & FF_CSPS_MASK) {
        if (rank)
            *rank = 200;
        return outPrimaryCSP|(inCSP&~FF_CSPS_MASK);
    }
    uint64_t outCSP=inCSP&wantedCSPS&FF_CSPS_MASK;
    if (outCSP) {
        if (rank) {
            *rank=100;
        }
        return outCSP|(inCSP&~FF_CSPS_MASK);
    }

    const uint64_t *bestcsps=NULL;
    switch (inCSP&FF_CSPS_MASK) {
        case FF_CSP_420P: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_NV12 ,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_422P: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_420P ,
                FF_CSP_NV12 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_444P: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_AYUV ,
                FF_CSP_444P10,
                FF_CSP_Y416 ,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P ,
                FF_CSP_NV12 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_411P: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_420P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_410P: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_420P ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_411P ,
                FF_CSP_NV12 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }

        case FF_CSP_YUY2: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_420P ,
                FF_CSP_NV12 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_UYVY: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_YUY2 ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_420P ,
                FF_CSP_NV12 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_YVYU: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_420P ,
                FF_CSP_NV12 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_VYUY: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_420P ,
                FF_CSP_NV12 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }

        case FF_CSP_ABGR: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_RGBA: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_ABGR ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P ,
                FF_CSP_NV12 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_BGR32: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P ,
                FF_CSP_NV12 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_BGR24: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_RGB24,
                FF_CSP_BGR32,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_RGB32,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P ,
                FF_CSP_NV12 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_BGR15: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_BGR32,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR24,
                FF_CSP_BGR16,
                FF_CSP_RGB32,
                FF_CSP_RGB24,
                FF_CSP_RGB15,
                FF_CSP_RGB16,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_BGR16: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_BGR32,
                FF_CSP_BGR24,
                FF_CSP_BGR15,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_RGB32,
                FF_CSP_RGB24,
                FF_CSP_RGB15,
                FF_CSP_RGB16,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_RGB32: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P ,
                FF_CSP_420P10,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR15,
                FF_CSP_BGR16,
                FF_CSP_RGB15,
                FF_CSP_RGB16,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_RGB24: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_BGR24,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR15,
                FF_CSP_BGR16,
                FF_CSP_RGB15,
                FF_CSP_RGB16,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_RGB15: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_BGR15,
                FF_CSP_RGB32,
                FF_CSP_RGB24,
                FF_CSP_RGB16,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_BGR24,
                FF_CSP_BGR16,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_RGB16: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_RGB15,
                FF_CSP_RGB32,
                FF_CSP_RGB24,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_BGR24,
                FF_CSP_BGR15,
                FF_CSP_BGR16,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_CLJR: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_420P ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_Y800: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_420P ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_NV12: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_420P ,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_420P10: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_444P10,
                FF_CSP_Y416 ,
                FF_CSP_420P ,
                FF_CSP_NV12 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_444P ,
                FF_CSP_AYUV ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_422P10: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_Y416 ,
                FF_CSP_422P ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_444P10,
                FF_CSP_444P ,
                FF_CSP_AYUV ,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_420P ,
                FF_CSP_NV12 ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_444P10: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_Y416 ,
                FF_CSP_444P ,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_RGB32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_AYUV ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_420P ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_BGR16,
                FF_CSP_RGB16,
                FF_CSP_BGR15,
                FF_CSP_RGB15,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_PAL8: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_RGB32,
                FF_CSP_BGR32,
                FF_CSP_RGB24,
                FF_CSP_BGR24,
                FF_CSP_RGB15,
                FF_CSP_BGR15,
                FF_CSP_RGB16,
                FF_CSP_BGR16,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_444P ,
                FF_CSP_422P ,
                FF_CSP_420P ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_GBRP: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_RGB32,
                FF_CSP_BGR32,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_444P ,
                FF_CSP_444P10,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P ,
                FF_CSP_422P10,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P ,
                FF_CSP_420P10,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR15,
                FF_CSP_BGR16,
                FF_CSP_RGB15,
                FF_CSP_RGB16,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_GBRP9: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_RGB32,
                FF_CSP_BGR32,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_444P10,
                FF_CSP_444P ,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P10,
                FF_CSP_422P ,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P10,
                FF_CSP_420P ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR15,
                FF_CSP_BGR16,
                FF_CSP_RGB15,
                FF_CSP_RGB16,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        case FF_CSP_GBRP10: {
            static const uint64_t best[FF_CSPS_NUM]= {
                FF_CSP_RGB32,
                FF_CSP_BGR32,
                FF_CSP_ABGR ,
                FF_CSP_RGBA ,
                FF_CSP_BGR32,
                FF_CSP_BGR24,
                FF_CSP_RGB24,
                FF_CSP_444P10,
                FF_CSP_444P ,
                FF_CSP_AYUV ,
                FF_CSP_Y416 ,
                FF_CSP_YUY2 ,
                FF_CSP_UYVY ,
                FF_CSP_YVYU ,
                FF_CSP_VYUY ,
                FF_CSP_422P10,
                FF_CSP_422P ,
                FF_CSP_P210 ,
                FF_CSP_P216 ,
                FF_CSP_420P10,
                FF_CSP_420P ,
                FF_CSP_411P ,
                FF_CSP_410P ,
                FF_CSP_NV12 ,
                FF_CSP_P010 ,
                FF_CSP_P016 ,
                FF_CSP_BGR15,
                FF_CSP_BGR16,
                FF_CSP_RGB15,
                FF_CSP_RGB16,
                FF_CSP_NULL
            };
            bestcsps=best;
            break;
        }
        default:
            return FF_CSP_NULL;
    }
    if (rank) {
        *rank=99;
    }
    while (*bestcsps) {
        if (*bestcsps&wantedCSPS) {
            return *bestcsps|(inCSP&~FF_CSPS_MASK);
        }
        bestcsps++;
        if (rank) {
            (*rank)--;
        }
    }
    return FF_CSP_NULL;
}

/*bool csp_inFOURCCmask(uint64_t x,FOURCC fcc)
{
    switch (fcc) {
        case FOURCC_MASK_YUV:
            return !!csp_isYUV(x);
        case FOURCC_MASK_RGB:
            return !!csp_isRGB(x);
        default:
            return false;
    }
}*/

bool TcspInfos::TsortFc::operator ()(const TcspInfo* &csp1,const TcspInfo* &csp2)
{
    int rank1;
    csp_bestMatch(csp,csp1->id,&rank1, outPrimaryCSP);
    int rank2;
    csp_bestMatch(csp,csp2->id,&rank2, outPrimaryCSP);
    return rank1>rank2;
}
/*void TcspInfos::sort(uint64_t csp, uint64_t outPrimaryCSP)
{
    std::sort(begin(),end(),TsortFc(csp&FF_CSPS_MASK, outPrimaryCSP));
}

uint64_t getBMPcolorspace(const BITMAPINFOHEADER *hdr,const TcspInfos &forcedCsps)
{
    uint64_t csp;
    switch(hdr->biCompression) {
        case BI_RGB:
            switch (hdr->biBitCount) {
                case 15:
                    csp=FF_CSP_RGB15|FF_CSP_FLAGS_VFLIP;
                    break;
                case 16:
                    csp=FF_CSP_RGB16|FF_CSP_FLAGS_VFLIP;
                    break;
                case 24:
                    csp=FF_CSP_RGB24|FF_CSP_FLAGS_VFLIP;
                    break;
                case 32:
                    csp=FF_CSP_RGB32|FF_CSP_FLAGS_VFLIP;
                    break;
                default:
                    return FF_CSP_NULL;
            }
            break;
        case FOURCC_I420:
        case FOURCC_IYUV:
            csp=FF_CSP_420P|FF_CSP_FLAGS_YUV_ADJ|FF_CSP_FLAGS_YUV_ORDER;
            break;
        case FOURCC_YV12:
            csp=FF_CSP_420P|FF_CSP_FLAGS_YUV_ADJ;
            break;
        case FOURCC_YUYV:
        case FOURCC_YUY2:
        case FOURCC_V422:
            csp=FF_CSP_YUY2;
            break;
        case FOURCC_YVYU:
            csp=FF_CSP_YVYU;
            break;
        case FOURCC_UYVY:
            csp=FF_CSP_UYVY;
            break;
        case FOURCC_VYUY:
            csp=FF_CSP_VYUY;
            break;
        case FOURCC_Y800:
            csp=FF_CSP_Y800;
            break;
        case FOURCC_444P:
        case FOURCC_YV24:
            csp=FF_CSP_444P;
            break;
        case FOURCC_422P:
            csp=FF_CSP_422P;
            break;
        case FOURCC_YV16:
            csp=FF_CSP_422P|FF_CSP_FLAGS_YUV_ADJ;
            break;
        case FOURCC_411P:
        case FOURCC_Y41B:
            csp=FF_CSP_411P;
            break;
        case FOURCC_410P:
            csp=FF_CSP_410P;
            break;
        case FOURCC_420R:
            csp=FF_CSP_420P10;
            break;
        case FOURCC_422R:
            csp=FF_CSP_422P10;
            break;
        case FOURCC_444R:
            csp=FF_CSP_444P10;
            break;
        case FOURCC_P016:
            csp=FF_CSP_P016;
            break;
        case FOURCC_P010:
            csp=FF_CSP_P010;
            break;
        case FOURCC_P210:
            csp=FF_CSP_P210;
            break;
        case FOURCC_P216:
            csp=FF_CSP_P216;
            break;
        case FOURCC_AYUV:
            csp=FF_CSP_AYUV;
            break;
        case FOURCC_Y416:
            csp=FF_CSP_Y416;
            break;
        default:
            return FF_CSP_NULL;
    }
    bool ok;
    if (!forcedCsps.empty()) {
        ok=std::find(forcedCsps.begin(),forcedCsps.end(),csp_getInfo(csp))!=forcedCsps.end();
    } else {
        ok=true;
    }
    return ok?csp:FF_CSP_NULL;
}*/
