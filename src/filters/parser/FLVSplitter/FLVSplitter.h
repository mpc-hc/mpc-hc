/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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

#include <atlcoll.h>
#include "../BaseSplitter/BaseSplitter.h"
#include "../../transform/BaseVideoFilter/BaseVideoFilter.h"

#define FlvSplitterName L"MPC FLV Splitter"
#define FlvSourceName   L"MPC FLV Source"

class __declspec(uuid("47E792CF-0BBE-4F7A-859C-194B0768650A"))
    CFLVSplitterFilter : public CBaseSplitterFilter
{
    UINT32 m_DataOffset;
    bool m_IgnorePrevSizes;

    bool Sync(__int64& pos);

    struct VideoTweak {
        BYTE x;
        BYTE y;
    };

    bool ReadTag(VideoTweak& t);

    struct Tag {
        UINT32 PreviousTagSize;
        BYTE TagType;
        UINT32 DataSize;
        UINT32 TimeStamp;
        UINT32 StreamID;
    };

    bool ReadTag(Tag& t);

    struct AudioTag {
        BYTE SoundFormat;
        BYTE SoundRate;
        BYTE SoundSize;
        BYTE SoundType;
    };

    bool ReadTag(AudioTag& at);

    struct VideoTag {
        BYTE FrameType;
        BYTE CodecID;
    };

    bool ReadTag(VideoTag& vt);

    void NormalSeek(REFERENCE_TIME rt);
    void AlternateSeek(REFERENCE_TIME rt);

protected:
    CAutoPtr<CBaseSplitterFileEx> m_pFile;
    HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

    bool DemuxInit();
    void DemuxSeek(REFERENCE_TIME rt);
    bool DemuxLoop();

public:
    CFLVSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);

    // CBaseFilter
    STDMETHODIMP_(HRESULT) QueryFilterInfo(FILTER_INFO* pInfo);

};

class __declspec(uuid("C9ECE7B3-1D8E-41F5-9F24-B255DF16C087"))
    CFLVSourceFilter : public CFLVSplitterFilter
{
public:
    CFLVSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};
