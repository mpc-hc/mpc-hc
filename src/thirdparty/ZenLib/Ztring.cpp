// ZenLib::Ztring - std::(w)string is better
// Copyright (C) 2002-2010 MediaArea.net SARL, Info@MediaArea.net
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
//
// More methods for std::(w)string
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#include "ZenLib/Conf_Internal.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef ZENLIB_USEWX
    #include <wx/strconv.h>
    #include <wx/datetime.h>
#else //ZENLIB_USEWX
    #ifdef ZENLIB_STANDARD
        #undef WINDOWS
    #endif
    #ifdef WINDOWS
        #undef __TEXT
        #include <windows.h>
        #include <tchar.h>
    #endif
#endif //ZENLIB_USEWX
#ifdef __MINGW32__
    #include <windows.h>
#endif //__MINGW32__
#include <cstring>
#include <cstdio>
#include <cstdlib>
#ifdef SS
   #undef SS //Solaris defines this in cstdlib
#endif
#include <ctime>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "ZenLib/Ztring.h"
#include "ZenLib/OS_Utils.h"
#include "ZenLib/File.h"
using namespace std;
//---------------------------------------------------------------------------

namespace ZenLib
{

//---------------------------------------------------------------------------
Ztring EmptyZtring;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(__MINGW32__) || defined(__sun__) || defined(__mips__) || defined(__mipsel__)
    #if defined (_UNICODE)
        #define _tnprintf snwprintf
    #else
        #define _tnprintf snprintf
    #endif
#endif
typedef basic_stringstream<Char>  tStringStream;
typedef basic_istringstream<Char> tiStringStream;
typedef basic_ostringstream<Char> toStringStream;
//---------------------------------------------------------------------------

//***************************************************************************
// Operators
//***************************************************************************

Char &Ztring::operator() (size_type Pos)
{
    if (Pos>size())
        resize(Pos);
    return operator[] (Pos);
}

//***************************************************************************
// Assign
//***************************************************************************

bool Ztring::Assign_FromFile (const Ztring &FileName)
{
    File F;
    if (!F.Open(FileName))
        return false;
    int64u F_Size=F.Size_Get();
    if (F_Size>((size_t)-1)-1)
        return false;

    //Creating buffer
    int8u* Buffer=new int8u[(size_t)F_Size+1];
    size_t Buffer_Offset=0;

    //Reading the file
    while(Buffer_Offset<F_Size)
    {
        size_t BytesRead=F.Read(Buffer+Buffer_Offset, (size_t)F_Size-Buffer_Offset);
        if (BytesRead==0)
            break; //Read is finished
        Buffer_Offset+=BytesRead;
    }
    if (Buffer_Offset<F_Size)
        return false;
    Buffer[Buffer_Offset]='\0';

    //Filling
    assign((const Char*)Buffer);
    delete[] Buffer;

    return true;
}

//***************************************************************************
// Conversions
//***************************************************************************

Ztring& Ztring::From_Unicode (const wchar_t* S)
{
    if (S==NULL)
        return *this;

    #ifdef _UNICODE
        assign(S);
    #else
        #ifdef ZENLIB_USEWX
            size_type OK=wxConvCurrent->WC2MB(NULL, S, 0);
            if (OK!=0 && OK!=Error)
                assign(wxConvCurrent->cWC2MB(S));
        #else //ZENLIB_USEWX
            #ifdef WINDOWS
                if (IsWin9X())
                {
                    clear();
                    return *this; //Is not possible, UTF-8 is not supported by Win9X
                }
                int Size=WideCharToMultiByte(CP_UTF8, 0, S, -1, NULL, 0, NULL, NULL);
                if (Size!=0)
                {
                    char* AnsiString=new char[Size+1];
                    WideCharToMultiByte(CP_UTF8, 0, S, -1, AnsiString, Size, NULL, NULL);
                    AnsiString[Size]='\0';
                    assign (AnsiString);
                    delete[] AnsiString;
                }
                else
                    clear();
            #else //WINDOWS
                size_t Size=wcstombs(NULL, S, 0);
                if (Size!=0 && Size!=(size_t)-1)
                {
                    char* AnsiString=new char[Size+1];
                    Size=wcstombs(AnsiString, S, wcslen(S));
                    AnsiString[Size]='\0';
                    assign (AnsiString);
                    delete[] AnsiString;
                }
                else
                    clear();
            #endif
        #endif //ZENLIB_USEWX
    #endif
    return *this;
}

Ztring& Ztring::From_Unicode (const wchar_t *S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length==Error)
        Length=wcslen(S+Start);
    wchar_t* Temp=new wchar_t[Length+1];
    wcsncpy (Temp, S+Start, Length);
    Temp[Length]=_T('\0');

    From_Unicode(Temp);
    delete[] Temp; //Temp=NULL;
    return *this;
}

Ztring& Ztring::From_UTF8 (const char* S)
{
    if (S==NULL)
        return *this;

    #ifdef ZENLIB_USEWX
        size_type OK=wxConvUTF8.MB2WC(NULL, S, 0);
        if (OK!=0 && OK!=Error)
            #ifdef _UNICODE
                assign(wxConvUTF8.cMB2WC(S).data());
            #else
                assign(wxConvCurrent->cWC2MB(wxConvUTF8.cMB2WC(S)));
            #endif
    #else //ZENLIB_USEWX
        #ifdef _UNICODE
            #ifdef WINDOWS
                if (IsWin9X())
                {
                    clear();
                    const int8u* Z=(const int8u*)S;
                    while (*Z) //0 is end
                    {
                        //1 byte
                        if (*Z<0x80)
                        {
                            operator += ((wchar_t)(*Z));
                            Z++;
                        }
                        //2 bytes
                        else if ((*Z&0xE0)==0xC0)
                        {
                            if ((*(Z+1)&0xC0)==0x80)
                            {
                                operator += ((((wchar_t)(*Z&0x1F))<<6)|(*(Z+1)&0x3F));
                                Z+=2;
                            }
                            else
                                break; //Bad character
                        }
                        else
                            break; //Bad character (or to be encoded in UTF-16LE, not yet supported)
                    }
                }
                else
                {
                    int Size=MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, S, -1, NULL, 0);
                    if (Size!=0)
                    {
                        Char* WideString=new Char[Size+1];
                        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, S, -1, WideString, Size);
                        WideString[Size]=L'\0';
                        assign (WideString+(WideString[0]==0xFEFF?1:0));
                        delete[] WideString; //WideString=NULL;
                    }
                    else
                        clear();
                }
            #else //WINDOWS
                clear();
                const int8u* Z=(const int8u*)S;
                while (*Z) //0 is end
                {
                    //1 byte
                    if (*Z<0x80)
                    {
                        operator += ((wchar_t)(*Z));
                        Z++;
                    }
                    //2 bytes
                    else if ((*Z&0xE0)==0xC0)
                    {
                        if ((*(Z+1)&0xC0)==0x80)
                        {
                            operator += ((((wchar_t)(*Z&0x1F))<<6)|(*(Z+1)&0x3F));
                            Z+=2;
                        }
                        else
                            break; //Bad character
                    }
                    //3 bytes
                    else if ((*Z&0xF0)==0xE0)
                    {
                        if ((*(Z+1)&0xC0)==0x80 && (*(Z+2)&0xC0)==0x80)
                        {
                            operator += ((((wchar_t)(*Z&0x0F))<<12)|((*(Z+1)&0x3F)<<6)|(*(Z+2)&0x3F));
                            Z+=3;
                        }
                        else
                            break; //Bad character
                    }
                    //4 bytes
                    else if ((*Z&0xF8)==0xF0)
                    {
                        if ((*(Z+1)&0xC0)==0x80 && (*(Z+2)&0xC0)==0x80 && (*(Z+2)&0xC0)==0x80)
                        {
                            operator += ((((wchar_t)(*Z&0x0F))<<18)|((*(Z+1)&0x3F)<<12)||((*(Z+2)&0x3F)<<6)|(*(Z+3)&0x3F));
                            Z+=4;
                        }
                        else
                            break; //Bad character
                    }
                    else
                        break; //Bad character
                }
            #endif
        #else
            assign(S); //Not implemented
        #endif
    #endif //ZENLIB_USEWX
    return *this;
}

Ztring& Ztring::From_UTF8 (const char* S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length==Error)
        Length=strlen(S+Start);
    char* Temp=new char[Length+1];
    strncpy (Temp, S+Start, Length);
    Temp[Length]='\0';

    From_UTF8(Temp);
    delete[] Temp; //Temp=NULL;
    return *this;
}

Ztring& Ztring::From_UTF16 (const char* S)
{
    if (S==NULL)
        return *this;

         if ((unsigned char)S[0]==(unsigned char)0xFF && (unsigned char)S[1]==(unsigned char)0xFE)
        return From_UTF16LE(S+2);
    else if ((unsigned char)S[0]==(unsigned char)0xFE && (unsigned char)S[1]==(unsigned char)0xFF)
        return From_UTF16BE(S+2);
    else if ((unsigned char)S[0]==(unsigned char)0x00 && (unsigned char)S[1]==(unsigned char)0x00)
    {
        clear(); //No begin!
        return *this;
    }
    else
        return From_UTF16LE(S); //Not sure, default
}

Ztring& Ztring::From_UTF16 (const char* S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length<2)
        return *this;

         if ((unsigned char)S[0]==(unsigned char)0xFF && (unsigned char)S[1]==(unsigned char)0xFE)
        return From_UTF16LE(S+2, Start, Length-2);
    else if ((unsigned char)S[0]==(unsigned char)0xFE && (unsigned char)S[1]==(unsigned char)0xFF)
        return From_UTF16BE(S+2, Start, Length-2);
    else if ((unsigned char)S[0]==(unsigned char)0x00 && (unsigned char)S[1]==(unsigned char)0x00)
    {
        clear(); //No begin!
        return *this;
    }
    else
        return From_UTF16LE(S, Start, Length); //Not sure, default
}

Ztring& Ztring::From_UTF16BE (const char* S)
{
    if (S==NULL)
        return *this;

    #ifdef ZENLIB_USEWX
        //clear(); return *this;
        wxMBConvUTF16BE wxConvUTF16BE;
        size_type OK=wxConvUTF16BE.MB2WC(NULL, S, 0);
        if (OK!=0 && OK!=Error)
            #ifdef _UNICODE
                assign(wxConvUTF16BE.cMB2WC(S).data());
            #else
                assign(wxConvCurrent->cWC2MB(wxConvUTF16BE.cMB2WC(S)));
            #endif
    #else //ZENLIB_USEWX
        #ifdef WINDOWS
            clear();
            const wchar_t* SW=(const wchar_t*)S;
            size_t Pos=0;
            while (SW[Pos]!=_T('\0'))
            {
                Char Temp=(Char)(((SW[Pos]&0xFF00)>>8)+((SW[Pos]&0x00FF)<<8));
                append(1, Temp);
                Pos++;
            }
        #else //WINDOWS
            clear();
            while (S[0]!=0 || S[1]!=0)
            {
                append(1, (Char)BigEndian2int16u(S));
                S+=2;
            }
        #endif
    #endif //ZENLIB_USEWX
    return *this;
}

Ztring& Ztring::From_UTF16BE (const char* S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length==Error)
    {
        Length=0;
        while(S[Length]!=0x0000)
            Length++;
    }

    char* Temp=new char[Length+2];
    memcpy (Temp, S+Start, Length);
    Temp[Length+0]=0x00;
    Temp[Length+1]=0x00;
    reserve(Length);
    From_UTF16BE(Temp);
    delete[] Temp; //Temp=NULL;
    return *this;
}

Ztring& Ztring::From_UTF16LE (const char* S)
{
    if (S==NULL)
        return *this;

    #ifdef ZENLIB_USEWX
        //clear(); return *this;
        wxMBConvUTF16LE wxConvUTF16LE;
        size_type OK=wxConvUTF16LE.MB2WC(NULL, S, 0);
        if (OK!=0 && OK!=Error)
            #ifdef _UNICODE
                assign(wxConvUTF16LE.cMB2WC(S).data());
            #else
                assign(wxConvCurrent->cWC2MB(wxConvUTF16LE.cMB2WC(S)));
            #endif
    #else //ZENLIB_USEWX
        #ifdef WINDOWS
            #ifdef UNICODE
                const wchar_t* SW=(const wchar_t*)S;
                assign(SW);
            #else
                clear(); //Not implemented
            #endif
        #else //WINDOWS
            clear();
            while (S[0]!=0 || S[1]!=0)
            {
                append(1, (Char)LittleEndian2int16u(S));
                S+=2;
            }
        #endif
    #endif //ZENLIB_USEWX
    return *this;
}

Ztring& Ztring::From_UTF16LE (const char* S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length==Error)
    {
        Length=0;
        while(S[Length]!=0x0000)
            Length+=2;
    }

    char* Temp=new char[Length+2];
    memcpy (Temp, S+Start, Length);
    Temp[Length+0]=0x00;
    Temp[Length+1]=0x00;
    From_UTF16LE(Temp);
    delete[] Temp; //Temp=NULL;
    return *this;
}

Ztring& Ztring::From_Local (const char* S)
{
    if (S==NULL)
        return *this;

    #ifdef _UNICODE
        #ifdef ZENLIB_USEWX
            size_type OK=wxConvCurrent->MB2WC(NULL, S, 0);
            if (OK!=0 && OK!=Error)
                assign(wxConvCurrent->cMB2WC(S).data());
        #else //ZENLIB_USEWX
            #ifdef WINDOWS
                int Size=MultiByteToWideChar(CP_ACP, 0, S, -1, NULL, 0);
                if (Size!=0)
                {
                    wchar_t* WideString=new wchar_t[Size+1];
                    MultiByteToWideChar(CP_ACP, 0, S, -1, WideString, Size);
                    WideString[Size]=L'\0';
                    assign (WideString);
                    delete[] WideString; //WideString=NULL;
                }
                else
                    clear();
            #else //WINDOWS
                size_t Size=mbstowcs(NULL, S, 0);
                if (Size!=0 && Size!=(size_t)-1)
                {
                    wchar_t* WideString=new wchar_t[Size+1];
                    Size=mbstowcs(WideString, S, Size);
                    WideString[Size]=L'\0';
                    assign (WideString);
                    delete[] WideString; //WideString=NULL;
                }
                else
                    clear();
            #endif
        #endif //ZENLIB_USEWX
    #else
        assign(S);
    #endif
    return *this;
}

Ztring& Ztring::From_Local (const char* S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length==Error)
        Length=strlen(S+Start);
    #ifdef _UNICODE
        char* Temp=new char[Length+1];
        strncpy (Temp, S+Start, Length);
        Temp[Length]='\0';
        From_Local(Temp);
        delete[] Temp; //Temp=NULL;
    #else
        assign(S+Start, Length);
        if (find(_T('\0'))!=std::string::npos)
            resize(find(_T('\0')));
    #endif
    return *this;
}

Ztring& Ztring::From_ISO_8859_1(const char* S)
{
    size_t Length = strlen(S);
    wchar_t* Temp = new wchar_t[Length +1];

    for (size_t Pos=0; Pos<Length+1; Pos++)
        Temp[Pos]=(wchar_t)((int8u)S[Pos]);

    From_Unicode(Temp);
    delete[] Temp;
    return *this;
}

Ztring& Ztring::From_ISO_8859_1(const char* S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length==Error)
        Length=strlen(S+Start);
    #ifdef _UNICODE
        char* Temp = new char[Length+1];
        strncpy(Temp, S +Start, Length);
        Temp[Length] = '\0';
        From_ISO_8859_1(Temp);
        delete[] Temp;
    #else
        assign(S +Start, Length);
        if (find(_T('\0')) != std::string::npos)
            resize(find(_T('\0')));
    #endif
    return *this;
}

Ztring& Ztring::From_GUID (const int128u S)
{
    Ztring S1;
    S1.From_CC1((int8u) ((S.hi&0x000000FF00000000LL)>>32)); append(S1);
    S1.From_CC1((int8u) ((S.hi&0x0000FF0000000000LL)>>40)); append(S1);
    S1.From_CC1((int8u) ((S.hi&0x00FF000000000000LL)>>48)); append(S1);
    S1.From_CC1((int8u) ((S.hi&0xFF00000000000000LL)>>56)); append(S1); append(_T("-"));
    S1.From_CC1((int8u) ((S.hi&0x0000000000FF0000LL)>>16)); append(S1);
    S1.From_CC1((int8u) ((S.hi&0x00000000FF000000LL)>>24)); append(S1); append(_T("-"));
    S1.From_CC1((int8u) ( S.hi&0x00000000000000FFLL     )); append(S1);
    S1.From_CC1((int8u) ((S.hi&0x000000000000FF00LL)>> 8)); append(S1); append(_T("-"));
    S1.From_CC2((int16u)((S.lo&0xFFFF000000000000LL)>>48)); append(S1); append(_T("-"));
    S1.From_CC2((int16u)((S.lo&0x0000FFFF00000000LL)>>32)); append(S1);
    S1.From_CC2((int16u)((S.lo&0x00000000FFFF0000LL)>>16)); append(S1);
    S1.From_CC2((int16u)( S.lo&0x000000000000FFFFLL     )); append(S1);

    return *this;
}

Ztring& Ztring::From_UUID (const int128u S)
{
    Ztring S1;
    S1.From_CC2((int16u)((S.hi&0xFFFF000000000000LL)>>48)); assign(S1);
    S1.From_CC2((int16u)((S.hi&0x0000FFFF00000000LL)>>32)); append(S1); append(_T("-"));
    S1.From_CC2((int16u)((S.hi&0x00000000FFFF0000LL)>>16)); append(S1); append(_T("-"));
    S1.From_CC2((int16u)( S.hi&0x000000000000FFFFLL     )); append(S1); append(_T("-"));
    S1.From_CC2((int16u)((S.lo&0xFFFF000000000000LL)>>48)); append(S1); append(_T("-"));
    S1.From_CC2((int16u)((S.lo&0x0000FFFF00000000LL)>>32)); append(S1);
    S1.From_CC2((int16u)((S.lo&0x00000000FFFF0000LL)>>16)); append(S1);
    S1.From_CC2((int16u)( S.lo&0x000000000000FFFFLL     )); append(S1);

    return *this;
}

Ztring& Ztring::From_CC4 (const int32u S)
{
    std::string S1;
    S1.append(1, (char)((S&0xFF000000)>>24));
    S1.append(1, (char)((S&0x00FF0000)>>16));
    S1.append(1, (char)((S&0x0000FF00)>> 8));
    S1.append(1, (char)((S&0x000000FF)>> 0));
    From_Local(S1.c_str());

    //Test
    if (empty())
        assign(_T("(empty)"));

    return *this;
}

Ztring& Ztring::From_CC3 (const int32u S)
{
    std::string S1;
    S1.append(1, (char)((S&0x00FF0000)>>16));
    S1.append(1, (char)((S&0x0000FF00)>> 8));
    S1.append(1, (char)((S&0x000000FF)>> 0));
    From_Local(S1.c_str());

    //Test
    if (empty())
        assign(_T("(empty)"));

    return *this;
}

Ztring& Ztring::From_CC2 (const int16u S)
{
    clear();
    Ztring Pos1; Pos1.From_Number(S, 16);
    resize(4-Pos1.size(), _T('0'));
    append(Pos1);
    MakeUpperCase();

    return *this;
}

Ztring& Ztring::From_CC1 (const int8u S)
{
    clear();
    Ztring Pos1; Pos1.From_Number(S, 16);
    resize(2-Pos1.size(), _T('0'));
    append(Pos1);
    MakeUpperCase();

    return *this;
}

Ztring& Ztring::From_Number (const int8s I, int8u Radix)
{
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) )
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        #ifdef __MINGW32__
            _itot (I, C1, Radix);
        #else
            _tnprintf(C1, 32, Radix==10?_T("%d"):(Radix==16?_T("%x"):(Radix==8?_T("%o"):_T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        #ifdef UNICODE
            SS << setbase(Radix) << I;
        #else //UNICODE
            SS << setbase(Radix) << (size_t)I; //On linux (at least), (un)signed char is detected as a char
        #endif //UNICODE
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int8u I, int8u Radix)
{
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) )
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        #ifdef __MINGW32__
            _ultot (I, C1, Radix);
        #else
            _tnprintf(C1, 32, Radix==10?_T("%d"):(Radix==16?_T("%x"):(Radix==8?_T("%o"):_T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        #ifdef UNICODE
            SS << setbase(Radix) << I;
        #else //UNICODE
            SS << setbase(Radix) << (size_t)I; //On linux (at least), (un)signed char is detected as a char
        #endif //UNICODE
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int16s I, int8u Radix)
{
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) )
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        #ifdef __MINGW32__
            _itot (I, C1, Radix);
        #else
            _tnprintf(C1, 32, Radix==10?_T("%d"):(Radix==16?_T("%x"):(Radix==8?_T("%o"):_T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        SS << setbase(Radix) << I;
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int16u I, int8u Radix)
{
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) )
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        #ifdef __MINGW32__
            _ultot (I, C1, Radix);
        #else
            _tnprintf(C1, 32, Radix==10?_T("%u"):(Radix==16?_T("%x"):(Radix==8?_T("%o"):_T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        SS << setbase(Radix) << I;
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int32s I, int8u Radix)
{
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) )
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        #ifdef __MINGW32__
            _itot (I, C1, Radix);
        #else
            _tnprintf(C1, 32, Radix==10?_T("%ld"):(Radix==16?_T("%lx"):(Radix==8?_T("%lo"):_T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        SS << setbase(Radix) << I;
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int32u I, int8u Radix)
{
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) )
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        #ifdef __MINGW32__
            _ultot (I, C1, Radix);
        #else
            _tnprintf(C1, 32, Radix==10?_T("%lu"):(Radix==16?_T("%lx"):(Radix==8?_T("%lo"):_T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        SS << setbase(Radix) << I;
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int64s I, int8u Radix)
{
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) )
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[65];
        #ifdef __MINGW32__
            _i64tot (I, C1, Radix);
        #else
            _tnprintf(C1, 64, Radix==10?_T("%lld"):(Radix==16?_T("%llx"):(Radix==8?_T("%llo"):_T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        SS << setbase(Radix) << I;
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int64u I, int8u Radix)
{
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) )
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[65];
        #ifdef __MINGW32__
            _ui64tot (I, C1, Radix);
        #else
            _tnprintf(C1, 64, Radix==10?_T("%llu"):(Radix==16?_T("%llx"):(Radix==8?_T("%llo"):_T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        SS << setbase(Radix) << I;
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int128u I, int8u Radix)
{
    From_Local(I.toString(Radix));

    return *this;
}

Ztring& Ztring::From_Number (const float32 F, int8u Precision, ztring_t Options)
{
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) ) || defined(__mips__) || defined(__mipsel__)
        Char C1[100];
        _tnprintf (C1, 99, (Ztring(_T("%."))+Ztring::ToZtring(Precision)+_T("f")).c_str(), F);
        assign(C1);
    #else
        toStringStream SS;
        SS << setprecision(Precision) << fixed << F;
        assign(SS.str());
        #if defined(__BORLANDC__)
            FindAndReplace(_T(","), _T(".")); //Borland C++ Builder 2010+Windows Seven put a comma for istringstream, but does not support comma for ostringstream
        #endif
    #endif

    if ((Options & Ztring_NoZero && size()>0) && find(_T('.'))>0)
    {
        while (size()>0 && ((*this)[size()-1]==_T('0')))
            resize(size()-1);
        if (size()>0 && (*this)[size()-1]==_T('.'))
            resize(size()-1);
    }

    return *this;
}

Ztring& Ztring::From_Number (const float64 F, int8u Precision, ztring_t Options)
{
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) ) || defined(__mips__) || defined(__mipsel__)
        Char C1[100];
        _tnprintf (C1, 99, (Ztring(_T("%."))+Ztring::ToZtring(Precision)+_T("f")).c_str(), F);
        assign(C1);
    #else
        toStringStream SS;
        SS << setprecision(Precision) << fixed << F;
        assign(SS.str());
        #if defined(__BORLANDC__)
            FindAndReplace(_T(","), _T(".")); //Borland C++ Builder 2010+Windows Seven put a comma for istringstream, but does not support comma for ostringstream
        #endif
    #endif

    if ((Options & Ztring_NoZero && size()>0) && find(_T('.'))>0)
    {
        while (size()>0 && ((*this)[size()-1]==_T('0')))
            resize(size()-1);
        if (size()>0 && (*this)[size()-1]==_T('.'))
            resize(size()-1);
    }

    return *this;
}

Ztring& Ztring::From_Number (const float80 F, int8u Precision, ztring_t Options)
{
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) ) || defined(__mips__) || defined(__mipsel__)
        Char C1[100];
        _tnprintf (C1, 99, (Ztring(_T("%."))+Ztring::ToZtring(Precision)+_T("f")).c_str(), F);
        assign(C1);
    #else
        toStringStream SS;
        SS << setprecision(Precision) << fixed << F;
        assign(SS.str());
        #if defined(__BORLANDC__)
            FindAndReplace(_T(","), _T(".")); //Borland C++ Builder 2010+Windows Seven put a comma for istringstream, but does not support comma for ostringstream
        #endif
    #endif

    if ((Options & Ztring_NoZero && size()>0) && find(_T('.'))>0)
    {
        while (size()>0 && ((*this)[size()-1]==_T('0')))
            resize(size()-1);
        if (size()>0 && (*this)[size()-1]==_T('.'))
            resize(size()-1);
    }

    return *this;
}

#ifdef NEED_SIZET
Ztring& Ztring::From_Number (const size_t I, int8u Radix)
{
    toStringStream SS;
    SS << setbase(Radix) << I;
    assign(SS.str());
    MakeUpperCase();
    return *this;
}
#endif //NEED_SIZET

Ztring& Ztring::From_BCD     (const int8u I)
{
    #if ( defined(__sun__) && !defined(__sparc__) )
        clear();
        append(1, _T('0')+I/0x10);
        append(1, _T('0')+I%0x10);
    #else
        toStringStream SS;
        SS << I/0x10;
        SS << I%0x10;
        assign(SS.str());
    #endif
    return *this;
}

//---------------------------------------------------------------------------
Ztring& Ztring::Duration_From_Milliseconds (const int64s Value_)
{
    int64s Value=Value_;
    bool Negative=false;
    if (Value<0)
    {
        Value=-Value;
        Negative=true;
    }

    int64u HH=(int8u)(Value/1000/60/60);
    int64u MM=Value/1000/60   -((HH*60));
    int64u SS=Value/1000      -((HH*60+MM)*60);
    int64u MS=Value           -((HH*60+MM)*60+SS)*1000;
    Ztring DateT;
    Ztring Date;
    DateT.From_Number(HH); if (DateT.size()<2){DateT=Ztring(_T("0"))+DateT;}
    Date+=DateT;
    Date+=_T(":");
    DateT.From_Number(MM); if (DateT.size()<2){DateT=Ztring(_T("0"))+DateT;}
    Date+=DateT;
    Date+=_T(":");
    DateT.From_Number(SS); if (DateT.size()<2){DateT=Ztring(_T("0"))+DateT;}
    Date+=DateT;
    Date+=_T(".");
    DateT.From_Number(MS); if (DateT.size()<2){DateT=Ztring(_T("00"))+DateT;} else if (DateT.size()<3){DateT=Ztring(_T("0"))+DateT;}
    Date+=DateT;
    if (Negative)
    {
        assign(_T("-"));
        append(Date);
    }
    else
        assign (Date.c_str());
    return *this;
}

//---------------------------------------------------------------------------
Ztring& Ztring::Duration_From_Milliseconds (const int64u Value)
{
    int64u HH=(int8u)(Value/1000/60/60);
    int64u MM=Value/1000/60   -((HH*60));
    int64u SS=Value/1000      -((HH*60+MM)*60);
    int64u MS=Value           -((HH*60+MM)*60+SS)*1000;
    Ztring DateT;
    Ztring Date;
    DateT.From_Number(HH); if (DateT.size()<2){DateT=Ztring(_T("0"))+DateT;}
    Date+=DateT;
    Date+=_T(":");
    DateT.From_Number(MM); if (DateT.size()<2){DateT=Ztring(_T("0"))+DateT;}
    Date+=DateT;
    Date+=_T(":");
    DateT.From_Number(SS); if (DateT.size()<2){DateT=Ztring(_T("0"))+DateT;}
    Date+=DateT;
    Date+=_T(".");
    DateT.From_Number(MS); if (DateT.size()<2){DateT=Ztring(_T("00"))+DateT;} else if (DateT.size()<3){DateT=Ztring(_T("0"))+DateT;}
    Date+=DateT;
    assign (Date.c_str());
    return *this;
}

Ztring& Ztring::Date_From_Milliseconds_1601 (const int64u Value)
{
    if (Value>=11644473600000LL) //Values <1970 are not supported
    {
        Date_From_Seconds_1970((int32u)((Value-11644473600000LL)/1000));
        append(_T("."));
        Ztring Milliseconds; Milliseconds.From_Number(Value%1000);
        while (Milliseconds.size()<3)
            Milliseconds+=_T('0');
        append(Milliseconds);
    }
    else
        clear(); //Not supported

    return *this;
}

Ztring& Ztring::Date_From_Seconds_1601 (const int64u Value)
{
    if (Value>=11644473600LL) //Values <1970 are not supported
        Date_From_Seconds_1970((int32u)(Value-11644473600LL));
    else
        clear(); //Not supported

    return *this;
}

Ztring& Ztring::Date_From_Seconds_1904 (const int64u Value)
{
    #ifdef ZENLIB_USEWX
        /*
        wxDateTime Date;
        Date.SetYear(1904);
        Date.SetMonth(wxDateTime::Jan);
        Date.SetDay(1);
        Date.SetHour(0);
        Date.SetMinute(0);
        Date.SetSecond(0);
        if (Value>=0x80000000)
        {
            //wxTimeSpan doesn't support unsigned int
            int64u Value2=Value;
            while (Value2>0x7FFFFFFF)
            {
                Date+=wxTimeSpan::Seconds(0x7FFFFFFF);
                Value2-=0x7FFFFFFF;
            }
            Date+=wxTimeSpan::Seconds(Value2);
        }
        else
            Date+=wxTimeSpan::Seconds(Value);

        Ztring ToReturn=_T("UTC ");
        ToReturn+=Date.FormatISODate();
        ToReturn+=_T(" ");
        ToReturn+=Date.FormatISOTime();

        assign (ToReturn.c_str());
        */ //WxDateTime is buggy???
        if (Value>2082844800 && Value<2082844800+0x100000000LL) //Values <1970 and >2038 are not supported, 1970-01-01 00:00:00 is considered as not possible too
            Date_From_Seconds_1970((int32u)(Value-2082844800));
        else
            clear(); //Not supported

    #else //ZENLIB_USEWX
        if (Value>2082844800 && Value<2082844800+0x100000000LL) //Values <1970 and >2038 are not supported, 1970-01-01 00:00:00 is considered as not possible too
            Date_From_Seconds_1970((int32u)(Value-2082844800));
        else
            clear(); //Not supported
    #endif //ZENLIB_USEWX
    return *this;
}

Ztring& Ztring::Date_From_Seconds_1970 (const int32u Value)
{
    time_t Time=(time_t)Value;
    struct tm *Gmt=gmtime(&Time);
    Ztring DateT;
    Ztring Date=_T("UTC ");
    Date+=Ztring::ToZtring((Gmt->tm_year+1900));
    Date+=_T("-");
    DateT.From_Number(Gmt->tm_mon+1); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_mon+1);}
    Date+=DateT;
    Date+=_T("-");
    DateT.From_Number(Gmt->tm_mday); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_mday);}
    Date+=DateT;
    Date+=_T(" ");
    DateT.From_Number(Gmt->tm_hour); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_hour);}
    Date+=DateT;
    Date+=_T(":");
    DateT=Ztring::ToZtring(Gmt->tm_min); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_min);}
    Date+=DateT;
    Date+=_T(":");
    DateT.From_Number(Gmt->tm_sec); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_sec);}
    Date+=DateT;
    assign (Date.c_str());
    return *this;
}

Ztring& Ztring::Date_From_Seconds_1970_Local (const int32u Value)
{
    time_t Time=(time_t)Value;
    struct tm *Gmt=localtime(&Time);
    Ztring DateT;
    Ztring Date;
    Date+=Ztring::ToZtring((Gmt->tm_year+1900));
    Date+=_T("-");
    DateT.From_Number(Gmt->tm_mon+1); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_mon+1);}
    Date+=DateT;
    Date+=_T("-");
    DateT.From_Number(Gmt->tm_mday); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_mday);}
    Date+=DateT;
    Date+=_T(" ");
    DateT.From_Number(Gmt->tm_hour); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_hour);}
    Date+=DateT;
    Date+=_T(":");
    DateT=Ztring::ToZtring(Gmt->tm_min); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_min);}
    Date+=DateT;
    Date+=_T(":");
    DateT.From_Number(Gmt->tm_sec); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_sec);}
    Date+=DateT;
    assign (Date.c_str());
    return *this;
}

Ztring& Ztring::Date_From_String (const char* Value, size_t Value_Size)
{
    //Only the year
    if (Value_Size<10)
    {
        From_Local(Value, 0, Value_Size);
        return *this;
    }

    #ifdef ZENLIB_USEWX
        Ztring ToReturn=_T("UTC ");
        wxDateTime Date;
        Ztring DateS;
        DateS.From_Local(Value, Value_Size).c_str();
        if (!DateS.empty() && DateS[DateS.size()-1]==_T('\n'))
            DateS.resize(DateS.size()-1);

        //Some strange formating : exactly 24 bytes (or 25 with 0x0A at the end) and Year is at the end
        if (DateS.size()==24 && DateS[23]>=_T('0') && DateS[23]<=_T('9') && DateS[21]>=_T('0') && DateS[21]<=_T('9') && DateS[19]==_T(' '))
            Date.ParseFormat(DateS.c_str(), _T("%a %b %d %H:%M:%S %Y"));
        //ISO date
        else if (DateS.size()==10 && (DateS[4]<_T('0') || DateS[4]>_T('9')) && (DateS[7]<_T('0') || DateS[7]>_T('9')))
        {
            DateS[4]=_T('-');
            DateS[7]=_T('-');
            ToReturn+=DateS;
        }
        //Default
        else
            Date.ParseDateTime(DateS.c_str());

        if (ToReturn.size()<5)
        {
            ToReturn+=Date.FormatISODate();
            ToReturn+=_T(" ");
            ToReturn+=Date.FormatISOTime();
        }
        else if (ToReturn.size()<5)
            ToReturn+=DateS;

        assign (ToReturn.c_str());
    #else //ZENLIB_USEWX
        Ztring DateS; DateS.From_Local(Value, 0, Value_Size);
        if (DateS.size()==20 && DateS[4]==_T('-') && DateS[7]==_T('-') && DateS[10]==_T('T') && DateS[13]==_T(':') && DateS[16]==_T(':') && DateS[19]==_T('Z'))
        {
            DateS.resize(19);
            DateS[10]=_T(' ');
            assign(_T("UTC "));
            append(DateS);
        }
        else if (DateS.size()==23 && DateS[4]==_T('-') && DateS[7]==_T('-') && DateS[10]==_T(' ') && DateS[14]==_T(' ') && DateS[17]==_T(':') && DateS[20]==_T(':'))
        {
            DateS.erase(10, 4);
            //assign(_T("UTC ")); //Is not UTC
            append(DateS);
        }
        else
            From_Local(Value, 0, Value_Size); //Not implemented
    #endif //ZENLIB_USEWX
    return *this;
}

Ztring& Ztring::Date_From_Numbers (const int8u Year, const int8u Month, const int8u Day, const int8u Hour, const int8u Minute, const int8u Second)
{
    Ztring DateT;
    Ztring Date=_T("UTC ");
    DateT.From_Number(Year); if (DateT.size()<2){DateT=Ztring(_T("200"))+Ztring::ToZtring(Year);}; if (DateT.size()<3){DateT=Ztring(_T("20"))+Ztring::ToZtring(Year);}
    Date+=DateT;
    Date+=_T("-");
    DateT.From_Number(Month); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Month);}
    Date+=DateT;
    Date+=_T("-");
    DateT.From_Number(Day); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Day);}
    Date+=DateT;
    Date+=_T(" ");
    DateT.From_Number(Hour); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Hour);}
    Date+=DateT;
    Date+=_T(":");
    DateT=Ztring::ToZtring(Minute); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Minute);}
    Date+=DateT;
    Date+=_T(":");
    DateT.From_Number(Second); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Second);}
    Date+=DateT;
    assign (Date.c_str());
    return *this;
}

#ifndef WSTRING_MISSING
//---------------------------------------------------------------------------
std::wstring Ztring::To_Unicode () const
{
    #ifdef _UNICODE
        return c_str();
    #else //_UNICODE
        #ifdef ZENLIB_USEWX
            return wxConvCurrent->cMB2WC(c_str()).data();
        #else //ZENLIB_USEWX
            return std::wstring(); //Not implemented
        #endif //ZENLIB_USEWX
    #endif //_UNICODE
}
#endif //WSTRING_MISSING

std::string Ztring::To_UTF8 () const
{
    #ifdef _UNICODE
        #ifdef ZENLIB_USEWX
            return wxConvUTF8.cWC2MB(c_str()).data();
        #else //ZENLIB_USEWX
            #ifdef WINDOWS
                if (IsWin9X())
                {
                    std::string ToReturn;
                    const wchar_t* Z=c_str();
                    while (*Z) //0 is end
                    {
                        //1 byte
                        if (*Z<0x80)
                            ToReturn += (char)  (*Z);
                        else if (*Z<0x1000)
                        {
                            ToReturn += (char)(((*Z)>> 6)&0x1F);
                            ToReturn += (char)( (*Z)     &0x3F);
                        }
                        else
                            break; //Bad character (or UTF-16LE, not yet supported)
                        Z++;
                    }
                    return ToReturn;
                }
                else
                {
                    int Size=WideCharToMultiByte(CP_UTF8, 0, c_str(), -1, NULL, 0, NULL, NULL);
                    if (Size!=0)
                    {
                        char* AnsiString=new char[Size+1];
                        WideCharToMultiByte(CP_UTF8, 0, c_str(), -1, AnsiString, Size, NULL, NULL);
                        AnsiString[Size]='\0';
                        std::string ToReturn(AnsiString);
                        delete[] AnsiString; //AnsiString=NULL;
                        return ToReturn;
                    }
                    else
                        return std::string();
                }
            #else //WINDOWS
                std::string ToReturn;
                const wchar_t* Z=c_str();
                while (*Z) //0 is end
                {
                    //1 byte
                    if (*Z<0x80)
                        ToReturn += (char)  (*Z);
                    else if (*Z<0x1000)
                    {
                        ToReturn += (char)(((*Z)>> 6)&0x1F);
                        ToReturn += (char)( (*Z)     &0x3F);
                    }
                    else if (*Z<0x40000)
                    {
                        ToReturn += (char)(((*Z)>>12)&0x0F);
                        ToReturn += (char)(((*Z)>> 6)&0x3F);
                        ToReturn += (char)( (*Z)     &0x3F);
                    }
                    else if (*Z<0x1000000)
                    {
                        ToReturn += (char)(((*Z)>>18)&0x07);
                        ToReturn += (char)(((*Z)>>12)&0x3F);
                        ToReturn += (char)(((*Z)>> 6)&0x3F);
                        ToReturn += (char)( (*Z)     &0x3F);
                    }
                    else
                        break; //Bad character
                    Z++;
                }
                return ToReturn;
            #endif
        #endif //ZENLIB_USEWX
    #else
        #ifdef ZENLIB_USEWX
            return wxConvUTF8.cWC2MB(wxConvCurrent->cMB2WC(c_str())).data();
        #else //ZENLIB_USEWX
            return c_str(); //Not implemented
        #endif //ZENLIB_USEWX
    #endif
}

std::string Ztring::To_Local () const
{
    #ifdef _UNICODE
        #ifdef ZENLIB_USEWX
            wxCharBuffer C=wxConvCurrent->cWC2MB(c_str());
            if (C.data())
                return C.data();
            else
                return std::string();
        #else //ZENLIB_USEWX
            #ifdef WINDOWS
                int Size=WideCharToMultiByte(CP_ACP, 0, c_str(), -1, NULL, 0, NULL, NULL);
                if (Size!=0)
                {
                    char* AnsiString=new char[Size+1];
                    WideCharToMultiByte(CP_ACP, 0, c_str(), -1, AnsiString, Size, NULL, NULL);
                    AnsiString[Size]='\0';
                    std::string ToReturn(AnsiString);
                    delete[] AnsiString; //AnsiString=NULL;
                    return ToReturn;
                }
                else
                    return std::string();
            #else //WINDOWS
                if (empty())
                    return std::string();

                size_t Size=wcstombs(NULL, c_str(), 0);
                if (Size!=0 && Size!=(size_t)-1)
                {
                    char* AnsiString=new char[Size+1];
                    Size=wcstombs(AnsiString, c_str(), Size);
                    if (Size!=0 && Size!=(size_t)-1)
                    {
                        AnsiString[Size]='\0';
                        std::string ToReturn(AnsiString);
                        delete[] AnsiString; //AnsiString=NULL;
                        return ToReturn;
                    }

                    //Failed
                    delete[] AnsiString; //AnsiString=NULL;
                }

                //Trying with bad chars
                char* Result=new char[MB_CUR_MAX];
                std::string AnsiString;
                for (size_t Pos=0; Pos<size(); Pos++)
                {
                    int Result_Size=wctomb(Result, operator[](Pos));
                    if (Result_Size>=0)
                        AnsiString.append(Result, Result_Size);
                    else
                        AnsiString+='?';
                }
                delete[] Result; //Result=NULL;
                return AnsiString;
            #endif
        #endif //ZENLIB_USEWX
    #else
        return c_str();
    #endif
}

//---------------------------------------------------------------------------
int128u Ztring::To_UUID () const
{
    if (size()!=36)
        return 0;

    Ztring Temp=*this;

    for (size_t Pos=0; Pos<36; Pos++)
    {
        if ((Temp[Pos]< _T('0') || Temp[Pos]> _T('9'))
         && (Temp[Pos]< _T('A') || Temp[Pos]> _T('F'))
         && (Temp[Pos]< _T('a') || Temp[Pos]> _T('f')))
            return 0;
        if (Temp[Pos]>=_T('A') && Temp[Pos]<=_T('F'))
        {
            Temp[Pos]-=_T('A');
            Temp[Pos]+=_T('9')+1;
        }
        if (Temp[Pos]>=_T('a') && Temp[Pos]<=_T('f'))
        {
            Temp[Pos]-=_T('a');
            Temp[Pos]+=_T('9')+1;
        }

        switch(Pos)
        {
            case  7 :
            case 12 :
            case 17 :
            case 22 :
                        if (at(Pos+1)!=_T('-'))
                            return 0;
                        Pos++; //Skipping dash in the test
        }
    }
    
    int128u I;
    I.hi=((int64u)((int8u)(Temp[ 0]-'0'))<<60)
       | ((int64u)((int8u)(Temp[ 1]-'0'))<<56)
       | ((int64u)((int8u)(Temp[ 2]-'0'))<<52)
       | ((int64u)((int8u)(Temp[ 3]-'0'))<<48)
       | ((int64u)((int8u)(Temp[ 4]-'0'))<<44)
       | ((int64u)((int8u)(Temp[ 5]-'0'))<<40)
       | ((int64u)((int8u)(Temp[ 6]-'0'))<<36)
       | ((int64u)((int8u)(Temp[ 7]-'0'))<<32)
       | ((int64u)((int8u)(Temp[ 9]-'0'))<<28)
       | ((int64u)((int8u)(Temp[10]-'0'))<<24)
       | ((int64u)((int8u)(Temp[11]-'0'))<<20)
       | ((int64u)((int8u)(Temp[12]-'0'))<<16)
       | ((int64u)((int8u)(Temp[14]-'0'))<<12)
       | ((int64u)((int8u)(Temp[15]-'0'))<< 8)
       | ((int64u)((int8u)(Temp[16]-'0'))<< 4)
       | ((int64u)((int8u)(Temp[17]-'0'))    );
    I.lo=((int64u)((int8u)(Temp[19]-'0'))<<60)
       | ((int64u)((int8u)(Temp[20]-'0'))<<56)
       | ((int64u)((int8u)(Temp[21]-'0'))<<52)
       | ((int64u)((int8u)(Temp[22]-'0'))<<48)
       | ((int64u)((int8u)(Temp[24]-'0'))<<44)
       | ((int64u)((int8u)(Temp[25]-'0'))<<40)
       | ((int64u)((int8u)(Temp[26]-'0'))<<36)
       | ((int64u)((int8u)(Temp[27]-'0'))<<32)
       | ((int64u)((int8u)(Temp[28]-'0'))<<28)
       | ((int64u)((int8u)(Temp[29]-'0'))<<24)
       | ((int64u)((int8u)(Temp[30]-'0'))<<20)
       | ((int64u)((int8u)(Temp[31]-'0'))<<16)
       | ((int64u)((int8u)(Temp[32]-'0'))<<12)
       | ((int64u)((int8u)(Temp[33]-'0'))<< 8)
       | ((int64u)((int8u)(Temp[34]-'0'))<< 4)
       | ((int64u)((int8u)(Temp[35]-'0'))    );

    return I;
}

//---------------------------------------------------------------------------
int32u Ztring::To_CC4 () const
{
    int32u I;
    I =((int32u)((int8u)at(0))<<24)
     | ((int32u)((int8u)at(1))<<16)
     | ((int32u)((int8u)at(2))<< 8)
     | ((int32u)((int8u)at(3))    );

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int8s Ztring::To_int8s (int8u Radix, ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int I;
    #ifdef __MINGW32__
        I=_ttoi(c_str());
    #elif ( defined(__sun__) && !defined(__sparc__) )
        #ifdef UNICODE
            std::string S=To_UTF8();
            I=atoi(S.c_str());
        #else //UNICODE
            I=atoi(c_str());
        #endif //UNICODE
    #else
        tStringStream SS(*this);
        SS >> setbase(Radix) >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=Error)
    {
        float80 F=To_float80();
        F-=I;
        if (F>=0.5)
            return (int8s)I+1;
    }

    return (int8s)I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int8u Ztring::To_int8u (int8u Radix, ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    unsigned int I;
    #ifdef __MINGW32__
        I=_ttoi64(c_str()); //TODO : I>0x7FFFFFFF - Replaced by i64 version to support, but not good
    #elif ( defined(__sun__) && !defined(__sparc__) )
        #ifdef UNICODE
            std::string S=To_UTF8();
            I=atoi(S.c_str());
        #else //UNICODE
            I=atoi(c_str());
        #endif //UNICODE
    #else
        tStringStream SS(*this);
        SS >> setbase(Radix) >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=std::string::npos)
    {
        float32 F=To_float32();
        F-=I;
        if (F>=0.5)
            return (int8u)I+1;
    }

    return (int8u)I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int16s Ztring::To_int16s (int8u Radix, ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int I;
    #ifdef __MINGW32__
        I=_ttoi(c_str());
    #elif ( defined(__sun__) && !defined(__sparc__) )
        #ifdef UNICODE
            std::string S=To_UTF8();
            I=atoi(S.c_str());
        #else //UNICODE
            I=atoi(c_str());
        #endif //UNICODE
    #else
        tStringStream SS(*this);
        SS >> setbase(Radix) >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=Error)
    {
        float80 F=To_float80();
        F-=I;
        if (F>=0.5)
            return (int16s)I+1;
    }

    return (int16s)I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int16u Ztring::To_int16u (int8u Radix, ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    unsigned int I;
    #ifdef __MINGW32__
        I=_ttoi64(c_str()); //TODO : I>0x7FFFFFFF - Replaced by i64 version to support, but not good
    #elif ( defined(__sun__) && !defined(__sparc__) )
        #ifdef UNICODE
            std::string S=To_UTF8();
            I=atoi(S.c_str());
        #else //UNICODE
            I=atoi(c_str());
        #endif //UNICODE
    #else
        tStringStream SS(*this);
        SS >> setbase(Radix) >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=std::string::npos)
    {
        float32 F=To_float32();
        F-=I;
        if (F>=0.5)
            return (int16u)I+1;
    }

    return (int16u)I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int32s Ztring::To_int32s (int8u Radix, ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int32s I;
    #ifdef __MINGW32__
        I=_ttoi(c_str());
    #elif ( defined(__sun__) && !defined(__sparc__) )
        #ifdef UNICODE
            std::string S=To_UTF8();
            I=atol(S.c_str());
        #else //UNICODE
            I=atol(c_str());
        #endif //UNICODE
    #else
        tStringStream SS(*this);
        SS >> setbase(Radix) >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=Error)
    {
        float80 F=To_float80();
        F-=I;
        if (F>=0.5)
            return I+1;
    }

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int32u Ztring::To_int32u (int8u Radix, ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int32u I;
    #ifdef __MINGW32__
        I=_ttoi64(c_str()); //TODO : I>0x7FFFFFFF - Replaced by i64 version to support, but not good
    #elif ( defined(__sun__) && !defined(__sparc__) )
        #ifdef UNICODE
            std::string S=To_UTF8();
            I=atol(S.c_str());
        #else //UNICODE
            I=atol(c_str());
        #endif //UNICODE
    #else
        tStringStream SS(*this);
        SS >> setbase(Radix) >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=std::string::npos)
    {
        float32 F=To_float32();
        F-=I;
        if (F>=0.5)
            return I+1;
    }

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int64s Ztring::To_int64s (int8u Radix, ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int64s I;
    #ifdef __MINGW32__
        I=_ttoi64(c_str());
    #elif ( defined(__sun__) && !defined(__sparc__) )
        #ifdef UNICODE
             std::string S=To_UTF8();
            I=atoll(S.c_str());
       #else //UNICODE
            I=atoll(c_str());
        #endif //UNICODE
    #else
        tStringStream SS(*this);
        SS >> setbase(Radix) >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=std::string::npos)
    {
        float32 F=To_float32();
        F-=I;
        if (F>0.5)
            return I+1;
    }

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int64u Ztring::To_int64u (int8u Radix, ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int64u I;
    #ifdef __MINGW32__
        I=_ttoi64(c_str()); //TODO : I>0x7FFFFFFFFFFFFFFF
    #elif ( defined(__sun__) && !defined(__sparc__) )
        #ifdef UNICODE
             std::string S=To_UTF8();
            I=atoll(S.c_str());
       #else //UNICODE
            I=atoll(c_str());
        #endif //UNICODE
    #else
        tStringStream SS(*this);
        SS >> setbase(Radix) >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=std::string::npos)
    {
        float32 F=To_float32();
        F-=I;
        if (F>=0.5)
            return I+1;
    }

    return I;
}

//---------------------------------------------------------------------------
int128u Ztring::To_int128u (int8u, ztring_t) const
{
    if (size()!=32)
        return 0;

    Ztring Temp=*this;

    for (size_t Pos=0; Pos<32; Pos++)
    {
        if ((Temp[Pos]< _T('0') || Temp[Pos]> _T('9'))
         && (Temp[Pos]< _T('A') || Temp[Pos]> _T('F'))
         && (Temp[Pos]< _T('a') || Temp[Pos]> _T('f')))
            return 0;
        if (Temp[Pos]>=_T('A') && Temp[Pos]<=_T('F'))
        {
            Temp[Pos]-=_T('A');
            Temp[Pos]+=_T('9')+1;
        }
        if (Temp[Pos]>=_T('a') && Temp[Pos]<=_T('f'))
        {
            Temp[Pos]-=_T('a');
            Temp[Pos]+=_T('9')+1;
        }
    }
    
    int128u I;
    I.hi=((int64u)((int8u)(Temp[ 0]-'0'))<<60)
       | ((int64u)((int8u)(Temp[ 1]-'0'))<<56)
       | ((int64u)((int8u)(Temp[ 2]-'0'))<<52)
       | ((int64u)((int8u)(Temp[ 3]-'0'))<<48)
       | ((int64u)((int8u)(Temp[ 4]-'0'))<<44)
       | ((int64u)((int8u)(Temp[ 5]-'0'))<<40)
       | ((int64u)((int8u)(Temp[ 6]-'0'))<<36)
       | ((int64u)((int8u)(Temp[ 7]-'0'))<<32)
       | ((int64u)((int8u)(Temp[ 8]-'0'))<<28)
       | ((int64u)((int8u)(Temp[ 9]-'0'))<<24)
       | ((int64u)((int8u)(Temp[10]-'0'))<<20)
       | ((int64u)((int8u)(Temp[11]-'0'))<<16)
       | ((int64u)((int8u)(Temp[12]-'0'))<<12)
       | ((int64u)((int8u)(Temp[13]-'0'))<< 8)
       | ((int64u)((int8u)(Temp[14]-'0'))<< 4)
       | ((int64u)((int8u)(Temp[15]-'0'))    );
    I.lo=((int64u)((int8u)(Temp[16]-'0'))<<60)
       | ((int64u)((int8u)(Temp[17]-'0'))<<56)
       | ((int64u)((int8u)(Temp[18]-'0'))<<52)
       | ((int64u)((int8u)(Temp[19]-'0'))<<48)
       | ((int64u)((int8u)(Temp[20]-'0'))<<44)
       | ((int64u)((int8u)(Temp[21]-'0'))<<40)
       | ((int64u)((int8u)(Temp[22]-'0'))<<36)
       | ((int64u)((int8u)(Temp[23]-'0'))<<32)
       | ((int64u)((int8u)(Temp[24]-'0'))<<28)
       | ((int64u)((int8u)(Temp[25]-'0'))<<24)
       | ((int64u)((int8u)(Temp[26]-'0'))<<20)
       | ((int64u)((int8u)(Temp[27]-'0'))<<16)
       | ((int64u)((int8u)(Temp[28]-'0'))<<12)
       | ((int64u)((int8u)(Temp[29]-'0'))<< 8)
       | ((int64u)((int8u)(Temp[30]-'0'))<< 4)
       | ((int64u)((int8u)(Temp[31]-'0'))    );

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToFloat
float32 Ztring::To_float32(ztring_t) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) )
        #ifdef UNICODE
            return (wcstod(c_str(),NULL));
        #else
            return (strtod(c_str(),NULL));
        #endif
    #else
        float32 F;
        tStringStream SS(*this);
        SS >> F;
        if (SS.fail())
            return 0;

        return F;
    #endif
}

//---------------------------------------------------------------------------
//Operateur ToFloat
float64 Ztring::To_float64(ztring_t) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) )
        #ifdef UNICODE
            return (wcstod(c_str(),NULL)); //TODO verify no wcstold
        #else
            return (strtod(c_str(),NULL)); //TODO verify no strtold
        #endif
    #else
        float64 F;
        tStringStream SS(*this);
        SS >> F;
        if (SS.fail())
            return 0;

        return F;
    #endif
}

//---------------------------------------------------------------------------
//Operateur ToFloat
float80 Ztring::To_float80(ztring_t) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    #if defined(__MINGW32__) || ( defined(__sun__) && !defined(__sparc__) )
        #ifdef UNICODE
            return (wcstod(c_str(),NULL)); //TODO verify no wcstold
        #else
            return (strtod(c_str(),NULL)); //TODO verify no strtold
        #endif
    #else
        float80 F;
        tStringStream SS(*this);
        SS >> F;
        if (SS.fail())
            return 0;

        return F;
    #endif
}

//***************************************************************************
// Edition
//***************************************************************************

//---------------------------------------------------------------------------
// Retourne une partie de la chaine
Ztring Ztring::SubString (const tstring &Begin, const tstring &End, size_type Pos, ztring_t Options) const
{
    //Recherche Début
    size_type I_Debut=find(Begin, Pos);
    if (I_Debut==Error)
        return Ztring();
    I_Debut+=Begin.size();

    //gestion fin NULL
    if (End.empty())
        return substr(I_Debut);

    //Recherche Fin
    size_type I_Fin=find(End, I_Debut);
    if (I_Fin==Error)
    {
        if (Options & Ztring_AddLastItem)
            return substr(I_Debut);
        else
            return Ztring();
    }

    return substr(I_Debut, I_Fin-I_Debut);
}

//---------------------------------------------------------------------------
//FindAndReplace
Ztring::size_type Ztring::FindAndReplace (const ZenLib::tstring &ToFind, const ZenLib::tstring &ReplaceBy, size_type Pos, ZenLib::ztring_t Options)
{
    if (ToFind.empty())
        return 0;

    size_type Count=0;
    size_type Middle=Pos;
    while (!(Count==1 && !(Options&Ztring_Recursive)) && (Middle=find(ToFind, Middle))!=npos)
    {
        replace(Middle, ToFind.length(), ReplaceBy);
        Middle += ReplaceBy.length();
        Count++;
    }

    return Count;
}

//---------------------------------------------------------------------------
//test if it is a number
bool Ztring::IsNumber() const
{
    if (empty())
        return false;

    bool OK=true;
    size_t Size=size();
    for (size_t Pos=0; Pos<Size; Pos++)
        if (operator[](Pos)<_T('0') || operator[](Pos)>_T('9'))
        {
            OK=false;
            break;
        }
    return OK;
}

//---------------------------------------------------------------------------
//Mise en minuscules
Ztring &Ztring::MakeLowerCase()
{
    transform(begin(), end(), begin(), (int(*)(int))tolower); //(int(*)(int)) is a patch for unix
    return *this;
}

//---------------------------------------------------------------------------
// Mise en majuscules
Ztring &Ztring::MakeUpperCase()
{
    transform(begin(), end(), begin(), (int(*)(int))toupper); //(int(*)(int)) is a patch for unix
    return *this;
}

//---------------------------------------------------------------------------
// Remove leading whitespaces from a string
Ztring &Ztring::TrimLeft(Char ToTrim)
{
    size_type First=0;
    while (First<size() && operator[](First)==ToTrim)
        First++;
    assign (c_str()+First);
    return *this;
}

//---------------------------------------------------------------------------
// Remove trailing whitespaces from a string
Ztring &Ztring::TrimRight(Char ToTrim)
{
    if (size()==0)
        return *this;

    size_type Last=size()-1;
    while (Last!=(size_type)-1 && operator[](Last)==ToTrim)
        Last--;
    assign (c_str(), Last+1);
    return *this;
}

//---------------------------------------------------------------------------
// Remove leading and trailing whitespaces from a string
Ztring &Ztring::Trim(Char ToTrim)
{
    TrimLeft(ToTrim);
    TrimRight(ToTrim);
    return *this;
}

//---------------------------------------------------------------------------
// Quotes a string
Ztring &Ztring::Quote(Char ToTrim)
{
    assign(tstring(1, ToTrim)+c_str()+ToTrim);
    return *this;
}

//***************************************************************************
// Information
//***************************************************************************

//---------------------------------------------------------------------------
//Count
Ztring::size_type Ztring::Count (const Ztring &ToCount, ztring_t) const
{
    size_type Count=0;
    for (size_type Pos=0; Pos<=size(); Pos++)
        if (find(ToCount, Pos)!=npos)
        {
            Count++;
            Pos+=ToCount.size()-1; //-1 because the loop will add 1
        }
    return Count;
}

//---------------------------------------------------------------------------
//Compare
bool Ztring::Compare (const Ztring &ToCompare, const Ztring &Comparator, ztring_t Options) const
{
    //Integers management
    if (IsNumber() && ToCompare.IsNumber())
    {
        int64s Left=To_int64s();
        int64s Right=ToCompare.To_int64s();
        if (Comparator==_T("==")) return (Left==Right);
        if (Comparator==_T("<"))  return (Left< Right);
        if (Comparator==_T("<=")) return (Left<=Right);
        if (Comparator==_T(">=")) return (Left>=Right);
        if (Comparator==_T(">"))  return (Left> Right);
        if (Comparator==_T("!=")) return (Left!=Right);
        if (Comparator==_T("<>")) return (Left!=Right);
        return false;
    }

    //Case sensitive option
    if (!(Options & Ztring_CaseSensitive))
    {
        //Need to copy strings and make it lowercase
        Ztring Left (c_str());
        Ztring Right (ToCompare.c_str());
        Left.MakeLowerCase();
        Right.MakeLowerCase();

        //string comparasion
        if (Comparator==_T("==")) return (Left==Right);
        if (Comparator==_T("IN")) {if (Left.find(Right)!=string::npos) return true; else return false;}
        if (Comparator==_T("<"))  return (Left< Right);
        if (Comparator==_T("<=")) return (Left<=Right);
        if (Comparator==_T(">=")) return (Left>=Right);
        if (Comparator==_T(">"))  return (Left> Right);
        if (Comparator==_T("!=")) return (Left!=Right);
        if (Comparator==_T("<>")) return (Left!=Right);
        return false;
    }
    else
    {
        //string comparasion
        if (Comparator==_T("==")) return (*this==ToCompare);
        if (Comparator==_T("IN")) {if (this->find(ToCompare)!=string::npos) return true; else return false;}
        if (Comparator==_T("<"))  return (*this< ToCompare);
        if (Comparator==_T("<=")) return (*this<=ToCompare);
        if (Comparator==_T(">=")) return (*this>=ToCompare);
        if (Comparator==_T(">"))  return (*this> ToCompare);
        if (Comparator==_T("!=")) return (*this!=ToCompare);
        if (Comparator==_T("<>")) return (*this!=ToCompare);
        return false;
    }
}

} //namespace

