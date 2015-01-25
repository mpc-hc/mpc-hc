/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about SDP streams
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_SdpH
#define MediaInfo_File_SdpH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Sdp
//***************************************************************************

class File_Teletext;

class File_Sdp : public File__Analyze
{
public :
    File_Sdp();
    ~File_Sdp();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Global
    void Read_Buffer_Unsynched();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Temp
    struct stream
    {
        File_Teletext*  Parser;

        stream()
            :
            Parser(NULL)
        {
        }
    };
    typedef std::map<int8u, stream> streams;
    streams Streams;
    int8u FieldLines[5];
};

} //NameSpace

#endif
