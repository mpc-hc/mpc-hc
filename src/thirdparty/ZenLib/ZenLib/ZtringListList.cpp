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
#include <algorithm>
#include "ZenLib/ZtringListList.h"
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
ZtringListList::ZtringListList()
: std::vector<ZenLib::ZtringList, std::allocator<ZenLib::ZtringList> > ()
{
    Separator[0]=EOL;
    Separator[1]=__T(";");
    Quote=__T("\"");
    Max[0]=Error;
    Max[1]=Error;
}

ZtringListList::ZtringListList(const ZtringListList &Source)
: std::vector<ZenLib::ZtringList, std::allocator<ZenLib::ZtringList> > ()
{
    Separator[0]=Source.Separator[0];
    Separator[1]=Source.Separator[1];
    Quote=Source.Quote;
    Max[0]=Source.Max[0];
    Max[1]=Source.Max[1];
    reserve(Source.size());
    for (intu Pos=0; Pos<Source.size(); Pos++)
        push_back(Source[Pos]);
}

ZtringListList::ZtringListList(const Ztring &Source)
{
    Separator[0]=EOL;
    Separator[1]=__T(";");
    Quote=__T("\"");
    Max[0]=Error;
    Max[1]=Error;
    Write(Source.c_str());
}

ZtringListList::ZtringListList(const Char *Source)
{
    Separator[0]=EOL;
    Separator[1]=__T(";");
    Quote=__T("\"");
    Max[0]=Error;
    Max[1]=Error;
    Write(Source);
}

#ifdef _UNICODE
ZtringListList::ZtringListList (const char* S)
{
    Separator[0]=EOL;
    Separator[1]=__T(";");
    Quote=__T("\"");
    Max[0]=Error;
    Max[1]=Error;
    Write(Ztring(S));
}
#endif

//***************************************************************************
// Operators
//***************************************************************************

//---------------------------------------------------------------------------
//Operator ==
bool ZtringListList::operator== (const ZtringListList &Source) const
{
    return (Read()==Source.Read());
}

//---------------------------------------------------------------------------
//Operator !=
bool ZtringListList::operator!= (const ZtringListList &Source) const
{
    return (!(Read()==Source.Read()));
}

//---------------------------------------------------------------------------
// Operator +=
ZtringListList &ZtringListList::operator+= (const ZtringListList &Source)
{
    reserve(size()+Source.size());
    for (size_type Pos=0; Pos<Source.size(); Pos++)
    {
        push_back(Source[Pos]);
        operator[](size()-1).Separator_Set(0, Separator[1]);
        operator[](size()-1).Quote_Set(Quote);
        operator[](size()-1).Max_Set(0, Max[1]);
    }

    return *this;
}

//---------------------------------------------------------------------------
// Operator =
ZtringListList &ZtringListList::operator= (const ZtringListList &Source)
{
    clear();

    reserve(Source.size());
    for (size_type Pos=0; Pos<Source.size(); Pos++)
    {
        push_back(Source[Pos]);
        operator[](size()-1).Separator_Set(0, Separator[1]);
        operator[](size()-1).Quote_Set(Quote);
        operator[](size()-1).Max_Set(0, Max[1]);
    }

    return *this;
}

//---------------------------------------------------------------------------
// Operatorr ()
ZtringList &ZtringListList::operator() (size_type Pos0)
{
    //Integrity
    if (Pos0>=size())
        Write(Ztring(), Pos0);

    return operator[](Pos0);
}

Ztring &ZtringListList::operator() (size_type Pos0, size_type Pos1)
{
    //Integrity
    if (Pos0>=size())
        Write(Ztring(), Pos0);

    return operator[](Pos0).operator()(Pos1);
}

Ztring &ZtringListList::operator() (const Ztring &Pos0, size_type Pos0_1, size_type Pos1)
{
    size_type Pos=0;
    size_t Size=size();
    for (; Pos<Size; Pos++)
        if (operator[](Pos).size()>Pos0_1)
            if (operator[](Pos)[Pos0_1]==Pos0)
                break;

    if (Pos>=Size)
    {
        Write(Pos0, Size, Pos0_1);
        Pos=size()-1;
    }

    return operator[](Pos).operator()(Pos1);
}

//***************************************************************************
// In/Out
//***************************************************************************

//---------------------------------------------------------------------------
// Read
Ztring ZtringListList::Read () const
{
    //Integrity
    if (size()==0)
        return Ztring();

    Ztring ToReturn;
    size_type Size=size()-1;
    for (size_type Pos0=0; Pos0<Size; Pos0++)
        ToReturn+=Read(Pos0)+Separator[0];
    ToReturn+=Read(Size);

    //Delete all useless separators at the end
    //if(ToReturn.size()>0 && Separator[0].size() && ToReturn(ToReturn.size()-1)==Separator[0][Separator[0].size()-1]) //Optimize speed
    //    while (ToReturn.find(Separator[0].c_str(), ToReturn.size()-Separator[0].size())!=std::string::npos)
    //        ToReturn.resize(ToReturn.size()-Separator[0].size());

    return ToReturn;
}

Ztring ZtringListList::Read (size_type Pos0) const
{
    //Integrity
    if (Pos0>=size())
        return Ztring();

    return operator[](Pos0).Read();
}

const Ztring &ZtringListList::Read (size_type Pos0, size_type Pos1) const
{
    //Integrity
    if (Pos0>=size())
        return EmptyZtring;

    return operator[](Pos0).Read(Pos1);
}

const Ztring &ZtringListList::Read (const Ztring &Pos0, size_type Pos1) const
{
    size_type Pos=Find(Pos0);
    if (Pos==Error)
        return EmptyZtring;

    return operator[](Pos).Read(Pos1);
}

const Ztring &ZtringListList::Read (const Ztring &Pos0, size_type Pos0_1, size_type Pos1) const
{
    size_type Pos=Find(Pos0, Pos0_1);
    if (Pos==Error)
        return EmptyZtring;

    return operator[](Pos).Read(Pos1);
}

const Ztring &ZtringListList::Read (const Ztring &Pos0, const Ztring &Default, size_type Pos1) const
{
    size_type Pos=Find(Pos0);
    if (Pos==Error)
        return Default;

    return operator[](Pos).Read(Pos1);
}

const Ztring &ZtringListList::Read (const Ztring &Pos0, const Ztring &Default, size_type Pos0_1, size_type Pos1) const //Lecture d'un champs en position 0 avec une option par defaut
{
    size_type Pos=Find(Pos0, Pos0_1);
    if (Pos==Error)
        return Default;

    return operator[](Pos).Read(Pos1);
}

Ztring ZtringListList::Read1 (size_type Pos1) const
{
    Ztring ToReturn;
    size_type Size=size()-1;
    for (size_type Pos=0; Pos<Size; Pos++)
        ToReturn+=operator[](Pos).Read(Pos1)+Separator[0];
    ToReturn+=operator[](Size).Read(Pos1);

    //Delete all useless separators at the end
    if(ToReturn(ToReturn.size()-1)==Separator[0][Separator[0].size()-1]) //Optimize speed
        while (ToReturn.find(Separator[0].c_str(), ToReturn.size()-Separator[0].size())!=std::string::npos)
            ToReturn.resize(ToReturn.size()-Separator[0].size());

    return ToReturn;
}

//---------------------------------------------------------------------------
// Write
void ZtringListList::Write(const Ztring &ToWrite)
{
    clear();

    if (ToWrite.empty())
        return;

    size_type PosC=0;
    bool Fini=false;
    Ztring C1;
    ZtringList ZL1;
    ZL1.Separator_Set(0, Separator[1]);
    ZL1.Quote_Set(Quote);
    ZL1.Max_Set(0, Max[1]);

    //Detecting carriage return format
    Ztring WriteSeparator;
    if (Separator[0]==EOL)
    {
        size_t CarriageReturn_Pos=ToWrite.find_first_of(__T("\r\n"));
        if (CarriageReturn_Pos!=string::npos)
        {
            if (ToWrite[CarriageReturn_Pos]==__T('\r'))
            {
                if (CarriageReturn_Pos+1<ToWrite.size() && ToWrite[CarriageReturn_Pos+1]==__T('\n'))
                    WriteSeparator=__T("\r\n");
                else
                    WriteSeparator=__T("\r");
            }
            else
                WriteSeparator=__T("\n");
        }
        else
            WriteSeparator=Separator[0];
    }
    else
        WriteSeparator=Separator[0];

    do
    {
        //Searching end of line, but it must not be in quotes
        bool InQuotes=false;
        Ztring CharsToFind=WriteSeparator+Quote;
        size_t Pos_End=PosC;
        while (Pos_End<ToWrite.size())
        {
            Pos_End=ToWrite.find(WriteSeparator, Pos_End);
            if (Pos_End!=string::npos)
            {
                if (Pos_End+Quote.size()<ToWrite.size() && ToWrite[Pos_End]==Quote[0] && ToWrite[Pos_End+1]!=Quote[0])
                {
                    InQuotes=!InQuotes; //This is not double quotes, so this is a normal quote
                    /*if (!InQuotes)
                    {
                        C1=ToWrite.substr(PosC, Pos_End-PosC);
                        break;
                    }*/
                }

                if (!InQuotes && Pos_End+WriteSeparator.size()<=ToWrite.size() && ToWrite[Pos_End]==WriteSeparator[0])
                {
                    C1=ToWrite.substr(PosC, Pos_End-PosC);
                    break;
                }

                if (InQuotes && Pos_End+Quote.size()*2<ToWrite.size() && ToWrite[Pos_End]==Quote[0] && ToWrite[Pos_End+1]==Quote[0])
                    Pos_End+=2;
                else
                    Pos_End++;
            }
        }
        if (Pos_End>=ToWrite.size())
            C1=ToWrite.substr(PosC, string::npos);

        ZL1.Write(C1);
        push_back(ZL1);
        PosC+=C1.size()+WriteSeparator.size();
        if (PosC>=ToWrite.size())
            Fini=true;
    }
    while (!Fini);
}

void ZtringListList::Write(const ZtringList &ToWrite, size_type Pos)
{
    //Integrity
    if (Pos==Error)
        return;

    //Writing
    if (Pos>=size())
    {
        //Reservation de ressources
        if (!capacity())
            reserve(1);
        while (Pos>=capacity())
            reserve(capacity()*2);

        while (Pos>size())
            push_back (Ztring());
        push_back(ToWrite);
    }
    else
        operator[](Pos)=ToWrite;
}

void ZtringListList::Write(const Ztring &ToWrite, size_type Pos0, size_type Pos1)
{
    if (Pos0>=size())
        Write(Ztring(), Pos0);

    operator[](Pos0).Write(ToWrite, Pos1);
}

void ZtringListList::push_back (const ZtringList &ToAdd)
{
    vector<ZtringList>::push_back(ToAdd); //Visual C++ 6 patch, should be std::vector
    operator[](size()-1).Separator_Set(0, Separator[1]);
    operator[](size()-1).Quote_Set(Quote);
    operator[](size()-1).Max_Set(0, Max[1]);
}

void ZtringListList::push_back (const Ztring &ToAdd)
{
    ZtringList ZL1;
    ZL1.Separator_Set(0, Separator[1]);
    ZL1.Quote_Set(Quote);
    ZL1.Max_Set(0, Max[1]);
    ZL1.Write(ToAdd);
    push_back(ZL1);
}

//---------------------------------------------------------------------------
// Insert
void ZtringListList::Insert1 (const Ztring &ToInsert, size_type Pos1)
{
    for (size_type Pos0=0; Pos0<size(); Pos0++)
        operator[](Pos0).Insert(ToInsert, Pos1);
}

//---------------------------------------------------------------------------
// Delete
void ZtringListList::Delete (const Ztring &ToFind, size_type Pos1, const Ztring &Comparator, ztring_t Options)
{
    size_type Pos0=0;
    while ((Pos0=Find(ToFind, Pos1, Pos0, Comparator, Options))!=Error)
        operator [] (Pos0).Delete(Pos1);
}

void ZtringListList::Delete1 (size_type Pos1)
{
    for (size_type Pos0=0; Pos0<size(); Pos0++)
        operator [] (Pos0).Delete(Pos1);
}

//***************************************************************************
// Edition
//***************************************************************************

//---------------------------------------------------------------------------
// Swap
void ZtringListList::Swap (size_type Pos0_A, size_type Pos0_B)
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

void ZtringListList::Swap1 (size_type Pos1_A, size_type Pos1_B)
{
    for (size_type Pos0=0; Pos0<size(); Pos0++)
        operator () (Pos0, Pos1_A).swap(operator () (Pos0, Pos1_B));
}

//---------------------------------------------------------------------------
// Sort
void ZtringListList::Sort(size_type, ztring_t)
{
    std::stable_sort(begin(), end());
    return;
}

//---------------------------------------------------------------------------
// Find
Ztring::size_type ZtringListList::Find (const Ztring &ToFind, size_type Pos1, size_type Pos0) const
{
    size_t Size=size();
    for (; Pos0<Size; Pos0++)
        if (operator[](Pos0).size()>Pos1)
            if (operator[](Pos0)[Pos1]==ToFind)
                break;

    if (Pos0>=Size)
        return Error;
    return Pos0;
}

Ztring::size_type ZtringListList::Find_Filled (size_type Pos1, size_type Pos0) const
{
    size_t Size=size();
    for (; Pos0<Size; Pos0++)
        if (operator[](Pos0).size()>Pos1)
            if (!operator[](Pos0)[Pos1].empty())
                break;

    if (Pos0>=Size)
        return Error;
    return Pos0;
}

Ztring::size_type ZtringListList::Find (const Ztring &ToFind, size_type Pos1, size_type Pos0, const Ztring &Comparator, ztring_t Options) const
{
    while (    Pos0<size()
       && (    Pos1>=at(Pos0).size()
            || !at(Pos0).at(Pos1).Compare(ToFind, Comparator, Options)))
        Pos0++;
    if (Pos0>=size())
        return Error;
    return Pos0;
}

Ztring ZtringListList::FindValue (const Ztring &ToFind, size_type Pos1Value, size_type Pos1, size_type Pos0Begin, const Ztring &Comparator, ztring_t) const
{
    size_type Pos0=Find(ToFind, Pos1, Pos0Begin, Comparator);
    if (Pos0==Error)
        return Ztring();

    return Read(Pos0, Pos1Value);
}

ZtringListList ZtringListList::SubSheet (const Ztring &ToFind, size_type Pos1, size_type Pos0, const Ztring &Comparator, ztring_t) const
{
    ZtringListList ToReturn;
    ToReturn.Separator[0]=Separator[0];
    ToReturn.Separator[1]=Separator[1];
    ToReturn.Quote=Quote;

    Pos0--;
    do
    {
        Pos0=Find(ToFind, Pos1, Pos0+1, Comparator);
        ToReturn.push_back(Read(Pos0));
    }
    while (Pos0!=Error);

    return ToReturn;
}

//***************************************************************************
// Configuration
//***************************************************************************

//---------------------------------------------------------------------------
// Separator
void ZtringListList::Separator_Set (size_type Level, const Ztring &NewSeparator)
{
    if (Level>1)
        return;

    Separator[Level]=NewSeparator;
    if (Level==1)
        for (size_type Pos0=0; Pos0<size(); Pos0++)
            operator () (Pos0).Separator_Set(0, Separator[1]);
}

//---------------------------------------------------------------------------
// Quote
void ZtringListList::Quote_Set (const Ztring &NewQuote)
{
    Quote=NewQuote;
    for (size_type Pos0=0; Pos0<size(); Pos0++)
        operator () (Pos0).Quote_Set(Quote);
}

//---------------------------------------------------------------------------
// Max
void ZtringListList::Max_Set (size_type Level, size_type NewMax)
{
    if (Level>1 || NewMax==0)
        return;

    Max[Level]=NewMax;
    if (Level==1)
        for (size_type Pos0=0; Pos0<size(); Pos0++)
            operator () (Pos0).Max_Set(0, Max[1]);
}

//***************************************************************************
//
//***************************************************************************

} //namespace
