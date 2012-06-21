// File_Tiff - Info for TIFF files
// Copyright (C) 2005-2011 MediaArea.net SARL, Info@MediaArea.net
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
