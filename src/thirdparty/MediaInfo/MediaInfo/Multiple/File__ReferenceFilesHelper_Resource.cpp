/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_REFERENCES_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper_Sequence_Common.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper_Common.h"
#include "ZenLib/FileName.h"
#include "ZenLib/Format/Http/Http_Utils.h"
#if MEDIAINFO_AES
    #include "base64.h"
#endif //MEDIAINFO_AES
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events_Internal.h"
    #include "MediaInfo/MediaInfo_Config_PerPackage.h"
#endif //MEDIAINFO_EVENTS
using namespace std;
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

resource::resource()
{
    //In
    FileNames.Separator_Set(0, __T(","));
    EditRate=0;
    IgnoreEditsBefore=0;
    IgnoreEditsAfter=(int64u)-1;

    //Config
    Sequence=NULL;
    #if MEDIAINFO_NEXTPACKET
        Demux_Offset_Frame=(int64u)-1;
        Demux_Offset_DTS=(int64u)-1;
    #endif //MEDIAINFO_NEXTPACKET

    //Private
    MI=NULL;



    IgnoreEditsAfterDuration=(int64u)-1;
    #if MEDIAINFO_DEMUX
        Demux_Offset_FileSize=0;
    #endif //MEDIAINFO_DEMUX
}

resource::~resource()
{
    delete MI;
}

//***************************************************************************
// In
//***************************************************************************

//---------------------------------------------------------------------------
void resource::UpdateFileName(const Ztring& OldFileName, const Ztring& NewFileName)
{
    size_t FileNames_Size=FileNames.size();
    for (size_t Pos=0; Pos<FileNames_Size; Pos++)
        if (FileNames[Pos]==OldFileName)
            FileNames[Pos]=NewFileName;
}

} //NameSpace

#endif //MEDIAINFO_REFERENCES_YES
