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
#if defined(MEDIAINFO_OTHER_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File_Other.h"
#include "ZenLib/Utils.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Format
//***************************************************************************

//---------------------------------------------------------------------------
void File_Other::Read_Buffer_Continue()
{
    //Integrity
    if (Buffer_Size<16)
    {
        Element_WaitForMoreData();
        return;
    }

    Ztring Format;
    if (Buffer[0]==0xEA
     && Buffer[1]==0x22
     && Buffer[2]<=0x03)
    {
        Accept();

        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, 0, Text_Format, "Cheetah");

        Finish();
        return;
    }
    if (Buffer[0]==0x4C
     && Buffer[1]==0x61
     && Buffer[2]==0x6D
     && Buffer[3]==0x62
     && Buffer[4]==0x64
     && Buffer[5]==0x61)
    {
        Accept();

        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, 0, Text_Format, "Lambda");

        Finish();
        return;
    }
         if (CC4(Buffer)==0xC5C6CBC3) {Format=__T("RISC OS Chunk data");}
    else if (CC4(Buffer)==0x110000EF) {Format=__T("RISC OS AIF executable");}
    else if (CC4(Buffer)==CC4("Draw")) {Format=__T("RISC OS Draw");}
    else if (CC4(Buffer)==CC4("FONT")) {Format=__T("RISC OS Font");}
    else if (CC8(Buffer)==CC8("Maestro\r")) {Format=__T("RISC OS music file");}
    else if (CC4(Buffer)==CC4("FC14")) {Format=__T("Amiga Future Composer");}
    else if (CC4(Buffer)==CC4("SMOD")) {Format=__T("Amiga Future Composer");}
    else if (CC4(Buffer)==CC4("AON4")) {Format=__T("Amiga Art Of Noise");}
    else if (CC8(Buffer+1)==CC8("MUGICIAN")) {Format=__T("Amiga Mugician");}
    else if (Buffer_Size>=66 && CC8(Buffer+58)==CC8("SIDMON I")) {Format=__T("Amiga Sidmon");}
    else if (CC8(Buffer)==CC8("Synth4.0")) {Format=__T("Amiga Synthesis");}
    else if (CC4(Buffer)==CC4("ARP.")) {Format=__T("Amiga Holy Noise");}
    else if (CC4(Buffer)==CC4("BeEp")) {Format=__T("Amiga JamCracker");}
    else if (CC4(Buffer)==CC4("COSO")) {Format=__T("Amiga Hippel-COSO");}
    else if (CC3(Buffer)==CC3("LSX")) {Format=__T("Amiga LZX");}
    else if (CC4(Buffer)==CC4("MOVI")) {Format=__T("Silicon Graphics movie");}
    else if (CC4(Buffer+10)==CC4("Vivo")) {Format=__T("Vivo");}
    else if (CC4(Buffer+1)==CC4("VRML")) {Format=__T("VRML");}
    else if (CC5(Buffer)==CC5("HVQM4")) {Format=__T("GameCube Movie");}
    else if (CC8(Buffer)==CC8("KW-DIRAC"))
    {
        Accept("Dirac");

        Stream_Prepare(Stream_Video);
        Fill(Stream_Video, 0, Video_Format, "Dirac");

        Finish("Dirac");
        return;
    }
    else if (CC5(Buffer)==CC5("ustar")) {Format=__T("Tar archive");}
    //TODO: all archive magic numbers
    else if (CC4(Buffer+1)==CC4("MSCB")) {Format=__T("MS Cabinet");}
    else if (CC4(Buffer)==CC4(".snd")) {Format=__T("SUN Audio");}
    else if (CC4(Buffer)==0x2E736400) {Format=__T("DEC Audio");}
    else if (CC4(Buffer)==CC4("MThd")) {Format=__T("MIDI");}
    else if (CC4(Buffer)==CC4("CTMF")) {Format=__T("CMF");}
    else if (CC3(Buffer)==CC3("SBI")) {Format=__T("SoundBlaster");}
    else if (CC4(Buffer)==CC4("EMOD")) {Format=__T("Ext. MOD");}
    //TODO: Other Sound magic numbers
    else if (CC7(Buffer)==CC7("BLENDER")) {Format=__T("Blender");}
    else if (CC4(Buffer)==CC4("AC10")) {Format=__T("AutoCAD"); ;}
    else if (CC2(Buffer)==0x1F9D) {Format=__T("Compress");}
    else if (CC2(Buffer)==0x1F8B) {Format=__T("GZip");}
    else if (CC2(Buffer)==0x1F1E) {Format=__T("Huffman");}
    else if (CC3(Buffer)==CC3("BZh")) {Format=__T("BZip2");}
    else if (CC2(Buffer)==CC2("BZ")) {Format=__T("BZip1");}
    else if (CC3(Buffer)==CC3("NES")) {Format=__T("NES ROM");}
    else if (Buffer_Size>=0x108 && CC4(Buffer+0x104)==0xCEED6666) {Format=__T("GameBoy");}
    else if (Buffer_Size>=0x104 && CC4(Buffer+0x100)==CC4("SEGA")) {Format=__T("MegaDrive");}
    else if (Buffer_Size>=0x284 && CC4(Buffer+0x280)==CC4("EAGN")) {Format=__T("SupeMegaDrive");}
    else if (Buffer_Size>=0x284 && CC4(Buffer+0x280)==CC4("EAMG")) {Format=__T("SupeMegaDrive");}
    else if (CC4(Buffer)==0x21068028) {Format=__T("Dreamcast");}
    else if (CC4(Buffer)==CC4("LCDi")) {Format=__T("Dreamcast");}
    else if (CC4(Buffer)==0x37804012) {Format=__T("Nintendo64");}
    else if (CC8(Buffer)==CC8("PS-X EXE")) {Format=__T("Playstation");}
    else if (CC4(Buffer)==CC4("LCDi")) {Format=__T("Dreamcast");}
    else if (CC4(Buffer)==CC4("XBEH")) {Format=__T("X-Box");}
    else if (CC4(Buffer)==CC4("XIP0")) {Format=__T("X-Box");}
    else if (CC4(Buffer)==CC4("XTF0")) {Format=__T("X-Box");}
    else if (CC2(Buffer)==0x8008) {Format=__T("Lynx");}
    else if (CC7(Buffer)==CC7("\x01ZZZZZ\x01")) {Format=__T("");}
    else if (CC6(Buffer)==CC6("1\x0A\x0D""00:"))
    {
        Accept("SRT");

        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, 0, Text_Format, "SRT");

        Finish("SRT");
        return;
    }
    else if ((CC1(Buffer+0)==CC1("[") && CC1(Buffer+2)==CC1("S") && CC1(Buffer+22)==CC1("o") && CC1(Buffer+24)==CC1("]")) //Unicode Text is : "[Script Info]
          || (CC1(Buffer+2)==CC1("[") && CC1(Buffer+4)==CC1("S") && CC1(Buffer+24)==CC1("o") && CC1(Buffer+26)==CC1("]")))
    {
        Accept("SSA");

        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, 0, Text_Format, "SSA");

        Finish("SSA");
        return;
    }
    else if (CC4(Buffer)==CC4("RIFF") && CC4(Buffer+8)==CC4("AMV ")) {Format=__T("AMV");}
    else if (CC4(Buffer)==CC4("RIFF") && CC4(Buffer+8)==CC4("WEBP"))
    {
        Accept("WEBP");

        Stream_Prepare(Stream_Image);
        Fill(Stream_Image, 0, Image_Format, "WebP");

        Finish("WEBP");
        return;
    }
    else if (CC4(Buffer)==0x414D5697) {Format=__T("MTV");}
    else if (CC6(Buffer)==CC6("Z\0W\0F\0"))
    {
        Accept("ZWF");

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "ZWF");

        Finish("ZWF");
        return;
    }
    else if (CC4(Buffer)==0x616A6B67) //"ajkg"
    {
        Accept("Shorten");

        Fill(Stream_General, 0, General_Format_Version, CC1(Buffer+4));

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "Shorten");

        Finish("Shorten");
        return;
    }
    else if (CC4(Buffer)==0x504C5646) {Format=__T("PlayLater Video");}
    else if (CC4(Buffer)==CC4("")) {Format=__T("");}

    if (Format.empty())
    {
        Reject();
        return;
    }

    Accept();

    Element_Offset=File_Size-(File_Offset+Buffer_Offset);
    Fill(Stream_General, 0, General_Format, Format);

    Finish();
}

} //NameSpace

#endif //MEDIAINFO_OTHER_YES
