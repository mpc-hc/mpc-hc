// File_Latm - Info for LATM files
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_LATM_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Latm.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor - Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Latm::File_Latm()
:File__Analyze()
{
    //In
    audioMuxVersionA=false;
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Latm::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+2<=Buffer_Size
        && (CC2(Buffer+Buffer_Offset)&0xFFE0)!=0x56E0)
        Buffer_Offset++;
    if (Buffer_Offset+2>=Buffer_Size)
        return false;

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Latm::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+2>Buffer_Size)
        return false;

    //Quick test of synchro
    if ((CC2(Buffer+Buffer_Offset)&0xFFE0)!=0x56E0)
        Synched=false;

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Latm::Header_Parse()
{
    int16u audioMuxLengthBytes;
    BS_Begin();
    Skip_S2(11,                                                 "syncword");
    Get_S2 (13, audioMuxLengthBytes,                            "audioMuxLengthBytes");
    BS_End();

    //Filling
    Header_Fill_Size(3+audioMuxLengthBytes);
    Header_Fill_Code(0, "LATM");
}

//---------------------------------------------------------------------------
void File_Latm::Data_Parse()
{
    AudioMuxElement(true);
    Accept("LATM");
    Finish("LATM");
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Latm::AudioMuxElement(bool muxConfigPresent)
{
    BS_Begin();
    if (muxConfigPresent)
    {
        bool useSameStreamMux;
        Get_SB (useSameStreamMux,                       "useSameStreamMux");
        if (!useSameStreamMux)
            StreamMuxConfig();
        if (audioMuxVersionA==0)
        {
        }
    }
            /*if (!useSameStreamMux)
            ;
            }
            if (audioMuxVersionA == 0) {
            for (i = 0; i <= numSubFrames; i++) {
            PayloadLengthInfo();
            PayloadMux();
            }
            if (otherDataPresent) {
            for(i = 0; i < otherDataLenBits; I++) {
            otherDataBit; 1 bslbf
            }
            }
            }
            else {
            }
            ByteAlign();
            BS_End();
            Skip_XX(audioMuxLengthBytes-1,                             "Data");

        }
    }
    */
}

//---------------------------------------------------------------------------
void File_Latm::StreamMuxConfig()
{
    Element_Begin("StreamMuxConfig");

    bool audioMuxVersion;
    Get_SB (audioMuxVersion,                                    "audioMuxVersion");
    if (audioMuxVersion)
        Get_SB (audioMuxVersionA,                               "audioMuxVersionA");
    else
        audioMuxVersionA=false;

    if (!audioMuxVersionA)
    {
        if (audioMuxVersion==1)
        {
            //taraBufferFullness=LatmGetValue();
        }
        Skip_SB(                                                "allStreamsSameTimeFraming");
        Skip_S1(6,                                              "numSubFrames");
        Skip_S1(4,                                              "numProgram");
/*        for (int8u prog=0; prog<=numProgram; prog++)
        {
            int8u numLayer;
            Get_S1(3,                                           "numLayer");
            for (lay = 0; lay <= numLayer; lay++) {
progSIndx[streamCnt] = prog; laySIndx[streamCnt] = lay;
streamID [ prog][ lay] = streamCnt++;
if (prog == 0 && lay == 0) {
useSameConfig = 0;
} else {
useSameConfig; 1 uimsbf
}
if (! useSameConfig) {
if ( audioMuxVersion == 0 ) {
AudioSpecificConfig();
}
else {
ascLen = LatmGetValue();
ascLen -= AudioSpecificConfig(); Note 1
fillBits; ascLen bslbf
}
}
*/
    }

    Element_End();
}

} //NameSpace

#endif //MEDIAINFO_LATM_YES

