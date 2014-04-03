/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// More methods for std::(w)string
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
#include <algorithm>
#include <iomanip>
#include <cmath>
#include "ZenLib/OS_Utils.h"
#include "ZenLib/File.h"
using namespace std;
//---------------------------------------------------------------------------

namespace ZenLib
{

int16u Ztring_ISO_8859_2[96]=
{
    0x00A0,
    0x0104,
    0x02D8,
    0x0141,
    0x00A4,
    0x013D,
    0x015A,
    0x00A7,
    0x00A8,
    0x0160,
    0x015E,
    0x0164,
    0x0179,
    0x00AD,
    0x017D,
    0x017B,
    0x00B0,
    0x0105,
    0x02DB,
    0x0142,
    0x00B4,
    0x013E,
    0x015B,
    0x02C7,
    0x00B8,
    0x0161,
    0x015F,
    0x0165,
    0x017A,
    0x02DD,
    0x017E,
    0x017C,
    0x0154,
    0x00C1,
    0x00C2,
    0x0102,
    0x00C4,
    0x0139,
    0x0106,
    0x00C7,
    0x010C,
    0x00C9,
    0x0118,
    0x00CB,
    0x011A,
    0x00CD,
    0x00CE,
    0x010E,
    0x0110,
    0x0143,
    0x0147,
    0x00D3,
    0x00D4,
    0x0150,
    0x00D6,
    0x00D7,
    0x0158,
    0x016E,
    0x00DA,
    0x0170,
    0x00DC,
    0x00DD,
    0x0162,
    0x00DF,
    0x0155,
    0x00E1,
    0x00E2,
    0x0103,
    0x00E4,
    0x013A,
    0x0107,
    0x00E7,
    0x010D,
    0x00E9,
    0x0119,
    0x00EB,
    0x011B,
    0x00ED,
    0x00EE,
    0x010F,
    0x0111,
    0x0144,
    0x0148,
    0x00F3,
    0x00F4,
    0x0151,
    0x00F6,
    0x00F7,
    0x0159,
    0x016F,
    0x00FA,
    0x0171,
    0x00FC,
    0x00FD,
    0x0163,
    0x02D9,
};

//---------------------------------------------------------------------------
Ztring EmptyZtring;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(STREAM_MISSING)
    #if defined (_UNICODE)
        #define _tnprintf snwprintf
    #else
        #define _tnprintf snprintf
    #endif
#else
    typedef basic_stringstream<Char>  tStringStream;
    typedef basic_istringstream<Char> tiStringStream;
    typedef basic_ostringstream<Char> toStringStream;
#endif
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
    {
        delete[] Buffer;
        return false;
    }
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
    Temp[Length]=__T('\0');

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
            // Don't use MultiByteToWideChar(), some characters are not well decoded
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
                    {
                        clear();
                        return *this; //Bad character
                    }
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
                    {
                        clear();
                        return *this; //Bad character
                    }
                }
                //4 bytes
                else if ((*Z&0xF8)==0xF0)
                {
                    if ((*(Z+1)&0xC0)==0x80 && (*(Z+2)&0xC0)==0x80 && (*(Z+3)&0xC0)==0x80)
                    {
                        operator += ((((wchar_t)(*Z&0x0F))<<18)|((*(Z+1)&0x3F)<<12)||((*(Z+2)&0x3F)<<6)|(*(Z+3)&0x3F));
                        Z+=4;
                    }
                    else
                    {
                        clear();
                        return *this; //Bad character
                    }
                }
                else
                {
                    clear();
                    return *this; //Bad character
                }
            }
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
            while (SW[Pos]!=__T('\0'))
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
    else
        Length&=(size_t)-2; //odd number

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
    else
        Length&=(size_t)-2; //odd number

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
        if (find(__T('\0'))!=std::string::npos)
            resize(find(__T('\0')));
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
        if (find(__T('\0')) != std::string::npos)
            resize(find(__T('\0')));
    #endif
    return *this;
}

Ztring& Ztring::From_ISO_8859_2(const char* S)
{
    size_t Length = strlen(S);
    wchar_t* Temp = new wchar_t[Length +1];

    for (size_t Pos=0; Pos<Length+1; Pos++)
    {
        if ((int8u)S[Pos]>=0xA0)
            Temp[Pos]=(wchar_t)Ztring_ISO_8859_2[((int8u)S[Pos])-0xA0];
        else
            Temp[Pos]=(wchar_t)((int8u)S[Pos]);
    }

    From_Unicode(Temp);
    delete[] Temp;
    return *this;
}

Ztring& Ztring::From_ISO_8859_2(const char* S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length==Error)
        Length=strlen(S+Start);
    #ifdef _UNICODE
        char* Temp = new char[Length+1];
        strncpy(Temp, S +Start, Length);
        Temp[Length] = '\0';
        From_ISO_8859_2(Temp);
        delete[] Temp;
    #else
        assign(S +Start, Length);
        if (find(__T('\0')) != std::string::npos)
            resize(find(__T('\0')));
    #endif
    return *this;
}

Ztring& Ztring::From_GUID (const int128u S)
{
    Ztring S1;
    S1.From_CC1((int8u) ((S.hi&0x000000FF00000000LL)>>32)); append(S1);
    S1.From_CC1((int8u) ((S.hi&0x0000FF0000000000LL)>>40)); append(S1);
    S1.From_CC1((int8u) ((S.hi&0x00FF000000000000LL)>>48)); append(S1);
    S1.From_CC1((int8u) ((S.hi&0xFF00000000000000LL)>>56)); append(S1); append(__T("-"));
    S1.From_CC1((int8u) ((S.hi&0x0000000000FF0000LL)>>16)); append(S1);
    S1.From_CC1((int8u) ((S.hi&0x00000000FF000000LL)>>24)); append(S1); append(__T("-"));
    S1.From_CC1((int8u) ( S.hi&0x00000000000000FFLL     )); append(S1);
    S1.From_CC1((int8u) ((S.hi&0x000000000000FF00LL)>> 8)); append(S1); append(__T("-"));
    S1.From_CC2((int16u)((S.lo&0xFFFF000000000000LL)>>48)); append(S1); append(__T("-"));
    S1.From_CC2((int16u)((S.lo&0x0000FFFF00000000LL)>>32)); append(S1);
    S1.From_CC2((int16u)((S.lo&0x00000000FFFF0000LL)>>16)); append(S1);
    S1.From_CC2((int16u)( S.lo&0x000000000000FFFFLL     )); append(S1);

    return *this;
}

Ztring& Ztring::From_UUID (const int128u S)
{
    Ztring S1;
    S1.From_CC2((int16u)((S.hi&0xFFFF000000000000LL)>>48)); assign(S1);
    S1.From_CC2((int16u)((S.hi&0x0000FFFF00000000LL)>>32)); append(S1); append(__T("-"));
    S1.From_CC2((int16u)((S.hi&0x00000000FFFF0000LL)>>16)); append(S1); append(__T("-"));
    S1.From_CC2((int16u)( S.hi&0x000000000000FFFFLL     )); append(S1); append(__T("-"));
    S1.From_CC2((int16u)((S.lo&0xFFFF000000000000LL)>>48)); append(S1); append(__T("-"));
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
        assign(__T("(empty)"));

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
        assign(__T("(empty)"));

    return *this;
}

Ztring& Ztring::From_CC2 (const int16u S)
{
    clear();
    Ztring Pos1; Pos1.From_Number(S, 16);
    resize(4-Pos1.size(), __T('0'));
    append(Pos1);
    MakeUpperCase();

    return *this;
}

Ztring& Ztring::From_CC1 (const int8u S)
{
    clear();
    Ztring Pos1; Pos1.From_Number(S, 16);
    resize(2-Pos1.size(), __T('0'));
    append(Pos1);
    MakeUpperCase();

    return *this;
}

Ztring& Ztring::From_Number (const int8s I, int8u Radix)
{
    #if defined(STREAM_MISSING)
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        #ifdef __MINGW32__
            _itot (I, C1, Radix);
        #else
            _tnprintf(C1, 32, Radix==10?__T("%d"):(Radix==16?__T("%x"):(Radix==8?__T("%o"):__T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream Stream;
        #ifdef UNICODE
            Stream << setbase(Radix) << I;
        #else //UNICODE
            Stream << setbase(Radix) << (size_t)I; //On linux (at least), (un)signed char is detected as a char
        #endif //UNICODE
        assign(Stream.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int8u I, int8u Radix)
{
    #if defined(STREAM_MISSING)
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        #ifdef __MINGW32__
            _ultot (I, C1, Radix);
        #else
            _tnprintf(C1, 32, Radix==10?__T("%d"):(Radix==16?__T("%x"):(Radix==8?__T("%o"):__T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        if (Radix==2)
        {
            clear();
            for (int8u Pos=0; Pos<8; Pos++)
            {
                if (I<(((int8u)1)<<Pos))
                    break;
                insert(0, 1, (I&(((int8u)1)<<Pos))?__T('1'):__T('0'));
            }
        }
        else
        {
            toStringStream Stream;
            #ifdef UNICODE
                Stream << setbase(Radix) << I;
            #else //UNICODE
                Stream << setbase(Radix) << (size_t)I; //On linux (at least), (un)signed char is detected as a char
            #endif //UNICODE
            assign(Stream.str());
        }
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int16s I, int8u Radix)
{
    #if defined(STREAM_MISSING)
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        #ifdef __MINGW32__
            _itot (I, C1, Radix);
        #else
            _tnprintf(C1, 32, Radix==10?__T("%d"):(Radix==16?__T("%x"):(Radix==8?__T("%o"):__T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream Stream;
        Stream << setbase(Radix) << I;
        assign(Stream.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int16u I, int8u Radix)
{
    #if defined(STREAM_MISSING)
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        #ifdef __MINGW32__
            _ultot (I, C1, Radix);
        #else
            _tnprintf(C1, 32, Radix==10?__T("%u"):(Radix==16?__T("%x"):(Radix==8?__T("%o"):__T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        if (Radix==2)
        {
            clear();
            for (int8u Pos=0; Pos<16; Pos++)
            {
                if (I<(((int16u)1)<<Pos))
                    break;
                insert(0, 1, (I&(((int16u)1)<<Pos))?__T('1'):__T('0'));
            }
        }
        else
        {
            toStringStream Stream;
            Stream << setbase(Radix) << I;
            assign(Stream.str());
        }
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int32s I, int8u Radix)
{
    #if defined(STREAM_MISSING)
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        #ifdef __MINGW32__
            _itot (I, C1, Radix);
        #else
            _tnprintf(C1, 32, Radix==10?__T("%ld"):(Radix==16?__T("%lx"):(Radix==8?__T("%lo"):__T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream Stream;
        Stream << setbase(Radix) << I;
        assign(Stream.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int32u I, int8u Radix)
{
    #if defined(STREAM_MISSING)
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        #ifdef __MINGW32__
            _ultot (I, C1, Radix);
        #else
            _tnprintf(C1, 32, Radix==10?__T("%lu"):(Radix==16?__T("%lx"):(Radix==8?__T("%lo"):__T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        if (Radix==2)
        {
            clear();
            for (int8u Pos=0; Pos<32; Pos++)
            {
                if (I<(((int32u)1)<<Pos))
                    break;
                insert(0, 1, (I&(((int32u)1)<<Pos))?__T('1'):__T('0'));
            }
        }
        else
        {
            toStringStream Stream;
            Stream << setbase(Radix) << I;
            assign(Stream.str());
        }
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int64s I, int8u Radix)
{
    #if defined(STREAM_MISSING)
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[65];
        #ifdef __MINGW32__
            _i64tot (I, C1, Radix);
        #else
            _tnprintf(C1, 64, Radix==10?__T("%lld"):(Radix==16?__T("%llx"):(Radix==8?__T("%llo"):__T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream Stream;
        Stream << setbase(Radix) << I;
        assign(Stream.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int64u I, int8u Radix)
{
    #if defined(STREAM_MISSING)
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[65]; C1[0] = 0;
        #ifdef __MINGW32__
            _ui64tot (I, C1, Radix);
        #else
            _tnprintf(C1, 64, Radix==10?__T("%llu"):(Radix==16?__T("%llx"):(Radix==8?__T("%llo"):__T(""))), I);
        #endif
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        if (Radix==2)
        {
            clear();
            for (int8u Pos=0; Pos<32; Pos++)
            {
                if (I<(((int64u)1)<<Pos))
                    break;
                insert(0, 1, (I&(((int64u)1)<<Pos))?__T('1'):__T('0'));
            }
        }
        else
        {
            toStringStream Stream;
            Stream << setbase(Radix) << I;
            assign(Stream.str());
        }
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
    #if defined(STREAM_MISSING)
        Char C1[100];
        _tnprintf (C1, 99, (Ztring(__T("%."))+Ztring::ToZtring(Precision)+__T("f")).c_str(), F);
        assign(C1);
    #else
        toStringStream Stream;
        Stream << setprecision(Precision) << fixed << F;
        assign(Stream.str());
        #if defined(__BORLANDC__)
            FindAndReplace(__T(","), __T(".")); //Borland C++ Builder 2010+Windows Seven put a comma for istringstream, but does not support comma for ostringstream
        #endif
    #endif

    if ((Options & Ztring_NoZero && size()>0) && find(__T('.'))!=string::npos)
    {
        while (size()>0 && ((*this)[size()-1]==__T('0')))
            resize(size()-1);
        if (size()>0 && (*this)[size()-1]==__T('.'))
            resize(size()-1);
    }

    return *this;
}

Ztring& Ztring::From_Number (const float64 F, int8u Precision, ztring_t Options)
{
    #if defined(STREAM_MISSING)
        Char C1[100];
        _tnprintf (C1, 99, (Ztring(__T("%."))+Ztring::ToZtring(Precision)+__T("f")).c_str(), F);
        assign(C1);
    #else
        toStringStream Stream;
        Stream << setprecision(Precision) << fixed << F;
        assign(Stream.str());
        #if defined(__BORLANDC__)
            FindAndReplace(__T(","), __T(".")); //Borland C++ Builder 2010+Windows Seven put a comma for istringstream, but does not support comma for ostringstream
        #endif
    #endif

    if ((Options & Ztring_NoZero && size()>0) && find(__T('.'))!=string::npos)
    {
        while (size()>0 && ((*this)[size()-1]==__T('0')))
            resize(size()-1);
        if (size()>0 && (*this)[size()-1]==__T('.'))
            resize(size()-1);
    }

    return *this;
}

Ztring& Ztring::From_Number (const float80 F, int8u Precision, ztring_t Options)
{
    #if defined(STREAM_MISSING)
        Char C1[100];
        _tnprintf (C1, 99, (Ztring(__T("%."))+Ztring::ToZtring(Precision)+__T("f")).c_str(), F);
        assign(C1);
    #else
        toStringStream Stream;
        Stream << setprecision(Precision) << fixed << F;
        assign(Stream.str());
        #if defined(__BORLANDC__)
            FindAndReplace(__T(","), __T(".")); //Borland C++ Builder 2010+Windows Seven put a comma for istringstream, but does not support comma for ostringstream
        #endif
    #endif

    if ((Options & Ztring_NoZero && size()>0) && find(__T('.'))!=string::npos)
    {
        while (size()>0 && ((*this)[size()-1]==__T('0')))
            resize(size()-1);
        if (size()>0 && (*this)[size()-1]==__T('.'))
            resize(size()-1);
    }

    return *this;
}

#ifdef SIZE_T_IS_LONG
Ztring& Ztring::From_Number (const size_t I, int8u Radix)
{
    #if defined(STREAM_MISSING)
        Char C1[100];
        _tnprintf(C1, 64, Radix==10?__T("%zu"):(Radix==16?__T("%zx"):(Radix==8?__T("%zo"):__T(""))), I);
        assign(C1);
    #else
        toStringStream Stream;
        Stream << setbase(Radix) << I;
        assign(Stream.str());
        #if defined(__BORLANDC__)
            FindAndReplace(__T(","), __T(".")); //Borland C++ Builder 2010+Windows Seven put a comma for istringstream, but does not support comma for ostringstream
        #endif
    #endif
    MakeUpperCase();
    return *this;
}
#endif //SIZE_T_IS_LONG

Ztring& Ztring::From_BCD     (const int8u I)
{
    #if defined(STREAM_MISSING)
        clear();
        append(1, __T('0')+I/0x10);
        append(1, __T('0')+I%0x10);
    #else
        toStringStream Stream;
        Stream << I/0x10;
        Stream << I%0x10;
        assign(Stream.str());
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
    int64u Stream=Value/1000      -((HH*60+MM)*60);
    int64u MS=Value           -((HH*60+MM)*60+Stream)*1000;
    Ztring DateT;
    Ztring Date;
    DateT.From_Number(HH); if (DateT.size()<2){DateT=Ztring(__T("0"))+DateT;}
    Date+=DateT;
    Date+=__T(":");
    DateT.From_Number(MM); if (DateT.size()<2){DateT=Ztring(__T("0"))+DateT;}
    Date+=DateT;
    Date+=__T(":");
    DateT.From_Number(Stream); if (DateT.size()<2){DateT=Ztring(__T("0"))+DateT;}
    Date+=DateT;
    Date+=__T(".");
    DateT.From_Number(MS); if (DateT.size()<2){DateT=Ztring(__T("00"))+DateT;} else if (DateT.size()<3){DateT=Ztring(__T("0"))+DateT;}
    Date+=DateT;
    if (Negative)
    {
        assign(__T("-"));
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
    int64u Stream=Value/1000      -((HH*60+MM)*60);
    int64u MS=Value           -((HH*60+MM)*60+Stream)*1000;
    Ztring DateT;
    Ztring Date;
    DateT.From_Number(HH); if (DateT.size()<2){DateT=Ztring(__T("0"))+DateT;}
    Date+=DateT;
    Date+=__T(":");
    DateT.From_Number(MM); if (DateT.size()<2){DateT=Ztring(__T("0"))+DateT;}
    Date+=DateT;
    Date+=__T(":");
    DateT.From_Number(Stream); if (DateT.size()<2){DateT=Ztring(__T("0"))+DateT;}
    Date+=DateT;
    Date+=__T(".");
    DateT.From_Number(MS); if (DateT.size()<2){DateT=Ztring(__T("00"))+DateT;} else if (DateT.size()<3){DateT=Ztring(__T("0"))+DateT;}
    Date+=DateT;
    assign (Date.c_str());
    return *this;
}

Ztring& Ztring::Date_From_Milliseconds_1601 (const int64u Value)
{
    if (Value>=11644473600000LL) //Values <1970 are not supported
    {
        Date_From_Seconds_1970((int32u)((Value-11644473600000LL)/1000));
        append(__T("."));
        Ztring Milliseconds; Milliseconds.From_Number(Value%1000);
        while (Milliseconds.size()<3)
            Milliseconds+=__T('0');
        append(Milliseconds);
    }
    else
        clear(); //Not supported

    return *this;
}

Ztring& Ztring::Date_From_Seconds_1601 (const int64u Value)
{
    return Date_From_Seconds_1970(((int64s)Value)-11644473600LL);
}

Ztring& Ztring::Date_From_Seconds_1900 (const int32u Value)
{
    if (Value>2208988800)
        return Date_From_Seconds_1970(((int64s)Value)-2208988800);
    else
        return Date_From_Seconds_1970(((int64s)Value)+0x100000000LL-2208988800); //Value is considering to loop e.g. NTP value
}

Ztring& Ztring::Date_From_Seconds_1900 (const int64s Value)
{
    return Date_From_Seconds_1970(Value-2208988800);
}

Ztring& Ztring::Date_From_Seconds_1904 (const int32u Value)
{
    return Date_From_Seconds_1970(((int64s)Value)-2082844800);
}

Ztring& Ztring::Date_From_Seconds_1904 (const int64u Value)
{
    return Date_From_Seconds_1970(((int64s)Value)-2082844800);
}

Ztring& Ztring::Date_From_Seconds_1904 (const int64s Value)
{
    return Date_From_Seconds_1970(Value-2082844800);
}

Ztring& Ztring::Date_From_Seconds_1970 (const int32u Value)
{
    return Date_From_Seconds_1970((int64s)Value);
}

Ztring& Ztring::Date_From_Seconds_1970 (const int32s Value)
{
    return Date_From_Seconds_1970((int64s)Value);
}

Ztring& Ztring::Date_From_Seconds_1970 (const int64s Value)
{
    time_t Time=(time_t)Value;
    struct tm *Gmt=gmtime(&Time);
    if (!Gmt)
    {
        clear();
        return *this;
    }
    Ztring DateT;
    Ztring Date=__T("UTC ");
    Date+=Ztring::ToZtring((Gmt->tm_year+1900));
    Date+=__T("-");
    DateT.From_Number(Gmt->tm_mon+1); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Gmt->tm_mon+1);}
    Date+=DateT;
    Date+=__T("-");
    DateT.From_Number(Gmt->tm_mday); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Gmt->tm_mday);}
    Date+=DateT;
    Date+=__T(" ");
    DateT.From_Number(Gmt->tm_hour); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Gmt->tm_hour);}
    Date+=DateT;
    Date+=__T(":");
    DateT=Ztring::ToZtring(Gmt->tm_min); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Gmt->tm_min);}
    Date+=DateT;
    Date+=__T(":");
    DateT.From_Number(Gmt->tm_sec); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Gmt->tm_sec);}
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
    Date+=__T("-");
    DateT.From_Number(Gmt->tm_mon+1); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Gmt->tm_mon+1);}
    Date+=DateT;
    Date+=__T("-");
    DateT.From_Number(Gmt->tm_mday); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Gmt->tm_mday);}
    Date+=DateT;
    Date+=__T(" ");
    DateT.From_Number(Gmt->tm_hour); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Gmt->tm_hour);}
    Date+=DateT;
    Date+=__T(":");
    DateT=Ztring::ToZtring(Gmt->tm_min); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Gmt->tm_min);}
    Date+=DateT;
    Date+=__T(":");
    DateT.From_Number(Gmt->tm_sec); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Gmt->tm_sec);}
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
        Ztring ToReturn=__T("UTC ");
        wxDateTime Date;
        Ztring DateS;
        DateS.From_Local(Value, Value_Size).c_str();
        if (!DateS.empty() && DateS[DateS.size()-1]==__T('\n'))
            DateS.resize(DateS.size()-1);

        //Some strange formating : exactly 24 bytes (or 25 with 0x0A at the end) and Year is at the end
        if (DateS.size()==24 && DateS[23]>=__T('0') && DateS[23]<=__T('9') && DateS[21]>=__T('0') && DateS[21]<=__T('9') && DateS[19]==__T(' '))
            Date.ParseFormat(DateS.c_str(), __T("%a %b %d %H:%M:%S %Y"));
        //ISO date
        else if (DateS.size()==10 && (DateS[4]<__T('0') || DateS[4]>__T('9')) && (DateS[7]<__T('0') || DateS[7]>__T('9')))
        {
            DateS[4]=__T('-');
            DateS[7]=__T('-');
            ToReturn+=DateS;
        }
        //Default
        else
            Date.ParseDateTime(DateS.c_str());

        if (ToReturn.size()<5)
        {
            ToReturn+=Date.FormatISODate();
            ToReturn+=__T(" ");
            ToReturn+=Date.FormatISOTime();
        }
        else if (ToReturn.size()<5)
            ToReturn+=DateS;

        assign (ToReturn.c_str());
    #else //ZENLIB_USEWX
        Ztring DateS; DateS.From_Local(Value, 0, Value_Size);
        //Unix style formating : exactly 24 bytes (or 25 with 0x0A at the end) and Year is at the end
        if ((DateS.size()==24 || (DateS.size()==25 && DateS[24]==__T('\n'))) && DateS[23]>=__T('0') && DateS[23]<=__T('9') && DateS[21]>=__T('0') && DateS[21]<=__T('9') && DateS[19]==__T(' '))
        {
            clear();
            append(1, DateS[20]);
            append(1, DateS[21]);
            append(1, DateS[22]);
            append(1, DateS[23]);
            append(1, __T('-'));
                 if (DateS[4]==__T('J') && DateS[5]==__T('a') && DateS[6]==__T('n') && DateS[7]==__T(' '))
            {
                append(1, __T('0'));
                append(1, __T('1'));
            }
            else if (DateS[4]==__T('F') && DateS[5]==__T('e') && DateS[6]==__T('b') && DateS[7]==__T(' '))
            {
                append(1, __T('0'));
                append(1, __T('2'));
            }
            else if (DateS[4]==__T('M') && DateS[5]==__T('a') && DateS[6]==__T('r') && DateS[7]==__T(' '))
            {
                append(1, __T('0'));
                append(1, __T('3'));
            }
            else if (DateS[4]==__T('A') && DateS[5]==__T('p') && DateS[6]==__T('r') && DateS[7]==__T(' '))
            {
                append(1, __T('0'));
                append(1, __T('4'));
            }
            else if (DateS[4]==__T('M') && DateS[5]==__T('a') && DateS[6]==__T('y') && DateS[7]==__T(' '))
            {
                append(1, __T('0'));
                append(1, __T('5'));
            }
            else if (DateS[4]==__T('J') && DateS[5]==__T('u') && DateS[6]==__T('n') && DateS[7]==__T(' '))
            {
                append(1, __T('0'));
                append(1, __T('6'));
            }
            else if (DateS[4]==__T('J') && DateS[5]==__T('u') && DateS[6]==__T('l') && DateS[7]==__T(' '))
            {
                append(1, __T('0'));
                append(1, __T('7'));
            }
            else if (DateS[4]==__T('A') && DateS[5]==__T('u') && DateS[6]==__T('g') && DateS[7]==__T(' '))
            {
                append(1, __T('0'));
                append(1, __T('8'));
            }
            else if (DateS[4]==__T('S') && DateS[5]==__T('e') && DateS[6]==__T('p') && DateS[7]==__T(' '))
            {
                append(1, __T('0'));
                append(1, __T('9'));
            }
            else if (DateS[4]==__T('O') && DateS[5]==__T('c') && DateS[6]==__T('t') && DateS[7]==__T(' '))
            {
                append(1, __T('1'));
                append(1, __T('0'));
            }
            else if (DateS[4]==__T('N') && DateS[5]==__T('o') && DateS[6]==__T('v') && DateS[7]==__T(' '))
            {
                append(1, __T('1'));
                append(1, __T('1'));
            }
            else if (DateS[4]==__T('D') && DateS[5]==__T('e') && DateS[6]==__T('c') && DateS[7]==__T(' '))
            {
                append(1, __T('1'));
                append(1, __T('2'));
            }
            else
            {
                assign(DateS);
                return *this;
            }
            append(1, __T('-'));
            append(1, DateS[8]);
            append(1, DateS[9]);
            append(1, __T(' '));
            append(1, DateS[11]);
            append(1, DateS[12]);
            append(1, __T(':'));
            append(1, DateS[14]);
            append(1, DateS[15]);
            append(1, __T(':'));
            append(1, DateS[17]);
            append(1, DateS[18]);
        }
        else if (DateS.size()==20 && DateS[4]==__T('-') && DateS[7]==__T('-') && DateS[10]==__T('T') && DateS[13]==__T(':') && DateS[16]==__T(':') && DateS[19]==__T('Z'))
        {
            DateS.resize(19);
            DateS[10]=__T(' ');
            assign(__T("UTC "));
            append(DateS);
        }
        else if (DateS.size()==23 && DateS[4]==__T('-') && DateS[7]==__T('-') && DateS[10]==__T(' ') && DateS[14]==__T(' ') && DateS[17]==__T(':') && DateS[20]==__T(':'))
        {
            DateS.erase(10, 4);
            //assign(__T("UTC ")); //Is not UTC
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
    Ztring Date=__T("UTC ");
    DateT.From_Number(Year); if (DateT.size()<2){DateT=Ztring(__T("200"))+Ztring::ToZtring(Year);}; if (DateT.size()<3){DateT=Ztring(__T("20"))+Ztring::ToZtring(Year);}
    Date+=DateT;
    Date+=__T("-");
    DateT.From_Number(Month); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Month);}
    Date+=DateT;
    Date+=__T("-");
    DateT.From_Number(Day); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Day);}
    Date+=DateT;
    Date+=__T(" ");
    DateT.From_Number(Hour); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Hour);}
    Date+=DateT;
    Date+=__T(":");
    DateT=Ztring::ToZtring(Minute); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Minute);}
    Date+=DateT;
    Date+=__T(":");
    DateT.From_Number(Second); if (DateT.size()<2){DateT=Ztring(__T("0"))+Ztring::ToZtring(Second);}
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
        //Correction thanks to Andrew Jang
        // Don't use WideCharToMultiByte(), some characters are not well converted
        std::string ToReturn;
        ToReturn.reserve(size()); // more efficient

        const wchar_t* Z=c_str();

        while (*Z)
        {
            int32u wc; // must be unsigned.

            #if defined(_MSC_VER)
                #pragma warning(push)
                #pragma warning(disable:4127)
            #endif //defined(_MSC_VER)
            if (sizeof(wchar_t) == 2)
            #if defined(_MSC_VER)
                #pragma warning(pop)
            #endif //defined(_MSC_VER)
                wc = (int16u) *Z; // avoid a cast problem if wchar_t is signed.
            else
                wc = *Z;

            int count;

            // refer to http://en.wikipedia.org/wiki/UTF-8#Description

            if (wc < 0x80)
                count = 1;
            else if (wc < 0x800)
                count = 2;
            else if (wc < 0x10000)
                count = 3;
            else if (wc < 0x200000)
                count = 4;
            else if (wc < 0x4000000)
                count = 5;
            else if (wc <= 0x7fffffff)
                count = 6;
            else
                break;  // bad character

            int64u utfbuf = 0; // 8 bytes
            char* utf8chars = (char*) &utfbuf;

            switch (count)
            {
            case 6:
                utf8chars[5] = 0x80 | (wc & 0x3f);
                wc = (wc >> 6) | 0x4000000;
            case 5:
                utf8chars[4] = 0x80 | (wc & 0x3f);
                wc = (wc >> 6) | 0x200000;
            case 4:
                utf8chars[3] = 0x80 | (wc & 0x3f);
                wc = (wc >> 6) | 0x10000;
            case 3:
                utf8chars[2] = 0x80 | (wc & 0x3f);
                wc = (wc >> 6) | 0x800;
            case 2:
                utf8chars[1] = 0x80 | (wc & 0x3f);
                wc = (wc >> 6) | 0xc0;
            case 1:
                utf8chars[0] = (char) wc;
            }

            ToReturn += utf8chars;

            ++Z;
        }

        return ToReturn;
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
        if ((Temp[Pos]< __T('0') || Temp[Pos]> __T('9'))
         && (Temp[Pos]< __T('A') || Temp[Pos]> __T('F'))
         && (Temp[Pos]< __T('a') || Temp[Pos]> __T('f')))
            return 0;
        if (Temp[Pos]>=__T('A') && Temp[Pos]<=__T('F'))
        {
            Temp[Pos]-=__T('A');
            Temp[Pos]+=__T('9')+1;
        }
        if (Temp[Pos]>=__T('a') && Temp[Pos]<=__T('f'))
        {
            Temp[Pos]-=__T('a');
            Temp[Pos]+=__T('9')+1;
        }

        switch(Pos)
        {
            case  7 :
            case 12 :
            case 17 :
            case 22 :
                        if (at(Pos+1)!=__T('-'))
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
    #if defined(STREAM_MISSING)
        #ifdef __MINGW32__
            I=_ttoi(c_str());
        #elif defined(UNICODE)
            std::string S=To_UTF8();
            I=atoi(S.c_str());
        #else //UNICODE
            I=atoi(c_str());
        #endif //UNICODE
    #else
        tStringStream Stream(*this);
        Stream >> setbase(Radix) >> I;
        if (Stream.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(__T("."))!=Error)
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
    #if defined(STREAM_MISSING)
        #ifdef __MINGW32__
            I=_ttoi64(c_str()); //TODO : I>0x7FFFFFFF - Replaced by i64 version to support, but not good
        #elif defined(UNICODE)
            std::string S=To_UTF8();
            I=atoi(S.c_str());
        #else //defined(UNICODE)
            I=atoi(c_str());
        #endif //defined(UNICODE)
    #else
        tStringStream Stream(*this);
        Stream >> setbase(Radix) >> I;
        if (Stream.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(__T("."))!=std::string::npos)
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
    #if defined(STREAM_MISSING)
        #ifdef __MINGW32__
            I=_ttoi(c_str());
        #elif defined(UNICODE)
            std::string S=To_UTF8();
            I=atoi(S.c_str());
        #else //defined(UNICODE)
            I=atoi(c_str());
        #endif //defined(UNICODE)
    #else
        tStringStream Stream(*this);
        Stream >> setbase(Radix) >> I;
        if (Stream.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(__T("."))!=Error)
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
    #if defined(STREAM_MISSING)
        #ifdef __MINGW32__
            I=_ttoi64(c_str()); //TODO : I>0x7FFFFFFF - Replaced by i64 version to support, but not good
        #elif defined(UNICODE)
            std::string S=To_UTF8();
            I=atoi(S.c_str());
        #else //defined(UNICODE)
            I=atoi(c_str());
        #endif //defined(UNICODE)
    #else
        tStringStream Stream(*this);
        Stream >> setbase(Radix) >> I;
        if (Stream.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(__T("."))!=std::string::npos)
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
    #if defined(STREAM_MISSING)
        #ifdef __MINGW32__
            I=_ttoi(c_str());
        #elif defined(UNICODE)
            std::string S=To_UTF8();
            I=atol(S.c_str());
        #else //defined(UNICODE)
            I=atol(c_str());
        #endif //defined(UNICODE)
    #else
        tStringStream Stream(*this);
        Stream >> setbase(Radix) >> I;
        if (Stream.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(__T("."))!=Error)
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
    #if defined(STREAM_MISSING)
        #ifdef __MINGW32__
            I=_ttoi64(c_str()); //TODO : I>0x7FFFFFFF - Replaced by i64 version to support, but not good
        #elif defined(UNICODE)
            std::string S=To_UTF8();
            I=atol(S.c_str());
        #else //defined(UNICODE)
            I=atol(c_str());
        #endif //defined(UNICODE)
    #else
        tStringStream Stream(*this);
        Stream >> setbase(Radix) >> I;
        if (Stream.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(__T("."))!=std::string::npos)
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
    #if defined(STREAM_MISSING)
        #ifdef __MINGW32__
            I=_ttoi64(c_str());
        #elif defined(UNICODE)
            std::string S=To_UTF8();
            I=atoll(S.c_str());
        #else //defined(UNICODE)
            I=atoll(c_str());
        #endif //defined(UNICODE)
    #else
        tStringStream Stream(*this);
        Stream >> setbase(Radix) >> I;
        if (Stream.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(__T("."))!=std::string::npos)
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
    #if defined(STREAM_MISSING)
        #ifdef __MINGW32__
            I=_ttoi64(c_str()); //TODO : I>0x7FFFFFFFFFFFFFFF
        #elif defined(UNICODE)
            std::string S=To_UTF8();
            I=atoll(S.c_str());
        #else //defined(UNICODE)
            I=atoll(c_str());
        #endif //defined(UNICODE)
    #else
        tStringStream Stream(*this);
        Stream >> setbase(Radix) >> I;
        if (Stream.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(__T("."))!=std::string::npos)
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
        if ((Temp[Pos]< __T('0') || Temp[Pos]> __T('9'))
         && (Temp[Pos]< __T('A') || Temp[Pos]> __T('F'))
         && (Temp[Pos]< __T('a') || Temp[Pos]> __T('f')))
            return 0;
        if (Temp[Pos]>=__T('A') && Temp[Pos]<=__T('F'))
        {
            Temp[Pos]-=__T('A');
            Temp[Pos]+=__T('9')+1;
        }
        if (Temp[Pos]>=__T('a') && Temp[Pos]<=__T('f'))
        {
            Temp[Pos]-=__T('a');
            Temp[Pos]+=__T('9')+1;
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
    #if defined(STREAM_MISSING)
        #ifdef UNICODE
            return (wcstod(c_str(),NULL));
        #else
            return (strtod(c_str(),NULL));
        #endif
    #else
        float32 F;
        tStringStream Stream(*this);
        Stream >> F;
        if (Stream.fail())
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
    #if defined(STREAM_MISSING)
        #ifdef UNICODE
            return (wcstod(c_str(),NULL)); //TODO verify no wcstold
        #else
            return (strtod(c_str(),NULL)); //TODO verify no strtold
        #endif
    #else
        float64 F;
        tStringStream Stream(*this);
        Stream >> F;
        if (Stream.fail())
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
    #if defined(STREAM_MISSING)
        #ifdef UNICODE
            return (wcstod(c_str(),NULL)); //TODO verify no wcstold
        #else
            return (strtod(c_str(),NULL)); //TODO verify no strtold
        #endif
    #else
        float80 F;
        tStringStream Stream(*this);
        Stream >> F;
        if (Stream.fail())
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
    //Recherche Debut
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
        if (operator[](Pos)<__T('0') || operator[](Pos)>__T('9'))
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
        if (Comparator==__T("==")) return (Left==Right);
        if (Comparator==__T("<"))  return (Left< Right);
        if (Comparator==__T("<=")) return (Left<=Right);
        if (Comparator==__T(">=")) return (Left>=Right);
        if (Comparator==__T(">"))  return (Left> Right);
        if (Comparator==__T("!=")) return (Left!=Right);
        if (Comparator==__T("<>")) return (Left!=Right);
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
        if (Comparator==__T("==")) return (Left==Right);
        if (Comparator==__T("IN")) {if (Left.find(Right)!=string::npos) return true; else return false;}
        if (Comparator==__T("<"))  return (Left< Right);
        if (Comparator==__T("<=")) return (Left<=Right);
        if (Comparator==__T(">=")) return (Left>=Right);
        if (Comparator==__T(">"))  return (Left> Right);
        if (Comparator==__T("!=")) return (Left!=Right);
        if (Comparator==__T("<>")) return (Left!=Right);
        return false;
    }
    else
    {
        //string comparasion
        if (Comparator==__T("==")) return (*this==ToCompare);
        if (Comparator==__T("IN")) {if (this->find(ToCompare)!=string::npos) return true; else return false;}
        if (Comparator==__T("<"))  return (*this< ToCompare);
        if (Comparator==__T("<=")) return (*this<=ToCompare);
        if (Comparator==__T(">=")) return (*this>=ToCompare);
        if (Comparator==__T(">"))  return (*this> ToCompare);
        if (Comparator==__T("!=")) return (*this!=ToCompare);
        if (Comparator==__T("<>")) return (*this!=ToCompare);
        return false;
    }
}

} //namespace
