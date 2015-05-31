/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef File__ReferenceFilesHelper_CommonH
#define File__ReferenceFilesHelper_CommonH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class File__ReferenceFilesHelper;
class File__Analyze;
class MediaInfo_Config_MediaInfo;

//***************************************************************************
// Class resource
//***************************************************************************

class rfh_common
{
public:
    #if MEDIAINFO_DEMUX
    rfh_common(bool* Demux_Interleave_p, int64u* DTS_Minimal):
    #else //MEDIAINFO_DEMUX
    rfh_common():
    #endif //MEDIAINFO_DEMUX
        MI(NULL),
        Config(NULL),
        FileSize((int64u)-1),
        HasMultipleSequences(false),
        HasMainFile(false),
        HasMainFile_Filled(false),
        ContainerHasNoId(false)
        #if MEDIAINFO_DEMUX
            ,
            Demux_Interleave(Demux_Interleave_p),
            DTS_Minimal(DTS_Minimal)
        #endif //MEDIAINFO_DEMUX
    {
    }

    //Data
    File__ReferenceFilesHelper*     ReferenceFilesHelper;
    File__Analyze*                  MI;
    MediaInfo_Config_MediaInfo*     Config;
    int64u                          FileSize;
    bool                            HasMultipleSequences;
    bool                            HasMainFile;
    bool                            HasMainFile_Filled;
    bool                            ContainerHasNoId;
    #if MEDIAINFO_DEMUX
    const bool                      Demux_Interleave_Get()                          {return *Demux_Interleave;}
    void                            Demux_Interleave_Set(bool Demux_Interleave_n)   {*Demux_Interleave=Demux_Interleave_n;}
    const int64u                    DTS_Minimal_Get()                               {return *DTS_Minimal;}
    void                            DTS_Minimal_Set(size_t DTS_Minimal_n)           {*DTS_Minimal=DTS_Minimal_n;}
    #endif //MEDIAINFO_DEMUX

private:
    #if MEDIAINFO_DEMUX
        bool*                       Demux_Interleave;
        int64u*                     DTS_Minimal;
    #endif //MEDIAINFO_DEMUX
};

} //NameSpace

#endif
