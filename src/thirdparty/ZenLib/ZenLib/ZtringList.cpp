// ZenLib::ZtringList - More methods for vector<std::(w)string>
// Copyright (C) 2002-2011 MediaArea.net SARL, Info@MediaArea.net
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
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
#include <algorithm>
#include <functional>
#include "ZenLib/ZtringList.h"
using namespace std;
#if defined(_MSC_VER) && _MSC_VER <= 1200
    using std::vector; //Visual C++ 6 patch
#endif
//---------------------------------------------------------------------------

namespace ZenLib
{

//---------------------------------------------------------------------------
extern Ztring EmptyZtring;
//---------------------------------------------------------------------------


//***************************************************************************
// Constructors/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
// Constructors
ZtringList::ZtringList ()
: std::vector<ZenLib::Ztring, std::allocator<ZenLib::Ztring> > ()
{
    Separator[0]=_T(";");
    Quote=_T("\"");
    Max[0]=Error;
}

ZtringList::ZtringList(const ZtringList &Source)
: std::vector<ZenLib::Ztring, std::allocator<ZenLib::Ztring> > ()
{
    Separator[0]=Source.Separator[0];
    Quote=Source.Quote;

    reserve(Source.size());
    for (intu Pos=0; Pos<Source.size(); Pos++)
        push_back(Source[Pos]);
}

ZtringList::ZtringList (const Ztring &Source)
{
    Separator[0]=_T(";");
    Quote=_T("\"");
    Max[0]=Error;
    Write(Source.c_str());
}

ZtringList::ZtringList (const Char *Source)
{
    Separator[0]=_T(";");
    Quote=_T("\"");
    Max[0]=Error;
    Write(Source);
}

#ifdef _UNICODE
ZtringList::ZtringList (const char* S)
{
    Write(Ztring(S));
}
#endif

//***************************************************************************
// Operator
//***************************************************************************

//---------------------------------------------------------------------------
// Operator ==
bool ZtringList::operator== (const ZtringList &Source) const
{
    return (Read()==Source.Read());
}

//---------------------------------------------------------------------------
// Operator !=
bool ZtringList::operator!= (const ZtringList &Source) const
{
    return (!(Read()==Source.Read()));
}

//---------------------------------------------------------------------------
// Operator +=
ZtringList &ZtringList::operator+= (const ZtringList &Source)
{
    reserve(size()+Source.size());
    for (size_type Pos=0; Pos<Source.size(); Pos++)
        push_back(Source[Pos]);

    return *this;
}

//---------------------------------------------------------------------------
// Operator =
ZtringList &ZtringList::operator= (const ZtringList &Source)
{
    clear();
    Ztring C=Separator[0];
    Ztring Q=Quote;

    Separator[0]=Source.Separator[0];
    Quote=Source.Quote;
    reserve(Source.size());
    for (size_type Pos=0; Pos<Source.size(); Pos++)
        push_back(Source[Pos]);

    Separator[0]=C;
    Quote=Q;

    return *this;
}

//---------------------------------------------------------------------------
// Operator ()
Ztring &ZtringList::operator() (size_type Pos)
{
    if (Pos>=size())
        Write(Ztring(), Pos);

    return operator[](Pos);
}

//***************************************************************************
// In/Out
//***************************************************************************

//---------------------------------------------------------------------------
// Read
Ztring ZtringList::Read () const
{
    //Integrity
    if (size()==0)
        return Ztring();

    Ztring Retour;
    Ztring ToFind=Separator[0]+Quote[0]+_T("\r\n");
    for (size_type Pos=0; Pos<size(); Pos++)
    {
        if (operator[](Pos).find_first_of(ToFind)==std::string::npos)
            Retour+=operator[](Pos)+Separator[0];
        else if (operator[](Pos).find(Separator[0])==std::string::npos
              && operator[](Pos).find(Quote)==std::string::npos
              && operator[](Pos).find('\r')==std::string::npos
              && operator[](Pos).find('\n')==std::string::npos)
            Retour+=operator[](Pos)+Separator[0];
        else
        {
            if (operator[](Pos).find(Quote)==std::string::npos)
                Retour+=Quote+operator[](Pos)+Quote+Separator[0];
            else
            {
                Ztring Value=operator[](Pos);
                Value.FindAndReplace(Quote, Quote+Quote, 0, Ztring_Recursive);
                Retour+=Quote+Value+Quote+Separator[0];
            }
        }
    }

    //delete all useless separators at the end
    //while (Retour.find(Separator[0].c_str(), Retour.size()-Separator[0].size())!=std::string::npos)
    if (Retour.find(Separator[0].c_str(), Retour.size()-Separator[0].size())!=std::string::npos)
        Retour.resize(Retour.size()-Separator[0].size());

    return Retour;
}

const Ztring &ZtringList::Read (size_type Pos) const
{
    //Integrity
    if (Pos>=size())
        return EmptyZtring;

    return operator[](Pos);
}

//---------------------------------------------------------------------------
// Write
void ZtringList::Write(const Ztring &ToWrite)
{
    clear();

    if (!&ToWrite || !ToWrite.size())
        return;

    size_type PosC=0;
    bool Fini=false;
    Ztring C1;

    Ztring DelimiterL;
    Ztring DelimiterR;
    do
    {
        //Searching quotes
        if (ToWrite[PosC]==Quote[0])
        {
            size_t Pos_End=PosC+1;
            while (Pos_End<ToWrite.size())
            {
                if (ToWrite[Pos_End]==Quote[0] && Pos_End+1<ToWrite.size() && ToWrite[Pos_End+1]==Quote[0])
                    Pos_End+=2; //Double quote, skipping
                else
                {
                    if (ToWrite[Pos_End]==Quote[0])
                        break;
                    Pos_End++;
                }
            }
            C1=ToWrite.substr(PosC+Quote.size(), Pos_End-PosC);
            PosC+=C1.size()+Quote.size();
            if (C1.size()>0 && C1[C1.size()-1]==Quote[0])
            {
                C1.resize(C1.size()-1);
                PosC+=Quote.size();
            }
        }
        else //Normal
        {
            C1=ToWrite.SubString(tstring(), Separator[0], PosC, Ztring_AddLastItem);
            PosC+=C1.size()+Separator[0].size();
        }
        C1.FindAndReplace(Quote+Quote, Quote, 0, Ztring_Recursive);
        if (size()<Max[0])
            push_back(C1);
        if (PosC>=ToWrite.size())
            Fini=true;
    }
    while (!Fini);

    return;
}

void ZtringList::Write(const Ztring &ToWrite, size_type Pos)
{
    if (Pos==Error)
        return;
    if (Pos>=size())
    {
        //Resource reservation
        size_t ToReserve=1;
        while (ToReserve<Pos)
            ToReserve*=2;
        reserve(ToReserve);

        while (Pos>size())
            push_back(Ztring());
        push_back(ToWrite);
    }
    else
        operator[](Pos)=ToWrite;

    return;
}

//***************************************************************************
// Edition
//***************************************************************************

//---------------------------------------------------------------------------
// Swap
void ZtringList::Swap (size_type Pos0_A, size_type Pos0_B)
{
    //Integrity
    size_type Pos_Max;
    if (Pos0_A<Pos0_B)
        Pos_Max=Pos0_B;
    else
        Pos_Max=Pos0_A;
    if (Pos_Max>=size())
        Write(Ztring(), Pos_Max);

    operator [] (Pos0_A).swap(operator [] (Pos0_B));
}

//---------------------------------------------------------------------------
// Sort
void ZtringList::Sort(ztring_t)
{
    std::stable_sort(begin(), end());
    return;
}

//***************************************************************************
// Information
//***************************************************************************

//---------------------------------------------------------------------------
// Find
Ztring::size_type ZtringList::Find (const Ztring &ToFind, size_type Pos, const Ztring &Comparator, ztring_t Options) const
{
    while (Pos<size() && !(operator[](Pos).Compare(ToFind, Comparator, Options)))
        Pos++;
    if (Pos>=size())
        return Error;
    return Pos;
}

//---------------------------------------------------------------------------
// Return the length of the longest string in the list.
Ztring::size_type ZtringList::MaxStringLength_Get ()
{
   size_type Max = 0;
   for (ZtringList::const_iterator it=begin(); it!=end(); ++it)
      if (it->size()>Max)
         Max=it->size();
   return Max;
}

//***************************************************************************
// Configuration
//***************************************************************************

//---------------------------------------------------------------------------
// Separator
void ZtringList::Separator_Set (size_type Level, const Ztring &NewSeparator)
{
    if (Level>0)
        return;
    Separator[Level]=NewSeparator;
}

//---------------------------------------------------------------------------
// Quote
void ZtringList::Quote_Set (const Ztring &NewQuote)
{
    Quote=NewQuote;
}

//---------------------------------------------------------------------------
// Separator
void ZtringList::Max_Set (size_type Level, size_type Max_New)
{
    if (Level>0 || Max_New==0)
        return;
    Max[Level]=Max_New;
}

} //namespace






