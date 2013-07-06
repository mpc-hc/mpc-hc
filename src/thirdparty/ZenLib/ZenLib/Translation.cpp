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
#include "ZenLib/Translation.h"
using namespace std;
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
// Constructors/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
// Constructors
Translation::Translation()
: std::map<ZenLib::Ztring, ZenLib::Ztring> ()
{
    Separator[0]=EOL;
    Separator[1]=__T(";");
}

Translation::Translation(const Ztring &Source)
{
    Separator[0]=EOL;
    Separator[1]=__T(";");
    Write(Source);
}

Translation::Translation(const Char *Source)
{
    Separator[0]=EOL;
    Separator[1]=__T(";");
    Write(Source);
}

#ifdef _UNICODE
Translation::Translation (const char* S)
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
// Get
Ztring Translation::Get () const
{
    Ztring ToReturn;
    const_iterator Temp=begin();
    while (Temp!=end())
    {
        ToReturn+=Temp->first;
        ToReturn+=Separator[1];
        ToReturn+=Temp->second;
        ToReturn+=Separator[0];
        ++Temp;
    }
    return ToReturn;
}

const Ztring &Translation::Get (const Ztring &Value)
{
    iterator Pos=find(Value);
    if (Pos==end())
        operator[](Value)=Value;
    return operator[](Value);
}

Ztring Translation::Get (const Ztring &Value, const Ztring &Default)
{
    iterator Pos=find(Value);
    if (Pos==end())
        operator[](Value)=Default;
    return operator[](Value);
}

//---------------------------------------------------------------------------
// Set
void Translation::Write(const Ztring &NewLanguage)
{
    clear();

    if (NewLanguage.empty())
        return;

    size_t Pos1=0, Pos2_EOL=0, Pos2_Separator=0;

    while (Pos2_EOL!=(size_t)-1)
    {
        Pos2_EOL=NewLanguage.find(__T('\n'), Pos1);
        Pos2_Separator=NewLanguage.find(__T(';'), Pos1);
        if (Pos2_Separator<Pos2_EOL)
        {
            operator[](NewLanguage.substr(Pos1, Pos2_Separator-Pos1))=NewLanguage.substr(Pos2_Separator+1, Pos2_EOL-Pos2_Separator-1);
        }
        Pos1=Pos2_EOL+1;
    }
}

//---------------------------------------------------------------------------
// Set
void Translation::Write(const Ztring &Value, const Ztring &NewLanguage)
{
    operator[](Value)=NewLanguage;
}

//***************************************************************************
// Configuration
//***************************************************************************

//---------------------------------------------------------------------------
// Separator
void Translation::Separator_Set (size_type Level, const Ztring &NewSeparator)
{
    if (Level>1)
        return;

    Separator[Level]=NewSeparator;
}

//***************************************************************************
// C++
//***************************************************************************

} //namespace
