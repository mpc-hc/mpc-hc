/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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
    //Buffer - File header
    bool FileHeader_Begin();

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
