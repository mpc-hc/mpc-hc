/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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

#pragma once

#include <vector>
#include <memory>
#include "../SubPic/ISubPic.h"

#define PT_MOVETONC         0xfe
#define PT_BSPLINETO        0xfc
#define PT_BSPLINEPATCHTO   0xfa

struct RasterizerNfo {
    int w;
    int h;
    int spdw;
    int overlayp;
    int pitch;
    DWORD color;

    int xo;
    int yo;

    const DWORD* sw;
    byte* s;
    byte* srcBody;
    byte* srcBorder;
    DWORD* dst;

    byte* am;

    RasterizerNfo(int w, int h, int xo, int yo, int overlayp, int spdw, int pitch, DWORD color,
                  const DWORD* sw, byte* s, byte* srcBody, byte* srcBorder, DWORD* dst, byte* am)
        : w(w)
        , h(h)
        , xo(xo)
        , yo(yo)
        , overlayp(overlayp)
        , spdw(spdw)
        , pitch(pitch)
        , color(color)
        , sw(sw)
        , s(s)
        , srcBody(srcBody)
        , srcBorder(srcBorder)
        , dst(dst)
        , am(am) {
    }
};

using tSpanBuffer = std::vector<std::pair<unsigned __int64, unsigned __int64>>;

struct COutlineData {
    int mWidth, mHeight;
    int mPathOffsetX, mPathOffsetY;
    int mWideBorder;
    tSpanBuffer mOutline, mWideOutline;

    COutlineData()
        : mWidth(0)
        , mHeight(0)
        , mPathOffsetX(0)
        , mPathOffsetY(0)
        , mWideBorder(0) {}
};

typedef std::shared_ptr<COutlineData> COutlineDataSharedPtr;

struct COverlayData {
    int mOffsetX, mOffsetY;
    int mOverlayWidth, mOverlayHeight, mOverlayPitch;
    byte* mpOverlayBufferBody, *mpOverlayBufferBorder;

    COverlayData()
        : mOffsetX(0)
        , mOffsetY(0)
        , mOverlayWidth(0)
        , mOverlayHeight(0)
        , mOverlayPitch(0)
        , mpOverlayBufferBody(nullptr)
        , mpOverlayBufferBorder(nullptr) {}

    COverlayData(const COverlayData& overlayData)
        : mOffsetX(overlayData.mOffsetX)
        , mOffsetY(overlayData.mOffsetY)
        , mOverlayWidth(overlayData.mOverlayWidth)
        , mOverlayHeight(overlayData.mOverlayHeight)
        , mOverlayPitch(overlayData.mOverlayPitch) {
        if (mOverlayPitch > 0 && mOverlayHeight > 0) {
            mpOverlayBufferBody = (byte*)_aligned_malloc(mOverlayPitch * mOverlayHeight, 16);
            mpOverlayBufferBorder = (byte*)_aligned_malloc(mOverlayPitch * mOverlayHeight, 16);
            if (!mpOverlayBufferBody || !mpOverlayBufferBorder) {
                mOffsetX = mOffsetY = 0;
                mOverlayWidth = mOverlayHeight = 0;
                DeleteOverlay();
            }
            memcpy(mpOverlayBufferBody, overlayData.mpOverlayBufferBody, mOverlayPitch * mOverlayHeight);
            memcpy(mpOverlayBufferBorder, overlayData.mpOverlayBufferBorder, mOverlayPitch * mOverlayHeight);
        } else {
            mpOverlayBufferBody = mpOverlayBufferBorder = nullptr;
        }
    }

    ~COverlayData() {
        DeleteOverlay();
    }

    COverlayData& operator=(const COverlayData& overlayData) {
        mOffsetX = overlayData.mOffsetX;
        mOffsetY = overlayData.mOffsetY;
        mOverlayWidth = overlayData.mOverlayWidth;
        mOverlayHeight = overlayData.mOverlayHeight;
        mOverlayPitch = overlayData.mOverlayPitch;

        DeleteOverlay();
        if (mOverlayPitch > 0 && mOverlayHeight > 0) {
            mpOverlayBufferBody = (byte*)_aligned_malloc(mOverlayPitch * mOverlayHeight, 16);
            mpOverlayBufferBorder = (byte*)_aligned_malloc(mOverlayPitch * mOverlayHeight, 16);
            if (!mpOverlayBufferBody || !mpOverlayBufferBorder) {
                mOffsetX = mOffsetY = 0;
                mOverlayWidth = mOverlayHeight = 0;
                DeleteOverlay();
            }
            memcpy(mpOverlayBufferBody, overlayData.mpOverlayBufferBody, mOverlayPitch * mOverlayHeight);
            memcpy(mpOverlayBufferBorder, overlayData.mpOverlayBufferBorder, mOverlayPitch * mOverlayHeight);
        } else {
            mpOverlayBufferBody = mpOverlayBufferBorder = nullptr;
        }

        return *this;
    };

    void DeleteOverlay() {
        if (mpOverlayBufferBody) {
            _aligned_free(mpOverlayBufferBody);
            mpOverlayBufferBody = nullptr;
        }
        if (mpOverlayBufferBorder) {
            _aligned_free(mpOverlayBufferBorder);
            mpOverlayBufferBorder = nullptr;
        }
    }
};

typedef std::shared_ptr<COverlayData> COverlayDataSharedPtr;

class Rasterizer
{
    bool fFirstSet;
    CPoint firstp, lastp;

protected:
    BYTE* mpPathTypes;
    POINT* mpPathPoints;
    int mPathPoints;
    bool m_bUseSSE2;

private:
    struct Edge {
        int next;
        int posandflag;
    }* mpEdgeBuffer;
    unsigned int mEdgeHeapSize;
    unsigned int mEdgeNext;

    unsigned int* mpScanBuffer;

protected:
    COutlineDataSharedPtr m_pOutlineData;
    COverlayDataSharedPtr m_pOverlayData;

private:
    void _TrashPath();
    void _ReallocEdgeBuffer(int edges);
    void _EvaluateBezier(int ptbase, bool fBSpline);
    void _EvaluateLine(int pt1idx, int pt2idx);
    void _EvaluateLine(int x0, int y0, int x1, int y1);
    static void _OverlapRegion(tSpanBuffer& dst, const tSpanBuffer& src, int dx, int dy);
    // helpers
    void Draw_noAlpha_spFF_Body_0(RasterizerNfo& rnfo);
    void Draw_noAlpha_spFF_noBody_0(RasterizerNfo& rnfo);
    void Draw_noAlpha_sp_Body_0(RasterizerNfo& rnfo);
    void Draw_noAlpha_sp_noBody_0(RasterizerNfo& rnfo);
    void Draw_noAlpha_spFF_Body_sse2(RasterizerNfo& rnfo);
    void Draw_noAlpha_spFF_noBody_sse2(RasterizerNfo& rnfo);
    void Draw_noAlpha_sp_Body_sse2(RasterizerNfo& rnfo);
    void Draw_noAlpha_sp_noBody_sse2(RasterizerNfo& rnfo);
    void Draw_Alpha_spFF_Body_0(RasterizerNfo& rnfo);
    void Draw_Alpha_spFF_noBody_0(RasterizerNfo& rnfo);
    void Draw_Alpha_sp_Body_0(RasterizerNfo& rnfo);
    void Draw_Alpha_sp_noBody_0(RasterizerNfo& rnfo);
    void Draw_Alpha_spFF_Body_sse2(RasterizerNfo& rnfo);
    void Draw_Alpha_spFF_noBody_sse2(RasterizerNfo& rnfo);
    void Draw_Alpha_sp_Body_sse2(RasterizerNfo& rnfo);
    void Draw_Alpha_sp_noBody_sse2(RasterizerNfo& rnfo);

public:
    Rasterizer();
    virtual ~Rasterizer();

    bool BeginPath(HDC hdc);
    bool EndPath(HDC hdc);
    bool PartialBeginPath(HDC hdc, bool bClearPath);
    bool PartialEndPath(HDC hdc, long dx, long dy);
    bool ScanConvert();
    bool CreateWidenedRegion(int borderX, int borderY);
    bool Rasterize(int xsub, int ysub, int fBlur, double fGaussianBlur);
    int getOverlayWidth();

    CRect Draw(SubPicDesc& spd, CRect& clipRect, byte* pAlphaMask, int xsub, int ysub, const DWORD* switchpts, bool fBody, bool fBorder);
    void FillSolidRect(SubPicDesc& spd, int x, int y, int nWidth, int nHeight, DWORD lColor);
};
