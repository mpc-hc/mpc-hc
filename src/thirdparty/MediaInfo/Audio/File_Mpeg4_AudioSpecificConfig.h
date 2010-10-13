// File_Aac - Info for AAC Audio files
// Copyright (C) 2002-2010 MediaArea.net SARL, Info@MediaArea.net
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
#ifndef MediaInfo_File_Mpeg4_AudioSpecificConfigH
#define MediaInfo_File_Mpeg4_AudioSpecificConfigH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Mpeg4_AudioSpecificConfig
//***************************************************************************

class File_Mpeg4_AudioSpecificConfig : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Mpeg4_AudioSpecificConfig();

private :
    //Buffer - Global
    void Read_Buffer_Continue ();

    //Elements
    void GASpecificConfig();
    void SBR();
    void PS();
    void ALS();

    //Temp
    int32u samplingFrequency;
    int8u  channelConfiguration;
    int8u  audioObjectType;
    int8u  extensionAudioObjectType;
    bool   sbrData;
    bool   sbrPresentFlag;
    bool   psData;
    bool   psPresentFlag;
};

} //NameSpace

#endif
