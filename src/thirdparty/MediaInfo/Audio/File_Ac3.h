// File_Ac3 - Info for AC3 files
// Copyright (C) 2004-2010 MediaArea.net SARL, Info@MediaArea.net
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
#ifndef MediaInfo_Ac3H
#define MediaInfo_Ac3H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Ac3
//***************************************************************************

class File_Ac3 : public File__Analyze
{
public :
    //In
    size_t Frame_Count_Valid;
    bool   MustParse_dac3;
    bool   MustParse_dec3;

    //Constructor/Destructor
    File_Ac3();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Global
    void Read_Buffer_Continue ();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Core();
    void HD();
    void dac3();
    void dec3();

    //Temp
    struct dolby
    {
        int8u  dialnorm;
        int8u  compr;
        int8u  dynrng;  //This is only the first occurence of aufblk
        bool   compre;
        bool   dynrnge;  //This is only the first occurence of aufblk

        dolby()
        {
            dialnorm=0;
            compre=false;
            dynrnge=false;
        }
    };
    dolby  FirstFrame_Dolby;
    dolby  FirstFrame_Dolby2;
    std::vector<int64u> dialnorms;
    std::vector<int64u> dialnorm2s;
    std::vector<int64u> comprs;
    std::vector<int64u> compr2s;
    std::vector<int64u> dynrngs;
    std::vector<int64u> dynrng2s;
    std::map<int8u, int64u> fscods;
    std::map<int8u, int64u> frmsizecods;
    size_t Frame_Count;
    size_t HD_Count;
    int16u chanmap;
    int16u frmsiz;
    int16u HD_BitRate_Max;
    int16u HD_Channels2;
    int8u  fscod;
    int8u  fscod2;
    int8u  frmsizecod;
    int8u  bsid;
    int8u  bsmod;
    int8u  acmod;
    int8u  dsurmod;
    int8u  numblks;
    int8u  HD_StreamType;
    int8u  HD_SubStreams_Count;
    int8u  HD_SamplingRate1;
    int8u  HD_SamplingRate2;
    int8u  HD_Channels1;
    int8u  HD_Resolution1;
    int8u  HD_Resolution2;
    int8u  dynrng_Old;
    bool   lfeon;
    bool   dxc3_Parsed;
    bool   HD_MajorSync_Parsed;
    bool   HD_NoRestart;
    bool   HD_ExtraParity;
    bool   HD_AlreadyCounted;
    bool   HD_IsVBR;
    bool   Core_IsPresent;
    bool   dynrnge_Exists;
};

} //NameSpace

#endif
