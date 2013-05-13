/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about ID3v2 tagged files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Id3v2H
#define MediaInfo_File_Id3v2H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

///***************************************************************************
// Class File_Id3v2
//***************************************************************************

class File_Id3v2 : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Id3v2();

    //Static stuff
    static bool Static_Synchronize_Tags (const int8u* Buffer, size_t Buffer_Offset, size_t Buffer_Size, bool &Tag_Found);

private :
    //Streams management
    void Streams_Fill();

    //Buffer - File header
    void FileHeader_Parse ();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    #ifdef TEXT
        #undef TEXT
    #endif
    void T___();
    void T__X();
    void W___();
    void W__X();
    void AENC()   {Skip_XX(Element_Size, "Data");}
    void APIC();
    void ASPI()   {Skip_XX(Element_Size, "Data");}
    void COMM();
    void COMR()   {Skip_XX(Element_Size, "Data");}
    void ENCR()   {Skip_XX(Element_Size, "Data");}
    void EQU2()   {Skip_XX(Element_Size, "Data");}
    void EQUA()   {Skip_XX(Element_Size, "Data");}
    void ETCO()   {Skip_XX(Element_Size, "Data");}
    void GEOB()   {Skip_XX(Element_Size, "Data");}
    void GRID()   {Skip_XX(Element_Size, "Data");}
    void IPLS()   {Skip_XX(Element_Size, "Data");}
    void LINK()   {Skip_XX(Element_Size, "Data");}
    void MCDI()   {T___();}
    void MLLT()   {Skip_XX(Element_Size, "Data");}
    void OWNE()   {Skip_XX(Element_Size, "Data");}
    void PCNT()   {Skip_XX(Element_Size, "Data");}
    void POPM()   {Skip_XX(Element_Size, "Data");}
    void POSS()   {Skip_XX(Element_Size, "Data");}
    void PRIV();
    void RBUF()   {Skip_XX(Element_Size, "Data");}
    void RGAD();
    void RVA2()   {Skip_XX(Element_Size, "Data");}
    void RVRB()   {Skip_XX(Element_Size, "Data");}
    void SEEK()   {Skip_XX(Element_Size, "Data");}
    void SIGN()   {Skip_XX(Element_Size, "Data");}
    void SYLT();
    void SYTC()   {Skip_XX(Element_Size, "Data");}
    void TALB()   {T___();}
    void TBPM()   {T___();}
    void TCMP()   {T___();}
    void TCOM()   {T___();}
    void TCON()   {T___();}
    void TCOP()   {T___();}
    void TDAT()   {T___();}
    void TDEN()   {T___();}
    void TDLY()   {T___();}
    void TDOR()   {T___();}
    void TDRC()   {T___();}
    void TDRL()   {T___();}
    void TDTG()   {T___();}
    void TENC()   {T___();}
    void TEXT()   {T___();}
    void TFLT()   {T___();}
    void TIME()   {T___();}
    void TIPL()   {T___();}
    void TIT1()   {T___();}
    void TIT2()   {T___();}
    void TIT3()   {T___();}
    void TKEY()   {T___();}
    void TLAN()   {T___();}
    void TLEN()   {T___();}
    void TMCL()   {T___();}
    void TMED()   {T___();}
    void TMOO()   {T___();}
    void TOAL()   {T___();}
    void TOFN()   {T___();}
    void TOLY()   {T___();}
    void TOPE()   {T___();}
    void TORY()   {T___();}
    void TOWN()   {T___();}
    void TPE1()   {T___();}
    void TPE2()   {T___();}
    void TPE3()   {T___();}
    void TPE4()   {T___();}
    void TPOS()   {T___();}
    void TPRO()   {T___();}
    void TPUB()   {T___();}
    void TRCK()   {T___();}
    void TRDA()   {T___();}
    void TRSN()   {T___();}
    void TRSO()   {T___();}
    void TSIZ()   {T___();}
    void TSO2()   {T___();}
    void TSOA()   {T___();}
    void TSOC()   {T___();}
    void TSOP()   {T___();}
    void TSOT()   {T___();}
    void TSRC()   {T___();}
    void TSSE()   {T___();}
    void TSST()   {T___();}
    void TXXX();
    void TYER()   {T___();}
    void UFID()   {Skip_XX(Element_Size, "Data");}
    void USER()   {Skip_XX(Element_Size, "Data");}
    void USLT();
    void WCOM()   {W___();}
    void WCOP()   {W___();}
    void WOAF()   {W___();}
    void WOAR()   {W___();}
    void WOAS()   {W___();}
    void WORS()   {W___();}
    void WPAY()   {W___();}
    void WPUB()   {W___();}
    void WXXX();
    void BUF()    {Skip_XX(Element_Size, "Data");}
    void CNT()    {Skip_XX(Element_Size, "Data");}
    void COM()    {COMM();}
    void CRA()    {Skip_XX(Element_Size, "Data");}
    void CRM()    {Skip_XX(Element_Size, "Data");}
    void EQU()    {Skip_XX(Element_Size, "Data");}
    void ETC()    {Skip_XX(Element_Size, "Data");}
    void GEO()    {Skip_XX(Element_Size, "Data");}
    void IPL()    {Skip_XX(Element_Size, "Data");}
    void LNK()    {Skip_XX(Element_Size, "Data");}
    void MCI()    {Skip_XX(Element_Size, "Data");}
    void MLL()    {Skip_XX(Element_Size, "Data");}
    void PIC_()   {APIC();}
    void POP()    {Skip_XX(Element_Size, "Data");}
    void REV()    {Skip_XX(Element_Size, "Data");}
    void RVA()    {Skip_XX(Element_Size, "Data");}
    void SLT()    {Skip_XX(Element_Size, "Data");}
    void STC()    {Skip_XX(Element_Size, "Data");}
    void TAL()    {T___();}
    void TBP()    {T___();}
    void TCM()    {T___();}
    void TCO()    {T___();}
    void TCP()    {Skip_XX(Element_Size, "Data");}
    void TCR()    {T___();}
    void TDA()    {T___();}
    void TDY()    {T___();}
    void TEN()    {T___();}
    void TFT()    {T___();}
    void TIM()    {T___();}
    void TKE()    {T___();}
    void TLA()    {T___();}
    void TLE()    {T___();}
    void TMT()    {T___();}
    void TOA()    {T___();}
    void TOF()    {T___();}
    void TOL()    {T___();}
    void TOR()    {T___();}
    void TOT()    {T___();}
    void TP1()    {T___();}
    void TP2()    {T___();}
    void TP3()    {T___();}
    void TP4()    {T___();}
    void TPA()    {T___();}
    void TPB()    {T___();}
    void TRC()    {T___();}
    void TRD()    {T___();}
    void TRK()    {T___();}
    void TSI()    {T___();}
    void TSS()    {T___();}
    void TT1()    {T___();}
    void TT2()    {T___();}
    void TT3()    {T___();}
    void TXT()    {T___();}
    void TXX()    {TXXX();}
    void TYE()    {T___();}
    void UFI()    {Skip_XX(Element_Size, "Data");}
    void ULT()    {USLT();}
    void WAF()    {W___();}
    void WAR()    {W___();}
    void WAS()    {W___();}
    void WCM()    {W___();}
    void WCP()    {W___();}
    void WPB()    {W___();}
    void WXX()    {WXXX();}
    void XRVA()   {RVA2();}

    //Temp
    ZtringList Element_Values;
    Ztring Element_Value;
    Ztring Year, Month, Day, Hour, Minute;
    stream_t StreamKind;
    int64u Id3v2_Size;
    int8u  Id3v2_Version;
    bool   Unsynchronisation_Global;
    bool   Unsynchronisation_Frame;
    bool   DataLengthIndicator;

    //Helpers
    void Fill_Name();
    void Normalize_Date (Ztring& Date);
};


} //NameSpace

#endif
