/*
 * (C) 2016-2017 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <wincrypt.h>

#define CRYPT_KEYLENGTH 0x01000000
#define Z_ENCODING_GZIP 16
#define Z_DECODING_ZLIB_GZIP 32
#define PROBE_SIZE 64 * 1024

struct SubtitlesInfo;

namespace SubtitlesProvidersUtils
{
    using regexResult = std::vector<std::string>;
    using regexResults = std::vector<regexResult>;
    using stringMap = std::unordered_map<std::string, std::string>;
    using stringArray = std::vector<std::string>;

    static constexpr std::regex::flag_type RegexFlags(std::regex_constants::ECMAScript | std::regex_constants::icase | std::regex_constants::optimize);

    int LevenshteinDistance(std::string s, std::string t);

    std::string StringToHex(const std::string& data);
    std::string StringToHex(const int& data);
    std::string StringToHash(const std::string& data, ALG_ID Algid = CALG_MD5);


    std::string StringEncrypt(const std::string& data, const std::string& key, ALG_ID Algid = CALG_AES_256);
    std::string StringDecrypt(const std::string& data, const std::string& key, ALG_ID Algid = CALG_AES_256);

    std::string StringFormat(_In_z_ _Printf_format_string_ char const* const fmt, ...);

    size_t stringMatch(const std::string& pattern, const std::string& text, regexResults& results);
    size_t stringMatch(const std::string& pattern, const std::string& text, regexResult& result);
    size_t stringMatch(const std::regex& pattern, const std::string& text, regexResults& results);
    size_t stringMatch(const std::regex& pattern, const std::string& text, regexResult& result);

    std::string StringGzipDeflate(const std::string& data);
    std::string StringGzipCompress(const std::string& data);
    std::string StringGzipInflate(const std::string& data);
    std::string StringGzipUncompress(const std::string& data);

    int FileUnzip(CStringA fn, stringMap& dataOut);
    bool FileUnRar(CString fn, stringMap& dataOut);
    int CALLBACK UnRarProc(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2);

    stringMap StringUncompress(const std::string& data, const std::string& fileName);
    std::string StringGenerateUniqueKey();

    HRESULT StringDownload(const std::string& url, const stringMap& headers, std::string& data,
                           bool bAutoRedirect = true, DWORD* dwStatusCode = nullptr);
    HRESULT StringUpload(const std::string& url, const stringMap& headers, const std::string& content, std::string& data,
                         bool bAutoRedirect = true, DWORD* dwStatusCode = nullptr);

    stringArray StringTokenize(const std::string& text, const std::string& delimiters, bool blank = false);
    std::string StringTrim(const std::string& text, const std::string& characters = " ", int side = 0);
    std::string StringReplace(const std::string& text, const std::string& find, const std::string& replace);

    std::list<std::string> LanguagesISO6391();
    std::list<std::string> LanguagesISO6392();
    UINT64 GenerateOSHash(SubtitlesInfo& pFileInfo);

    template <typename T>
    std::string JoinContainer(const T& c, LPCSTR delim)
    {
        std::ostringstream stringStream;
        if (c.cbegin() != c.cend()) {
            std::copy(c.cbegin(), std::prev(c.cend()), std::ostream_iterator<typename T::value_type>(stringStream, delim));
            stringStream << *c.crbegin();
        }
        return stringStream.str();
    }
}
