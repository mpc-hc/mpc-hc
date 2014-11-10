/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Teletext streams
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_TeletextH
#define MediaInfo_File_TeletextH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <bitset>
#include <string>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Teletext
//***************************************************************************

class File_Teletext : public File__Analyze
{
public :
    File_Teletext();
    ~File_Teletext();

    //In
    #if defined(MEDIAINFO_MPEGPS_YES)
        bool FromMpegPs;
    #endif
    bool IsSubtitle;

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();
    void Synched_Init();

    //Buffer - Global
    void Read_Buffer_Unsynched();
    void Read_Buffer_Continue();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Character_Fill(wchar_t Character);
    void HasChanged();

    //Streams
    struct stream
    {
        vector<std::wstring> CC_Displayed_Values;

        stream()
        {
            CC_Displayed_Values.resize(26);
            for (size_t PosY=0; PosY<26; ++PosY)
                CC_Displayed_Values[PosY].resize(40, L' ');
        }

        void Clear()
        {
            for (size_t PosY=0; PosY<26; ++PosY)
                for (size_t PosX=0; PosX<40; ++PosX)
                    CC_Displayed_Values[PosY][PosX]=L' ';
        }

    };
    typedef map<int16u, stream> streams; //Key is Magazine+PageNumber
    streams Streams;
    int16u Stream_HasChanged;

    //Temp
    int8u   X;
    int8u   Y;
    std::bitset<16> C;
    int8u   PageNumber;
    int16u  SubCode;
    int64u  End;

    //Ancillary
    #if defined(MEDIAINFO_MPEGPS_YES)
        File_Teletext* Parser;
    #endif
};

} //NameSpace

#endif
