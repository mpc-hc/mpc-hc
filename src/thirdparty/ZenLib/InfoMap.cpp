// ZenLib::InfoMap - Helper for InfoMap
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
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#include "ZenLib/Conf_Internal.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef ZENLIB_USEWX
    #include <wx/strconv.h>
#endif //ZENLIB_USEWX
#include <algorithm>
#include "ZenLib/InfoMap.h"
using namespace std;
//---------------------------------------------------------------------------

namespace ZenLib
{

//---------------------------------------------------------------------------
const Ztring InfoMap_EmptyZtring_Const; //Use it when we can't return a reference to a true Ztring, const version
//---------------------------------------------------------------------------

//***************************************************************************
// Constructors/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
// Constructors
InfoMap::InfoMap()
: std::multimap<ZenLib::Ztring, ZenLib::ZtringList> ()
{
    Separator[0]=EOL;
    Separator[1]=_T(";");
}

InfoMap::InfoMap(const Ztring &Source)
: std::multimap<ZenLib::Ztring, ZenLib::ZtringList> ()
{
    Separator[0]=EOL;
    Separator[1]=_T(";");
    Write(Source);
}

InfoMap::InfoMap(const Char *Source)
: std::multimap<ZenLib::Ztring, ZenLib::ZtringList> ()
{
    Separator[0]=EOL;
    Separator[1]=_T(";");
    Write(Source);
}

#ifdef _UNICODE
InfoMap::InfoMap (const char* S)
: std::multimap<ZenLib::Ztring, ZenLib::ZtringList> ()
{
    Separator[0]=EOL;
    Separator[1]=_T(";");
    Write(Ztring(S));
}
#endif

//***************************************************************************
// In/Out
//***************************************************************************

//---------------------------------------------------------------------------
const Ztring &InfoMap::Get (const Ztring &Value, size_t Pos) const
{
    InfoMap::const_iterator List=find(Value);
    if (List==end())
        return InfoMap_EmptyZtring_Const; //Not found
    if (Pos<List->second.size())
        return List->second[Pos];
    else
        return InfoMap_EmptyZtring_Const; //Not found
}

//---------------------------------------------------------------------------
const Ztring &InfoMap::Get (const Ztring &Value, size_t Pos, const Ztring &WithValue, size_t WithValue_Pos) const
{
    InfoMap::const_iterator List=find(Value);
    if (List==end())
        return InfoMap_EmptyZtring_Const; //Not found
    if (Pos<List->second.size())
    {
        if (List->second[WithValue_Pos]==WithValue)
            return List->second[Pos];
        else
        {
            List++; //The second one, this is a stupid hack for a 2 value, should be changed later...
            if (Pos<List->second.size())
            {
                if (List->second[WithValue_Pos]==WithValue)
                    return List->second[Pos];
                else
                    return InfoMap_EmptyZtring_Const; //Not found
            }
            else
                return InfoMap_EmptyZtring_Const; //Not found
        }
    }
    else
        return InfoMap_EmptyZtring_Const; //Not found
}

//---------------------------------------------------------------------------
// Set
void InfoMap::Write(const Ztring &NewInfoMap)
{
    clear();

    if (!&NewInfoMap || !NewInfoMap.size())
        return;

    size_t Pos1=0, Pos2_EOL=0, Pos2_Separator=0;

    while (Pos2_EOL!=(size_t)-1)
    {
        Pos2_EOL=NewInfoMap.find(_T('\n'), Pos1);
        Pos2_Separator=NewInfoMap.find(_T(';'), Pos1);
        if (Pos2_Separator<Pos2_EOL)
        {
            ZtringList List; List.Write(NewInfoMap.substr(Pos1, Pos2_EOL-Pos1));
            insert (pair<Ztring, ZtringList>(NewInfoMap.substr(Pos1, Pos2_Separator-Pos1), List));
        }
        Pos1=Pos2_EOL+1;
    }
}

//***************************************************************************
// Configuration
//***************************************************************************

//---------------------------------------------------------------------------
// Separator
void InfoMap::Separator_Set (size_type Level, const Ztring &NewSeparator)
{
    if (Level>1)
        return;

    Separator[Level]=NewSeparator;
}

//***************************************************************************
// C++
//***************************************************************************

} //namespace








