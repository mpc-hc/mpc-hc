// ZenLib::File - File functions
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// File functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_FileH
#define ZenLib_FileH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf.h"
#include "ZenLib/Ztring.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
/// @brief File manipulation
//***************************************************************************

class File
{
public :
    //---------------------------------------------------------------------------
    /// @brief Options for Open method
    enum access_t
    {
        Access_Read                 = 0,            ///< Read permission
        Access_Write                = 1,            ///< Write permission
        Access_Read_Write           = 2,            ///< Read and Write permissions
        Access_Write_Append         = 3,            ///< Write permission without deleting old file
        Access_Write_Excluding      = 4             ///< Write permission preventing reading
    };

    //---------------------------------------------------------------------------
    /// @brief Options for Move method
    enum move_t
    {
        FromBegin                   = 0,            ///< Begin of file
        FromCurrent                 = 1,            ///< Current position
        FromEnd                     = 2             ///< End of file
    };

    //Constructor/Destructor
    File  ();
    File  (ZenLib::Ztring File_Name, access_t Access=Access_Read);
    ~File ();

    //Open/close
    bool Open  (const tstring &File_Name, access_t Access=Access_Read);
    bool Create(const ZenLib::Ztring &File_Name, bool OverWrite=true);
    void Close ();

    //Read/Write
    size_t Read  (int8u* Buffer, size_t Buffer_Size);
    size_t Write (const int8u* Buffer, size_t Buffer_Size);
    size_t Write (const Ztring &ToWrite);
    bool   Truncate (int64u Offset=(int64u)-1);

    //Moving
    bool GoTo (int64s Position, move_t MoveMethod=FromBegin);
    int64u Position_Get ();

    //Attributes
    int64u Size_Get();
    Ztring Created_Get();
    Ztring Created_Local_Get();
    Ztring Modified_Get();
    Ztring Modified_Local_Get();
    bool   Opened_Get();

    //Helpers
    static int64u           Size_Get(const Ztring &File_Name);
    static Ztring           Created_Get(const Ztring &File_Name);
    static Ztring           Modified_Get(const Ztring &File_Name);
    static bool             Exists(const Ztring &File_Name);
    static bool             Copy(const Ztring &Source, const Ztring &Destination, bool OverWrite=false);
    static bool             Move(const Ztring &Source, const Ztring &Destination, bool OverWrite=false);
    static bool             Delete(const Ztring &File_Name);

    //Temp
    Ztring File_Name;
    int64u Position; //Position is saved, may be not good because position may change
    int64u Size; //Size is saved, may be not good because size may change
    void*  File_Handle;
};

} //NameSpace

#endif
