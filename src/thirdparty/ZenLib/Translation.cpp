// ZenLib::Translation - Helper for translation
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
    Separator[1]=_T(";");
}

Translation::Translation(const Ztring &Source)
{
    Separator[0]=EOL;
    Separator[1]=_T(";");
    Write(Source);
}

Translation::Translation(const Char *Source)
{
    Separator[0]=EOL;
    Separator[1]=_T(";");
    Write(Source);
}

#ifdef _UNICODE
Translation::Translation (const char* S)
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
        Temp++;
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

    if (!&NewLanguage || !NewLanguage.size())
        return;

    size_t Pos1=0, Pos2_EOL=0, Pos2_Separator=0;

    while (Pos2_EOL!=(size_t)-1)
    {
        Pos2_EOL=NewLanguage.find(_T('\n'), Pos1);
        Pos2_Separator=NewLanguage.find(_T(';'), Pos1);
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








