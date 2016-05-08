/*
* (C) 2014, 2016 see Authors.txt
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

struct SubPicQueueSettings {
    int  nSize;
    int  nMaxRes;
    bool bDisableSubtitleAnimation;
    int  nRenderAtWhenAnimationIsDisabled;
    int  nAnimationRate;
    bool bAllowDroppingSubpic;

    SubPicQueueSettings(int nSize, int nMaxRes,
                        bool bDisableSubtitleAnimation, int nRenderAtWhenAnimationIsDisabled, int nAnimationRate,
                        bool bAllowDroppingSubpic)
        : nSize(nSize)
        , nMaxRes(nMaxRes)
        , bDisableSubtitleAnimation(bDisableSubtitleAnimation)
        , nRenderAtWhenAnimationIsDisabled(nRenderAtWhenAnimationIsDisabled)
        , nAnimationRate(nAnimationRate)
        , bAllowDroppingSubpic(bAllowDroppingSubpic)
    {};

    SubPicQueueSettings()
        : SubPicQueueSettings(10, 0, false, 50, 100, true)
    {};
};
