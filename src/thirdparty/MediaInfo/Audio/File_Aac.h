// File_Aac - Info for AAC (Raw) files
// Copyright (C) 2008-2010 MediaArea.net SARL, Info@MediaArea.net
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
// Information about AAC (Raw) files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_AacH
#define MediaInfo_File_AacH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#ifdef MEDIAINFO_MPEG4_YES
    #include "MediaInfo/Multiple/File_Mpeg4_Descriptors.h"
#endif
#ifdef MEDIAINFO_FAAD_YES
    #include "neaacdec.h";
#endif
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Aac
//***************************************************************************

class File_Aac : public File__Analyze
{
public :
    //In
    ZenLib::Ztring Codec;
    #ifdef MEDIAINFO_MPEG4_YES
        File_Mpeg4_Descriptors::decspecificinfotag* DecSpecificInfoTag;
        File_Mpeg4_Descriptors::slconfig* SLConfig;
    #endif

    //Constructor/Destructor
    File_Aac();
    ~File_Aac();

protected :
    //Buffer - Global
    void Read_Buffer_Continue ();

    //Helpers
    void From_Codec();

    //libfaad specific
    void libfaad();
    #if defined(MEDIAINFO_FAAD_YES) && defined(MEDIAINFO_MPEG4_YES)
        NeAACDecHandle hAac;
    #endif
};

} //NameSpace

#endif
