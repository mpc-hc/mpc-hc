// File_Flv - Info for Flash files
// Copyright (C) 2005-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Flash files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_FlvH
#define MediaInfo_File_FlvH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <map>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Flv
//***************************************************************************

class File_Flv : public File__Analyze
{
protected :
    //Streams management
    void Streams_Finish();

public :
    File_Flv();

private :
    //Buffer
    void FileHeader_Parse ();
    void Header_Parse();
    void Data_Parse();

    //Elements
    void header();
    void video();
    void video_H263();
    void video_ScreenVideo(int8u Version);
    void video_VP6(bool WithAlpha);
    void video_AVC();
    void audio();
    void audio_MPEG();
    void audio_AAC();
    void meta();
    void meta_SCRIPTDATAOBJECT();
    void meta_SCRIPTDATAVARIABLE();
    void meta_SCRIPTDATAVALUE(const std::string &StringData);
    void Rm();
    
    //Streams
    struct stream
    {
        File__Analyze*          Parser;
        size_t                  PacketCount;
        int32u                  Delay;

        stream()
        {
            Parser=NULL;
            PacketCount=0;
            Delay=(int32u)-1;
        }

        ~stream()
        {
            delete Parser; //Parser=NULL;
        }
    };
    std::vector<stream> Stream; //Null, Video, Audio

    //Count
    bool   video_stream_Count;
    bool   audio_stream_Count;

    //Temp
    bool   video_stream_FrameRate_Detected;
    std::vector<int32u> video_stream_FrameRate;
    int32u Time;
    int8u  meta_Level;
    std::map<int8u, bool> meta_LevelFinished;
    bool Searching_Duration;
    int32u PreviousTagSize;
    int64u meta_filesize;
    float64 meta_duration;
    int32u FirstFrame_Time;
    int8u  FirstFrame_Type;
    int32u LastFrame_Time;
    int8u  LastFrame_Type;
};

} //NameSpace

#endif
