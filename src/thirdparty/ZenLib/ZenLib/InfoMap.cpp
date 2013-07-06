/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "ZenLib/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf_Internal.h"
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
    Separator[1]=__T(";");
}

InfoMap::InfoMap(const Ztring &Source)
: std::multimap<ZenLib::Ztring, ZenLib::ZtringList> ()
{
    Separator[0]=EOL;
    Separator[1]=__T(";");
    Write(Source);
}

InfoMap::InfoMap(const Char *Source)
: std::multimap<ZenLib::Ztring, ZenLib::ZtringList> ()
{
    Separator[0]=EOL;
    Separator[1]=__T(";");
    Write(Source);
}

#ifdef _UNICODE
InfoMap::InfoMap (const char* S)
: std::multimap<ZenLib::Ztring, ZenLib::ZtringList> ()
{
    Separator[0]=EOL;
    Separator[1]=__T(";");
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
            ++List; //The second one, this is a stupid hack for a 2 value, should be changed later...
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

    if (NewInfoMap.empty())
        return;

    size_t Pos1=0, Pos2_EOL=0, Pos2_Separator=0;

    while (Pos2_EOL!=(size_t)-1)
    {
        Pos2_EOL=NewInfoMap.find(__T('\n'), Pos1);
        Pos2_Separator=NewInfoMap.find(__T(';'), Pos1);
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
