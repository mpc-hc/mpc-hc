// File_Dvdv - Info for DVD objects (IFO) files
// Copyright (C) 2002-2011 MediaArea.net SARL, Info@MediaArea.net
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
//
// Information about DVD objects
// (.ifo files on DVD-Video)
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DvdvH
#define MediaInfo_File_DvdvH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <map>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Dvdv
//***************************************************************************

class File_Dvdv : public File__Analyze
{
protected :
    //Streams management
    void Streams_Finish();

public :
    File_Dvdv();

private :
    //Buffer
    void FileHeader_Parse ();
    void Header_Parse();
    void Data_Parse();

    //Elements
    void VMG();
    void VTS();
    void VTS_PTT_SRPT();
    void VTS_PGCI();
    void VTSM_PGCI_UT();
    void VTS_TMAPTI();
    void VTSM_C_ADT();
    void VTSM_VOBU_ADMAP();
    void VTS_C_ADT();
    void VTS_VOBU_ADMAP();
    void TT_SRPT();
    void VMGM_PGCI_UT();
    void VMG_PTL_MAIT();
    void VMG_VTS_ATRT();
    void VMG_TXTDT_MG();
    void VMGM_C_ADT();
    void VMGM_VOBU_ADMAP();

    void Video();
    void Audio();
    void Text();
    void MultiChannel();

    //Temp
    bool                                VTS_Attributes_AreHere;
    //std::vector<size_t>                 Sectors;
    //std::vector<std::vector<size_t> >   Sectors_Times;
    //std::vector<int8u>                  Sectors_Times_SecondsPerTime;

    enum sector
    {
        Sector_Nothing,
        Sector_VTS_PTT_SRPT,
        Sector_VTS_PGCI,
        Sector_VTSM_PGCI_UT,
        Sector_VTS_TMAPTI,
        Sector_VTSM_C_ADT,
        Sector_VTSM_VOBU_ADMAP,
        Sector_VTS_C_ADT,
        Sector_VTS_VOBU_ADMAP,
        Sector_TT_SRPT,
        Sector_VMGM_PGCI_UT,
        Sector_VMG_PTL_MAIT,
        Sector_VMG_VTS_ATRT,
        Sector_VMG_TXTDT_MG,
        Sector_VMGM_C_ADT,
        Sector_VMGM_VOBU_ADMAP,
    };
    std::vector<sector>                 Sectors;

    //Helpers
    Ztring Time_ADT(int32u Value);
    size_t Program_Pos;
    size_t Time_Pos;
    void Get_Duration(int64u  &Duration, const Ztring &Name);
    Ztring Time_String; //Value from Time_*()
    void PGC(int64u Offset, bool Title=false);
};

//***************************************************************************
// Const
//***************************************************************************

namespace Dvdv
{
    const int32u VMG=0x2D564D47;
    const int32u VTS=0x2D565453;
}

} //NameSpace

#endif
