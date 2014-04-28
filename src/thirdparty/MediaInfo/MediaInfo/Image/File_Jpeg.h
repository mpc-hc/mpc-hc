/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about JPEG files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_JpegH
#define MediaInfo_File_JpegH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Jpeg
//***************************************************************************

class File_Jpeg : public File__Analyze
{
public :
    //In
    stream_t StreamKind;
    bool     Interlaced;

    //Constructor/Destructor
    File_Jpeg();

private :
    //Streams management
    void Streams_Accept();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();
    void Synched_Init();

    //Buffer - Demux
    #if MEDIAINFO_DEMUX
    bool Demux_UnpacketizeContainer_Test() {return Demux_UnpacketizeContainer_Test_OneFramePerFile();}
    #endif //MEDIAINFO_DEMUX

    //Buffer - Global
    void Read_Buffer_Unsynched();
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID) {return Read_Buffer_Seek_OneFramePerFile(Method, Value, ID);}
    #endif //MEDIAINFO_SEEK

    //Buffer - Per element
    void Header_Parse();
    bool Header_Parser_Fill_Size();
    void Data_Parse();

    //Elements
    void TEM () {};
    void SOC () {}
    void SIZ ();
    void COD ();
    void COC () {Skip_XX(Element_Size, "Data");}
    void TLM () {Skip_XX(Element_Size, "Data");}
    void PLM () {Skip_XX(Element_Size, "Data");}
    void PLT () {Skip_XX(Element_Size, "Data");}
    void QCD ();
    void QCC () {Skip_XX(Element_Size, "Data");}
    void RGN () {Skip_XX(Element_Size, "Data");}
    void PPM () {Skip_XX(Element_Size, "Data");}
    void PPT () {Skip_XX(Element_Size, "Data");}
    void CME () {Skip_XX(Element_Size, "Data");}
    void SOT () {Skip_XX(Element_Size, "Data");}
    void SOP () {Skip_XX(Element_Size, "Data");}
    void EPH () {Skip_XX(Element_Size, "Data");}
    void SOD ();
    void SOF_();
    void S0F0() {SOF_();};
    void S0F1() {SOF_();};
    void S0F2() {SOF_();};
    void S0F3() {SOF_();}
    void DHT () {Skip_XX(Element_Size, "Data");}
    void S0F5() {SOF_();}
    void S0F6() {SOF_();}
    void S0F7() {SOF_();}
    void JPG () {Skip_XX(Element_Size, "Data");}
    void S0F9() {SOF_();}
    void S0FA() {SOF_();}
    void S0FB() {SOF_();}
    void DAC () {Skip_XX(Element_Size, "Data");}
    void S0FD() {SOF_();}
    void S0FE() {SOF_();}
    void S0FF() {SOF_();}
    void RST0() {};
    void RST1() {};
    void RST2() {};
    void RST3() {};
    void RST4() {};
    void RST5() {};
    void RST6() {};
    void RST7() {};
    void SOI () {};
    void EOI () {};
    void SOS ();
    void DQT () {Skip_XX(Element_Size, "Data");}
    void DNL () {Skip_XX(Element_Size, "Data");}
    void DRI () {Skip_XX(Element_Size, "Data");}
    void DHP () {Skip_XX(Element_Size, "Data");}
    void EXP () {Skip_XX(Element_Size, "Data");}
    void APP0();
    void APP0_AVI1();
    void APP0_JFIF();
    void APP0_JFFF();
    void APP0_JFFF_JPEG();
    void APP0_JFFF_1B();
    void APP0_JFFF_3B();
    void APP1();
    void APP1_EXIF();
    void APP2() {Skip_XX(Element_Size, "Data");}
    void APP3() {Skip_XX(Element_Size, "Data");}
    void APP4() {Skip_XX(Element_Size, "Data");}
    void APP5() {Skip_XX(Element_Size, "Data");}
    void APP6() {Skip_XX(Element_Size, "Data");}
    void APP7() {Skip_XX(Element_Size, "Data");}
    void APP8() {Skip_XX(Element_Size, "Data");}
    void APP9() {Skip_XX(Element_Size, "Data");}
    void APPA() {Skip_XX(Element_Size, "Data");}
    void APPB() {Skip_XX(Element_Size, "Data");}
    void APPC() {Skip_XX(Element_Size, "Data");}
    void APPD() {Skip_XX(Element_Size, "Data");}
    void APPE();
    void APPE_Adobe0();
    void APPF() {Skip_XX(Element_Size, "Data");}
    void JPG0() {Skip_XX(Element_Size, "Data");}
    void JPG1() {Skip_XX(Element_Size, "Data");}
    void JPG2() {Skip_XX(Element_Size, "Data");}
    void JPG3() {Skip_XX(Element_Size, "Data");}
    void JPG4() {Skip_XX(Element_Size, "Data");}
    void JPG5() {Skip_XX(Element_Size, "Data");}
    void JPG6() {Skip_XX(Element_Size, "Data");}
    void JPG7() {Skip_XX(Element_Size, "Data");}
    void JPG8() {Skip_XX(Element_Size, "Data");}
    void JPG9() {Skip_XX(Element_Size, "Data");}
    void JPGA() {Skip_XX(Element_Size, "Data");}
    void JPGB() {Skip_XX(Element_Size, "Data");}
    void JPGC() {Skip_XX(Element_Size, "Data");}
    void JPGD() {Skip_XX(Element_Size, "Data");}
    void COM () {Skip_XX(Element_Size, "Data");}

    //Temp
    int8u APPE_Adobe0_transform;
    bool  APP0_JFIF_Parsed;
    bool  SOS_SOD_Parsed;
};

} //NameSpace

#endif
