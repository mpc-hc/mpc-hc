// File_Rm - Info for Real Media files
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
// Information about Real Media files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_RmH
#define MediaInfo_File_RmH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Classe File_Rm
//***************************************************************************

class File_Rm : public File__Analyze
{
public :
    //In
    stream_t FromMKV_StreamType;

public :
    File_Rm();

private :
    //Buffer
    void Header_Parse();
    void Data_Parse();

    //Elements
    void  RMF();
    void CONT();
    void DATA();
    void INDX();
    void MDPR();
    void MDPR_realvideo();
    void MDPR_realaudio();
    void MDPR_fileinfo();
    void PROP();
    void RJMD();
    void RJMD_property(std::string Name);
    void RMJE();
    void RMMD();
    void TAG ();

    //Temp
    bool MDPR_IsStream;
};

} //NameSpace

#endif
