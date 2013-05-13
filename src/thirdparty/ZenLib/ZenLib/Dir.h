/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Directories functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_DirH
#define ZenLib_DirH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/ZtringList.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
/// @brief Directory manipulation
//***************************************************************************

class Dir
{
public :
    //---------------------------------------------------------------------------
    /// @brief Options for Open method
    enum dirlist_t
    {
        Nothing         = 0x00,         ///<
        Include_Files   = 0x01,         ///< Include files
        Include_Dirs    = 0x02,         ///< Include directories
        Include_Hidden  = 0x04,         ///< Include hidden files
        Parse_SubDirs   = 0x10          ///< Parse subdirectories
    };

    //Constructor/Destructor

    //Open/close
    static ZtringList GetAllFileNames(const Ztring &Dir_Name, dirlist_t Options=(dirlist_t)(Include_Files|Parse_SubDirs));

    //Helpers
    static bool Exists(const Ztring &Dir_Name);
    static bool Create(const Ztring &Dir_Name);
};

} //NameSpace

#endif
