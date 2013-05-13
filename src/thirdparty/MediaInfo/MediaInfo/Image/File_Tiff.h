/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about TIFF files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_TiffH
#define MediaInfo_File_TiffH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Tiff
//***************************************************************************

class File_Tiff : public File__Analyze
{
public:
    //Constructor/Destructor
    File_Tiff();

private :
    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();
    void Data_Parse_Fill();

    //Elements
    void Read_Directories();
    void Read_Directory();

    //Temp
    struct ifditem
    {
        int16u Tag;
        int16u Type;
        int32u Count;
    };
    typedef std::map<int32u, ifditem> ifditems; //Key is byte offset
    ifditems IfdItems;
    typedef std::map<int16u, ZtringList> infos; //Key is Tag value
    infos Infos;
    bool LittleEndian;

    //Helpers
    void Get_X2 (int16u &Info, const char* Name);
    void Get_X4 (int32u &Info, const char* Name);
    void GetValueOffsetu(ifditem &IfdItem);
};

} //NameSpace

#endif
