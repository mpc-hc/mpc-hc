// File_Mpeg4_Descriptors - Info for MPEG files
// Copyright (C) 2007-2007 Jerome Martinez, Zen@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about MPEG-4 files, Descriptors
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Mpeg4_DescriptorsH
#define MediaInfo_Mpeg4_DescriptorsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Mpeg4_Descriptors
//***************************************************************************

class File_Mpeg4_Descriptors : public File__Analyze
{
public :
    //In
    stream_t KindOfStream;
    bool     Parser_DoNotFreeIt; //If you want to keep the Parser
    bool     SLConfig_DoNotFreeIt; //If you want to keep the SLConfig

    //Out
    File__Analyze* Parser;
    int16u ES_ID;

    struct slconfig
    {
        bool   useAccessUnitStartFlag;
        bool   useAccessUnitEndFlag;
        bool   useRandomAccessPointFlag;
        bool   hasRandomAccessUnitsOnlyFlag;
        bool   usePaddingFlag;
        bool   useTimeStampsFlag;
        bool   useIdleFlag;
        bool   durationFlag;
        int32u timeStampResolution;
        int32u OCRResolution;
        int8u  timeStampLength;
        int8u  OCRLength;
        int8u  AU_Length;
        int8u  instantBitrateLength;
        int8u  degradationPriorityLength;
        int8u  AU_seqNumLength;
        int8u  packetSeqNumLength;

        int32u timeScale;
        int16u accessUnitDuration;
        int16u compositionUnitDuration;

        int64u startDecodingTimeStamp;
        int64u startCompositionTimeStamp;
    };

    slconfig* SLConfig;

public :
    //Constructor/Destructor
    File_Mpeg4_Descriptors();
    ~File_Mpeg4_Descriptors();

private :
    //Buffer
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Descriptor_00() {Skip_XX(Element_Size, "Data");};
    void Descriptor_01();
    void Descriptor_02() {Descriptor_01();}
    void Descriptor_03();
    void Descriptor_04();
    void Descriptor_05();
    void Descriptor_06();
    void Descriptor_07() {Skip_XX(Element_Size, "Data");};
    void Descriptor_08() {Skip_XX(Element_Size, "Data");};
    void Descriptor_09();
    void Descriptor_0A() {Skip_XX(Element_Size, "Data");};
    void Descriptor_0B() {Skip_XX(Element_Size, "Data");};
    void Descriptor_0C() {Skip_XX(Element_Size, "Data");};
    void Descriptor_0D() {Skip_XX(Element_Size, "Data");};
    void Descriptor_0E();
    void Descriptor_0F();
    void Descriptor_10();
    void Descriptor_11();
    void Descriptor_12() {Skip_XX(Element_Size, "Data");};
    void Descriptor_13() {Skip_XX(Element_Size, "Data");};
    void Descriptor_14() {Skip_XX(Element_Size, "Data");};
    void Descriptor_40() {Skip_XX(Element_Size, "Data");};
    void Descriptor_41() {Skip_XX(Element_Size, "Data");};
    void Descriptor_42() {Skip_XX(Element_Size, "Data");};
    void Descriptor_43() {Skip_XX(Element_Size, "Data");};
    void Descriptor_44() {Skip_XX(Element_Size, "Data");};
    void Descriptor_45() {Skip_XX(Element_Size, "Data");};
    void Descriptor_46() {Skip_XX(Element_Size, "Data");};
    void Descriptor_47() {Skip_XX(Element_Size, "Data");};
    void Descriptor_48() {Skip_XX(Element_Size, "Data");};
    void Descriptor_49() {Skip_XX(Element_Size, "Data");};
    void Descriptor_4A() {Skip_XX(Element_Size, "Data");};
    void Descriptor_4B() {Skip_XX(Element_Size, "Data");};
    void Descriptor_4C() {Skip_XX(Element_Size, "Data");};
    void Descriptor_60() {Skip_XX(Element_Size, "Data");};
    void Descriptor_61() {Skip_XX(Element_Size, "Data");};
    void Descriptor_62() {Skip_XX(Element_Size, "Data");};
    void Descriptor_63() {Skip_XX(Element_Size, "Data");};
    void Descriptor_64() {Skip_XX(Element_Size, "Data");};
    void Descriptor_65() {Skip_XX(Element_Size, "Data");};
    void Descriptor_66() {Skip_XX(Element_Size, "Data");};
    void Descriptor_67() {Skip_XX(Element_Size, "Data");};
    void Descriptor_68() {Skip_XX(Element_Size, "Data");};
    void Descriptor_69() {Skip_XX(Element_Size, "Data");};

    //Temp
    int8u ObjectTypeId;
};

} //NameSpace

#endif
