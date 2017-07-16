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

#include <afxstr.h>

template<typename StringType>
struct ISOLangT {
    StringType name, iso6392, iso6391;
    LCID lcid;

    constexpr ISOLangT(StringType name = nullptr, StringType iso6392 = nullptr, StringType iso6391 = nullptr, LCID lcid = 0)
        : name(name), iso6392(iso6392), iso6391(iso6391), lcid(lcid)
    {};

    template<typename StringType2>
    constexpr ISOLangT(const ISOLangT<StringType2>& isoLang)
        : name(isoLang.name), iso6392(isoLang.iso6392), iso6391(isoLang.iso6391), lcid(isoLang.lcid)
    {};

    template<typename StringType2>
    ISOLangT& operator=(const ISOLangT<StringType2>& isoLang) = delete;
};

struct ISOLang : public ISOLangT<LPCSTR> {
    using ISOLangT::ISOLangT;

    static CString ISO6391ToLanguage(LPCSTR code);
    static CString ISO6392ToLanguage(LPCSTR code);
    static bool IsISO639Language(LPCSTR code);
    static CString ISO639XToLanguage(LPCSTR code, bool bCheckForFullLangName = false);
    static LCID ISO6391ToLcid(LPCSTR code);
    static LCID ISO6392ToLcid(LPCSTR code);
    static CStringA ISO6391To6392(LPCSTR code);
    static CString ISO6392To6391(LPCSTR code);
    static CString LanguageToISO6392(LPCTSTR lang);
    static ISOLang ISO6391ToISOLang(LPCSTR code);
    static ISOLang ISO6392ToISOLang(LPCSTR code);
    static ISOLang ISO639XToISOLang(LPCSTR code);
};
