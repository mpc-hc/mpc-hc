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
#ifndef ZenLib_ZtringH
#define ZenLib_ZtringH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Utils.h"
#include <string>
#include <sstream>
//---------------------------------------------------------------------------

namespace ZenLib
{

//---------------------------------------------------------------------------
typedef std::basic_string<Char, std::char_traits<Char>, std::allocator<Char> > tstring;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/// @brief Options for Ztring methods
enum ztring_t
{
    Ztring_Nothing,
    Ztring_Rounded              =  1,           ///< if >.5, upper, else lower
    Ztring_CaseSensitive        =  2,           ///< Case sensitive ("A" and "a" are different)
    Ztring_AddLastItem          =  4,           ///< if Begin is found and End is not found, return between Begin and end of string
    Ztring_Recursive            =  8,           ///< Do all strings
    Ztring_NoZero               = 16            ///> Doesn't keep Zero in the float number
};

//---------------------------------------------------------------------------

//***************************************************************************
/// @brief String manipulation (based on std::(w)string)
//***************************************************************************

class Ztring : public tstring  //for details about undocumented methods see http://www.sgi.com/tech/stl/basic_string.html
{
public :
    //Constructor/destructor
    Ztring ()                                                                   : tstring(){};
    Ztring (const tstring& str)                                                 : tstring(str){};
    Ztring (const tstring& str, size_type pos, size_type n=npos)                : tstring(str, pos, n){};
    Ztring (const Char* s, size_type n)                                         : tstring(s, n){};
    Ztring (const Char* s)                                                      : tstring(s){};
    Ztring (size_type n, Char c)                                                : tstring(n, c){};
    #ifdef UNICODE
    Ztring (const char* S)                                                      : tstring(){From_UTF8(S);};
    Ztring (const char* S, size_type n)                                         : tstring(){From_UTF8(S, 0, n);};
    #endif //UNICODE

    //Operators
        ///Same as [], but resize the string if Pos doesn't exist yet
    Char &operator () (size_type Pos);

    //Assign
    bool Assign_FromFile (const Ztring &FileName);

    //Conversions - From
    #ifndef WSTRING_MISSING
        /// @brief convert an Unicode encoded string into Ztring
    Ztring& From_Unicode (const std::wstring &S)                                {return From_Unicode(S.c_str());};
    #endif //WSTRING_MISSING
        /// @brief convert an Unicode encoded wchar_t into Ztring
    Ztring& From_Unicode (const wchar_t S);
        /// @brief convert an Unicode encoded string into Ztring
    Ztring& From_Unicode (const wchar_t *S);
        /// @brief convert an Unicode encoded string into Ztring
    Ztring& From_Unicode (const wchar_t *S, size_type Start, size_type Length);
        /// @brief convert an Unicode encoded string into Ztring
    Ztring& From_Unicode (const wchar_t *S, size_type Length)                   {return From_Unicode(S, 0, Length);};
        /// @brief convert an UTF-8 encoded string into Ztring
    Ztring& From_UTF8    (const std::string &S)                                 {return From_UTF8(S.c_str());};
        /// @brief convert an UTF-8 encoded string into Ztring
    Ztring& From_UTF8    (const char *S);
        /// @brief convert an UTF-8 encoded string into Ztring
    Ztring& From_UTF8    (const char *S, size_type Start, size_type Length);
        /// @brief convert an UTF-8 encoded string into Ztring
    Ztring& From_UTF8    (const char *S, size_type Length)                      {return From_UTF8(S, 0, Length);};
        /// @brief convert an UTF-16 encoded string into Ztring
    Ztring& From_UTF16   (const char *S);
        /// @brief convert an UTF-16 encoded string into Ztring
    Ztring& From_UTF16   (const char *S, size_type Start, size_type Length);
        /// @brief convert an UTF-16 encoded string into Ztring
    Ztring& From_UTF16   (const char *S, size_type Length)                      {return From_UTF16(S, 0, Length);};
        /// @brief convert an UTF-16BE encoded string into Ztring
    Ztring& From_UTF16BE (const char *S);
        /// @brief convert an UTF-16BE encoded string into Ztring
    Ztring& From_UTF16BE (const char *S, size_type Start, size_type Length);
        /// @brief convert an UTF-16BE encoded string into Ztring
    Ztring& From_UTF16BE (const char *S, size_type Length)                      {return From_UTF16BE(S, 0, Length);};
        /// @brief convert an UTF-16LE encoded string into Ztring
    Ztring& From_UTF16LE (const char *S);
        /// @brief convert an UTF-16LE encoded string into Ztring
    Ztring& From_UTF16LE (const char *S, size_type Start, size_type Length);
        /// @brief convert an UTF-16LE encoded string into Ztring
    Ztring& From_UTF16LE (const char *S, size_type Length)                      {return From_UTF16LE(S, 0, Length);};
        /// @brief convert an Locael encoded string into Ztring
    Ztring& From_Local   (const std::string &S)                                 {return From_Local(S.c_str());};
        /// @brief convert an Local encoded string into Ztring
    Ztring& From_Local   (const char  *S);
        /// @brief convert an Local encoded string into Ztring
    Ztring& From_Local   (const char  *S, size_type Start,  size_type Length);
        /// @brief convert an Local encoded string into Ztring
    Ztring& From_Local   (const char  *S, size_type Length)                     {return From_Local(S, 0, Length);};

        /// @brief convert an ISO-8859-1 encoded string into Ztring
    Ztring& From_ISO_8859_1   (const char  *S);
        /// @brief convert an ISO-8859-1 encoded string into Ztring
    Ztring& From_ISO_8859_1   (const char  *S, size_type Start,  size_type Length);
        /// @brief convert an ISO-8859-1 encoded string into Ztring
    Ztring& From_ISO_8859_1   (const char  *S, size_type Length)                {return From_ISO_8859_1(S, 0, Length);};

        /// @brief convert an ISO-8859-2 encoded string into Ztring
    Ztring& From_ISO_8859_2   (const char  *S);
        /// @brief convert an ISO-8859-1 encoded string into Ztring
    Ztring& From_ISO_8859_2   (const char  *S, size_type Start,  size_type Length);
        /// @brief convert an ISO-8859-1 encoded string into Ztring
    Ztring& From_ISO_8859_2   (const char  *S, size_type Length)                {return From_ISO_8859_2(S, 0, Length);};

        /// @brief convert an 16 byte GUID into Ztring
    Ztring& From_GUID    (const int128u S);
        /// @brief convert an 16 byte UUID into Ztring
    Ztring& From_UUID    (const int128u S);
        /// @brief convert an 4 Character Code into Ztring
    Ztring& From_CC4     (const char  *S)                                       {return From_Local(S, 0, 4);};
        /// @brief convert an 4 Character Code into Ztring
    Ztring& From_CC4     (const int8u *S)                                       {return From_Local((const char*)S, 0, 4);};
        /// @brief convert an 4 Character Code into Ztring
    Ztring& From_CC4     (const int32u S);
        /// @brief convert an 2 Character Code into Ztring
    Ztring& From_CC3     (const char  *S)                                       {return From_Local(S, 0, 3);};
        /// @brief convert an 4 Character Code into Ztring
    Ztring& From_CC3     (const int8u *S)                                       {return From_Local((const char*)S, 0, 3);};
        /// @brief convert an 4 Character Code into Ztring
    Ztring& From_CC3     (const int32u S);
        /// @brief convert an 2 Character Code into Ztring
    Ztring& From_CC2     (const char  *S)                                       {return From_CC2(ZenLib::CC2(S));};
        /// @brief convert an 2 Character Code into Ztring
    Ztring& From_CC2     (const int8u *S)                                       {return From_CC2(ZenLib::CC2(S));};
        /// @brief convert an 2 Character Code into Ztring
    Ztring& From_CC2     (const int16u S);
        /// @brief convert an 1 Character Code into Ztring
    Ztring& From_CC1     (const char  *S)                                       {return From_CC1(ZenLib::CC1(S));};
        /// @brief convert an 1 Character Code into Ztring
    Ztring& From_CC1     (const int8u *S)                                       {return From_CC1(ZenLib::CC1(S));};
        /// @brief convert an 1 Character Code into Ztring
    Ztring& From_CC1     (const int8u  S);
        /// @brief convert number into Ztring
    Ztring& From_Number  (const int8s,    int8u Radix=10);
        /// @brief convert number into Ztring
    Ztring& From_Number  (const int8u,    int8u Radix=10);
        /// @brief convert number into Ztring
    Ztring& From_Number  (const int16s,   int8u Radix=10);
        /// @brief convert number into Ztring
    Ztring& From_Number  (const int16u,   int8u Radix=10);
        /// @brief convert number into Ztring
    Ztring& From_Number  (const int32s,   int8u Radix=10);
        /// @brief convert number into Ztring
    Ztring& From_Number  (const int32u,   int8u Radix=10);
        /// @brief convert number into Ztring
    Ztring& From_Number  (const int64s,   int8u Radix=10);
        /// @brief convert number into Ztring
    Ztring& From_Number  (const int64u,   int8u Radix=10);
        /// @brief convert number into Ztring
    Ztring& From_Number  (const int128u,  int8u Radix=10);
    /// @brief convert number into Ztring
    Ztring& From_Number  (const float32,  int8u AfterComma=3, ztring_t Options=Ztring_Nothing);
        /// @brief convert number into Ztring
    Ztring& From_Number  (const float64,  int8u AfterComma=3, ztring_t Options=Ztring_Nothing);
        /// @brief convert number into Ztring
    Ztring& From_Number  (const float80,  int8u AfterComma=3, ztring_t Options=Ztring_Nothing);
    #ifdef SIZE_T_IS_LONG
        /// @brief convert number into Ztring
    Ztring& From_Number  (const size_t,   int8u Radix=10);
    #endif //SIZE_T_IS_LONG
        /// @brief convert number (BCD coded) into Ztring
    Ztring& From_BCD     (const int8u);
        /// @brief convert count of milliseconds into a readable and sortable string
    Ztring& Duration_From_Milliseconds (const int64s Milliseconds);
        /// @deprecated replaced by the int64s version
    Ztring& Duration_From_Milliseconds (const int64u Milliseconds);
        /// @brief convert count of seconds since 1601 into a readable and sortable string
    Ztring& Date_From_Milliseconds_1601 (const int64u Milliseconds);
        /// @brief convert count of seconds since 1601 into a readable and sortable string
    Ztring& Date_From_Seconds_1601 (const int64u Seconds);
        /// @brief convert count of seconds since 1900 into a readable and sortable string
    Ztring& Date_From_Seconds_1900 (const int32u Seconds);
        /// @brief convert count of seconds since 1900 into a readable and sortable string
    Ztring& Date_From_Seconds_1900 (const int64s Seconds);
        /// @brief convert count of seconds since 1904 into a readable and sortable string
    Ztring& Date_From_Seconds_1904 (const int32u Seconds);
        /// @brief convert count of seconds since 1904 into a readable and sortable string
    Ztring& Date_From_Seconds_1904 (const int64u Seconds);
        /// @brief convert count of seconds since 1904 into a readable and sortable string
    Ztring& Date_From_Seconds_1904 (const int64s Seconds);
        /// @brief convert count of seconds since 1970 into a readable and sortable string
    Ztring& Date_From_Seconds_1970 (const int32u Seconds);
        /// @brief convert count of seconds since 1970 into a readable and sortable string
    Ztring& Date_From_Seconds_1970 (const int32s Seconds);
        /// @brief convert count of seconds since 1970 into a readable and sortable string
    Ztring& Date_From_Seconds_1970 (const int64s Seconds);
        /// @brief convert count of seconds since 1970 into a readable and sortable string (in local time)
    Ztring& Date_From_Seconds_1970_Local (const int32u Seconds);
        /// @brief convert a free formated string into a readable and sortable string
    Ztring& Date_From_String (const char* Date, size_type Value_Size=Error);
        /// @brief convert numbers into a readable and sortable string
    Ztring& Date_From_Numbers (const int8u Year, const int8u Month, const int8u Day, const int8u Hour, const int8u Minute, const int8u Second);

    //Conversions - To
    #ifndef WSTRING_MISSING
        /// @brief Convert into Unicode chars
        /// @return the string corresponding \n
    std::wstring To_Unicode () const;
    #endif //WSTRING_MISSING
        /// @brief Convert into char* (UTF-8 encoded)
        /// @return the string corresponding \n
    std::string To_UTF8     () const;
        /// @brief Convert into char* (Local encoded)
        /// @return the string corresponding \n
    std::string To_Local    () const;
        /// @brief Convert into 16 byte UUID number
        /// @return the value corresponding \n
        ///         0 if there is a problem
    int128u     To_UUID    () const;
        /// @brief Convert into a 4 Character Code
        /// @return the value corresponding \n
        ///         0 if there is a problem
    int32u      To_CC4    () const;
        /// @brief Convert into Int (8 bits)
        /// @return the value corresponding \n
        ///         0 if there is a problem
    int8s       To_int8s    (int8u Radix=10, ztring_t Options=Ztring_Rounded) const;
        /// @brief Convert into unsigned Int (8 bits)
        /// @return the value corresponding
        ///         0 if there is a problem
    int8u       To_int8u    (int8u Radix=10, ztring_t Options=Ztring_Rounded) const;
        /// @brief Convert into Int (16 bits)
        /// @return the value corresponding \n
        ///         0 if there is a problem
    int16s      To_int16s   (int8u Radix=10, ztring_t Options=Ztring_Rounded) const;
        /// @brief Convert into unsigned Int (16 bits)
        /// @return the value corresponding
        ///         0 if there is a problem
    int16u      To_int16u   (int8u Radix=10, ztring_t Options=Ztring_Rounded) const;
        /// @brief Convert into Int (32 bits)
        /// @return the value corresponding \n
        ///         0 if there is a problem
    int32s      To_int32s   (int8u Radix=10, ztring_t Options=Ztring_Rounded) const;
        /// @brief Convert into unsigned Int (32 bits)
        /// @return the value corresponding
        ///         0 if there is a problem
    int32u      To_int32u   (int8u Radix=10, ztring_t Options=Ztring_Rounded) const;
        /// @brief Convert into Int (64 bits)
        /// @return the value corresponding \n
        ///         0 if there is a problem
    int64s      To_int64s   (int8u Radix=10, ztring_t Options=Ztring_Rounded) const;
        /// @brief Convert into unsigned Int (64 bits)
        /// @return the value corresponding \n
        ///         0 if there is a problem
    int64u      To_int64u   (int8u Radix=10, ztring_t Options=Ztring_Rounded) const;
        /// @brief Convert into unsigned Int (64 bits)
        /// @warning only hexadecimal and no rounding are currenlty supported \n
        /// @return the value corresponding \n
        ///         0 if there is a problem
    int128u     To_int128u   (int8u Radix=10, ztring_t Options=Ztring_Rounded) const;
        /// @brief Convert into float
        /// @return the value corresponding \n
        ///         0 if there is a problem
    float32     To_float32  (ztring_t Options=Ztring_Nothing) const;
    float64     To_float64  (ztring_t Options=Ztring_Nothing) const;
    float80     To_float80  (ztring_t Options=Ztring_Nothing) const;

    //Static versions
    static Ztring ToZtring_From_Local(const std::string &S)                               {return Ztring().From_Local(S);};
    static Ztring ToZtring_From_Local(const char  *S)                                     {return Ztring().From_Local(S);};
    static Ztring ToZtring_From_Local(const char  *S, size_type Start,  size_type Length) {return Ztring().From_Local(S, Start, Length);};
    static Ztring ToZtring_From_Local(const char  *S, size_type Length)                   {return Ztring().From_Local(S, Length);};
    static Ztring ToZtring_From_CC4  (const char  *S)                                     {return Ztring().From_CC4(S);};
    static Ztring ToZtring_From_CC4  (const int8u *S)                                     {return Ztring().From_CC4(S);};
    static Ztring ToZtring_From_CC4  (const int32u S)                                     {return Ztring().From_CC4(S);};
    static Ztring ToZtring_From_CC3  (const char  *S)                                     {return Ztring().From_CC3(S);};
    static Ztring ToZtring_From_CC3  (const int8u *S)                                     {return Ztring().From_CC3(S);};
    static Ztring ToZtring_From_CC3  (const int32u S)                                     {return Ztring().From_CC3(S);};
    static Ztring ToZtring_From_CC2  (const char  *S)                                     {return Ztring().From_CC2(S);};
    static Ztring ToZtring_From_CC2  (const int8u *S)                                     {return Ztring().From_CC2(S);};
    static Ztring ToZtring_From_CC2  (const int16u S)                                     {return Ztring().From_CC2(S);};
    static Ztring ToZtring_From_CC1  (const char  *S)                                     {return Ztring().From_CC1(S);};
    static Ztring ToZtring_From_CC1  (const int8u *S)                                     {return Ztring().From_CC1(S);};
    static Ztring ToZtring_From_CC1  (const int8u  S)                                     {return Ztring().From_CC1(S);};
    static Ztring ToZtring  (const int8s    I, int8u Radix=10)                   {return Ztring().From_Number(I, Radix);};
    static Ztring ToZtring  (const int8u    I, int8u Radix=10)                   {return Ztring().From_Number(I, Radix);};
    static Ztring ToZtring  (const int16s   I, int8u Radix=10)                   {return Ztring().From_Number(I, Radix);};
    static Ztring ToZtring  (const int16u   I, int8u Radix=10)                   {return Ztring().From_Number(I, Radix);};
    static Ztring ToZtring  (const int32s   I, int8u Radix=10)                   {return Ztring().From_Number(I, Radix);};
    static Ztring ToZtring  (const int32u   I, int8u Radix=10)                   {return Ztring().From_Number(I, Radix);};
    static Ztring ToZtring  (const int64s   I, int8u Radix=10)                   {return Ztring().From_Number(I, Radix);};
    static Ztring ToZtring  (const int64u   I, int8u Radix=10)                   {return Ztring().From_Number(I, Radix);};
    static Ztring ToZtring  (const int128u  I, int8u Radix=10)                   {return Ztring().From_Number(I, Radix);};
    static Ztring ToZtring  (const float32  F, int8u AfterComma=3)               {return Ztring().From_Number(F, AfterComma);};
    static Ztring ToZtring  (const float64  F, int8u AfterComma=3)               {return Ztring().From_Number(F, AfterComma);};
    static Ztring ToZtring  (const float80  F, int8u AfterComma=3)               {return Ztring().From_Number(F, AfterComma);};
    #ifdef SIZE_T_IS_LONG
    static Ztring ToZtring  (const size_t   I,  int8u Radix=10)                  {return Ztring().From_Number(I, Radix);};
    #endif //SIZE_T_IS_LONG

    //Edition
        /// @brief test if it is a number
    bool IsNumber() const;
        /// @brief convert into lowercase
    Ztring &MakeLowerCase();
        /// @brief convert into uppercase
    Ztring &MakeUpperCase();
        /// @brief Remove leading whitespaces from a string
    Ztring &TrimLeft(Char ToTrim=__T(' '));
        /// @brief Remove trailing whitespaces from a string
    Ztring &TrimRight(Char ToTrim=__T(' '));
        /// @brief Remove leading and trailing whitespaces from a string
    Ztring &Trim(Char ToTrim=__T(' '));
        /// @brief Quotes a string
    Ztring &Quote(Char ToTrim=__T('\"'));
        /// @brief return a string between two strings
        /// @param Begin First string
        /// @param End Second string
        /// @param Pos Position to begin to scan string
        /// @param Options Options for searching \n
        ///                Available : Ztring_CaseSensitive
        /// @return The substring \n
        ///         "" if not found
    Ztring SubString (const tstring &Begin, const tstring &End, size_type Pos=0, ztring_t Options=Ztring_Nothing) const;
        /// @brief replace a string by another one
        /// @param ToFind string to find
        /// @param ToReplace string wich replace the string found
        /// @param Pos Position to begin to scan string
        /// @param Options Options for searching \n
        ///                Available : Ztring_CaseSensitive, Ztring_Recursive
        /// @return The count of replacements
    size_type FindAndReplace (const tstring &ToFind, const tstring &ReplaceBy, size_type Pos=0, ztring_t Options=Ztring_Nothing); //Remplace une chaine par une autre
        /// @brief Count the number of occurencies of a string in the string
        /// @param ToCount string to count
        /// @param Options Options for count \n
        ///                Available : Ztring_CaseSensitive
        /// @return the count

    //Information
    size_type Count (const Ztring &ToCount, ztring_t Options=Ztring_Nothing) const;
        /// @brief compare with another string
        /// @param ToCompare string to compare with
        /// @param Options Options for comaparing \n
        ///                Available : Ztring_CaseSensitive
        /// @return The result of comparasion
    bool Compare (const Ztring &ToCompare, const Ztring &Comparator=__T("=="), ztring_t Options=Ztring_Nothing) const;
};

} //NameSpace

#endif
