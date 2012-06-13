/*
 * $Id$
 *
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

DWORD CharSetToCodePage(DWORD dwCharSet)
{
    if (dwCharSet == CP_UTF8) {
        return CP_UTF8;
    }
    if (dwCharSet == CP_UTF7) {
        return CP_UTF7;
    }
    CHARSETINFO cs = {0};
    ::TranslateCharsetInfo((DWORD*)dwCharSet, &cs, TCI_SRCCHARSET);
    return cs.ciACP;
}

CStringA ConvertMBCS(CStringA str, DWORD SrcCharSet, DWORD DstCharSet)
{
    WCHAR* utf16 = DNew WCHAR[str.GetLength() + 1];
    memset(utf16, 0, (str.GetLength() + 1)*sizeof(WCHAR));

    CHAR* mbcs = DNew CHAR[str.GetLength() * 6 + 1];
    memset(mbcs, 0, str.GetLength() * 6 + 1);

    int len = MultiByteToWideChar(
                  CharSetToCodePage(SrcCharSet), 0,
                  str, -1, // null terminated string
                  utf16, str.GetLength() + 1);

    len = WideCharToMultiByte(
              CharSetToCodePage(DstCharSet), 0,
              utf16, len,
              mbcs, str.GetLength() * 6,
              NULL, NULL);

    str = mbcs;

    delete [] utf16;
    delete [] mbcs;

    return str;
}

CStringA UrlEncode(CStringA str, bool fRaw)
{
    CStringA urlstr;

    for (int i = 0; i < str.GetLength(); i++) {
        CHAR c = str[i];
        if (fRaw && c == '+') {
            urlstr += "%2B";
        } else if (c > 0x20 && c < 0x7f && c != '&') {
            urlstr += c;
        } else if (c == 0x20) {
            urlstr += fRaw ? ' ' : '+';
        } else {
            CStringA tmp;
            tmp.Format("%%%02x", (BYTE)c);
            urlstr += tmp;
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
        CHAR s11 = (s1 < e - 1) ? (__isascii(s1[1]) && isupper(s1[1]) ? tolower(s1[1]) : s1[1]) : 0;
        CHAR s12 = (s1 < e - 2) ? (__isascii(s1[2]) && isupper(s1[2]) ? tolower(s1[2]) : s1[2]) : 0;

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
