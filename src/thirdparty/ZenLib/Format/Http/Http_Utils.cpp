// ZenLib::Server::Http::Utils - Utils for HTTP handling
// Copyright (C) 2008-2010 MediaArea.net SARL, Info@MediaArea.net
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
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#include "ZenLib/Conf_Internal.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
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
string Hex2Char (unsigned char Char)
{
    string Result;
    Result+=(char)((Char/16>=10?'a'-10:'0')+Char/16);
    Result+=(char)((Char%16>=10?'a'-10:'0')+Char%16);
    return Result;
}

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
         || URL[Pos]=='\0x7F'
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
            Result+='%'+Hex2Char(URL[Pos]);
        else
            Result+=URL[Pos];
    }
    return Result;
}

//---------------------------------------------------------------------------
std::string URL_Encoded_Decode (const std::string& URL)
{
    string Result;
    string::size_type Pos;
    for (Pos=0; Pos<URL.size(); Pos++)
    {
        if (URL[Pos]=='%' && Pos+2<URL.size()) //At least 3 chars
        {
            const unsigned char Char1 = Char2Hex(URL[Pos+1]);
            const unsigned char Char2 = Char2Hex(URL[Pos+2]);
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
