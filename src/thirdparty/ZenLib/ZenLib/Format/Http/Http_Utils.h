/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

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
