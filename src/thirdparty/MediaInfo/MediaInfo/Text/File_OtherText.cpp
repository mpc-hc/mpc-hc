/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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
    {
        Element_WaitForMoreData();
        return;
    }

    Element_Offset=File_Size-(File_Offset+Buffer_Offset);

    Ztring Format, FormatMore, Codec;
    Ztring File;
    ZtringList Lines;

    //Feed File and Lines
    File.From_UTF8((const char*)Buffer, Buffer_Size>65536?65536:Buffer_Size);
    if (File.empty())
        File.From_Local((const char*)Buffer, Buffer_Size>65536?65536:Buffer_Size); // Trying from local code page
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
    File.FindAndReplace(__T("\r\n"), __T("\n"), 0, Ztring_Recursive);
    File.FindAndReplace(__T("\r"), __T("\n"), 0, Ztring_Recursive);
    Lines.Separator_Set(0, __T("\n"));
    Lines.Write(File);
    Lines.resize(0x20);

         if (Lines[0]==__T("[Script Info]")
          && (Lines.Find(__T("ScriptType: v4.00"))!=Error || Lines.Find(__T("Script Type: V4.00"))!=Error)
          && Lines.Find(__T("[V4 Styles]"))!=Error
          )
    {
       Format=__T("SSA");
       FormatMore=__T("SubStation Alpha");
       Codec=__T("SSA");
    }
    else if (Lines[0]==__T("[Script Info]")
          && (Lines.Find(__T("ScriptType: v4.00+"))!=Error || Lines.Find(__T("Script Type: V4.00+"))!=Error)
          && Lines.Find(__T("[V4+ Styles]"))!=Error
          )
    {
       Format=__T("ASS");
       FormatMore=__T("Advanced SubStation Alpha");
       Codec=__T("ASS");
    }
    else if (Lines[0].size()>24
          && Lines[0][ 0]==__T('0') && Lines[0][ 1]==__T('0')
          && Lines[0][ 2]==__T(':') && Lines[0][ 5]==__T(':') && Lines[0][ 8]==__T(':')
          && Lines[0][11]==__T(' ')
          && Lines[0][12]==__T('0') && Lines[0][13]==__T('0')
          && Lines[0][14]==__T(':') && Lines[0][17]==__T(':') && Lines[0][20]==__T(':')
          && Lines[0][23]==__T(' ')
          )
    {
       Format=__T("Adobe encore DVD");
       Codec=__T("Adobe");
    }
    else if (Lines[0].size()==11
          && Lines[0][0]==__T('-') && Lines[0][1]==__T('-') && Lines[0][2]==__T('>') && Lines[0][3]==__T('>') && Lines[0][4]==__T(' ')
          && Lines[0][5]==__T('0')
          && Lines[1].empty()!=true
          )
    {
       Format=__T("AQTitle");
       Codec=__T("AQTitle");
    }
    else if (Lines[0].size()>28
          && Lines[0][ 0]==__T('0') && Lines[0][ 1]==__T('0')
          && Lines[0][ 2]==__T(':') && Lines[0][ 5]==__T(':') && Lines[0][ 8]==__T(':')
          && Lines[0][11]==__T(' ') && Lines[0][12]==__T(',') && Lines[0][13]==__T(' ')
          && Lines[0][14]==__T('0') && Lines[0][15]==__T('0')
          && Lines[0][16]==__T(':') && Lines[0][19]==__T(':') && Lines[0][22]==__T(':')
          && Lines[0][25]==__T(' ') && Lines[0][16]==__T(',') && Lines[0][27]==__T(' ')
          )
    {
       Format=__T("Captions 32");
       Codec=__T("Caption 32");
    }
    else if (Lines[0].size()==23
          && Lines[0]==__T("*Timecode type: PAL/EBU")
          && Lines[1].empty()
          && Lines[2].size()==23
          && Lines[2][ 0]==__T('0') && Lines[2][ 1]==__T('0')
          && Lines[2][ 2]==__T(':') && Lines[2][ 5]==__T(':') && Lines[2][ 8]==__T(':')
          && Lines[2][11]==__T(' ')
          && Lines[2][12]==__T('0') && Lines[2][13]==__T('0')
          && Lines[2][14]==__T(':') && Lines[2][17]==__T(':') && Lines[2][20]==__T(':')
          && Lines[2].size()>0
          )
    {
       Format=__T("Captions Inc");
       Codec=__T("Captions inc");
    }
    else if (Lines[0].size()>1
          && Lines[0][0]==__T('*')
          && Lines.Find(__T("** Caption Number 1"))!=Error
    )
    {
       Format=__T("Cheeta");
    }
    else if (Lines[0].size()>10
          && Lines[0][0]==__T('~') && Lines[0][1]==__T('C') && Lines[0][2]==__T('P') && Lines[0][3]==__T('C') && Lines[0][9]==__T('~')
          && Lines[1][ 0]==__T('0') && Lines[1][ 1]==__T('0')
          && Lines[1][ 2]==__T(':') && Lines[1][ 5]==__T(':') && Lines[1][ 8]==__T(':')
    )
    {
       Format=__T("CPC Captioning");
       Codec=__T("CPC Captioning");
    }
    else if (Lines[0].find(__T("<SAMI>"))==0)
    {
       Format=__T("SAMI");
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
    Element_Begin1(Format);
    Element_End0();
    Finish("Other text");
}

} //NameSpace

#endif
