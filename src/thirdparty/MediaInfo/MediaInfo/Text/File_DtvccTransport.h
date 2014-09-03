/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Bar Data files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_DtvccTransportH
#define MediaInfo_DtvccTransportH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_DtvccTransport
//***************************************************************************

class File_DtvccTransport : public File__Analyze
{
public :
    //In
    enum format
    {
        Format_Unknown,
        Format_A53_4_GA94_03,   //MPEG_cc_data
        Format_DVD,             //Unknown standard
    };
    format Format;
    float64 AspectRatio;

    //Constructor/Destructor
    File_DtvccTransport();
    ~File_DtvccTransport();

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

        stream()
        {
            Parser=NULL;
        }

        ~stream()
        {
            delete Parser; //Parser=NULL;
        }
    };
    std::vector<stream*> Streams;

    //Helpers
    void  CreateStream(int8u Parser_Pos);
};

} //NameSpace

#endif

