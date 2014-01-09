/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Ancillary data (SMPTE ST291)
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_AncillaryH
#define MediaInfo_AncillaryH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Ancillary
//***************************************************************************

class File_Ancillary : public File__Analyze
{
public :
    //In
    bool    WithTenBit;
    bool    WithChecksum;
    bool    HasBFrames;
    bool    InDecodingOrder;
    bool    LineNumber_IsSecondField;
    float64 AspectRatio;
    float64 FrameRate;
    int32u  LineNumber;

    //In/Out
    struct buffered_data
    {
        size_t Size;
        int8u* Data;

        buffered_data()
        {
            Size=0;
            Data=NULL;
        }

        ~buffered_data()
        {
            delete[] Data; //Data=NULL;
        }
    };
    #if defined(MEDIAINFO_CDP_YES)
        std::vector<buffered_data*> Cdp_Data;
        File__Analyze*  Cdp_Parser;
    #endif //defined(MEDIAINFO_CDP_YES)
    #if defined(MEDIAINFO_AFDBARDATA_YES)
        std::vector<buffered_data*> AfdBarData_Data;
    #endif //defined(MEDIAINFO_AFDBARDATA_YES)
    #if defined(MEDIAINFO_ARIBSTDB24B37_YES)
        File__Analyze*  AribStdB34B37_Parser;
    #endif //defined(MEDIAINFO_ARIBSTDB24B37_YES)

    //Constructor/Destructor
    File_Ancillary();
    ~File_Ancillary();

private :
    //Streams management
    void Streams_Finish();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Global
    void Read_Buffer_Continue();
    void Read_Buffer_AfterParsing();
    void Read_Buffer_Unsynched();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Temp
    int8u DataID;
    int8u SecondaryDataID;
    int8u DataCount;
};

} //NameSpace

#endif

