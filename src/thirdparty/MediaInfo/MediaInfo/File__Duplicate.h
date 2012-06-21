// File__Extract - Extract of some formats
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
#ifndef File__DuplicateH
#define File__DuplicateH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/Duplicate/File__Duplicate__Writer.h"
#include "ZenLib/ZtringListList.h"
#include <map>
using namespace ZenLib;
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
