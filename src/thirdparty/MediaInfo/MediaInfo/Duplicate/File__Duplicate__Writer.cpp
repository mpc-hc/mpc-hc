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
//
// Duplication helper for some specific formats
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include "MediaInfo/Duplicate/File__Duplicate__Writer.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "ZenLib/ZtringList.h"
#include "ZenLib/File.h"
#include <cstring>
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File__Duplicate__Writer::File__Duplicate__Writer ()
{
    //Out
    Output_Buffer_Configured=false;

    //Buffer
    Method=method_none;
    Buffer=NULL;
    Buffer_Size=0;
    Buffer_Size_Max=0;
    File_Pointer=NULL;
    Registered_Count=0; //Count of registerd streams
}

//---------------------------------------------------------------------------
File__Duplicate__Writer::~File__Duplicate__Writer ()
{
    //Buffer
    delete (File*)File_Pointer; //File_Pointer=NULL
}

//***************************************************************************
// Configure
//***************************************************************************

//---------------------------------------------------------------------------
void File__Duplicate__Writer::Configure (const Ztring &Target)
{
    //Form: "memory://pointer:size"  <--Memory block is specified by user
    //WARNING: pointer must be in ***DECIMAL*** format.
    //Example: "memory://123456789:1316"
    if (Target.find(_T("memory://"))==0 && Target.find(_T(":"), 9)!=std::string::npos)
    {
        size_t SemiColumn_Pos=Target.find(_T(":"), 9);
        Ztring Address=Target.substr(9, SemiColumn_Pos-9);
        Ztring Size=Target.substr(SemiColumn_Pos+1);
        Method=method_buffer;
        Buffer=(int8u*)Address.To_int64u();
        Buffer_Size_Max=(size_t)Size.To_int64u();
    }

    //Form: "file://filename" or     <--the exported filename is specified by user
    else if (Target.find(_T("file://"))==0)
    {
        Method=method_filename;
        File_Name=Target.substr(7, std::string::npos);
    }
}

//***************************************************************************
// Write
//***************************************************************************

void File__Duplicate__Writer::Write (const int8u* ToAdd, size_t ToAdd_Size)
{
    //Integrity
    if (ToAdd==NULL || ToAdd_Size==0)
        return;

    //Writing
    switch (Method)
    {
        //File based
        case method_filename :
            //Open the file if not yet done
            if (File_Pointer==NULL)
            {
                File_Pointer=new File;
                ((File*)File_Pointer)->Open(File_Name, File::Access_Write_Append);
            }
            //Write to the file
            ((File*)File_Pointer)->Write(ToAdd, ToAdd_Size);
            break;

        //Memory based
        case method_buffer :
            //Testing if enough place
            if (Buffer_Size+ToAdd_Size>Buffer_Size_Max)
            {
                Buffer_Size=0; //Buffer underrun, cleaning the buffer
                if (ToAdd_Size>Buffer_Size_Max)
                    ToAdd_Size=0; //Buffer is too small, writing nothing
            }

            //Copying buffer
            std::memcpy(Buffer+Buffer_Size, ToAdd, ToAdd_Size);
            Buffer_Size+=ToAdd_Size;
            break;
        default : ;
    }
}

//***************************************************************************
// Output_Buffer
//***************************************************************************

//---------------------------------------------------------------------------
size_t File__Duplicate__Writer::Output_Buffer_Get ()
{
    size_t Output_Buffer_Size=Buffer_Size;
    Buffer_Size=0; //Can be erased later...
    return Output_Buffer_Size;
}

//---------------------------------------------------------------------------
size_t File__Duplicate__Writer::Output_Buffer_Get (unsigned char** Output_Buffer)
{
    if (Output_Buffer)
        *Output_Buffer=Buffer;
    size_t Output_Buffer_Size=Buffer_Size;
    Buffer_Size=0; //Can be erased later...
    return Output_Buffer_Size;
}

} //NameSpace

