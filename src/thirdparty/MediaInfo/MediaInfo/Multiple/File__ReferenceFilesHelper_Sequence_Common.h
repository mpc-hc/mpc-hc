/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef File__ReferenceFilesHelper_Sequence_CommonH
#define File__ReferenceFilesHelper_Sequence_CommonH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Const.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class rfh_common;

//***************************************************************************
// Class resource
//***************************************************************************

class rfhs_common
{
public:
    rfhs_common(stream_t* StreamKind_p, size_t* StreamPos_p, int64u* StreamID_p, int64u* DTS_p, bool* Enabled_p, bool* IsMain_p):
        Package(NULL),
        FileSize((int64u)-1),
        Edits((int64u)-1),
        DemuxRate(0),
        HasMultipleResources(false),
        StreamKind(StreamKind_p),
        StreamPos(StreamPos_p),
        StreamID(StreamID_p),
        DTS(DTS_p),
        Enabled(Enabled_p),
        IsMain(IsMain_p)
    {
    }

    //Data
    rfh_common*                     Package;
    int64u                          FileSize;
    int64u                          Edits;
    float64                         DemuxRate;
    bool                            HasMultipleResources;
    const stream_t                  StreamKind_Get()                            {return *StreamKind;}
    void                            StreamKind_Set(stream_t StreamKind_n)       {*StreamKind=StreamKind_n;}
    const size_t                    StreamPos_Get()                             {return *StreamPos;}
    void                            StreamPos_Set(size_t StreamPos_n)           {*StreamPos=StreamPos_n;}
    const int64u                    StreamID_Get()                              {return *StreamID;}
    void                            StreamID_Set(int64u StreamID_n)             {*StreamID=StreamID_n;}
    const int64u                    DTS_Get()                                   {return *DTS;}
    void                            DTS_Set(int64u DTS_n)                       {*DTS=DTS_n;}
    const bool                      Enabled_Get()                               {return *Enabled;}
    const bool                      IsMain_Get()                                {return *IsMain;}

private:
    stream_t*                       StreamKind;
    size_t*                         StreamPos;
    int64u*                         StreamID;
    int64u*                         DTS;
    bool*                           Enabled;
    bool*                           IsMain;
};

} //NameSpace

#endif
