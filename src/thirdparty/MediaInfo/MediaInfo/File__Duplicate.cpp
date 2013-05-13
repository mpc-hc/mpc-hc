/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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
#if MEDIAINFO_DUPLICATE
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Duplicate.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "ZenLib/ZtringList.h"
#include "ZenLib/File.h"
using namespace ZenLib;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

File__Duplicate::File__Duplicate ()
:File__Analyze()
{
    //Temp
    File__Duplicate_HasChanged_=false;
    File__Duplicate_Needed=false;
    Config_File_Duplicate_Get_AlwaysNeeded_Count=0;
}

File__Duplicate::~File__Duplicate ()
{
}

//***************************************************************************
// Get
//***************************************************************************

bool File__Duplicate::File__Duplicate_Get ()
{
    return File__Duplicate_Needed;
}

bool File__Duplicate::File__Duplicate_HasChanged ()
{
    //Retrieving general configuration
    while (Config->File_Duplicate_Get_AlwaysNeeded(Config_File_Duplicate_Get_AlwaysNeeded_Count))
    {
        if (File__Duplicate_Set(Config->File_Duplicate_Get(Config_File_Duplicate_Get_AlwaysNeeded_Count)))
            File__Duplicate_HasChanged_=true;
        Config_File_Duplicate_Get_AlwaysNeeded_Count++;
    }

    bool File__Duplicate_HasChanged_Temp=File__Duplicate_HasChanged_;
    File__Duplicate_HasChanged_=false;
    return File__Duplicate_HasChanged_Temp;
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_DUPLICATE
