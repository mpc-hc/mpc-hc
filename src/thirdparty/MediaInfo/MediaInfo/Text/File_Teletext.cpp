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
#if defined(MEDIAINFO_DVBSUBTITLE_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_Teletext.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Teletext::File_Teletext()
:File__Analyze()
{
    //Configuration
    ParserName=__T("Teletext");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Teletext;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    PTS_DTS_Needed=true;
    IsRawStream=true;
}

//---------------------------------------------------------------------------
File_Teletext::~File_Teletext()
{
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Teletext::Streams_Fill()
{
    Stream_Prepare(Stream_Text);
}

//---------------------------------------------------------------------------
void File_Teletext::Streams_Finish()
{
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Teletext::Read_Buffer_Continue()
{
    Fill();
    Finish();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_DVBSUBTITLE_YES
