/*
 * (C) 2014 see Authors.txt
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

#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

int LevenshteinDistance(std::string s, std::string t);

std::string string_hex(const std::string& data);

std::string string_hex(const int& data);

std::string string_hash(const std::string& data, ALG_ID Algid = CALG_MD5);

#define CRYPT_KEYLENGTH  0x00800000

std::string string_encrypt(const std::string& data, const std::string& key, ALG_ID Algid = CALG_AES_128);

std::string string_decrypt(const std::string& data, const std::string& key, ALG_ID Algid = CALG_AES_128);

std::string string_format(char const* fmt, ...);

const std::regex::flag_type regex_flags(std::regex_constants::ECMAScript | std::regex_constants::icase | std::regex_constants::optimize);

typedef std::vector<std::string> regex_result;

typedef std::vector<std::vector<std::string>> regex_results;

size_t string_regex(const std::string& pattern, const std::string& text, regex_results& results);

size_t string_regex(const std::string& pattern, const std::string& text, regex_result& result);

size_t string_regex(const std::regex& pattern, const std::string& text, regex_results& results);

size_t string_regex(const std::regex& pattern, const std::string& text, regex_result& result);

#define Z_ENCODING_GZIP 16

std::string string_gzdeflate(const std::string& data);
std::string string_gzcompress(const std::string& data);

#define Z_DECODING_ZLIB_GZIP 32

std::string string_gzinflate(const std::string& data);
std::string string_gzuncompress(const std::string& data);

typedef std::unordered_map<std::string, std::string> string_map;
int file_unzip(CStringA file, CStringA fn, string_map& dataOut);

int CALLBACK UnRarProc(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2);

bool file_unrar(CString fn, string_map& dataOut);

string_map string_uncompress(const std::string& data, const std::string& fileName);

std::string string_generate_unique_key();

HRESULT string_download(const std::string& url, const string_map& headers, std::string& data, BOOL bAutoRedirect = TRUE, DWORD* dwStatusCode = nullptr);

HRESULT string_upload(const std::string& url, const string_map& headers, const std::string& content, std::string& data, BOOL bAutoRedirect = TRUE, DWORD* dwStatusCode = nullptr);

typedef std::vector<std::string> string_array;
string_array string_tokenize(const std::string& text, const std::string& delimiters, bool blank = false);

std::string string_trim(const std::string& text, const std::string& characters = " ", int side = 0);

std::string string_replace(const std::string& text, const std::string& find, const std::string& replace);

std::string LanguagesISO6391(const char delimiter = ',');

std::string LanguagesISO6392(const char delimiter = ',');
