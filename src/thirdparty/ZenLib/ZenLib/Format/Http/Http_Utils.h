// ZenLib::Format::Http::Utils - Utils for HTTP handling
// Copyright (C) 2008-2011 MediaArea.net SARL, Info@MediaArea.net
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
// HTTP utils
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenLib_Server_Http_UtilsH
#define ZenLib_Server_Http_UtilsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <string>
#include <ctime>
#include <map>
#include <vector>
//---------------------------------------------------------------------------

namespace ZenLib
{

namespace Format
{

namespace Http
{

//***************************************************************************
//
//***************************************************************************

//---------------------------------------------------------------------------
// Character manipulation
unsigned char Char2Hex (unsigned char Char);
wchar_t       Char2Hex (wchar_t Char);
std::string   Hex2Char (unsigned char Char);
std::wstring  Hex2Char (wchar_t Char);

//---------------------------------------------------------------------------
// URL manipulation
std::string URL_Encoded_Encode (const std::string& URL);
std::wstring URL_Encoded_Encode (const std::wstring& URL);
std::string URL_Encoded_Decode (const std::string& URL);
std::wstring URL_Encoded_Decode (const std::wstring& URL);

//---------------------------------------------------------------------------
// Cleanup
void TrimLeft (std::string& String, char ToTrim);

} //Namespace

} //Namespace

} //Namespace

#endif //ZENLIB_SERVER_HTTP_UTILS




