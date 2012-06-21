// ZenLib::FileName - FileName functions
// Copyright (C) 2007-2011 MediaArea.net SARL, Info@MediaArea.net
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
// File name related functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_FileNameH
#define ZenLib_FileNameH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf.h"
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
