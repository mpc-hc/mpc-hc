/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about MPEG files, Program Map Section
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Mpeg_PsiH
#define MediaInfo_Mpeg_PsiH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mpeg_Descriptors.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Mpeg_Psi
//***************************************************************************

class File_Mpeg_Psi : public File__Analyze
{
public :
    //In
    bool    From_TS;
    complete_stream* Complete_Stream;
    int16u  pid;

public :
    File_Mpeg_Psi();
    ~File_Mpeg_Psi();

private :
    //Buffer - File header
    void FileHeader_Parse();

    //Buffer - Per element
    bool Header_Begin();
    void Header_Parse();
    void Data_Parse();

    //Elements - Base
    void Table_reserved();
    void Table_iso13818_6();
    void Table_user_private();
    void Table_forbidden();

    //Elements - MPEG
    void program_stream_map(); //From PS
    void Table_00();
    void Table_01();
    void Table_02();
    void Table_03();
    void Table_04() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_05() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_06() {Skip_XX(Element_Size-Element_Offset, "Data");};
    //Elements - DVB
    void Table_38() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_39() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_3A() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_3B() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_3C() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_3D() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_3E() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_3F() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_40();
    void Table_41() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_42();
    void Table_46();
    void Table_4A() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_4E();
    void Table_4F();
    void Table_5F(); //50 --> 5F
    void Table_6F(); //60 --> 6F
    void Table_70();
    void Table_71() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_72() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_73();
    void Table_74() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_75() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_76() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_77() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_78() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_79() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_7E() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_7F();
    //Elements - ASTC
    void Table_C0();
    void Table_C1();
    void Table_C2() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_C3() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_C4() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_C5() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_C6() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_C7();
    void Table_C8() {Table_C9();};
    void Table_C9();
    void Table_CA();
    void Table_CB();
    void Table_CC();
    void Table_CD();
    void Table_CE() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_CF() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_D0() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_D1() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_D2() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_D3() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_D4() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_D5() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_D6();
    void Table_D7() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_D8() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_D9() {Skip_XX(Element_Size-Element_Offset, "Data");};
    void Table_DA() {Table_C9();};
    void Table_FC();
    void Table_FC_00();
    void Table_FC_04();
    void Table_FC_05();
    void Table_FC_05_break_duration();
    void Table_FC_05_splice_time();
    void Table_FC_06();
    void Table_FC_07();

    //Helpers
    int16u Descriptors_Size;
    void Descriptors();
    void ATSC_multiple_string_structure(Ztring &Value, const char* Info);
    void SCTE_multilingual_text_string(int8u Size, Ztring &Value, const char* Info);
    Ztring Date_MJD(int16u Date);
    Ztring Time_BCD(int32u Time);

    //Items removal
    void program_number_Update();
    void program_number_Remove();
    void elementary_PID_Update(int16u PCR_PID);
    void elementary_PID_Remove();

    //Data
    int32u CRC_32;
    int16u transport_stream_id;
    int16u table_id_extension;
    int16u elementary_PID;
    int16u program_number;
    int8u  stream_type;
    int16u event_id;
    int8u  pointer_field;
    int8u  table_id;
    int8u  version_number;
    bool   current_next_indicator;
    bool   section_syntax_indicator;
    bool   elementary_PID_IsValid;
    bool   program_number_IsValid;
    bool   stream_type_IsValid;
    bool   event_id_IsValid;
    bool   IsATSC;
    bool   ForceStreamDisplay;
};

} //NameSpace

#endif
