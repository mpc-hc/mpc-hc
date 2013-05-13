/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about SCTE 20 streams
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Scte20H
#define MediaInfo_Scte20H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Scte20
//***************************************************************************

class File_Scte20 : public File__Analyze
{
public :
    //In
    int8u picture_structure;
    bool  progressive_sequence;
    bool  progressive_frame;
    bool  top_field_first;
    bool  repeat_first_field;

    //Constructor/Destructor
    File_Scte20();
    ~File_Scte20();

private :
    //Streams management
    void Streams_Update();
    void Streams_Update_PerStream(size_t Pos);
    void Streams_Finish();

    //Synchro
    void Read_Buffer_Unsynched();

    //Buffer - Global
    void Read_Buffer_Continue();

    //Temp
    struct stream
    {
        File__Analyze*  Parser;
        size_t          StreamPos;
        bool            IsFilled;

        stream()
        {
            Parser=NULL;
            StreamPos=(size_t)-1;
            IsFilled=false;
        }

        ~stream()
        {
            delete Parser; //Parser=NULL;
        }
    };
    std::vector<stream*> Streams;
    size_t               Streams_Count;
};

} //NameSpace

#endif

