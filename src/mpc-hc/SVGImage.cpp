/*
* (C) 2016-2017 see Authors.txt
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
#include "SVGImage.h"
#include "mplayerc.h"
#include <atlimage.h>
#define NANOSVG_IMPLEMENTATION
#include <nanosvg/src/nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvg/src/nanosvgrast.h>

namespace
{
    HRESULT NSVGimageToCImage(NSVGimage* svgImage, CImage& image, float scale)
    {
        image.Destroy();

        if (!svgImage) {
            return E_FAIL;
        }

        NSVGrasterizer* rasterizer = nsvgCreateRasterizer();
        if (!rasterizer) {
            nsvgDelete(svgImage);
            return E_FAIL;
        }

        if (!image.Create(int(svgImage->width * scale), int(svgImage->height * scale), 32)) {
            nsvgDeleteRasterizer(rasterizer);
            nsvgDelete(svgImage);
            return E_FAIL;
        }

        nsvgRasterize(rasterizer, svgImage, 0.0f, 0.0f, scale,
                      static_cast<unsigned char*>(image.GetBits()),
                      image.GetWidth(), image.GetHeight(), image.GetPitch());

        // NanoSVG outputs RGBA but we need BGRA so we swap red and blue
        BYTE* bits = static_cast<BYTE*>(image.GetBits());
        for (int y = 0; y < image.GetHeight(); y++, bits += image.GetPitch()) {
            RGBQUAD* p = reinterpret_cast<RGBQUAD*>(bits);
            for (int x = 0; x < image.GetWidth(); x++) {
                std::swap(p[x].rgbRed, p[x].rgbBlue);
            }
        }

        nsvgDeleteRasterizer(rasterizer);
        nsvgDelete(svgImage);

        return S_OK;
    }
}

HRESULT SVGImage::Load(LPCTSTR filename, CImage& image, float scale /*= 1.0f*/)
{
    return NSVGimageToCImage(nsvgParseFromFile(CStringA(filename), "px", 96.0f), image, scale);
}

HRESULT SVGImage::Load(UINT uResId, CImage& image, float scale /*= 1.0f*/)
{
    CStringA svg;
    if (!LoadResource(uResId, svg, _T("SVG"))) {
        return E_FAIL;
    }

    return NSVGimageToCImage(nsvgParse(svg.GetBuffer(), "px", 96.0f), image, scale);
}
