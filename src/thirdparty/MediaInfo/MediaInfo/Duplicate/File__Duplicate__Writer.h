// File__Duplicate__Writer - Duplication of some formats
// Copyright (C) 2007-2011 MediaArea.net SARL, Info@MediaArea.net
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
#ifndef File__Duplicate__WriterH
#define File__Duplicate__WriterH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "ZenLib/ZtringListList.h"
#include <map>
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File__Duplicate__Writer
//***************************************************************************

class File__Duplicate__Writer
{
public :
    //Constructor/Destructor
    File__Duplicate__Writer();
    ~File__Duplicate__Writer();

    //Out
    bool Output_Buffer_Configured;

    //Configure
    void   Configure (const Ztring &Target);
    void   UnRegister(); //

    //Write
    void   Write (const int8u* ToAdd, size_t ToAdd_Size);

    //Output buffer
    size_t Output_Buffer_Get ();
    size_t Output_Buffer_Get (unsigned char** Output_Buffer);

private :
    //Buffer
    enum method
    {
        method_none,
        method_buffer,
        method_filename,
    };
    method  Method;
    int8u*  Buffer;
    size_t  Buffer_Size;
    size_t  Buffer_Size_Max;
    Ztring  File_Name;
    void*   File_Pointer; //ZenLib::File*
public :
    size_t  Registered_Count;
};


} //NameSpace

#endif
