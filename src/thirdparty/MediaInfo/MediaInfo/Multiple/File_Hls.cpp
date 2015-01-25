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
#if defined(MEDIAINFO_HLS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Hls.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "ZenLib/File.h"
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
    if (Document[LinesSeparator_Pos]==__T('\r') && LinesSeparator_Pos+1<Document.size() && Document[LinesSeparator_Pos+1]==__T('\n'))
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
    if (!IsSub)
        ReferenceFiles->ContainerHasNoId=true;

    File__ReferenceFilesHelper::reference ReferenceFile;

    bool IsGroup=false;
    for (size_t Line=0; Line<Lines.size(); Line++)
    {
        if (!Lines[Line].empty())
        {
            if (Lines[Line].find(__T("#EXT-X-KEY:"))==0)
            {
                ZtringListList List;
                List.Separator_Set(0, __T(","));
                List.Separator_Set(1, __T("="));
                List.Write(Lines[Line].substr(11, string::npos));
                for (size_t Pos=0; Pos<List.size(); ++Pos)
                {
                    if (List[Pos](0)==__T("METHOD"))
                    {
                        if (List[Pos](1).find(__T("AES-128"))==0)
                        {
                            Fill(Stream_General, 0, General_Encryption_Format, "AES");
                            Fill(Stream_General, 0, General_Encryption_Length, "128");
                            Fill(Stream_General, 0, General_Encryption_Method, "Segment");
                            Fill(Stream_General, 0, General_Encryption_Mode, "CBC");
                            Fill(Stream_General, 0, General_Encryption_Padding, "PKCS7");
                            Fill(Stream_General, 0, General_Encryption_InitializationVector, "Sequence number");

                            #if MEDIAINFO_AES
                            //Trying to get the key from FileName.FileExt.key
                            if (Config->Encryption_Key_Get().empty())
                            {
                                File KeyFile;
                                if (KeyFile.Open(File_Name+__T(".key")))
                                {
                                    if (KeyFile.Size_Get()==16)
                                    {
                                        int8u Key[16];
                                        if (KeyFile.Read(Key, 16)==16)
                                            Config->Encryption_Key_Set(Key, 16);
                                    }
                                    else
                                        Fill(Stream_General, 0, "Encryption_Key_Problem", KeyFile.Size_Get());
                                }
                            }
                            #endif
                        }
                        Fill(Stream_General, 0, General_Encryption, List[Pos](1));
                    }
                }
            }
            else if (Lines[Line].find(__T("#EXT-X-STREAM-INF:"))==0)
            {
                IsGroup=true;
            }
            else if (Lines[Line][0]==__T('#'))
                ;
            else
            {
                if (IsGroup)
                {
                    ReferenceFile.FileNames.push_back(Lines[Line]);
                    ReferenceFile.StreamID=ReferenceFiles->References.size()+1;
                    ReferenceFiles->References.push_back(ReferenceFile);
                    IsGroup=false;
                    ReferenceFile=File__ReferenceFilesHelper::reference();
                    #if MEDIAINFO_EVENTS
                        ParserIDs[0]=MediaInfo_Parser_HlsIndex;
                        StreamIDs_Width[0]=sizeof(size_t);
                    #endif //MEDIAINFO_EVENTS
                }
                else
                    ReferenceFile.FileNames.push_back(Lines[Line]);
            }
        }
    }

    if (!ReferenceFile.FileNames.empty())
    {
        ReferenceFiles->References.push_back(ReferenceFile);
        Fill(Stream_General, 0, General_Format_Profile, "Media");
    }
    else
    {
        Fill(Stream_General, 0, General_Format_Profile, "Master");
    }

    Element_Offset=File_Size;

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_HLS_YES

