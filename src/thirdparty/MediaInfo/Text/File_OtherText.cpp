// File_OtherText - Use magic number to detect only the format (Text)
// Copyright (C) 2006-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_OTHERTEXT_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_OtherText.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Functions
//***************************************************************************

void File_OtherText::Read_Buffer_Continue()
{
    if (Buffer_Size<0x200)
        return;

    Ztring Format, FormatMore, Codec;
    Ztring File;
    ZtringList Lines;

    //Feed File and Lines
    File.From_Local((char*)Buffer, Buffer_Size); //Ansi
    if (File.size()<0x100)
    {
        File.From_Unicode((wchar_t*)Buffer, 0, Buffer_Size/sizeof(wchar_t)); //Unicode with BOM
        //TODO: Order of bytes (big or Little endian)
        if (File.size()<0x100)
        {
            Reject("Other text");
            return;
        }
    }
    if (File.size()>0x1000)
        File.resize(0x1000); //Do not work on too big
    if (File.find(_T("\x0D\x0A"))!=Error)
    {
        Lines.Separator_Set(0, _T("\x0D\x0A"));
        Lines.Write(File);
    }
    else if (File.find(_T("\x0D"))!=Error)
    {
        Lines.Separator_Set(0, _T("\x0D"));
        Lines.Write(File);
    }
    else if (File.find(_T("\x0A"))!=Error)
    {
        Lines.Separator_Set(0, _T("\x0A"));
        Lines.Write(File);
    }
    else
    {
        Reject("Other text");
        return;
    }
    if (Lines.size()<0x10)
    {
        Reject("Other text");
        return;
    }
    Lines.resize(0x20);

         if (Lines[0].size()==1
          && Lines[0][0]==_T('1')
          && Lines[1].size()==29
          && Lines[1][ 0]==_T('0') && Lines[1][ 1]==_T('0')
          && Lines[1][ 2]==_T(':') && Lines[1][ 5]==_T(':') && Lines[1][ 8]==_T(',')
          && Lines[1][12]==_T(' ') && Lines[1][13]==_T('-') && Lines[1][14]==_T('-') && Lines[1][15]==_T('>') && Lines[1][16]==_T(' ')
          && Lines[1][17]==_T('0') && Lines[1][18]==_T('0')
          && Lines[1][19]==_T(':') && Lines[1][22]==_T(':') && Lines[1][25]==_T(',')
          && Lines.Find(_T("2"))!=Error
          )
    {
        Format=_T("SubRip");
        Codec=_T("SubRip");
    }
    else if (Lines[0]==_T("[Script Info]")
          && Lines.Find(_T("ScriptType: v4.00"))!=Error
          && Lines.Find(_T("[V4 Styles]"))!=Error
          )
    {
       Format=_T("SSA");
       FormatMore=_T("SubStation Alpha");
       Codec=_T("SSA");
    }
    else if (Lines[0]==_T("[Script Info]")
          && Lines.Find(_T("ScriptType: v4.00+"))!=Error
          && Lines.Find(_T("[V4+ Styles]"))!=Error
          )
    {
       Format=_T("ASS");
       FormatMore=_T("Advanced SubStation Alpha");
       Codec=_T("ASS");
    }
    else if (Lines[0].size()>24
          && Lines[0][ 0]==_T('0') && Lines[0][ 1]==_T('0')
          && Lines[0][ 2]==_T(':') && Lines[0][ 5]==_T(':') && Lines[0][ 8]==_T(':')
          && Lines[0][11]==_T(' ')
          && Lines[0][12]==_T('0') && Lines[0][13]==_T('0')
          && Lines[0][14]==_T(':') && Lines[0][17]==_T(':') && Lines[0][20]==_T(':')
          && Lines[0][23]==_T(' ')
          )
    {
       Format=_T("Adobe encore DVD");
       Codec=_T("Adobe");
    }
    else if (Lines[0].size()==11
          && Lines[0][0]==_T('-') && Lines[0][1]==_T('-') && Lines[0][2]==_T('>') && Lines[0][3]==_T('>') && Lines[0][4]==_T(' ')
          && Lines[0][5]==_T('0')
          && Lines[1].empty()!=true
          )
    {
       Format=_T("AQTitle");
       Codec=_T("AQTitle");
    }
    else if (Lines[0].size()>28
          && Lines[0][ 0]==_T('0') && Lines[0][ 1]==_T('0')
          && Lines[0][ 2]==_T(':') && Lines[0][ 5]==_T(':') && Lines[0][ 8]==_T(':')
          && Lines[0][11]==_T(' ') && Lines[0][12]==_T(',') && Lines[0][13]==_T(' ')
          && Lines[0][14]==_T('0') && Lines[0][15]==_T('0')
          && Lines[0][16]==_T(':') && Lines[0][19]==_T(':') && Lines[0][22]==_T(':')
          && Lines[0][25]==_T(' ') && Lines[0][16]==_T(',') && Lines[0][27]==_T(' ')
          )
    {
       Format=_T("Captions 32");
       Codec=_T("Caption 32");
    }
    else if (Lines[0].size()==23
          && Lines[0]==_T("*Timecode type: PAL/EBU")
          && Lines[1].empty()
          && Lines[2].size()==23
          && Lines[2][ 0]==_T('0') && Lines[2][ 1]==_T('0')
          && Lines[2][ 2]==_T(':') && Lines[2][ 5]==_T(':') && Lines[2][ 8]==_T(':')
          && Lines[2][11]==_T(' ')
          && Lines[2][12]==_T('0') && Lines[2][13]==_T('0')
          && Lines[2][14]==_T(':') && Lines[2][17]==_T(':') && Lines[2][20]==_T(':')
          && Lines[2].size()>0
          )
    {
       Format=_T("Captions Inc");
       Codec=_T("Captions inc");
    }
    else if (Lines[0].size()>1
          && Lines[0][0]==_T('*')
          && Lines.Find(_T("** Caption Number 1"))!=Error
    )
    {
       Format=_T("Cheeta");
    }
    else if (Lines[0].size()>10
          && Lines[0][0]==_T('~') && Lines[0][1]==_T('C') && Lines[0][2]==_T('P') && Lines[0][3]==_T('C') && Lines[0][9]==_T('~')
          && Lines[1][ 0]==_T('0') && Lines[1][ 1]==_T('0')
          && Lines[1][ 2]==_T(':') && Lines[1][ 5]==_T(':') && Lines[1][ 8]==_T(':')
    )
    {
       Format=_T("CPC Captioning");
       Codec=_T("CPC Captioning");
    }
    else if (false
    )
    {
    }
    else
        return;

    if (Format.empty())
        return;

    Accept("Other text");

    if (!IsSub)
    {
        Fill(Stream_General, 0, General_Format, Format);
        Fill(Stream_General, 0, General_Format_Info, FormatMore, true);
    }

    Stream_Prepare(Stream_Text);
    Fill(Stream_Text, 0, Text_Format, Format);
    Fill(Stream_Text, 0, Text_Codec, Codec);

    //No more need data
    Element_Begin(Format);
    Element_End();
    Finish("Other text");
}

} //NameSpace

#endif
