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
#include "ZenLib/Format/Http/Http_Utils.h"
using namespace std;
//---------------------------------------------------------------------------

namespace ZenLib
{

namespace Format
{

namespace Http
{

//***************************************************************************
// Global variables
//***************************************************************************

std::string EmptyString;

//***************************************************************************
// Character manipulation
//***************************************************************************

//---------------------------------------------------------------------------
unsigned char Char2Hex (unsigned char Char)
{
         if (Char<='9' && Char>='0')
        Char-='0';
    else if (Char<='f' && Char>='a')
        Char-='a'-10;
    else if (Char<='F' && Char>='A')
        Char-='A'-10;
    else
        Char =0;
    return Char;
}

//---------------------------------------------------------------------------
wchar_t Char2Hex (wchar_t Char)
{
         if (Char<='9' && Char>='0')
        Char-='0';
    else if (Char<='f' && Char>='a')
        Char-='a'-10;
    else if (Char<='F' && Char>='A')
        Char-='A'-10;
    else
        Char =0;
    return Char;
}

//---------------------------------------------------------------------------
string Hex2Char (unsigned char Char)
{
    string Result;
    Result+=(char)((Char/16>=10?'a'-10:'0')+Char/16);
    Result+=(char)((Char%16>=10?'a'-10:'0')+Char%16);
    return Result;
}

//---------------------------------------------------------------------------
#ifndef WSTRING_MISSING
wstring Hex2Char (wchar_t Char)
{
    wstring Result;
    Result+=(Char/16>=10?'a'-10:'0')+Char/16;
    Result+=(Char%16>=10?'a'-10:'0')+Char%16;
    return Result;
}
#endif //WSTRING_MISSING

//***************************************************************************
// URL manipulation
//***************************************************************************

//---------------------------------------------------------------------------
std::string URL_Encoded_Encode (const std::string& URL)
{
    string Result;
    string::size_type Pos;
    for (Pos=0; Pos<URL.size(); Pos++)
    {
        if ((URL[Pos]>='\x00' && URL[Pos]<='\x20')
         || URL[Pos]=='\x7F'
         || URL[Pos]==' '
         || URL[Pos]=='<'
         || URL[Pos]=='>'
         || URL[Pos]=='#'
         || URL[Pos]=='%'
         || URL[Pos]=='\"'
         || URL[Pos]=='{'
         || URL[Pos]=='}'
         || URL[Pos]=='|'
         || URL[Pos]=='\\'
         || URL[Pos]=='^'
         || URL[Pos]=='['
         || URL[Pos]==']'
         || URL[Pos]=='`'
         /*|| URL[Pos]==';'
         || URL[Pos]=='/'
         || URL[Pos]=='?'
         || URL[Pos]==':'
         || URL[Pos]=='@'
         || URL[Pos]=='&'
         || URL[Pos]=='='
         || URL[Pos]=='+'
         || URL[Pos]=='$'
         || URL[Pos]==','*/)
            Result+='%'+Hex2Char((unsigned char)URL[Pos]);
        else
            Result+=URL[Pos];
    }
    return Result;
}

//---------------------------------------------------------------------------
#ifndef WSTRING_MISSING
std::wstring URL_Encoded_Encode (const std::wstring& URL)
{
    wstring Result;
    wstring::size_type Pos;
    for (Pos=0; Pos<URL.size(); Pos++)
    {
        if (URL[Pos]<=L'\x20'
         || URL[Pos]==L'\x7F'
         || URL[Pos]==L' '
         || URL[Pos]==L'<'
         || URL[Pos]==L'>'
         || URL[Pos]==L'#'
         || URL[Pos]==L'%'
         || URL[Pos]==L'\"'
         || URL[Pos]==L'{'
         || URL[Pos]==L'}'
         || URL[Pos]==L'|'
         || URL[Pos]==L'\\'
         || URL[Pos]==L'^'
         || URL[Pos]==L'['
         || URL[Pos]==L']'
         || URL[Pos]==L'`'
         /*|| URL[Pos]==L';'
         || URL[Pos]==L'/'
         || URL[Pos]==L'?'
         || URL[Pos]==L':'
         || URL[Pos]==L'@'
         || URL[Pos]==L'&'
         || URL[Pos]==L'=L'
         || URL[Pos]==L'+'
         || URL[Pos]==L'$'
         || URL[Pos]==L','*/)
            Result+=L'%'+Hex2Char(URL[Pos]);
        else
            Result+=URL[Pos];
    }
    return Result;
}
#endif //WSTRING_MISSING

//---------------------------------------------------------------------------
std::string URL_Encoded_Decode (const std::string& URL)
{
    string Result;
    string::size_type Pos;
    for (Pos=0; Pos<URL.size(); Pos++)
    {
        if (URL[Pos]=='%' && Pos+2<URL.size()) //At least 3 chars
        {
            const unsigned char Char1 = Char2Hex((unsigned char)URL[Pos+1]);
            const unsigned char Char2 = Char2Hex((unsigned char)URL[Pos+2]);
            const unsigned char Char = (Char1<<4) | Char2;
            Result+=Char;
            Pos+=2; //3 chars are used
        }
        else if (URL[Pos]=='+')
            Result+=' ';
        else
            Result+=URL[Pos];
    }
    return Result;
}

//---------------------------------------------------------------------------
#ifndef WSTRING_MISSING
std::wstring URL_Encoded_Decode (const std::wstring& URL)
{
    wstring Result;
    wstring::size_type Pos;
    for (Pos=0; Pos<URL.size(); Pos++)
    {
        if (URL[Pos]==L'%' && Pos+2<URL.size()) //At least 3 chars
        {
            const wchar_t Char1 = Char2Hex(URL[Pos+1]);
            const wchar_t Char2 = Char2Hex(URL[Pos+2]);
            const wchar_t Char = (Char1<<4) | Char2;
            Result+=Char;
            Pos+=2; //3 chars are used
        }
        else if (URL[Pos]==L'+')
            Result+=L' ';
        else
            Result+=URL[Pos];
    }
    return Result;
}
#endif //WSTRING_MISSING

//***************************************************************************
// Cleanup
//***************************************************************************

//---------------------------------------------------------------------------
void TrimLeft (std::string& String, char ToTrim)
{
    string::size_type First=0;
    while (String.operator[](First)==ToTrim)
        First++;
    String.assign (String.c_str()+First);
}

} //Namespace

} //Namespace

} //Namespace
