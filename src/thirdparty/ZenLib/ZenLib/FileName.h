/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// File name related functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_FileNameH
#define ZenLib_FileNameH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Ztring.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
/// @brief File name manipulation
//***************************************************************************

class FileName : public ZenLib::Ztring
{
public :
    //Constructor/Destructor
    FileName ()                                                                 : Ztring(){};
    FileName (const tstring& str)                                               : Ztring(str){};
    FileName (const tstring& str, size_type pos, size_type n=npos)              : Ztring(str, pos, n){};
    FileName (const Char* s, size_type n)                                       : Ztring(s, n){};
    FileName (const Char* s)                                                    : Ztring(s){};
    FileName (size_type n, char c)                                              : Ztring(n, c){};

    //Read/Write
    ZenLib::Ztring  Path_Get             () const;
    ZenLib::Ztring  Name_Get             () const;
    ZenLib::Ztring  Extension_Get        () const;
    ZenLib::Ztring& Path_Set             (const Ztring &Path);
    ZenLib::Ztring& Name_Set             (const Ztring &Name);
    ZenLib::Ztring& Extension_Set        (const Ztring &Extension);

    //Helpers
    static ZenLib::Ztring Path_Get              (const Ztring &File_Name)       {return ((FileName&)File_Name).Path_Get();};
    static ZenLib::Ztring Name_Get              (const Ztring &File_Name)       {return ((FileName&)File_Name).Name_Get();};
    static ZenLib::Ztring Extension_Get         (const Ztring &File_Name)       {return ((FileName&)File_Name).Extension_Get();};
    static ZenLib::Ztring TempFileName_Create   (const Ztring &Prefix);
};

//Platform differences
extern const Char* FileName_PathSeparator;

} //NameSpace

//---------------------------------------------------------------------------
#ifdef __BORLANDC__
    #pragma warn .8027
#endif
//---------------------------------------------------------------------------

#endif
