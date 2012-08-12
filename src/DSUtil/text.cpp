/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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

#include "stdafx.h"
#include "text.h"

CStringA ConvertMBCS(CStringA str, DWORD SrcCharSet, DWORD DstCharSet)
{
    unsigned int uiStrLength = str.GetLength();
    wchar_t* utf16 = DNewNT wchar_t[uiStrLength];
    if (!utf16) {
        ASSERT(0);
        return "";
    }

    // relate the input and output code page trough UTF-16
    UINT SrcCodeP = 0, DstCodeP = 0;
    CHARSETINFO cs;
    // zero-extend the incoming charset values if required, these calls do not actually use them as a pointer
    if (::TranslateCharsetInfo(reinterpret_cast<DWORD*>(static_cast<uintptr_t>(SrcCharSet)), &cs, TCI_SRCCHARSET)) {
        SrcCodeP = cs.ciACP;
    }
    if (::TranslateCharsetInfo(reinterpret_cast<DWORD*>(static_cast<uintptr_t>(DstCharSet)), &cs, TCI_SRCCHARSET)) {
        DstCodeP = cs.ciACP;
    }

    int len = MultiByteToWideChar(SrcCodeP, 0, str, uiStrLength, utf16, uiStrLength * 2);
    if (!len) {
        ASSERT(0);
        delete [] utf16;
        return "";
    }

    char *pRawStr = str.GetBuffer(uiStrLength * 6); // allow 5 times the string length as extra headroom
    len = WideCharToMultiByte(DstCodeP, 0, utf16, len, pRawStr, uiStrLength * 6, NULL, NULL);
    ASSERT(len);
    str.ReleaseBuffer(len); // also works nicely when len equals 0

    delete [] utf16;
    return str;
}

CStringA UrlEncode(CStringA str, bool fRaw)
{
    char separator = fRaw ? ' ' : '+';
    CStringA urlstr;

    int len = str.GetLength();
    for (int i = 0; i < len; ++i) {
        CHAR c = str[i];
        if (fRaw && c == '+') {
            urlstr += "%2B";
        } else if (c > 0x20 && c < 0x7f && c != '&') {
            urlstr += c;
        } else if (c == 0x20) {
            urlstr += separator;
        } else {
            urlstr.AppendFormat("%%%02hx", c);
        }
    }

    return urlstr;
}

CStringA UrlDecode(CStringA str, bool fRaw)
{
    str.Replace("&amp;", "&");

    CHAR* s = str.GetBuffer(str.GetLength());
    CHAR* e = s + str.GetLength();
    CHAR* s1 = s;
    CHAR* s2 = s;
    while (s1 < e) {
        CHAR s11 = (s1 < e - 1) ? (__isascii(s1[1]) && isupper(static_cast<unsigned char>(s1[1])) ? tolower(s1[1]) : s1[1]) : 0;
        CHAR s12 = (s1 < e - 2) ? (__isascii(s1[2]) && isupper(static_cast<unsigned char>(s1[2])) ? tolower(s1[2]) : s1[2]) : 0;

        if (*s1 == '%' && s1 < e - 2
                && (s1[1] >= '0' && s1[1] <= '9' || s11 >= 'a' && s11 <= 'f')
                && (s1[2] >= '0' && s1[2] <= '9' || s12 >= 'a' && s12 <= 'f')) {
            s1[1] = s11;
            s1[2] = s12;
            *s2 = 0;
            if (s1[1] >= '0' && s1[1] <= '9') {
                *s2 |= s1[1] - '0';
            } else if (s1[1] >= 'a' && s1[1] <= 'f') {
                *s2 |= s1[1] - 'a' + 10;
            }
            *s2 <<= 4;
            if (s1[2] >= '0' && s1[2] <= '9') {
                *s2 |= s1[2] - '0';
            } else if (s1[2] >= 'a' && s1[2] <= 'f') {
                *s2 |= s1[2] - 'a' + 10;
            }
            s1 += 2;
        } else {
            *s2 = *s1 == '+' && !fRaw ? ' ' : *s1;
        }

        s1++;
        s2++;
    }

    str.ReleaseBuffer(int(s2 - s));

    return str;
}

CString ExtractTag(CString tag, CMapStringToString& attribs, bool& fClosing)
{
    tag.Trim();
    attribs.RemoveAll();

    fClosing = !tag.IsEmpty() ? tag[0] == '/' : false;
    tag.TrimLeft('/');

    int i = tag.Find(' ');
    if (i < 0) {
        i = tag.GetLength();
    }
    CString type = tag.Left(i).MakeLower();
    tag = tag.Mid(i).Trim();

    while ((i = tag.Find('=')) > 0) {
        CString attrib = tag.Left(i).Trim().MakeLower();
        tag = tag.Mid(i + 1);
        for (i = 0; i < tag.GetLength() && _istspace(tag[i]); i++) {
            ;
        }
        tag = i < tag.GetLength() ? tag.Mid(i) : _T("");
        if (!tag.IsEmpty() && tag[0] == '\"') {
            tag = tag.Mid(1);
            i = tag.Find('\"');
        } else {
            i = tag.Find(' ');
        }
        if (i < 0) {
            i = tag.GetLength();
        }
        CString param = tag.Left(i).Trim();
        if (!param.IsEmpty()) {
            attribs[attrib] = param;
        }
        tag = i + 1 < tag.GetLength() ? tag.Mid(i + 1) : _T("");
    }

    return type;
}

CAtlList<CString>& MakeLower(CAtlList<CString>& sl)
{
    POSITION pos = sl.GetHeadPosition();
    while (pos) {
        sl.GetNext(pos).MakeLower();
    }
    return sl;
}

CAtlList<CString>& MakeUpper(CAtlList<CString>& sl)
{
    POSITION pos = sl.GetHeadPosition();
    while (pos) {
        sl.GetNext(pos).MakeUpper();
    }
    return sl;
}

CString FormatNumber(CString szNumber, bool bNoFractionalDigits /*= true*/)
{
    CString ret;

    int nChars = GetNumberFormat(LOCALE_USER_DEFAULT, 0, szNumber, NULL, NULL, 0);
    GetNumberFormat(LOCALE_USER_DEFAULT, 0, szNumber, NULL, ret.GetBuffer(nChars), nChars);
    ret.ReleaseBuffer();

    if (bNoFractionalDigits) {
        TCHAR szNumberFractionalDigits[2] = {0};
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDIGITS, szNumberFractionalDigits, _countof(szNumberFractionalDigits));
        int nNumberFractionalDigits = _tcstol(szNumberFractionalDigits, NULL, 10);
        if (nNumberFractionalDigits) {
            ret.Truncate(ret.GetLength() - nNumberFractionalDigits - 1);
        }
    }

    return ret;
}
