/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Source: http://wiki.multimedia.cx/index.php?title=VQF
//
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
#if defined(MEDIAINFO_TWINVQ_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_TwinVQ.h"
#include "ZenLib/Utils.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
const char* TwinVQ_samplerate(int32u samplerate)
{
    switch (samplerate)
    {
        case 11 : return "11025";
        case 22 : return "22050";
        case 44 : return "44100";
        default : return "";
    }
}

//---------------------------------------------------------------------------
namespace Elements
{
    const int32u _c__=0x28632920;
    const int32u AUTH=0x41555448;
    const int32u COMM=0x434F4D4D;
    const int32u COMT=0x434F4D54;
    const int32u DATA=0x44415441;
    const int32u DSIZ=0x4453495A;
    const int32u FILE=0x46494C45;
    const int32u NAME=0x4E414D45;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_TwinVQ::FileHeader_Begin()
{
    //Testing
    if (Buffer_Offset+4>Buffer_Size)
        return false;
    if (CC4(Buffer+Buffer_Offset)!=0x5457494E) //"TWIN"
    {
        Reject("TwinVQ");
        return false;
    }

    //All should be OK...
    return true;
}

//---------------------------------------------------------------------------
void File_TwinVQ::FileHeader_Parse()
{
    //Parsing
    Skip_C4(                                                    "magic");
    Skip_Local(8,                                               "version");
    Skip_B4(                                                    "subchunks_size");

    FILLING_BEGIN();
        Accept("TwinVQ");

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "TwinVQ");
        Fill(Stream_Audio, 0, Audio_Codec, "TwinVQ");
    FILLING_END();
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_TwinVQ::Header_Parse()
{
    //Parsing
    int32u id, size;
    Get_C4 (id,                                                 "id");
    Get_B4 (size,                                               "size");

    //Filling
    Header_Fill_Code(id, Ztring().From_CC4(id));
    Header_Fill_Size(8+(id==Elements::DATA?0:size)); //DATA chunk indicates the end of the header, with no chunk size
}

//---------------------------------------------------------------------------
void File_TwinVQ::Data_Parse()
{
    #define ELEMENT_CASE(_NAME, _DETAIL) \
        case Elements::_NAME : Element_Info1(_DETAIL); _NAME(); break;

    //Parsing
    switch (Element_Code)
    {
        ELEMENT_CASE(_c__, "Copyright");
        ELEMENT_CASE(AUTH, "Author");
        ELEMENT_CASE(COMM, "Mandatory information");
        ELEMENT_CASE(COMT, "Comment");
        ELEMENT_CASE(DATA, "Data");
        ELEMENT_CASE(DSIZ, "Data size");
        ELEMENT_CASE(FILE, "Filename");
        ELEMENT_CASE(NAME, "Song title");
        default : Skip_XX(Element_Size,                         "Unknown");
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_TwinVQ::COMM()
{
    //Parsing
    int32u channel_mode, bitrate, samplerate;
    Get_B4 (channel_mode,                                       "channel_mode");
    Get_B4 (bitrate,                                            "bitrate");
    Get_B4 (samplerate,                                         "samplerate");
    Skip_B4(                                                    "security_level");

    //Filling
    Fill(Stream_Audio, 0, Audio_Channel_s_, channel_mode+1);
    Fill(Stream_Audio, 0, Audio_BitRate, bitrate*1000);
    Fill(Stream_Audio, 0, Audio_SamplingRate, TwinVQ_samplerate(samplerate));
    if (!IsSub && File_Size!=(int64u)-1)
        Fill(Stream_Audio, 0, Audio_StreamSize, File_Size);
}

//---------------------------------------------------------------------------
void File_TwinVQ::DATA()
{
    //This is the end of the parsing (DATA chunk format is unknown)
    Finish("TwinVQ");
}

//---------------------------------------------------------------------------
void File_TwinVQ::DSIZ()
{
    //Parsing
    Skip_B4(                                                    "Value");
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_TwinVQ::_____char()
{
    //Parsing
    Skip_Local(Element_Size,                                    "Value");
}

//---------------------------------------------------------------------------
void File_TwinVQ::_____char(const char* Parameter)
{
    //Parsing
    Ztring Value;
    Get_Local(Element_Size, Value,                              "Value");

    //Filling
    Fill(Stream_General, 0, Parameter, Value);
}

} //Namespace

#endif //MEDIAINFO_TWINVQ_YES

