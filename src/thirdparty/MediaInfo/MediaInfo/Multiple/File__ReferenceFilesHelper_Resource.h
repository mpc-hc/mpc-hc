/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef File__ReferenceFilesHelper_ResourceH
#define File__ReferenceFilesHelper_ResourceH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include <vector>
#if MEDIAINFO_EVENTS
    #include <set>
#endif //MEDIAINFO_EVENTS
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class rfhs_common;

class resource
{
public:
    //Constructor/Desctructor
                                    resource();
                                    ~resource();

    //In
    void                            UpdateFileName(const Ztring& OldFileName, const Ztring& NewFileName);
    ZtringList                      FileNames; //Source file name (relative path)
    float64                         EditRate;
    int64u                          IgnoreEditsBefore;
    int64u                          IgnoreEditsAfter;

    //Config
    rfhs_common*                    Sequence;
    #if MEDIAINFO_NEXTPACKET
        int64u                      Demux_Offset_Frame;
        int64u                      Demux_Offset_DTS;
    #endif //MEDIAINFO_NEXTPACKET

    MediaInfo_Internal*             MI;



    int64u  IgnoreEditsAfterDuration; //temporary value, some formats have duration instead of frame position
    #if MEDIAINFO_DEMUX
        int64u Demux_Offset_FileSize;
    #endif //MEDIAINFO_DEMUX
};

typedef std::vector<resource*> resources;

} //NameSpace

#endif
