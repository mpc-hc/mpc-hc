/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Caption Distribution Packet files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_CdpH
#define MediaInfo_File_CdpH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#if defined(MEDIAINFO_EIA608_YES)
    #include "MediaInfo/Text/File_Eia608.h"
#endif
#if defined(MEDIAINFO_EIA708_YES)
    #include "MediaInfo/Text/File_Eia708.h"
#endif
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Cdp
//***************************************************************************

class File_Cdp : public File__Analyze
{
public :
    //In
    bool    WithAppleHeader;
    float64 AspectRatio;

    //Constructor/Destructor
    File_Cdp();
    ~File_Cdp();

private :
    //Streams management
    void Streams_Accept();
    void Streams_Update();
    void Streams_Update_PerStream(size_t Pos);
    void Streams_Finish();

    //Synchro
    void Read_Buffer_Unsynched();

    //Buffer - Global
    void Read_Buffer_Continue();

    //Elements
    void cdp_header();
    void time_code_section();
    void ccdata_section();
    void ccsvcinfo_section();
    void cdp_footer();
    void future_section();

    //Stream
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

    //Temp
    int8u cdp_frame_rate;

    //Helpers
    void  CreateStream(int8u Parser_Pos);

    //Tests
    int8u cdp_length_Min;
    int8u cdp_length_Max;
};

} //NameSpace

#endif
