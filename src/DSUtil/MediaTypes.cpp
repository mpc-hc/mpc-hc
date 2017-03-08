/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012, 2016-2017 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "moreuuids.h"
#include "MediaTypes.h"
#include "DSUtil.h"

#define VIH_NORMAL     (sizeof(VIDEOINFOHEADER))
#define VIH_BITFIELDS  (sizeof(VIDEOINFOHEADER) + 3 * sizeof(RGBQUAD))
#define VIH2_NORMAL    (sizeof(VIDEOINFOHEADER2))
#define VIH2_BITFIELDS (sizeof(VIDEOINFOHEADER2) + 3 * sizeof(RGBQUAD))
#define BIH_SIZE       (sizeof(BITMAPINFOHEADER))

const VIH vihs[] = {
    // VYUY
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, mmioFOURCC('V', 'Y', 'U', 'Y'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_VYUY                                              // subtype
    },
    // YVYU
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, mmioFOURCC('Y', 'V', 'Y', 'U'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_YVYU                                              // subtype
    },
    // UYVY
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, mmioFOURCC('U', 'Y', 'V', 'Y'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_UYVY                                              // subtype
    },
    // YUY2
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, mmioFOURCC('Y', 'U', 'Y', '2'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_YUY2                                              // subtype
    },
    // YV12
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('Y', 'V', '1', '2'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_YV12                                              // subtype
    },
    // I420
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('I', '4', '2', '0'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_I420                                              // subtype
    },
    // IYUV
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('I', 'Y', 'U', 'V'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_IYUV                                              // subtype
    },
    // NV12
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('N', 'V', '1', '2'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_NV12                                              // subtype
    },
    // 8888 normal
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_RGB, 0, 0, 0, 0, 0}              // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_RGB32                                             // subtype
    },
    // 8888 bitf
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_BITFIELDS, 0, 0, 0, 0, 0}        // bmiHeader
        },
        {0xFF0000, 0x00FF00, 0x0000FF},                                 // mask[3]
        VIH_BITFIELDS,                                                  // size
        &MEDIASUBTYPE_RGB32                                             // subtype
    },
    // A888 normal
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_RGB, 0, 0, 0, 0, 0}              // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_ARGB32                                            // subtype
    },
    // A888 bitf (I'm not sure if this exist...)
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_BITFIELDS, 0, 0, 0, 0, 0}        // bmiHeader
        },
        {0xFF0000, 0x00FF00, 0x0000FF},                                 // mask[3]
        VIH_BITFIELDS,                                                  // size
        &MEDIASUBTYPE_ARGB32                                            // subtype
    },
    // 888 normal
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 24, BI_RGB, 0, 0, 0, 0, 0}              // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_RGB24                                             // subtype
    },
    // 888 bitf
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 24, BI_BITFIELDS, 0, 0, 0, 0, 0}        // bmiHeader
        },
        {0xFF0000, 0x00FF00, 0x0000FF},                                 // mask[3]
        VIH_BITFIELDS,                                                  // size
        &MEDIASUBTYPE_RGB24                                             // subtype
    },
    // 565 normal
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_RGB, 0, 0, 0, 0, 0}              // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_RGB565                                            // subtype
    },
    // 565 bitf
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_BITFIELDS, 0, 0, 0, 0, 0}        // bmiHeader
        },
        {0xF800, 0x07E0, 0x001F},                                       // mask[3]
        VIH_BITFIELDS,                                                  // size
        &MEDIASUBTYPE_RGB565                                            // subtype
    },
    // 555 normal
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_RGB, 0, 0, 0, 0, 0}              // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_RGB555                                            // subtype
    },
    // 555 bitf
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_BITFIELDS, 0, 0, 0, 0, 0}        // bmiHeader
        },
        {0x7C00, 0x03E0, 0x001F},                                       // mask[3]
        VIH_BITFIELDS,                                                  // size
        &MEDIASUBTYPE_RGB555                                            // subtype
    },
};

const VIH2 vih2s[] = {
    // VYUY
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, mmioFOURCC('V', 'Y', 'U', 'Y'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_VYUY                                              // subtype
    },
    // YVYU
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, mmioFOURCC('Y', 'V', 'Y', 'U'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_YVYU                                              // subtype
    },
    // UYVY
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, mmioFOURCC('U', 'Y', 'V', 'Y'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_UYVY                                              // subtype
    },
    // YUY2
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, mmioFOURCC('Y', 'U', 'Y', '2'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_YUY2                                              // subtype
    },
    // YV12
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('Y', 'V', '1', '2'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_YV12                                              // subtype
    },
    // I420
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('I', '4', '2', '0'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_I420                                              // subtype
    },
    // IYUV
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('I', 'Y', 'U', 'V'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_IYUV                                              // subtype
    },
    // NV12
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('N', 'V', '1', '2'), 0, 0, 0, 0, 0}  // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_NV12                                              // subtype
    },
    // 8888 normal
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_RGB, 0, 0, 0, 0, 0}              // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_RGB32                                             // subtype
    },
    // 8888 bitf
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_BITFIELDS, 0, 0, 0, 0, 0}        // bmiHeader
        },
        {0xFF0000, 0x00FF00, 0x0000FF},                                 // mask[3]
        VIH2_BITFIELDS,                                                 // size
        &MEDIASUBTYPE_RGB32                                             // subtype
    },
    // A888 normal
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_RGB, 0, 0, 0, 0, 0}              // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_ARGB32                                            // subtype
    },
    // A888 bitf (I'm not sure if this exist...)
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_BITFIELDS, 0, 0, 0, 0, 0}        // bmiHeader
        },
        {0xFF0000, 0x00FF00, 0x0000FF},                                 // mask[3]
        VIH2_BITFIELDS,                                                 // size
        &MEDIASUBTYPE_ARGB32                                            // subtype
    },
    // 888 normal
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 24, BI_RGB, 0, 0, 0, 0, 0}              // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_RGB24                                             // subtype
    },
    // 888 bitf
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 24, BI_BITFIELDS, 0, 0, 0, 0, 0}        // bmiHeader
        },
        {0xFF0000, 0x00FF00, 0x0000FF},                                 // mask[3]
        VIH2_BITFIELDS,                                                 // size
        &MEDIASUBTYPE_RGB24                                             // subtype
    },
    // 565 normal
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_RGB, 0, 0, 0, 0, 0}              // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_RGB565                                            // subtype
    },
    // 565 bitf
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_BITFIELDS, 0, 0, 0, 0, 0}        // bmiHeader
        },
        {0xF800, 0x07E0, 0x001F},                                       // mask[3]
        VIH2_BITFIELDS,                                                 // size
        &MEDIASUBTYPE_RGB565                                            // subtype
    },
    // 555 normal
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_RGB, 0, 0, 0, 0, 0}              // bmiHeader
        },
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_RGB555                                            // subtype
    },
    // 555 bitf
    {
        {
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_BITFIELDS, 0, 0, 0, 0, 0}        // bmiHeader
        },
        {0x7C00, 0x03E0, 0x001F},                                       // mask[3]
        VIH2_BITFIELDS,                                                 // size
        &MEDIASUBTYPE_RGB555                                            // subtype
    },
};

extern const UINT VIHSIZE = _countof(vihs);

CString VIH2String(int i)
{
    CString ret(GuidNames[*vihs[i].subtype]);
    if (!ret.Left(13).CompareNoCase(_T("MEDIASUBTYPE_"))) {
        ret = ret.Mid(13);
    }
    if (vihs[i].vih.bmiHeader.biCompression == 3) {
        ret += _T(" BITF");
    }
    if (*vihs[i].subtype == MEDIASUBTYPE_I420) {
        ret = _T("I420");    // FIXME
    }
    return ret;
}

CString Subtype2String(const GUID& subtype)
{
    CString ret(GuidNames[subtype]);
    if (!ret.Left(13).CompareNoCase(_T("MEDIASUBTYPE_"))) {
        ret = ret.Mid(13);
    }
    if (subtype == MEDIASUBTYPE_I420) {
        ret = _T("I420");    // FIXME
    }
    return ret;
}

void CorrectMediaType(AM_MEDIA_TYPE* pmt)
{
    if (!pmt) {
        return;
    }

    CMediaType mt(*pmt);

    if (mt.formattype == FORMAT_VideoInfo) {
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)mt.pbFormat;

        for (UINT i = 0; i < VIHSIZE; i++) {
            if (mt.subtype == *vihs[i].subtype
                    && vih->bmiHeader.biCompression == vihs[i].vih.bmiHeader.biCompression) {
                mt.AllocFormatBuffer(vihs[i].size);
                memcpy(mt.pbFormat, &vihs[i], vihs[i].size);
                memcpy(mt.pbFormat, pmt->pbFormat, sizeof(VIDEOINFOHEADER));
                break;
            }
        }
    } else if (mt.formattype == FORMAT_VideoInfo2) {
        VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)mt.pbFormat;

        for (UINT i = 0; i < VIHSIZE; i++) {
            if (mt.subtype == *vih2s[i].subtype
                    && vih2->bmiHeader.biCompression == vih2s[i].vih.bmiHeader.biCompression) {
                mt.AllocFormatBuffer(vih2s[i].size);
                memcpy(mt.pbFormat, &vih2s[i], vih2s[i].size);
                memcpy(mt.pbFormat, pmt->pbFormat, sizeof(VIDEOINFOHEADER2));
                break;
            }
        }
    }

    CopyMediaType(pmt, &mt);
}
