/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef File__ReferenceFilesHelperH
#define File__ReferenceFilesHelperH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper_Sequence.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper_Common.h"
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File__ReferenceFilesHelper
//***************************************************************************

class File__ReferenceFilesHelper
{
public :
    //Constructor / Destructor
                                    File__ReferenceFilesHelper(File__Analyze* MI, MediaInfo_Config_MediaInfo* Config);
                                    ~File__ReferenceFilesHelper();

    //In
    void                            AddSequence(sequence* NewSequence);
    void                            UpdateFileName(const Ztring& OldFileName, const Ztring& NewFileName);
    bool                            TestContinuousFileNames;
    bool                            ContainerHasNoId;
    int64u                          ID_Max;

    //Streams management
    void ParseReferences();

private :
    sequences                       Sequences;
    size_t                          Sequences_Current;

    //Temp
    rfh_common*                     Common;



public:
    size_t                          Sequences_Size() {return Sequences.size();}
    void                            Clear() {Sequences.clear();}
    bool                            FilesForStorage;
    bool                            HasMainFile;
    bool                            HasMainFile_Filled;

    //Streams management
    bool ParseReference_Init();

    #if MEDIAINFO_SEEK
    size_t Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK

private :
    //Streams management
    void ParseReference ();
    void ParseReference_Finalize ();
    void ParseReference_Finalize_PerStream ();
    void Open_Buffer_Unsynch() {Read_Buffer_Unsynched();}

    //Buffer - Global
    void Read_Buffer_Unsynched();

    //temp
    File__Analyze*                  MI;
    MediaInfo_Config_MediaInfo*     Config;
    bool                            Init_Done;
    bool                            Demux_Interleave;
    size_t                          CountOfReferencesToParse;
    size_t                          CountOfReferences_ForReadSize;
    float64                         FrameRate;
    float64                         Duration;
    stream_t                        StreamKind_Last;
    size_t                          StreamPos_From;
    size_t                          StreamPos_To;
    #if MEDIAINFO_NEXTPACKET
        int64u                      DTS_Minimal;
        int64u                      DTS_Interval;
    #endif //MEDIAINFO_NEXTPACKET

    //Helpers
    size_t Stream_Prepare(stream_t StreamKind, size_t StreamPos=(size_t)-1);
    void   FileSize_Compute();
    void   CountOfReferences_ForReadSize_Run();
    MediaInfo_Internal* MI_Create();
    #if MEDIAINFO_ADVANCED || MEDIAINFO_MD5
        void   List_Compute();
    #endif //MEDIAINFO_ADVANCED || MEDIAINFO_MD5
    #if MEDIAINFO_EVENTS
    void SubFile_Start();
    int64u                          StreamID_Previous;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        int64u                      Offset_Video_DTS;
    #endif //MEDIAINFO_DEMUX
};

} //NameSpace

#endif
