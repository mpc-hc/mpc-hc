/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef File__DuplicateH
#define File__DuplicateH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Duplicate/File__Duplicate__Writer.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File__Duplicate
//***************************************************************************

class File__Duplicate : public File__Analyze
{
public :
    //Constructor/Destructor
    File__Duplicate();
    virtual ~File__Duplicate();

protected :
    virtual bool File__Duplicate_Set  (const Ztring &Value)=0; //Fill a new File__Duplicate value

    //Get
    bool   File__Duplicate_Get  ();

    //Modifications
    bool   File__Duplicate_HasChanged();

private :
    bool   File__Duplicate_HasChanged_;
    bool   File__Duplicate_Needed;
    size_t Config_File_Duplicate_Get_AlwaysNeeded_Count;
};


} //NameSpace

#endif
