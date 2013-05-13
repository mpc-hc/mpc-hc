/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef File__Duplicate__WriterH
#define File__Duplicate__WriterH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
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
