// File_Hls - Info for HLS files
// Copyright (C) 2010-2012 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
#if defined(MEDIAINFO_HLS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Hls.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "ZenLib/Dir.h"
#include "ZenLib/FileName.h"
#include "ZenLib/ZtringList.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Hls::File_Hls()
:File__Analyze()
{
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Hls;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_EventWasSent_Accept_Specific=true;
    #endif //MEDIAINFO_DEMUX

    //Temp
    ReferenceFiles=NULL;
}

//---------------------------------------------------------------------------
File_Hls::~File_Hls()
{
    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Hls::Streams_Finish()
{
    if (ReferenceFiles==NULL)
        return;

    ReferenceFiles->ParseReferences();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_Hls::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    if (ReferenceFiles==NULL)
        return 0;

    return ReferenceFiles->Read_Buffer_Seek(Method, Value, ID);
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Hls::FileHeader_Begin()
{
    //Element_Size
    if (File_Size>1024*1024 || File_Size<10)
    {
        Reject("HLS");
        return false; //HLS files are not big
    }

    if (Buffer_Size<File_Size)
        return false; //Wait for complete file

    Ztring Document; Document.From_UTF8((char*)Buffer, Buffer_Size);
    ZtringList Lines;
    size_t LinesSeparator_Pos=Document.find_first_of(__T("\r\n"));
    if (LinesSeparator_Pos>File_Size-1)
    {
        Reject("HLS");
        return false;
    }
    Ztring LinesSeparator;
    if (Document[LinesSeparator_Pos]==__T('\r') && Document[LinesSeparator_Pos+1]==__T('\n'))
        LinesSeparator=__T("\r\n");
    else if (Document[LinesSeparator_Pos]==__T('\r'))
        LinesSeparator=__T("\r");
    else if (Document[LinesSeparator_Pos]==__T('\n'))
        LinesSeparator=__T("\n");
    else
    {
        Reject("HLS");
        return false;
    }
    Lines.Separator_Set(0, LinesSeparator);
    Lines.Write(Document);

    if (Lines(0)!=__T("#EXTM3U"))
    {
        Reject("HLS");
        return false;
    }

    Accept("HLS");
    Fill(Stream_General, 0, General_Format, "HLS");

    ReferenceFiles=new File__ReferenceFilesHelper(this, Config);

    File__ReferenceFilesHelper::reference ReferenceFile;

    bool IsGroup=false;
    for (size_t Line=0; Line<Lines.size(); Line++)
    {
        if (!Lines[Line].empty())
        {
            if (Lines[Line].find(__T("#EXT-X-STREAM-INF"))==0)
                IsGroup=true;
            else if (Lines[Line][0]==__T('#'))
                ;
            else
            {
                if (IsGroup)
                {
                    File__ReferenceFilesHelper::reference ReferenceStream;
                    ReferenceStream.FileNames.push_back(Lines[Line]);
                    ReferenceStream.StreamID=ReferenceFiles->References.size()+1;
                    ReferenceFiles->References.push_back(ReferenceStream);
                    IsGroup=false;
                    #if MEDIAINFO_EVENTS
                        ParserIDs[0]=MediaInfo_Parser_HlsIndex;
                        StreamIDs_Width[0]=sizeof(size_t)*2;
                    #endif //MEDIAINFO_EVENTS
                }
                else
                    ReferenceFile.FileNames.push_back(Lines[Line]);
            }
        }
    }

    if (ReferenceFiles->References.empty())
    {
        ReferenceFiles->References.push_back(ReferenceFile);
        ReferenceFiles->TestContinuousFileNames=true;
    }

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_HLS_YES

